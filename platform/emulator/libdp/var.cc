/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Per Brand (perbrand@sics.se)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Copyright:
 *    Michael Mehl (1997,1998)
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
#pragma implementation "var.hh"
#endif

#include "var.hh"
#include "var_ext.hh"
#include "var_obj.hh"
#include "dpMarshaler.hh"
#include "unify.hh"
#include "var_simple.hh"
#include "var_future.hh"
#include "chain.hh"

/* --- Common unification --- */

#define UNIFY_ERRORMSG \
   "Unification of distributed variable with term containing resources"


// compare NAs
#define GET_ADDR(var,SD,OTI)                                            \
DSite* SD;int OTI;                                                      \
if (var->getIdV()==OZ_EVAR_PROXY) {                                     \
  NetAddress *na=BT->getBorrow(var->getIndex())->getNetAddress();       \
  SD=na->site;                                                          \
  OTI=na->index;                                                        \
} else {                                                                \
  SD=myDSite;                                                           \
  OTI=var->getIndex();                                                  \
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

inline
OZ_Return ProxyManagerVar::unifyV(TaggedRef *lPtr, TaggedRef *rPtr)
{
  TaggedRef rVal = *rPtr;

  if (!oz_isExtVar(rVal)) {
    // switch order
    if (isSimpleVar(rVal))  {
      return oz_var_bind(tagged2CVar(rVal),rPtr,makeTaggedRef(lPtr));
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

static
void sendRequested(BorrowEntry *be){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_REQUESTED(bs,na->index);
  SendTo(na->site,bs,M_REQUESTED,na->site,na->index);
}

OZ_Return ProxyVar::addSuspV(TaggedRef *, Suspension susp, int unstable)
{
  // mm2: always send requested, maybe this should be done only once!
  BorrowEntry *be=BT->getBorrow(getIndex());
  sendRequested(be);

  addSuspSVar(susp, unstable);
  return SUSPEND;
}

void ProxyVar::gcRecurseV(void)
{
  PD((GC,"ProxyVar b:%d",getIndex()));
  BT->getBorrow(getIndex())->gcPO();
  OZ_collect(&binding);
  setInfo(gcEntityInfoInternal(getInfo()));
}

static
OZ_Return sendSurrender(BorrowEntry *be,OZ_Term val){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_SURRENDER(bs,na->index,myDSite,val);
  CheckNogoods(val,bs,"unify:resources",UNIFY_ERRORMSG,);
  SendTo(na->site,bs,M_SURRENDER,na->site,na->index);
  return PROCEED;
}

Bool dealWithHandlers(EntityInfo *info,EntityCond ec,Thread* th,Bool &hit){
  Assert(isHandlerCondition(ec));
  info->meToBlocked();

  Watcher* w;
  Watcher** base= info->getWatcherBase();
  while(TRUE){
    if((*base)==NULL) return FALSE;
    if(((*base)->watchcond) & ec){
      if(((*base)->isSiteBased())) break;
      else{
        if(th==(*base)->thread) break;}}
    base = &((*base)->next);}
  (*base)->varInvokeHandler(ec,hit);
  hit=TRUE;
  if(!(*base)->isPersistent()){
    return TRUE;
    *base=(*base)->next;}
  return FALSE;
}

inline EntityCond handlerPart(EntityCond ec){
  return ec & (PERM_ME|TEMP_ME);}

Bool varFailurePreemption(EntityInfo* info,Bool &hit){
  EntityCond ec=handlerPart(info->getEntityCond());
  if(ec==ENTITY_NORMAL) return FALSE;
  return dealWithHandlers(info,ec,oz_currentThread(),hit);}

Bool ProxyVar::failurePreemption(){
  Assert(info!=NULL);
  Bool hit=FALSE;
  EntityCond oldC=info->getSummaryWatchCond();
  if(varFailurePreemption(info,hit)){
    EntityCond newC=info->getSummaryWatchCond();
    varAdjustPOForFailure(getIndex(),oldC,newC);}
  return hit;
}

OZ_Return ProxyVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  PD((PD_VAR,"ProxyVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind proxy b:%d v:%s",getIndex(),toC(r)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    if(!errorIgnore()){
      if(failurePreemption()) return BI_REPLACEBICALL;}
    if (!binding) {
      BorrowEntry *be=BT->getBorrow(getIndex());
      OZ_Return aux = sendSurrender(be,r);
      if (aux!=PROCEED) return aux;
      PD((THREAD_D,"stop thread proxy bind %x",oz_currentThread()));
      binding=r;
    }
    am.addSuspendVarList(lPtr);
    return SUSPEND;
  } else {
    // in guard: bind and trail
    oz_bindGlobalVar(this,lPtr,r);
    return PROCEED;
  }
}

void ProxyVar::redirect(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be)
{
  int BTI=getIndex();
  PD((TABLE,"REDIRECT - borrow entry hit b:%d",BTI));
  if (binding) {
    DebugCode(binding=0);
    PD((PD_VAR,"REDIRECT while pending"));
  }
  oz_bindLocalVar(this,vPtr,val);
  be->changeToRef();
  BT->maybeFreeBorrowEntry(BTI);
}

void ProxyVar::acknowledge(TaggedRef *vPtr, BorrowEntry *be)
{
  int BTI=getIndex();
  PD((PD_VAR,"acknowledge"));

  oz_bindLocalVar(this,vPtr,binding);

  be->changeToRef();
  BT->maybeFreeBorrowEntry(BTI);
}

/* --- ManagerVar --- */

OZ_Return ManagerVar::addSuspV(TaggedRef *vPtr, Suspension susp, int unstable)
{
  if (origVar->getType()==OZ_VAR_FUTURE) {
    if (((Future *)origVar)->kick(vPtr))
      return PROCEED;
  }
  addSuspSVar(susp, unstable);
  return SUSPEND;
}

void ManagerVar::gcRecurseV(void)
{
  origVar=origVar->gcVar();
  origVar->gcVarRecurse();
  OT->getOwner(getIndex())->gcPO();
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
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_ACKNOWLEDGE(bs,myDSite,OTI);
  SendTo(sd,bs,M_ACKNOWLEDGE,myDSite,OTI);
}

// extern
OZ_Return sendRedirect(DSite* sd,int OTI,TaggedRef val)
{
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_REDIRECT(bs,myDSite,OTI,val);
  CheckNogoods(val,bs,"unify:resources",UNIFY_ERRORMSG,);
  SendTo(sd,bs,M_REDIRECT,myDSite,OTI);
  return PROCEED;
}

OZ_Return ManagerVar::sendRedirectToProxies(OZ_Term val, DSite* ackSite)
{
  ProxyList *pl = proxies;
  // Assert(pl); // dist. vars are not yet localized again
  while (pl) {
    DSite* sd = pl->sd;
    if (sd==ackSite) {
      sendAcknowledge(sd,getIndex());
    } else {
      OZ_Return ret = sendRedirect(sd,getIndex(),val);
      if (ret != PROCEED) {
        Assert(pl==proxies); // the first redirect must fail!
        return ret;
      }
    }
    pl=pl->dispose();
  }
  proxies = 0;
  return PROCEED;
}

OZ_Return ManagerVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  int OTI=getIndex();
  PD((PD_VAR,"ManagerVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind manager o:%d v:%s",OTI,toC(*lPtr)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    if(isFuture()){
      //    if (origVar->getType()==OZ_VAR_FUTURE) {
      am.addSuspendVarList(lPtr);
      return SUSPEND;
    }
    // because of failure ManagerVar need not be in OwnerTable
    // can now localize this variable

    // send redirect done first to check if r is exportable
    OZ_Return ret = sendRedirectToProxies(r, myDSite);
    if (ret != PROCEED) return ret;
    oz_bindLocalVar(this,lPtr,r);
    OT->getOwner(OTI)->changeToRef();
    return PROCEED;
  } else {
    oz_bindGlobalVar(this,lPtr,r);
    return PROCEED;
  }
}

void ManagerVar::getStatus(DSite* site,int OTI, TaggedRef tr){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(site);
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_SENDSTATUS(bs,myDSite,OTI,tr);
  SendTo(site,bs,M_SENDSTATUS,myDSite,OTI);
}

void ProxyVar::receiveStatus(TaggedRef tr){
  Assert(status!=0);
  SiteUnify(status,tr);
}


OZ_Return ManagerVar::forceBindV(TaggedRef *lPtr, TaggedRef r)
{
  int OTI=getIndex();
  PD((PD_VAR,"ManagerVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind manager o:%d v:%s",OTI,toC(*lPtr)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    // send redirect done first to check if r is exportable
    OZ_Return ret = sendRedirectToProxies(r, myDSite);
    if (ret != PROCEED) return ret;
    oz_bindLocalVar(this,lPtr,r);
    OT->getOwner(OTI)->changeToRef();
    if (OT->getOwner(OTI)->hasFullCredit()) {
      PD((WEIRD,"SURRENDER: full credit"));
    }
    return PROCEED;
  } else {
    oz_bindGlobalVar(this,lPtr,r);
    return PROCEED;
  }
}

void ManagerVar::surrender(TaggedRef *vPtr, TaggedRef val)
{
  OZ_Return ret = bindV(vPtr,val);
  if (ret == SUSPEND) {
    Assert(origVar->getType()==OZ_VAR_FUTURE);
    Bool ret=((Future *)origVar)->kick(vPtr);
    Assert(!ret);
    am.emptySuspendVarList();
    return;
  }
  Assert(ret==PROCEED);
}

void ManagerVar::requested(TaggedRef *vPtr)
{
  if(isFuture()){
    int ret = ((Future *)origVar)->kick(vPtr);
  }
}

/* --- Marshal --- */

void ManagerVar::marshal(MsgBuffer *bs)
{
  int i=getIndex();
  PD((MARSHAL,"var manager o:%d",i));
  if(isFuture()){
    marshalOwnHead(DIF_FUTURE,i,bs);}
  else{
    marshalOwnHead(DIF_VAR,i,bs);}
}

void ProxyVar::marshal(MsgBuffer *bs)
{
  DSite *sd=bs->getSite();
  int i=getIndex();
  PD((MARSHAL,"var proxy o:%d",i));
  if(sd && borrowTable->getOriginSite(i)==sd) {
    marshalToOwner(i,bs);}
  else {
    if(isFuture()){
      marshalBorrowHead(DIF_FUTURE,i,bs);}
    else{
      marshalBorrowHead(DIF_VAR,i,bs);}
  }
}

ManagerVar* globalizeFreeVariable(TaggedRef *tPtr,TaggedRef var){
  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  PD((GLOBALIZING,"globalize var index:%d",i));
  oe->mkVar(makeTaggedRef(tPtr));
  ManagerVar *mv = new ManagerVar(oz_getVar(tPtr),i);
  if (isCVar(var)) {
    OzVariable *cv=tagged2CVar(var);
    mv->setSuspList(cv->unlinkSuspList());
  }
  *tPtr=makeTaggedCVar(mv);
  return mv;
}

// mm3 tPtr comes originally from DEREF(_,_,tPtr)
ManagerVar* globalizeFreeVariable(TaggedRef *tPtr){
  return globalizeFreeVariable(tPtr,*tPtr);}

// Returning 'NO' means we are going to proceed with 'marshal bomb';
Bool marshalVariableImpl(TaggedRef *tPtr, MsgBuffer *bs) {
  const TaggedRef var = *tPtr;
  if (oz_isManagerVar(var)) {
    if (!bs->globalize()) return TRUE;
    oz_getManagerVar(var)->marshal(bs);
  } else if (oz_isProxyVar(var)) {
    if (!bs->globalize()) return TRUE;
    oz_getProxyVar(var)->marshal(bs);
  } else if (oz_isObjectVar(var)) {
    Assert(bs->globalize());
    oz_getObjectVar(var)->marshal(bs);
  } else if (oz_isFree(var) || isFuture(var)) {
    if (!bs->globalize()) return TRUE;
    globalizeFreeVariable(tPtr,var)->marshal(bs);
  } else {
    if (!bs->globalize()) return FALSE;
    return FALSE;
  }
  return TRUE;
}

/* --- Unmarshal --- */

static
void sendRegister(BorrowEntry *be) {
  Assert(creditSiteOut == NULL);
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_REGISTER(bs,na->index,myDSite);
  SendTo(na->site,bs,M_REGISTER,na->site,na->index);
}

// PER-LOOK why indirection
static OZ_Term unmarshalVarAux(MsgBuffer* bs, Bool isFuture,Bool isAuto)
{
  OB_Entry *ob;
  int bi;
  OZ_Term val1 = unmarshalBorrow(bs,ob,bi);

  if (val1) {
    PD((UNMARSHAL,"var/chunk hit: b:%d",bi));
    return val1;}

  PD((UNMARSHAL,"var miss: b:%d",bi));
  ProxyVar *pvar = new ProxyVar(oz_currentBoard(),bi,isFuture);

  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  ob->mkVar(val);
  if(!isAuto) sendRegister((BorrowEntry *)ob);
  else Assert(0); // PER-LOOK
  return val;
}


// extern
OZ_Term unmarshalVarImpl(MsgBuffer* bs, Bool isFuture, Bool isAuto){
  return unmarshalVarAux(bs,isFuture,isAuto);
}

/* --- IsVar test --- */

static
void sendGetStatus(BorrowEntry *be){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_GETSTATUS(bs,myDSite,na->index);
  SendTo(na->site,bs,M_GETSTATUS,na->site,na->index);
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
  if(getInfo()!=NULL) return;
  origVar->setSuspList(unlinkSuspList());
  *vPtr=makeTaggedCVar(origVar);
  origVar=0;
  disposeV();
}

OZ_Term ManagerVar::statusV() {
  return origVar->getType()==OZ_VAR_FUTURE ? AtomFuture : AtomFree;
}

VarStatus ManagerVar::checkStatusV(){
  return origVar->getType()==OZ_VAR_FUTURE ?  EVAR_STATUS_FUTURE :
    EVAR_STATUS_FREE;
}

void oz_dpvar_localize(TaggedRef *vPtr) {
  Assert(classifyVar(vPtr)==VAR_MANAGER);
  oz_getManagerVar(*vPtr)->localize(vPtr);
}


// FAILURE structure fundamentals

// mm3 tPtr comes from DEREF(_,_,tPtr)

VarKind classifyVarLim(TaggedRef tr){
  if(oz_isProxyVar(tr)){
    return VAR_PROXY;}
  if(oz_isManagerVar(tr)){
    return VAR_MANAGER;}
  if(oz_isObjectVar(tr)){
    return VAR_OBJECT;}
  Assert(0);
  return VAR_FREE;
}

VarKind classifyVar(TaggedRef* tPtr){
  TaggedRef tr= *tPtr;
  if(oz_isProxyVar(tr)){
    return VAR_PROXY;}
  if(oz_isManagerVar(tr)){
    return VAR_MANAGER;}
  if(oz_isObjectVar(tr)){
    return VAR_OBJECT;}
  if(oz_isFree(tr)){
    return VAR_FREE;}
  if(isFuture(tr)){
    return VAR_FUTURE;}
  return VAR_KINDED;
}


EntityInfo* varGetEntityInfo(TaggedRef* tPtr){
  switch(classifyVar(tPtr)){
  case VAR_MANAGER:
    return oz_getManagerVar(*tPtr)->getInfo();
  case VAR_PROXY:
    return oz_getProxyVar(*tPtr)->getInfo();
  case VAR_OBJECT:
    return oz_getObjectVar(*tPtr)->getInfo();
  default:
    Assert(0);}
  return NULL;}

EntityInfo* varMakeEntityInfo(TaggedRef* tPtr){
  EntityInfo* ei= new EntityInfo();
  switch(classifyVar(tPtr)){
  case VAR_MANAGER:
    oz_getManagerVar(*tPtr)->setInfo(ei);
    return ei;
  case VAR_PROXY:
    oz_getProxyVar(*tPtr)->setInfo(ei);
    return ei;
  case VAR_OBJECT:
    oz_getObjectVar(*tPtr)->setInfo(ei);
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
  Bool hit=FALSE;
  if(info==NULL) info= new EntityInfo();
  if(!info->addEntityCond(ec)) return;
  if(isHandlerCondition(ec)) {
    wakeAll();
    return;}
  info->dealWithWatchers(ec);}

void ProxyVar::newWatcher(Bool b){
  if(b){
    wakeAll();
    return;}
  info->dealWithWatchers(info->getEntityCond());
}

void ProxyVar::subEntityCond(EntityCond ec){
  Assert(info!=NULL);
  info->subEntityCond(ec);
}

void ManagerVar::newWatcher(Bool b){
  if(b) return;
  info->dealWithWatchers(info->getEntityCond());
}

void ManagerVar::addEntityCond(EntityCond ec){
  Assert((ec & (TEMP_SOME|PERM_SOME))==ec);
  Assert(!isHandlerCondition(ec));
  if(info==NULL) info= new EntityInfo();
  if(!info->addEntityCond(ec)) return;
  int i=getIndex();
  OwnerEntry* oe=OT->getOwner(i);
  triggerInforms(&inform,oe,i,ec);
  info->dealWithWatchers(ec);
}

void ManagerVar::subEntityCond(EntityCond ec){
  Assert(ec==TEMP_SOME);
  Assert(info!=NULL);
  info->subEntityCond(ec);
  int i=getIndex();
  OwnerEntry* oe=OT->getOwner(i);
  triggerInforms(&inform,oe,i,ec);
}

Bool ManagerVar::siteInProxyList(DSite* s){
  ProxyList* pl=proxies;
  while(pl!=NULL){
    if(pl->sd==s) return TRUE;
    pl=pl->next;}
  return FALSE;
}

void ManagerVar::probeFault(DSite *s,int pr){
  if(inform==NULL) return;
  if(!siteInProxyList(s)) return;
  if(pr==PROBE_PERM){
    addEntityCond(PERM_SOME);
    return;}
  if(pr==PROBE_TEMP){
    addEntityCond(TEMP_SOME);
    return;}
  Assert(pr==PROBE_OK);
  subEntityCond(TEMP_SOME);
}

void ProxyVar::probeFault(int pr){
  if(pr==PROBE_PERM){
    if(binding!=0){
      addEntityCond(PERM_ME|PERM_BLOCKED);
      return;}
    addEntityCond(PERM_ME);
    return;}
  if(pr==PROBE_TEMP){
    if(binding!=0){
      addEntityCond(TEMP_ME|TEMP_BLOCKED);
      return;}
    addEntityCond(TEMP_ME);
    return;}
  Assert(pr==PROBE_OK);
  if(binding!=0){
      subEntityCond(TEMP_ME|TEMP_BLOCKED);
      return;}
  subEntityCond(TEMP_ME);
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
  case OZ_EVAR_OBJECT:
    return VAR_OBJECT;
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
  case VAR_OBJECT:
    ie= GET_VAR(b,Object)->getInfo();
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
  case VAR_OBJECT:
    ie= GET_VAR(b,Object)->getInfo();
    if(ie==NULL) return;
    Assert(0);
    //    int i=GET_VAR(b,Object)->getIndex(); PER-LOOK
    //    varAdjustPOForFailure(i,ie->getEntityCond(),ENTITY_NORMAL);
    return;
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
