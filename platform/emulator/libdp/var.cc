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
#include "var_emanager.hh"
#include "var_eproxy.hh"
#include "msgContainer.hh"
#include "dpMarshaler.hh"
#include "unify.hh"
#include "var_simple.hh"
#include "var_future.hh"
#include "chain.hh"
#include "flowControl.hh"


Bool globalRedirectFlag=AUT_REG;

/* --- Common unification --- */

#define UNIFY_ERRORMSG \
   "Unification of distributed variable with term containing resources"


// compare NAs
#define GET_ADDR(var,SD,OTI)						\
DSite* SD;int OTI;							\
if (var->getIdV()==OZ_EVAR_PROXY) {					\
  NetAddress *na=BT->getBorrow(var->getIndex())->getNetAddress();	\
  SD=na->site;								\
  OTI=na->index;							\
} else {								\
  SD=myDSite;								\
  OTI=var->getIndex();							\
}

// mm2: simplify: first check OTI only if same compare NA
static
int compareNetAddress(ProxyManagerVar *lVar,ProxyManagerVar *rVar)
{
  GET_ADDR(lVar,lSD,lOTI);
  GET_ADDR(rVar,rSD,rOTI);
  int ret = lSD->compareSites(rSD);
  if (ret != 0) return ret;
  return lOTI<rOTI ? -1 : 1;
}

TaggedRef ProxyVar::getTaggedRef(){
  return borrowTable->getBorrow(getIndex())->getRef();}

TaggedRef ManagerVar::getTaggedRef(){
  return OT->getEntry(getIndex())->getRef();}

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

//
// kost@ : certain versions of gcc (e.g. 2.7.2.3/linux) have problems
// with an 'inline' version of this function: its usage in
// 'BorrowEntry::copyBorrow(BorrowEntry* from,int i)' cannot be
// resolved as in inline one, while a compiled one does not exist
// either...
void ProxyManagerVar::gcSetIndex(int i)
{
  index =  i;
}


/* --- ProxyVar --- */

OZ_Return ProxyVar::addSuspV(TaggedRef *, Suspendable * susp)
{
  if(!errorIgnore()){
    if(failurePreemption(AtomWait)) return BI_REPLACEBICALL;}
  BorrowEntry *be=BT->getBorrow(getIndex());
  addSuspSVar(susp);
  return SUSPEND;
}

void ProxyVar::gCollectRecurseV(void)
{ 
  PD((GC,"ProxyVar b:%d",getIndex()));
  Assert(getIndex()!=BAD_BORROW_INDEX);
  BT->getBorrow(getIndex())->gcPO();
  if (binding)
    oz_gCollectTerm(binding,binding);
  if (status)
    oz_gCollectTerm(status,status);
  setInfo(gcEntityInfoInternal(getInfo()));
} 

static
void sendSurrender(BorrowEntry *be,OZ_Term val){
  NetAddress *na = be->getNetAddress();  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->put_M_SURRENDER(na->index,myDSite,val);
  send(msgC,-1);
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
      BorrowEntry *be=BT->getBorrow(getIndex());
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
    oz_bindGlobalVar(this,lPtr,r);
    return PROCEED;
  }    
}

void ProxyVar::redoStatus(TaggedRef val, TaggedRef status)
{
  OZ_unifyInThread(status, oz_status(val));
}

void ProxyVar::redirect(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be)
{
  int BTI=getIndex();
  if(status!=0){
    redoStatus(val,status);}
  PD((TABLE,"REDIRECT - borrow entry hit b:%d",BTI));
  if (binding) {
    DebugCode(binding=0);
    PD((PD_VAR,"REDIRECT while pending"));
  }
  EntityInfo* ei=info;
  oz_bindLocalVar(this,vPtr,val);
  be->changeToRef();
  maybeHandOver(ei,val);
  (void) BT->maybeFreeBorrowEntry(BTI);
}

void ProxyVar::acknowledge(TaggedRef *vPtr, BorrowEntry *be) 
{
  int BTI=getIndex();
  if(status!=0){
    redoStatus(binding,status);}
  PD((PD_VAR,"acknowledge"));

  EntityInfo* ei=info;
  oz_bindLocalVar(this,vPtr,binding);

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
  addSuspSVar(susp);
  return SUSPEND;
}

void ManagerVar::gCollectRecurseV(void)
{
  oz_gCollectTerm(origVar,origVar);
  OT->getEntry(getIndex())->gcPO();
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

static void sendAcknowledge(DSite* sd,int OTI){
  PD((PD_VAR,"sendAck %s",sd->stringrep()));  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(sd);
  msgC->put_M_ACKNOWLEDGE(myDSite,OTI);

  send(msgC,-1);
}

// extern
void sendRedirect(DSite* sd,int OTI,TaggedRef val)
{
  PD((PD_VAR,"sendRedirect %s",sd->stringrep()));  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(sd);
  msgC->put_M_REDIRECT(myDSite,OTI,val);

  send(msgC,-1);
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
      if(!canSend(sd)){
	flowControler->addElement(val,sd,getIndex());}
      else{
	if(!(USE_ALT_VAR_PROTOCOL) && (pl->kind==EXP_REG || queueTrigger(sd))){
	  //NOTE globalRedirect is only important if we use the alt var 
	  //NOTE protocol.
	  globalRedirectFlag=EXP_REG;
	  sendRedirect(sd,getIndex(),val);
	  globalRedirectFlag=AUT_REG;}
	else{
	  Assert(pl->kind==AUT_REG);
	  sendRedirect(sd,getIndex(),val);}}}
    pl=pl->dispose();
  }
  proxies = 0;
}


OZ_Return ManagerVar::bindVInternal(TaggedRef *lPtr, TaggedRef r,DSite *s)
{
  int OTI=getIndex();
  PD((PD_VAR,"ManagerVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind manager o:%d v:%s",OTI,toC(*lPtr)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    if(isFuture()){
      return oz_addSuspendVarList(lPtr);
    }
    EntityInfo *ei=info;
    sendRedirectToProxies(r, s);
    oz_bindLocalVar(this,lPtr,r);
    OT->getEntry(OTI)->changeToRef();
    maybeHandOver(ei,r);
    return PROCEED;
  } else {
    oz_bindGlobalVar(this,lPtr,r);
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

void varGetStatus(DSite* site,int OTI, TaggedRef tr){
  MsgContainer *msgC = msgContainerManager->newMsgContainer(site);
  msgC->put_M_SENDSTATUS(myDSite,OTI,tr);

  send(msgC,-1);
}

void ProxyVar::receiveStatus(TaggedRef tr)
{
  Assert(status!=0);
  OZ_unifyInThread(status, tr);
  status=0;
}


OZ_Return ManagerVar::forceBindV(TaggedRef *lPtr, TaggedRef r)
{
  int OTI=getIndex();
  PD((PD_VAR,"ManagerVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind manager o:%d v:%s",OTI,toC(*lPtr)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    // In this case noone could get acknowledge => NULL
    sendRedirectToProxies(r, NULL);
    EntityInfo *ei=info;
    oz_bindLocalVar(this,lPtr,r);
    OT->getEntry(OTI)->changeToRef();
    maybeHandOver(ei,r);
    return PROCEED;
  } else {
    oz_bindGlobalVar(this,lPtr,r);
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

/* --- Marshal --- */

void ManagerVar::marshal(ByteBuffer *bs)
{
  int i=getIndex();
  PD((MARSHAL,"var manager o:%d",i));
  if((USE_ALT_VAR_PROTOCOL) && globalRedirectFlag==AUT_REG){
    if(isFuture()){ 
      marshalOwnHead(bs, DIF_FUTURE_AUTO, i);}
    else{
      marshalOwnHead(bs, DIF_VAR_AUTO, i);}  
    registerSite(bs->getSite());
    return;}
  if(isFuture()){ 
    marshalOwnHead(bs, DIF_FUTURE, i);}
  else{
    marshalOwnHead(bs, DIF_VAR, i);}  
}

//
ManagerVar* globalizeFreeVariable(TaggedRef *tPtr){
  OwnerEntry *oe;
  int i = OT->newOwner(oe);
  PD((GLOBALIZING,"globalize var index:%d",i));
  oe->mkVar(makeTaggedRef(tPtr));
  OzVariable *cv = oz_getNonOptVar(tPtr);
  ManagerVar *mv = new ManagerVar(cv,i);
  mv->setSuspList(cv->unlinkSuspList());
  *tPtr = makeTaggedVar(mv);
  return (mv);
}

// Returning 'NO' means we are going to proceed with 'marshal bomb';
Bool marshalVariable(TaggedRef *tPtr, ByteBuffer *bs)
{
  const TaggedRef var = *tPtr;
  if (oz_isExtVar(var)) {
    ExtVarType evt = oz_getExtVar(var)->getIdV();
    switch (evt) {
    case OZ_EVAR_MANAGER:
      oz_getManagerVar(var)->marshal(bs);
      break;
    case OZ_EVAR_PROXY:
      oz_getProxyVar(var)->marshal(bs);
      break;
    case OZ_EVAR_LAZY:
      oz_getLazyVar(var)->marshal(bs);
      break;
    case OZ_EVAR_EMANAGER:
      oz_getEManagerVar(var)->marshal(bs);
      break;
    case OZ_EVAR_EPROXY:
      oz_getEProxyVar(var)->marshal(bs);
      break;
    case OZ_EVAR_GCSTUB:
      Assert(0);
      break;
    default:
      Assert(0);
      break;
    }
    return (TRUE);
  } else if (oz_isFree(var) || oz_isFuture(var)) {
    Assert(perdioInitialized);
    globalizeFreeVariable(tPtr)->marshal(bs);
    return (TRUE);
  } else { 
    return (FALSE);
  }
  Assert(0);
}

// Return 'TRUE' if successful (that is, the variable is bound)
Bool triggerVariable(TaggedRef *tPtr){
  Assert(tPtr!=NULL);
  const TaggedRef var = *tPtr;
  if (oz_isFuture(var)) {
    // kost@ : 'oz_isFuture(var)' does NOT mean that the 'var' is of
    // type Future: it can be also a manager var keeping a future!
    Future *fut;
    if (oz_isManagerVar(var)) {
      ManagerVar *mv = oz_getManagerVar(var);
      fut = (Future *) mv->getOrigVar();
    } else {
      fut = (Future *) tagged2Var(var);
    }
    Assert(fut->getType() == OZ_VAR_FUTURE);

    //
    switch (fut->kick(tPtr)) {
    case PROCEED: return (TRUE);
    case SUSPEND: return (FALSE);
      // kost@ : I dunno how to handle it. Those who have introduced
      // 'RAISE' as a return value of 'Future::kick' should have fixed
      // this part as well.
    case RAISE: return (FALSE);
    }
    return (FALSE);
  } else {
    return (FALSE);
  }
}

/* --- Unmarshal --- */

static void sendRegister(BorrowEntry *be) {
  PD((PD_VAR,"sendRegister"));
  NetAddress *na = be->getNetAddress();  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->put_M_REGISTER(na->index,myDSite);
  send(msgC,-1);
}

static void sendDeRegister(BorrowEntry *be) {
  PD((PD_VAR,"sendDeRegister"));  

  NetAddress *na = be->getNetAddress();  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->put_M_DEREGISTER(na->index,myDSite);
  send(msgC,-1);
}

void ProxyVar::nowGarbage(BorrowEntry* be){
  PD((PD_VAR,"nowGarbage"));  
  sendDeRegister(be);}


OZ_Term unmarshalVarRobust(MarshalerBuffer* bs, Bool isFuture, 
			     Bool isAuto, int *error)
{

  OB_Entry *ob;
  int bi;
  BYTE ec;
  OZ_Term val1 = unmarshalBorrowRobust(bs, ob, bi, ec, error);
  if (*error) return ((OZ_Term) 0);
  
  if (val1) {
    PD((UNMARSHAL,"var/chunk hit: b:%d",bi));
    // If the entity had a failed condition, propagate it.
    if (ec & PERM_FAIL)deferProxyVarProbeFault(val1,PROBE_PERM);
    return val1;}
  
  PD((UNMARSHAL,"var miss: b:%d",bi));
  ProxyVar *pvar = new ProxyVar(oz_currentBoard(),bi,isFuture);
  
  TaggedRef val = makeTaggedRef(newTaggedVar(pvar));
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
  msgC->put_M_GETSTATUS(myDSite,na->index);
  send(msgC,-1);
}
 
OZ_Term ProxyVar::statusV()
{
  if(status ==0){
    BorrowEntry *be=BT->getBorrow(getIndex());
    sendGetStatus(be);
    status= oz_newVariable();}
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
  getOrigVar()->setSuspList(unlinkSuspList());
  *vPtr=origVar;
  origVar=makeTaggedNULL();
  disposeV();
}

OZ_Term ManagerVar::statusV() {
  return getOrigVar()->getType()==OZ_VAR_FUTURE ? AtomFuture : AtomFree;
}

VarStatus ManagerVar::checkStatusV(){
  return getOrigVar()->getType()==OZ_VAR_FUTURE ?  EVAR_STATUS_FUTURE : 
    EVAR_STATUS_FREE;
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
    case OZ_EVAR_EMANAGER:
      Assert(0);
      return (VAR_MANAGER);
    case OZ_EVAR_EPROXY:
      Assert(0);
      return (VAR_PROXY);
    case OZ_EVAR_GCSTUB:
      Assert(0);
      return (VAR_PROXY);
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
  int i=getIndex();
  OwnerEntry* oe=OT->getEntry(i);
  triggerInforms(&inform,oe,i,ec);
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
    int i=getIndex();
    OwnerEntry* oe=OT->getEntry(i);
    triggerInforms(&inform,oe,i,ec);
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
    int i = GET_VAR(b, Lazy)->getIndex(); 
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
  oz_checkSuspensionList(this,pc_all);
}

void ManagerVar::wakeAll(){
  oz_checkSuspensionList(this,pc_all);
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
void recDeregister(TaggedRef tr,DSite* s)
{
  OZ_Term vars = extractVars(tr, NO);
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


