/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Copyright:
 *    Michael Mehl (1997,1998)
 *    Per Brand, 1998
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "var.hh"
#endif

#include "var.hh"
#include "var_ext.hh"
#include "var_obj.hh"
#include "msgContainer.hh"
#include "dpMarshaler.hh"
#include "unify.hh"
#include "var_simple.hh"
#include "var_readonly.hh"
#include "chain.hh"


Bool globalRedirectFlag=AUT_REG;

/* --- Common unification --- */

#define UNIFY_ERRORMSG \
   "Unification of distributed variable with term containing resources"


//
//  kost@ GET_ADDR happily compared crocodiles with parrots:
//  external indexes of proxies with internal indexes for managers.
//  I guess a comparison of the same two variables should give 
//  the same results on different hosts, shouldn't it? ;-(
// 
//  The OZ_EVAR_MANAGER OTI assignment was like this:
// OTI=var->getIndex();

// compare NAs
#define GET_ADDR(var,SD,OTI)						\
DSite* SD;Ext_OB_TIndex OTI;						\
if (var->getIdV()==OZ_EVAR_PROXY) {					\
  NetAddress *na=borrowIndex2borrowEntry(var->getIndex())->getNetAddress();	\
  SD=na->site;								\
  OTI = na->index;							\
} else {								\
  SD=myDSite;								\
  OTI = ownerEntry2extOTI(var->getIndex());				\
}

// mm2: simplify: first check OTI only if same compare NA
static
int compareNetAddress(ProxyManagerVar *lVar,ProxyManagerVar *rVar)
{
  GET_ADDR(lVar,lSD,lOTI);
  GET_ADDR(rVar,rSD,rOTI);
  int ret = lSD->compare(rSD);
  if (ret != 0) return ret;
  return lOTI<rOTI ? -1 : 1;
}

inline
OZ_Return ProxyManagerVar::unifyV(TaggedRef *lPtr, TaggedRef *rPtr)
{
  TaggedRef rVal = *rPtr;

  if (!oz_isExtVar(rVal)) {
    // switch order
    if (oz_isFree(rVal))  {
      return oz_var_bind(tagged2Var(rVal),rPtr,makeTaggedRef(lPtr));
    } else {
      return bindV(lPtr,makeTaggedRef(rPtr));
    }
  }

  ExtVar *rVar = oz_getExtVar(rVal);
  int rTag=rVar->getIdV();
  if (rTag!=OZ_EVAR_PROXY && rTag!=OZ_EVAR_MANAGER) {
    return bindV(lPtr,makeTaggedRef(rPtr));
  }

  // Note: for distr. variables: isLocal == onToplevel
  if (oz_isLocalVar(this)) {
    int ret = compareNetAddress(this, (ProxyManagerVar*)rVar);
    Assert(ret!=0);
    if (ret>0) {
      return rVar->bindV(rPtr,makeTaggedRef(lPtr));
    }
  }

  return bindV(lPtr,makeTaggedRef(rPtr));
}

/* --- ProxyVar --- */

OZ_Return ProxyVar::addSuspV(TaggedRef *, Suspendable * susp)
{
  if(!errorIgnore()){
    if(failurePreemption(AtomWait)) return BI_REPLACEBICALL;}
  BorrowEntry *be=borrowIndex2borrowEntry(getIndex());
  extVar2Var(this)->addSuspSVar(susp);
  return SUSPEND;
}

void ProxyVar::gCollectRecurseV(void)
{ 
  PD((GC,"ProxyVar b:%d",getIndex()));
  Assert(getIndex() != MakeOB_TIndex((void*) BAD_BORROW_INDEX));
  borrowIndex2borrowEntry(getIndex())->gcPO();
  if (binding)
    oz_gCollectTerm(binding,binding);
  if (status)
    oz_gCollectTerm(status,status);
  setInfo(gcEntityInfoInternal(getInfo()));
} 

static
void sendSurrender(BorrowEntry *be,OZ_Term val){
  NetAddress *na = be->getNetAddress();  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site, am.currentThread()->getPriority());
  msgC->put_M_SURRENDER(na->index,val);
  send(msgC);
}

Bool dealWithInjectors(TaggedRef t,EntityInfo *info,EntityCond ec,Thread* th,Bool &hit,TaggedRef term){
  Watcher** base= info->getWatcherBase();
  while(TRUE){
    if((*base)==NULL) return FALSE;
    if((*base)->isSiteBased()) break;
    if(th==(*base)->thread) break;
    base= &((*base)->next);}
  if(!((((*base)->watchcond)) & ec)) return FALSE;
  (*base)->varInvokeInjector(t,(*base)->watchcond & ec,term);
  hit=TRUE;
  if((*base)->isPersistent()) return FALSE;
  *base=(*base)->next;
  return TRUE;
}

Bool varFailurePreemption(TaggedRef t,EntityInfo* info,Bool &hit,TaggedRef term){
  EntityCond ec=info->getEntityCond();
  if(ec==ENTITY_NORMAL) return FALSE;
  Bool ret=dealWithInjectors(t,info,ec,oz_currentThread(),hit,term);
  if(hit) return ret;
  if(globalWatcher==NULL) return FALSE;
  ec=globalWatcher->watchcond & ec;
  if(!ec) return FALSE;
  globalWatcher->varInvokeInjector(t,ec,term);
  hit=TRUE;
  return FALSE;
}

Bool ProxyVar::failurePreemption(TaggedRef term){
  Assert(info!=NULL);
  info->dealWithWatchers(getTaggedRef(),info->getEntityCond());
  Bool hit=FALSE;
  EntityCond oldC=info->getSummaryWatchCond();  
  if(varFailurePreemption(getTaggedRef(),info,hit,term)){
    EntityCond newC=info->getSummaryWatchCond();
    varAdjustPOForFailure(getIndex(),oldC,newC);}
  return hit;
}

Bool ManagerVar::failurePreemption(TaggedRef term){
  Assert(info!=NULL);
  info->dealWithWatchers(getTaggedRef(),info->getEntityCond());
  Bool hit=FALSE;
  EntityCond oldC=info->getSummaryWatchCond();  
  if(varFailurePreemption(getTaggedRef(),info,hit,term)){}
  return hit;
}
 
OZ_Return ProxyVar::bindV(TaggedRef *lPtr, TaggedRef r){
  PD((PD_VAR,"ProxyVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind proxy b:%d v:%s",getIndex(),toC(r)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    if(!errorIgnore()){
      if(isFuture()){
	if(failurePreemption(AtomWait)) return BI_REPLACEBICALL;}	
      else{
	if(failurePreemption(mkOp1("bind",r))) return BI_REPLACEBICALL;}}
    if (!binding) {
      if(isFuture()){
	return oz_addSuspendVarList(lPtr);
      }
      BorrowEntry *be=borrowIndex2borrowEntry(getIndex());
      sendSurrender(be,r);
      PD((THREAD_D,"stop thread proxy bind %x",oz_currentThread()));
      binding=r;
    }
    return oz_addSuspendVarList(lPtr);
  } else {
    // in guard: bind and trail
    if(!errorIgnore()){
      if(failurePreemption(mkOp1("bind",r))) {
	return BI_REPLACEBICALL;}}
    oz_bindGlobalVar(extVar2Var(this),lPtr,r);
    return PROCEED;
  }    
}

void ProxyVar::redoStatus(TaggedRef val, TaggedRef status)
{
  OZ_unifyInThread(status, oz_status(val));
}

void ProxyVar::redirect(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be)
{
  OB_TIndex BTI = getIndex();
  if(status!=0){
    redoStatus(val,status);}
  PD((TABLE,"REDIRECT - borrow entry hit b:%d",BTI));
  if (binding) {
    DebugCode(binding=0);
    PD((PD_VAR,"REDIRECT while pending"));
  }
  EntityInfo* ei=info;
  oz_bindLocalVar(extVar2Var(this),vPtr,val);
  be->changeToRef();
  maybeHandOver(ei,val);
  (void) BT->maybeFreeBorrowEntry(BTI);
}

void ProxyVar::acknowledge(TaggedRef *vPtr, BorrowEntry *be) 
{
  OB_TIndex BTI = getIndex();
  if(status!=0){
    redoStatus(binding,status);}
  PD((PD_VAR,"acknowledge"));

  EntityInfo* ei=info;
  oz_bindLocalVar(extVar2Var(this),vPtr,binding);

  be->changeToRef();
  maybeHandOver(ei,binding);
  (void) BT->maybeFreeBorrowEntry(BTI);
}

/* --- ManagerVar --- */

OZ_Return ManagerVar::addSuspV(TaggedRef *vPtr, Suspendable * susp)
{
  if(!errorIgnore()){
    if(failurePreemption(AtomWait)) return BI_REPLACEBICALL;}
  
  /*
  if (origVar->getType()==OZ_VAR_FUTURE) {
    if (((Future *)origVar)->kick(vPtr))
      return PROCEED;
  }
  */
  extVar2Var(this)->addSuspSVar(susp);
  return SUSPEND;
}

void ManagerVar::gCollectRecurseV(void)
{
  oz_gCollectTerm(origVar,origVar);
  ownerIndex2ownerEntry(getIndex())->gcPO();
  PD((GC,"ManagerVar o:%d",getIndex()));
  ProxyList **last=&proxies;
  for (ProxyList *pl = proxies; pl; pl = pl->next) {
    pl->sd->makeGCMarkSite();
    ProxyList *newPL = new ProxyList(pl->sd,0);
    *last = newPL;
    last = &newPL->next;}
  *last = 0;
  setInfo(gcEntityInfoInternal(getInfo()));
}

static void sendAcknowledge(DSite* sd, OB_TIndex OTI) {
  PD((PD_VAR,"sendAck %s",sd->stringrep()));  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(sd);
  msgC->put_M_ACKNOWLEDGE(ownerEntry2extOTI(OTI));

  send(msgC);
}

// extern
void sendRedirect(DSite* sd, OB_TIndex OTI, TaggedRef val)
{
  PD((PD_VAR,"sendRedirect %s",sd->stringrep()));  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(sd);
  msgC->put_M_REDIRECT(myDSite, ownerEntry2extOTI(OTI), val);

  send(msgC);
}

inline Bool queueTrigger(DSite* s){
  int msgs;
  if(s->getQueueStatus()>0) return TRUE;
  return FALSE;}

// ERIK-LOOK use antoher ozconf.

static inline Bool canSend(DSite* s){
  int msgs;
  if(s->getQueueStatus()>ozconf.dpFlowBufferSize) return FALSE;
  return TRUE;}

Bool varCanSend(DSite* s){
  return canSend(s);}

void ManagerVar::sendRedirectToProxies(OZ_Term val, DSite* ackSite)
{
  PD((PD_VAR,"sendRedirectToProxies"));  
  ProxyList *pl = proxies;
  while (pl) {
    DSite* sd = pl->sd;
    Assert(sd!=myDSite);
    if (sd==ackSite) {
      sendAcknowledge(sd,getIndex());}
    else {
      if(!(USE_ALT_VAR_PROTOCOL) && (pl->kind==EXP_REG || queueTrigger(sd))){
	//NOTE globalRedirect is only important if we use the alt var 
	//NOTE protocol.
	globalRedirectFlag=EXP_REG;
	sendRedirect(sd,getIndex(),val);
	globalRedirectFlag=AUT_REG;}
      else{
	Assert(pl->kind==AUT_REG);
	sendRedirect(sd,getIndex(),val);}}
    pl=pl->dispose();
  }
  proxies = 0;
}


OZ_Return ManagerVar::bindVInternal(TaggedRef *lPtr, TaggedRef r,DSite *s)
{
  OB_TIndex OTI = getIndex();
  PD((PD_VAR,"ManagerVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind manager o:%d v:%s",OTI,toC(*lPtr)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    if(isFuture()){
      return oz_addSuspendVarList(lPtr);
    }
    EntityInfo *ei=info;
    sendRedirectToProxies(r, s);
    oz_bindLocalVar(extVar2Var(this),lPtr,r);
    ownerIndex2ownerEntry(OTI)->changeToRef();
    maybeHandOver(ei,r);
    return PROCEED;
  } else {
    oz_bindGlobalVar(extVar2Var(this),lPtr,r);
    return PROCEED;
  }
} 

OZ_Return ManagerVar::bindV(TaggedRef *lPtr, TaggedRef r){
  if(!errorIgnore()){
    if(failurePreemption(mkOp1("bind",r))) return BI_REPLACEBICALL;}
  // AN: Pass NULL and not myDSite. myDSite can never be a site
  // that acknowledge should be sent to. It will also never be in the
  // proxy list.
  return bindVInternal(lPtr,r,NULL);
}


void varGetStatus(DSite* site, Ext_OB_TIndex OTI, TaggedRef tr)
{
  MsgContainer *msgC = msgContainerManager->newMsgContainer(site);
  msgC->put_M_SENDSTATUS(myDSite,OTI,tr);
  send(msgC);
}

void ProxyVar::receiveStatus(TaggedRef tr)
{
  Assert(status!=0);
  OZ_unifyInThread(status, tr);
  status=0;
}


OZ_Return ManagerVar::forceBindV(TaggedRef *lPtr, TaggedRef r)
{
  OB_TIndex OTI = getIndex();
  PD((PD_VAR,"ManagerVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind manager o:%d v:%s",OTI,toC(*lPtr)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    // In this case noone could get acknowledge => NULL
    sendRedirectToProxies(r, NULL);
    EntityInfo *ei=info;
    oz_bindLocalVar(extVar2Var(this),lPtr,r);
    ownerIndex2ownerEntry(OTI)->changeToRef();
    maybeHandOver(ei,r);
    return PROCEED;
  } else {
    oz_bindGlobalVar(extVar2Var(this),lPtr,r);
    return PROCEED;
  }
} 

void ManagerVar::surrender(TaggedRef *vPtr, TaggedRef val,DSite *ackSite)
{
  // AckSite has to be passed on if acknowledge and not redirect should
  // be sent to the 'binding' site.

  // AN! These two lines were copied from bindV. What do they do? Are they
  // needed here?
//    if(!errorIgnore()){
//      if(failurePreemption(mkOp1("bind",val))) return BI_REPLACEBICALL;}

  OZ_Return ret =  bindVInternal(vPtr,val,ackSite);
  Assert(ret==PROCEED);
}

void requested(TaggedRef*);

//
ManagerVar* globalizeFreeVariable(TaggedRef *tPtr)
{
  OwnerEntry *oe;
  OB_TIndex i = OT->newOwner(oe);
  PD((GLOBALIZING,"globalize var index:%d",i));
  oe->mkVar(makeTaggedRef(tPtr));
  // raph: make the variable needed
  oz_var_makeNeeded(tPtr);
  OzVariable *cv = oz_getNonOptVar(tPtr);
  ManagerVar *mv = new ManagerVar(cv,i);
  extVar2Var(mv)->setSuspList(cv->unlinkSuspList());
  *tPtr = makeTaggedVar(extVar2Var(mv));
  return (mv);
}

/* --- Unmarshal --- */

static void sendRegister(BorrowEntry *be) {
  PD((PD_VAR,"sendRegister"));
  NetAddress *na = be->getNetAddress();  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->put_M_REGISTER(na->index);
  send(msgC);
}

static void sendDeRegister(BorrowEntry *be) {
  PD((PD_VAR,"sendDeRegister"));  

  NetAddress *na = be->getNetAddress();  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->put_M_DEREGISTER(na->index);
  send(msgC);
}

void ProxyVar::nowGarbage(BorrowEntry* be){
  PD((PD_VAR,"nowGarbage"));  
  sendDeRegister(be);}

//
// Marshaling code is in dpMarshaler.cc;

//
OZ_Term unmarshalVar(MarshalerBuffer* bs, Bool isFuture, Bool isAuto)
{
  OB_Entry *ob;
  OB_TIndex bi;
  BYTE ec;
  OZ_Term val1 = unmarshalBorrow(bs, ob, bi, ec);
  
  if (val1) {
    PD((UNMARSHAL,"var/chunk hit: b:%d",bi));
    // If the entity had a failed condition, propagate it.
    if (ec & PERM_FAIL)deferProxyVarProbeFault(val1,PROBE_PERM);
    return val1;}
  
  PD((UNMARSHAL,"var miss: b:%d",bi));
  ProxyVar *pvar = new ProxyVar(oz_currentBoard(),bi,isFuture);
  
  TaggedRef val = makeTaggedRef(newTaggedVar(extVar2Var(pvar)));
  ob->changeToVar(val); // PLEASE DONT CHANGE THIS 
  if(!isAuto) {
    sendRegister((BorrowEntry *)ob);}
  else{
    pvar->makeAuto();}
   // If the entity carries failure info react to it
  // othervise check the local environment. The present 
  // representation of the entities site might have information
  // about the failure state.
   
  if (ec & PERM_FAIL)
    {  
      deferProxyVarProbeFault(val,PROBE_PERM);
    }
  else
    {
      switch(((BorrowEntry*)ob)->getSite()->siteStatus()){
      case SITE_OK:{
	break;}
      case SITE_PERM:{
	deferProxyVarProbeFault(val,PROBE_PERM);
	break;}
      case SITE_TEMP:{
	deferProxyVarProbeFault(val,PROBE_TEMP);
	break;}
      default:
	Assert(0);
      } 
    }
  return val;
}


/* --- IsVar test --- */

static
void sendGetStatus(BorrowEntry *be){
  NetAddress *na = be->getNetAddress();  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->put_M_GETSTATUS(na->index);
  send(msgC);
}

OZ_Term ProxyVar::statusV()
{
  if (status==0) {
    BorrowEntry *be = borrowIndex2borrowEntry(getIndex());
    sendGetStatus(be);
    status= oz_newVariable();
  }
  return status;
}

VarStatus ProxyVar::checkStatusV()
{
  return EVAR_STATUS_UNKNOWN;
}

/* --- IsVar test --- */

inline
void ManagerVar::localize(TaggedRef *vPtr)
{
  Assert(getInfo()==NULL);
  getOrigVar()->setSuspList(extVar2Var(this)->unlinkSuspList());
  *vPtr=origVar;
  origVar=makeTaggedNULL();
  disposeV();
}

OZ_Term ManagerVar::statusV() {
  return oz_isFuture(origVar) ? AtomFuture : AtomFree;
}

VarStatus ManagerVar::checkStatusV(){
  return oz_isFuture(origVar) ? EVAR_STATUS_FUTURE : EVAR_STATUS_FREE;
}

void oz_dpvar_localize(TaggedRef *vPtr) {
  Assert(classifyVar(vPtr)==VAR_MANAGER);
  oz_getManagerVar(*vPtr)->localize(vPtr);
}

//
// FAILURE structure fundamentals 
VarKind classifyVar(TaggedRef* tPtr)
{ 
  TaggedRef tr = *tPtr;
  if (oz_isExtVar(tr)) {
    ExtVarType evt = oz_getExtVar(tr)->getIdV();
    switch (evt) {
    case OZ_EVAR_MANAGER:
      return (VAR_MANAGER);
    case OZ_EVAR_PROXY:
      return (VAR_PROXY);
    case OZ_EVAR_LAZY:
      return (VAR_LAZY);
    default:
      Assert(0);
      return (VAR_PROXY);
    }
  } else if (oz_isFree(tr)) {
    return (VAR_FREE);
  } else if (oz_isFuture(tr)) {
    return (VAR_FUTURE);
  } else {
    return (VAR_KINDED);
  }
  Assert(0);
}

//
EntityInfo* varGetEntityInfo(TaggedRef* tPtr)
{
  switch (classifyVar(tPtr)) {
  case VAR_MANAGER:
    return (oz_getManagerVar(*tPtr)->getInfo());
  case VAR_PROXY:
    return (oz_getProxyVar(*tPtr)->getInfo());
  case VAR_LAZY:
    return (oz_getLazyVar(*tPtr)->getInfo());
  default:
    break;
  }
  Assert(0); return ((EntityInfo *) 0);
}

EntityInfo* varMakeEntityInfo(TaggedRef* tPtr){
  EntityInfo* ei= new EntityInfo();
  switch(classifyVar(tPtr)){
  case VAR_MANAGER:
    oz_getManagerVar(*tPtr)->setInfo(ei);
    return ei;
  case VAR_PROXY:
    oz_getProxyVar(*tPtr)->setInfo(ei);
    return ei;
  case VAR_LAZY:
    oz_getLazyVar(*tPtr)->setInfo(ei);
    return ei;
  default:
    Assert(0);}
  return NULL;
}

EntityInfo* varMakeOrGetEntityInfo(TaggedRef* tPtr){
  EntityInfo* ei=varGetEntityInfo(tPtr);
  if(ei!=NULL) return ei;
  return varMakeEntityInfo(tPtr);
}

// FAILURE stuff

void ProxyVar::addEntityCond(EntityCond ec){
  if(info==NULL) info= new EntityInfo();
  if(!info->addEntityCond(ec)) return;
  wakeAll();
  info->dealWithWatchers(getTaggedRef(),ec);
}

void ProxyVar::newWatcher(Bool b){
  if(b){
    wakeAll();
    return;}
  info->dealWithWatchers(getTaggedRef(),info->getEntityCond());
}

void ProxyVar::subEntityCond(EntityCond ec){
  Assert(info!=NULL);
  info->subEntityCond(ec);
}
  
void ManagerVar::newWatcher(Bool b){
  if(b) {
    wakeAll();
    return;}
  info->dealWithWatchers(getTaggedRef(),info->getEntityCond());
}
  
void ManagerVar::addEntityCond(EntityCond ec){
  if(info==NULL) info= new EntityInfo();
  if(!info->addEntityCond(ec)) return;
  OB_TIndex i = getIndex();
  OwnerEntry* oe = ownerIndex2ownerEntry(i);
  triggerInforms(&inform,oe,ec);
  wakeAll();
  info->dealWithWatchers(getTaggedRef(),ec);
}

void ManagerVar::subEntityCond(EntityCond ec){
  // Hack! must be further investigated
  // It sems as ManagerVars can receive PROBE_OK
  // without being in the state TEMP_SOME
  // Erik
  if (info != NULL) {
    info->subEntityCond(ec);
    OB_TIndex i = getIndex();
    OwnerEntry* oe = ownerIndex2ownerEntry(i);
    triggerInforms(&inform,oe,ec);
  }
}

Bool ManagerVar::siteInProxyList(DSite* s){
  ProxyList* pl=proxies;
  while(pl!=NULL){
    if(pl->sd==s) return TRUE;
    pl=pl->next;}
  return FALSE;
}

void ManagerVar::probeFault(DSite *s,int pr){
  if(!siteInProxyList(s)) return;
  if(pr==PROBE_PERM){
    deregisterSite(s);
    addEntityCond(PERM_SOME);
    return;}
  if(pr==PROBE_TEMP){
    addEntityCond(TEMP_SOME);
    return;}
  Assert(pr==PROBE_OK);
  subEntityCond(TEMP_SOME);
}

void ProxyVar::probeFault(int pr){
  if (info && (info->getEntityCond() & (PERM_FAIL))) return;
  if(pr==PROBE_PERM){
    addEntityCond(PERM_FAIL|PERM_SOME);    
    return;}
  if(pr==PROBE_TEMP){
    addEntityCond(TEMP_FAIL|TEMP_SOME);    
    return;}
  Assert(pr==PROBE_OK);
  subEntityCond(TEMP_FAIL|TEMP_SOME);
}
  
EntityCond varGetEntityCond(TaggedRef* tr){
  return varGetEntityInfo(tr)->getEntityCond();
}

VarKind typeOfBorrowVar(BorrowEntry* b){
  Assert(b->isVar());
  ExtVar *ev=oz_getExtVar((oz_deref(b->getRef())));
  switch(ev->getIdV()){ 
  case OZ_EVAR_PROXY:
    return VAR_PROXY;
  case OZ_EVAR_LAZY:
    return VAR_LAZY;
  default:
    Assert(0);}
  return VAR_PROXY; // stupid compiler
}

Bool errorIgnoreVar(BorrowEntry* b){
  EntityInfo* ie;
  switch(typeOfBorrowVar(b)){
  case VAR_PROXY:
    ie = GET_VAR(b,Proxy)->getInfo();
    if(ie==NULL) return TRUE;
    return FALSE;
  case VAR_LAZY:
    ie= GET_VAR(b, Lazy)->getInfo();
    if(ie==NULL) return TRUE;
    return FALSE;
  default:
    Assert(0);}
  return FALSE; // stupid compiler
}

void maybeUnaskVar(BorrowEntry* b){
  EntityInfo* ie;
  switch(typeOfBorrowVar(b)){
  case VAR_PROXY:
    ie= GET_VAR(b,Proxy)->getInfo();
    if(ie==NULL) return;
    varAdjustPOForFailure(GET_VAR(b,Proxy)->getIndex(),
			  ie->getEntityCond(),ENTITY_NORMAL);
    return;
  case VAR_LAZY:{
    ie = GET_VAR(b, Lazy)->getInfo();
    if(ie==NULL) return;
    OB_TIndex i = GET_VAR(b, Lazy)->getIndex(); 
    varAdjustPOForFailure(i,ie->getEntityCond(),ENTITY_NORMAL);
    return;}
  default:
    Assert(0);}
}

void ManagerVar::newInform(DSite* s,EntityCond ec){
  InformElem* ie=new InformElem(s,ec);
  ie->next=inform;
  inform=ie;
}

void ProxyVar::wakeAll(){
  OzVariable*p=extVar2Var(this);
  oz_checkSuspensionList(p,pc_all);
}

void ManagerVar::wakeAll(){
  OzVariable*p=extVar2Var(this);
  oz_checkSuspensionList(p,pc_all);
}

//
//
inline 
void VariableExcavator::processSmallInt(OZ_Term siTerm) {}
inline 
void VariableExcavator::processFloat(OZ_Term floatTerm) {}
inline 
void VariableExcavator::processLiteral(OZ_Term litTerm) {}
inline 
void VariableExcavator::processExtension(OZ_Term t) {}
inline 
void VariableExcavator::processBigInt(OZ_Term biTerm) {}
inline 
void VariableExcavator::processBuiltin(OZ_Term biTerm, ConstTerm *biConst) {}

inline 
Bool VariableExcavator::processObject(OZ_Term objTerm, ConstTerm *objConst)
{
  VisitNodeTrav(objTerm, vIT, return(TRUE));
  return (TRUE);
}

inline 
void VariableExcavator::processNoGood(OZ_Term resTerm)
{
  Assert(!oz_isVar(resTerm));
}

inline 
void VariableExcavator::processLock(OZ_Term lockTerm, Tertiary *tert) {}

inline 
Bool VariableExcavator::processCell(OZ_Term cellTerm, Tertiary *tert)
{
  VisitNodeTrav(cellTerm, vIT, return(TRUE));
  return (TRUE);
}

inline 
void VariableExcavator::processPort(OZ_Term portTerm, Tertiary *tert) {}
inline 
void VariableExcavator::processResource(OZ_Term rTerm, Tertiary *tert) {}

//
inline 
void VariableExcavator::processVar(OZ_Term v, OZ_Term *vRef)
{
  Assert(oz_isVar(v));
  OZ_Term vrt = makeTaggedRef(vRef);
  VisitNodeTrav(vrt, vIT, return);
  addVar(vrt);
}

//
inline 
Bool VariableExcavator::processLTuple(OZ_Term ltupleTerm)
{
  VisitNodeTrav(ltupleTerm, vIT, return(TRUE));
  return (NO);
}
inline 
Bool VariableExcavator::processSRecord(OZ_Term srecordTerm)
{
  VisitNodeTrav(srecordTerm, vIT, return(TRUE));
  return (NO);
}
inline Bool
VariableExcavator::processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst)
{
  VisitNodeTrav(chunkTerm, vIT, return(TRUE));
  return (NO);
}

//
inline Bool VariableExcavator::processFSETValue(OZ_Term fsetvalueTerm)
{
  return (NO);
}

//
inline Bool
VariableExcavator::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  VisitNodeTrav(dictTerm, vIT, return(TRUE));
  //
  OzDictionary *d = (OzDictionary *) dictConst;
  return (d->isSafeDict() ? NO : OK);
}

//
inline Bool
VariableExcavator::processArray(OZ_Term arrayTerm, ConstTerm *arrayConst)
{
  VisitNodeTrav(arrayTerm, vIT, return(TRUE));
  return (OK);
}

//
inline Bool
VariableExcavator::processClass(OZ_Term classTerm, ConstTerm *classConst)
{ 
  VisitNodeTrav(classTerm, vIT, return(TRUE));
  //
  ObjectClass *cl = (ObjectClass *) classConst;
  return (cl->isSited());
}

//
inline Bool
VariableExcavator::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  VisitNodeTrav(absTerm, vIT, return(TRUE));

  //
  Abstraction *pp = (Abstraction *) absConst;
  PrTabEntry *pred = pp->getPred();
  if (pred->isSited()) {
    return (OK);		// done - a leaf;
  } else {
    ProgramCounter start = pp->getPC() - sizeOf(DEFINITION);
    XReg reg;
    int nxt, line, colum;
    TaggedRef file, predName;
    CodeArea::getDefinitionArgs(start, reg, nxt, file,
				line, colum, predName);

    //
    DPMarshalerCodeAreaDescriptor *desc = 
      new DPMarshalerCodeAreaDescriptor(start, start + nxt,
					(AddressHashTableO1Reset *) 0);
    traverseBinary(traverseCode, desc);
    return (NO);
  }
}

//
inline 
void VariableExcavator::processSync() {}

//
#define	TRAVERSERCLASS	VariableExcavator
#include "gentraverserLoop.cc"
#undef	TRAVERSERCLASS

static VariableExcavator ve;

//
static inline
OZ_Term extractVars(OZ_Term in)
{
  ve.init();
  ve.prepareTraversing((Opaque *) 0);
  ve.traverse(in);
  ve.finishTraversing();
  return (ve.getVars());
}

//
// kost@ : "deautosite"ing relies on the fact that when a variable
// manager is bound, its value - including all variables! - is
// immediately exported, thus, local variables become variable
// managers and can be marked 'EXP_REG', which, in turn, disables
// auto-exportation for (variables that occur in) their future values.
// Should that exportation happen some time later, it can also happen
// after the arrival of the 'deregister' message, so the effect of the
// message will be lost.
void recDeregister(TaggedRef tr, DSite* s)
{
  OZ_Term vars = extractVars(tr);
  while (!oz_isNil(vars)) {
    OZ_Term t = oz_head(vars);
    OZ_Term *tp = tagged2Ref(t);
    Assert(oz_isVar(*tp));
    if (classifyVar(tp) == VAR_MANAGER)
      oz_getManagerVar(*tp)->deAutoSite(s);
    vars = oz_tail(vars);
  }
}

static ProxyList** findBefore(DSite* s,ProxyList** base ){
  while((*base)!=NULL){
    if((*base)->sd==s) return base;
    base= &((*base)->next);}
  return NULL;}

void ManagerVar::deAutoSite(DSite* s){
  ProxyList **aux= findBefore(s, &proxies);
  if (aux!=NULL && *aux!=NULL){
    ProxyList *pl=*aux;
    pl->kind=EXP_REG;}
}

void ManagerVar::deregisterSite(DSite* s){
  ProxyList **pl=findBefore(s, &proxies);
  Assert(pl!=NULL);
  *pl= (*pl)->next;
}


