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
 *    Michael Mehl (1997)
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
int compareNetAddress(PerdioVar *lVar,PerdioVar *rVar);
void sendHelpX(MessageType mt,BorrowEntry *be);
void marshalToOwner(int bi,MsgBuffer *bs);
void marshalBorrowHead(MarshalTag tag, int bi,MsgBuffer *bs);
void marshalOwnHead(int tag,int i,MsgBuffer *bs);
OZ_Term unmarshalBorrow(MsgBuffer *bs,OB_Entry *&ob,int &bi);
void SendTo(Site *toS,MsgBuffer *bs,MessageType mt,Site *sS,int sI);

// from components
OZ_Return raiseGeneric(char *msg, OZ_Term arg);

/* -------------------------------------------------------------------- */

void PerdioVar::addSusp(TaggedRef * v, Suspension susp, int unstable)
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

void gcBorrowNow(int i) { BT->getBorrow(i)->gcPO(); }

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

void PerdioVar::gcRecurse(void)
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

PerdioVar *var2PerdioVar(TaggedRef *tPtr)
{
  if (isPerdioVar(*tPtr) ) {
    return tagged2PerdioVar(*tPtr);
  }
  if (!oz_isFree(*tPtr)) return 0;

  // mm2: handle future
  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  PD((GLOBALIZING,"globalize var index:%d",i));

  oe->mkVar(makeTaggedRef(tPtr));

  PerdioVar *ret = new PerdioVar(oz_currentBoard());
  ret->setIndex(i);

  if (isCVar(*tPtr))
    ret->setSuspList(tagged2SVarPlus(*tPtr)->getSuspList());
  doBindCVar(tPtr,ret);
  return ret;
}

void marshalVar(PerdioVar *pvar,MsgBuffer *bs)
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

int compareNetAddress(PerdioVar *lVar,PerdioVar *rVar)
{
  GET_ADDR(lVar,lSD,lOTI);
  GET_ADDR(rVar,rSD,rOTI);
  int ret = lSD->compareSites(rSD);
  if (ret != 0) return ret;
  return lOTI<rOTI ? -1 : 1;
}

void sendRegister(BorrowEntry *be) {
  // EK Hack to avoid credit crashes 
  // In case of secondary credits in the
  // Message that contained the Var we must
  // nullify the globalvar creditSite
  Site * tmpS = creditSite;
  creditSite = NULL;


  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_REGISTER(bs,na->index,mySite);
  creditSite = tmpS;
  SendTo(na->site,bs,M_REGISTER,na->site,na->index);}

OZ_Term unmarshalVar(MsgBuffer* bs){
  OB_Entry *ob;
  int bi;
  OZ_Term val1 = unmarshalBorrow(bs,ob,bi);
  
  if (val1) {
    PD((UNMARSHAL,"var/chunk hit: b:%d",bi));
    return val1;}
  
  PD((UNMARSHAL,"var miss: b:%d",bi));
  PerdioVar *pvar = new PerdioVar(bi,oz_currentBoard());
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  ob->mkVar(val); 
  sendRegister((BorrowEntry *)ob);
  return val;
}

#define UNIFY_ERRORMSG \
   "Unification of distributed variable with term containing resources"

OZ_Return sendSurrender(BorrowEntry *be,OZ_Term val){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_SURRENDER(bs,na->index,mySite,val);
  CheckNogoods(val,bs,UNIFY_ERRORMSG,);
  SendTo(na->site,bs,M_SURRENDER,na->site,na->index);
  return PROCEED;
}

OZ_Return sendRedirect(Site* sd,int OTI,TaggedRef val){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_REDIRECT(bs,mySite,OTI,val);
  CheckNogoods(val,bs,UNIFY_ERRORMSG,);
  SendTo(sd,bs,M_REDIRECT,mySite,OTI);
  return PROCEED;
}

void sendAcknowledge(Site* sd,int OTI){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);  
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_ACKNOWLEDGE(bs,mySite,OTI);
  SendTo(sd,bs,M_ACKNOWLEDGE,mySite,OTI);
}

void PerdioVar::acknowledge(OZ_Term *p){
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
void PerdioVar::redirect(OZ_Term val) {
  PD((PD_VAR,"redirect v:%s",toC(val)));
  while (u.bindings) {
    PD((PD_VAR,"redirect pending unify =%s",toC(u.bindings->val)));
    PD((THREAD_D,"start thread redirect"));
    ControlVarUnify(u.bindings->controlvar,val,u.bindings->val);
    PendBinding *tmp=u.bindings->next;
    u.bindings->dispose();
    u.bindings=tmp;}
}

OZ_Return sendRedirect(PerdioVar *pv,OZ_Term val, Site* ackSite, int OTI)
{
  ProxyList *pl = pv->getProxies();
  OZ_Return ret = PROCEED;
  OwnerEntry *oe = OT->getOwner(OTI);
  if (pl==NULL && pv->isExported()) {
    MsgBuffer *bs=msgBufferManager->getMsgBuffer(NULL);
    marshal_M_REDIRECT(bs,mySite,OTI,val);
    CheckNogoods(val,bs,UNIFY_ERRORMSG,);
  }
  while (pl) {
    Site* sd = pl->sd;
    if (sd==ackSite) {
      sendAcknowledge(sd,OTI);
    } else {
      ret = sendRedirect(sd,OTI,val);
      if (ret != PROCEED)
	break;
    }
    ProxyList *tmp = pl->next;
    pl->dispose();
    pl = tmp;
  }
  return ret;
}

OZ_Return bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v)
{
  PD((PD_VAR,"bindPerdioVar by thread: %x",oz_currentThread()));
  if (pv->isManager()) {
    PD((PD_VAR,"bind manager o:%d v:%s",pv->getIndex(),toC(v)));
    OZ_Return aux = sendRedirect(pv,v,mySite,pv->getIndex());
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

void PerdioVar::primBind(TaggedRef *lPtr,TaggedRef v)
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

OZ_Return PerdioVar::unify(TaggedRef *lPtr, TaggedRef r, ByteCode *scp)
{
  if (oz_isRef(r)) {
    TaggedRef *rPtr = tagged2Ref(r);
    TaggedRef rVal = *rPtr;
    TaggedRef lVal = *lPtr;
    GenCVariable *cv = tagged2CVar(rVal);

    if (cv->getType()!=PerdioVariable) return FAILED;

    PerdioVar *lVar = this;

    PerdioVar *rVar = (PerdioVar *)cv;

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
  if (!valid(r)) return FAILED;

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
