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

#include "base.hh"
#include "dpBase.hh"
#include "var.hh"
#include "thr_int.hh"
#include "msgType.hh"
#include "table.hh"
#include "gname.hh"
#include "msgbuffer.hh"
#include "dpMarshaler.hh"
#include "perdio.hh"
#include "gc.hh"

/* --- GC --- */

static inline
void gcBorrowNow(int i) { BT->getBorrow(i)->gcPO(); }

static
void gcPendBindingList(PendBinding **last){
  PendBinding *bl = *last;
  PendBinding *newBL;
  for (; bl; bl = bl->next) {
    newBL = new PendBinding();
    OZ_collectHeapTerm(bl->val,newBL->val);
    OZ_collectHeapTerm(bl->controlvar,newBL->controlvar);
    *last = newBL;
    last = &newBL->next;}
  *last=NULL;}


void ProxyVar::gcRecurseV(void)
{ 
  PD((GC,"PerdioVar b:%d",getIndex()));
  gcBorrowNow(getIndex());
  gcPendBindingList(&bindings);
} 

void ManagerVar::gcRecurseV(void)
{ 
  OT->getOwner(getIndex())->gcPO();
  PD((GC,"PerdioVar o:%d",getIndex()));
  ProxyList **last=&proxies;
  for (ProxyList *pl = proxies; pl; pl = pl->next) {
    pl->sd->makeGCMarkSite();
    ProxyList *newPL = new ProxyList(pl->sd,0);
    *last = newPL;
    last = &newPL->next;}
  *last = 0;
}

/* --- MISC --- */

// mm2
// extern
Bool checkExportable(TaggedRef var)
{
  Assert(OZ_isVariable(var));
  return (oz_isProxyVar(var) || oz_isManagerVar(var) || oz_isFree(var));
}

/* --- Common unification --- */

#define UNIFY_ERRORMSG \
   "Unification of distributed variable with term containing resources"

void PerdioVar::primBind(TaggedRef *lPtr,TaggedRef v)
{
  oz_checkSuspensionList(this, pc_std_unif);

  TaggedRef vv=oz_deref(v);
  if (isCVar(vv)) {
    OzVariable *sv=tagged2SVarPlus(vv);
    if (sv==this) return;
    oz_checkSuspensionList(sv, pc_std_unif);
    relinkSuspListTo(sv);
  }
  doBind(lPtr, v);
}


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
OZ_Return PerdioVar::unifyVarVar(TaggedRef *lPtr, TaggedRef *rPtr,
				 ByteCode *scp)
{
  TaggedRef rVal = *rPtr;
  TaggedRef lVal = *lPtr;
  if (!oz_isExtVar(rVal)) {
    // switch binding order
    if (isSimpleVar(rVal) || isFuture(rVal))  {
      return oz_cv_unify(tagged2CVar(rVal),rPtr,makeTaggedRef(lPtr),scp);
    } else {
      return FAILED;
    }
  }

  ExtVar *rVar = oz_getExtVar(rVal);

  if (getIdV()==OZ_EVAR_OBJECT) {
    if (rVar->getIdV()==OZ_EVAR_OBJECT) {
      // both are objects --> token equality
      return this==rVar ? PROCEED : FAILED;
    }
    /*
     * binding preferences
     * bind perdiovar -> proxy
     * bind url proxy -> object proxy
     */
    return oz_cv_unify(tagged2CVar(rVal),rPtr,makeTaggedRef(lPtr),scp);
  }

  Assert(getIdV()==OZ_EVAR_PROXY || getIdV()==OZ_EVAR_MANAGER);

  // Note: for perdio variables: am.isLocal == am.onToplevel
  if (scp!=0 || !am.isLocalSVar(this)) {
    // in any kind of guard then bind and trail
    oz_checkSuspensionList(tagged2SVarPlus(lVal),pc_std_unif);
    am.doBindAndTrail(lPtr,makeTaggedRef(rPtr));
    return PROCEED;
  } else {
    if ((rVar->getIdV()==OZ_EVAR_PROXY || rVar->getIdV()==OZ_EVAR_PROXY)
	&& compareNetAddress((ProxyManagerVar*)this,(ProxyManagerVar*)rVar)>=0)
      {
	return ((ProxyManagerVar*)rVar)->doBindPV(rPtr,makeTaggedRef(lPtr));
      } else {
	return doBindPV(lPtr,makeTaggedRef(rPtr));
      }
  }
}

inline
OZ_Return PerdioVar::unifyLocalVarVal(TaggedRef *lPtr, TaggedRef r)
{
  Bool ret = validV(r);
  if (!ret) return FAILED;

  // onToplevel: distributed unification
  return doBindPV(lPtr,r);
}

inline
OZ_Return PerdioVar::unifyGlobalVarVal(TaggedRef *lPtr, TaggedRef r)
{
  Bool ret = validV(r);
  if (!ret) return FAILED;
  // in guard: bind and trail
  oz_checkSuspensionList(tagged2SVarPlus(*lPtr),pc_std_unif);
  am.doBindAndTrail(lPtr,r);
  return PROCEED;
}

OZ_Return PerdioVar::unifyV(TaggedRef *lPtr, TaggedRef r, ByteCode *scp)
{
  Assert(oz_safeDeref(r)==r);
  if (oz_isRef(r)) {
    return unifyVarVar(lPtr,tagged2Ref(r),scp);
  } else {
    Bool isLocal = am.isLocalSVar(this) && scp==0;  // mm2
    if (isLocal) {
      return unifyLocalVarVal(lPtr,r);
    } else {
      return unifyGlobalVarVal(lPtr,r);
    }
  }
}

/* --- Proxy unification --- */

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

OZ_Return ProxyVar::doBindPV(TaggedRef *lPtr, TaggedRef v)
{
  PD((PD_VAR,"PerdioVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind proxy b:%d v:%s",getIndex(),toC(v)));
  if (hasVal()) {    
    return pushVal(v); // save binding for ack message, ...
  }
  
  BorrowEntry *be=BT->getBorrow(getIndex());
  OZ_Return aux = sendSurrender(be,v);
  if (aux!=PROCEED) 
    return aux;
  return setVal(v); // save binding for ack message, ...
}

static
void sendRegister(BorrowEntry *be) {
  Assert(creditSiteOut == NULL);
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_REGISTER(bs,na->index,myDSite);
  SendTo(na->site,bs,M_REGISTER,na->site,na->index);
}

//
// mm2: choose a better name! wakeUpAfterBind
//   why are threads in u.bindings not simply retried?
void ProxyVar::redirect(OZ_Term val)
{
  PD((PD_VAR,"redirect v:%s",toC(val)));
  while (bindings) {
    PD((PD_VAR,"redirect pending unify =%s",toC(bindings->val)));
    PD((THREAD_D,"start thread redirect"));
    ControlVarUnify(bindings->controlvar,val,bindings->val);
    PendBinding *tmp=bindings->next;
    bindings->dispose();
    bindings=tmp;}
}

void ProxyVar::proxyBind(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be)
{
  PD((TABLE,"REDIRECT - borrow entry hit b:%d",getIndex()));
  primBind(vPtr,val);
  be->mkRef();
  if (hasVal()) {
    PD((PD_VAR,"REDIRECT while pending"));
    redirect(val);
  }
  // dispose();
  BT->maybeFreeBorrowEntry(getIndex());
}

void ProxyVar::proxyAck(TaggedRef *vPtr, BorrowEntry *be) 
{
  PD((PD_VAR,"acknowledge"));
  OZ_Term val=bindings->val;
  primBind(vPtr,val);
  PD((THREAD_D,"start thread ackowledge"));
  ControlVarResume(bindings->controlvar);
  PendBinding *tmp=bindings->next;
  bindings->dispose();
  bindings=tmp;
  redirect(val);
  be->mkRef();
  BT->maybeFreeBorrowEntry(getIndex());
  // dispose();
}

/* --- Manager unification --- */

static
void sendAcknowledge(DSite* sd,int OTI){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);  
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_ACKNOWLEDGE(bs,myDSite,OTI);
  SendTo(sd,bs,M_ACKNOWLEDGE,myDSite,OTI);
}

OZ_Return ManagerVar::sendRedirectToProxies(OZ_Term val, DSite* ackSite)
{
  ProxyList *pl = getProxies();
  OwnerEntry *oe = OT->getOwner(getIndex());
  if (pl) { // perdio vars are not yet localized again
    do {
      DSite* sd = pl->sd;
      if (sd==ackSite) {
	sendAcknowledge(sd,getIndex());
      } else {
	OZ_Return ret = sendRedirect(sd,getIndex(),val);
	if (ret != PROCEED) return ret;
      }
      ProxyList *tmp = pl->next;
      pl->dispose();
      pl = tmp;
    } while (pl);
  }
  return PROCEED;
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

OZ_Return ManagerVar::doBindPV(TaggedRef *lPtr, TaggedRef v)
{
  PD((PD_VAR,"PerdioVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind manager o:%d v:%s",getIndex(),toC(v)));
  OZ_Return aux = sendRedirectToProxies(v, myDSite);
  if (aux == PROCEED) {
    OT->getOwner(getIndex())->mkRef();
    primBind(lPtr,v);
  }
  return aux;
} 

void ManagerVar::managerBind(TaggedRef *vPtr, TaggedRef val,
			     OwnerEntry *oe, DSite *rsite)
{
  primBind(vPtr,val);
  oe->mkRef();
  if (oe->hasFullCredit()) {
    PD((WEIRD,"SURRENDER: full credit"));
  }
  sendRedirectToProxies(val,rsite);
}

/* --- Marshal --- */

static
PerdioVar *var2PerdioVar(TaggedRef *tPtr)
{
  if (oz_isManagerVar(*tPtr)||oz_isProxyVar(*tPtr)||oz_isObjectVar(*tPtr)) {
    return (PerdioVar*)oz_getExtVar(*tPtr);
  }

  // mm2: handle futures
  if (!oz_isFree(*tPtr)) return 0;

  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  PD((GLOBALIZING,"globalize var index:%d",i));

  oe->mkVar(makeTaggedRef(tPtr));

  ManagerVar *ret = new ManagerVar(oz_currentBoard(),i);

  if (isCVar(*tPtr))
    ret->setSuspList(tagged2SVarPlus(*tPtr)->getSuspList());
  doBindCVar(tPtr,ret);
  return ret;
}

// Returning 'NO' means we are going to proceed with 'marshal bomb';
Bool marshalVariable(TaggedRef *tPtr, MsgBuffer *bs) {
  if (!bs->globalize())
    return checkExportable(*tPtr);

  PerdioVar *pvar = var2PerdioVar(tPtr);
  if (pvar==NULL) {
    return (NO);
  }
  pvar->marshalV(bs);
  return (OK);
}

void ProxyVar::marshalV(MsgBuffer *bs)
{
  DSite *sd=bs->getSite();
  int i=getIndex();
  PD((MARSHAL,"var proxy o:%d",i));
  if(sd && borrowTable->getOriginSite(i)==sd) {
    marshalToOwner(i,bs);
    return;}
  marshalBorrowHead(DIF_VAR,i,bs);
}

void ManagerVar::marshalV(MsgBuffer *bs)
{
  DSite *sd=bs->getSite();
  int i=getIndex();
  PD((MARSHAL,"var manager o:%d",i));
  marshalOwnHead(DIF_VAR,i,bs);
}

void ObjectVar::marshalV(MsgBuffer *bs)
{
  PD((MARSHAL,"var objectproxy"));

  if (checkCycleOutLine(*(getObject()->getCycleRef()),bs,OZCONST))
    return;

  GName *classgn =  isObjectClassAvail()
    ? globalizeConst(getClass(),bs) : getGNameClass();

  marshalObject(getObject(),bs,classgn);
}

// extern
OZ_Term unmarshalVar(MsgBuffer* bs){
  OB_Entry *ob;
  int bi;
  OZ_Term val1 = unmarshalBorrow(bs,ob,bi);
  
  if (val1) {
    PD((UNMARSHAL,"var/chunk hit: b:%d",bi));
    return val1;}
  
  PD((UNMARSHAL,"var miss: b:%d",bi));
  ProxyVar *pvar = new ProxyVar(oz_currentBoard(),bi);
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  ob->mkVar(val); 
  sendRegister((BorrowEntry *)ob);
  return val;
}

/* --- IsVar test --- */

static
OZ_Term sendIsDet(BorrowEntry *be){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  OZ_Term var = OZ_newVariable();
  marshal_M_ISDET(bs,na->index,var);
  SendTo(na->site,bs,M_ISDET,na->site,na->index);
  return var;
}
 
OZ_Term ProxyVar::isDetV()
{
  BorrowEntry *be=BT->getBorrow(getIndex());
  return sendIsDet(be);
}

/* --- ObjectProxis --- */

// extern
TaggedRef newObjectProxy(Object *o, GName *gnobj,
			 GName *gnclass, TaggedRef clas)
{
  ObjectVar *pvar = new ObjectVar(o,oz_currentBoard());
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  addGName(gnobj, val);
  if (gnclass) {
    pvar->setGNameClass(gnclass);
  } else {
    pvar->setClass(tagged2ObjectClass(oz_deref(clas)));
  }
  return val;
}


static
void sendHelpX(MessageType mt,BorrowEntry *be)
{
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  switch (mt) {
  case M_GET_OBJECT:
    marshal_M_GET_OBJECT(bs,na->index,myDSite);
    break;
  case M_GET_OBJECTANDCLASS:
    marshal_M_GET_OBJECTANDCLASS(bs,na->index,myDSite);
    break;
  default:
    Assert(0);
  }
  SendTo(na->site,bs,mt,na->site,na->index);
}

void ObjectVar::addSuspV(TaggedRef * v, Suspension susp, int unstable)
{
  addSuspSVar(susp, unstable);
  if (isObjectClassNotAvail()) {
    MessageType mt; 
    if(oz_findGName(getGNameClass())==0) {mt=M_GET_OBJECTANDCLASS;}
    else {mt=M_GET_OBJECT;}
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());
    sendHelpX(mt,be);
  } else {
    Assert(isObjectClassAvail());
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());      
    sendHelpX(M_GET_OBJECT,be);
  }
}

void ObjectVar::gcRecurseV(void)
{
  gcBorrowNow(getObject()->getIndex());
  obj = getObject()->gcObject();
  if (isObjectClassAvail()) {
    u.aclass = u.aclass->gcClass();}
}

void ObjectVar::primBind(TaggedRef *lPtr,TaggedRef v)
{
  PerdioVar::primBind(lPtr,v);
  if (isObjectClassNotAvail()) {
    deleteGName(u.gnameClass);
  }
}

OZ_Return ObjectVar::doBindPV(TaggedRef *lPtr, TaggedRef v)
{
  PD((PD_VAR,"PerdioVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind object u:%s",toC(makeTaggedConst(getObject()))));
  primBind(lPtr,v);
  return PROCEED;
} 
