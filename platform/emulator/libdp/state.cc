/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "state.hh"
#endif

#include "base.hh"
#include "perdio.hh"
#include "state.hh"
#include "chain.hh"
#include "controlvar.hh"
#include "protocolState.hh"
#include "table.hh"

//
static inline void sendPrepOwner(int index){
  OwnerEntry *oe=OT->getOwner(index);
  oe->getOneCreditOwner();}

/**********************************************************************/
/*  Exported Utility                       */
/**********************************************************************/

Chain* getChainFromTertiary(Tertiary *t){
  Assert(t->isManager());
  if(t->getType()==Co_Cell){
    return ((CellManager *)t)->getChain();}
  Assert(t->getType()==Co_Lock);
  return ((LockManager *)t)->getChain();}

int getStateFromLockOrCell(Tertiary*t){
  if(t->getType()==Co_Cell){
    if(t->isManager()){
      return ((CellManager*)t)->getSec()->getState();}
    Assert(t->isFrame());
    return ((CellFrame*)t)->getSec()->getState();}
  Assert(t->getType()==Co_Lock);
  if(t->isManager()){
    return ((LockManager*)t)->getSec()->getState();}
  Assert(t->isFrame());
  return ((LockFrame*)t)->getSec()->getState();}

/**********************************************************************/
/*  Utility                       */
/**********************************************************************/

CellSec *getCellSecFromTert(Tertiary *c){
  if(c->isManager()){
    return ((CellManager*)c)->getCellSec();}
  Assert(!c->isProxy());
  return ((CellFrame*)c)->getCellSec();}

/**********************************************************************/
/*  Globalizing                       */
/**********************************************************************/

void globalizeCell(CellLocal* cl, int myIndex){
  PD((CELL,"globalize cell index:%d",myIndex));
  TaggedRef val1=cl->getValue();
  CellManager* cm=(CellManager*) cl;
  CellSec* sec=new CellSec(val1);
  Chain* ch=newChain();
  ch->init(myDSite);
  cm->initOnGlobalize(myIndex,ch,sec);}

void globalizeLock(LockLocal* ll, int myIndex){
  PD((LOCK,"globalize lock index:%d",myIndex));
  Assert(sizeof(LockLocal)==sizeof(LockManager));
  Thread* th=ll->getLocker();
  PendThread* pt=ll->getPending();
  LockManager* lm=(LockManager*) ll;
  LockSec* sec=new LockSec(th,pt);
  Chain* ch=newChain();
  ch->init(myDSite);
  lm->initOnGlobalize(myIndex,ch,sec);}

void CellManager::initOnGlobalize(int index,Chain* ch,CellSec *secX){
  setTertType(Te_Manager);
  setIndex(index);
  setChain(ch);
  sec=secX;
  initForFailure();}

void LockManager::initOnGlobalize(int index,Chain* ch,LockSec *secX){
  setTertType(Te_Manager);
  setIndex(index);
  setChain(ch);
  sec=secX;
  initForFailure();}


void convertCellProxyToFrame(Tertiary *t){
  Assert(t->isProxy());
  CellFrame *cf=(CellFrame*) t;
  cf->convertFromProxy();}

void convertLockProxyToFrame(Tertiary *t){
  Assert(t->isProxy());
  LockFrame *lf=(LockFrame*) t;
  lf->convertFromProxy();}


/**********************************************************************/
/*   basic cell routine */
/**********************************************************************/

OZ_Return CellSec::exchangeVal(TaggedRef old, TaggedRef nw, Thread *th,
                               TaggedRef controlvar, ExKind exKind)
{
  if(!isRealThread(th))
    return PROCEED;

  TaggedRef exception;
  Bool inplace = (th==oz_currentThread());
  switch (exKind){
  case ASSIGN:{
    contents = oz_deref(contents);
    if (!tagged2SRecord(contents)->replaceFeature(old,nw)) {
      exception = OZ_makeException(E_ERROR,E_OBJECT,"<-",3,contents,old,nw);
      goto exception;
    }
    goto exit;
  }
  case AT:{
    contents = oz_deref(contents);
    TaggedRef tr = tagged2SRecord(contents)->getFeature(old);
    if(tr) {
      if (inplace) {
        return oz_unify(tr,nw);
      } else {
        ControlVarUnify(controlvar,tr,nw);
        return PROCEED;
      }
    }
    exception = OZ_makeException(E_ERROR,E_OBJECT,"@",2, contents, old);
    goto exception;
  }
  case EXCHANGE:{
    Assert(old!=0 && nw!=0);
    TaggedRef tr=contents;
    contents = nw;
    if (inplace) {
      return oz_unify(tr,old);
    } else {
      ControlVarUnify(controlvar,tr,old);
      return PROCEED;
    }
  }
  case REMOTEACCESS:{
    cellSendReadAns((DSite*)th,(DSite*)old,(int)nw,contents);
    return PROCEED;
  }
  default:
    Assert(0);
  }

exception:
  if (inplace) {
    return OZ_raise(exception);
  } else {
    ControlVarRaise(controlvar,exception);
    return PROCEED;
  }

exit:
  if (!inplace) {
    ControlVarResume(controlvar);
  }
  return PROCEED;
}

OZ_Return CellSec::exchange(Tertiary* c,TaggedRef old,TaggedRef nw,Thread* th,
                            ExKind exKind){
  OZ_Return ret = PROCEED;
  switch(state){
  case Cell_Lock_Valid:{
    PD((CELL,"CELL: exchange on valid"));
    return exchangeVal(old,nw,th,0,exKind);
  }
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    PD((CELL,"CELL: exchange on requested"));
    ret = pendThreadAddToEnd(&pending,th,old,nw,exKind,c->getBoardInternal());
    if(errorIgnore(c)) return ret;
    break;}
  case Cell_Lock_Invalid:{
    PD((CELL,"CELL: exchange on invalid"));
    state=Cell_Lock_Requested;
    Assert(isRealThread(th) || th==DummyThread);
    ret = pendThreadAddToEnd(&pending,th,old,nw,exKind,c->getBoardInternal());
    int index=c->getIndex();
    if(c->isFrame()){
      BorrowEntry* be=BT->getBorrow(index);
      be->getOneMsgCredit();
      cellLockSendGet(be);
      if(errorIgnore(c)) return ret;
      break;}
    Assert(c->isManager());
    if(!((CellManager*)c)->getChain()->hasFlag(TOKEN_LOST)){
      DSite *toS=((CellManager*)c)->getChain()->setCurrent(myDSite,c);
      sendPrepOwner(index);
      cellLockSendForward(toS,myDSite,index);
      if(errorIgnore(c)) return ret;}
    break;}
  default: Assert(0);
  }
  maybeStateError(c,th);
  return ret;
}


OZ_Return CellSec::access(Tertiary* c,TaggedRef val,TaggedRef fea){
  switch(state){
  case Cell_Lock_Valid:{
    PD((CELL,"CELL: access on valid"));
    Assert(fea == 0);
    return oz_unify(val,contents);
  }
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    PD((CELL,"CELL: access on requested"));
    break;}
  case Cell_Lock_Invalid:{
    PD((CELL,"CELL: access on invalid"));
    break;}
  default: Assert(0);}

  int index=c->getIndex();

  if (pendBinding !=NULL);
    goto exit;
  if(!c->isManager()) {
    Assert(c->isFrame());
    PD((CELL,"Sending to mgr read"));
    BorrowEntry *be=BT->getBorrow(index);
    be->getOneMsgCredit();
    cellSendRead(be,myDSite);
    goto exit;
  }
  PD((CELL,"ShortCircuit mgr sending to tokenholder"));
  if(((CellManager*)c)->getChain()->getCurrent() == myDSite){
    return fea ? cellAtExchange(c,val,fea):
      cellDoExchange(c,val,val);
  }

  sendPrepOwner(index);
  cellSendRemoteRead(((CellManager*)c)->getChain()->getCurrent(),
                     myDSite,index,myDSite);
exit:
  Thread* th=oz_currentThread();
  ControlVarNew(controlvar,c->getBoardInternal());
  pendBinding=new PendThread(th,pendBinding,val,fea,
                             controlvar,fea ? DEEPAT : ACCESS);

  if(!errorIgnore(c)) {
    maybeStateError(c,th);}
  SuspendOnControlVar;
}

OZ_Return cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw,Thread *th,
                         ExKind e)
{
  PD((SPECIAL,"exchange old:%d new:%s type:%d",toC(old),toC(nw),e));
  maybeConvertCellProxyToFrame(c);
  PD((CELL,"CELL: exchange on %s-%d",
      (c->isManager()?myDSite:BT->getOriginSite(c->getIndex()))->stringrep(),
      (c->isManager()?c->getIndex():BT->getOriginIndex(c->getIndex()))));
  return getCellSecFromTert(c)->exchange(c,old,nw,th,e);
}

static
OZ_Return cellDoAccess(Tertiary *c,TaggedRef val,TaggedRef fea){
  if(c->isProxy()){
    convertCellProxyToFrame(c);}
  return getCellSecFromTert(c)->access(c,val,fea);}


/**********************************************************************/
/*   interface */
/**********************************************************************/

OZ_Return cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw){
   return cellDoExchange(c,old,nw,oz_currentThread(),EXCHANGE);}

OZ_Return cellAssignExchange(Tertiary *c,TaggedRef fea,TaggedRef val){
   return cellDoExchange(c,fea,val,oz_currentThread(), ASSIGN);}

OZ_Return cellAtExchange(Tertiary *c,TaggedRef old,TaggedRef nw){
  return cellDoExchange(c,old,nw,oz_currentThread(), AT);}

OZ_Return cellAtAccess(Tertiary *c, TaggedRef fea, TaggedRef val){
  return cellDoAccess(c,val,fea);}

/* PER-HANDLE
OZ_Return cellDoAccess(Tertiary *c, TaggedRef val){
  if(oz_onToplevel() && c->handlerExists(oz_currentThread()))
    return cellDoExchange(c,val,val);
  else
    return cellDoAccess(c,val,0);}
*/

OZ_Return cellDoAccess(Tertiary *c, TaggedRef val){
  if(oz_onToplevel())
    return cellDoExchange(c,val,val);
  else
    return cellDoAccess(c,val,0);}

/**********************************************************************/
/*   Lock - basic routines                             */
/**********************************************************************/

void LockProxy::lock(Thread *t){
  PD((LOCK,"convertToFrame %s-%d",
      BT->getOriginSite(getIndex())->stringrep(),
      BT->getOriginIndex(getIndex())));
  convertLockProxyToFrame(this);
  ((LockFrame*)this)->lock(t);}

/**********************************************************************/
/*   Lock - interface                             */
/**********************************************************************/

void lockLockProxy(Tertiary *t, Thread *thr)
{
  Assert(t->isProxy());
  ((LockProxy *)t)->lock(thr);
}

void lockLockManagerOutline(LockManagerEmul *lmu, Thread *thr)
{
  LockSec *ls = (LockSec *) (lmu->getSec());
  ls->lockComplex(thr, lmu);
}
void unlockLockManagerOutline(LockManagerEmul *lmu, Thread *thr)
{
  LockSec *ls = (LockSec *) (lmu->getSec());
  ls->unlockComplex(lmu);
}

void lockLockFrameOutline(LockFrameEmul *lfu, Thread *thr)
{
  LockSec *ls = (LockSec *) (lfu->getSec());
  ls->lockComplex(thr, lfu);
}
void unlockLockFrameOutline(LockFrameEmul *lfu, Thread *thr)
{
  LockSec *ls = (LockSec *) (lfu->getSec());
  ls->unlockComplex(lfu);
}

/**********************************************************************/
/*   Lock - interface                             */
/**********************************************************************/

void secLockToNext(LockSec* sec,Tertiary* t,DSite* toS){
  int index=t->getIndex();
  if(t->isFrame()){
    BorrowEntry *be=BT->getBorrow(index);
    be->getOneMsgCredit();
    NetAddress *na=be->getNetAddress();
    lockSendToken(na->site,na->index,toS);
    return;}
  Assert(t->isManager());
  OwnerEntry *oe=OT->getOwner(index);
  oe->getOneCreditOwner();
  lockSendToken(myDSite,index,toS);}

void secLockGet(LockSec* sec,Tertiary* t,Thread* th){
  int index=t->getIndex();
  sec->makeRequested();
  if(t->isFrame()){
    BorrowEntry *be=BT->getBorrow(index);
    be->getOneMsgCredit();
    cellLockSendGet(be);
    return;}
  Assert(t->isManager());
  OwnerEntry *oe=OT->getOwner(index);
  Chain* ch=((LockManager*) t)->getChain();
  DSite* current=ch->setCurrent(myDSite,t);
  oe->getOneCreditOwner();
  cellLockSendForward(current,myDSite,index);
  return;}

void LockSec::lockComplex(Thread *th,Tertiary* t){
  PD((LOCK,"lockComplex in state:%d",state));
  Board *home = t->getBoardInternal();
  switch(state){
  case Cell_Lock_Valid|Cell_Lock_Next:{
    Assert(getLocker()!=th);
    Assert(getLocker()!=NULL);
    if(pending==NULL){
      (void) pendThreadAddToEnd(getPendBase(),MoveThread,home);}}
  case Cell_Lock_Valid:{
    Assert(getLocker()!=th);
    Assert(getLocker()!=NULL);
    (void) pendThreadAddToEnd(getPendBase(),th,home);
    if(errorIgnore(t)) return;
    break;}
  case Cell_Lock_Next|Cell_Lock_Requested:
  case Cell_Lock_Requested:{
    (void) pendThreadAddToEnd(getPendBase(),th,home);
    if(errorIgnore(t)) return;
    break;}
  case Cell_Lock_Invalid:{
    (void) pendThreadAddToEnd(getPendBase(),th,home);
    secLockGet(this,t,th);
    if(errorIgnore(t)) return;
    break;}
  default: Assert(0);}
  maybeStateError(t,th);}

void LockSec::unlockPending(Thread *t){
  PendThread **pt=&pending;
  while((*pt)->thread!=t) {
    pt=&((*pt)->next);}
  *pt=(*pt)->next;}

void LockSec::unlockComplex(Tertiary* tert){
  PD((LOCK,"unlock complex in state:%d",getState()));
  Assert(getState() & Cell_Lock_Valid);
  if(getState() & Cell_Lock_Next){
    Assert(getState()==(Cell_Lock_Next | Cell_Lock_Valid));
    if(pending==NULL){
      secLockToNext(this,tert,next);
      state=Cell_Lock_Invalid;
      return;}
    Thread *th=pending->thread;
    if(th==DummyThread){
      Assert(tert->isManager());
      pendThreadRemoveFirst(getPendBase());
      unlockComplex(tert);
      return;}
    if(th==MoveThread){
      pendThreadRemoveFirst(getPendBase());
      secLockToNext(this,tert,next);
      state=Cell_Lock_Invalid;
      if(pending==NULL) return;
      secLockGet(this,tert,NULL);
      return;}
    locker=pendThreadResumeFirst(getPendBase());
    return;}
  if(pending!=NULL){
    locker=pendThreadResumeFirst(getPendBase());
    return;}
  return;}

/**********************************************************************/
/*   gc                             */
/**********************************************************************/

void gcDistCellRecurse(Tertiary *t)
{
  gcEntityInfo(t);
  switch (t->getTertType()) {
  case Te_Proxy:
    gcProxy(t);
    break;
  case Te_Frame: {
    CellFrame *cf=(CellFrame*)t;
    CellSec *cs=cf->getCellSec();
    cf->setCellSec((CellSec*)gcRealloc(cs,sizeof(CellSec)));
    cf->gcCellFrame();
    break; }
  case Te_Manager:{
    CellManager *cm=(CellManager*)t;
    CellFrame *cf=(CellFrame*)t;
    CellSec *cs=cf->getCellSec();
    cf->setCellSec((CellSec*)gcRealloc(cs,sizeof(CellSec)));
    cm->gcCellManager();
    break;}
  default: {
    Assert(0); }
  }
}

void gcDistLockRecurse(Tertiary *t)
{
  gcEntityInfo(t);
  switch(t->getTertType()){
  case Te_Manager:{
    LockManager* lm=(LockManager*)t;
    LockFrame* lf=(LockFrame*)t;
    LockSec* ls= lf->getLockSec();
    lf->setLockSec((LockSec*)gcRealloc(ls,sizeof(LockSec)));
    lm->gcLockManager();
    break;}

  case Te_Frame:{
    LockFrame *lf=(LockFrame*)t;
    LockSec *ls=lf->getLockSec();
    lf->setLockSec((LockSec*)gcRealloc(ls,sizeof(LockSec)));
    lf->gcLockFrame();
    break;}

  case Te_Proxy:{
    gcProxy(t);
    break;}

  default:{
    Assert(0);}
  }
}

ConstTerm* auxGcDistCell(Tertiary *t)
{
  CellFrame *cf=(CellFrame *)t;
  if (cf->isAccessBit()) {
    // has only been reached via gcBorrowRoot so far
    void* forward=cf->getForward();
    ((CellFrame*)forward)->resetAccessBit();
    cf->gcMark((ConstTerm *) forward);
    return (ConstTerm*) forward;
  } else {
    return (NULL);
  }
}

ConstTerm* auxGcDistLock(Tertiary *t)
{
  LockFrame *lf=(LockFrame *)t;
  if(lf->isAccessBit()){
    DebugCode(lf->resetAccessBit());
    void* forward=lf->getForward();
    ((LockFrame*)forward)->resetAccessBit();
    lf->gcMark((ConstTerm *) forward);
    return (ConstTerm*) forward;
  } else {
    return (NULL);
  }
}

void CellSec::gcCellSec(){
  gcPendThread(&pendBinding);
  switch(stateWithoutAccessBit()){
  case Cell_Lock_Next|Cell_Lock_Requested:{
    next->makeGCMarkSite();}
  case Cell_Lock_Requested:{
    gcPendThread(&pending);
    return;}
  case Cell_Lock_Next:{
    next->makeGCMarkSite();}
  case Cell_Lock_Invalid:{
    return;}
  case Cell_Lock_Valid:{
    OZ_collectHeapTerm(contents,contents);
    return;}
  default: Assert(0);}}

void CellFrame::gcCellFrame(){
  Tertiary *t=(Tertiary*)this;
  gcProxy(t);
  PD((GC,"relocate cellFrame:%d",t->getIndex()));
  getCellSec()->gcCellSec();}

void CellManager::gcCellManager(){
  getChain()->gcChainSites();
  int i=getIndex();
  PD((GC,"relocate cellManager:%d",i));
  OwnerEntry* oe=OT->getOwner(i);
  oe->gcPO(this);
  CellFrame *cf=(CellFrame*)this;
  getCellSec()->gcCellSec();}

void LockSec::gcLockSec(){
  if(state & Cell_Lock_Next){
    getNext()->makeGCMarkSite();}
  PD((GC,"relocate Lock in state %d",state));
  if(state & Cell_Lock_Valid)
    locker=locker->gcThread();
  if(pending!=NULL)
    gcPendThread(&pending);
  return;}

void LockFrame::gcLockFrame(){
  Tertiary *t=(Tertiary*)this;
  gcProxy(t);
  PD((GC,"relocate lockFrame:%d",t->getIndex()));
  getLockSec()->gcLockSec();}

void LockManager::gcLockManager(){
  getChain()->gcChainSites();
  int i=getIndex();
  PD((GC,"relocate lockManager:%d",i));
  OwnerEntry* oe=OT->getOwner(i);
  oe->gcPO(this);
  getLockSec()->gcLockSec();}


/* ******************************************************************* */
/*  object                            */
/* ******************************************************************* */

Tertiary* getOtherTertFromObj(Tertiary* o, Tertiary* lockORcell){
  Assert(o->getType()==Co_Object);
  Assert(!(o->isProxy()));
  Object *object = (Object *) o;
  if(object->getLock()==NULL && object->getLock()==lockORcell)
    return getCell(object->getState());
  return object->getLock();
}
/**********************************************************************/
/*   failure                             */
/**********************************************************************/

void LockManager::initForFailure(){return;}
void CellManager::initForFailure(){return;}

void cellLock_Perm(int state,Tertiary* t){return;}
void cellLock_Temp(int state,Tertiary* t){return;}
void cellLock_OK(int state,Tertiary* t){return;}

/*
PER-HANDLE

void LockManager::initForFailure(){
  Watcher *w = getWatchersIfExist(this);
  while(w!=NULL){
    if(managerPart(w->getWatchCond()) != ENTITY_NORMAL){
      getChain()->newInform(myDSite,w->getWatchCond());}
    w = w->getNext();}}

void CellManager::initForFailure(){
  Watcher *w = getWatchersIfExist(this);
  while(w!=NULL){
    if(managerPart(w->getWatchCond()) != ENTITY_NORMAL){
      getChain()->newInform(myDSite,w->getWatchCond());}
    w = w->getNext();}}

void cellLock_Perm(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    // ATTENTION PER I added Perm_block /EK
    if(setEntityCondOwn(t,PERM_SOME|PERM_ME|PERM_BLOCKED)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(setEntityCondOwn(t,PERM_SOME|PERM_BLOCKED|PERM_ME)) break;
               // ATTENTION note that we don't know if we'll be blocked maybe TEMP?
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(setEntityCondOwn(t,PERM_ALL|PERM_SOME)) break;
    return;}
  default: {
    Assert(0);}}
  entityProblem(t);}

void cellLock_Temp(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    if(setEntityCondOwn(t,TEMP_SOME|TEMP_ME)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(setEntityCondOwn(t,TEMP_SOME|TEMP_ME|TEMP_BLOCKED)) break;
                   // ATTENTION: note that we don't know if we'll be blocked
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(setEntityCondOwn(t,TEMP_SOME|TEMP_ALL)) break;
    return;}
  default: {
    Assert(0);}}
  entityProblem(t);}

void cellLock_OK(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    if(resetEntityCondProxy(t,TEMP_SOME|TEMP_ME)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(resetEntityCondProxy(t,TEMP_SOME|TEMP_ME|TEMP_BLOCKED)) break;
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(resetEntityCondProxy(t,TEMP_SOME|TEMP_ALL)) break;
    return;}
  default: {
    Assert(0);}}}

*/
