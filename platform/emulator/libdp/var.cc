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
#include "var_obj.hh"
#include "dpMarshaler.hh"
#include "unify.hh"
#include "var_simple.hh"
#include "var_future.hh"

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
  PD((PD_VAR,"ProxyVar::doBind by thread: %x",oz_currentThread()));
  PD((PD_VAR,"bind proxy b:%d v:%s",getIndex(),toC(r)));
  Bool isLocal = oz_isLocalVar(this);
  if (isLocal) {
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
    if (origVar->getType()==OZ_VAR_FUTURE) {
      am.addSuspendVarList(lPtr);
      return SUSPEND;
    }
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
  if (origVar->getType()==OZ_VAR_FUTURE) {
    int ret = ((Future *)origVar)->kick(vPtr);
  }
}

/* --- Marshal --- */

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
    // globalize free variable
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
    mv->marshal(bs);
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

// extern
OZ_Term unmarshalVarImpl(MsgBuffer* bs)
{
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
OZ_Term sendGetStatus(BorrowEntry *be){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  OZ_Term var = OZ_newVariable();
  marshal_M_GETSTATUS(bs,na->index,var);
  SendTo(na->site,bs,M_GETSTATUS,na->site,na->index);
  return var;
}

OZ_Term ProxyVar::statusV()
{
  BorrowEntry *be=BT->getBorrow(getIndex());
  return sendGetStatus(be);
}

VarStatus ProxyVar::checkStatusV()
{
  return EVAR_STATUS_UNKNOWN;
}

/* --- IsVar test --- */

inline
void ManagerVar::localize(TaggedRef *vPtr)
{
  origVar->setSuspList(unlinkSuspList());
  *vPtr=makeTaggedCVar(origVar);
  origVar=0;
  disposeV();
}

OZ_Term ManagerVar::statusV()
{
  return origVar->getType()==OZ_VAR_FUTURE ? AtomFuture : AtomFree;
}

VarStatus ManagerVar::checkStatusV()
{
  return origVar->getType()==OZ_VAR_FUTURE ?  EVAR_STATUS_FUTURE : EVAR_STATUS_FREE;
}


void oz_dpvar_localize(TaggedRef *vPtr) {
  oz_getManagerVar(*vPtr)->localize(vPtr);
}
