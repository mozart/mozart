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
#include "unify.hh"

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
OZ_Return PerdioVar::unifyV(TaggedRef *lPtr, TaggedRef *rPtr)
{
  TaggedRef rVal = *rPtr;

  if (!oz_isExtVar(rVal)) {
    // switch binding order
    if (isSimpleVar(rVal) || isFuture(rVal))  {
      return oz_var_bind(tagged2CVar(rVal),rPtr,makeTaggedRef(lPtr));
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
    return rVar->bindV(rPtr,makeTaggedRef(lPtr));
  }

  Assert(getIdV()==OZ_EVAR_PROXY || getIdV()==OZ_EVAR_MANAGER);

  // Note: for perdio variables: isLocal == onToplevel
  // mm2: moreLocal bug!
  if (oz_isLocalVar(this)) {
    int rTag=rVar->getIdV();
    if (rTag==OZ_EVAR_PROXY || rTag==OZ_EVAR_MANAGER) {
      int ret = compareNetAddress((ProxyManagerVar*)this,
                                  (ProxyManagerVar*)rVar);
      if (ret>=0) {
        return rVar->bindV(rPtr,makeTaggedRef(lPtr));
      }
    }
  }
  return bindV(lPtr,makeTaggedRef(rPtr));
}


OZ_Return ObjectVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  return FAILED;
}


/* --- ProxyVar --- */

void ProxyVar::gcRecurseV(void)
{
  PD((GC,"PerdioVar b:%d",getIndex()));
  BT->getBorrow(getIndex())->gcPO();

  PendBinding **last=&bindings;
  PendBinding *bl = *last;
  PendBinding *newBL;
  for (; bl; bl = bl->next) {
    newBL = new PendBinding();
    OZ_collectHeapTerm(bl->val,newBL->val);
    OZ_collectHeapTerm(bl->controlvar,newBL->controlvar);
    *last = newBL;
    last = &newBL->next;}
  *last=NULL;
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

OZ_Return ProxyVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  PD((PD_VAR,"PerdioVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind proxy b:%d v:%s",getIndex(),toC(r)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    if (hasVal()) {
      // mm2: should return suspend: binding list can be removed
      return pushVal(r); // save binding for ack message, ...
    }

    BorrowEntry *be=BT->getBorrow(getIndex());
    OZ_Return aux = sendSurrender(be,r);
    if (aux!=PROCEED) return aux;
    return setVal(r); // save binding for ack message, ...
  } else {
    // in guard: bind and trail
    oz_checkSuspensionList(tagged2SVarPlus(*lPtr),pc_std_unif);
    doBindAndTrail(lPtr,r);
    return PROCEED;
  }
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
  be->changeToRef();
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
  be->changeToRef();
  BT->maybeFreeBorrowEntry(getIndex());
  // dispose();
}

/* --- ManagerVar --- */

void ManagerVar::gcRecurseV(void)
{
#ifdef ORIG
  origVar=origVar->gcVar();
  origVar->gcVarRecurse();
#endif
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

static
void sendAcknowledge(DSite* sd,int OTI){
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
  ProxyList *pl = getProxies();
  while (pl) { // perdio vars are not yet localized again (mm2: ????)
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
  }
  return PROCEED;
}

OZ_Return ManagerVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  PD((PD_VAR,"PerdioVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind manager o:%d v:%s",getIndex(),toC(v)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
    OZ_Return aux = sendRedirectToProxies(r, myDSite);
    if (aux != PROCEED) return aux;
    OT->getOwner(getIndex())->changeToRef();
    primBind(lPtr,r);
    return PROCEED;
  } else {
    // in guard: bind and trail
    oz_checkSuspensionList(tagged2SVarPlus(*lPtr),pc_std_unif);
    doBindAndTrail(lPtr,r);
    return PROCEED;
  }
}

// after a surrender message is received
void ManagerVar::managerBind(TaggedRef *vPtr, TaggedRef val,
                             OwnerEntry *oe, DSite *rsite)
{
#ifdef ORIG
  OZ_Return ret = oz_var_unify(origVar,vPtr,val);
  if (ret != PROCEED) {
    // ignore: SUSPENDs of futures
    am.cleanupSuspAndControl();
    return;
  }
#endif
  primBind(vPtr,val);
  oe->changeToRef();
  if (oe->hasFullCredit()) {
    PD((WEIRD,"SURRENDER: full credit"));
  }
  OZ_Return ret = sendRedirectToProxies(val,rsite);
  Assert(ret==PROCEED); // can not fail because val is imported
}

/* --- Marshal --- */

void ObjectVar::marshal(MsgBuffer *bs)
{
  PD((MARSHAL,"var objectproxy"));
  int done=checkCycleOutLine(*(getObject()->getCycleRef()),bs,OZCONST);
  if (!done) {
    GName *classgn =  isObjectClassAvail()
      ? globalizeConst(getClass(),bs) : getGNameClass();
    marshalObject(getObject(),bs,classgn);
  }
}

void ManagerVar::marshal(MsgBuffer *bs)
{
  int i=getIndex();
  PD((MARSHAL,"var manager o:%d",i));
  marshalOwnHead(DIF_VAR,i,bs);
}

void ProxyVar::marshal(MsgBuffer *bs)
{
  DSite *sd=bs->getSite();
  int i=getIndex();
  PD((MARSHAL,"var proxy o:%d",i));
  if(sd && borrowTable->getOriginSite(i)==sd) {
    marshalToOwner(i,bs);
  } else {
    marshalBorrowHead(DIF_VAR,i,bs);
  }
}

// Returning 'NO' means we are going to proceed with 'marshal bomb';
Bool marshalVariable(TaggedRef *tPtr, MsgBuffer *bs) {
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
  } else if (oz_isFree(var)) {
    if (!bs->globalize()) return TRUE;
    // globalize free variable
    OwnerEntry *oe;
    int i = ownerTable->newOwner(oe);
    PD((GLOBALIZING,"globalize var index:%d",i));
    oe->mkVar(makeTaggedRef(tPtr));
    ManagerVar *mv = new ManagerVar(oz_currentBoard(),i);
    if (isCVar(var))
      mv->setSuspList(tagged2SVarPlus(var)->getSuspList());
    doBindCVar(tPtr,mv);
    mv->marshal(bs);
  } else {
    // mm2: handle futures
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
  ObjectVar *pvar;
  if (gnclass) {
    pvar = new ObjectVar(oz_currentBoard(),o,gnclass);
  } else {
    pvar = new ObjectVar(oz_currentBoard(),o,
                         tagged2ObjectClass(oz_deref(clas)));
  }
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  addGName(gnobj, val);
  return val;
}


static
void sendRequest(MessageType mt,BorrowEntry *be)
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
  Bool send=FALSE;
  if(getSuspListLengthS()==0) send=TRUE;
  addSuspSVar(susp, unstable);
  if(! send) return;
  if (isObjectClassNotAvail()) {
    MessageType mt;
    if(oz_findGName(getGNameClass())==0) {mt=M_GET_OBJECTANDCLASS;}
    else {mt=M_GET_OBJECT;}
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());
    sendRequest(mt,be);
  } else {
    Assert(isObjectClassAvail());
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());
    sendRequest(M_GET_OBJECT,be);
  }
}

void ObjectVar::gcRecurseV(void)
{
  BT->getBorrow(getObject()->getIndex())->gcPO();
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

void ObjectVar::sendObject(DSite* sd, int si, ObjectFields& of,
                                BorrowEntry *be)
{
  Object *o = getObject();
  Assert(o);
  GName *gnobj = o->getGName1();
  Assert(gnobj);
  gnobj->setValue(makeTaggedConst(o));

  fillInObject(&of,o);
  ObjectClass *cl;
  if (isObjectClassAvail()) {
    cl=getClass();
  } else {
    cl=tagged2ObjectClass(oz_deref(oz_findGName(getGNameClass())));
  }
  o->setClass(cl);
  primBind(be->getPtr(),makeTaggedConst(o));
  be->changeToRef();
  BT->maybeFreeBorrowEntry(o->getIndex());
  o->localize();
}

void ObjectVar::sendObjectAndClass(ObjectFields& of, BorrowEntry *be)
{
  Object *o = getObject();
  Assert(o);
  GName *gnobj = o->getGName1();
  Assert(gnobj);
  gnobj->setValue(makeTaggedConst(o));

  fillInObjectAndClass(&of,o);
  primBind(be->getPtr(),makeTaggedConst(o));
  be->changeToRef();
  BT->maybeFreeBorrowEntry(o->getIndex());
  o->localize();
}
