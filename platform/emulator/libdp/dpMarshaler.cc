/* -*- C++ -*-
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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
#pragma implementation "dpMarshaler.hh"
#endif

#include "base.hh"
#include "dpBase.hh"
#include "perdio.hh"
#include "msgType.hh"
#include "table.hh"
#include "dpMarshaler.hh"
#include "dpInterface.hh"
#include "marshaler.hh"
#include "var.hh"
#include "gname.hh"
#include "state.hh"
#include "port.hh"
#include "protocolVar.hh"

/* for now credit is a 32-bit word */

void marshalCreditOutline(Credit credit,MsgBuffer *bs){
  marshalCredit(credit,bs);}

Credit unmarshalCreditOutline(MsgBuffer *bs){
  return unmarshalCredit(bs);}

/**********************************************************************/
/*  basic borrow, owner */
/**********************************************************************/

void marshalOwnHead(int tag,int i,MsgBuffer *bs){
  if (bs->isPersistentBuffer()) return;
  PD((MARSHAL_CT,"OwnHead"));
  bs->put(tag);
  myDSite->marshalDSite(bs);
  marshalNumber(i,bs);
  bs->put(DIF_PRIMARY);
  Credit c=ownerTable->getOwner(i)->getSendCredit();
  marshalNumber(c,bs);
  PD((MARSHAL,"ownHead o:%d rest-c: ",i));
  return;}

void marshalToOwner(int bi,MsgBuffer *bs){
  if (bs->isPersistentBuffer()) return;
  PD((MARSHAL,"toOwner"));
  BorrowEntry *b=BT->getBorrow(bi);
  int OTI=b->getOTI();
  if(b->getOnePrimaryCredit()){
    bs->put((BYTE) DIF_OWNER);
    marshalNumber(OTI,bs);
    PD((MARSHAL,"toOwner Borrow b:%d Owner o:%d",bi,OTI));
    return;}
  bs->put((BYTE) DIF_OWNER_SEC);
  DSite* xcs = b->getOneSecondaryCredit();
  marshalNumber(OTI,bs);
  xcs->marshalDSite(bs);
  return;}

void marshalBorrowHead(MarshalTag tag, int bi,MsgBuffer *bs){
  if (bs->isPersistentBuffer()) return;
  PD((MARSHAL,"BorrowHead"));
  bs->put((BYTE)tag);
  BorrowEntry *b=borrowTable->getBorrow(bi);
  NetAddress *na=b->getNetAddress();
  na->site->marshalDSite(bs);
  marshalNumber(na->index,bs);
  Credit cred=b->getSmallPrimaryCredit();
  if(cred) {
    PD((MARSHAL,"borrowed b:%d remCredit c: give c:%d",bi,cred));
    bs->put(DIF_PRIMARY);
    marshalCredit(cred,bs);
    return;}
  DSite* ss=b->getSmallSecondaryCredit(cred);
  bs->put(DIF_SECONDARY);
  marshalCredit(cred,bs);
  marshalDSite(ss,bs);
  return;}

OZ_Term unmarshalBorrow(MsgBuffer *bs,OB_Entry *&ob,int &bi){
  PD((UNMARSHAL,"Borrow"));
  DSite*  sd=unmarshalDSite(bs);
  int si=unmarshalNumber(bs);
  Credit cred;
  MarshalTag mt=(MarshalTag) bs->get();
  PD((UNMARSHAL,"borrow o:%d",si));
  if(sd==myDSite){
    Assert(0);
//     if(mt==DIF_PRIMARY){
//       cred = unmarshalCredit(bs);
//       PD((UNMARSHAL,"myDSite is owner"));
//       OwnerEntry* oe=OT->getOwner(si);
//       if(cred != PERSISTENT_CRED)
//      oe->returnCreditOwner(cred);
//       OZ_Term ret = oe->getValue();
//       return ret;}
//     Assert(mt==DIF_SECONDARY);
//     cred = unmarshalCredit(bs);
//     DSite* cs=unmarshalDSite(bs);
//     sendSecondaryCredit(cs,myDSite,si,cred);
//     PD((UNMARSHAL,"myDSite is owner"));
//     OwnerEntry* oe=OT->getOwner(si);
//     OZ_Term ret = oe->getValue();
//     return ret;
  }
  NetAddress na = NetAddress(sd,si);
  BorrowEntry *b = borrowTable->find(&na);
    if (b!=NULL) {
      PD((UNMARSHAL,"borrow found"));
      cred = unmarshalCredit(bs);
      if(mt==DIF_PRIMARY){
        if(cred!=PERSISTENT_CRED)
          b->addPrimaryCredit(cred);
        else Assert(b->isPersistent());}
      else{
        Assert(mt==DIF_SECONDARY);
        DSite* s=unmarshalDSite(bs);
        b->addSecondaryCredit(cred,s);}
      ob = b;
      return b->getValue();}
  cred = unmarshalCredit(bs);
  if(mt==DIF_PRIMARY){
    bi=borrowTable->newBorrow(cred,sd,si);
    b=borrowTable->getBorrow(bi);
    if(cred == PERSISTENT_CRED )
      b->makePersistent();
    PD((UNMARSHAL,"borrowed miss"));
    ob=b;
    return 0;}
  Assert(mt==DIF_SECONDARY);
  DSite* site = unmarshalDSite(bs);
  bi=borrowTable->newSecBorrow(site,cred,sd,si);
  b=borrowTable->getBorrow(bi);
  PD((UNMARSHAL,"borrowed miss"));
  // PER-LOOK indeed!
  b->moreCredit(); // The Borrow needs some of the real McCoys
  ob=b;
  return 0;
}

/**********************************************************************/
/*  header */
/**********************************************************************/

MessageType unmarshalHeader(MsgBuffer *bs){
  bs->unmarshalBegin();
  refTable->reset();
  MessageType mt= (MessageType) bs->get();
  mess_counter[mt].recv();
  return mt;}

/* *********************************************************************/
/*   perdiovar - special                                  */
/* *********************************************************************/

//
// These two function are used by 'OldPerdioVar::marshalV()';
void marshalVar(OldPerdioVar *pvar, MsgBuffer *bs)
{
  DSite *sd=bs->getSite();
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

/* *********************************************************************/
/*   objects                                  */
/* *********************************************************************/

void marshalFullObjectRT(Object *o,MsgBuffer* bs){
  Assert(refTrail->isEmpty());
  marshalFullObject(o,bs);
  refTrail->unwind();}

void marshalFullObjectAndClassRT(Object *o,MsgBuffer* bs){
  Assert(refTrail->isEmpty());
  marshalFullObjectAndClass(o, bs);
  refTrail->unwind();}

void marshalObject(Object *o, MsgBuffer *bs, GName *gnclass)
{
  if (marshalTertiary(o,DIF_OBJECT,bs)) return;   /* ATTENTION */
  Assert(o->hasGName());
  marshalGName(o->hasGName(),bs);
  marshalGName(gnclass,bs);
  trailCycleOutLine(o->getCycleRef(),bs);
}

void unmarshalObject(ObjectFields *o, MsgBuffer *bs){
  o->feat = unmarshalSRecord(bs);
  o->state=unmarshalTerm(bs);
  o->lock=unmarshalTerm(bs);}

void fillInObject(ObjectFields *of, Object *o){
  o->setFreeRecord(of->feat);
  o->setState(tagged2Tert(of->state));
  o->setLock(oz_isNil(of->lock) ? (LockProxy*)NULL : (LockProxy*)tagged2Tert(of->lock));}

void unmarshalUnsentObject(MsgBuffer *bs){
  unmarshalUnsentSRecord(bs);
  unmarshalUnsentTerm(bs);
  unmarshalUnsentTerm(bs);}

void unmarshalObjectAndClass(ObjectFields *o, MsgBuffer *bs){
  unmarshalObject(o,bs);
  o->clas = unmarshalTerm(bs);}

void unmarshalUnsentObjectAndClass(MsgBuffer *bs){
  unmarshalUnsentObject(bs);
  unmarshalUnsentTerm(bs);}

void fillInObjectAndClass(ObjectFields *of, Object *o){
  fillInObject(of,o);
  o->setClass(tagged2ObjectClass(of->clas));}

void unmarshalObjectRT(ObjectFields *o, MsgBuffer *bs){
  refTable->reset();
  Assert(refTrail->isEmpty());
  unmarshalObject(o,bs);
  refTrail->unwind();}

void unmarshalObjectAndClassRT(ObjectFields *o, MsgBuffer *bs){
  refTable->reset();
  Assert(refTrail->isEmpty());
  unmarshalObjectAndClass(o, bs);
  refTrail->unwind();}

void marshalFullObject(Object *o,MsgBuffer* bs){
  marshalSRecord(o->getFreeRecord(),bs);
  marshalTerm(makeTaggedConst(getCell(o->getState())),bs);
  if (o->getLock()) {marshalTerm(makeTaggedConst(o->getLock()),bs);}
  else {marshalTerm(oz_nil(),bs);}}

void marshalFullObjectAndClass(Object *o,MsgBuffer* bs){
  ObjectClass *oc=o->getClass();
  marshalFullObject(o,bs);
  marshalClass(oc,bs);}

/* *********************************************************************/
/*   interface to Oz-core                                  */
/* *********************************************************************/

void marshalObject(ConstTerm* t, MsgBuffer *bs)
{
  PD((MARSHAL,"object"));
  Object *o = (Object*) t;
  Assert(o->getType() == Co_Object);

  ObjectClass *oc = o->getClass();
  oc->globalize();
  o->globalize();
  bs->addRes(makeTaggedConst(t));
  marshalObject(o,bs,oc->getGName());}

Bool marshalTertiary(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
{
  PD((MARSHAL,"Tert"));
  DSite* sd=bs->getSite();
  switch(t->getTertType()){
  case Te_Local:
    globalizeTert(t);
    // no break here!
  case Te_Manager:
    {
      PD((MARSHAL_CT,"manager"));
      int OTI=t->getIndex();
      marshalOwnHead(tag,OTI,bs);
      break;
    }
  case Te_Frame:
  case Te_Proxy:
    {
      PD((MARSHAL,"proxy"));
      int BTI=t->getIndex();
      if (bs->getSite() && borrowTable->getOriginSite(BTI)==sd) {
        marshalToOwner(BTI,bs);
        return OK;}
      marshalBorrowHead(tag,BTI,bs);
      break;
    }
  default:
    Assert(0);
  }
  return NO;
}

static char *tagToComment(MarshalTag tag)
{
  switch(tag){
  case DIF_PORT:
    return "port";
  case DIF_THREAD_UNUSED:
    return "thread";
  case DIF_SPACE:
    return "space";
  case DIF_CELL:
    return "cell";
  case DIF_LOCK:
    return "lock";
  case DIF_OBJECT:
    return "object";
  default:
    Assert(0);
    return "";
}}

OZ_Term unmarshalTertiary(MsgBuffer *bs, MarshalTag tag)
{
  OB_Entry* ob;
  int bi;
  OZ_Term val = unmarshalBorrow(bs,ob,bi);
  if(val){
    PD((UNMARSHAL,"%s hit b:%d",tagToComment(tag),bi));
    switch (tag) {
    case DIF_PORT:
    case DIF_THREAD_UNUSED:
    case DIF_SPACE:
      break;
    case DIF_CELL:{
      Tertiary *t=ob->getTertiary(); // mm2: bug: ob is 0 if I am the owner
      if((t->getType()==Co_Cell) && (t->isFrame())){
        ((CellFrame *)t)->resetDumpBit();}
      break;}
    case DIF_LOCK:{
      Tertiary *t=ob->getTertiary();
      if((t->getType()==Co_Lock) && (t->isFrame())){
        ((LockFrame *)t)->resetDumpBit();}
      break;}
    case DIF_OBJECT:
      TaggedRef obj;
      (void) unmarshalGName(&obj,bs);
      TaggedRef clas;
      (void) unmarshalGName(&clas,bs);
      break;
    default:
      Assert(0);
    }
    return val;
  }

  PD((UNMARSHAL,"%s miss b:%d",tagToComment(tag),bi));
  Tertiary *tert;

  switch (tag) {
  case DIF_PORT:
    tert = new PortProxy(bi);
    break;
  case DIF_THREAD_UNUSED:
    // tert = new Thread(bi,Te_Proxy);
    break;
  case DIF_SPACE:
    tert = new Space(bi,Te_Proxy);
    break;
  case DIF_CELL:
    tert = new CellProxy(bi);
    break;
  case DIF_LOCK:
    tert = new LockProxy(bi);
    break;
  case DIF_OBJECT:
    {

      TaggedRef obj;
      GName *gnobj = unmarshalGName(&obj,bs);
      TaggedRef clas;
      GName *gnclass = unmarshalGName(&clas,bs);
      if(!gnobj){
        BT->maybeFreeBorrowEntry(bi);
        return obj;}

      Object *o = new Object(bi);
      o->setGName(gnobj);

      // mm2: abstraction val=newObjectProxy(o,gnobj,gnclass,clas)
      val = newObjectProxy(o,gnobj,gnclass,clas);

      ob->mkVar(val, ob->getFlags());
      return val;}
  default:
    Assert(0);
  }
  val=makeTaggedConst(tert);
  ob->mkTertiary(tert,ob->getFlags());
  return val;
}

OZ_Term unmarshalOwner(MsgBuffer *bs,MarshalTag mt){
  if(mt==DIF_OWNER){
    int OTI=unmarshalNumber(bs);
    PD((UNMARSHAL,"OWNER o:%d",OTI));
    OwnerEntry* oe=OT->getOwner(OTI);
    oe->returnCreditOwner(1);
    OZ_Term oz=oe->getValue();
    return oz;}
  Assert(mt==DIF_OWNER_SEC);
  int OTI=unmarshalNumber(bs);
  DSite* cs=unmarshalDSite(bs);
  sendSecondaryCredit(cs,myDSite,OTI,1);
  return OT->getOwner(OTI)->getValue();
}


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

// Returning 'NO' means we are going to proceed with 'marshal bomb';
Bool marshalVariable(TaggedRef *tPtr, MsgBuffer *bs) {
  PerdioVar *pvar = var2PerdioVar(tPtr);
  if (pvar==NULL) {
    return (NO);
  }
  bs->addRes(makeTaggedRef(tPtr));
  pvar->marshalV(bs);
  return (OK);
}

void unmarshalUnsentTerm(MsgBuffer *bs) {
  OZ_Term t=unmarshalTerm(bs);}

void unmarshalUnsentSRecord(MsgBuffer *bs){
  unmarshalUnsentTerm(bs);}

int unmarshalUnsentNumber(MsgBuffer *bs)
{
  return unmarshalNumber(bs);
}

void unmarshalUnsentString(MsgBuffer *bs)
{
  char *aux = unmarshalString(bs);
  delete [] aux;
}

void marshalObjVar(OldPerdioVar *pvar, MsgBuffer *bs)
{
  Assert(pvar->isObject());
  PD((MARSHAL,"var objectproxy"));

  if (checkCycleOutLine(*(pvar->getObject()->getCycleRef()),bs,OZCONST))
    return;

  GName *classgn =  pvar->isObjectClassAvail() ?
                      pvar->getClass()->getGName() : pvar->getGNameClass();

  marshalObject(pvar->getObject(),bs,classgn);
}

/* *********************************************************************/
/*   SECTION 15: statistics                                            */
/* *********************************************************************/

#ifdef MISC_BUILTINS

OZ_BI_define(BIperdioStatistics,0,1)
{
  OZ_Term dif_send_ar=oz_nil();
  OZ_Term dif_recv_ar=oz_nil();
  int i;
  for (i=0; i<DIF_LAST; i++) {
    dif_send_ar=oz_cons(oz_pairAI(dif_names[i].name,dif_counter[i].getSend()),
                        dif_send_ar);
    dif_recv_ar=oz_cons(oz_pairAI(dif_names[i].name,dif_counter[i].getRecv()),
                        dif_recv_ar);
  }
  OZ_Term dif_send=OZ_recordInit(oz_atom("dif"),dif_send_ar);
  OZ_Term dif_recv=OZ_recordInit(oz_atom("dif"),dif_recv_ar);

  OZ_Term misc_send_ar=oz_nil();
  OZ_Term misc_recv_ar=oz_nil();
  for (i=0; i<MISC_LAST; i++) {
    misc_send_ar=oz_cons(oz_pairAI(misc_names[i],misc_counter[i].getSend()),
                         misc_send_ar);
    misc_recv_ar=oz_cons(oz_pairAI(misc_names[i],misc_counter[i].getRecv()),
                         misc_recv_ar);
  }
  OZ_Term misc_send=OZ_recordInit(oz_atom("misc"),misc_send_ar);
  OZ_Term misc_recv=OZ_recordInit(oz_atom("misc"),misc_recv_ar);

  OZ_Term mess_send_ar=oz_nil();
  OZ_Term mess_recv_ar=oz_nil();
  for (i=0; i<M_LAST; i++) {
    mess_send_ar=oz_cons(oz_pairAI(mess_names[i],mess_counter[i].getSend()),
                         mess_send_ar);
    mess_recv_ar=oz_cons(oz_pairAI(mess_names[i],mess_counter[i].getRecv()),
                         mess_recv_ar);
  }
  OZ_Term mess_send=OZ_recordInit(oz_atom("messages"),mess_send_ar);
  OZ_Term mess_recv=OZ_recordInit(oz_atom("messages"),mess_recv_ar);


  OZ_Term send_ar=oz_nil();
  send_ar = oz_cons(oz_pairA("dif",dif_send),send_ar);
  send_ar = oz_cons(oz_pairA("misc",misc_send),send_ar);
  send_ar = oz_cons(oz_pairA("messages",mess_send),send_ar);
  OZ_Term send=OZ_recordInit(oz_atom("send"),send_ar);

  OZ_Term recv_ar=oz_nil();
  recv_ar = oz_cons(oz_pairA("dif",dif_recv),recv_ar);
  recv_ar = oz_cons(oz_pairA("misc",misc_recv),recv_ar);
  recv_ar = oz_cons(oz_pairA("messages",mess_recv),recv_ar);
  OZ_Term recv=OZ_recordInit(oz_atom("recv"),recv_ar);


  OZ_Term ar=oz_nil();
  ar=oz_cons(oz_pairA("send",send),ar);
  ar=oz_cons(oz_pairA("recv",recv),ar);
  OZ_RETURN(OZ_recordInit(oz_atom("perdioStatistics"),ar));
} OZ_BI_end

#endif
