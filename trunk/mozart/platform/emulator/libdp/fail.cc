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
#pragma implementation "fail.hh"
#endif

#include "base.hh"
#include "thr_int.hh"
#include "controlvar.hh"
#include "builtins.hh"
#include "var_ext.hh"
#include "tagged.hh"

#include "dsite.hh"
#include "fail.hh" 
#include "perdio.hh"
#include "table.hh"
#include "chain.hh"
#include "state.hh"
#include "protocolFail.hh"
#include "protocolState.hh"
#include "port.hh"
#include "var.hh"
#include "var_obj.hh"
#include "builtins.hh"

Twin *usedTwins;
Watcher* globalWatcher;

/**********************************************************************/
/*   forward                                   */
/**********************************************************************/

void adjustProxyForFailure(Tertiary*,EntityCond,EntityCond);

/**********************************************************************/
/*   deferoperations                                          */
/**********************************************************************/


TaggedRef BI_defer;

DeferElement *DeferdEvents;

OZ_BI_define(BIdefer,0,0)
{ 
  DeferElement *ptr = DeferdEvents, *old;
  DeferdEvents = NULL;
  while(ptr){
    switch(ptr->type){
    case DEFER_PROXY_PROBLEM:
      proxyProbeFault(ptr->tert, ptr->prob);
      break;
    case DEFER_MANAGER_PROBLEM:
      managerProbeFault(ptr->tert,ptr->site, ptr->prob);
      break;
    case DEFER_ENTITY_PROBLEM:{
      entityProblem(ptr->tert);
      break;}
    default: Assert(0);
    }
    old = ptr;
    ptr = ptr->next;
    genFreeListManager->putOne_5((FreeListEntry*)old);
  }
  return PROCEED;
} OZ_BI_end

void addDeferElement(DeferElement* e){
  if(DeferdEvents==NULL){
    Thread *tt = oz_newThreadToplevel(DEFAULT_PRIORITY);
    tt->pushCall(BI_defer);}
  e->next = DeferdEvents;
  DeferdEvents = e;
}

DeferElement* newDeferElement(){
  return (DeferElement*) genFreeListManager->getOne_5();}

void gcDeferEvents(){
  DeferElement* ptr = DeferdEvents;
  while(ptr!=NULL) {
    ptr->tert=(Tertiary*)ptr->tert->gcConstTerm();
    if(ptr->type==DEFER_MANAGER_PROBLEM)
      ptr->site->makeGCMarkSite();
    ptr=ptr->next;}
}

/**********************************************************************/
/*   support                                   */
/**********************************************************************/

#define PROBE_UNUSED PROBE_OK

void deferEntityProblem(Tertiary* t){
  DeferElement *e = newDeferElement();
  e->init(DEFER_ENTITY_PROBLEM,t);
  addDeferElement(e);
}

void deferManagerProbeFault(Tertiary* t,DSite* s, int pr){
  DeferElement *e = newDeferElement();
  e->init(s,DEFER_MANAGER_PROBLEM,pr,t);
  addDeferElement(e);
}

void deferProxyProbeFault(Tertiary* t, int pr){
  DeferElement *e = newDeferElement();
  e->init(DEFER_PROXY_PROBLEM,pr,t);
  addDeferElement(e);
}
void managerInstallProbe(Tertiary* t,ProbeType pt){
  installProbeNoRet(BT->getOriginSite(t->getIndex()),pt);}

void watcherRemoved(Watcher* w, Tertiary* t){
  EntityCond oldC=getSummaryWatchCond(t);
  Watcher** base=getWatcherBase(t);
  while((*base)!=w){
    base=&((*base)->next);
    Assert((*base)!=NULL);}
  *base= w->next;
  EntityCond newC=getSummaryWatchCond(t);
  if(t->getTertType()==Te_Manager){}
  else
    adjustProxyForFailure(t,oldC,newC);
}

EntityCond EntityInfo::getSummaryWatchCond(){
  EntityCond ec=ENTITY_NORMAL;
  Watcher *w=watchers;
  while(w!=NULL){
    ec |= w->watchcond;
    w=w->next;}
  return ec;
}

Bool EntityInfo::meToBlocked(){
  Bool changed=FALSE;
  if(entityCond & PERM_ME) {
    if(!(entityCond & PERM_BLOCKED)){
      entityCond|= PERM_BLOCKED;
      changed=TRUE;}}
  if(entityCond & TEMP_ME){
    if(!(entityCond & TEMP_BLOCKED)){
      entityCond|= TEMP_BLOCKED;
      changed=TRUE;}}
  return changed;
}

/**********************************************************************/
/*   insert and remove watchers */
/**********************************************************************/

static Bool checkForExistentInjector(EntityInfo *ei, Thread* th, 
				     EntityCond wc,unsigned int kind){
  if(!(kind & WATCHER_INJECTOR)) return FALSE;
  if(ei==NULL) return FALSE;
  Assert(isInjectorCondition(wc));
  Watcher *w=ei->watchers;
  if(kind & WATCHER_SITE_BASED){
    while(w!=NULL){
      if(w->isInjector() && (w->isSiteBased())) return TRUE;    
      w=w->next;}
    return FALSE;}
  while(w!=NULL){
    if(w->isInjector() && (w->thread==th)) return TRUE;    
    w=w->next;}
  return FALSE;
}


void insertWatcherLocal(Tertiary *t,Watcher *w){
  EntityInfo* info=t->getInfo();
  if(info==NULL){
    info=new EntityInfo(w);
    t->setInfo(info);
    return;}  
  w->next=info->watchers;
  info->watchers=w;
}

Watcher*  basicInsertWatcher(Watcher* w,Watcher* old){
  if(w->isSiteBased()){
    if(old==NULL) {
      w->next=NULL;
      return w;}
    while(old->next!=NULL){
      old=old->next;}
    old->next=w;
    w->next=NULL;
    return old;}
  w->next=old;
  return w;}

void insertWatcher(EntityInfo* ei,Watcher* w,EntityCond &oldEC, EntityCond &newEC){
  oldEC=ei->getSummaryWatchCond();
  ei->watchers=basicInsertWatcher(w,ei->watchers);
  newEC=ei->getSummaryWatchCond();}

void insertWatcher(Tertiary *t,Watcher *w, EntityCond &oldEC, EntityCond &newEC){
  EntityInfo* info=t->getInfo();
  if(info==NULL){
    info=new EntityInfo(w);
    t->setInfo(info);
    oldEC=ENTITY_NORMAL;
    newEC=w->watchcond;
    return;}
  insertWatcher(info,w,oldEC,newEC);
}

/**********************************************************************/
/*   entityProblem                            */
/**********************************************************************/

PendThread* threadTrigger(Tertiary* t,Watcher* w){
  PendThread* aux, *old = NULL, *pd;
  switch(t->getType()){
  case Co_Port:
    pd = getPendThreadStartFromPort(t);
    break;
  case Co_Cell:
  case Co_Lock:
    if(t->getTertType()==Te_Proxy) return NULL;
    pd=getPendThreadStartFromCellLock(t);
    break;
  default:
    Assert(0);
    return NULL;
  }
  while(TRUE){
    if(pd==NULL) return NULL;
    if(pd->thread==NULL){
      pd=pd->next;
      continue;}
    if(w->isSiteBased() && pd->thread!=NULL) return pd;
    if(!w->isSiteBased() && pd->thread==w->thread) return pd;
    pd=pd->next;}
  return NULL;
}

void dealWithContinue(Tertiary* t,PendThread* pd){
  switch(t->getType()){
  case Co_Cell:{
    switch(pd->exKind){
    case EXCHANGE:{ 
      pd->thread->pushCall(BI_exchangeCell,makeTaggedTert(t), 
				pd->nw, pd->old); 
      return;}
    case ASSIGN:{
      pd->thread->pushCall(BI_assign,makeTaggedTert(t),pd->old,pd->nw); 
      return;}
    case AT:{
      pd->thread->pushCall(BI_atRedo,makeTaggedTert(t),pd->old,pd->nw); 
      return;}
    default: Assert(0);}}
  case Co_Lock:{
    // mm2: no builtin lockLock available
    // pd->thread->pushCall(BI_lockLock,makeTaggedTert(t));
    Assert(0);
    return;}
  case Co_Port:
    pd->thread->pushCall(BI_send, makeTaggedTert(t), pd->old);
  default:
    Assert(0); 
  }}

  
Bool entityProblemPerWatcher(Tertiary*t, Watcher* w,Bool &hit){
  EntityCond ec=getEntityCond(t) & w->watchcond;
  if(w->isInjector()){
    PendThread* pd=threadTrigger(t,w);
    if(pd==NULL) return FALSE;
    hit=TRUE;
    if(ec==ENTITY_NORMAL) return FALSE;    
    if(w->isRetry() && !w->isFired()) dealWithContinue(t,pd);
    w->invokeInjector(makeTaggedTert(t),ec,pd->controlvar,pd->thread);
    pd->thread=NULL;
    if(w->isPersistent()) return FALSE;
    watcherRemoved(w,t);
    return TRUE;}
  if(ec==ENTITY_NORMAL) return FALSE;    
  w->invokeWatcher(makeTaggedTert(t),ec);
  return TRUE;
}


Bool entityCondMeToBlocked(Tertiary* t){
  if(getEntityCond(t) & PERM_ME|TEMP_ME) {
    return t->getInfo()->meToBlocked();}
  return FALSE;
}
  
void entityProblem(Tertiary *t) { 
  PD((ERROR_DET,"entityProblem invoked"));
  Watcher* w;
  Watcher** aux;
  EntityCond oldC=getSummaryWatchCond(t);
  Bool hit=FALSE;
  if(errorIgnore(t)) {
    if(globalWatcher==NULL) return;
    goto global;}
  else{
    aux=getWatcherBase(t);
    if(aux==NULL){
      if(globalWatcher==NULL) return;
      goto global;}}
  
  while((*aux)!=NULL){
    if(entityProblemPerWatcher(t,(*aux),hit)){
      *aux=(*aux)->next;
      continue;}
    if(hit) break;
    aux= &((*aux)->next);}

 global:

  if((!hit) && (globalWatcher!=NULL) &&
     (globalWatcher->watchcond & getEntityCond(t)))
    entityProblemPerWatcher(t,globalWatcher,hit);

  EntityCond newC=getSummaryWatchCond(t);
  if(t->getTertType()!=Te_Manager)
    adjustProxyForFailure(t,oldC,newC);
}
    
/**********************************************************************/
/*   SECTION::  watcher                */
/**********************************************************************/

void Watcher::invokeWatcher(TaggedRef t,EntityCond ec){
if(!isFired()){
  Assert(!isInjector());
  Thread *tt = oz_newThreadToplevel(DEFAULT_PRIORITY);
  tt->pushCall(proc, t,listifyWatcherCond(ec));}
}

void Watcher::varInvokeInjector(TaggedRef t,EntityCond ec){
  Assert(isInjector());
  am.prepareCall(proc, t,listifyWatcherCond(ec));
}

void Watcher::invokeInjector(TaggedRef t,EntityCond ec,TaggedRef controlvar,Thread *th){
  Assert(isInjector());
  Assert(!isFired());
  th->pushCall(proc, t,listifyWatcherCond(ec));
  ControlVarResume(controlvar);
}

/**********************************************************************/
/*   SECTION::  probeFault -- first indication of error                */
/**********************************************************************/

void cellLockManagerProbeFault(Tertiary *t, DSite* s, int pr){
    Chain *ch=getChainFromTertiary(t);
  if(pr==PROBE_OK){
    if(!ch->hasFlag(INTERESTED_IN_OK)) return;
    ch->managerSeesSiteOK(t,s);
    return;}
  if(pr==PROBE_TEMP){
    ChainElem *ce=ch->getFirstNonGhost();
    if((ce->getSite()==s) || ((ce->getNext()!=NULL) &&
			      (ce->getNext()->getSite()==s)))
      ch->managerSeesSiteTemp(t,s);
    return;}
  ch->managerSeesSitePerm(t,s);}
 
void cellLockProxyProbeFault(Tertiary *t, int pr){
  int state;
  if(t->isProxy()){
    state=Cell_Lock_Invalid;}
  else{
    state=getStateFromLockOrCell(t);}
  if(pr==PROBE_PERM){
    cellLock_Perm(state,t);
    return;}
  if(pr == PROBE_OK){
    cellLock_OK(state,t);
    return;}
  Assert(pr==PROBE_TEMP);
  cellLock_Temp(state,t);
  return;}

void portProxyProbeFault(Tertiary *t, int pr){
  PortProxy *pp = (PortProxy*) t;
  switch(pr){
  case PROBE_PERM:
    port_Perm(pp);
    break;
  case PROBE_OK:
    port_Ok(pp);
    break;
  case PROBE_TEMP:
    port_Temp(pp);
    break;
  default: 
    Assert(0);
  }
}

void proxyProbeFault(Tertiary *t, int pr) {
  PD((ERROR_DET,"proxy probe invoked %d",pr));
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock:
    cellLockProxyProbeFault(t,pr);
    return;
  case Co_Port:
    portProxyProbeFault(t,pr);
    return;   
  case Co_Object:
  case Co_Resource:
    return;
  default: Assert(0);
    return;}
}

void managerProbeFault(Tertiary *t, DSite* s,int pr) {
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock:
    if(getChainFromTertiary(t)->siteExists(s))
      cellLockManagerProbeFault(t, s, pr);
    return;
  case Co_Port: 
    /* The portManager is not affected by 
       other sites. */
    return;
  case Co_Object:
    return;
  default:
    printf("WARNING %d\n",t->getType());
    Assert(0);}
}

void DSite::probeFault(ProbeReturn pr) {
  PD((PROBES,"PROBEfAULT  site:%s",stringrep()));
  int limit=OT->getSize();
  for(int ctr = 0; ctr<limit;ctr++){
    OwnerEntry *oe = OT->getOwner(ctr);
    if(oe->isTertiary()){
      Tertiary *tr=oe->getTertiary();
      PD((PROBES,"Informing Manager"));
      Assert(tr->isManager());
      managerProbeFault(tr,this,pr);}
    else{
      if(oe->isVar()){
	GET_VAR(oe,Manager)->probeFault(this,pr);}}}

  limit=BT->getSize();
  for(int ctr1 = 0; ctr1<limit;ctr1++){
    BorrowEntry *be = BT->getBorrow(ctr1);
    if(be->isTertiary()){
      if(be->getSite()==this){
	proxyProbeFault(be->getTertiary(),pr);}}
    else{
      if(be->isVar()){
	if(be->getSite()==this){	
	  if(typeOfBorrowVar(be)==VAR_PROXY){
	    GET_VAR(be,Proxy)->probeFault(pr);}
	  else{
	    Assert(typeOfBorrowVar(be)==VAR_OBJECT);
	    GET_VAR(be,Object)->probeFault(pr);}}}}}
}

/**********************************************************************/
/*   SECTION::              chain and error                            */
/**********************************************************************/

void Chain::establish_PERM_SOME(Tertiary* t){
  if(hasFlag(TOKEN_PERM_SOME)) return;
  setFlag(TOKEN_PERM_SOME);
  int OTI=t->getIndex();
  triggerInforms(&inform,OT->getOwner(OTI),OTI,PERM_SOME);
  addEntityCond(t,PERM_SOME);
  entityProblem(t);}

void Chain::establish_TOKEN_LOST(Tertiary* t){
  setFlagAndCheck(TOKEN_LOST);
  int OTI=t->getIndex();
  triggerInforms(&inform,OT->getOwner(OTI),OTI,PERM_SOME|PERM_ME);
  addEntityCond(t,PERM_SOME|PERM_ME);}

void Chain::shortcutCrashLock(LockManager* lm){
  establish_PERM_SOME(lm);
  ChainElem** base=getFirstNonGhostBase();
  ChainElem *ce;
  LockSec* sec=lm->getLockSec();
  if((*base)->next==NULL){
    LockSec *sec=lm->getLockSec();
    ChainElem *ce=*base;
    ce->reinit(myDSite);
    Assert(sec->state==Cell_Lock_Invalid);
    sec->state=Cell_Lock_Valid;
    return;}
  removeNextChainElem(base);
  ce=getFirstNonGhost();
  int OTI=lm->getIndex();
  if(ce->site==myDSite){
    lockReceiveTokenManager(OT->getOwner(OTI),OTI);
    return;}
  lockSendToken(myDSite,OTI,ce->site);}

void Chain::shortcutCrashCell(CellManager* cm,TaggedRef val){
  establish_PERM_SOME(cm);
  ChainElem** base=getFirstNonGhostBase();
  ChainElem *ce;
  CellSec* sec=cm->getCellSec();
  if((*base)->next==NULL){
    CellSec *sec=cm->getCellSec();
    ChainElem *ce=*base;
    ce->reinit(myDSite);
    Assert(sec->state=Cell_Lock_Invalid);
    sec->state=Cell_Lock_Valid;
    sec->contents=val;
    return;}
  removeNextChainElem(base);
  ce=getFirstNonGhost();
  int index=cm->getIndex();
  if(ce->site==myDSite){
    cellReceiveContentsManager(OT->getOwner(index),val,index);
    return;}
  OT->getOwner(index)->getOneCreditOwner();
  cellSendContents(val,ce->site,myDSite,index);}

void Chain::handleTokenLost(Tertiary* t,OwnerEntry *oe,int OTI){
  establish_TOKEN_LOST(t);
  ChainElem *ce=first->next;
  ChainElem *back;
  Assert(first->site->siteStatus()==SITE_PERM);
  releaseChainElem(first);
  while(ce){
    if(!ce->flagIsSet(CHAIN_GHOST)){
      if(ce->site!=myDSite){
	sendTellError(oe,ce->site,OTI,PERM_BLOCKED,TRUE);}}
    back=ce;
    ce=ce->next;
    releaseChainElem(back);}
  first=NULL;
  last=NULL;
  entityProblem(t);
}

void Chain::managerSeesSitePerm(Tertiary *t,DSite* s){
  PD((ERROR_DET,"managerSeesSitePerm site:%s nr:%d",
      s->stringrep(),t->getIndex()));
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
      removeNextChainElem(&(before->next));
      managerSeesSitePerm(t,before->site);
      return;}}
  if(after==NULL){
    PD((ERROR_DET,"managerSeesSitePerm - perm is last site"));
    dead->setFlag(CHAIN_BEFORE);}
  else{
    PD((ERROR_DET,"managerSeesSitePerm - perm is not last site"));
    if(after->site->siteStatus()==SITE_PERM){
      removeNextChainElem(&(dead->next));
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
    shortcutCrashLock((LockManager*) t);
    return;}
  PD((ERROR_DET,"Token lost"));
  int OTI=t->getIndex();
  handleTokenLost(t,OT->getOwner(OTI),OTI);
  Assert(inform==NULL);
  return;
}

Bool InformElem::maybeTrigger(OwnerEntry* oe, int index, EntityCond ec){
  EntityCond xec= ec & watchcond;
  xec &= ~foundcond;
  if(xec == ENTITY_NORMAL) return FALSE;
  foundcond |= xec;
  sendTellError(oe,site,index,xec,TRUE);
  if(somePermCondition(xec)) return TRUE;
  return FALSE;
}

void InformElem::maybeTriggerOK(OwnerEntry* oe, int index, EntityCond ec){
  EntityCond xec= ec & foundcond;
  if(xec == ENTITY_NORMAL) return;
  foundcond &= ~xec;
  sendTellError(oe,site,index,xec,FALSE);
}

void triggerInforms(InformElem **base,OwnerEntry* oe,int index,EntityCond ec){
  while((*base)!=NULL){
    if((*base)->maybeTrigger(oe,index,ec))
      (*base) = ((*base)->next);
    else
      base= &((*base)->next);}
}       

void triggerInformsOK(InformElem **base,OwnerEntry* oe,int index,EntityCond ec){
  InformElem *ie=(*base);
  while(ie!=NULL){
    ie->maybeTriggerOK(oe,index,ec);
    ie=ie->next;}
}       
  
  // approximative PER-LOOK
void Chain::managerSeesSiteTemp(Tertiary *t,DSite* s){
  EntityCond ec;
  int index=t->getIndex();
  OwnerEntry *oe=OT->getOwner(index);
  PD((ERROR_DET,"managerSeesSiteTemp site:%s nr:%d",
      s->stringrep(),index));

// deal with TEMP_SOME|TEMP_ME watchers
  triggerInforms(&inform,oe,index,(TEMP_SOME|TEMP_ME));

  ChainElem *ce=findAfter(s); // deal with TEMP_BLOCKED injectors
  while(ce!=NULL){
    if(ce->getSite()->siteStatus()!=SITE_OK) break;
    sendTellError(oe,ce->getSite(),index,ec,TRUE);
    ce=ce->next;}

  setFlag(INTERESTED_IN_OK);
}

void Chain::managerSeesSiteOK(Tertiary *t,DSite* s){
  Assert(siteExists(s));
  Assert(hasFlag(INTERESTED_IN_OK));

  int index=t->getIndex();  
  OwnerEntry *oe=OT->getOwner(index);
  PD((ERROR_DET,"managerSeesSiteOK site:%s nr:%d",
      s->stringrep(),index));

// deal with TEMP_SOME|TEMP_ME watchers
  if(!(tempConnectionInChain())){
    triggerInformsOK(&inform,oe,index,(TEMP_SOME|TEMP_ME));}

  ChainElem *ce=findAfter(s); // deal with TEMP_BLOCKED injectors
  while(ce!=NULL){
    if(ce->getSite()->siteStatus()==SITE_TEMP) break;
    sendTellError(oe,ce->getSite(),index,TEMP_BLOCKED,FALSE);
    ce=ce->next;}

  if(!tempConnectionInChain()){
    resetFlagAndCheck(INTERESTED_IN_OK);}
}

/**********************************************************************/
/**********************************************************************/

inline void proxyInform(Tertiary* t,EntityCond ec){
  sendAskError(BT->getBorrow(t->getIndex()),ec);
}

inline void proxyDeInform(Tertiary* t,EntityCond ec){
    sendUnAskError(BT->getBorrow(t->getIndex()),ec);
}

EntityCond askPart(Tertiary* t, EntityCond ec){
  Assert(!(t->isLocal()));
  Assert(!(t->isManager()));
  switch(t->getType()){
  case Co_Lock:
  case Co_Cell:
    return ec & (PERM_SOME|TEMP_SOME|PERM_ME|TEMP_ME);
  case Co_Port:
    break;
  default:
    Assert(0);}
  return ENTITY_NORMAL;   
}

EntityCond varAskPart(EntityCond ec){
  return ec & (TEMP_SOME|PERM_SOME);}

void adjustProxyForFailure(Tertiary*t, EntityCond oldEC, EntityCond newEC){
  if(askPart(t,newEC)!=askPart(t,oldEC)){
    if(askPart(t,oldEC)!=ENTITY_NORMAL) 
      proxyDeInform(t,askPart(t,oldEC));
    proxyInform(t,askPart(t,newEC));}
}

void varAdjustPOForFailure(int index,EntityCond oldC, EntityCond newC){
  if(varAskPart(oldC)==varAskPart(newC)) return;
  if(varAskPart(oldC)!=ENTITY_NORMAL){
    sendUnAskError(BT->getBorrow(index),oldC);}
  if(varAskPart(newC)!=ENTITY_NORMAL){
    sendAskError(BT->getBorrow(index),newC);}
}

/**********************************************************************/
/*   SECTION::       installation/deinstallation utility  VARS        */
/**********************************************************************/

OZ_Return installWatcher(TaggedRef* tPtr,EntityCond wc,TaggedRef proc, 
		    Thread* th, unsigned int kind) {

  VarKind vk=classifyVar(tPtr);
  Assert(vk!=VAR_KINDED);
  if((vk==VAR_FREE) || (vk==VAR_FUTURE)){
    (void) globalizeFreeVariable(tPtr);
    Assert(classifyVar(tPtr)==VAR_MANAGER);
    vk=VAR_MANAGER;}
    
  EntityInfo *ei=varMakeOrGetEntityInfo(tPtr);
  if(checkForExistentInjector(ei,th,wc,kind)) 
    return IncorrectFaultSpecification;
  Watcher* w=new Watcher(proc,th,wc,kind);
  EntityCond oldC,newC;
  insertWatcher(ei,w,oldC,newC);

  switch(vk){
  case VAR_MANAGER:{
    if(ei->getEntityCond()!=ENTITY_NORMAL){
      oz_getManagerVar(*tPtr)->newWatcher(w->isInjector());}
    break;}
  case VAR_PROXY:{
    ProxyVar *pv=oz_getProxyVar(*tPtr);
    if(ei->getEntityCond()!=ENTITY_NORMAL){
      pv->newWatcher(w->isInjector());}
    varAdjustPOForFailure(pv->getIndex(),oldC,newC);
    break;}
  case VAR_OBJECT:{
    ObjectVar *ov=oz_getObjectVar(*tPtr);
    if(ei->getEntityCond()!=ENTITY_NORMAL){
      ov->newWatcher(w->isInjector());}
    varAdjustPOForFailure(ov->getObject()->getIndex(),oldC,newC);
    break;}
  default:
    Assert(0);}
  return TRUE;
}

OZ_Return deinstallWatcher(TaggedRef* tPtr,EntityCond wc,TaggedRef proc, 
		      Thread* th, unsigned int kind){
  VarKind vk=classifyVar(tPtr);
  Assert(vk!=VAR_KINDED);
  Assert(vk!=VAR_FREE);
  Assert(vk!=VAR_FUTURE);

  EntityInfo* ei=varGetEntityInfo(tPtr);
  if(ei==NULL) return IncorrectFaultSpecification;
  
  EntityCond oldC,newC;
  Watcher** base= &ei->watchers;
  if(base==NULL) return IncorrectFaultSpecification;
  oldC=ei->getSummaryWatchCond();

  Bool found = FALSE;
  while(*base!=NULL){
    if((*base)->matches(proc,th,wc,kind)){
      *base = (*base)->next;
      found = TRUE;
      break;}
    else{ 
      base= &((*base)->next);}}

  if(!found) return FALSE;

  PD((NET_HANDLER,"Watcher deinstalled")); 
  newC=ei->getSummaryWatchCond();

  switch(vk){
  case VAR_MANAGER:{
    break;}
  case VAR_PROXY:{
    ProxyVar *pv=oz_getProxyVar(*tPtr);
    varAdjustPOForFailure(pv->getIndex(),oldC,newC);
    break;}
  case VAR_OBJECT:{
    ObjectVar *ov=oz_getObjectVar(*tPtr);
    varAdjustPOForFailure(ov->getObject()->getIndex(),oldC,newC);
    break;}
  default:
    Assert(0);}
  return TRUE;
}

/**********************************************************************/
/*   SECTION::       installation/deinstallation utility             */
/**********************************************************************/

Bool isWatcherEligible(Tertiary *c){
  switch(c->getType()){
  case Co_Object:
  case Co_Cell:
  case Co_Lock:
  case Co_Port: return TRUE;
  default: return FALSE;}
  Assert(0);
  return FALSE;
}

OZ_Return installGlobalWatcher(EntityCond wc,TaggedRef proc,int kind){
  if(globalWatcher!=NULL) {return IncorrectFaultSpecification;}
  globalWatcher=new Watcher(proc,NULL,wc,kind);
  return PROCEED;}

OZ_Return deInstallGlobalWatcher(EntityCond wc,TaggedRef proc,int kind){
  if(wc!=globalWatcher->watchcond) {return IncorrectFaultSpecification;}
  if(globalWatcher==NULL) {return IncorrectFaultSpecification;}
  globalWatcher=NULL;
  return PROCEED;}

EntityInfo *tertiaryMakeOrGetEntityInfo(Tertiary* t){
  if(t->getInfo()==NULL){
    t->setInfo(new EntityInfo);}
  return t->getInfo();}

void transferWatchers(Object* o){
  EntityInfo *cei=o->getInfo();  
  if(cei==NULL) return;
  Assert(cei->getEntityCond()==ENTITY_NORMAL);
  EntityInfo *lei=new EntityInfo();
  Watcher *ow=cei->watchers;
  Assert(ow!=NULL);
  Assert(stateIsCell(o->getState()));
  CellManager* cm=(CellManager*) getCell(o->getState());
  LockManager* lm=(LockManager*) o->getLock();
  if(lm==NULL){
    cm->setInfo(cei);
    return;}

  cm->setInfo(cei);
  lm->setInfo(lei);
  Watcher* nw=new Watcher(ow->proc,ow->thread,ow->kind,ow->watchcond);
  nw->lockTwin(ow->cellTwin());
  ow=ow->next;
  Watcher* aux;
  while(ow!=NULL){
    aux=new Watcher(ow->proc,ow->thread,ow->kind,ow->watchcond);
    aux->lockTwin(ow->cellTwin());
    ow=ow->next;
    nw->next=aux;
    nw=aux;}
}

OZ_Return installWatcher(Tertiary* t,EntityCond wc,TaggedRef proc, 
		    Thread* th, unsigned int kind) {

  if(!isWatcherEligible(t)) {return IncorrectFaultSpecification;}
  Watcher *w=new Watcher(proc,th,wc,kind);

  PD((NET_HANDLER,"Watcher installed on tertiary %x",t));  
  EntityInfo *ei=tertiaryMakeOrGetEntityInfo(t);
  if(checkForExistentInjector(ei,th,wc,kind)){
    return IncorrectFaultSpecification;}

  EntityCond oldC,newC;
  if(t->isLocal()){
    insertWatcherLocal(t,w);    
    return PROCEED;}
  insertWatcher(t,w,oldC,newC);
  if(t->isManager()){}
  else{
    adjustProxyForFailure(t,oldC,newC);}
  if(w->isTriggered(getEntityCond(t))) deferEntityProblem(t);
  return PROCEED;
}

OZ_Return deinstallWatcher(Tertiary* t,EntityCond wc,TaggedRef proc, 
		      Thread* th, unsigned int kind){
  if(!isWatcherEligible(t)) {return IncorrectFaultSpecification;}

  if((t->getType()==Co_Object) & (t->getTertType()!=Te_Local)){
    Object* o= (Object*) t;
    if(!stateIsCell(o->getState())){
      return IncorrectFaultSpecification;}
    OZ_Return ret=deinstallWatcher(o->getLock(),wc,proc,th,kind);
    if(ret!=PROCEED) return ret;
    PD((NET_HANDLER,"Watcher on object deinstalled")); 
    ret=deinstallWatcher(getCell(o->getState()),wc,proc,th,kind);
    Assert(ret==TRUE);
    return PROCEED;}

  EntityCond oldEC=getSummaryWatchCond(t);
  Bool found = FALSE;
  Watcher **base=getWatcherBase(t);
  while(*base!=NULL){
    if((*base)->matches(proc,th,wc,kind)){
      *base = (*base)->next;
      found = TRUE;
      break;}
    else{ 
      base= &((*base)->next);}}

  if(!found) return IncorrectFaultSpecification;

  PD((NET_HANDLER,"Watcher deinstalled")); 
  EntityCond newEC=getSummaryWatchCond(t);

  if(t->isLocal()) return PROCEED;
  if(!t->isManager())
    adjustProxyForFailure(t,oldEC,newEC);
  return PROCEED;
}

/**********************************************************************/
/*   SECTION::              user interface                            */
/**********************************************************************/

EntityCond translateWatcherCond(TaggedRef tr){
  if(tr==AtomPermBlocked)
    return PERM_BLOCKED;
 if(tr== AtomBlocked)
    return PERM_BLOCKED|TEMP_BLOCKED;
  if(tr== AtomPermWillBlock)
    return PERM_ME;
  if(tr== AtomWillBlock)
    return TEMP_ME|PERM_ME;
  if(tr== AtomPermSome)
    return PERM_SOME;
  if(tr== AtomSome)
    return TEMP_SOME|PERM_SOME;
  if(tr== AtomPermAll)
    return PERM_ALL;
  if(tr== AtomAll)
    return TEMP_ALL|PERM_ALL;
  Assert(0);
  return 0;
}

#define DerefVarTest(tt) { \
  if(oz_isVariable(tt)){OZ_suspendOn(tt);} \
  tt=oz_deref(tt);}

OZ_Return translateWatcherConds(TaggedRef tr,EntityCond &ec){
  TaggedRef car,cdr;
  ec=ENTITY_NORMAL;
  
  cdr=tr;
  DerefVarTest(cdr);
  while(!oz_isNil(cdr)){
    if(!oz_isLTuple(cdr)){
      return IncorrectFaultSpecification;}
    car=tagged2LTuple(cdr)->getHead();
    cdr=tagged2LTuple(cdr)->getTail();
    DerefVarTest(car);
    DerefVarTest(cdr);
    ec |= translateWatcherCond(car);}
  if(ec == ENTITY_NORMAL) ec=UNREACHABLE;
  return PROCEED;
}

TaggedRef listifyWatcherCond(EntityCond ec){
  TaggedRef list = oz_nil();
  if(ec & PERM_BLOCKED)
    list = oz_cons(AtomPermBlocked, list);
    ec = ec & ~PERM_BLOCKED;
  if(ec & TEMP_BLOCKED){
    list = oz_cons(AtomTempBlocked, list);
    ec = ec & ~TEMP_BLOCKED;}
  if(ec & PERM_ME)
    {list = oz_cons(AtomPermWillBlock, list);
    ec = ec & ~PERM_ME;}
  if(ec & TEMP_ME){
    list = oz_cons(AtomTempWillBlock, list);
    ec = ec & ~TEMP_ME;}
  if(ec & PERM_SOME){
    list = oz_cons(AtomPermSome, list);
    ec = ec & ~PERM_SOME;}
  if(ec & TEMP_SOME){
    list = oz_cons(AtomTempSome, list);
    ec = ec & ~TEMP_SOME;}
  if(ec & PERM_ALL){
    list = oz_cons(AtomPermAll, list);
    ec = ec & ~PERM_ALL;}
  if(ec & TEMP_ALL){
    list = oz_cons(AtomTempAll, list);
    ec = ec & ~TEMP_ALL;}
  Assert(ec==0);
  return list;}

/**********************************************************************/
/*   new                          */
/**********************************************************************/

inline OZ_Return checkWatcherConds(EntityCond ec,EntityCond allowed){
  if((ec & ~allowed) != ENTITY_NORMAL) return IncorrectFaultSpecification;
  return PROCEED;}


#define FeatureTest(cond,tt,atom) { \
   tt = cond->getFeature(OZ_atom(atom)); \
   if(tt==0) {return IncorrectFaultSpecification;}}

OZ_Return checkRetry(SRecord *condStruct,short &kind){
  TaggedRef aux;

  aux = condStruct->getFeature(OZ_atom("prop"));
  if(aux==0) {
    return PROCEED;}
  DerefVarTest(aux);
  if(aux==AtomRetry){
    kind |= WATCHER_RETRY;    
    return PROCEED;}
  if(aux!=AtomSkip){
    return IncorrectFaultSpecification;}
  return PROCEED;
}

// the following should check for the existence of not allowed features 
  
OZ_Return distHandlerInstallHelp(SRecord *condStruct, 
		     EntityCond& ec,Thread* &th,TaggedRef &entity,short &kind){
  kind=0;
  ec=ENTITY_NORMAL;
  entity=0;
  th=NULL;

  TaggedRef aux,aux2;

  FeatureTest(condStruct,aux,"cond");

  OZ_Return ret = translateWatcherConds(aux,ec);
  if(ret!=PROCEED) return  ret;

  TaggedRef label=condStruct->getLabel();

  if(label==AtomInjector){
    kind |= (WATCHER_PERSISTENT|WATCHER_INJECTOR);
    ret=checkWatcherConds(ec,PERM_BLOCKED|TEMP_BLOCKED);
    if(ret!=PROCEED) return ret;
    FeatureTest(condStruct,aux,"entityType");  
    DerefVarTest(aux);
    if(aux==AtomAll) {
      entity=0;
      kind |= WATCHER_SITE_BASED;
      FeatureTest(condStruct,aux,"thread");  
      DerefVarTest(aux);
      if(aux!=AtomAll){return IncorrectFaultSpecification;}
      return checkRetry(condStruct,kind);}

    if(aux!=AtomSingle) {return IncorrectFaultSpecification;}
    FeatureTest(condStruct,aux,"entity");
    entity=aux;
    FeatureTest(condStruct,aux,"thread");  
    DerefVarTest(aux);
    if(aux==AtomAll){ 
      th=NULL;
      kind |= WATCHER_SITE_BASED;
      return checkRetry(condStruct,kind);}
    if(aux==AtomThis){
      th=oz_currentThread();
      return checkRetry(condStruct,kind);}      
    if(!oz_isThread(aux)) {return IncorrectFaultSpecification;}      
    th=oz_ThreadToC(aux);
    return checkRetry(condStruct,kind);}

  if(label==AtomSiteWatcher){
    FeatureTest(condStruct,aux,"entity");    
    entity=aux;
    return checkWatcherConds(ec,PERM_BLOCKED|TEMP_BLOCKED|PERM_ME|TEMP_ME);}

  if(label!=AtomNetWatcher) {return IncorrectFaultSpecification;}
  FeatureTest(condStruct,aux,"entity");    
  entity=aux;
  return checkWatcherConds(ec,PERM_SOME|TEMP_SOME|PERM_ALL|TEMP_ALL);
}


OZ_Return DistHandlerInstall(SRecord *condStruct, TaggedRef proc){
			    
  EntityCond ec;
  short kind;
  Thread *th;
  OZ_Return ret;
  TaggedRef entity;

  ret=distHandlerInstallHelp(condStruct,ec,th,entity,kind);
  if(ret!=PROCEED) return ret;
  if(entity==0){
    return installGlobalWatcher(ec,proc,kind);}
  DEREF(entity,vs_ptr,vs_tag);
  if(!isVariableTag(vs_tag)){
    Tertiary* tert = tagged2Tert(entity);
    return installWatcher(tert,ec,proc,th,kind);}
  return installWatcher(vs_ptr,ec,proc,th,kind);
}

OZ_Return DistHandlerDeInstall(SRecord *condStruct, TaggedRef proc){
			    
  EntityCond ec;
  short kind;
  Thread *th;
  TaggedRef entity;
  OZ_Return ret;

  ret=distHandlerInstallHelp(condStruct,ec,th,entity,kind);
  if(ret!=PROCEED) return ret;
  if(entity==0){
    return deInstallGlobalWatcher(ec,proc,kind);}

  DEREF(entity,vs_ptr,vs_tag);
  if(!isVariableTag(vs_tag)){
    Tertiary* tert = tagged2Tert(entity);
    return deinstallWatcher(tert,ec,proc,th,kind);}
  return deinstallWatcher(vs_ptr,ec,proc,th,kind);
}

/**********************************************************************/
/*   gc                           */
/**********************************************************************/

EntityInfo* gcEntityInfoInternal(EntityInfo *info){
  if(info==NULL) return NULL;
  EntityInfo *newInfo = (EntityInfo *) OZ_hrealloc(info,sizeof(EntityInfo));  
  newInfo->gcWatchers();
  return newInfo;}

void gcEntityInfoImpl(Tertiary *t) {
  t->setInfo(gcEntityInfoInternal(t->getInfo()));}

void gcTwins(){
  Twin** base=&usedTwins;
  Twin* aux;
  while(*base!=NULL){
    if((*base)->hasGCMark()){
      (*base)->resetGCMark();
      base= &((*base)->next);}
    else{
      aux=*base;
      base= &((*base)->next);
      aux->free();}}
}

void EntityInfo::gcWatchers(){
  Watcher **base=&watchers;
  Watcher *w=*base;
  Thread* nth=NULL;
  while(w!=NULL){
    if(w->twin!=NULL){
      if((!(w->isPersistent())) && w->isFired()){
	*base= w->next;
	w=*base;
	continue;}}
    if(w->isInjector()){
      nth= w->thread->gcThread();
      if(((nth==NULL) && !(w->isSiteBased()))){
	*base= w->next;
	w=*base;
	continue;}}
    if(w->twin!=NULL){
      w->twin->setGCMark();}

    Watcher* newW=(Watcher*) OZ_hrealloc(w,sizeof(Watcher));
    *base=newW;
    newW->thread=nth;
    OZ_collectHeapTerm(newW->proc,newW->proc);
    base= &(newW->next);
    w=*base;}
}

void gcGlobalWatcher(){
  if(globalWatcher==NULL) return;
  globalWatcher = (Watcher*) OZ_hrealloc(globalWatcher,sizeof(Watcher));
  OZ_collectHeapTerm(globalWatcher->proc,globalWatcher->proc);}

// called from gc
void maybeUnask(Tertiary* t){
  adjustProxyForFailure(t,getSummaryWatchCond(t),ENTITY_NORMAL);}

void EntityInfo::dealWithWatchers(TaggedRef tr,EntityCond ec){
  Watcher **base=getWatcherBase();
  while((*base)!=NULL){
    if((ec & (*base)->watchcond) && !(*base)->isInjector()){
      (*base)->invokeWatcher(tr,ec);
      base= &((*base)->next);}
    else
      base= &((*base)->next);}
}

/**********************************************************************/
/*   SeifHandler                                                      */
/**********************************************************************/

DSite* gBTI(int i){
  return BT->getBorrow(i)->getNetAddress()->site;}

OZ_BI_define(BIseifHandler,2,0){
  oz_declareIN(0,entity);
  oz_declareIN(1,what);
  return oz_raise(E_ERROR,E_DISTRIBUTION,"default",2,entity,what);
}OZ_BI_end


/**********************************************************************/
/*   Handover */
/**********************************************************************/

enum CompWatchers{
  COMPARE_DIFFERENT,
  COMPARE_EQUAL,
  COMPARE_UNEQUAL,
  COMPARE_GREATER,
  COMPARE_LESS};

inline Bool isSubset(EntityCond one,EntityCond two){
  if(one & (~two) ==ENTITY_NORMAL) return TRUE;
  return FALSE;}

int compareWatchers(Watcher* one,Watcher* two){
  Assert(one->isInjector());
  Assert(two->isInjector());
  if(one->isSiteBased()){
    if(!two->isSiteBased()){ return COMPARE_DIFFERENT;}
    if(one->watchcond == two->watchcond){
      if(one->proc==two->proc) return COMPARE_EQUAL;
      return COMPARE_UNEQUAL;}
    if(isSubset(one->watchcond,two->watchcond))
      return COMPARE_LESS;
    return COMPARE_GREATER;}
  if(two->isSiteBased()){ return COMPARE_DIFFERENT;}
  if(two->thread!=one->thread) { return COMPARE_DIFFERENT;}
  if(one->watchcond == two->watchcond){
    if(one->proc==two->proc) return COMPARE_EQUAL;
    return COMPARE_UNEQUAL;}
  if(isSubset(one->watchcond,two->watchcond))
    return COMPARE_LESS;
  return COMPARE_GREATER;
}

Watcher* removeWatcher(Watcher* w,Watcher* l){
  if(l==w) return l->next;
  Watcher** base= &(l->next);
  while(TRUE){
    Assert((*base)!=NULL);
    if((*base)==w){
      *base=w->next;
      return l;}
    base= &((*base)->next);}
  return l;
}

Watcher* mergeWatchers(Watcher* oW,Watcher *nW){
  Watcher *w,*toBe,*aux;
  int comp;
  w=oW;
  toBe=nW;
  while(w!=NULL){
    if(w->isInjector()) {
      aux=toBe;
      while(aux!=NULL){
	if(aux->isInjector()){
	  comp=compareWatchers(w,aux);
	  if(comp!=COMPARE_DIFFERENT) break;}
	aux=aux->next;}
      if((comp==COMPARE_LESS) || (comp==COMPARE_EQUAL)){
	w=w->next;
	continue;}}
    aux=w->next;
    toBe=basicInsertWatcher(w,toBe);
    w=aux;}
  return toBe;
}      

void maybeHandOver(EntityInfo* oldE, TaggedRef t){
  if(oldE==NULL) return;
  if(oldE->watchers==NULL) return;
  EntityInfo *newE;
  DEREF(t,vs_ptr,vs_tag);
  if(isVariableTag(vs_tag)){
    newE=varMakeOrGetEntityInfo(vs_ptr);
    newE->watchers=mergeWatchers(oldE->watchers,newE->watchers);
    return;}
  if(!oz_isConst(t)) return;
  ConstTerm* c=tagged2Const(t);
  Bool flag=NO;
  switch(c->getType()){
  case Co_Cell:
  case Co_Lock: break;
  case Co_Object:flag=OK;
  case Co_Port:break;
  default: return;}
  Tertiary* tert=(Tertiary*)c;
  newE=tertiaryMakeOrGetEntityInfo(tert);
  newE->watchers=mergeWatchers(oldE->watchers,newE->watchers);
  if(flag) transferWatchers((Object*)c);
}



