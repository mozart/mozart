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

#include "dsite.hh"
#include "fail.hh"
#include "perdio.hh"
#include "table.hh"
#include "chain.hh"
#include "state.hh"
#include "protocolFail.hh"
#include "protocolState.hh"
#include "port.hh"


Twin *usedTwins;
Watcher* globalWatcher;

/**********************************************************************/
/*   forward                                   */
/**********************************************************************/

void adjustProxyForFailure(Tertiary*,EntityCond,EntityCond);
void adjustManagerForFailure(Tertiary*,EntityCond,EntityCond);

/**********************************************************************/
/*   support                                   */
/**********************************************************************/


void deferEntityProblem(Tertiary* t){
  Assert(0); // ERIK-LOOK
}

void deferManagerProbeFault(Tertiary* t,DSite* s){
  Assert(0); // ERIK-LOOK
}

void deferProxyProbeFault(Tertiary* t){
  Assert(0); // ERIK-LOOK
}

void tertiaryInstallProbe(DSite* s,ProbeType pt,Tertiary *t){
  if(s==myDSite) return;
  ProbeReturn pr=installProbe(s,pt);
  if(pr==PROBE_INSTALLED) return;
  if(t->isManager())
    deferManagerProbeFault(t,s);
  else
    deferProxyProbeFault(t);}

void managerInstallProbe(Tertiary* t,ProbeType pt){
  installProbeNoRet(BT->getOriginSite(t->getIndex()),pt);}

void managerDeInstallProbe(Tertiary* t,ProbeType pt){
  BT->getOriginSite(t->getIndex())->deinstallProbe(pt);}

void genRemoveWatcher(Tertiary* t,Watcher* w){
  EntityCond oldC=getSummaryWatchCond(t);
  Watcher** base=getWatcherBase(t);
  while((*base)!=w){
    base=&((*base)->next);
    Assert((*base)!=NULL);}
  *base= w->next;
  EntityCond newC=getSummaryWatchCond(t);
  if(t->getTertType()==Te_Manager)
    adjustManagerForFailure(t,oldC,newC);
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

/**********************************************************************/
/*   insert and remove watchers */
/**********************************************************************/

Bool checkForExistentHandler(Tertiary *t,Thread* th,EntityCond wc){
  if(t==NULL){
    Assert(isHandlerCondition(wc));
    return (globalWatcher!=NULL) ;}
  if(!isHandlerCondition(wc)) return FALSE;
  Watcher *w=getWatchersIfExist(t);
  while(w!=NULL){
    if(w->isHandler() && w->thread==th) return TRUE;
    w=w->next;}
  return FALSE;}

void insertWatcherLocal(Tertiary *t,Watcher *w){
  EntityInfo* info=t->getInfo();
  if(info==NULL){
    info=new EntityInfo(w);
    t->setInfo(info);
    return;}
  w->next=info->watchers;
  info->watchers=w;
}

void insertWatcher(Tertiary *t,Watcher *w, EntityCond &oldEC, EntityCond &newEC){
  EntityInfo* info=t->getInfo();
  if(info==NULL){
    info=new EntityInfo(w);
    t->setInfo(info);
    oldEC=ENTITY_NORMAL;
    newEC=w->watchcond;
    return;}
  oldEC=getSummaryWatchCond(t);
  w->next=info->watchers;
  info->watchers=w;
  newEC=getSummaryWatchCond(t);
}

/**********************************************************************/
/*   entityProblem                            */
/**********************************************************************/

PendThread* threadTrigger(Tertiary* t,Watcher* w){
  PendThread* aux;
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock:{
    PendThread* pd=getPendThreadStartFromCellLock(t);
    while(TRUE){
      while((pd!=NULL) && (pd->thread!=NULL)){
        pd=pd->next;}
      if(pd==NULL) return NULL;
      if(w->thread==NULL || (pd->thread== w->thread)){
        return pd;}
      pd=pd->next;
      if(pd==NULL) break;}
    return NULL;}
  case Co_Port:
    Assert(0); // ERIK-LOOK
  default:
    Assert(0);
  }}

void dealWithContinue(Tertiary* t,PendThread* pd){
  switch(t->getType()){
  case Co_Cell:{
    switch(pd->exKind){
    case EXCHANGE:{ // PER-LOOK maybe exchange old ,nw
      pd->thread->pushCall(BI_exchangeCell,makeTaggedTert(t),
                                pd->old, pd->nw);
      return;}
    case ASSIGN:{
      pd->thread->pushCall(BI_assign,pd->old,pd->nw);
      return;}
    case AT:{
      pd->thread->pushCall(BI_atRedo,pd->old,pd->nw);
      return;}
    default: Assert(0);}}
  case Co_Lock:{
    pd->thread->pushCall(BI_lockLock,makeTaggedTert(t));
    return;}
  case Co_Port:
    Assert(0); // ERIK-LOOK
  default:
    Assert(0);
  }}

// returns TRUE if watcher to be removed
Bool entityProblemPerWatcher(Tertiary*t, Watcher* w){
  EntityCond ec=getEntityCond(t) & w->watchcond;
  if(ec == ENTITY_NORMAL) return FALSE;
  if(isHandlerCondition(ec)){
    PendThread* pd=threadTrigger(t,w);
    if(pd!=NULL){
      if(w->isRetry()) dealWithContinue(t,pd);
      w->invokeHandler(ec,pd->controlvar);
      if(w->isPersistent()) return FALSE;
      return TRUE;}
    return FALSE;}
  w->invokeWatcher(ec);
  return TRUE;}

Bool entityCondMeToBlocked(Tertiary* t){
  if(getEntityCond(t) & PERM_ME|TEMP_ME) {
    if(getEntityCond(t) & PERM_ME)
      addEntityCond(t,PERM_BLOCKED);
    if(getEntityCond(t) & TEMP_ME)
      addEntityCond(t,TEMP_BLOCKED);
    return TRUE;}
  return FALSE;}

void entityProblem(Tertiary *t) {
  PD((ERROR_DET,"entityProblem invoked"));
  Watcher* w;
  Bool hit=FALSE;
  if(errorIgnore(t)) {
    if(globalWatcher==NULL) return;
    w= NULL;}
  else{
    Watcher **aux=getWatcherBase(t);
    if(aux==NULL){
      if(globalWatcher==NULL) return;
      w=NULL;}
    else w= *aux;}

  while(w!=NULL){
    if(w->watchcond & getEntityCond(t)){
      if(entityProblemPerWatcher(t,w)){
        hit=TRUE;
        Watcher *aux=w->next;
        genRemoveWatcher(t,w);
        w=aux;}
      else w=w->next;}
    else w=w->next;}

  if((!hit) && (globalWatcher!=NULL))
    entityProblemPerWatcher(t,globalWatcher);
}

/**********************************************************************/
/*   SECTION::  watcher                */
/**********************************************************************/

void Watcher::invokeWatcher(EntityCond ec){
  if(!isFired()){
    Assert(!isHandler());
    Thread *tt = oz_newThreadToplevel(DEFAULT_PRIORITY);
    tt->pushCall(proc, listifyWatcherCond(ec));}
}

void Watcher::invokeHandler(EntityCond ec,TaggedRef controlvar){
  if(!isFired()){
    Assert(isHandler());
    thread->pushCall(proc,listifyWatcherCond(ec));
    ControlVarResume(controlvar);}
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

void proxyProbeFault(Tertiary *t, int pr) {
  PD((ERROR_DET,"proxy probe invoked %d",pr));
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock:
    cellLockProxyProbeFault(t,pr);
    return;
  case Co_Port:
    Assert(pr==PROBE_PERM);
    return;              // ERIK-LOOK;
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
  case Co_Port: //ERIK-LOOK
    break;
  default:
    Assert(0);}
}

void DSite::probeFault(ProbeReturn pr) {
  PD((PROBES,"PROBEfAULT  site:%s",stringrep()));
  int limit=OT->getSize();
  for(int ctr = 0; ctr<limit;ctr++){
    OwnerEntry *oe = OT->getEntry(ctr);
    if(oe==NULL) continue;
    if(oe->isTertiary()){
      Tertiary *tr=oe->getTertiary();
      PD((PROBES,"Informing Manager"));
      Assert(tr->isManager());
      managerProbeFault(tr,this,pr);}
    else{
      // TO_BE_IMPLEMENTED vars
    }}

  limit=BT->getSize();
  for(int ctr1 = 0; ctr1<limit;ctr1++){
    BorrowEntry *be = BT->getEntry(ctr1);
    if(be==NULL) continue;
    if(be->isTertiary()){
      Tertiary *tr=be->getTertiary();
      if(be->getSite()){
        proxyProbeFault(tr,pr);}}}} // TO_BE_IMPLEMENTED vars

/**********************************************************************/
/*   SECTION::              chain and error                            */
/**********************************************************************/

void Chain::establish_PERM_SOME(Tertiary* t){
  if(hasFlag(TOKEN_PERM_SOME)) return;
  setFlag(TOKEN_PERM_SOME);
  int OTI=t->getIndex();
  informHandle(OT->getOwner(OTI),OTI,PERM_SOME);
  addEntityCond(t,PERM_SOME);
  entityProblem(t);}

void Chain::establish_TOKEN_LOST(Tertiary* t){
  setFlagAndCheck(TOKEN_LOST);
  int OTI=t->getIndex();
  informHandle(OT->getOwner(OTI),OTI,PERM_SOME|PERM_ME);
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
  setFlagAndCheck(TOKEN_LOST);
  establish_TOKEN_LOST(t);
  int OTI=t->getIndex();
  handleTokenLost(t,OT->getOwner(OTI),OTI);
  Assert(inform==NULL);
  return;
}

void InformElem::maybeTrigger(OwnerEntry* oe, int index, EntityCond ec){
  EntityCond xec= ec & watchcond;
  xec &= ~foundcond;
  if(xec == ENTITY_NORMAL) return;
  foundcond |= xec;
  sendTellError(oe,site,index,xec,TRUE);
}

void InformElem::maybeTriggerOK(OwnerEntry* oe, int index, EntityCond ec){
  EntityCond xec= ec & foundcond;
  if(xec == ENTITY_NORMAL) return;
  foundcond &= ~xec;
  sendTellError(oe,site,index,xec,FALSE);
}

  // approximative PER-LOOK
void Chain::managerSeesSiteTemp(Tertiary *t,DSite* s){
  EntityCond ec;
  int index=t->getIndex();
  OwnerEntry *oe=OT->getOwner(index);
  PD((ERROR_DET,"managerSeesSiteTemp site:%s nr:%d",
      s->stringrep(),index));

  InformElem *cur=inform;  // deal with TEMP_SOME|TEMP_ME watchers
  while(cur!=NULL){
    cur->maybeTrigger(oe,index, (TEMP_SOME|TEMP_ME));
    cur=cur->next;}

  ChainElem *ce=findAfter(s); // deal with TEMP_BLOCKED handlers
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

  if(!(tempConnectionInChain())){
    InformElem *cur=inform;  // deal with TEMP_SOME|TEMP_ME watchers
    while(cur!=NULL){
      cur->maybeTriggerOK(oe,index, (TEMP_SOME|TEMP_ME));
      cur=cur->next;}}

  ChainElem *ce=findAfter(s); // deal with TEMP_BLOCKED handlers
  while(ce!=NULL){
    if(ce->getSite()->siteStatus()==SITE_TEMP) break;
    sendTellError(oe,ce->getSite(),index,TEMP_BLOCKED,FALSE);
    ce=ce->next;}

  if(!tempConnectionInChain()){
    resetFlagAndCheck(INTERESTED_IN_OK);}
}

/**********************************************************************/
/**********************************************************************/

void proxyInform(Tertiary* t,EntityCond ec){
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock: break;
  case Co_Port: return;
  default: NOT_IMPLEMENTED;}
  if(someTempCondition(ec)){
    managerInstallProbe(t,PROBE_TYPE_ALL);}
  else{
    managerInstallProbe(t,PROBE_TYPE_PERM);}
  sendAskError(t,ec);}

void proxyDeInform(Tertiary* t,EntityCond ec){
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock: break;
  case Co_Port: return;
  default: NOT_IMPLEMENTED;}
  if(someTempCondition(ec)){
    managerDeInstallProbe(t,PROBE_TYPE_ALL);}
  else{
    managerDeInstallProbe(t,PROBE_TYPE_PERM);}
  sendUnAskError(t,ec);}

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

ProbeType managerProbePart(Tertiary* t, EntityCond ec){
  Assert(!(t->isLocal));
  Assert(!(t->isManager));
  switch(t->getType()){
  case Co_Lock:
  case Co_Cell:
    if(ec & (TEMP_ME|TEMP_SOME)) return PROBE_TYPE_ALL;
    if(ec & (PERM_ME|PERM_SOME)) return PROBE_TYPE_PERM;
    return PROBE_TYPE_NONE;
 case Co_Port:
    if(ec & (TEMP_ME)) return PROBE_TYPE_ALL;
    if(ec & (PERM_ME)) return PROBE_TYPE_PERM;
    return PROBE_TYPE_NONE;
 default:
    Assert(0);}
  return PROBE_TYPE_NONE;
}

void adjustProxyForFailure(Tertiary*t, EntityCond oldEC, EntityCond newEC){
  if(askPart(t,newEC)!=askPart(t,oldEC)){
    if(askPart(t,oldEC)!=ENTITY_NORMAL)
      proxyDeInform(t,askPart(t,oldEC));
    proxyInform(t,askPart(t,newEC));}
  if(managerProbePart(t,oldEC)!=managerProbePart(t,newEC)){
    if(someTempCondition(managerProbePart(t,oldEC)))
      getSiteFromTertiaryProxy(t)->deinstallProbe(PROBE_TYPE_ALL);
    else
      getSiteFromTertiaryProxy(t)->deinstallProbe(PROBE_TYPE_PERM);
    if(managerProbePart(t,newEC) != ENTITY_NORMAL){
      if(someTempCondition(managerProbePart(t,newEC))){
        tertiaryInstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_ALL,t);}
      else
        tertiaryInstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_PERM,t);}}
}

// current scheme doesn't need anything else - VARIABLES will
void initManagerForFailure(Tertiary* t){
  return;
}

void adjustManagerForFailure(Tertiary* t,EntityCond oldEC, EntityCond newEC){
  return;
}

/**********************************************************************/
/*   SECTION::       installation/deinstallation utility             */
/**********************************************************************/

/*
  INSTALL:

  wc if TEMP_X set then so it PERM_X
  PER ENTITY:
   ordinary watcher (t!= NULL thread = NULL, isRetry=FALSE, isPersistent=FALSE)
                     wc: PERM_BLOCKED and TEMP_BLOCKED not set
   thread handler   (t!= NULL thread !=NULL, isRetry=?, isPersistent=?)
   site handler     (t!= NULL thread = NULL, isRetry=?, isPersistent=?)
                     wc: only PERM_BLOCKED and/or TEMP_BLOCKED are set

  ALL ENTITY:
   site handler     (t==NULL, thread =NULL, isRetry=FALSE, isPersistent=TRUE)
                     wc: only PERM_BLOCKED and/or TEMP_BLOCKED are set

  DEINSTALL:

  wc if TEMP_X set then so it PERM_X
  PER ENTITY:
   ordinary watcher (t!= NULL thread = NULL, isRetry=FALSE, isPersistent=FALSE)
                     wc: PERM_BLOCKED and TEMP_BLOCKED not set
   thread handler   (t!= NULL thread !=NULL, isRetry=?, isPersistent=?)
   site handler     (t!= NULL thread = NULL, isRetry=?, isPersistent=?)
                     wc: only PERM_BLOCKED and/or TEMP_BLOCKED are set

  ALL ENTITY:
   site handler     (t!=NULL, thread =NULL, isRetry=FALSE, isPersistent=TRUE)
*/

// only debug
Bool checkWatcherCorrectness(Tertiary* t, EntityCond wc,
                             Thread* th, unsigned int kind){
// global handler
  if(t==NULL) {
    if(!(kind & PERSISTENT)) return FALSE;
    if(kind & RETRY) return FALSE;
    if(th!=NULL) return FALSE;
    if(!isHandlerCondition(wc)) return FALSE;
    return TRUE;}

// handler
  if(isHandlerCondition(wc)){
    if(t==NULL) return FALSE;
    return TRUE;}

  // watcher
  if(th!=NULL) return FALSE;
  if(kind & PERSISTENT) return FALSE;
  if(kind & RETRY) return FALSE;
  return TRUE;
}

Bool installWatcher(Tertiary* t,EntityCond wc,TaggedRef proc,
                    Thread* th, unsigned int kind) {

  if(checkForExistentHandler(t,th,wc)) return FALSE;
    Watcher *w=new Watcher(proc,th,wc,kind);
  if(t==NULL){
    PD((NET_HANDLER,"Watcher installed globally old was %x",globalWatcher));
    globalWatcher=w;
    return TRUE;}

  PD((NET_HANDLER,"Watcher installed on tertiary %x",t));
  if(t->getType()==Co_Object){
    Object* o=(Object*)t;
    if(t->isLocal()) cellifyObject(o);
    LockManager *lm=(LockManager*) o->getLock();
    CellManager *cm=(CellManager*) o->getState();
    if(checkForExistentHandler(t,th,wc)) return FALSE;
    Watcher *tw=new Watcher(proc,th,wc,kind);
    Assert(cm->getType()==Co_Cell);
    Twin *twin=w->cellTwin();
    tw->lockTwin(twin);
    if(lm->isLocal()) {
      insertWatcherLocal(cm,w);
      insertWatcherLocal(lm,w);
      Assert(cm->isLocal());
      return TRUE; }
    EntityCond oldC,newC;
    EntityCond oldL,newL;
    insertWatcher(cm,w,oldC,newC);
    insertWatcher(lm,tw,oldL,newL);
    adjustManagerForFailure(cm,oldC,newC);
    adjustManagerForFailure(lm,oldC,newC);
    if(w->isTriggered(getEntityCond(lm))) entityProblem(lm);
    if(w->isTriggered(getEntityCond(cm))) entityProblem(cm);
    return TRUE;}

  EntityCond oldC,newC;
  if(t->isLocal()){
    insertWatcherLocal(t,w);
    return TRUE;}
  insertWatcher(t,w,oldC,newC);
  if(t->isManager())
    adjustManagerForFailure(t,oldC,newC);
  else{
    adjustProxyForFailure(t,oldC,newC);}
  if(w->isTriggered(getEntityCond(t))) deferEntityProblem(t);
  return TRUE;
}

Bool deinstallWatcher(Tertiary* t,EntityCond wc,TaggedRef proc,
                      Thread* th, unsigned int kind){
  if(t->getType()==Co_Object){
    Object* o= (Object*) t;
    if(!stateIsCell(o->getState())) return FALSE;
    if(deinstallWatcher(o->getLock(),wc,proc,th,kind)){
      PD((NET_HANDLER,"Watcher on object deinstalled"));
      Bool ret=deinstallWatcher(getCell(o->getState()),
                                wc,proc,th,kind);
      Assert(ret==TRUE);
      return TRUE;}
    return FALSE;}

  if(t==NULL){
    if(globalWatcher->matches(proc,th,wc,kind)){
      PD((NET_HANDLER,"Watcher deinstalled globally old"));
      globalWatcher=NULL;
      return TRUE;}
    return FALSE;}

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

  if(!found) return FALSE;

  PD((NET_HANDLER,"Watcher deinstalled"));
  EntityCond newEC=getSummaryWatchCond(t);

  if(t->isLocal()) return TRUE;
  if(t->isProxy())
    adjustProxyForFailure(t,oldEC,newEC);
  else
    adjustManagerForFailure(t,oldEC,newEC);
  return TRUE;
}

/**********************************************************************/
/*   SECTION::              user interface                            */
/**********************************************************************/


EntityCond translateWatcherCond(TaggedRef tr){
  if(tr==AtomPermHome)
    return PERM_ME;
 if(tr== AtomTempHome)
   return TEMP_ME;
  if(tr== AtomPermAllOthers)
    return PERM_ALL;
  if(tr== AtomTempAllOthers)
    return TEMP_ALL;
  if(tr== AtomPermSomeOther)
    return PERM_SOME;
  if(tr== AtomTempSomeOther)
    return TEMP_SOME;
  if(tr== AtomPermBlocked)
    return PERM_BLOCKED;
  if(tr== AtomTempBlocked)
    return TEMP_BLOCKED;
  Assert(0);
  return 0;
}

OZ_Return translateWatcherConds(TaggedRef tr,EntityCond &ec){
  TaggedRef car;
  ec=ENTITY_NORMAL;

  NONVAR(tr,cdr);

  while(!oz_isNil(cdr)){
    if(!oz_isLTuple(cdr)){
      NONVAR(cdr,tmp);
      ec |= translateWatcherCond(cdr);
      return PROCEED;}
    car=tagged2LTuple(cdr)->getHead();
    cdr=tagged2LTuple(cdr)->getTail();
    NONVAR(cdr,tmp);
    cdr = tmp;
    ec |= translateWatcherCond(car);}
  if(ec==ENTITY_NORMAL) {
    return IncorrectFaultSpecification;}
  return PROCEED;
}


OZ_Return watcherInstallHelp(SRecord *condStruct,
                     EntityCond& ec,Thread* &th,short& kind, Tertiary* &entity){
  TaggedRef aux;
  kind=0;
  aux = condStruct->getFeature(OZ_atom("cond"));
  OZ_Return tr = translateWatcherConds(aux,ec);
  if(tr!=PROCEED) return  tr;

  TaggedRef label=condStruct->getLabel();

  if(label==AtomWatcher){
    th=NULL;
    return PROCEED;}

  if(label!=AtomHandler) {return IncorrectFaultSpecification;}
  aux = condStruct->getFeature(OZ_atom("once_only"));
  if(aux == AtomNo) kind |= PERSISTENT;

  aux = condStruct->getFeature(OZ_atom("retry"));
  if(aux == AtomYes) kind |= RETRY;

  aux  = condStruct->getFeature(OZ_atom("basis"));
  if(aux== AtomPerThread){
    th = oz_currentThread();
    return PROCEED;}
  if(aux == AtomPerSite){
    th = NULL;
    return PROCEED;}
  if(aux==AtomGlobal){
    th = NULL;
    entity = NULL;
    return PROCEED;}
  return IncorrectFaultSpecification;
}


OZ_Return WatcherInstall(Tertiary* entity,SRecord *condStruct,TaggedRef proc){
  EntityCond ec    = ENTITY_NORMAL;
  short kind;
  Thread *th;
  TaggedRef aux;

  return PROCEED; // ERIK-LOOK
  watcherInstallHelp(condStruct,ec,th,kind,entity);
  if(!checkWatcherCorrectness(entity,ec,th,kind))
    {return IncorrectFaultSpecification;}
  if(installWatcher(entity,ec,proc,th,kind)) return PROCEED;
  return IncorrectFaultSpecification;}

OZ_Return WatcherDeInstall(Tertiary *entity, SRecord *condStruct,
                           TaggedRef proc){
  return PROCEED; // ERIK-LOOK PER-LOOK
  EntityCond ec    = ENTITY_NORMAL;
  short kind;
  Thread *th;
  TaggedRef aux;

  watcherInstallHelp(condStruct,ec,th,kind,entity);
  if(!checkWatcherCorrectness(entity,ec,th,kind))
    {return IncorrectFaultSpecification;}

  Assert(checkWatcherCorrectness(entity,ec,th,kind));
  if(deinstallWatcher(entity,ec,proc,th,kind)) return PROCEED;
  return IncorrectFaultSpecification;}

TaggedRef listifyWatcherCond(EntityCond ec){
  TaggedRef list = oz_nil();
  if(ec & PERM_BLOCKED)
    list = oz_cons(AtomPerm, list);
    ec = ec & ~PERM_BLOCKED;
  if(ec & TEMP_BLOCKED){
    list = oz_cons(AtomTemp, list);
    ec = ec & ~TEMP_BLOCKED;}
  if(ec & PERM_ME)
    {list = oz_cons(AtomPermHome, list);
    ec = ec & ~PERM_ME;}
  if(ec & TEMP_ME){
    list = oz_cons(AtomTempHome, list);
    ec = ec & ~TEMP_ME;}
  if(ec & PERM_SOME){
    list = oz_cons(AtomPermSomeOther, list);
    ec = ec & ~PERM_SOME;}
  if(ec & TEMP_SOME){
    list = oz_cons(AtomTempForeign, list);
    ec = ec & ~TEMP_SOME;}
  if(ec & PERM_ALL){
    list = oz_cons(AtomPermAllOthers, list);
    ec = ec & ~PERM_ALL;}
  if(ec & TEMP_ALL){
    list = oz_cons(AtomTempAllOthers, list);
    ec = ec & ~TEMP_ALL;}
  if(ec & PERM_BLOCKED){
    list = oz_cons(AtomPermAllOthers, list);
    ec = ec & ~PERM_BLOCKED;}
  if(ec & TEMP_BLOCKED){
    list = oz_cons(AtomTempAllOthers, list);
    ec = ec & ~TEMP_BLOCKED;}
  Assert(ec==0);
  return list;}

/**********************************************************************/
/*   gc                           */
/**********************************************************************/

void gcEntityInfoImpl(Tertiary *t) {
  EntityInfo* info = t->getInfo();
  if (info==NULL) return;
  EntityInfo *newInfo = (EntityInfo *) gcRealloc(info,sizeof(EntityInfo));
  t->setInfo(newInfo);
  newInfo->gcWatchers();
}

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
  while(w!=NULL){
    Thread *th = w->thread;
    Thread *nth= th->gcThread();
    if(w->twin!=NULL){
      if((!w->isPersistent()) && (w->isFired())){
        *base= w->next;
        w=*base;
        continue;}
      w->twin->setGCMark();}
    Watcher* newW=(Watcher*) gcRealloc(w,sizeof(Watcher));
    *base=newW;
    newW->thread=nth;
    OZ_collectHeapTerm(newW->proc,newW->proc);
    base= &(newW->next);
    w=*base;}
}

void gcGlobalWatcher(){
    if(globalWatcher==NULL) return;
    Watcher* newW=(Watcher*) gcRealloc(globalWatcher,sizeof(Watcher));
    OZ_collectHeapTerm(globalWatcher->proc,globalWatcher->proc);}
