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

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "perdiovar.hh"
#endif

#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#include "perdiovar.hh"
#include "threadInterface.hh"
#include "pickle.hh"
#include "dp_msgType.hh"
#include "dp_table.hh"
#include "dp_gname.hh"
#include "comm.hh"
#include "msgbuffer.hh"


// from perdio.cc
void sendHelpX(MessageType mt,BorrowEntry *be);
void marshalToOwner(int bi,MsgBuffer *bs);
void marshalBorrowHead(MarshalTag tag, int bi,MsgBuffer *bs);
void marshalOwnHead(int tag,int i,MsgBuffer *bs);
OZ_Term unmarshalBorrow(MsgBuffer *bs,OB_Entry *&ob,int &bi);
void SendTo(Site *toS,MsgBuffer *bs,MessageType mt,Site *sS,int sI);

// from components
OZ_Return raiseGeneric(char *msg, OZ_Term arg);

// from marshaler
void marshalObjVar(OldPerdioVar *pv,MsgBuffer *bs);

/* -------------------------------------------------------------------- */

void OldPerdioVar::addSuspV(TaggedRef * v, Suspension susp, int unstable)
{
  if (suspList!=NULL) {
    addSuspSVar(susp, unstable);
    return;
  }

  addSuspSVar(susp, unstable);

  if (isObjectClassNotAvail()) {
    MessageType mt; 
    if(oz_findGName(getGNameClass())==0) {mt=M_GET_OBJECTANDCLASS;}
    else {mt=M_GET_OBJECT;}
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());
    sendHelpX(mt,be);
    return;}
  
  if (isObjectClassAvail()) {
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());      
    sendHelpX(M_GET_OBJECT,be);
    return;
  }
}

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

void OldPerdioVar::gcRecurseV(void)
{ 
  if (isProxy()) {
    PD((GC,"PerdioVar b:%d",getIndex()));
    gcBorrowNow(getIndex());
    gcPendBindingList(&u.bindings);
    return;
  } 
  if (isManager()) {
    OT->getOwner(getIndex())->gcPO();
    PD((GC,"PerdioVar o:%d",getIndex()));
    ProxyList **last=&u.proxies;
    for (ProxyList *pl = u.proxies; pl; pl = pl->next) {
      pl->sd->makeGCMarkSite();
      ProxyList *newPL = new ProxyList(pl->sd,0);
      *last = newPL;
      last = &newPL->next;}
    *last = 0;
    
    return;
  }
  Assert(isObject());
  gcBorrowNow(getObject()->getIndex());
  ptr = getObject()->gcObject();
  if (isObjectClassAvail()) {
    u.aclass = u.aclass->gcClass();}
}

// extern
OldPerdioVar *var2PerdioVar(TaggedRef *tPtr)
{
  if (isPerdioVar(*tPtr) ) {
    return tagged2PerdioVar(*tPtr);
  }

  // mm2: handle futures
  if (!oz_isFree(*tPtr)) return 0;

  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  PD((GLOBALIZING,"globalize var index:%d",i));

  oe->mkVar(makeTaggedRef(tPtr));

  OldPerdioVar *ret = new OldPerdioVar(oz_currentBoard());
  ret->setIndex(i);

  if (isCVar(*tPtr))
    ret->setSuspList(tagged2SVarPlus(*tPtr)->getSuspList());
  doBindCVar(tPtr,ret);
  return ret;
}

// extern
void marshalVar(OldPerdioVar *pvar,MsgBuffer *bs)
{
  Site *sd=bs->getSite();
  if (pvar->isProxy()) {
    int i=pvar->getIndex();
    PD((MARSHAL,"var proxy o:%d",i));
    if(sd && borrowTable->getOriginSite(i)==sd) {
      marshalToOwner(i,bs);
      return;}
    marshalBorrowHead(DIF_VAR,i,bs);
  } else {
    Assert(pvar->isManager());
    int i=pvar->getIndex();
    PD((MARSHAL,"var manager o:%d",i));
    marshalOwnHead(DIF_VAR,i,bs);
  }
}

// compare NAs
#define GET_ADDR(var,SD,OTI)						\
Site* SD;int OTI;							\
if (var->isProxy()) {							\
  NetAddress *na=BT->getBorrow(var->getIndex())->getNetAddress();	\
  SD=na->site;								\
  OTI=na->index;							\
} else {								\
  SD=mySite;                                                            \
  OTI=var->getIndex();							\
}

static
int compareNetAddress(OldPerdioVar *lVar,OldPerdioVar *rVar)
{
  GET_ADDR(lVar,lSD,lOTI);
  GET_ADDR(rVar,rSD,rOTI);
  int ret = lSD->compareSites(rSD);
  if (ret != 0) return ret;
  return lOTI<rOTI ? -1 : 1;
}

static
void sendRegister(BorrowEntry *be) {
  Assert(creditSiteOut == NULL);
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_REGISTER(bs,na->index,mySite);
  SendTo(na->site,bs,M_REGISTER,na->site,na->index);}

// extern
OZ_Term unmarshalVar(MsgBuffer* bs){
  OB_Entry *ob;
  int bi;
  OZ_Term val1 = unmarshalBorrow(bs,ob,bi);
  
  if (val1) {
    PD((UNMARSHAL,"var/chunk hit: b:%d",bi));
    return val1;}
  
  PD((UNMARSHAL,"var miss: b:%d",bi));
  OldPerdioVar *pvar = new OldPerdioVar(bi,oz_currentBoard());
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  ob->mkVar(val); 
  sendRegister((BorrowEntry *)ob);
  return val;
}

#define UNIFY_ERRORMSG \
   "Unification of distributed variable with term containing resources"

static
OZ_Return sendSurrender(BorrowEntry *be,OZ_Term val){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_SURRENDER(bs,na->index,mySite,val);
  CheckNogoods(val,bs,UNIFY_ERRORMSG,);
  SendTo(na->site,bs,M_SURRENDER,na->site,na->index);
  return PROCEED;
}

// extern
OZ_Return sendRedirect(Site* sd,int OTI,TaggedRef val){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_REDIRECT(bs,mySite,OTI,val);
  CheckNogoods(val,bs,UNIFY_ERRORMSG,);
  SendTo(sd,bs,M_REDIRECT,mySite,OTI);
  return PROCEED;
}

static
void sendAcknowledge(Site* sd,int OTI){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);  
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_ACKNOWLEDGE(bs,mySite,OTI);
  SendTo(sd,bs,M_ACKNOWLEDGE,mySite,OTI);
}

void OldPerdioVar::acknowledge(OZ_Term *p){
  PD((PD_VAR,"acknowledge"));
  OZ_Term val=u.bindings->val;
  primBind(p,val);
  PD((THREAD_D,"start thread ackowledge"));
  ControlVarResume(u.bindings->controlvar);
  PendBinding *tmp=u.bindings->next;
  u.bindings->dispose();
  u.bindings=tmp;
  redirect(val);
}

// mm2: choose a better name! wakeUpAfterBind
//   why are threads in u.bindings not simply retried?
void OldPerdioVar::redirect(OZ_Term val) {
  PD((PD_VAR,"redirect v:%s",toC(val)));
  while (u.bindings) {
    PD((PD_VAR,"redirect pending unify =%s",toC(u.bindings->val)));
    PD((THREAD_D,"start thread redirect"));
    ControlVarUnify(u.bindings->controlvar,val,u.bindings->val);
    PendBinding *tmp=u.bindings->next;
    u.bindings->dispose();
    u.bindings=tmp;}
}

static
OZ_Return sendRedirectToProxies(OldPerdioVar *pv,OZ_Term val,
				Site* ackSite, int OTI)
{
  ProxyList *pl = pv->getProxies();
  OwnerEntry *oe = OT->getOwner(OTI);
  if (!pl) {
    MsgBuffer *bs=msgBufferManager->getMsgBuffer(NULL);
    marshal_M_REDIRECT(bs,mySite,OTI,val);
    CheckNogoods(val,bs,UNIFY_ERRORMSG,);
  } else {
      do {
	  Site* sd = pl->sd;
	  if (sd==ackSite) {
	      sendAcknowledge(sd,OTI);
	  } else {
	      OZ_Return ret = sendRedirect(sd,OTI,val);
	      if (ret != PROCEED) return ret;
	  }
	  ProxyList *tmp = pl->next;
	  pl->dispose();
	  pl = tmp;
      } while (pl);
  }
  return PROCEED;
}

// local bind request
static
OZ_Return bindPerdioVar(OldPerdioVar *pv,TaggedRef *lPtr,TaggedRef v)
{
  PD((PD_VAR,"bindPerdioVar by thread: %x",oz_currentThread()));
  if (pv->isManager()) {
    PD((PD_VAR,"bind manager o:%d v:%s",pv->getIndex(),toC(v)));
    OZ_Return aux = sendRedirectToProxies(pv,v,mySite,pv->getIndex());
    if (aux == PROCEED) {
      OT->getOwner(pv->getIndex())->mkRef();
      pv->primBind(lPtr,v);
    }
    return aux;
  } 
  if (pv->isObject()) {
    PD((PD_VAR,"bind object u:%s",toC(makeTaggedConst(pv->getObject()))));
    pv->primBind(lPtr,v);
    return PROCEED;
  } 

  PD((PD_VAR,"bind proxy b:%d v:%s",pv->getIndex(),toC(v)));
  Assert(pv->isProxy());
  if (pv->hasVal()) {    
    return pv->pushVal(v); // save binding for ack message, ...
  }
  
  BorrowEntry *be=BT->getBorrow(pv->getIndex());
  OZ_Return aux = sendSurrender(be,v);
  if (aux!=PROCEED) 
    return aux;
  return pv->setVal(v); // save binding for ack message, ...
}

void OldPerdioVar::primBind(TaggedRef *lPtr,TaggedRef v)
{
  oz_checkSuspensionList(this, pc_std_unif);

  TaggedRef vv=oz_deref(v);
  if (isCVar(vv)) {
    SVariable *sv=tagged2SVarPlus(vv);
    if (sv==this) return;
    oz_checkSuspensionList(sv, pc_std_unif);
    relinkSuspListTo(sv);
  }
  doBind(lPtr, v);
  if (isObjectClassNotAvail()) {
    deleteGName(u.gnameClass);
  }
}

OZ_Return OldPerdioVar::unifyV(TaggedRef *lPtr, TaggedRef r, ByteCode *scp)
{
  if (oz_isRef(r)) {
    TaggedRef *rPtr = tagged2Ref(r);
    TaggedRef rVal = *rPtr;
    TaggedRef lVal = *lPtr;
    GenCVariable *cv = tagged2CVar(rVal);

    if (cv->getType()!=PerdioVariable) return FAILED;

    OldPerdioVar *lVar = this;

    OldPerdioVar *rVar = (OldPerdioVar *)cv;

    if (isObject()) {
      if (rVar->isObject()) {
	// both are objects --> token equality
	return lVar==rVar ? PROCEED : FAILED;
      }
      if (!rVar->isObject()) {
	/*
	 * binding preferences
	 * bind perdiovar -> proxy
	 * bind url proxy -> object proxy
	 */
	Swap(rVal,lVal,TaggedRef);
	Swap(rPtr,lPtr,TaggedRef*);
      }
    }

    PD((PD_VAR,"unify i:%d i:%d",lVar->getIndex(),rVar->getIndex()));

    // Note: for perdio variables: am.isLocal == am.onToplevel
    if (scp!=0 || !am.isLocalSVar(lVar)) {
      // in any kind of guard then bind and trail
      oz_checkSuspensionList(tagged2SVarPlus(lVal),pc_std_unif);
      am.doBindAndTrail(lPtr,makeTaggedRef(rPtr));
      return PROCEED;
    } else {
      // not in guard: distributed unification
      Assert(am.isLocalSVar(rVar));
      int cmp = compareNetAddress(lVar,rVar);
      Assert(cmp!=0);
      if (cmp<0) {
	return bindPerdioVar(lVar,lPtr,makeTaggedRef(rPtr));
      } else {
	return bindPerdioVar(rVar,rPtr,makeTaggedRef(lPtr));
      }
    }
  } // both PVARs


  // PVAR := non PVAR
  if (!validV(r)) return FAILED;

  if (am.isLocalSVar(this)) {
    // onToplevel: distributed unification
    return bindPerdioVar(this,lPtr,r);
  } else {
    // in guard: bind and trail
    oz_checkSuspensionList(tagged2SVarPlus(*lPtr),pc_std_unif);
    am.doBindAndTrail(lPtr,r);
    return PROCEED;
  }
}

// ----------------------------------------------------------------------

// extern
TaggedRef newObjectProxy(Object *o, GName *gnobj,
			 GName *gnclass, TaggedRef clas)
{
  OldPerdioVar *pvar = new OldPerdioVar(o,oz_currentBoard());
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  addGName(gnobj, val);
  if (gnclass) {
    pvar->setGNameClass(gnclass);
  } else {
    pvar->setClass(tagged2ObjectClass(oz_deref(clas)));
  }
  return val;
}

void OldPerdioVar::proxyBindV(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be)
{
  PD((TABLE,"REDIRECT - borrow entry hit b:%d",getIndex()));
  Assert(isProxy());
  primBind(vPtr,val);
  be->mkRef();
  if (hasVal()) {
    PD((PD_VAR,"REDIRECT while pending"));
    redirect(val);
  }
  // pv->dispose();
  BT->maybeFreeBorrowEntry(getIndex());
}

void OldPerdioVar::managerBindV(TaggedRef *vPtr, TaggedRef val,
				OwnerEntry *oe, Site *rsite, int OTI)
{
  primBind(vPtr,val);
  oe->mkRef();
  if (oe->hasFullCredit()) {
    PD((WEIRD,"SURRENDER: full credit"));
  }
  sendRedirectToProxies(this,val,rsite,OTI);
}

void OldPerdioVar::proxyAckV(TaggedRef *vPtr, BorrowEntry *be) 
{
  acknowledge(vPtr);
  be->mkRef();
  BT->maybeFreeBorrowEntry(getIndex());
  // dispose();
}

void OldPerdioVar::marshalV(MsgBuffer *bs)
{
  if ((isProxy()) || isManager()) {
    marshalVar(this,bs);
  } else {
    marshalObjVar(this,bs);
  }
}

