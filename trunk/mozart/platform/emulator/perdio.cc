/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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

//  protocol and message layer

/* ***************************************************************************
   ***************************************************************************
                       ORGANIZATION

	    1  forward declarations
	    2  global variables
	    3  utility routines
	    4  class PendThread
	    5  class ProtocolObject
            6  class GNameTable & gname routines
            7  class OB_Entry Owner/Borrower common
	    8  class NetAddress & class NetHashTable
	    9  class OwnerCreditExtension
	    10  class OwnerEntry
	    11 class OwnerTable
	    12 class BorrowCreditExtension
	    13 creditExtension methods
	    14 class BorrowEntry
	    15 class BorrowTable
	    16 div small routines
	    17 Pending Thread control utility routines
	    18 garbage collection
	    19 globalizing
	    20 localizing
	    21 marshaling/unmarshaling by protocol-layer
	    22 main receive msg
	    23 remote send protocol
	    24 port protocol
	    25 variable protocol
	    26 object protocol
	    27 credit protocol
	    28 cell protocol - receive
	    29 cell protocol - send
	    30 cell protocol - basics
	    31 chain routines
	    32 chain protocol
	    33 lock protocol - receive
	    34 lock protocol - send
	    35 lock protocol - basics
	    36 error msgs
	    37 handlers/watchers
	    38 error
	    39 probes
	    40 commincation problem
	    41 builtins
	    42 initialization
	    43 misc

   **************************************************************************
   **************************************************************************/

#include "wsock.hh"

#include "threadInterface.hh"
#include "codearea.hh"
#include "indexing.hh"

#include "dp_table.hh"
#include "dp_gname.hh"
#include "dp_pendThread.hh"
#include "dp_msgType.hh"
#include "perdio_debug.hh"  

#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "urlc.hh"
#include "marshaler.hh"
#include "comm.hh"
#include "msgbuffer.hh"
#include "vs_comm.hh"
#include "chain.hh"
#include "builtins.hh"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>

// mm2: hack alert
inline
TaggedRef ProtocolObject::getValue()
{
  if (isTertiary()) {return makeTaggedConst(getTertiary());}
  else {return getRef();}
}

// mm2: hack alert
inline
PerdioVar *ProtocolObject::getPerdioVar()
{
  return tagged2PerdioVar(*getPtr());
}

/* *********************************************************************/
/*   SECTION 1: forward declarations                                   */
/* *********************************************************************/

#define NOT_IMPLEMENTED						\
  {								\
    warning("in file %s at line %d: not implemented - perdio",	\
	    __FILE__,__LINE__);					\
    Assert(0);							\
  }

class MsgBuffer;

void marshalSite(Site *,MsgBuffer*);
OZ_Return raiseGeneric(char *msg, OZ_Term arg);
OZ_Return sendRedirect(Site* sd,int OTI,TaggedRef val);
OZ_Return sendRedirect(PerdioVar *pv,OZ_Term val, Site* ackSite, int OTI);

void sendCreditBack(Site* sd,int OTI,Credit c);

void cellLockSendForward(Site *toS,Site *rS,int mI);
void cellLockSendGet(BorrowEntry*);
void cellLockSendDump(BorrowEntry*);

void cellLockReceiveForward(BorrowEntry*,Site*,Site*,int);
void cellLockReceiveDump(OwnerEntry*,Site *);
void cellLockReceiveGet(OwnerEntry*,Site *);

void cellReceiveGet(OwnerEntry* oe,CellManager*,Site*);
void cellReceiveDump(CellManager*,Site*);
void cellReceiveForward(BorrowEntry*,Site*,Site*,int);
void cellReceiveContentsManager(OwnerEntry*,TaggedRef,int);
void cellReceiveContentsFrame(BorrowEntry*,TaggedRef,Site*,int);
void cellReceiveRemoteRead(BorrowEntry*,Site*,int,Site*);
void cellReceiveRead(OwnerEntry*,Site*);
void cellReceiveReadAns(Tertiary*,TaggedRef);
void cellReceiveCantPut(OwnerEntry*,TaggedRef,int,Site*,Site*);
void cellSendReadAns(Site*,Site*,int,TaggedRef);
void cellSendRemoteRead(Site* toS,Site* mS,int mI,Site* fS);
void cellSendContents(TaggedRef tr,Site* toS,Site *mS,int mI);
void cellSendRead(BorrowEntry *be,Site *dS);
static OZ_Return cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw,
				Thread *th,ExKind e);

void lockReceiveGet(OwnerEntry* oe,LockManager*,Site*);
void lockReceiveDump(LockManager*,Site*);
void lockReceiveTokenManager(OwnerEntry*,int);
void lockReceiveTokenFrame(BorrowEntry*,Site*,int);
void lockReceiveForward(BorrowEntry*,Site*,Site*,int);
void lockReceiveCantPut(OwnerEntry*,int,Site*,Site*);
void lockSendToken(Site*,int,Site*);

void cellSendContentsFailure(TaggedRef,Site*,Site*,int);
void lockReceiveCantPut(LockManager *cm,int mI,Site* rsite, Site* dS);

void receiveAskError(OwnerEntry*,Site*,EntityCond);
void sendAskError(Tertiary*, EntityCond);
void receiveTellError(Tertiary*, Site*, int, EntityCond, Bool);

void chainReceiveAck(OwnerEntry*, Site*);
void chainReceiveAnswer(OwnerEntry*,Site*,int,Site*);
void chainReceiveQuestion(BorrowEntry*,Site*,int,Site*);
void chainSendAnswer(BorrowEntry*,Site*,int,int,Site*);
void chainSendQuestion(Site*,int,Site*);
void chainSendAck(Site*,int);
void receiveAskError(OwnerEntry *,Site*,EntityCond);
void receiveUnAskError(OwnerEntry *,Site*,EntityCond);
void sendTellError(OwnerEntry *,Site*,int,EntityCond,Bool);
void lockSendForward(Site *toS,Site *fS,int mI);
void lockSendTokenFailure(Site*,Site*,int);
void lockSendDump(BorrowEntry*,LockFrame*);
void sendUnAskError(Tertiary*,EntityCond);

int printChain(Chain*);
void insertDangelingEvent(Tertiary*);
EntityCond getEntityCondPort(Tertiary*);

void sendObject(Site* sd, Object *o, Bool);

TaggedRef listifyWatcherCond(EntityCond);
PERDIO_DEBUG_DO(void printTables());

/* *********************************************************************/
/*   SECTION 2: global variables                                       */
/* *********************************************************************/

// global variables
MsgBufferManager* msgBufferManager= new MsgBufferManager();
Site* mySite;  // known to network-layer also 
Site* creditSite;
OZ_Term GateStream;

int PortSendTreash = 100000;
int PortWaitTimeSlice = 800;
int PortWaitTimeK = 1;

/* *********************************************************************/
/*   SECTION 3:: Utility routines                                      */
/* *********************************************************************/

void SendTo(Site *toS,MsgBuffer *bs,MessageType mt,Site *sS,int sI)
{
  OZ_Term nogoods = bs->getNoGoods();
  if (!literalEq(oz_nil(),nogoods)) {
    warning("send message '%s' contains nogoods: %s",
	    mess_names[mt],toC(nogoods));
  }

  int ret=toS->sendTo(bs,mt,sS,sI);
  if(ret==ACCEPTED) return;
  if(ret==PERM_NOT_SENT)
    toS->communicationProblem(mt,sS,sI,COMM_FAULT_PERM_NOT_SENT,
			      (FaultInfo) bs);
  else
    toS->communicationProblem(mt,sS,sI,COMM_FAULT_TEMP_NOT_SENT,ret);
}

inline Bool SEND_SHORT(Site* s){
  if(s->siteStatus()==PERM_SITE) {return OK;}
  return NO;}

void pushUnify(Thread *t, TaggedRef t1, TaggedRef t2)
{
  t->pushCall(BI_Unify,t1,t2);
}

// mm2: should be OZ_unifyInThread???
void SiteUnify(TaggedRef val1,TaggedRef val2)
{
  TaggedRef aux1 = val1; DEREF(aux1,_1,_2);
  TaggedRef aux2 = val2; DEREF(aux2,_3,_4);
  
    if (isUVar(aux1) || isUVar(aux2)) {
      // cannot fail --> do it in current thread
    OZ_unify(val1,val2); // mm2: should be bind?
    return;
    }
  
  Assert(oz_onToplevel());
  Thread *th=oz_newThread(DEFAULT_PRIORITY);
#ifdef PERDIO_DEBUG
  PD((SITE_OP,"SITE_OP: site unify called %d %d",val1, val2));

  Assert(MemChunks::isInHeap(val1) && MemChunks::isInHeap(val1));
#endif
  pushUnify(th,val1,val2);
}

void SiteUnifyCannotFail(TaggedRef val1,TaggedRef val2){
  SiteUnify(val1,val2); // ATTENTION
}

Chain * tertiaryGetChain(Tertiary*t){
  if(t->getType()==Co_Cell){
    return ((CellManager*)t)->getChain();}
  Assert(t->getType()==Co_Lock);
  return ((LockManager*)t)->getChain();}

inline CellSec* getCellSecFromFrameOrManager(Tertiary *t){
  if(t->isFrame()){
    return ((CellFrame*)t)->getSec();}
  return ((CellManager*)t)->getSec();}

/* ********************************************************************** */

OZ_BI_define(BIsetNetBufferSize,1,0)
{
  OZ_Term s = OZ_in(0);
  DEREF(s,_1,tagS);
  int size = 0;
  if (isSmallIntTag(tagS))
    size = smallIntValue(s);
  if(size < 0)
    oz_raise(E_ERROR,E_KERNEL,
	     "NetBufferSize must be of type int and larger than 0",0);
  PortSendTreash = size * 20000;
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIgetNetBufferSize,0,1)
{
  OZ_RETURN(oz_int(PortSendTreash / 20000));
} OZ_BI_end

/* ******************************************************************* */
/*   SECTION 16a :: div small routines                                 */
/* ******************************************************************* */

Site* getNASiteFromTertiary(Tertiary* t){
  if(t->isManager()){
    return mySite;}
  Assert(!(t->isProxy()));
  return BT->getOriginSite(t->getIndex());}

int getNAIndexFromTertiary(Tertiary* t){
  if(t->isManager()){
    return t->getIndex();}
  Assert(!(t->isProxy()));
  return BT->getOriginIndex(t->getIndex());}

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

/* ******************************************************************* */
/*   SECTION 16b :: div small routines                                  */
/* ******************************************************************* */

inline Bool someTempCondition(EntityCond ec){
  return ec & (TEMP_SOME|TEMP_BLOCKED|TEMP_ME|TEMP_ALL);}

inline Bool somePermCondition(EntityCond ec){
  return ec & (PERM_SOME|PERM_BLOCKED|PERM_ME|PERM_ALL);}

inline EntityCond managerPart(EntityCond ec){
  return ec & (PERM_SOME|PERM_BLOCKED|PERM_ME|TEMP_SOME|TEMP_BLOCKED|TEMP_ME);}

ChainElem *newChainElem(){
  return (ChainElem*) genFreeListManager->getOne_3();}  

void freeChainElem(ChainElem* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}

Chain *newChain(){
  return (Chain*) genFreeListManager->getOne_4();}

void freeChain(Chain* e){
  genFreeListManager->putOne_4((FreeListEntry*) e);}

InformElem* newInformElem(){
  return (InformElem*) genFreeListManager->getOne_3();}  

void freeInformElem(InformElem* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}


inline void installProbe(Site *s,ProbeType pt){
  if(s==mySite) return;
  s->installProbe(pt,PROBE_INTERVAL);}

void tertiaryInstallProbe(Site *s,ProbeType pt,Tertiary *t){
  if(s==mySite) return;
  ProbeReturn pr=s->installProbe(pt,PROBE_INTERVAL);
  if(pr==PROBE_INSTALLED) return;
  if(t->isManager())
    t->managerProbeFault(s,pr);
  else
    t->proxyProbeFault(pr);}

inline void deinstallProbe(Site *s,ProbeType pt){
  if(s==mySite) return;
  s->deinstallProbe(pt);}

Chain* getChainFromTertiary(Tertiary *t){
  Assert(t->isManager());
  if(t->getType()==Co_Cell){
    return ((CellManager *)t)->getChain();}
  Assert(t->getType()==Co_Lock);
  return ((LockManager *)t)->getChain();}

/* ******************************************************************* */
/*   SECTION 16c :: div small routines                                 */
/* ******************************************************************* */

void Chain::newInform(Site* toS,EntityCond ec){
  InformElem *ie=newInformElem();
  ie->init(toS,ec);
  ie->next=inform;
  inform=ie;}

void Chain::releaseChainElem(ChainElem *ce){
  PD((CHAIN,"Releaseing Element"));
  if((!ce->flagIsSet(CHAIN_GHOST))){
    if(hasFlag(INTERESTED_IN_TEMP)){
      deinstallProbe(ce->getSite(),PROBE_TYPE_ALL);}
    else{
      deinstallProbe(ce->getSite(),PROBE_TYPE_PERM);}}
  freeChainElem(ce);}


void Chain::removePerm(ChainElem** base){
  ChainElem *ce=*base;
  *base=ce->next;
  freeChainElem(ce);}

void Chain::removeNextChainElem(ChainElem** base){
  ChainElem *ce=*base;
  *base=ce->next;
  releaseChainElem(ce);}

void Chain::releaseInformElem(InformElem *ie){
  EntityCond ec=ie->watchcond;
  if(someTempCondition(ie->watchcond)){
    InformElem *tmp=inform;
    int interestedInTemp=NO;
    while(tmp!=NULL){
      if(someTempCondition(tmp->watchcond)) {
	interestedInTemp=OK;
	break;}
      tmp=tmp->next;}
    if(!interestedInTemp){
      deProbeTemp();}}
  freeInformElem(ie);}

void Chain::init(Site *s){
  ChainElem *e=newChainElem();
  e->init(s);
  inform = NULL;
  first=last=e;}

Site* Chain::setCurrent(Site* s, Tertiary* t){
  ChainElem *e=newChainElem();
  e->init(s);
  Site *toS=last->site;
  last->next=e;
  last= e;
  if(s==mySite){
    return toS;}
  ChainElem *de = getFirstNonGhost();
  if(de->site==s){
    de->setFlagAndCheck(CHAIN_DUPLICATE);}
  if(hasFlag(INTERESTED_IN_TEMP)){
    tertiaryInstallProbe(s,PROBE_TYPE_ALL,t);}
  else{
    tertiaryInstallProbe(s,PROBE_TYPE_PERM,t);}
  return toS;}

inline Bool tokenLostCheckProxy(Tertiary*t){ 
  if(t->getEntityCond() & PERM_ME){
    PD((WEIRD,"lost token found BUT cannot recover"));
    return OK;}
  return NO;}

inline Bool tokenLostCheckManager(Tertiary *t){
  if(getChainFromTertiary(t)->hasFlag(TOKEN_LOST)) {
    PD((WEIRD,"lost token found BUT cannot recover"));
    return OK;}
  return NO;}


/* ******************************************************************* */
/*   SECTION 16d :: div small routines                                 */
/* ******************************************************************* */

Tertiary* getOtherTertFromObj(Tertiary* o, Tertiary* lockORcell){
  Assert(o->getType()==Co_Object);
  Object *object = (Object *) o;
  if(object->getLock()==NULL && object->getLock()==lockORcell)
    return getCell(object->getState());
  return object->getLock();
}

void sendHelpX(MessageType mt,BorrowEntry *be)
{
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  switch (mt) {
  case M_GET_OBJECT:
    marshal_M_GET_OBJECT(bs,na->index,mySite);
    break;
  case M_GET_OBJECTANDCLASS:
    marshal_M_GET_OBJECTANDCLASS(bs,na->index,mySite);
    break;
  default:
    Assert(0);
  }
  SendTo(na->site,bs,mt,na->site,na->index);
}

/* ******************************************************************* */
/*   SECTION 18::  garbage-collection                                  */
/* ******************************************************************* */

/* OBS: ---------- interface to gc.cc ----------*/

void gcBorrowTableUnusedFrames() { borrowTable->gcBorrowTableUnusedFrames();}
void gcGName(GName* name) { if (name) name->gcGName(); }
void gcFrameToProxy()     { borrowTable->gcFrameToProxy(); }

void Tertiary::gcProxy(){
  int i=getIndex();
  BorrowEntry *be=BT->getBorrow(i);
  if(be->isGCMarked()){
    PD((GC,"borrow already marked:%d",i));
    return;}
  be->makeGCMark();
  PD((GC,"relocate borrow :%d old:%x new:%x",i,be,this));
  if (be->isTertiary())  /* might be avariable for an object */
    be->mkTertiary(this,be->getFlags());
  return;}

void Tertiary::gcManager(){
  Assert(!isFrame());
  int i=getIndex();
  OwnerEntry *oe=OT->getOwner(i);
  if(oe->isGCMarked()){
    PD((GC,"owner already marked:%d",i));
    return;
  }
  PD((GC,"relocate owner:%d old%x new %x",i,oe,this));
  oe->gcPO(this);}

void gcPendThread(PendThread **pt){
  PendThread *tmp;
  while(*pt!=NULL){
    if(((*pt)->thread == MoveThread) || ((*pt)->thread==DummyThread)){
      tmp=new PendThread((*pt)->thread,(*pt)->next);
    } else {
      if((*pt)->exKind==REMOTEACCESS) {
	tmp=new PendThread((*pt)->thread,(*pt)->next);	
	tmp->exKind = (*pt)->exKind;
	tmp->nw = (*pt)->nw; 
	tmp->old = (*pt)->old; 
	((Site *)(*pt)->thread)->makeGCMarkSite();
	((Site *)(*pt)->old)->makeGCMarkSite();
	OZ_collectHeapTerm((*pt)->controlvar,tmp->controlvar);
      } else {
	tmp=new PendThread((*pt)->thread->gcThread(),(*pt)->next);
	tmp->exKind = (*pt)->exKind;
	OZ_collectHeapTerm((*pt)->old,tmp->old);
	OZ_collectHeapTerm((*pt)->nw,tmp->nw);
	OZ_collectHeapTerm((*pt)->controlvar,tmp->controlvar);
      }
    }
    *pt=tmp;
    pt=&(tmp->next);}}
  
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
  t->gcProxy();
  PD((GC,"relocate cellFrame:%d",t->getIndex()));
  sec->gcCellSec();}

void Chain::gcChainSites(){
  ChainElem *ce=first;
  while(ce!=NULL){
    ce->site->makeGCMarkSite();
    ce=ce->next;}
  InformElem *ie=inform;
  while(ie!=NULL){
    ie->site->makeGCMarkSite();
    ie=ie->next;}}
  
void CellManager::gcCellManager(){
  getChain()->gcChainSites(); 
  int i=getIndex();
  PD((GC,"relocate cellManager:%d",i));
  OwnerEntry* oe=OT->getOwner(i);
  oe->gcPO(this);
  CellFrame *cf=(CellFrame*)this;
  sec->gcCellSec();}

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
  t->gcProxy();
  PD((GC,"relocate lockFrame:%d",t->getIndex()));
  sec->gcLockSec();}

void LockManager::gcLockManager(){
  getChain()->gcChainSites(); 
  int i=getIndex();
  PD((GC,"relocate lockManager:%d",i));
  OwnerEntry* oe=OT->getOwner(i);
  oe->gcPO(this);
  sec->gcLockSec();}

/*--------------------*/

void gcPerdioRoots()
{
  OT->gcOwnerTableRoots();
  BT->gcBorrowTableRoots();
}

void gcPerdioFinal()
{
  BT->gcBorrowTableFinal();
  OT->gcOwnerTableFinal();
  GT.gcGNameTable();
  gcSiteTable();
}

void GName::gcMarkSite(){
  site->makeGCMarkSite();}


void OwnerTable::gcOwnerTableRoots()
{
  PD((GC,"owner gc"));
  for(int i=0;i<size;i++) {
    OwnerEntry* o = getOwner(i);
    if(!o->isFree() && !o->hasFullCredit()) {
      PD((GC,"OT o:%d",i));
      o->gcPO();
    }
  }
  return;
}

void OwnerTable::gcOwnerTableFinal()
{
  PD((GC,"owner gc"));
  for(int i=0;i<size;i++) {
    OwnerEntry* o = getOwner(i);
    if(!o->isFree()) {
      PD((GC,"OT o:%d",i));
      if(o->hasFullCredit() && !o->isGCMarked()) {
	 freeOwnerEntry(i);
      } else {
	o->gcPO();
	o->removeGCMark();
      }
    }
  }
  compactify();
  return;
}

void BorrowTable::gcBorrowTableRoots()
{
  PD((GC,"borrowTable1 gc"));
  for(int i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if (!b->isFree() && !b->isGCMarked())
      b->gcBorrowRoot(i);
  }
}

void BorrowEntry::gcBorrowUnusedFrame(int i) {
  if(isTertiary() && getTertiary()->isFrame())
    {u.tert= (Tertiary*) u.tert->gcConstTermSpec();}}

void BorrowTable::gcBorrowTableUnusedFrames()
{
  PD((GC,"borrow gc roots"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if(!b->isFree()){
      Assert((b->isVar()) || (b->getTertiary()->isFrame()) 
	     || (b->getTertiary()->isProxy()));
      if(!(b->isGCMarked())) {b->gcBorrowUnusedFrame(i);}}}
}

void BorrowTable::gcFrameToProxy(){
  PD((GC,"borrow frame to proxy"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if((!b->isFree()) && (!b->isVar())){
      Tertiary *t=b->getTertiary();
      if(t->isFrame()) {
	if((t->getType()==Co_Cell)
	   && ((CellFrame*)t)->getState()==Cell_Lock_Invalid){
	  ((CellFrame*)t)->convertToProxy();}
	else{
	  if((t->getType()==Co_Lock)
	     && ((LockFrame*)t)->getState()==Cell_Lock_Invalid){
	    ((LockFrame*)t)->convertToProxy();}}}}}
}

void maybeUnask(BorrowEntry *be){
  Tertiary *t=be->getTertiary();
  Watcher *w=t->getWatchersIfExist();
  EntityCond ec;
  while(w!=NULL){
    ec=managerPart(w->getWatchCond());
    if(ec!=ENTITY_NORMAL){
      sendUnAskError(t,ec);}
    w=w->getNext();}}


/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTableFinal()
{
  PD((GC,"borrow gc"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);

    if (b->isFree())
      continue;

    if(b->isVar()) {
      if(b->isGCMarked()) {
	b->removeGCMark();
	b->getSite()->makeGCMarkSite();
	PD((GC,"BT b:%d mark variable found",i));
      } else{
	PD((GC,"BT b:%d unmarked variable found",i));
	borrowTable->maybeFreeBorrowEntry(i);
      }
    } else {
      Tertiary *t = b->getTertiary();
      if(b->isGCMarked()) {
	b->removeGCMark();
	b->getSite()->makeGCMarkSite();
	PD((GC,"BT b:%d mark tertiary found",i));
	
	if(t->isFrame()) {
	  switch(t->getType()){
	  case Co_Cell:{
	    CellFrame *cf=(CellFrame *)t;
	    if(cf->isAccessBit()){
	      cf->resetAccessBit();
	      if(cf->dumpCandidate()) {
		cellLockSendDump(b);
	      }
	    }
	    break;
	  }
	  case Co_Lock:{
	    LockFrame *lf=(LockFrame *)t;
	    if(lf->isAccessBit()){
	      lf->resetAccessBit();
	      if(lf->dumpCandidate()) {
		cellLockSendDump(b);
	      }
	    }
	    break;
	  }
	  default:
	    Assert(0);
	    break;
	  }
	}
      } else{
	if(t->maybeHasInform() && t->getType()!=Co_Port)
	  maybeUnask(b);
	Assert(t->isProxy());
	borrowTable->maybeFreeBorrowEntry(i);
      }
    }
  }
  compactify();
  hshtbl->compactify();
}

/* OBSERVE - this must be done at the end of other gc */
void GNameTable::gcGNameTable()
{
  PD((GC,"gname gc"));
  int index;
  GenHashNode *aux = getFirst(index);
  DebugCode(int used = getUsed());
  while (aux!=NULL) {
    GName *gn = (GName*) aux->getBaseKey();

    DebugCode(used--);

    /* code is never garbage collected */
    if (gn->getGNameType()==GNT_CODE){
      gn->site->makeGCMarkSite();
      goto next_one;}

    if (gn->getGCMark()) {
      gn->resetGCMark();
      gn->site->makeGCMarkSite();
    } else {
      if (gn->getGNameType()==GNT_NAME &&
	  tagged2Literal(gn->getValue())->isNamedName()) {
	goto next_one;
      }
      delete gn;
      if (!htSub(index,aux)) 
	continue;}
  next_one:
    aux = getNext(aux,index);
  }

  Assert(used==0);
  compactify();
}

/**********************************************************************/
/*   SECTION 19 :: Globalizing                                        */
/**********************************************************************/

GName *Name::globalize()
{
  if (!hasGName()) {
    Assert(oz_isRootBoard(GETBOARD(this)));
    homeOrGName = ToInt32(newGName(makeTaggedLiteral(this),GNT_NAME));
    setFlag(Lit_hasGName);
  }
  return getGName();
}

GName *Abstraction::globalize(){
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_PROC));}
  return getGName();
}

GName *SChunk::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CHUNK));}
  return getGName();
}

void ObjectClass::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CLASS));}
}

void Object::globalize(){
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_OBJECT));}
}

void CellLocal::globalize(int myIndex){
  PD((CELL,"globalize cell index:%d",myIndex));
  TaggedRef val1=val;
  CellManager* cm=(CellManager*) this;
  CellSec* sec=new CellSec(val1);
  Chain* ch=newChain();
  ch->init(mySite);
  cm->initOnGlobalize(myIndex,ch,sec);}


void LockLocal::globalize(int myIndex){
  PD((LOCK,"globalize lock index:%d",myIndex));
  Thread* th=getLocker();
  PendThread* pt=getPending();
  LockManager* lm=(LockManager*) this;
  LockSec* sec=new LockSec(th,pt);
  Chain* ch=newChain();
  ch->init(mySite);
  lm->initOnGlobalize(myIndex,ch,sec);}

void CellManager::initOnGlobalize(int index,Chain* ch,CellSec *secX){
  Watcher *w = getWatchersIfExist();
  setTertType(Te_Manager);
  setIndex(index);
  setChain(ch);
  sec=secX;
  while(w!=NULL){
    if(managerPart(w->getWatchCond()) != ENTITY_NORMAL){
      getChain()->newInform(mySite,w->getWatchCond());}
    w = w->getNext();}}

void LockManager::initOnGlobalize(int index,Chain* ch,LockSec *secX){
  Watcher *w = getWatchersIfExist();
  setTertType(Te_Manager);
  setIndex(index);
  setChain(ch);
  sec=secX;
  while(w!=NULL){
    if(managerPart(w->getWatchCond()) != ENTITY_NORMAL){
      getChain()->newInform(mySite,w->getWatchCond());}
    w = w->getNext();}}

void Tertiary::globalizeTert()
{ 
  Assert(isLocal());

  switch(getType()) {
  case Co_Cell:
    {
      OwnerEntry *oe_manager;
      int manI=ownerTable->newOwner(oe_manager);
      PD((GLOBALIZING,"GLOBALIZING cell index:%d",manI));
      oe_manager->mkTertiary(this);
      CellLocal *cl=(CellLocal*)this;
      cl->globalize(manI);
      return;
    }
  case Co_Lock:
    {
      OwnerEntry *oe_manager;
      int manI=ownerTable->newOwner(oe_manager);
      PD((GLOBALIZING,"GLOBALIZING lock index:%d",manI));
      oe_manager->mkTertiary(this);
      LockLocal *ll=(LockLocal*)this;
      ll->globalize(manI);
      return;
    }
  case Co_Object:
    {
      Object *o = (Object*) this;
      RecOrCell state = o->getState();
      if (!stateIsCell(state)) {
	SRecord *r = getRecord(state);
	Assert(r!=NULL);
	Tertiary *cell = tagged2Tert(OZ_newCell(makeTaggedSRecord(r)));
	Watcher *w, **ww = getWatcherBase();
	if(ww!=NULL){
	  w = *ww;
	  *ww = NULL;
	  cell->setMasterTert(o);
	  while(w!=NULL){
	    cell->insertWatcher(w);
	  }}
	cell->globalizeTert();
	o->setState(cell);}
      break;
    }
  case Co_Space:
  case Co_Port:
    break;
  default:
    Assert(0);
  }

  setTertType(Te_Manager);
  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  PD((GLOBALIZING,"GLOBALIZING port/object/space/thread index:%d",i));
  if(getType()==Co_Object)
    {PD((SPECIAL,"object:%x class%x",this,((Object *)this)->getClass()));}
  oe->mkTertiary(this);
  setIndex(i);
  if(getType()==Co_Object)
    {PD((SPECIAL,"object:%x class%x",this,((Object *)this)->getClass()));}
}

inline void convertCellProxyToFrame(Tertiary *t){
  Assert(t->isProxy());
  CellFrame *cf=(CellFrame*) t;
  cf->convertFromProxy();}

inline void convertLockProxyToFrame(Tertiary *t){
  Assert(t->isProxy());
  LockFrame *lf=(LockFrame*) t;
  lf->convertFromProxy();}

inline void maybeConvertLockProxyToFrame(Tertiary *t){
  if(t->isProxy()){
    convertLockProxyToFrame(t);}}
  
inline void maybeConvertCellProxyToFrame(Tertiary *t){
  if(t->isProxy()){
    convertCellProxyToFrame(t);}}


/**********************************************************************/
/*   SECTION 20 :: Localizing                                         */
/**********************************************************************/

void Object::localize()
{
  setTertType(Te_Local);
  setBoard(oz_currentBoard());
}

/**********************************************************************/
/*   SECTION 21 :: marshaling/unmarshaling by protocol-layer          */
/**********************************************************************/

/* for now credit is a 32-bit word */

inline void marshalCredit(Credit credit,MsgBuffer *bs){  
  Assert(sizeof(Credit)==sizeof(long));
  Assert(sizeof(Credit)==sizeof(unsigned int));
  PD((MARSHAL,"credit c:%d",credit));
  marshalNumber(credit,bs);}

void marshalCreditOutline(Credit credit,MsgBuffer *bs){  
  marshalCredit(credit,bs);}


inline Credit unmarshalCredit(MsgBuffer *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Credit c=unmarshalNumber(bs);
  PD((UNMARSHAL,"credit c:%d",c));
  return c;}

Credit unmarshalCreditOutline(MsgBuffer *bs){
  return unmarshalCredit(bs);}

void marshalOwnHead(int tag,int i,MsgBuffer *bs){
  if (bs->isPersistentBuffer()) return;
  PD((MARSHAL_CT,"OwnHead"));
  bs->put(tag);
  mySite->marshalSite(bs);
  marshalNumber(i,bs);
  bs->put(DIF_PRIMARY);
  Credit c=ownerTable->getOwner(i)->getSendCredit();
  marshalNumber(c,bs);
  PD((MARSHAL,"ownHead o:%d rest-c: ",i));
  return;}

void marshalToOwner(int bi,MsgBuffer *bs){
  if (bs->isPersistentBuffer()) return;
  PD((MARSHAL,"toOwner"));
  BorrowEntry *b=BT->getBorrow(bi); 
  int OTI=b->getOTI();
  if(b->getOnePrimaryCredit()){
    bs->put((BYTE) DIF_OWNER);
    marshalNumber(OTI,bs);
    PD((MARSHAL,"toOwner Borrow b:%d Owner o:%d",bi,OTI));
    return;}
  bs->put((BYTE) DIF_OWNER_SEC);
  Site* xcs = b->getOneSecondaryCredit();
  marshalNumber(OTI,bs);
  xcs->marshalSite(bs);
  return;}

void marshalBorrowHead(MarshalTag tag, int bi,MsgBuffer *bs){
  if (bs->isPersistentBuffer()) return;
  PD((MARSHAL,"BorrowHead"));	
  bs->put((BYTE)tag);
  BorrowEntry *b=borrowTable->getBorrow(bi);
  NetAddress *na=b->getNetAddress();
  na->site->marshalSite(bs);
  marshalNumber(na->index,bs);
  Credit cred=b->getSmallPrimaryCredit();
  if(cred) {
    PD((MARSHAL,"borrowed b:%d remCredit c: give c:%d",bi,cred));
    bs->put(DIF_PRIMARY);
    marshalCredit(cred,bs);
    return;}
  Site *ss=b->getSmallSecondaryCredit(cred);  
  bs->put(DIF_SECONDARY);
  marshalCredit(cred,bs);
  marshalSite(ss,bs);
  return;}

OZ_Term unmarshalBorrow(MsgBuffer *bs,OB_Entry *&ob,int &bi){
  PD((UNMARSHAL,"Borrow"));
  Site * sd=unmarshalSite(bs);
  int si=unmarshalNumber(bs);
  Credit cred;
  MarshalTag mt=(MarshalTag) bs->get();
  PD((UNMARSHAL,"borrow o:%d",si));
  if(sd==mySite){
    if(mt==DIF_PRIMARY){
      cred = unmarshalCredit(bs);      
      PD((UNMARSHAL,"mySite is owner"));
      OwnerEntry* oe=OT->getOwner(si);
      if(cred != PERSISTENT_CRED)
	oe->returnCreditOwner(cred);
      OZ_Term ret = oe->getValue();
      return ret;}
    Assert(mt==DIF_SECONDARY);
    cred = unmarshalCredit(bs);      
    Site* cs=unmarshalSite(bs);
    sendSecondaryCredit(cs,mySite,si,cred);
    PD((UNMARSHAL,"mySite is owner"));
    OwnerEntry* oe=OT->getOwner(si);
    OZ_Term ret = oe->getValue();
    return ret;}
  NetAddress na = NetAddress(sd,si); 
  BorrowEntry *b = borrowTable->find(&na);
    if (b!=NULL) {
      PD((UNMARSHAL,"borrow found"));
      cred = unmarshalCredit(bs);    
      if(mt==DIF_PRIMARY){
	if(cred!=PERSISTENT_CRED)
	  b->addPrimaryCredit(cred);
	else Assert(b->isPersistent());}
      else{
	Assert(mt==DIF_SECONDARY);
	Site *s=unmarshalSite(bs);
	b->addSecondaryCredit(cred,s);}
      ob = b;
      return b->getValue();}
  cred = unmarshalCredit(bs);    		
  if(mt==DIF_PRIMARY){
    bi=borrowTable->newBorrow(cred,sd,si);
    b=borrowTable->getBorrow(bi);
    if(cred == PERSISTENT_CRED )
      b->makePersistent();
    PD((UNMARSHAL,"borrowed miss"));
    ob=b;
    return 0;}
  Assert(mt==DIF_SECONDARY);
  Site* site = unmarshalSite(bs);    		  
  bi=borrowTable->newSecBorrow(site,cred,sd,si);
  b=borrowTable->getBorrow(bi);
  PD((UNMARSHAL,"borrowed miss"));
  ob=b;
  return 0;
}

char *tagToComment(MarshalTag tag){
  switch(tag){
  case DIF_PORT:
    return "port";
  case DIF_THREAD_UNUSED:
    return "thread";
  case DIF_SPACE:
    return "space";
  case DIF_CELL:
    return "cell";
  case DIF_LOCK:
    return "lock";
  case DIF_OBJECT:
    return "object";
  default:
    Assert(0);
    return "";
}}

/* ******************  interface *********************************** */

Bool marshalTertiary(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
{
  PD((MARSHAL,"Tert"));
  Site *sd=bs->getSite();
  switch(t->getTertType()){
  case Te_Local:
    t->globalizeTert();
    // no break here!
  case Te_Manager:
    {
      PD((MARSHAL_CT,"manager"));
      int OTI=t->getIndex();
      marshalOwnHead(tag,OTI,bs);
      break;
    }
  case Te_Frame:
  case Te_Proxy:
    {
      PD((MARSHAL,"proxy"));
      int BTI=t->getIndex();
      if (bs->getSite() && borrowTable->getOriginSite(BTI)==sd) {
	marshalToOwner(BTI,bs);
	return OK;}
      marshalBorrowHead(tag,BTI,bs);
      break;
    }
  default:
    Assert(0);
  }
  return NO;
}

OZ_Term unmarshalTertiary(MsgBuffer *bs, MarshalTag tag)
{
  OB_Entry* ob;
  int bi;
  OZ_Term val = unmarshalBorrow(bs,ob,bi);
  if(val){
    PD((UNMARSHAL,"%s hit b:%d",tagToComment(tag),bi));
    switch (tag) {
    case DIF_PORT:
    case DIF_THREAD_UNUSED:
    case DIF_SPACE:
      break;
    case DIF_CELL:{
      Tertiary *t=ob->getTertiary(); // mm2: bug: ob is 0 if I am the owner
      if((t->getType()==Co_Cell) && (t->isFrame())){
	((CellFrame *)t)->resetDumpBit();}
      break;}
    case DIF_LOCK:{
      Tertiary *t=ob->getTertiary();
      if((t->getType()==Co_Lock) && (t->isFrame())){
	((LockFrame *)t)->resetDumpBit();}
      break;}
    case DIF_OBJECT:
      TaggedRef obj;
      (void) unmarshalGName(&obj,bs);
      TaggedRef clas;
      (void) unmarshalGName(&clas,bs);
      break;
    default:         
      Assert(0);
    }
    return val;
  }

  PD((UNMARSHAL,"%s miss b:%d",tagToComment(tag),bi));  
  Tertiary *tert;

  switch (tag) {
  case DIF_PORT:
    tert = new PortProxy(bi);        
    break;
  case DIF_THREAD_UNUSED:
    // tert = new Thread(bi,Te_Proxy);  
    break;
  case DIF_SPACE:
    tert = new Space(bi,Te_Proxy);   
    break;
  case DIF_CELL:
    tert = new CellProxy(bi); 
    break;
  case DIF_LOCK:
    tert = new LockProxy(bi); 
    break;
  case DIF_OBJECT:
    {
      
      TaggedRef obj;
      GName *gnobj = unmarshalGName(&obj,bs);
      TaggedRef clas;
      GName *gnclass = unmarshalGName(&clas,bs);
      if(!gnobj){
	BT->maybeFreeBorrowEntry(bi);
	return obj;}
      
      Object *o = new Object(bi);
      o->setGName(gnobj);

      // mm2: abstraction val=newObjectProxy(o,gnobj,gnclass,clas)
      PerdioVar *pvar = new PerdioVar(o,oz_currentBoard());
      val = makeTaggedRef(newTaggedCVar(pvar));
      addGName(gnobj, val);
      if (gnclass) {
	pvar->setGNameClass(gnclass);
      } else {
	pvar->setClass(tagged2ObjectClass(oz_deref(clas)));
      }

      ob->mkVar(val); 
      return val;}
  default:         
    Assert(0);
  }
  val=makeTaggedConst(tert);
  ob->mkTertiary(tert,ob->getFlags()); 
  return val;
}

OZ_Term unmarshalOwner(MsgBuffer *bs,MarshalTag mt){
  if(mt==DIF_OWNER){
    int OTI=unmarshalNumber(bs);
    PD((UNMARSHAL,"OWNER o:%d",OTI));
    OwnerEntry* oe=OT->getOwner(OTI);
    oe->returnCreditOwner(1);
    OZ_Term oz=oe->getValue();
    return oz;}
  Assert(mt==DIF_OWNER_SEC);
  int OTI=unmarshalNumber(bs);
  Site *cs=unmarshalSite(bs);
  sendSecondaryCredit(cs,mySite,OTI,1);
  return OT->getOwner(OTI)->getValue();
}

/**********************************************************************/
/*   SECTION 22:: Main Receive                                       */
/**********************************************************************/

inline OwnerEntry* maybeReceiveAtOwner(Site *mS,int OTI){
  if(mS==mySite){
    OwnerEntry *oe=OT->getOwner(OTI);
    Assert(!oe->isFree());
    if(!oe->isPersistent())
      oe->receiveCredit(OTI);
    return oe;}
  return NULL;}

inline OwnerEntry* receiveAtOwner(int OTI){
  OwnerEntry *oe=OT->getOwner(OTI);
  Assert(!oe->isFree());
  if(!oe->isPersistent())
    oe->receiveCredit(OTI);
  return oe;}

inline OwnerEntry* receiveAtOwnerNoCredit(int OTI){
  OwnerEntry *oe=OT->getOwner(OTI);
  Assert(!oe->isFree());
  return oe;}

inline BorrowEntry* receiveAtBorrow(Site* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  Assert(be!=NULL);
  be->receiveCredit();
  return be;}

inline BorrowEntry* receiveAtBorrowNoCredit(Site* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  Assert(be!=NULL);
  return be;}

inline BorrowEntry* maybeReceiveAtBorrow(Site *mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  if(be==NULL){sendCreditBack(na.site,na.index,1);}
  else {be->receiveCredit();}
  return be;}

inline void sendPrepOwner(int index){
  OwnerEntry *oe=OT->getOwner(index);
  oe->getOneCreditOwner();}

//
// kost@ 26.3.98 : 'msgReceived()' is NOT a method of a site object.
void msgReceived(MsgBuffer* bs)
{
  Assert(oz_onToplevel());
  MessageType mt = (MessageType) unmarshalHeader(bs);
  PD((MSG_RECEIVED,"msg type %d",mt));

  switch (mt) {
  case M_PORT_SEND:   
    {
      int portIndex;
      OZ_Term t;
      unmarshal_M_PORT_SEND(bs,portIndex,t);
      PD((MSG_RECEIVED,"PORTSEND: o:%d v:%s",portIndex,toC(t)));

      OwnerEntry *oe=receiveAtOwner(portIndex);
      Assert(oe);
      PortManager *pm=(PortManager*)(oe->getTertiary());
      Assert(pm->checkTertiary(Co_Port,Te_Manager));

      LTuple *lt = new LTuple(t,am.currentUVarPrototype());
      OZ_Term old = pm->exchangeStream(lt->getTail());
      PD((SPECIAL,"just after send port"));
      Assert(MemChunks::isInHeap(makeTaggedConst(pm)));
      SiteUnify(makeTaggedLTuple(lt),old); // ATTENTION
      break;
      }

  case M_REMOTE_SEND:    /* index string term */
    {
      int i;
      char *biName;
      OZ_Term t;
      unmarshal_M_REMOTE_SEND(bs,i,biName,t);
      PD((MSG_RECEIVED,"REMOTE_SEND: o:%d bi:%s v:%s",i,biName,toC(t)));

      OwnerEntry *oe=receiveAtOwner(i);
      Tertiary *tert= oe->getTertiary();
      Builtin *found = string2Builtin(biName);
      if (!found) {
	PD((WEIRD,"builtin %s not found",biName));
	break;
      }

      RefsArray args=allocateRefsArray(2,NO);
      args[0]=makeTaggedConst(tert);
      args[1]=t;
      int arity=found->getArity();
      Assert(arity<=2);
      OZ_Return ret = oz_bi_wrapper(found,args);
      if (ret != PROCEED) {
	PD((SPECIAL,"REMOTE_SEND failed: %d\n",ret));
      }
      break;
    }

  case M_ASK_FOR_CREDIT:
    {
      int na_index;
      Site* rsite;
      unmarshal_M_ASK_FOR_CREDIT(bs,na_index,rsite);
      PD((MSG_RECEIVED,"ASK_FOR_CREDIT index:%d site:%s",
	  na_index,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(na_index);
      Credit c= oe->giveMoreCredit();
      MsgBuffer *bs=msgBufferManager->getMsgBuffer(rsite);  
      marshal_M_BORROW_CREDIT(bs,mySite,na_index,c);
      SendTo(rsite,bs,M_BORROW_CREDIT,mySite,na_index);
      break;
    }

  case M_OWNER_CREDIT: 
    {
      int index;
      Credit c;
      unmarshal_M_OWNER_CREDIT(bs,index,c);
      PD((MSG_RECEIVED,"OWNER_CREDIT index:%d credit:%d",index,c));
      receiveAtOwnerNoCredit(index)->returnCreditOwner(c);
      break;
    }

  case M_OWNER_SEC_CREDIT:
    {
      int index;
      Credit c;
      Site *s;
      unmarshal_M_OWNER_SEC_CREDIT(bs,s,index,c);
      PD((MSG_RECEIVED,"OWNER_SEC_CREDIT site:%s index:%d credit:%d",
	  s->stringrep(),index,c));    
      receiveAtBorrowNoCredit(s,index)->addSecondaryCredit(c,mySite);
      creditSite = NULL;
      break;
    }

  case M_BORROW_CREDIT:  
    {
      int si;
      Credit c;
      Site *sd;
      unmarshal_M_BORROW_CREDIT(bs,sd,si,c);
      PD((MSG_RECEIVED,"BORROW_CREDIT site:%s index:%d credit:%d",
	  sd->stringrep(),si,c));
      receiveAtBorrowNoCredit(sd,si)->addPrimaryCredit(c);
      break;
    }

  case M_REGISTER:
    {
      int OTI;
      Site *rsite;
      unmarshal_M_REGISTER(bs,OTI,rsite);
      PD((MSG_RECEIVED,"REGISTER index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(OTI);
      if (oe->isVar()) {
	oe->getPerdioVar()->registerSite(rsite);
      } else {
	sendRedirect(rsite,OTI,OT->getOwner(OTI)->getRef());
      }
      break;
    }

  case M_GET_OBJECT:
  case M_GET_OBJECTANDCLASS:
    {
      int OTI;
      Site *rsite;
      unmarshal_M_GET_OBJECT(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_GET_OBJECT(ANDCLASS) index:%d site:%s",
	  OTI,rsite->stringrep()));
      //      OwnerEntry *oe=receiveAtOwner(OTI);
      OwnerEntry *oe=OT->getOwner(OTI);
      Tertiary *t = oe->getTertiary();
      Assert(isObject(t));
      PD((SPECIAL,"object get %x %x",t,((Object *)t)->getClass()));
      sendObject(rsite,(Object *)t, mt==M_GET_OBJECTANDCLASS);
      break;
    }	
  case M_SEND_OBJECT:
    {
      ObjectFields of;
      Site *sd;
      int si;
      unmarshal_M_SEND_OBJECT(bs,sd,si,&of);
      PD((MSG_RECEIVED,"M_SEND_OBJECT site:%s index:%d",sd->stringrep(),si));
      BorrowEntry *be=receiveAtBorrow(sd,si);

      PerdioVar *pv = be->getPerdioVar();
      Object *o = pv->getObject();
      Assert(o);
      GName *gnobj = o->hasGName();
      Assert(gnobj);
      gnobj->setValue(makeTaggedConst(o));
            
      fillInObject(&of,o);
      ObjectClass *cl;
      if (pv->isObjectClassAvail()) {cl=pv->getClass();}
      else {
	cl=tagged2ObjectClass(oz_deref(oz_findGName(pv->getGNameClass())));}
      o->setClass(cl);

      pv->primBind(be->getPtr(),makeTaggedConst(o));
      be->mkRef();
      BT->maybeFreeBorrowEntry(o->getIndex());
      o->localize();
      break;
    }

  case M_SEND_OBJECTANDCLASS:
    {
      ObjectFields of;
      Site *sd;
      int si;
      unmarshal_M_SEND_OBJECTANDCLASS(bs,sd,si,&of);
      PD((MSG_RECEIVED,"M_SEND_OBJECTANDCLASS site:%s index:%d",
	  sd->stringrep(),si));
      BorrowEntry *be=receiveAtBorrow(sd,si);
      
      PerdioVar *pv = be->getPerdioVar();
      Object *o = pv->getObject();
      Assert(o);
      GName *gnobj = o->hasGName();
      Assert(gnobj);
      gnobj->setValue(makeTaggedConst(o));
      
      fillInObjectAndClass(&of,o);
      pv->primBind(be->getPtr(),makeTaggedConst(o));
      be->mkRef();
      BT->maybeFreeBorrowEntry(o->getIndex());
      o->localize();
      break;
    }

  case M_REDIRECT:
    {
      Site *sd;
      int si;
      TaggedRef val;
      unmarshal_M_REDIRECT(bs,sd,si,val);
      PD((MSG_RECEIVED,"M_REDIRECT site:%s index:%d val%s",
	  sd->stringrep(),si,toC(val)));
      BorrowEntry* be=maybeReceiveAtBorrow(sd,si);
      if (!be) { // if not found, then forget the redirect message
	PD((WEIRD,"REDIRECT: no borrow entry found"));
	break;
      }
      Assert(be->isVar());

      // mm2: abstraction: pv->proxyBind(vPtr,val)
      PerdioVar *pv = be->getPerdioVar();
      PD((TABLE,"REDIRECT - borrow entry hit b:%d",pv->getIndex()));
      Assert(pv->isProxy());
      pv->primBind(be->getPtr(),val);
      be->mkRef();
      if (pv->hasVal()) {
	PD((PD_VAR,"REDIRECT while pending"));
	pv->redirect(val);
      }
      // pv->dispose();
      BT->maybeFreeBorrowEntry(pv->getIndex());

      break;
    }

  case M_SURRENDER:
    {
      int OTI;
      Site* rsite;
      TaggedRef v;
      unmarshal_M_SURRENDER(bs,OTI,rsite,v);
      PD((MSG_RECEIVED,"M_SURRENDER index:%d site:%s val%s",
	  OTI,rsite->stringrep(),toC(v)));
      OwnerEntry *oe = receiveAtOwner(OTI);

      if (oe->isVar()) {

	// mm2: abstraction: pv->managerBind(varPtr,v)
	PD((PD_VAR,"SURRENDER do it"));
	PerdioVar *pv = oe->getPerdioVar();
	// mm2: bug: the new var may no be the correct one wrt.
        //           to variable ordering -> may introduce net cycle.
	// ??: bug fixed: may be bound to a different perdio var
	pv->primBind(oe->getPtr(),v);
	oe->mkRef();
	if (oe->hasFullCredit()) {
	  PD((WEIRD,"SURRENDER: full credit"));
	}
	sendRedirect(pv,v,rsite,OTI);


      } else {
	PD((PD_VAR,"SURRENDER discard"));
	PD((WEIRD,"SURRENDER discard"));
	// ignore redirect: NOTE: v is handled by the usual garbage collection
      }
      break;
    }

  case M_ACKNOWLEDGE:
    {
      
      Site *sd;
      int si;
      unmarshal_M_ACKNOWLEDGE(bs,sd,si);
      PD((MSG_RECEIVED,"M_ACKNOWLEDGE site:%s index:%d",sd->stringrep(),si));

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);
      Assert(be);
      be->receiveCredit();

      // mm2: abstraction: pv->proxyAck(varPtr);
      Assert(be->isVar());
      PerdioVar *pv = be->getPerdioVar();
      pv->acknowledge(be->getPtr());
      be->mkRef();
      // pv->dispose();
      BT->maybeFreeBorrowEntry(pv->getIndex());

      break;
    }
  case M_CELL_LOCK_GET:
    {
      int OTI;
      Site* rsite;
      unmarshal_M_CELL_LOCK_GET(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_GET index:%d site:%s",OTI,rsite->stringrep()));
      cellLockReceiveGet(receiveAtOwner(OTI),rsite);
      break;
    }
   case M_CELL_CONTENTS:
    {
      Site *rsite;
      int OTI;
      TaggedRef val;
      unmarshal_M_CELL_CONTENTS(bs,rsite,OTI,val);
      PD((MSG_RECEIVED,"M_CELL_CONTENTS index:%d site:%s val:%s",
	  OTI,rsite->stringrep(),toC(val)));

      OwnerEntry* oe=maybeReceiveAtOwner(rsite,OTI);
      if(oe!=NULL){
	cellReceiveContentsManager(oe,val,OTI);
	break;}

      cellReceiveContentsFrame(receiveAtBorrow(rsite,OTI),val,rsite,OTI);
      break;
    }
  case M_CELL_READ:
    {
      int OTI;
      Site *fS;
      unmarshal_M_CELL_READ(bs,OTI,fS);
      PD((MSG_RECEIVED,"M_CELL_READ"));
      cellReceiveRead(receiveAtOwner(OTI),fS); 
      break;
    }
  case M_CELL_REMOTEREAD:      
    {
      int OTI;
      Site *fS,*mS;
      unmarshal_M_CELL_REMOTEREAD(bs,mS,OTI,fS);
      PD((MSG_RECEIVED,"CELL_REMOTEREAD %s",fS->stringrep()));
      cellReceiveRemoteRead(receiveAtBorrow(mS,OTI),mS,OTI,fS); 
      break;
    }
  case M_CELL_READANS:
    {
      int index;
      Site*mS;
      TaggedRef val;
      unmarshal_M_CELL_READANS(bs,mS,index,val);
      PD((MSG_RECEIVED,"CELL_READANS"));
      OwnerEntry *oe=maybeReceiveAtOwner(mS,index);
      if(oe==NULL){
	cellReceiveReadAns(receiveAtBorrow(mS,index)->getTertiary(),val);
	break;}
      cellReceiveReadAns(oe->getTertiary(),val); 
      break;
   }   
  case M_CELL_LOCK_FORWARD:
    {
      Site *site,*rsite;
      int OTI;
      unmarshal_M_CELL_LOCK_FORWARD(bs,site,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_FORWARD index:%d site:%s rsite:%s",
	  OTI,site->stringrep(),rsite->stringrep()));

      cellLockReceiveForward(receiveAtBorrow(site,OTI),rsite,site,OTI);
      break;
    }
  case M_CELL_LOCK_DUMP:
    {
      int OTI;
      Site* rsite;
      unmarshal_M_CELL_LOCK_DUMP(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_DUMP index:%d site:%s",
	  OTI,rsite->stringrep()));
      cellLockReceiveDump(receiveAtOwner(OTI),rsite);
      break;
    }
  case M_CELL_CANTPUT:
    {
      Site *rsite, *ssite;
      int OTI;
      TaggedRef val;
      unmarshal_M_CELL_CANTPUT(bs, OTI, rsite, val, ssite);
      PD((MSG_RECEIVED,"M_CELL_CANTPUT index:%d site:%s val:%s",
	  OTI,rsite->stringrep(),toC(val)));
      cellReceiveCantPut(receiveAtOwner(OTI),val,OTI,ssite,rsite);
      break;
    }  
  case M_LOCK_TOKEN:
    {
      Site *rsite;
      int OTI;
      unmarshal_M_LOCK_TOKEN(bs,rsite,OTI);
      PD((MSG_RECEIVED,"M_LOCK_TOKEN index:%d site:%s",
	  OTI,rsite->stringrep()));
      OwnerEntry *oe=maybeReceiveAtOwner(rsite,OTI);
      if(oe!=NULL){
	lockReceiveTokenManager(oe,OTI);
	break;}
      lockReceiveTokenFrame(receiveAtBorrow(rsite,OTI),rsite,OTI);
      break;
    }
  case M_CHAIN_ACK:
    {
      int OTI;
      Site* rsite;
      unmarshal_M_CHAIN_ACK(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CHAIN_ACK index:%d site:%s",
	  OTI,rsite->stringrep()));
      chainReceiveAck(receiveAtOwner(OTI),rsite);
      break;
    }
  case M_LOCK_CANTPUT:
    {
      Site *rsite, *ssite;
      int OTI;
      TaggedRef val;
      unmarshal_M_LOCK_CANTPUT(bs, OTI, rsite, ssite);
      PD((MSG_RECEIVED,"M_LOCK_CANTPUT index:%d site:%s val:%s",
	  OTI,rsite->stringrep()));
      lockReceiveCantPut(receiveAtOwner(OTI),OTI,ssite,rsite);
      break;
    }
  case M_CHAIN_QUESTION:
   {
      Site *site,*rsite,*deadS;
      int OTI;
      unmarshal_M_CHAIN_QUESTION(bs,OTI,site,deadS);
      PD((MSG_RECEIVED,"M_CHAIN_QUESTION index:%d site:%s",
	  OTI,site->stringrep()));
      BorrowEntry *be=maybeReceiveAtBorrow(site,OTI);
      if(be==NULL) break;
      chainReceiveQuestion(be,site,OTI,deadS);
      break;
   }
  case M_CHAIN_ANSWER:
    {
      Site *rsite,*deadS;
      int OTI;
      int ans;
      unmarshal_M_CHAIN_ANSWER(bs,OTI,rsite,ans,deadS);
      PD((MSG_RECEIVED,"M_CHAIN_ANSWER index:%d site:%s val:%d",
	  OTI,rsite->stringrep(),ans));
      chainReceiveAnswer(receiveAtOwner(OTI),rsite,ans,deadS);
      break;
    }  

  case M_TELL_ERROR:
    {
      Site *site;
      int OTI;
      int ec,flag;
      unmarshal_M_TELL_ERROR(bs,site,OTI,ec,flag);
      PD((MSG_RECEIVED,"M_TELL_ERROR index:%d site:%s ec:%d",
	  OTI,site->stringrep(),ec));
      BorrowEntry *be=maybeReceiveAtBorrow(site,OTI);
      if(be==NULL) break;
      receiveTellError(be->getTertiary(),site,OTI,ec,flag);
      break;
    }

  case M_ASK_ERROR:
    {
      int OTI;
      int ec;
      Site *toS;
      unmarshal_M_ASK_ERROR(bs,OTI,toS,ec);
      PD((MSG_RECEIVED,"M_ASK_ERROR index:%d ec:%d toS:%s",
	  OTI,ec,toS->stringrep()));
      receiveAskError(receiveAtOwner(OTI),toS,ec);
      break; 
    }
  case M_UNASK_ERROR:
    {
      int OTI;
      int ec;
      Site *toS;
      unmarshal_M_UNASK_ERROR(bs,OTI,toS,ec);
      PD((MSG_RECEIVED,"M_UNASK_ERROR index:%d ec:%d toS:%s",
	  OTI,ec,toS->stringrep()));
      receiveUnAskError(receiveAtOwner(OTI),toS,ec);
      break; 
    }

  default:
    error("siteReceive: unknown message %d\n",mt);
    break;
  }
}


/**********************************************************************/
/*   SECTION 23:: remote send protocol                                */
/**********************************************************************/


/* engine-interface */
OZ_Return remoteSend(Tertiary *p, char *biName, TaggedRef msg) {
  BorrowEntry *b= borrowTable->getBorrow(p->getIndex());
  NetAddress *na = b->getNetAddress();
  Site* site = na->site;
  int index = na->index;

  MsgBuffer *bs = msgBufferManager->getMsgBuffer(site);
  b->getOneMsgCredit();
  marshal_M_REMOTE_SEND(bs,index,biName,msg);
  CheckNogoods(msg,bs,"Resources found during send to port",);
  SendTo(site,bs,M_REMOTE_SEND,site,index);
  return PROCEED;
}
  
/**********************************************************************/
/*   SECTION 24:: Port protocol                                       */
/**********************************************************************/

OZ_Return portWait(int queueSize, int restTime, Tertiary *t)
{
  PD((ERROR_DET,"PortWait q: %d r: %d", queueSize, restTime));
  int v = queueSize - PortSendTreash;
  int time;
  if(v<0) return PROCEED;
  int tot=PortWaitTimeK * queueSize;
  if(restTime && restTime < tot) tot = restTime;
  if (v < PortWaitTimeSlice) {
    time = v; 
  } else {
    time = PortWaitTimeSlice;
    am.prepareCall(BI_portWait,makeTaggedTert(t),
		   oz_int(tot - PortWaitTimeSlice));
  }
  am.prepareCall(BI_Delay,oz_int(time));
  return BI_REPLACEBICALL;
}

Bool Tertiary::startHandlerPort(Thread* th, Tertiary* t, TaggedRef msg,
				EntityCond ec){
  PD((ERROR_DET,"entityProblem invoked Port"));
  if(getWatcherBase() == NULL) return FALSE;
  Watcher** base=getWatcherBase();
  Watcher* w=*base;
  Bool ret=FALSE;
  while(w!=NULL){
    if((!w->isTriggered(ec)) || 
       ((w->isHandler()) && ((th != w->getThread()) || 
	(DefaultThread == w->getThread())))){
      base= &(w->next);
      w=*base;}
    else{
      if(w->isHandler()){
	ret = TRUE;
	if(w->isContinueHandler()) {
	  Assert(th == oz_currentThread());
	  am.prepareCall(BI_send,makeTaggedTert(t),msg);
	}
	w->invokeHandler(ec,this,th,0);}
      else{
	w->invokeWatcher(ec,this);}
       if(!w->isPersistent()){
	 releaseWatcher(w);
	 *base=w->next;
	 w=*base;}
       else{
	 base= &(w->next);
	 w=*base;}
    }}
  return ret;}

OZ_Return portSend(Tertiary *p, TaggedRef msg) 
{
  BorrowEntry* b = BT->getBorrow(p->getIndex());
  NetAddress *na = b->getNetAddress();
  Site* site     = na->site;
  int index      = na->index;
  int dummy;
  Bool wait = FALSE;
  switch(getEntityCondPort(p)){
  case PERM_BLOCKED|PERM_ME:{
    PD((ERROR_DET,"Port is PERM"));
    if(p->startHandlerPort(oz_currentThread(), p, msg, PERM_BLOCKED|PERM_ME))
      return BI_REPLACEBICALL;
    /* no handler --> suspend forever: */
    ControlVarNew(var,p->getBoardInternal());
    SuspendOnControlVar;
  }
  case TEMP_BLOCKED|TEMP_ME:{
    PD((ERROR_DET,"Port is Tmp size:%d treash:%d",
	site->getQueueStatus(dummy),PortSendTreash));
    wait = TRUE;
    if(p->startHandlerPort(oz_currentThread(), p, msg, TEMP_BLOCKED|TEMP_ME))
      return BI_REPLACEBICALL;
    break;
  }
  case ENTITY_NORMAL: break;
  default: Assert(0);
  }

  MsgBuffer *bs=msgBufferManager->getMsgBuffer(site);
  b->getOneMsgCredit();
  marshal_M_PORT_SEND(bs,index,msg);
  
  OZ_Term nogoods = bs->getNoGoods();
  if (!literalEq(oz_nil(),nogoods)) {
  /*
    int portIndex;
    OZ_Term t;
    unmarshal_M_PORT_SEND(bs,portIndex,t);
    dumpRemoteMsgBuffer(bs);
    */
    return raiseGeneric("Resources found during send to port",
			oz_mklist(OZ_pairA("Resources",nogoods),
				  OZ_pairA("Port",makeTaggedTert(p))));
  }
  

  SendTo(site,bs,M_PORT_SEND,site,index);
  return wait ? portWait(site->getQueueStatus(dummy), 0, p)
              : PROCEED;
  }

/**********************************************************************/
/*   SECTION 25:: Variable protocol                                       */
/**********************************************************************/


/**********************************************************************/
/*   SECTION 26:: Object protocol                                     */
/**********************************************************************/

void sendObject(Site* sd, Object *o, Bool sendClass){ // holding one credit
  int OTI = o->getIndex();
  OT->getOwner(OTI)->getOneCreditOwner();
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  if(sendClass){
    marshal_M_SEND_OBJECTANDCLASS(bs,mySite,OTI,o);
    SendTo(sd,bs,M_SEND_OBJECTANDCLASS,mySite,OTI);}
  else{
    marshal_M_SEND_OBJECT(bs,mySite,OTI,o);
    SendTo(sd,bs,M_SEND_OBJECT,mySite,OTI);}}

/**********************************************************************/
/*   SECTION 27:: Credit protocol                                     */
/**********************************************************************/

void sendPrimaryCredit(Site *sd,int OTI,Credit c){
  PD((CREDIT,"Sending PrimaryCreds"));
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  Site *cr = creditSite;
  marshal_M_OWNER_CREDIT(bs,OTI,c);
  creditSite = cr;
  SendTo(sd,bs,M_OWNER_CREDIT,sd,OTI);}

void sendSecondaryCredit(Site *cs,Site *sd,int OTI,Credit c){
  PD((CREDIT,"Sending PrimaryCreds"));
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(cs);
  Site *cr = creditSite;
  marshal_M_OWNER_SEC_CREDIT(bs,sd,OTI,c);
  creditSite = cr;
  SendTo(cs,bs,M_OWNER_SEC_CREDIT,sd,OTI);}

void sendCreditBack(Site* sd,int OTI,Credit c){ 
  int ret;
  if(creditSite==NULL){
    sendPrimaryCredit(sd,OTI,c);
    return;}
  sendSecondaryCredit(creditSite,sd,OTI,c);
  return;}

/**********************************************************************/
/*   SECTION 28:: Cell lock protocol common                           */
/**********************************************************************/

void Chain::informHandleTempOnAdd(OwnerEntry* oe,Tertiary *t,Site *s){
  InformElem *ie=getInform();
  while(ie!=NULL){
    if(ie->site==s){
      EntityCond ec=ie->wouldTrigger(TEMP_BLOCKED|TEMP_SOME|TEMP_ME);
      if(ec!=ENTITY_NORMAL){
	sendTellError(oe,s,t->getIndex(),ec,TRUE);}}
    ie=ie->next;}}

void cellLockReceiveGet(OwnerEntry* oe,Site* toS){  
  Tertiary* t=oe->getTertiary();
  Chain *ch=getChainFromTertiary(t); 
  if(ch->hasFlag(TOKEN_LOST)){
    PD((ERROR_DET,"TOKEN_LOST message bouncing"));
    sendTellError(oe,toS,t->getIndex(),PERM_BLOCKED|PERM_SOME|PERM_ME,true);
    return;}
  if(t->getType()==Co_Cell){
    cellReceiveGet(oe,(CellManager*) t,toS);}
  else{
    lockReceiveGet(oe,(LockManager*) t,toS);}
  if(ch->hasFlag(INTERESTED_IN_OK)){ 
    Assert(ch->tempConnectionInChain());
    ch->informHandleTempOnAdd(oe,t,toS);}}

void cellLockReceiveForward(BorrowEntry *be,Site* toS,Site* mS,int mI){
  if(be->getTertiary()->getType()==Co_Cell){
    cellReceiveForward(be,toS,mS,mI);
    return;}
  lockReceiveForward(be,toS,mS,mI);}

void cellLockSendGet(BorrowEntry *be){      
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  PD((CELL,"M_CELL_LOCK_GET indx:%d site:%s",na->index,toS->stringrep()));
  marshal_M_CELL_LOCK_GET(bs,na->index,mySite);
  SendTo(toS,bs,M_CELL_LOCK_GET,toS,na->index);}

void cellLockSendForward(Site *toS,Site *fS,int mI){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_LOCK_FORWARD(bs,mySite,mI,fS);
  SendTo(toS,bs,M_CELL_LOCK_FORWARD,mySite,mI);}

void cellLockReceiveDump(OwnerEntry *oe,Site* fromS){
  Tertiary *t=oe->getTertiary();
  if(t->getType()==Co_Cell){
    cellReceiveDump((CellManager*) t,fromS);}
  else{
    lockReceiveDump((LockManager*) t,fromS);}}

void cellLockSendDump(BorrowEntry *be){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  if(SEND_SHORT(toS)){return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_LOCK_DUMP(bs,na->index,mySite);
  SendTo(toS,bs,M_CELL_LOCK_DUMP,toS,na->index);}

/**********************************************************************/
/*   SECTION 28:: Cell protocol - receive                            */
/**********************************************************************/

Bool CellSec::secForward(Site* toS,TaggedRef &val){
  if(state==Cell_Lock_Valid){
    state=Cell_Lock_Invalid;
    val=contents;
    return OK;}
  Assert(state==Cell_Lock_Requested);
  state=Cell_Lock_Requested|Cell_Lock_Next;
  next=toS;
  return NO;}

Bool CellSec::secReceiveContents(TaggedRef val,Site* &toS,TaggedRef &outval){
  PendThread *tmp; 
  contents = val;
  while(pending!=NULL){
    Thread *t=pending->thread;
    Assert(t!=MoveThread);
    (void) exchangeVal(pending->old,pending->nw,t,
		       pending->controlvar,pending->exKind);
    tmp=pending;
    pending=pending->next;
    tmp->dispose();}
  outval = contents;
  pending=NULL;
  if(state & Cell_Lock_Next){
    state = Cell_Lock_Invalid;
    toS=next;
    return OK;}
  state = Cell_Lock_Valid;
  return NO;}

void CellSec::secReceiveReadAns(TaggedRef val){
  PendThread* pb=pendBinding;
  while(pb!=NULL){
    if(pb->exKind==ACCESS) {
      ControlVarUnify(pb->controlvar,pb->old,val);
    } else {
      val = oz_deref(val);
      TaggedRef tr = tagged2SRecord(val)->getFeature(pb->nw);
      if(tr) {
	ControlVarUnify(pb->controlvar,tr,pb->old);
      } else {
	ControlVarRaise(pb->controlvar,
			OZ_makeException(E_ERROR,E_OBJECT,"@",2,val,pb->nw));
      }
    }
    pb=pb->next;
  }
  pendBinding=NULL;
}

Bool CellSec::secReceiveRemoteRead(Site* toS,Site* mS, int mI){
  switch(state){
  case Cell_Lock_Invalid:
    return NO;
  case Cell_Lock_Valid:{
    cellSendReadAns(toS,mS,mI,contents);
    return TRUE;}
  case Cell_Lock_Requested:{
    OZ_Return aux = pendThreadAddToEnd(&pending,(Thread*)toS,(TaggedRef) mS,
				       mI,REMOTEACCESS,oz_rootBoard());
    Assert(aux==PROCEED);
    return TRUE;}
  default: Assert(0);}
  return NO;}

void cellReceiveGet(OwnerEntry* oe,CellManager* cm,Site* toS){  
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  Chain *ch=cm->getChain();
  Site* current=ch->setCurrent(toS,cm);
  PD((CELL,"CellMgr Received get from %s",toS->stringrep()));
  PD((CHAIN,"%d",printChain(ch)));
  if(current==mySite){
    PD((CELL,"CELL - shortcut in cellReceiveGet"));    
    TaggedRef val;
    if(cm->getSec()->secForward(toS,val)){
      oe->getOneCreditOwner();
      cellSendContents(val,toS,mySite,cm->getIndex());}
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,cm->getIndex());}

void cellReceiveDump(CellManager *cm,Site *fromS){
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  if((cm->getChain()->getCurrent()!=fromS) ||
     (cm->getState()!=Cell_Lock_Invalid)){
    PD((WEIRD,"CELL dump not needed"));
    return;}
  TaggedRef tr=oz_newVariable();
  (void) cellDoExchange((Tertiary *)cm,tr,tr,DummyThread,EXCHANGE);
  return;}

void cellReceiveForward(BorrowEntry *be,Site *toS,Site* mS,int mI){
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->isFrame());
  Assert(cf->getType()==Co_Cell);
  CellSec *sec=cf->getSec();
  TaggedRef val;
  cf->resetDumpBit();
  if(!sec->secForward(toS,val)) return;
  be->getOneMsgCredit();
  cellSendContents(val,toS,mS,mI);
  return;}

void cellReceiveContentsManager(OwnerEntry *oe,TaggedRef val,int mI){ 
  CellManager *cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  if(tokenLostCheckManager(cm)) return; // ERROR-HOOK ATTENTION
  chainReceiveAck(oe,mySite);
  CellSec *sec=cm->getSec();
  Site *toS;
  TaggedRef outval;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  oe->getOneCreditOwner();
  cellSendContents(outval,toS,mySite,mI);
  return;}

void cellReceiveContentsFrame(BorrowEntry *be,TaggedRef val,Site *mS,int mI){ 
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->getType()==Co_Cell);
  Assert(cf->isFrame());
  if(tokenLostCheckProxy(cf)) return; // ERROR-HOOK ATTENTION
  be->getOneMsgCredit();
  chainSendAck(mS,mI);    
  CellSec *sec=cf->getSec();
  TaggedRef outval;
  Site *toS;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  be->getOneMsgCredit();
  cellSendContents(outval,toS,mS,mI);}

void cellReceiveRemoteRead(BorrowEntry *be,Site* mS,int mI,Site* fS){ 
  PD((CELL,"Receive REMOTEREAD toS:%s",fS->stringrep()));
  Tertiary* t=be->getTertiary();
  Assert(t->isFrame());
  Assert(t->getType()==Co_Cell);
  CellSec *sec=((CellFrame*)t)->getSec();
  be->getOneMsgCredit();
  TaggedRef val;
  if(sec->secReceiveRemoteRead(fS,mS,mI)) return;
  PD((WEIRD,"miss on read"));
  be->getOneMsgCredit();
  cellSendRead(be,fS);}

void cellReceiveRead(OwnerEntry *oe,Site* fS){ 
  PD((CELL,"Recevie READ toS:%s",fS->stringrep()));
  CellManager* cm=(CellManager*) oe->getTertiary();
  Assert(cm->isManager());
  Assert(cm->getType()==Co_Cell);
  CellSec *sec=cm->getSec();
  oe->getOneCreditOwner();
  Chain* ch=cm->getChain();
  if(ch->getCurrent()==mySite){
    PD((CELL,"Token at mgr site short circuit"));
    sec->secReceiveRemoteRead(fS,mySite,cm->getIndex());
    return;}
  cellSendRemoteRead(ch->getCurrent(),mySite,cm->getIndex(),fS);}

void cellReceiveReadAns(Tertiary* t,TaggedRef val){ 
  Assert((t->isManager())|| (t->isFrame()));
  getCellSecFromFrameOrManager(t)->secReceiveReadAns(val);}

/**********************************************************************/
/*   SECTION 29:: Cell protocol - holding credit                      */
/**********************************************************************/

void cellSendReadAns(Site* toS,Site* mS,int mI,TaggedRef val){ 
  if(toS == mySite) {
    OwnerEntry *oe=maybeReceiveAtOwner(mS,mI);
    if(mS!=mySite)
      cellReceiveReadAns(receiveAtBorrow(mS,mI)->getTertiary(),val);
    else
      cellReceiveReadAns(maybeReceiveAtOwner(mS,mI)->getTertiary(),val); 
    return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_READANS(bs,mS,mI,val);
  SendTo(toS,bs,M_CELL_READANS,mS,mI);}

void cellSendRemoteRead(Site* toS,Site* mS,int mI,Site* fS){ 
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_REMOTEREAD(bs,mS,mI,fS);
  SendTo(toS,bs,M_CELL_REMOTEREAD,mS,mI);}

void cellSendContents(TaggedRef tr,Site* toS,Site *mS,int mI){
  PD((CELL,"Cell Send Contents to:%s",toS->stringrep()));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_CONTENTS(bs,mS,mI,tr);
  PD((SPECIAL,"CellContents %s",toC(tr)));
  SendTo(toS,bs,M_CELL_CONTENTS,mS,mI);}

void cellSendRead(BorrowEntry *be,Site *dS){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_READ(bs,na->index,dS);
  SendTo(toS,bs,M_CELL_READ,na->site,na->index);}

/**********************************************************************/
/*   SECTION 30:: Cell protocol - basics                              */
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
    cellSendReadAns((Site*)th,(Site*)old,(int)nw,contents);
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

inline CellSec *getCellSecFromTert(Tertiary *c){
  if(c->isManager()){
    return ((CellManager*)c)->getSec();}
  Assert(!c->isProxy());
  return ((CellFrame*)c)->getSec();}

inline Bool maybeInvokeHandler(Tertiary* t,Thread* th){
  if(t->getEntityCond()==ENTITY_NORMAL) return NO;
  if(!t->handlerExists(th)) return NO;
  return OK;}

inline void genInvokeHandlerLockOrCell(Tertiary* t,Thread* th){
  if(!t->handlerExists(th)) return;
  if(t->findWatcherBase(th,t->getEntityCond()) == NULL) return;
  insertDangelingEvent(t);
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
    if(c->errorIgnore()) return ret;
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
      if(c->errorIgnore()) return ret;
      break;}
    Assert(c->isManager());
    if(!((CellManager*)c)->getChain()->hasFlag(TOKEN_LOST)){
      Site *toS=((CellManager*)c)->getChain()->setCurrent(mySite,c);
      sendPrepOwner(index);
      cellLockSendForward(toS,mySite,index);
      if(c->errorIgnore()) return ret;}
    break;}
  default: Assert(0);
  }
  if(maybeInvokeHandler(c,th)){ // ERROR-HOOK
    genInvokeHandlerLockOrCell(c,th);}

  return ret;
}
  
static
OZ_Return cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw,Thread *th,
			 ExKind e)
{
  PD((SPECIAL,"exchange old:%d new:%s type:%d",toC(old),toC(nw),e));
  if(c->isProxy()){
    convertCellProxyToFrame(c);}
  PD((CELL,"CELL: exchange on %s-%d",getNASiteFromTertiary(c)->stringrep(),
      getNAIndexFromTertiary(c)));
  return getCellSecFromTert(c)->exchange(c,old,nw,th,e);
}

OZ_Return cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw){
   return cellDoExchange(c,old,nw,oz_currentThread(),EXCHANGE);}

OZ_Return cellAssignExchange(Tertiary *c,TaggedRef fea,TaggedRef val){
   return cellDoExchange(c,fea,val,oz_currentThread(), ASSIGN);}

OZ_Return cellAtExchange(Tertiary *c,TaggedRef old,TaggedRef nw){
  return cellDoExchange(c,old,nw,oz_currentThread(), AT);}

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

  Bool ask = (pendBinding==NULL);

  if (!ask) 
    goto exit;
  if(!c->isManager()) {
    Assert(c->isFrame());
    PD((CELL,"Sending to mgr read"));
    BorrowEntry *be=BT->getBorrow(index);
    be->getOneMsgCredit();
    cellSendRead(be,mySite);
    goto exit;
  }
  PD((CELL,"ShortCircuit mgr sending to tokenholder"));
  if(((CellManager*)c)->getChain()->getCurrent() == mySite){
    return fea ? cellAtExchange(c,val,fea):
      cellDoExchange(c,val,val);
  }
  
  sendPrepOwner(index);
  cellSendRemoteRead(((CellManager*)c)->getChain()->getCurrent(),
		     mySite,index,mySite);
exit:
  Thread* th=oz_currentThread();
  ControlVarNew(controlvar,c->getBoardInternal());
  pendBinding=new PendThread(th,pendBinding,val,fea,
			     controlvar,fea ? DEEPAT : ACCESS);

  if(!c->errorIgnore()) {
    if(maybeInvokeHandler(c,th)){// ERROR-HOOK
      genInvokeHandlerLockOrCell(c,th);}}
  SuspendOnControlVar;
}


static
OZ_Return cellDoAccess(Tertiary *c,TaggedRef val,TaggedRef fea){
  if(c->isProxy()){
    convertCellProxyToFrame(c);}
  return getCellSecFromTert(c)->access(c,val,fea);}

OZ_Return cellAtAccess(Tertiary *c, TaggedRef fea, TaggedRef val){
  return cellDoAccess(c,val,fea);}

OZ_Return cellDoAccess(Tertiary *c, TaggedRef val){
  if(oz_onToplevel() && c->handlerExists(oz_currentThread()))
    return cellDoExchange(c,val,val);
  else
    return cellDoAccess(c,val,0);}


/**********************************************************************/
/*   SECTION 31a:: ChainElem routines                                  */
/**********************************************************************/

void ChainElem::init(Site *s){
  next=NULL;
  flags=0;
  site=s;}

/**********************************************************************/
/*   SECTION 31b:: InformElem routines                                  */
/**********************************************************************/
    
void InformElem::init(Site*s,EntityCond c){
  site=s;
  next=NULL;
  watchcond=c;
  foundcond=0;}

/**********************************************************************/
/*   SECTION 31c:: Chain routines working on ChainElem                */
/**********************************************************************/

Bool Chain::basicSiteExists(ChainElem *ce,Site* s){
  while(ce!=NULL){
    if(ce->site==s) {return OK;}
    ce=ce->next;}
  return NO;}

ChainElem** Chain::getFirstNonGhostBase(){
  if(first==last) {return &first;}
  ChainElem **ce=&first;
  while((*ce)->next->flagIsSet(CHAIN_GHOST)){
    ce= &((*ce)->next);}
  return ce;}

ChainElem* Chain::getFirstNonGhost(){
  return *getFirstNonGhostBase();}

Bool Chain::siteExists(Site *s){
  return basicSiteExists(getFirstNonGhost(),s);}

Bool Chain::siteOrGhostExists(Site *s){
  return basicSiteExists(first,s);}

void Chain::makeGhost(ChainElem* ce){
  ce->setFlagAndCheck(CHAIN_GHOST);
  ce->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
  if(hasFlag(INTERESTED_IN_TEMP)){
    deinstallProbe(ce->site,PROBE_TYPE_ALL);}
  else{
    deinstallProbe(ce->site,PROBE_TYPE_PERM);}}

void Chain::removeBefore(Site* s){
  ChainElem **base,*ce;
  base=getFirstNonGhostBase();
  Assert(siteExists(s));
  while(((*base)->site!=s) || (*base)->flagIsSet(CHAIN_DUPLICATE)){
    if((*base)->flagIsSet(CHAIN_QUESTION_ASKED)){
      makeGhost(*base);
      base=&((*base)->next);}
    else{
      removeNextChainElem(base);}}}

ChainElem *Chain::findAfter(Site *s){
  Assert(siteExists(s));
  if(first->next==NULL){
    return NULL;}
  ChainElem *ce=getFirstNonGhost();
  while(ce->site!=s){
    ce=ce->next;}
  return ce->next;}

ChainElem *Chain::findChainElemFrom(ChainElem *ce,Site*s){// ATTENTION
  while(TRUE){
    if(ce->site==s) return ce;
    ce=ce->next;
    if(ce==NULL) return NULL;}}

Bool Chain::removeGhost(Site* s){
  ChainElem **ce=&first;
  while(TRUE){
    if((*ce)==NULL) return NO;
    if(!(*ce)->flagIsSet(CHAIN_GHOST)) return NO;
    if((*ce)->site==s) {
      removeNextChainElem(ce);
      return OK;}
    ce = &((*ce)->next);}}

void Chain::probeTemp(Tertiary* t){
  PD((CHAIN,"Probing Temp"));
  ChainElem *ce=getFirstNonGhost();
  while(ce!=NULL){
    tertiaryInstallProbe(ce->site,PROBE_TYPE_ALL,t); 
    deinstallProbe(ce->site,PROBE_TYPE_PERM);
    ce=ce->next;}}

void Chain::deProbeTemp(){
  PD((CHAIN,"DeProbing Temp"));
  ChainElem *ce=getFirstNonGhost();
  while(ce!=NULL){
    installProbe(ce->site,PROBE_TYPE_PERM);
    deinstallProbe(ce->site,PROBE_TYPE_ALL);
    ce=ce->next;}}

Bool Chain::tempConnectionInChain(){
  ChainElem *ce=first;
  while(ce!=NULL){
    if(ce->site->siteStatus()==SITE_TEMP) return OK;
    ce=ce->next;}
  return NO;}

/**********************************************************************/
/*   SECTION 31c:: Chain routines working on InformElem                */
/**********************************************************************/

void Chain::removeInformOnPerm(Site *s){ 
  InformElem **ce= &inform;
  InformElem *tmp;
  while(*ce!=NULL){
    if((*ce)->site==s){
      tmp=*ce;
      releaseInformElem(tmp);
      *ce=tmp->next;
      return;}
    ce= &((*ce)->next);}}

/**********************************************************************/
/*   SECTION 32:: chain protocol                                      */
/**********************************************************************/

void chainSendAck(Site* toS, int mI){
  if(SEND_SHORT(toS)) {return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  PD((CHAIN,"M_CHAIN_ACK indx:%d site:%s",mI,toS->stringrep()));
  marshal_M_CHAIN_ACK(bs,mI,mySite);
  SendTo(toS,bs,M_CHAIN_ACK,toS,mI);}

void chainReceiveAck(OwnerEntry* oe,Site* rsite){
  Tertiary *t=oe->getTertiary();
  Chain* chain=tertiaryGetChain(t);
  if(!(chain->siteExists(rsite))) {
    return;}
  chain->removeBefore(rsite);
  PD((CHAIN,"%d",printChain(chain)));
}

  ChainAnswer answerChainQuestion(Tertiary *t){
  if(t->isProxy()){
    return PAST_ME;}
  switch(getStateFromLockOrCell(t)){
  case Cell_Lock_Invalid:
    return PAST_ME;
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:
    return BEFORE_ME;
  case Cell_Lock_Valid|Cell_Lock_Next: 
  case Cell_Lock_Valid: 
    return AT_ME;
  default: 
    Assert(0);}
  Assert(0);
  return BEFORE_ME;}

void chainReceiveQuestion(BorrowEntry *be,Site* site,int OTI,Site* deadS){
  if(be==NULL){
    chainSendAnswer(be,site,OTI,PAST_ME,deadS);}
  chainSendAnswer(be,site,OTI,answerChainQuestion(be->getTertiary()),deadS);}

void chainReceiveAnswer(OwnerEntry* oe,Site* fS,int ans,Site* deadS){
  Tertiary* t=oe->getTertiary();
  getChainFromTertiary(t)->receiveAnswer(t,fS,ans,deadS);
  PD((CHAIN,"%d",printChain(getChainFromTertiary(t))));
}

inline void maybeChainSendQuestion(ChainElem *ce,Tertiary *t,Site* deadS){
  if(ce->getSite()!=mySite){
    if(!(ce->flagIsSet(CHAIN_QUESTION_ASKED))){
      ce->setFlagAndCheck(CHAIN_QUESTION_ASKED);
      chainSendQuestion(ce->getSite(),t->getIndex(),deadS);}
    return;}
  getChainFromTertiary(t)->receiveAnswer(t,mySite,answerChainQuestion(t),deadS);}

void Chain::receiveAnswer(Tertiary* t,Site* site,int ans,Site* deadS){
  PD((ERROR_DET,"chain receive answer %d",ans));
  if(hasFlag(TOKEN_LOST)) return; 
  if(removeGhost(site)) return; 
  Assert(siteExists(site));
  ChainElem **base=getFirstNonGhostBase();
  ChainElem *dead,*answer;

  while(((*base)->site!=deadS) && ((*base)->site!=site)){
    base= &((*base)->next);}
  if((*base)->site==site){ //      order Answer-Dead
    PD((ERROR_DET,"chain receive answer - order answer-dead"));
    answer=*base;
    answer->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
    dead=answer->next;
    if(dead->site!=deadS) {
      PD((ERROR_DET,"dead->site!=deadS"));
      Assert(answer==getFirstNonGhost());
      return;}
    if(answer->flagIsSet(CHAIN_DUPLICATE)){
      PD((ERROR_DET,"answer->flagIsSet(CHAIN_DUPLICATE)"));
      dead->setFlagAndCheck(CHAIN_PAST);
      managerSeesSitePerm(t,deadS);
      return;}
    if(ans==PAST_ME){
      PD((ERROR_DET,"ans==PAST_ME"));
      dead->setFlagAndCheck(CHAIN_PAST);
      managerSeesSitePerm(t,deadS);      
      return;}
    PD((ERROR_DET,"Manager will receive CANT_PUT from %s",site->stringrep())); 
    dead->setFlagAndCheck(CHAIN_CANT_PUT);
    dead->resetFlag(CHAIN_BEFORE);
    dead->resetFlag(CHAIN_PAST);
    PD((CHAIN,"%d",printChain(this)));
    return;}
  PD((ERROR_DET,"chain receive answer - order dead-answer"));
  dead= *base;                      // order Dead-Answer
  answer=dead->next;
  Assert(answer->site=site);
  Assert(ans==BEFORE_ME);
  answer->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
  dead->setFlagAndCheck(CHAIN_BEFORE);
  managerSeesSitePerm(t,deadS);
  return;}

void chainSendQuestion(Site* toS,int mI,Site *deadS){
  OT->getOwner(mI)->getOneCreditOwner();
  PD((ERROR_DET,"chainSendQuestion  %s",toS->stringrep()));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CHAIN_QUESTION(bs,mI,mySite,deadS);
  SendTo(toS,bs,M_CHAIN_QUESTION,mySite,mI);}

void chainSendAnswer(BorrowEntry* be,Site* toS, int mI, int ans, Site *deadS){
  be->getOneMsgCredit();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CHAIN_ANSWER(bs,mI,mySite,ans,deadS);
  SendTo(toS,bs,M_CHAIN_ANSWER,toS,mI);}

/**********************************************************************/
/*   SECTION 33:: Lock protocol - receive                             */
/**********************************************************************/

Bool LockSec::secReceiveToken(Tertiary* t,Site* &toS){
  if(state & Cell_Lock_Next) state = Cell_Lock_Next|Cell_Lock_Valid;
  else state=Cell_Lock_Valid;
  if(pending->thread!=DummyThread){
    locker=pendThreadResumeFirst(&pending);
    return OK;}
  pendThreadRemoveFirst(getPendBase());
  if(pending==NULL){
    locker=NULL;
    if(state!=Cell_Lock_Valid|Cell_Lock_Next) return OK;
    toS=next;
    return OK;}
  unlockComplex(t);
  return OK;}

Bool LockSec::secForward(Site* toS){
  if(state==Cell_Lock_Valid){
    if(locker==NULL){
      state=Cell_Lock_Invalid;
      return OK;}
    state=Cell_Lock_Valid|Cell_Lock_Next;
    next=toS;
    return NO;}
  Assert(state==Cell_Lock_Requested);
  state= Cell_Lock_Requested|Cell_Lock_Next;
  next=toS;
  return NO;}

void lockReceiveGet(OwnerEntry* oe,LockManager* lm,Site* toS){  
  Assert(lm->getType()==Co_Lock);
  Assert(lm->isManager());
  Chain *ch=lm->getChain();
  PD((LOCK,"LockMgr Received get from %s",toS->stringrep()));
  Site* current=ch->setCurrent(toS,lm);            
  PD((CHAIN,"%d",printChain(ch)));
  if(current==mySite){                             // shortcut
    PD((LOCK," shortcut in lockReceiveGet"));
    TaggedRef val;
    if(lm->getSec()->secForward(toS)){
      oe->getOneCreditOwner();
      lockSendToken(mySite,lm->getIndex(),toS);}
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,lm->getIndex());}

void lockReceiveDump(LockManager* lm,Site *fromS){
  Assert(lm->getType()==Co_Lock);
  Assert(lm->isManager());
  LockSec* sec=lm->getSec();
  if((lm->getChain()->getCurrent()!=fromS) || (sec->getState()!=Cell_Lock_Invalid)){
    PD((WEIRD,"WEIRD- LOCK dump not needed"));
    return;}
  Assert(sec->getState()==Cell_Lock_Invalid);
  sec->lockComplex(DummyThread,lm);
  return;}

void lockReceiveTokenManager(OwnerEntry* oe,int mI){
  Tertiary *t=oe->getTertiary();
  Assert(t->getType()==Co_Lock);
  Assert(t->isManager());
  if(tokenLostCheckManager(t)) return; // ERROR-HOOK ATTENTION
  LockManager*lm=(LockManager*)t;
  chainReceiveAck(oe,mySite);
  LockSec *sec=lm->getSec();  
  Site* toS;
  if(sec->secReceiveToken(t,toS)) return;
  PD((CHAIN,"%d",printChain(lm->getChain())));
  oe->getOneCreditOwner();
  lockSendToken(mySite,mI,toS);}
  
void lockReceiveTokenFrame(BorrowEntry* be, Site *mS,int mI){
  LockFrame *lf=(LockFrame*) be->getTertiary();
  Assert(lf->getType()==Co_Lock);
  Assert(lf->isFrame());
  if(tokenLostCheckProxy(lf)) return; // ERROR-HOOK ATTENTION
  be->getOneMsgCredit();
  chainSendAck(mS,mI);
  LockSec *sec=lf->getSec();
  Site* toS;
  if(sec->secReceiveToken(lf,toS)) return;
  be->getOneMsgCredit();
  lockSendToken(mS,mI,toS);}
  
void lockReceiveForward(BorrowEntry *be,Site *toS,Site* mS,int mI){
  LockFrame *lf= (LockFrame*) be->getTertiary();
  lf->resetDumpBit();
  Assert(lf->isFrame());
  Assert(lf->getType()==Co_Lock);
  LockSec* sec=lf->getSec();
  if(!sec->secForward(toS)) return;
  be->getOneMsgCredit();
  lockSendToken(mS,mI,toS);}

/**********************************************************************/
/*   SECTION 34:: Lock protocol - send                                */
/**********************************************************************/

void lockSendToken(Site *mS,int mI,Site* toS){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_LOCK_TOKEN(bs,mS,mI);
  SendTo(toS,bs,M_LOCK_TOKEN,mS,mI);}

/**********************************************************************/
/*   SECTION 35:: Lock protocol - basics                             */
/**********************************************************************/

void LockProxy::lock(Thread *t){
  PD((LOCK,"convertToFrame %s-%d",
      BT->getOriginSite(getIndex())->stringrep(),
      BT->getOriginIndex(getIndex())));
  convertLockProxyToFrame(this);
  ((LockFrame*)this)->lock(t);}

void secLockToNext(LockSec* sec,Tertiary* t,Site* toS){
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
  lockSendToken(mySite,index,toS);}

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
  Site* current=ch->setCurrent(mySite,t);
  oe->getOneCreditOwner();
  cellLockSendForward(current,mySite,index);
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
    if(t->errorIgnore()) return; 
    break;}
  case Cell_Lock_Next|Cell_Lock_Requested:
  case Cell_Lock_Requested:{
    (void) pendThreadAddToEnd(getPendBase(),th,home);
    if(t->errorIgnore()) return;
    break;}
  case Cell_Lock_Invalid:{
    (void) pendThreadAddToEnd(getPendBase(),th,home);
    secLockGet(this,t,th);
    if(t->errorIgnore()) return;
    break;}
  default: Assert(0);}
  if(maybeInvokeHandler(t,th)){// ERROR-HOOK
    genInvokeHandlerLockOrCell(t,th);
  }}
    
void LockLocal::unlockComplex(){
  setLocker(pendThreadResumeFirst(&pending));
  return;}

void LockLocal::lockComplex(Thread *t){
  (void) pendThreadAddToEnd(getPendBase(),t,getBoardInternal());}

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
/*   SECTION 36:: error msgs                                         */
/**********************************************************************/

Bool CellSec::cellRecovery(TaggedRef tr){
  if(state==Cell_Lock_Invalid){
    state=Cell_Lock_Valid;
    contents=tr;
    return NO;}
  Assert(state==Cell_Lock_Requested);
  return OK;}

Bool LockSec::lockRecovery(){
  if(state==Cell_Lock_Invalid){
    state=Cell_Lock_Valid;
    locker=NULL;
    return NO;}
  state &= ~Cell_Lock_Next;
  Assert(state==Cell_Lock_Requested);
  return OK;}

void cellManagerIsDown(TaggedRef tr,Site* mS,int mI){
  NetAddress na=NetAddress(mS,mI);
  BorrowEntry *be=BT->find(&na);
  if(be==NULL){return;}
  Tertiary* t=be->getTertiary();
  maybeConvertCellProxyToFrame(t);
  if(((CellFrame*)t)->getSec()->cellRecovery(tr)){
    cellReceiveContentsFrame(be,tr,mS,mI);}}

void lockManagerIsDown(Site* mS,int mI){
  NetAddress na=NetAddress(mS,mI);
  BorrowEntry *be=BT->find(&na);
  if(be==NULL) {return;} // has been gced 
  Tertiary* t=be->getTertiary();
  maybeConvertLockProxyToFrame(t);
  if(((LockFrame*)t)->getSec()->lockRecovery()){  
    lockReceiveTokenFrame(be,mS,mI);}}

void cellSendCantPut(TaggedRef tr,Site* toS, Site *mS, int mI){
  PD((ERROR_DET,"Proxy cant put to %s site: %s:%d",
      toS->stringrep(), mS->stringrep(),mI));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(mS);
  marshal_M_CELL_CANTPUT(bs, mI, toS, tr, mySite);
  SendTo(mS,bs,M_CELL_CANTPUT,mS,mI);}

void cellSendContentsFailure(TaggedRef tr,Site* toS,Site *mS, int mI){ 
  if(toS==mS) {// ManagerSite is down
    cellManagerIsDown(tr,toS,mI);
    return;}
  if(mS==mySite){// At managerSite 
    cellReceiveCantPut(OT->getOwner(mI),tr,mI,mS,toS);
    return;}  
  cellSendCantPut(tr,toS,mS,mI);
  return;}

void lockSendCantPut(Site* toS, Site *mS, int mI){
  PD((ERROR_DET,"Proxy cant put - to %s site: %s:%d Nr %d",
      toS->stringrep(),mS->stringrep(),mI));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(mS);
  marshal_M_LOCK_CANTPUT(bs, mI, toS, mySite);
  SendTo(mS,bs,M_LOCK_CANTPUT,mS,mI);
  return;}

void lockSendTokenFailure(Site* toS,Site *mS, int mI){ 
  PD((ERROR_DET,"LockTokenFailure"));
  if(toS==mS) {// ManagerSite is down
    lockManagerIsDown(mS,mI);
    return;}
  if(mS==mySite){// At managerSite 
    lockReceiveCantPut(OT->getOwner(mI),mI,mS,toS);
    return;}  
  lockSendCantPut(toS,mS,mI);
  return;}

void lockReceiveCantPut(OwnerEntry *oe,int mI,Site* rsite, Site* bad){ 
  LockManager* lm=(LockManager*)oe->getTertiary();
  Assert(lm->getType()==Co_Lock);
  Assert(lm->isManager());
  PD((ERROR_DET,"Proxy cant Put"));
  Chain *ch=lm->getChain();
  ch->removeBefore(bad);
  ch->shortcutCrashLock(lm);
  PD((CHAIN,"%d",printChain(ch)));
}

void cellReceiveCantPut(OwnerEntry* oe,TaggedRef val,int mI,Site* rsite,
			Site* badS){ 
  CellManager* cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  PD((ERROR_DET,"Proxy cant Put"));
  Chain *ch=cm->getChain();
  ch->removeBefore(badS);
  ch->shortcutCrashCell(cm,val);
  PD((CHAIN,"%d",printChain(ch)));
}

void sendAskError(Tertiary *t,EntityCond ec){ // caused by installing handler/watcher
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  be->getOneMsgCredit();
  marshal_M_ASK_ERROR(bs,na->index,mySite,ec);
  SendTo(na->site,bs,M_ASK_ERROR,na->site,na->index);}

void sendUnAskError(Tertiary *t,EntityCond ec){ // caused by deinstalling handler/watcher
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  be->getOneMsgCredit();
  marshal_M_UNASK_ERROR(bs,na->index,mySite,ec);
  SendTo(na->site,bs,M_UNASK_ERROR,na->site,na->index);}

                                   // caused by installing remote watcher/handler
void receiveAskError(OwnerEntry *oe,Site *toS,EntityCond ec){  
  PD((NET_HANDLER,"Ask Error Received"));
  Tertiary *t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell: break;
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  Chain *ch=getChainFromTertiary(t);
  if(ch->hasFlag(TOKEN_LOST)){
    PD((NET_HANDLER,"Token Lost"));
    EntityCond tmp=ec & (PERM_SOME|PERM_ME);
    if(tmp != ENTITY_NORMAL){
      sendTellError(oe,toS,t->getIndex(),tmp,true);
      return;}
    ch->newInform(toS,ec);
    ch->dealWithTokenLostBySite(oe,t->getIndex(),toS);
    return;}
  Assert(!(ec & TEMP_ALL));
  if((ch->hasFlag(TOKEN_PERM_SOME)) && (ec & PERM_SOME)){
    PD((NET_HANDLER,"State and q match PERM_SOME"));
    sendTellError(oe,toS,t->getIndex(),PERM_SOME,TRUE);
    return;}
  ch->newInform(toS,ec);
  PD((NET_HANDLER,"Adding Inform Element"));
  if(someTempCondition(ec)){
    if(!ch->hasFlag(INTERESTED_IN_TEMP)){
      ch->setFlagAndCheck(INTERESTED_IN_TEMP);
      ch->probeTemp(t);
      return;}
    else   PD((NET_HANDLER,"Tmp q; allredy interested in that"));
    if(ch->hasFlag(INTERESTED_IN_OK)){
      PD((NET_HANDLER,"Manager is in TmpCond"));
      EntityCond ecS = (ch->getInform())->wouldTrigger(TEMP_SOME|TEMP_ME|TEMP_BLOCKED);
      PD((NET_HANDLER,"ecS %d",ecS));
      if(ecS !=ENTITY_NORMAL)
	sendTellError(oe,toS,t->getIndex(),
		      ecS,TRUE);}}}

  
void Chain::receiveUnAsk(Site* s,EntityCond ec){
  InformElem **ie=&inform;
  InformElem *tmp;
  while(*ie!=NULL){
    if(((*ie)->site==s) && ((*ie)->watchcond==ec)){
      tmp=*ie;
      *ie=tmp->next;
      releaseInformElem(tmp);
      break;}
    ie=&((*ie)->next);}
  PD((WEIRD,"unaskerror with no error"));
  return;}

void receiveUnAskError(OwnerEntry *oe,Site *toS,EntityCond ec){ 
  Tertiary* t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  getChainFromTertiary(t)->receiveUnAsk(toS,ec);}

/**********************************************************************/
/*   SECTION 37:: handlers/watchers                                   */
/**********************************************************************/

Watcher** Tertiary::findWatcherBase(Thread* th,EntityCond ec){
  Watcher** def = NULL;
  Watcher** base=getWatcherBase();
  while(*base!=NULL){
    if(((*base)->isHandler()) && ((*base)->isTriggered(ec))){
      if((*base)->thread==th)
	return base;
      if((*base)->thread==DefaultThread) 
	def = base;}
    base= &((*base)->next);}
  return def;}

Bool Tertiary::handlerExists(Thread *t){
  Bool foundD = NO;
  if(info==NULL) return NO;
  Watcher *w=info->watchers;
  while(w!=NULL){
    if(w->isHandler()){
      if(w->getThread()==t) return OK;
      if(w->getThread()==DefaultThread) foundD = OK;}
    w=w->next;}
  return foundD;}

Bool Tertiary::handlerExistsThread(Thread *t){
  if(info==NULL) return NO;
  Watcher *w=info->watchers;
  while(w!=NULL){
    if(w->isHandler())
      if(w->getThread()==t) return OK;
    w=w->next;}
  return NO;}

void Tertiary::insertWatcher(Watcher *w){
  if(info==NULL){
    info=new EntityInfo(w);
    return;}
  w->next=info->watchers;
  info->watchers=w;}

inline Site* getSiteFromTertiaryProxy(Tertiary* t){
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  Assert(be!=NULL);
  return be->getNetAddress()->site;}

EntityCond getEntityCondPort(Tertiary* p){
  EntityCond ec = p->getEntityCond();
  int dummy;
  if(ec!=ENTITY_NORMAL)return ec;
  if(getSiteFromTertiaryProxy(p)->getQueueStatus(dummy)>=PortSendTreash)
    return TEMP_BLOCKED|TEMP_ME;
  return ENTITY_NORMAL;}

void informInstallHandler(Tertiary* t,EntityCond ec){
  switch(t->getType()){
  case Co_Cell: 
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  if(t->isManager()){
    Chain *ch=getChainFromTertiary(t);
    ch->newInform(mySite,ec);
    if(someTempCondition(ec) && !ch->hasFlag(INTERESTED_IN_TEMP)){ 
      ch->setFlagAndCheck(INTERESTED_IN_TEMP);
      ch->probeTemp(t);}
    return;}
  sendAskError(t,managerPart(ec));
  if(someTempCondition(ec)) 
    tertiaryInstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_ALL,t); 
  else
    tertiaryInstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_PERM,t); }

Bool Tertiary::installHandler(EntityCond wc,TaggedRef proc,Thread* th, Bool Continue, Bool pr){
  if(handlerExistsThread(th)){return FALSE;} // duplicate
  PD((NET_HANDLER,"Handler installed on tertiary:%x",this));
  Watcher *w=new Watcher(proc,th,wc);
  insertWatcher(w);
  if(pr)w->setPersistent();
  if(Continue) w->setContinueHandler();
  if(this->isLocal()){
    return TRUE;}
  if(wc & TEMP_BLOCKED && (getType()!=Co_Port)){
    Assert(wc==TEMP_BLOCKED|PERM_BLOCKED);
    informInstallHandler(this,wc);
    return TRUE;}
  if(isManager()) return TRUE;
  tertiaryInstallProbe(getSiteFromTertiaryProxy(this),PROBE_TYPE_PERM,this);
  return TRUE;}

Bool Tertiary::deinstallHandler(Thread *th,TaggedRef proc){
  if(!handlerExistsThread(th)){return NO;}
  PD((NET_HANDLER,"Handler deinstalled on tertiary %x",this));  
  EntityCond Mec=(TEMP_BLOCKED|TEMP_ME|TEMP_SOME);
  Watcher** base=getWatcherBase();
  Bool found = FALSE;

  while(*base != NULL)
    if(((Watcher*)*base)->isHandler() &&
       ((Watcher*)*base)->thread==th &&
       (((Watcher*)*base)->proc == proc || proc == AtomAny)){
      releaseWatcher(((Watcher*)*base));
      Assert(found == FALSE);
      found = TRUE;
      *base = (*base)->next;}
    else{
      Mec &= ~(((Watcher*)*base)->getWatchCond() & (TEMP_BLOCKED|TEMP_ME|TEMP_SOME));
      base = &((*base)->next);} 
  resetEntityCondManager(Mec);
  return found;}

void Tertiary::installWatcher(EntityCond wc,TaggedRef proc, Bool pr){
  PD((NET_HANDLER,"Watcher installed on tertiary %x",this));
  Watcher *w=new Watcher(proc,wc);
  insertWatcher(w);
  if(pr) w->setPersistent();
  if(isLocal()) return;
  if(w->isTriggered(getEntityCond()) || 
     (getType()==Co_Port && w->isTriggered(getEntityCondPort(this)))){
    entityProblem();
    return;}
  if(managerPart(wc) != ENTITY_NORMAL && getType()!=Co_Port){
    informInstallHandler(this,managerPart(wc));
    return;}
  if(this->isManager()){
    return;}
  if(someTempCondition(wc) && getType()!=Co_Port)
    tertiaryInstallProbe(getSiteFromTertiaryProxy(this),PROBE_TYPE_ALL,this);
  else 
    tertiaryInstallProbe(getSiteFromTertiaryProxy(this),PROBE_TYPE_PERM,this);}

Bool Tertiary::deinstallWatcher(EntityCond wc, TaggedRef proc){
  Watcher** base=getWatcherBase();
  EntityCond Mec=(TEMP_BLOCKED|TEMP_ME|TEMP_SOME);
  Bool found = FALSE;
  while(*base!=NULL){
    if((!((*base)->isHandler())) && 
       ((*base)->getWatchCond() == wc) && 
       (((*base)->proc==proc) || proc==AtomAny || proc==AtomAll)){
      releaseWatcher((*base));
      *base = (*base)->next;
      found = TRUE;
      if(proc == AtomAny) proc = 0;}
    else{ 
      Mec &= ~(((Watcher*)*base)->getWatchCond() & (TEMP_BLOCKED|TEMP_ME|TEMP_SOME));
      base= &((*base)->next);}}
  resetEntityCondManager(Mec);
  return found;}
  
void Watcher::invokeHandler(EntityCond ec,Tertiary* entity, 
			    Thread * th, TaggedRef controlvar)
{
  Assert(isHandler());
  TaggedRef arg0 = makeTaggedTert(entity);
  TaggedRef arg1 = (ec & PERM_BLOCKED)?AtomPermBlocked:AtomTempBlocked;
  if(entity->getType()==Co_Port) {
    am.prepareCall(proc,arg0,arg1);
  } else {
    Assert(th!=oz_currentThread()); 
    th->pushCall(proc,arg0,arg1);
    ControlVarResume(controlvar);
  }
}


void Watcher::invokeWatcher(EntityCond ec,Tertiary* entity){
  Assert(!isHandler());
  Thread *tt = oz_newThreadToplevel(DEFAULT_PRIORITY);
  tt->pushCall(proc, makeTaggedTert(entity), listifyWatcherCond(ec));
}

Bool CellSec::threadIsPending(Thread *t){
  return basicThreadIsPending(pending,t);}

Bool LockSec::threadIsPending(Thread *t){
  return basicThreadIsPending(pending,t);}

Bool Tertiary::threadIsPending(Thread* th){
  switch(getType()){
  case Co_Cell: {
    if(isProxy()) {return NO;}
    if(isFrame()){
      return ((CellFrame*)this)->getSec()->threadIsPending(th);}
    Assert(isManager());
    return ((CellManager*)this)->getSec()->threadIsPending(th);}
  case Co_Lock:{
  if(isProxy()) {return NO;}
    if(isFrame()){
      return ((LockFrame*)this)->getSec()->threadIsPending(th);}
    Assert(isManager());
  return ((LockManager*)this)->getSec()->threadIsPending(th);}
  case Co_Port:{
    Assert(isProxy());
    return ((PortProxy*)this)->threadIsPending(th);}
  default:
    NOT_IMPLEMENTED;}
  return NO;}

  
/**********************************************************************/
/*   SECTION 38:: error                                               */
/**********************************************************************/

void sendTellError(OwnerEntry *oe,Site* toS,int mI,EntityCond ec,Bool set){
  if(toS==mySite){
    receiveTellError(oe->getTertiary(),mySite,mI,ec,set);
    return;}
  if(SEND_SHORT(toS)) {return;}
  oe->getOneCreditOwner();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_TELL_ERROR(bs,mySite,mI,ec,set);
  SendTo(toS,bs,M_TELL_ERROR,mySite,mI);}

void receiveTellError(Tertiary *t,Site* mS,int mI,EntityCond ec,Bool set){
  if(set){
    if(t->setEntityCondManager(ec)){
      t->entityProblem();}
    return;}
  t->resetEntityCondManager(ec);}

void Tertiary::releaseWatcher(Watcher* w){
  if(!isManager()){
    EntityCond ec=managerPart(w->watchcond);
    switch(getType()){
    case Co_Cell:
    case Co_Lock: {
      ec &= ~(PERM_BLOCKED|PERM_SOME|PERM_ME); // Automatic
      break;}
    case Co_Port:{
      deinstallProbe(getSiteFromTertiaryProxy(this),PROBE_TYPE_PERM);
      return;}
    default: NOT_IMPLEMENTED;}
    Assert(getType()!=Co_Port);
    if(ec!=ENTITY_NORMAL) {
      sendUnAskError(this,managerPart(w->watchcond));}
    if(someTempCondition(w->watchcond))
      deinstallProbe(getSiteFromTertiaryProxy(this),PROBE_TYPE_ALL);
    else 
      deinstallProbe(getSiteFromTertiaryProxy(this),PROBE_TYPE_PERM);
  }}

 
void Tertiary::entityProblem(){ 

  PD((ERROR_DET,"entityProblem invoked"));
  
  EntityCond ec=getEntityCond();
  Watcher** base=getWatcherBase();
  
  if(errorIgnore()) return;
  if(*base==NULL) return;
  
  Tertiary *other = NULL, *obj =getInfoTert() ;
  if(obj){
    other = getOtherTertFromObj(obj,this);
    Assert(obj->getType() == Co_Object);}
  PendThread *pd;

  if(isProxy())
    pd = NULL;
  else{
    if(getType()==Co_Cell){
      if(isFrame())
	pd = *(((CellFrame *)this)->getSec()->getPendBase());
      else
	pd = *(((CellManager *)this)->getSec()->getPendBase());}
    if(getType()==Co_Lock){
      if(isFrame())
	pd = *(((LockFrame *)this)->getSec()->getPendBase());
      else
	pd = *(((LockManager *)this)->getSec()->getPendBase());}}
  
  while(pd!=NULL){
    if(isRealThread(pd->thread)){
    Watcher **ww = findWatcherBase(pd->thread,ec);
    Thread *cThread = pd->thread;
    if(ww!=NULL){
      Watcher *w = *ww;
      Assert(!isProxy());
      pd->thread = DummyThread;
      if(w->isContinueHandler()){
	Assert(cThread!=oz_currentThread());
	if(getType()==Co_Cell){
	  switch(pd->exKind){
	  case EXCHANGE:{cThread->pushCall(BI_exchangeCell,makeTaggedTert(this), pd->old, pd->nw); break;}
	  case ASSIGN:{cThread->pushCall(BI_assign,pd->old,pd->nw); break;}
	  case AT:{cThread->pushCall(BI_atRedo,pd->old,pd->nw); break;}
	  default: Assert(0);}}
      if(getType()==Co_Lock)
	cThread->pushCall(BI_lockLock,makeTaggedTert(this));}
      
      if(obj)
	w->invokeHandler(ec,obj,cThread,pd->controlvar);
      else
	w->invokeHandler(ec,this,cThread,pd->controlvar);
      if(!w->isPersistent()){
	if(obj && other) other->deinstallHandler(cThread,AtomAny);
	*ww = w->next;
	releaseWatcher(w);}
    }}
    pd = pd->next;}
  
  EntityCond Mec=(TEMP_BLOCKED|TEMP_ME|TEMP_SOME);
  Watcher* w=*base;
  
  while(w!=NULL)
    if((!w->isTriggered(ec)) || (w->isHandler())){
      Mec &= ~(w->getWatchCond() & (TEMP_BLOCKED|TEMP_ME|TEMP_SOME));
      base= &(w->next);
      w=*base;}
    else{
      if(obj){
	w->invokeWatcher(ec,obj);
	if(other) other->deinstallWatcher(w->watchcond,w->proc);}
      else
	w->invokeWatcher(ec,this);
      if(w->isPersistent()){
	Mec &= ~(w->getWatchCond() & (TEMP_BLOCKED|TEMP_ME|TEMP_SOME));
	base= &(w->next);
	w=*base;} 
      else{  
	releaseWatcher(w);
	*base=w->next;
	w=*base;}}
    resetEntityCondManager(Mec);}
 
void Chain::informHandle(OwnerEntry* oe,int OTI,EntityCond ec){
  Assert(somePermCondition(ec));
  InformElem **base=&inform;
  InformElem *cur=*base;
  while(cur!=NULL){
    if(cur->watchcond & ec){
      sendTellError(oe,cur->site,OTI,cur->watchcond & ec,TRUE);
      *base=cur->next;
      freeInformElem(cur);
      cur=*base;
      continue;}
    base=&(cur->next);
    cur=*base;}}

void Chain::dealWithTokenLostBySite(OwnerEntry*oe,int OTI,Site *s){ // ATTENTION
  InformElem **base=&inform;
  InformElem *cur=*base;
  while(cur!=NULL){
    if((cur->site==s) & (cur->watchcond & PERM_BLOCKED)){
      Assert((cur->watchcond == PERM_BLOCKED) || (cur->watchcond == TEMP_BLOCKED|PERM_BLOCKED));
      sendTellError(oe,cur->site,OTI,PERM_BLOCKED,TRUE);
      *base=cur->next;
      freeInformElem(cur);
      cur=*base;}
    else{
      base=&(cur->next);
      cur=*base;}}}

void Chain::shortcutCrashLock(LockManager* lm){
  setFlag(TOKEN_PERM_SOME);
  int OTI=lm->getIndex();
  informHandle(OT->getOwner(OTI),OTI,PERM_SOME);
  lm->setEntityCondManager(PERM_SOME);
  lm->entityProblem(); 
  ChainElem** base=getFirstNonGhostBase();
  ChainElem *ce;
  LockSec* sec=lm->getSec();
  if((*base)->next==NULL){
    LockSec *sec=lm->getSec();
    ChainElem *ce=*base;
    ce->init(mySite);
    Assert(sec->state==Cell_Lock_Invalid);
    sec->state=Cell_Lock_Valid;
    return;}
  removePerm(base);
  ce=getFirstNonGhost();
  if(ce->site==mySite){
    lockReceiveTokenManager(OT->getOwner(OTI),OTI);
    return;}
  lockSendToken(mySite,OTI,ce->site);}

void Chain::shortcutCrashCell(CellManager* cm,TaggedRef val){
  setFlag(TOKEN_PERM_SOME);
  int OTI=cm->getIndex();
  informHandle(OT->getOwner(OTI),OTI,PERM_SOME);
  cm->setEntityCondManager(PERM_SOME);
  cm->entityProblem(); 
  ChainElem** base=getFirstNonGhostBase();
  ChainElem *ce;
  CellSec* sec=cm->getSec();
  if((*base)->next==NULL){
    CellSec *sec=cm->getSec();
    ChainElem *ce=*base;
    ce->init(mySite);
    Assert(sec->state=Cell_Lock_Invalid);
    sec->state=Cell_Lock_Valid;
    sec->contents=val;
    return;}
  removePerm(base);
  ce=getFirstNonGhost();
  int index=cm->getIndex();
  if(ce->site==mySite){
    cellReceiveContentsManager(OT->getOwner(index),val,index);
    return;}
  OT->getOwner(index)->getOneCreditOwner();
  cellSendContents(val,ce->site,mySite,index);}

void Chain::handleTokenLost(OwnerEntry *oe,int OTI){
  ChainElem *ce=first->next;
  ChainElem *back;
  Assert(first->site->siteStatus()==SITE_PERM);
  releaseChainElem(first);
  while(ce){
    if(!ce->flagIsSet(CHAIN_GHOST)){
      sendTellError(oe,ce->site,OTI,PERM_SOME|PERM_BLOCKED|PERM_ME,TRUE);}
    back=ce;
    ce=ce->next;
    releaseChainElem(back);}
  first=NULL;
  last=NULL;}

void Chain::managerSeesSitePerm(Tertiary *t,Site *s){
  PD((ERROR_DET,"managerSeesSitePerm site:%s nr:%d",s->stringrep(),t->getIndex()));
  PD((CHAIN,"%d",printChain(this)));
  removeGhost(s); // remove ghost if any
  if(!siteExists(s)) return;
  ChainElem **base=getFirstNonGhostBase();
  ChainElem *after,*dead,*before;
  if((*base)->site==s){
    PD((ERROR_DET,"managerSeesSitePerm - perm is first site"));
    dead=*base;
    after=dead->next;
    before=NULL;}
  else{
    PD((ERROR_DET,"managerSeesSitePerm - perm is not first site"));
    while((*base)->next->site!=s){base=&((*base)->next);}
    before=*base;
    dead=before->next;
    after=dead->next;}
  if(before==NULL){
    dead->setFlag(CHAIN_PAST);}
  else{
    if(before->site->siteStatus()==SITE_PERM){
      if(dead->flagIsSet(CHAIN_BEFORE)){
	before->setFlagAndCheck(CHAIN_BEFORE);}
      removePerm(&(before->next));
      managerSeesSitePerm(t,before->site);
      return;}}
  if(after==NULL){
    PD((ERROR_DET,"managerSeesSitePerm - perm is last site"));
    dead->setFlag(CHAIN_BEFORE);}
  else{
    PD((ERROR_DET,"managerSeesSitePerm - perm is not last site"));
    if(after->site->siteStatus()==SITE_PERM){
      removePerm(&(dead->next));
      managerSeesSitePerm(t,s);
      return;}}
  if(dead->flagIsSet(CHAIN_CANT_PUT)) return;
  if(!dead->flagIsSet(CHAIN_PAST)){
    maybeChainSendQuestion(before,t,s);
    return;}
  if(!dead->flagIsSet(CHAIN_BEFORE)){
    maybeChainSendQuestion(after,t,s);
    return;}
  PD((ERROR_DET,"managerSeesSitePerm - token lost (lock can recover"));
  if(before!=NULL) {
    removeBefore(dead->site);}
  if(t->getType()==Co_Lock){
    PD((ERROR_DET,"LockToken lost, now recreated"));
    ((LockManager*)t)->getChain()->shortcutCrashLock((LockManager*) t);
    return;}
  PD((ERROR_DET,"Token lost"));
  setFlagAndCheck(TOKEN_LOST);
  int OTI=t->getIndex();
  OwnerEntry *oe=OT->getOwner(OTI);
  informHandle(oe,OTI,PERM_SOME|PERM_ME);  
  t->setEntityCondManager(PERM_SOME|PERM_ME);
  t->entityProblem();
  handleTokenLost(oe,OTI);
  return;}
  
void Chain::managerSeesSiteTemp(Tertiary *t,Site *s){
  Assert(siteExists(s));
  Assert(hasFlag(INTERESTED_IN_TEMP));
  EntityCond ec;
  Bool change=NO;
  PD((ERROR_DET,"managerSeesSiteTemp site:%s nr:%d",s->stringrep(),t->getIndex()));
  int index;
  OwnerEntry *oe;
  InformElem *cur=inform;  // deal with TEMP_SOME|TEMP_ME watchers
  while(cur!=NULL){
    ec=cur->wouldTrigger(TEMP_SOME|TEMP_ME);
    if(ec != ENTITY_NORMAL && cur->site != s){
      change=OK;
      index=t->getIndex();
      sendTellError(OT->getOwner(index),cur->site,index,ec,TRUE);}
    cur=cur->next;}    
  ChainElem *ce=findAfter(s); // deal with TEMP_BLOCKED handlers
  while(ce!=NULL){
    cur=inform;
    while(cur!=NULL){
      if(cur->site!=s){
	ec=cur->wouldTrigger(TEMP_BLOCKED);
	if(ec!= ENTITY_NORMAL){
	  change=OK;
	  index=t->getIndex();
	  sendTellError(OT->getOwner(index),cur->site,index,ec,TRUE);}}
      cur=cur->next;}
    ce=ce->next;}
  setFlag(INTERESTED_IN_OK);}

void Chain::managerSeesSiteOK(Tertiary *t,Site *s){
  Assert(siteExists(s));
  Assert(hasFlag(INTERESTED_IN_OK));
  PD((ERROR_DET,"managerSeesSiteOK site:%s nr:%d",s->stringrep(),t->getIndex()));
  int index;
  Bool change=NO;
  OwnerEntry *oe;
  EntityCond ec;
  InformElem *cur;   
  ChainElem *ce=findAfter(s); // deal with TEMP_BLOCKED handlers
  while(ce!=NULL){
    cur=inform;
    while(cur!=NULL){
      if(cur->site==s){
	ec=cur->wouldTriggerOK(TEMP_BLOCKED);
	if(ec!= ENTITY_NORMAL){
	  change=OK;
	  index=t->getIndex();
	  sendTellError(OT->getOwner(index),cur->site,index,ec,FALSE);}}
      cur=cur->next;}
    ce=ce->next;}

  if(tempConnectionInChain()) return;
  
  cur=inform;  // deal with TEMP_SOME|TEMP_ME watchers
  while(cur!=NULL){
    ec=cur->wouldTriggerOK(TEMP_SOME|TEMP_ME);
    if(ec!=ENTITY_NORMAL){
      change=OK;
      index=t->getIndex();
      sendTellError(OT->getOwner(index),cur->site,index,ec,FALSE);}
    cur=cur->next;}    
  
  resetFlagAndCheck(INTERESTED_IN_OK);}

/**********************************************************************/
/*   SECTION 39:: probes                                            */
/**********************************************************************/

void cellLock_Perm(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    // ATTENTION PER I added Perm_block /EK
    if(t->setEntityCondOwn(PERM_SOME|PERM_ME|PERM_BLOCKED)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(t->setEntityCondOwn(PERM_SOME|PERM_BLOCKED|PERM_ME)) break;
               // ATTENTION note that we don't know if we'll be blocked maybe TEMP?
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(t->setEntityCondOwn(PERM_ALL|PERM_SOME)) break;
    return;}
  default: {
    Assert(0);}}
  t->entityProblem();}

void cellLock_Temp(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    if(t->setEntityCondOwn(TEMP_SOME|TEMP_ME)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(t->setEntityCondOwn(TEMP_SOME|TEMP_ME|TEMP_BLOCKED)) break;    
                   // ATTENTION: note that we don't know if we'll be blocked
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(t->setEntityCondOwn(TEMP_SOME|TEMP_ALL)) break;    
    return;}
  default: {
    Assert(0);}}
  t->entityProblem();}

void cellLock_OK(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    if(t->resetEntityCondProxy(TEMP_SOME|TEMP_ME)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(t->resetEntityCondProxy(TEMP_SOME|TEMP_ME|TEMP_BLOCKED)) break;
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(t->resetEntityCondProxy(TEMP_SOME|TEMP_ALL)) break;
    return;}
  default: {
    Assert(0);}}}

void Tertiary::managerProbeFault(Site *s, int pr){
  PD((ERROR_DET,"Mgr probe invoked %d",pr));    
  switch(getType()){
  case Co_Cell:
  case Co_Lock:{
    Chain *ch=getChainFromTertiary(this);
    if(pr==PROBE_OK){
      if(!ch->hasFlag(INTERESTED_IN_OK)) return;
      if(!ch->siteOfInterest(s)) return;
      ch->managerSeesSiteOK(this,s);
      return;}
    if(pr==PROBE_TEMP){
      if(!ch->hasFlag(INTERESTED_IN_TEMP)) return;
      if(!ch->siteOfInterest(s)) return;
      ch->managerSeesSiteTemp(this,s);
      return;}
    if(ch->hasInform()){
      ch->removeInformOnPerm(s);}
    if(!ch->siteOfInterest(s)) return;    
    ch->managerSeesSitePerm(this,s);}
  default: return;  // TO_BE_IMPLEMENTED
    return;}}

void Tertiary::proxyProbeFault(int pr){
  PD((ERROR_DET,"proxy probe invoked %d",pr));
  switch(getType()){
  case Co_Cell:
  case Co_Lock:{
    int state;
    if(isProxy()){
      state=Cell_Lock_Invalid;}
    else{
      state=getStateFromLockOrCell(this);}
    if(pr==PROBE_PERM){
      cellLock_Perm(state,this);
      return;}
    if(pr == PROBE_OK){
      cellLock_OK(state,this);
      return;}
    Assert(pr==PROBE_TEMP);
    cellLock_Temp(state,this);
    return;}
  case Co_Port:{
    Assert(pr==PROBE_PERM);
    setEntityCondOwn(PERM_BLOCKED|PERM_ME);
    startHandlerPort(NULL, this,0 , PERM_BLOCKED|PERM_ME);
  }
  default: return;}}     // TO_BE_IMPLEMENTED

void Site::probeFault(ProbeReturn pr){
  PD((PROBES,"PROBEfAULT  site:%s",stringrep()));
  int limit=OT->getSize();
  for(int ctr = 0; ctr<limit;ctr++){
    OwnerEntry *oe = OT->getEntry(ctr);
    if(oe==NULL){continue;}
    Assert(oe!=NULL);
    if(oe->isTertiary()){
      Tertiary *tr=oe->getTertiary();
      PD((PROBES,"Informing Manager"));
      Assert(tr->isManager());
      tr->managerProbeFault(this,pr);}} // TO_BE_IMPLEMENTED vars
  limit=BT->getSize();
  for(int ctr1 = 0; ctr1<limit;ctr1++){
    BorrowEntry *be = BT->getEntry(ctr1);
    if(be==NULL){continue;}
    Assert(be!=NULL);
    if(be->isTertiary()){
      Tertiary *tr=be->getTertiary();
      if((be->getSite() == this && tr->hasWatchers()) || 
	 tr->getEntityCond()!=ENTITY_NORMAL){
	tr->proxyProbeFault(pr);}}}
  return;}

/**********************************************************************/
/*   SECTION 39b:: ProbeHack                                       */
/**********************************************************************/

void insertDangelingEvent(Tertiary *t){
  PD((PROBES,"Starting DangelingThread"));
  Thread *tt = oz_newThreadToplevel(DEFAULT_PRIORITY);
  tt->pushCall(BI_probe, makeTaggedTert(t));
}

/**********************************************************************/
/*   SECTION 40:: communication problem                               */
/**********************************************************************/

inline void returnSendCredit(Site* s,int OTI){
  if(s==mySite){
    OT->getOwner(OTI)->receiveCredit(OTI);
    return;}
  sendCreditBack(s,OTI,1);}

enum CommCase{
    USUAL_OWNER_CASE,
    USUAL_BORROW_CASE
  };

#define ResetCP(buf,mt) {\
  buf->unmarshalReset();\
  MessageType mt1=unmarshalHeader(buf);\
  Assert(mt1==mt);}

void Site::communicationProblem(MessageType mt,Site* 
				storeSite,int storeIndex
				,FaultCode fc,FaultInfo fi){
  int OTI,Index;
  Site *s1,*s2;
  TaggedRef tr;
  CommCase flag;

  if (storeSite) {
  PD((SITE,"CommProb type:%d site:%s\n storeSite: %s \n indx:%d faultCode:%d",
      mt,this->stringrep(),storeSite->stringrep(), storeIndex, fc));
  }
  switch(mt){
    
  case M_PORT_SEND:{
    flag=USUAL_BORROW_CASE;
    break;}
  
  case M_REMOTE_SEND:{
    NOT_IMPLEMENTED;}
  
  case M_ASK_FOR_CREDIT:{
    flag=USUAL_BORROW_CASE;
    break;}

  case M_OWNER_CREDIT:{
      flag=USUAL_BORROW_CASE;
      break;}

  case M_OWNER_SEC_CREDIT:{
    flag=USUAL_BORROW_CASE;
    break;}

  case M_BORROW_CREDIT:{
    flag=USUAL_OWNER_CASE;
    break;}

    case M_REGISTER:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_REDIRECT:{
      if(fc==COMM_FAULT_PERM_NOT_SENT){
	ResetCP(((MsgBuffer*)fi),M_REDIRECT);
	unmarshal_M_REDIRECT((MsgBuffer*)fi,s1,OTI,tr);
	returnSendCredit(s1,OTI);
	return;}
      flag=USUAL_OWNER_CASE;
      break;}

    case M_ACKNOWLEDGE:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_SURRENDER:{
      if(fc==COMM_FAULT_PERM_NOT_SENT){
	ResetCP(((MsgBuffer*)fi),M_SURRENDER);
	unmarshal_M_SURRENDER((MsgBuffer*)fi,OTI,s1,tr);
	returnSendCredit(mySite,OTI);
	return;}
      flag=USUAL_OWNER_CASE;
      break;}

    case M_CELL_LOCK_GET:{
      flag=USUAL_BORROW_CASE;
      break;}

    case  M_CELL_LOCK_FORWARD:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_CELL_LOCK_DUMP:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_CELL_CONTENTS:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
	ResetCP(((MsgBuffer*)fi),M_CELL_CONTENTS);
	unmarshal_M_CELL_CONTENTS((MsgBuffer*)fi,s1,OTI,tr);
	Assert(s1==storeSite);
	Assert(OTI=storeIndex);
	returnSendCredit(s1,OTI);	  
	cellSendContentsFailure(tr,this,storeSite,OTI);
	return;}
      /*
	if(fc==COMM_FAULT_PERM_MAYBE_SENT){
	NOT_IMPLEMENTED;}
	*/
      return;}

    case M_CELL_READ:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_CELL_REMOTEREAD:{
      NOT_IMPLEMENTED;}

    case M_CELL_READANS:{
      NOT_IMPLEMENTED;}

    case M_CELL_CANTPUT:{
      NOT_IMPLEMENTED;}

    case M_LOCK_TOKEN:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
	ResetCP(((MsgBuffer*)fi),M_LOCK_TOKEN);
	unmarshal_M_LOCK_TOKEN((MsgBuffer*)fi,s1,OTI);
	Assert(s1==storeSite);
	Assert(OTI=storeIndex);
	returnSendCredit(s1,OTI);
	lockSendTokenFailure(this,storeSite,OTI);
	return;}
      return;}

    case M_LOCK_CANTPUT:{
      return;}

    case M_FILE:{
      Assert(0);
      warning("impossible\n");
      return;}

    case M_CHAIN_ACK:{
      flag=USUAL_BORROW_CASE;
      break;}


    case M_CHAIN_QUESTION:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_CHAIN_ANSWER:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_ASK_ERROR:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_UNASK_ERROR:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_TELL_ERROR:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_GET_OBJECT:{
      NOT_IMPLEMENTED;}

    case M_GET_OBJECTANDCLASS:{
      NOT_IMPLEMENTED;}

    case M_SEND_OBJECT:{
      NOT_IMPLEMENTED;}

    case M_SEND_OBJECTANDCLASS:{
      NOT_IMPLEMENTED;}

    case M_SEND_GATE:{
      return;}

  default:
    warning("communication problem - impossible");
    Assert(0);
  }

  switch(flag){
  case USUAL_OWNER_CASE:{
    switch(fc){
    case COMM_FAULT_TEMP_NOT_SENT:
    case COMM_FAULT_TEMP_MAYBE_SENT: {
      PD((SITE,"Owner:CommProb temp ignored"));
      return;}
    case COMM_FAULT_PERM_NOT_SENT:{
      PD((SITE,"Owner:CommProb perm not sent extract send credit and ignore"));
      returnSendCredit(storeSite,storeIndex);
      return;}
    case COMM_FAULT_PERM_MAYBE_SENT:{
      PD((SITE,"Owner:CommProb perm maybe sent lose send credit and ignore"));
      return;}}}
  case USUAL_BORROW_CASE:{
    switch(fc){
    case COMM_FAULT_TEMP_NOT_SENT:
    case COMM_FAULT_TEMP_MAYBE_SENT: {
      PD((SITE,"Borrow:CommProb temp ignored"));
      return;}
    case COMM_FAULT_PERM_NOT_SENT:
    case COMM_FAULT_PERM_MAYBE_SENT:{
      PD((SITE,"Borrow:CommProb perm maybe sent lose send credit and ignore"));
      NetAddress na=NetAddress(storeSite,storeIndex);
      BorrowEntry *be=BT->find(&na);
      if(be==NULL) return;
      return;}}}}}
    

/**********************************************************************/
/*   SECTION 41:: Builtins                                            */
/**********************************************************************/

#ifdef MISC_BUILTINS

#ifdef DEBUG_PERDIO
OZ_BI_define(BIdvset,2,0)
{
  OZ_declareIntIN(0,what);
  OZ_declareIntIN(1,val);

  if (val) {
    DV->set(what);
  } else {
    DV->unset(what);
  }
  return PROCEED;
} OZ_BI_end
#endif

#endif

Bool openClosedConnection(int);
int openclose(int);
void wakeUpTmp(int,int);

OZ_BI_define(BIstartTmp,2,0)
{
  OZ_declareIntIN(0,val);
  OZ_declareIntIN(1,time);
  PD((TCPCACHE,"StartTmp v:%d t:%d",val,time));
  if(openClosedConnection(val)){
    PD((TCPCACHE,"StartTmp; continuing"));
    wakeUpTmp(val,time);}
  return PROCEED;
} OZ_BI_end
  
OZ_BI_define(BIcloseCon,1,1)
{
  OZ_declareIntIN(0,what);
  OZ_RETURN(oz_int(openclose(what)));
} OZ_BI_end

OZ_BI_define(BIportWait,2,0)
{
   oz_declareIN(0,prt);
   Assert(oz_isPort(prt));
   oz_declareIntIN(1,t);
   Tertiary *tert = tagged2Tert(prt);
   int dummy;
   return portWait(getSiteFromTertiaryProxy(tert)-> getQueueStatus(dummy),
		   t,tert);
} OZ_BI_end

void wakeUpTmp(int i, int time){
  PD((TCPCACHE,"Starting DangelingThread"));
  Thread *tt = oz_newThreadToplevel(LOW_PRIORITY);
  tt->pushCall(BI_startTmp, oz_int(i), oz_int(time));
  tt->pushCall(BI_Delay, oz_int(time));
}

GenHashNode *getPrimaryNode(GenHashNode* node, int &i);
GenHashNode *getSecondaryNode(GenHashNode* node, int &i);

#ifdef MISC_BUILTINS

OZ_BI_define(BIsiteStatistics,0,1)
{
  int indx;
  Site* found;
  GenHashNode *node = getPrimaryNode(NULL, indx);
  OZ_Term sitelist = oz_nil(); 
  OZ_Term ownerlist = oz_nil();
  OZ_Term borrowlist = oz_nil();
  
  Bool primary = TRUE;
  while(node!=NULL){
    GenCast(node->getBaseKey(),GenHashBaseKey*,found,Site*);  

    TimeStamp *ts = found->getTimeStamp();
    sitelist=
      oz_cons(OZ_recordInit(oz_atom("site"),
      oz_cons(oz_pairA("siteString", oz_atom(found->stringrep())),
      oz_cons(oz_pairAI("port",(int)found->getPort()),
      oz_cons(oz_pairAI("timeint",(int)ts->start),
      oz_cons(oz_pairA("timestr",oz_atom(ctime(&ts->start))),
      oz_cons(oz_pairAI("ipint",(unsigned int)found->getAddress()),
      oz_cons(oz_pairAI("hval",(int)found),
	      oz_nil()))))))),sitelist);
    if(primary){
      node = getPrimaryNode(node,indx);
      if(node!=NULL) {
	printf("p:%s\n",found->stringrep());
	continue;}
      else primary = FALSE;}
    printf("s:%s\n",found->stringrep());
    node = getSecondaryNode(node,indx);}
    
   int limit=OT->getSize();
   char *str;
   for(int ctr = 0; ctr<limit;ctr++){
     OwnerEntry *oe = OT->getEntry(ctr);
     if(oe==NULL){continue;}
     printf("Found something in owner\n");
     Assert(oe!=NULL);
     str = "unknown";
     if(oe->isVar()){
       str = "var";}
    if(oe->isRef()){
       str = "ref";}
     if(oe->isTertiary())
       switch (oe->getTertiary()->getType()){
       case Co_Cell:{str = "cell";break; }
       case Co_Lock:{str = "lock";break; }
     case Co_Port:{str = "port";break; }
     case Co_Object:{str = "object";break; }
     case Co_Array:{str = "array";break; }
     case Co_Dictionary:{str = "dictionary";
     break; }
     case Co_Class:{str = "class";break; }
     default:str = "unknownTert";}
     ownerlist=
     oz_cons(OZ_recordInit(oz_atom("oe"),
     oz_cons(oz_pairA("type", oz_atom(str)),
     oz_cons(oz_pairAI("indx",ctr),oz_nil()))),ownerlist);
 }
 limit=BT->getSize();
 for(int ctr1 = 0; ctr1<limit;ctr1++){
   BorrowEntry *be = BT->getEntry(ctr1);
   if(be==NULL){continue;}
   Assert(be!=NULL);
   printf("Found something in borrow\n");
   str = "unknown";
   if(be->isVar()){
     str = "var";}
   if(be->isRef()){
     str = "ref";}
   if(be->isTertiary())
     switch (be->getTertiary()->getType()){
     case Co_Cell:{str = "cell";break; }
     case Co_Lock:{str = "lock";break; }
     case Co_Port:{str = "port";break; }
     case Co_Object:{str = "object";break; }
     case Co_Array:{str = "array";break; }
     case Co_Dictionary:{str = "dictionary";
     break; }
     case Co_Class:{str = "class";break; }
     default:str = "unknownTert";}
    borrowlist=
     oz_cons(OZ_recordInit(oz_atom("be"),
     oz_cons(oz_pairA("type", oz_atom(str)),
     oz_cons(oz_pairAI("siteHVal", (int) be->getSite()),
     oz_cons(oz_pairAI("indx",be->getOTI()),oz_nil())))),borrowlist);}
 OZ_RETURN(oz_cons(sitelist,
		   oz_cons(borrowlist,
			   oz_cons(ownerlist,oz_nil()))));

} OZ_BI_end

#endif


/**********************************************************************/
/*   SECTION 42:: Initialization                                      */
/**********************************************************************/
//
Bool perdioInit()
{
  Assert(mySite == (Site *) 0);
  initNetwork();
  Assert(mySite != (Site *) 0);
  return (OK);
}

void BIinitPerdio()
{
#ifdef DEBUG_PERDIO
  DV = new DebugVector();
#endif

  initMarshaler();

  creditSite=NULL;

  genFreeListManager=new GenFreeListManager();
  ownerTable = new OwnerTable(DEFAULT_OWNER_TABLE_SIZE);
  borrowTable = new BorrowTable(DEFAULT_BORROW_TABLE_SIZE);
  msgBufferManager= new MsgBufferManager();
  idCounter  = new FatInt();

  //
  // The gate is implemented as a Port reciding at location 0 in
  // the ownertable. The gateStream is keept alive, the Connection 
  // library will fetch it later.
  // The port is made persistent so it should not disapear.
  //
  
  GateStream = oz_newVariable();
  OZ_protect(&GateStream);
  {
    Tertiary *t=(Tertiary*)new PortWithStream(oz_currentBoard(),GateStream);
    t->globalizeTert();
    int ind = t->getIndex();
    Assert(ind == 0);
    OwnerEntry* oe=OT->getOwner(ind);
    oe->makePersistent();
  }
  
  Assert(sizeof(BorrowCreditExtension)==sizeof(Construct_3));
  Assert(sizeof(OwnerCreditExtension)==sizeof(Construct_3));
  Assert(sizeof(Chain)==sizeof(Construct_4));
  Assert(sizeof(ChainElem)==sizeof(Construct_3));
  Assert(sizeof(InformElem)==sizeof(Construct_3));
  Assert(sizeof(CellProxy)==sizeof(CellFrame));
  Assert(sizeof(CellManager)==sizeof(CellFrame));
  Assert(sizeof(CellManager)==sizeof(CellLocal));
  Assert(sizeof(LockProxy)==sizeof(LockFrame));
  Assert(sizeof(LockManager)==sizeof(LockLocal));
  Assert(sizeof(LockManager)==sizeof(LockFrame));
  Assert(sizeof(PortManager)==sizeof(PortLocal));
}

/**********************************************************************/
/*   SECTION 43:: MISC                                                */
/**********************************************************************/


void marshalSite(Site *s,MsgBuffer *buf){
	s->marshalSite(buf);}

Site* getSiteFromBTI(int i){
  return BT->getBorrow(i)->getNetAddress()->site;}

OwnerEntry *getOwnerEntryFromOTI(int i){
  return OT->getOwner(i);}

Tertiary* getTertiaryFromOTI(int i){
  return OT->getOwner(i)->getTertiary();}
  
TaggedRef listifyWatcherCond(EntityCond ec){
  TaggedRef list = oz_nil();
  if(ec & PERM_BLOCKED)
    {list = oz_cons(AtomPerm, list);
    ec = ec & ~(PERM_BLOCKED|TEMP_BLOCKED);}
  if(ec & TEMP_BLOCKED){
    list = oz_cons(AtomTemp, list);
    ec = ec & ~(TEMP_BLOCKED);}
  if(ec & PERM_ME)
    {list = oz_cons(AtomPermHome, list);
    ec = ec & ~(PERM_ME|TEMP_ME);}
  if(ec & TEMP_ME){
    list = oz_cons(AtomTempHome, list);
    ec = ec & ~(TEMP_ME);}
  if(ec & (PERM_ALL| PERM_SOME))
    {list = oz_cons(AtomPermForeign, list);
    ec = ec & ~(TEMP_SOME|PERM_SOME|TEMP_ALL|PERM_ALL);}
  if(ec & (TEMP_ALL|TEMP_SOME)){
    list = oz_cons(AtomTempForeign, list);
    ec = ec & ~(TEMP_SOME|PERM_SOME|TEMP_ALL|PERM_ALL);}
  Assert(ec==0);
  return list;
}

/**********************************************************************/
/*   SECTION 44::Debug                                                */
/**********************************************************************/

ChainElem* Chain::getFirst(){return first;}
ChainElem* Chain::getLast(){return last;}

int printChain(Chain* chain){
  printf("Chain ### Flags: [");
  if(chain->hasFlag(INTERESTED_IN_OK))
    printf(" INTERESTED_IN_OK");
  if(chain->hasFlag(INTERESTED_IN_TEMP ))
    printf(" INTERESTED_IN_TEMP");
  if(chain->hasFlag( TOKEN_PERM_SOME))
    printf(" TOKEN_PERM_SOME");
  if(chain->hasFlag(TOKEN_LOST))
    printf(" TOKEN_LOST");
  printf("]\n");
  ChainElem* cp = chain->getFirst();
  while(cp!= chain->getLast()){
    Assert(cp!=NULL);
    printf("Elem  Flags: [ ");
    if(cp->flagIsSet(CHAIN_GHOST ))
       printf("CHAIN_GHOST ");
    if(cp->flagIsSet( CHAIN_QUESTION_ASKED))
       printf("CHAIN_QUESTION_ASKED ");
    if(cp->flagIsSet( CHAIN_BEFORE))
       printf("CHAIN_BEFORE ");
    if(cp->flagIsSet( CHAIN_PAST))
       printf("CHAIN_PAST ");
    if(cp->flagIsSet( CHAIN_CANT_PUT))
       printf("CHAIN_CANT_PUT ");
    if(cp->flagIsSet( CHAIN_DUPLICATE))
       printf("CHAIN_DUPLICATE ");
    printf("] %s\n",cp->getSite()->stringrep());
    cp = cp->getNext();}
  Assert(cp!=NULL);
  printf("Elem  Flags: [ ");
  if(cp->flagIsSet(CHAIN_GHOST ))
    printf(" CHAIN_GHOST");
  if(cp->flagIsSet( CHAIN_QUESTION_ASKED))
    printf(" CHAIN_QUESTION_ASKED");
  if(cp->flagIsSet( CHAIN_BEFORE))
    printf(" CHAIN_BEFORE");
  if(cp->flagIsSet( CHAIN_PAST))
    printf(" CHAIN_PAST");
  if(cp->flagIsSet( CHAIN_CANT_PUT))
    printf(" CHAIN_CANT_PUT");
  if(cp->flagIsSet( CHAIN_DUPLICATE))
    printf(" CHAIN_DUPLICATE");
  printf("] %s\n",cp->getSite()->stringrep());
  return 8;
}
  
extern Bool checkMySite(){
  return (mySite->isMySite());}


/**********************************************************************/
/*   SECTION 45::Exported for gates                                   */
/**********************************************************************/

OZ_Term getGatePort(Site *sd){
  int si=0; /* Gates are always located at position 0 */
  if(sd==mySite){
    OwnerEntry* oe=OT->getOwner(si);
    Assert(oe->isPersistent());
    return  oe->getValue();}
  NetAddress na = NetAddress(sd,si); 
  BorrowEntry *b = borrowTable->find(&na);
  if (b==NULL) {
    int bi=borrowTable->newBorrow( PERSISTENT_CRED,sd,si);
    b=borrowTable->getBorrow(bi);
    b->mkTertiary((new PortProxy(bi)),b->getFlags());
    b->makePersistent();
    return b->getValue();}
  Assert(b->isPersistent());
  return b->getValue();}


/**********************************************************************/
/* Builtins */
/**********************************************************************/

#ifdef MISC_BUILTINS

OZ_BI_define(BIcrash,0,0)   /* only for debugging */
{
  exit(1);  

  return PROCEED;
} OZ_BI_end

#endif

OZ_BI_define(BIprobe,1,0)
{ 
  OZ_Term e = OZ_in(0);
  NONVAR(e,entity);
  Tertiary *tert = tagged2Tert(entity);
  if(tert->getType()!=Co_Port)
    tert->entityProblem();
  return PROCEED;
} OZ_BI_end

OZ_Return HandlerInstall(Tertiary *entity, SRecord *condStruct,TaggedRef proc){
  EntityCond ec = PERM_BLOCKED;
  Thread *th      = oz_currentThread();
  Bool Continue   = FALSE;
  Bool Persistent = FALSE;
  
  if(condStruct->hasFeature(OZ_atom("cond"))){
    TaggedRef coo = condStruct->getFeature(OZ_atom("cond"));
    NONVAR(coo,co);
    if(co == AtomPermBlocked || co == AtomPerm)
      ec=PERM_BLOCKED;
    else{
      if(co == AtomTempBlocked || co == AtomTemp)
	ec=TEMP_BLOCKED|PERM_BLOCKED;
      else
	return oz_raise(E_ERROR,E_SYSTEM,"invalid handler condition",0);}}
  
  if(condStruct->hasFeature(OZ_atom("basis"))){
    TaggedRef thtt = condStruct->getFeature(OZ_atom("basis"));
    NONVAR(thtt,tht);
    if(tht == AtomPerThread)
      th = oz_currentThread();
    else{
      if(tht == AtomPerSite)
	th = DefaultThread;
      else      
	return oz_raise(E_ERROR,E_SYSTEM,"invalid handler condition",0);}}
  
  if(condStruct->hasFeature(OZ_atom("retry"))){
    TaggedRef ree = condStruct->getFeature(OZ_atom("retry"));
    NONVAR(ree,re);
    if(re == AtomYes)
      Continue = TRUE;
    else{
      if(re == AtomNo)
	Continue = FALSE;
      else
	return oz_raise(E_ERROR,E_SYSTEM,"invalid handler condition",0);}}

  if(condStruct->hasFeature(OZ_atom("once_only"))){  
    TaggedRef onn = condStruct->getFeature(OZ_atom("once_only"));
    NONVAR(onn,on);
    if(on == AtomYes)
      Persistent = FALSE;
    else{
      if(on == AtomNo)
	Persistent = TRUE;
      else
	return oz_raise(E_ERROR,E_SYSTEM,"invalid handler condition",0);}}
  
  Tertiary *lock, *cell;
  switch(entity->getType()){
  case Co_Object:{
    Object *o = (Object *)entity;
    if(entity->getTertType()==Te_Local && !stateIsCell(o->getState())) 
      cell = entity;
    else{
      cell = getCell(o->getState());
      cell->setMasterTert(entity);}
    lock = o->getLock();
    if(cell->installHandler(ec,proc,th,Continue,Persistent)){
      if(lock!=NULL){
	o->getLock()->setMasterTert(entity);
	if(!lock->installHandler(ec,proc,th,Continue,Persistent)){
	  cell->deinstallHandler(th,proc);
	  break;}}
      return PROCEED;}
    break;}
  case Co_Port:
    if(!entity->isProxy()) return PROCEED;
  case Co_Lock:
  case Co_Cell:{
    if(entity->installHandler(ec,proc,th,Continue,Persistent))
      return PROCEED;
    break;}  
  default:
    return oz_raise(E_ERROR,E_SYSTEM,"handlers on ? not implemented",0);}
  return oz_raise(E_ERROR,E_SYSTEM,"handler already installed",0);
}


OZ_Return HandlerDeInstall(Tertiary *entity, SRecord *condStruct,TaggedRef proc){
  Thread *th      = oz_currentThread();
  

  if(condStruct->hasFeature(OZ_atom("basis"))){
    TaggedRef thtt = condStruct->getFeature(OZ_atom("basis"));
    NONVAR(thtt,tht);
    if(tht == AtomPerThread)
      th = oz_currentThread();
    else{
      if(tht == AtomPerSite)
	th = DefaultThread;
      else      
	return oz_raise(E_ERROR,E_SYSTEM,"invalid handler condition",0);}}
  
  Tertiary *lock, *cell;
  switch(entity->getType()){
  case Co_Object:{
    Object *o = (Object *)entity;
    if(entity->getTertType()==Te_Local && !stateIsCell(o->getState())) 
      cell = entity;
    else{
      cell = getCell(o->getState());
      cell->setMasterTert(entity);}
    lock = o->getLock();
    if(cell->deinstallHandler(th,proc)){
      if(lock!=NULL){
	o->getLock()->setMasterTert(entity);
	if(!lock->deinstallHandler(th,proc))
	  break;}
      return PROCEED;}
    break;}
  case Co_Port:
    if(!entity->isProxy()) return PROCEED;
  case Co_Lock:
  case Co_Cell: {
    if(entity->deinstallHandler(th,proc))
      return PROCEED;
    break;}  
  default:
    return oz_raise(E_ERROR,E_SYSTEM,"handlers on ? not implemented",0);}
  return oz_raise(E_ERROR,E_SYSTEM,"handler already Not installed",0);
}



Bool translateWatcherCond(TaggedRef tr,EntityCond &ec, TypeOfConst toc){
    if(tr==AtomPermHome){
      ec|=PERM_ME;
      return TRUE;}
    if(tr==AtomTempHome) {
      ec |= (TEMP_ME|PERM_ME);
      return TRUE;}

    if(tr==AtomPermForeign && toc!=Co_Port){
      ec|=(PERM_SOME|PERM_ALL);
      return TRUE;}
    if(tr==AtomTempForeign && toc!=Co_Port) {
      ec |= (TEMP_SOME|PERM_SOME|TEMP_ALL|PERM_ALL);
      return TRUE;}

    if(tr==AtomTempMe) {
      ec |= (TEMP_ME|PERM_ME);
      return TRUE;}
    if(tr==AtomPermAllOthers && toc!=Co_Port){
      ec |= PERM_ALL;
      return TRUE;}
    if(tr==AtomTempAllOthers && toc!=Co_Port){
      ec |= (TEMP_ALL|PERM_ALL);
      return TRUE;}
    if(tr==AtomPermSomeOther && toc!=Co_Port){
      ec |= PERM_SOME;
      return TRUE;}
    if(tr==AtomTempSomeOther  && toc!=Co_Port){
      ec |= (TEMP_SOME|PERM_SOME);
      return TRUE;}
    return NO;}

OZ_Return translateWatcherConds(TaggedRef tr,EntityCond &ec, TypeOfConst toc){
  TaggedRef car;
  ec=ENTITY_NORMAL;
  
  NONVAR(tr,cdr);
  
  if(!oz_isCons(cdr)){
    if(translateWatcherCond(cdr,ec,toc))
      return PROCEED;
    goto twexit;}
  
  while(!oz_isNil(cdr)){
    if(oz_isVariable(cdr)) {
      return NO;}
    if(!oz_isCons(cdr)){
      return NO;
      return OK;}
    car=tagged2LTuple(cdr)->getHead();
    cdr=tagged2LTuple(cdr)->getTail();
    NONVAR(cdr,tmp);
    cdr = tmp;
    OZ_Return or= translateWatcherCond(car,ec,toc);
    if(translateWatcherCond(tr,ec,toc))
      goto twexit;}
  if(ec==ENTITY_NORMAL) goto twexit;
  return PROCEED;
twexit:
  return oz_raise(E_ERROR,E_SYSTEM,"invalid watcher condition",0);
}

OZ_Return WatcherInstall(Tertiary *entity, SRecord *condStruct,TaggedRef proc){
  EntityCond ec    = PERM_ME;  
  Bool Persistent  = FALSE;
  
  if(condStruct->hasFeature(OZ_atom("cond"))){
    TaggedRef cond = condStruct->getFeature(OZ_atom("cond"));
    OZ_Return tr = 
      translateWatcherConds(cond,ec,entity->getType());
    if(tr!=PROCEED)
      return  tr;}
  
  if(condStruct->hasFeature(OZ_atom("once_only"))){
    TaggedRef oncee = condStruct->getFeature(OZ_atom("once_only"));
    NONVAR(oncee,once);
    if(once == AtomYes)
      Persistent = FALSE;
    else
      if(once == AtomNo)
	Persistent = TRUE;
      else
	return oz_raise(E_ERROR,E_SYSTEM,"invalid handler condition",0);}
      
  switch(entity->getType()){
  case Co_Object:{
    Object *o = (Object *)entity;
    Tertiary *lock, *cell;
    if(entity->getTertType()==Te_Local)
      entity->installWatcher(ec,proc,Persistent);
    else{
      cell = getCell(o->getState());
      cell->setMasterTert(entity);
      cell->installWatcher(ec,proc,Persistent);}
    lock = o->getLock();
    if(lock!=NULL){
      lock->setMasterTert(entity);
      lock->installWatcher(ec,proc,Persistent);}
    break;}
  case Co_Port:
    if(!entity->isProxy()) break;
  case Co_Cell:
  case Co_Lock:{
    entity->installWatcher(ec,proc,Persistent);
    break;}
  default: return oz_raise(E_ERROR,E_SYSTEM,"watchers on ? not implemented",0);}
    return PROCEED;
}


OZ_Return WatcherDeInstall(Tertiary *entity, SRecord *condStruct,TaggedRef proc){
  EntityCond ec    = PERM_ME;  

  if(condStruct->hasFeature(OZ_atom("cond"))){
    TaggedRef cond = condStruct->getFeature(OZ_atom("cond"));
    OZ_Return tr = 
      translateWatcherConds(cond,ec,entity->getType());
    if(tr!=PROCEED)
      return  tr;}
  
  switch(entity->getType()){
  case Co_Object:{
    Object *o = (Object *)entity;
    Tertiary *lock, *cell;
    if(entity->getTertType()==Te_Local)
      cell = entity;
    else
      cell = getCell(o->getState());
    if(cell->deinstallWatcher(ec,proc)){
      lock = o->getLock();
      if(lock!=NULL || lock->deinstallWatcher(ec,proc))
	return PROCEED;}
    break;}
  case Co_Port:
    if(!entity->isProxy()) return PROCEED;
  case Co_Cell:
  case Co_Lock:{
    if(entity->deinstallWatcher(ec,proc))
      return PROCEED;}
  default: return oz_raise(E_ERROR,E_SYSTEM,"watchers on ? not implemented",0);}
  return oz_raise(E_ERROR,E_SYSTEM,"Watcher Not installed",0);
}

OZ_BI_define(BIhwInstall,3,0){
  OZ_Term e0        = OZ_in(0);
  OZ_Term c0        = OZ_in(1);
  OZ_Term proc      = OZ_in(2);  
  
  
  NONVAR(c0, c);
  NONVAR(e0, e);
  TaggedRef label;
  SRecord  *condStruct;
  if(oz_isSRecord(c)){
    condStruct = tagged2SRecord(c);
    label = condStruct->getLabel();}
  else
    return oz_raise(E_ERROR,E_SYSTEM,"???? is not a Srecord",0);
  
  Tertiary *entity = tagged2Tert(e);
  
  TaggedRef type = condStruct->getLabel();
  if(type == AtomHandler)
    return HandlerInstall(entity,condStruct,proc);
  if(type == AtomWatcher)
    return WatcherInstall(entity,condStruct,proc);
  return oz_raise(E_ERROR,E_SYSTEM,"label must be either handler or watcher",0);
}OZ_BI_end


OZ_BI_define(BIhwDeInstall,3,0){
  OZ_Term e0        = OZ_in(0);
  OZ_Term c0        = OZ_in(1);
  OZ_Term proc      = OZ_in(2);  
  
  NONVAR(c0, c);
  NONVAR(e0, e);
  TaggedRef label;
  SRecord  *condStruct;
  if(oz_isSRecord(c)){
    condStruct = tagged2SRecord(c);
    label = condStruct->getLabel();}
  else
    oz_raise(E_ERROR,E_SYSTEM,"???? is not a Srecord",0);
  
  Tertiary *entity = tagged2Tert(e);
  TaggedRef type = condStruct->getLabel();
  if(type == AtomHandler)
    return HandlerDeInstall(entity,condStruct,proc);
  if(type == AtomWatcher)
    return WatcherDeInstall(entity,condStruct,proc);
  oz_raise(E_ERROR,E_SYSTEM,"label must be either handler or watcher",0);
  return PROCEED;
}OZ_BI_end

OZ_BI_define(BIgetEntityCond,1,1)
{
  OZ_Term e = OZ_in(0);
  NONVAR(e, entity);
  Tertiary *tert = tagged2Tert(entity);
  
  EntityCond ec = tert->getEntityCond();
  if(ec == ENTITY_NORMAL)
    OZ_RETURN(oz_cons(AtomEntityNormal,oz_nil()));
  OZ_RETURN(listifyWatcherCond(ec));
}OZ_BI_end
