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

/**********************************************************************/
/*   support                                   */
/**********************************************************************/

/*
void maybeStateError(Tertiary *t,Thread* th){
  if(maybeInvokeHandler(c,th)){ 
    genInvokeHandlerLockOrCell(c,th);}}
*/

void maybeStateError(Tertiary *t,Thread* th){
  return;}



/**********************************************************************/
/*   support                                   */
/**********************************************************************/

void managerProbeFault(Tertiary *t, DSite* s, int pr);
void proxyProbeFault(Tertiary *t, int pr);

void insertWatcher(Tertiary *t,Watcher *w){
  EntityInfo* info=t->getInfo();
  if(info==NULL){
    info=new EntityInfo(w);
    t->setInfo(info);
    return;}
  w->next=info->watchers;
  info->watchers=w;}

Watcher** findWatcherBase(Tertiary* t,Thread* th,EntityCond ec){
  Watcher** def = NULL;
  Watcher** base=getWatcherBase(t);
  while(*base!=NULL){
    if(((*base)->isHandler()) && ((*base)->isTriggered(ec))){
      if((*base)->thread==th)
	return base;
      if((*base)->thread==DefaultThread) 
	def = base;}
    base= &((*base)->next);}
  return def;}

void installProbe(DSite* s,ProbeType pt){
  if(s==myDSite) return;
  s->installProbe(pt,PROBE_INTERVAL);}

void tertiaryInstallProbe(DSite* s,ProbeType pt,Tertiary *t){
  if(s==myDSite) return;
  ProbeReturn pr=s->installProbe(pt,PROBE_INTERVAL);
  if(pr==PROBE_INSTALLED) return;
  if(t->isManager())
    managerProbeFault(t, s, pr);
  else
    proxyProbeFault(t,pr);}

void deinstallProbe(DSite* s,ProbeType pt){
  if(s==myDSite) return;
  s->deinstallProbe(pt);}

void releaseWatcher(Tertiary *t, Watcher* w) {
  if(!t->isManager()){
    EntityCond ec=managerPart(w->watchcond);
    switch(t->getType()){
    case Co_Cell:
    case Co_Lock: {
      ec &= ~(PERM_BLOCKED|PERM_SOME|PERM_ME); // Automatic
      break;}
    case Co_Port:{
      deinstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_PERM);
      return;}
    default: NOT_IMPLEMENTED;}
    Assert(t->getType()!=Co_Port);
    if(ec!=ENTITY_NORMAL) {
      sendUnAskError(t, managerPart(w->watchcond));}
    if(someTempCondition(w->watchcond))
      deinstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_ALL);
    else 
      deinstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_PERM);
  }}

/**********************************************************************/
/*   entityProblem                            */
/**********************************************************************/

Bool deinstallWatcher(Tertiary* t,EntityCond wc, TaggedRef proc);
Bool deinstallHandler(Tertiary* t,Thread *th,TaggedRef proc);

void entityProblem(Tertiary *t) { 
  PD((ERROR_DET,"entityProblem invoked"));
  
  EntityCond ec = getEntityCond(t);
  Watcher** base = getWatcherBase(t);
  
  if(errorIgnore(t)) return;
  if(*base==NULL) return;
  
  Tertiary *other = NULL, *obj = getInfoTert(t);
  if(obj){
    other = getOtherTertFromObj(obj, t);
    Assert(obj->getType() == Co_Object);}
  PendThread *pd;

  if(t->isProxy())
    pd = NULL;
  else{
    if(t->getType()==Co_Cell){
      if(t->isFrame())
	pd = *(((CellFrame *)t)->getCellSec()->getPendBase());
      else
	pd = *(((CellManager *)t)->getCellSec()->getPendBase());}
    if(t->getType()==Co_Lock){
      if(t->isFrame())
	pd = *(((LockFrame *)t)->getLockSec()->getPendBase());
      else
	pd = *(((LockManager *)t)->getLockSec()->getPendBase());}}
  
  while(pd!=NULL){
    if(isRealThread(pd->thread)){
    Watcher **ww = findWatcherBase(t, pd->thread, ec);
    Thread *cThread = pd->thread;
    if(ww!=NULL){
      Watcher *w = *ww;
      Assert(!t->isProxy());
      pd->thread = DummyThread;
      if(w->isContinueHandler()){
	Assert(cThread!=oz_currentThread());
	if(t->getType()==Co_Cell){
	  switch(pd->exKind){
	  case EXCHANGE:{cThread->pushCall(BI_exchangeCell,makeTaggedTert(t), pd->nw, pd->old); break;}
	  case ASSIGN:{cThread->pushCall(BI_assign,pd->old,pd->nw); break;}
	  case AT:{cThread->pushCall(BI_atRedo,pd->old,pd->nw); break;}
	  default: Assert(0);}}
      if(t->getType()==Co_Lock)
	cThread->pushCall(BI_lockLock,makeTaggedTert(t));}
      
      if(obj)
	w->invokeHandler(ec,obj,cThread,pd->controlvar);
      else
	w->invokeHandler(ec,t,cThread,pd->controlvar);
      if(!w->isPersistent()){
	if(obj && other) deinstallHandler(other,cThread,AtomAny);
	*ww = w->next;
	releaseWatcher(t, w);}
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
	if (other) deinstallWatcher(other, w->watchcond, w->proc);}
      else
	w->invokeWatcher(ec,t);
      if(w->isPersistent()){
	Mec &= ~(w->getWatchCond() & (TEMP_BLOCKED|TEMP_ME|TEMP_SOME));
	base= &(w->next);
	w=*base;} 
      else{  
	releaseWatcher(t, w);
	*base=w->next;
	w=*base;}}
    resetEntityCondManager(t, Mec);
}

/**********************************************************************/
/*   SECTION::  watcher                */
/**********************************************************************/

void Watcher::invokeWatcher(EntityCond ec,Tertiary* entity){
  Assert(!isHandler());
  Thread *tt = oz_newThreadToplevel(DEFAULT_PRIORITY);
  tt->pushCall(proc, makeTaggedTert(entity), listifyWatcherCond(ec));
}

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


/**********************************************************************/
/*   SECTION::  probeFault -- first indication of error                */
/**********************************************************************/

void managerProbeFault(Tertiary *t, DSite* s, int pr){
  PD((ERROR_DET,"Mgr probe invoked %d",pr));    
  switch (t->getType()) {
  case Co_Cell:
  case Co_Lock:{
    Chain *ch=getChainFromTertiary(t);
    if(pr==PROBE_OK){
      if(!ch->hasFlag(INTERESTED_IN_OK)) return;
      if(!ch->siteOfInterest(s)) return;
      ch->managerSeesSiteOK(t,s);
      return;}
    if(pr==PROBE_TEMP){
      if(!ch->hasFlag(INTERESTED_IN_TEMP)) return;
      if(!ch->siteOfInterest(s)) return;
      ch->managerSeesSiteTemp(t,s);
      return;}
    if(ch->hasInform()){
      ch->removeInformOnPerm(s);}
    if(!ch->siteOfInterest(s)) return;    
    ch->managerSeesSitePerm(t,s);}
  default: return;  // TO_BE_IMPLEMENTED
    return;}}

void proxyProbeFault(Tertiary *t, int pr) {
  PD((ERROR_DET,"proxy probe invoked %d",pr));
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock:{
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
  case Co_Port:{
    Assert(pr==PROBE_PERM);
    setEntityCondOwn(t,PERM_BLOCKED|PERM_ME);
    /* PER-HANDLE
    startHandlerPort(NULL, t, 0, PERM_BLOCKED|PERM_ME);
    */
  }
  default: return;}}     // TO_BE_IMPLEMENTED

void DSite::probeFault(ProbeReturn pr) {
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
      managerProbeFault(tr, this, pr);}} // TO_BE_IMPLEMENTED vars
  limit=BT->getSize();
  for(int ctr1 = 0; ctr1<limit;ctr1++){
    BorrowEntry *be = BT->getEntry(ctr1);
    if(be==NULL){continue;}
    Assert(be!=NULL);
    if(be->isTertiary()){
      Tertiary *tr=be->getTertiary();
      if((be->getSite() == this && !errorIgnore(tr)) || 
	 getEntityCond(tr)!=ENTITY_NORMAL){
	proxyProbeFault(tr,pr);}}}
  return;}

/**********************************************************************/
/*   SECTION                                                          */
/**********************************************************************/
// PER-LOOK

OZ_BI_define(BIprobe,1,0)
{ 
  OZ_Term e = OZ_in(0);
  NONVAR(e,entity);
  Tertiary *tert = tagged2Tert(entity);
  if(tert->getType()!=Co_Port)
    entityProblem(tert);
  return PROCEED;
} OZ_BI_end

TaggedRef BI_probe;

void insertDangelingEvent(Tertiary *t){
  PD((PROBES,"Starting DangelingThread"));
  Thread *tt = oz_newThreadToplevel(DEFAULT_PRIORITY);
  tt->pushCall(BI_probe, makeTaggedTert(t));
}

/**********************************************************************/
/*   SECTION::              chain and error                            */
/**********************************************************************/

void Chain::dealWithTokenLostBySite(OwnerEntry*oe,int OTI,DSite* s){ 
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
  setEntityCondManager(lm,PERM_SOME);
  entityProblem(lm); 
  ChainElem** base=getFirstNonGhostBase();
  ChainElem *ce;
  LockSec* sec=lm->getLockSec();
  if((*base)->next==NULL){
    LockSec *sec=lm->getLockSec();
    ChainElem *ce=*base;
    ce->init(myDSite);
    Assert(sec->state==Cell_Lock_Invalid);
    sec->state=Cell_Lock_Valid;
    return;}
  removePerm(base);
  ce=getFirstNonGhost();
  if(ce->site==myDSite){
    lockReceiveTokenManager(OT->getOwner(OTI),OTI);
    return;}
  lockSendToken(myDSite,OTI,ce->site);}

void Chain::shortcutCrashCell(CellManager* cm,TaggedRef val){
  setFlag(TOKEN_PERM_SOME);
  int OTI=cm->getIndex();
  informHandle(OT->getOwner(OTI),OTI,PERM_SOME);
  setEntityCondManager(cm,PERM_SOME);
  entityProblem(cm); 
  ChainElem** base=getFirstNonGhostBase();
  ChainElem *ce;
  CellSec* sec=cm->getCellSec();
  if((*base)->next==NULL){
    CellSec *sec=cm->getCellSec();
    ChainElem *ce=*base;
    ce->init(myDSite);
    Assert(sec->state=Cell_Lock_Invalid);
    sec->state=Cell_Lock_Valid;
    sec->contents=val;
    return;}
  removePerm(base);
  ce=getFirstNonGhost();
  int index=cm->getIndex();
  if(ce->site==myDSite){
    cellReceiveContentsManager(OT->getOwner(index),val,index);
    return;}
  OT->getOwner(index)->getOneCreditOwner();
  cellSendContents(val,ce->site,myDSite,index);}

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

void Chain::managerSeesSitePerm(Tertiary *t,DSite* s){
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
  setEntityCondManager(t,PERM_SOME|PERM_ME);
  entityProblem(t);
  handleTokenLost(oe,OTI);
  return;}
  
void Chain::managerSeesSiteTemp(Tertiary *t,DSite* s){
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

void Chain::managerSeesSiteOK(Tertiary *t,DSite* s){
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
/*   SECTION::       installation/deinstallation utility             */
/**********************************************************************/

Bool deinstallHandler(Tertiary* t,Thread *th,TaggedRef proc){
  return OK;}

/*
Bool deinstallHandler(Tertiary* t,Thread *th,TaggedRef proc){
  if(!handlerExistsThread(t,th)){return NO;}
  PD((NET_HANDLER,"Handler deinstalled on tertiary %x",this));  
  EntityCond Mec=(TEMP_BLOCKED|TEMP_ME|TEMP_SOME);
  Watcher** base=getWatcherBase(t);
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
  
*/

void installWatcher(Tertiary* t,EntityCond wc,TaggedRef proc, Bool pr){
  PD((NET_HANDLER,"Watcher installed on tertiary %x",t));
  Watcher *w=new Watcher(proc,wc);
  insertWatcher(t,w);
  if(pr) w->setPersistent();
  if(t->isLocal()) return;
  if(w->isTriggered(getEntityCond(t)) || 
     (t->getType()==Co_Port && w->isTriggered(getEntityCondPort(t)))){
    entityProblem(t);
    return;}
  if(managerPart(wc) != ENTITY_NORMAL && t->getType()!=Co_Port){
    informInstallHandler(t,managerPart(wc));
    return;}
  if(t->isManager()){
    return;}
  if(someTempCondition(wc) && t->getType()!=Co_Port)
    tertiaryInstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_ALL,t);
  else 
    tertiaryInstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_PERM,t);}


Bool deinstallWatcher(Tertiary* t,EntityCond wc, TaggedRef proc){
  Watcher** base=getWatcherBase(t);
  EntityCond Mec=(TEMP_BLOCKED|TEMP_ME|TEMP_SOME);
  Bool found = FALSE;
  while(*base!=NULL){
    if((!((*base)->isHandler())) && 
       ((*base)->getWatchCond() == wc) && 
       (((*base)->proc==proc) || proc==AtomAny || proc==AtomAll)){
      releaseWatcher(t,(*base));
      *base = (*base)->next;
      found = TRUE;
      if(proc == AtomAny) proc = 0;}
    else{ 
      Mec &= ~(((Watcher*)*base)->getWatchCond() & (TEMP_BLOCKED|TEMP_ME|TEMP_SOME));
      base= &((*base)->next);}}
  resetEntityCondManager(t,Mec);
  return found;}

/**********************************************************************/
/*   SECTION::              user interface                            */
/**********************************************************************/

OZ_Return HandlerInstall(Tertiary *entity, SRecord *condStruct,
			 TaggedRef proc) {
  return PROCEED;
  /*
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
      setMasterTert(cell,entity);}
    lock = o->getLock();
    if(cell->installHandler(ec,proc,th,Continue,Persistent)){
      if(lock!=NULL){
	setMasterTert(o->getLock(),entity);
	if(!lock->installHandler(ec,proc,th,Continue,Persistent)){
	  deinstallHandler(cell,th,proc);
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
  */
}


OZ_Return HandlerDeInstall(Tertiary *entity, SRecord *condStruct,TaggedRef proc){
  return PROCEED;
  /*
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
      setMasterTert(cell,entity);}
    lock = o->getLock();
    if(cell->deinstallHandler(th,proc)){
      if(lock!=NULL){
	setMasterTert(o->getLock(),entity);
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
  */
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
      installWatcher(entity,ec,proc,Persistent);
    else{
      cell = getCell(o->getState());
      setMasterTert(cell,entity);
      installWatcher(cell,ec,proc,Persistent);}
    lock = o->getLock();
    if(lock!=NULL){
      setMasterTert(lock,entity);
      installWatcher(lock,ec,proc,Persistent);}
    break;}
  case Co_Port:
    if(!entity->isProxy()) break;
  case Co_Cell:
  case Co_Lock:{
    installWatcher(entity,ec,proc,Persistent);
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
    if(deinstallWatcher(cell,ec,proc)){
      lock = o->getLock();
      if(lock!=NULL || deinstallWatcher(lock,ec,proc))
	return PROCEED;}
    break;}
  case Co_Port:
    if(!entity->isProxy()) return PROCEED;
  case Co_Cell:
  case Co_Lock:{
    if(deinstallWatcher(entity,ec,proc))
      return PROCEED;}
  default: return oz_raise(E_ERROR,E_SYSTEM,"watchers on ? not implemented",0);}
  return oz_raise(E_ERROR,E_SYSTEM,"Watcher Not installed",0);
}
  
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
/*   gc                           */
/**********************************************************************/

void gcEntityInfoImpl(Tertiary *t) {
  EntityInfo* info = t->getInfo();
  if (info==NULL) return;
  EntityInfo *newInfo = (EntityInfo *) gcRealloc(info,sizeof(EntityInfo));
  t->setInfo(newInfo);
  newInfo->gcWatchers();
}

void EntityInfo::gcWatchers(){
  Watcher **base=&watchers;
  Watcher *w=*base;
  if(object) object=((Object *)object)->gcObject();
  while(w!=NULL){
    Thread *th = w->thread;
    if(w->isHandler() && w->thread != DefaultThread)
      th=w->thread->gcThread();
    if(w->isHandler() && th==NULL){
      *base= w->next;
      w=*base;
      continue;}
    Watcher* newW=(Watcher*) gcRealloc(w,sizeof(Watcher));
    *base=newW;
    newW->thread=th;
    OZ_collectHeapTerm(newW->proc,newW->proc);
    base= &(newW->next);
    w=*base;}}

/**********************************************************************/
/*   misc                           */
/**********************************************************************/

/*

Bool Tertiary::startHandlerPort(Thread* th, Tertiary* t, TaggedRef msg,
				EntityCond ec){
  PD((ERROR_DET,"entityProblem invoked Port"));
  Watcher** base=getWatcherBase(this);
  if(base == NULL) return FALSE;
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
*/

Bool Chain::siteOfInterest(DSite* s){
  if(inform!=NULL) return OK;
  ChainElem *tmp=first;
  while(tmp!=NULL){
    if(tmp->site==s) return OK;
    tmp=tmp->next;}
  return NO;}








