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
#pragma implementation "dpMarshaler.hh"
#endif

#include "base.hh"
#include "dpBase.hh"
#include "perdio.hh"
#include "msgType.hh"
#include "table.hh"
#include "dpMarshaler.hh"
#include "dpInterface.hh"
#include "newmarshaler.hh"
#include "var.hh"
#include "gname.hh"
#include "state.hh"
#include "port.hh"
#include "dpResource.hh"
/* for now credit is a 32-bit word */

// from var_obj
TaggedRef newObjectProxy(Object*, GName*, GName*, TaggedRef);

void marshalCreditOutline(Credit credit,MsgBuffer *bs){
  marshalCredit(credit,bs);}

#ifdef USE_FAST_UNMARSHALER
Credit unmarshalCreditOutline(MsgBuffer *bs){
  return unmarshalCredit(bs);}
#else
Credit unmarshalCreditRobustOutline(MsgBuffer *bs, int *error){
  return unmarshalCreditRobust(bs,error);}
#endif

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

#ifndef USE_FAST_UNMARSHALER
OZ_Term unmarshalBorrowRobust(MsgBuffer *bs,OB_Entry *&ob,int &bi,int *error){
#else
OZ_Term unmarshalBorrow(MsgBuffer *bs,OB_Entry *&ob,int &bi){
#endif
  PD((UNMARSHAL,"Borrow"));
#ifndef USE_FAST_UNMARSHALER
  DSite*  sd=unmarshalDSiteRobust(bs,error);
  if(*error)  {Assert(0); return 0;}
  int si=unmarshalNumberRobust(bs,error);
  if(*error) {Assert(0); return 0;}
#else
  DSite*  sd=unmarshalDSite(bs);
  int si=unmarshalNumber(bs);
#endif
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
#ifndef USE_FAST_UNMARSHALER
    cred = unmarshalCreditRobust(bs,error);
    if(*error){ Assert(0); return 0;}
#else
    cred = unmarshalCredit(bs);
#endif
    if(mt==DIF_PRIMARY){
      if(cred!=PERSISTENT_CRED)
        b->addPrimaryCredit(cred);
      else Assert(b->isPersistent());}
    else{
      Assert(mt==DIF_SECONDARY);
#ifndef USE_FAST_UNMARSHALER
      DSite* s=unmarshalDSiteRobust(bs,error);
      if(*error){ Assert(0);return 0;}
#else
      DSite* s=unmarshalDSite(bs);
#endif
      b->addSecondaryCredit(cred,s);}
    ob = b;
    // Assert(b->getValue() != (OZ_Term) 0);
    return b->getValue();}
#ifndef USE_FAST_UNMARSHALER
  cred = unmarshalCreditRobust(bs,error);
  if(*error) {Assert(0);return 0;}
#else
  cred = unmarshalCredit(bs);
#endif
  if(mt==DIF_PRIMARY){
    bi=borrowTable->newBorrow(cred,sd,si);
    b=borrowTable->getBorrow(bi);
    if(cred == PERSISTENT_CRED )
      b->makePersistent();
    PD((UNMARSHAL,"borrowed miss"));
    ob=b;
    return 0;}
  Assert(mt==DIF_SECONDARY);
#ifndef USE_FAST_UNMARSHALER
  DSite* site = unmarshalDSiteRobust(bs,error);
  if(*error) {Assert(0);return 0;}
#else
  DSite* site = unmarshalDSite(bs);
#endif
  bi=borrowTable->newSecBorrow(site,cred,sd,si);
  b=borrowTable->getBorrow(bi);
  PD((UNMARSHAL,"borrowed miss"));
  b->moreCredit(); // The Borrow needs some of the real McCoys
  ob=b;
  return 0;
}

/**********************************************************************/
/*  header */
/**********************************************************************/

MessageType unmarshalHeader(MsgBuffer *bs){
  MessageType mt= (MessageType) bs->get();
  mess_counter[mt].recv();
  return mt;}

/* *********************************************************************/
/*   objects                                  */
/* *********************************************************************/

static void marshalFullObjectInternal(Object *o);
static void marshalFullObjectAndClassInternal(Object *o);

void marshalFullObject(Object *o, MsgBuffer* bs)
{
  newMarshalerStartBatch(bs);
  marshalFullObjectInternal(o);
  newMarshalerFinishBatch();
}

void marshalFullObjectAndClass(Object *o, MsgBuffer* bs)
{
  newMarshalerStartBatch(bs);
  marshalFullObjectAndClassInternal(o);
  newMarshalerFinishBatch();
}

//
// kost@ : both 'DIF_VAR_OBJECT' and 'DIF_OBJECT' (currently) should
// contain the same representation, since both are unmarshaled using
// 'unmarshalTertiary';
void marshalVarObject(Object *o, MsgBuffer *bs, GName *gnclass)
{
  Assert(o->getTertType() == Te_Proxy);

  //
  int BTI = o->getIndex();
  DSite* sd = bs->getSite();
  if (sd && borrowTable->getOriginSite(BTI) == sd) {
    marshalToOwner(BTI, bs);
  } else {
    marshalBorrowHead(DIF_VAR_OBJECT, BTI, bs);

    //
    Assert(o->getGName1());
    marshalGName(globalizeConst(o,bs),bs);
    marshalGName(gnclass,bs);
  }
}

static
void marshalObjectInternal(Object *o, MsgBuffer *bs, GName *gnclass)
{
  Assert(o->getTertType() == Te_Local || o->getTertType() == Te_Manager);
  if (o->getTertType() == Te_Local)
    globalizeTert(o);
  Assert(o->getTertType() == Te_Manager);
  marshalOwnHead(DIF_OBJECT, o->getIndex(), bs);

  //
  Assert(o->getGName1());
  marshalGName(globalizeConst(o,bs),bs);
  marshalGName(gnclass,bs);
}

#ifdef USE_FAST_UNMARSHALER
void unmarshalFullObject(ObjectFields *o, MsgBuffer *bs)
{
  TaggedRef t = newUnmarshalTerm(bs);
  o->feat  =  oz_isNil(t) ? (SRecord*)NULL : tagged2SRecord(t);
  o->state = newUnmarshalTerm(bs);
  o->lock  = newUnmarshalTerm(bs);
}
#else
void unmarshalFullObjectRobust(ObjectFields *o, MsgBuffer *bs, int *error)
{
  newUnmarshalerStartBatch();
  unmarshalFullObjectRobustInternal(o, bs, error);
}
void unmarshalFullObjectRobustInternal(ObjectFields *o, MsgBuffer *bs,
                                       int *error)
{
  TaggedRef t = newUnmarshalTermInternal(bs);
  if(t == 0) { *error = OK; return; }
  o->feat  =  oz_isNil(t) ? (SRecord*)NULL : tagged2SRecord(t);
  o->state = newUnmarshalTermInternal(bs);
  if(o->state == 0) { *error = false; return; }
  o->lock  = newUnmarshalTermInternal(bs);
  *error = (o->lock == 0);
}
#endif

void fillInObject(ObjectFields *of, Object *o){
  o->setFreeRecord(of->feat);
  o->setState((Tertiary *) tagged2Const(of->state));
  o->setLock(oz_isNil(of->lock) ?
             (LockProxy*) NULL :
             (LockProxy*) tagged2Const(of->lock));}

#ifdef USE_FAST_UNMARSHALER
void unmarshalFullObjectAndClass(ObjectFields *o, MsgBuffer *bs)
{
  unmarshalFullObject(o,bs);
  o->clas = newUnmarshalTerm(bs);
}
#else
void unmarshalFullObjectAndClassRobust(ObjectFields *o,MsgBuffer *bs,
                                       int *error)
{
  newUnmarshalerStartBatch();
  unmarshalFullObjectAndClassRobustInternal(o,bs,error);
}
void unmarshalFullObjectAndClassRobustInternal(ObjectFields *o,MsgBuffer *bs,
                                               int *error)
{
  int e;
  unmarshalFullObjectRobustInternal(o,bs,&e);
  o->clas = newUnmarshalTermInternal(bs);
  *error = (e || (o->clas == 0));
}
#endif

void fillInObjectAndClass(ObjectFields *of, Object *o){
  fillInObject(of,o);
  o->setClass(tagged2ObjectClass(of->clas));}

static void marshalFullObjectInternal(Object *o)
{
  //
  SRecord *sr = o->getFreeRecord();
  OZ_Term tsr;
  if (sr)
    tsr = makeTaggedSRecord(sr);
  else
    tsr = oz_nil();
  newMarshalTermInBatch(tsr);

  //
  newMarshalTermInBatch(makeTaggedConst(getCell(o->getState())));

  //
  if (o->getLock())
    newMarshalTermInBatch(makeTaggedConst(o->getLock()));
  else
    newMarshalTermInBatch(oz_nil());
}

static void marshalFullObjectAndClassInternal(Object *o)
{
  marshalFullObjectInternal(o);
  newMarshalTermInBatch(makeTaggedConst(o->getClass()));
}

/* *********************************************************************/
/*   interface to Oz-core                                  */
/* *********************************************************************/

void marshalObjectImpl(ConstTerm* t, MsgBuffer *bs)
{
  PD((MARSHAL,"object"));
  Object *o = (Object*) t;
  Assert(isObject(o));

  ObjectClass *oc = o->getClass();
  globalizeConst(o,bs);
  marshalObjectInternal(o, bs, globalizeConst(oc, bs));
}

void marshalTertiaryImpl(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
{
  PD((MARSHAL,"Tert"));
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
      DSite* sd=bs->getSite();
      if (sd && borrowTable->getOriginSite(BTI)==sd)
        marshalToOwner(BTI, bs);
      else
        marshalBorrowHead(tag, BTI, bs);
      break;
    }
  default:
    Assert(0);
  }
}

void marshalSPPImpl(TaggedRef entity, MsgBuffer *bs, Bool trail)
{
  int index = RHT->find(entity);
  if(index == RESOURCE_NOT_IN_TABLE){
    OwnerEntry *oe_manager;
    index=ownerTable->newOwner(oe_manager);
    PD((GLOBALIZING,"GLOBALIZING Resource index:%d",index));
    oe_manager->mkRef(entity);
    RHT->add(entity, index);
  }
  if(trail)  marshalOwnHead(DIF_RESOURCE_T,index,bs);
  else  marshalOwnHead(DIF_RESOURCE_N,index,bs);
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
  case DIF_VAR_OBJECT:
    return "object";
  case DIF_RESOURCE_T:
  case DIF_RESOURCE_N:
    return "resource";
  default:
    Assert(0);
    return "";
}}

#ifndef USE_FAST_UNMARSHALER
OZ_Term unmarshalTertiaryRobustImpl(MsgBuffer *bs, MarshalTag tag, int *error)
#else
OZ_Term unmarshalTertiaryImpl(MsgBuffer *bs, MarshalTag tag)
#endif
{
  OB_Entry* ob;
  int bi;
#ifndef USE_FAST_UNMARSHALER
  int e1,e2,e3;
  OZ_Term val = unmarshalBorrowRobust(bs,ob,bi,&e1);
#else
  OZ_Term val = unmarshalBorrow(bs,ob,bi);
#endif
  if(val){
    PD((UNMARSHAL,"%s hit b:%d",tagToComment(tag),bi));
    switch (tag) {
    case DIF_RESOURCE_T:
    case DIF_RESOURCE_N:
    case DIF_PORT:
    case DIF_THREAD_UNUSED:
    case DIF_SPACE:
      break;
    case DIF_CELL:{
      Tertiary *t=ob->getTertiary(); // mm2: bug: ob is 0 if I am the owner
      break;}
    case DIF_LOCK:{
      Tertiary *t=ob->getTertiary();
      break;}
    case DIF_OBJECT:
    case DIF_VAR_OBJECT:
      TaggedRef obj;
      TaggedRef clas;
#ifndef USE_FAST_UNMARSHALER
      (void) unmarshalGNameRobust(&obj,bs,&e2);
      (void) unmarshalGNameRobust(&clas,bs,&e3);
#else
      (void) unmarshalGName(&obj,bs);
      (void) unmarshalGName(&clas,bs);
#endif
      break;
    default:
      Assert(0);
    }
#ifndef USE_FAST_UNMARSHALER
    *error = e1;
#endif
    return val;
  }

  PD((UNMARSHAL,"%s miss b:%d",tagToComment(tag),bi));
  Tertiary *tert;

  switch (tag) {
  case DIF_RESOURCE_N:
  case DIF_RESOURCE_T:
    tert = new DistResource(bi);
    break;
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
  case DIF_VAR_OBJECT:
    {
      TaggedRef obj;
      TaggedRef clas;
#ifndef USE_FAST_UNMARSHALER
      GName *gnobj = unmarshalGNameRobust(&obj,bs,&e2);
      GName *gnclass = unmarshalGNameRobust(&clas,bs,&e3);
      *error = e1 || e2 || e3;
#else
      GName *gnobj = unmarshalGName(&obj,bs);
      GName *gnclass = unmarshalGName(&clas,bs);
#endif
      if(!gnobj){
        if(!(BT->maybeFreeBorrowEntry(bi))){
          ob->mkRef(obj,ob->getFlags());
          //printf("indx:%d %xd\n",((BorrowEntry *)ob)->getNetAddress()->index,
          //     ((BorrowEntry *)ob)->getNetAddress()->site);
        }
        return obj;
      }

      Object *o = new Object(bi);
      o->setGName(gnobj);

      // mm2: abstraction val=newObjectProxy(o,gnobj,gnclass,clas)
      val = newObjectProxy(o,gnobj,gnclass,clas);

      ob->changeToVar(val);
      return val;}
  default:
    Assert(0);
  }
  val=makeTaggedConst(tert);
  ob->changeToTertiary(tert);
#ifndef USE_FAST_UNMARSHALER
  *error = e1;
#endif
  switch(((BorrowEntry*)ob)->getSite()->siteStatus()){
  case SITE_OK:{
    break;}
  case SITE_PERM:{
    deferProxyTertProbeFault(tert,PROBE_PERM);
    break;}
  case SITE_TEMP:{
    deferProxyTertProbeFault(tert,PROBE_TEMP);
    break;}
  default:
    Assert(0);
  }
  return val;
}

#ifndef USE_FAST_UNMARSHALER
OZ_Term unmarshalOwnerRobustImpl(MsgBuffer *bs,MarshalTag mt,int *error){
#else
OZ_Term unmarshalOwnerImpl(MsgBuffer *bs,MarshalTag mt){
#endif
  if(mt==DIF_OWNER){
#ifndef USE_FAST_UNMARSHALER
    int OTI=unmarshalNumberRobust(bs,error);
#else
    int OTI=unmarshalNumber(bs);
#endif
    PD((UNMARSHAL,"OWNER o:%d",OTI));
    OwnerEntry* oe=OT->getOwner(OTI);
    oe->returnCreditOwner(1,OTI);
    OZ_Term oz=oe->getValue();
    return oz;}
  Assert(mt==DIF_OWNER_SEC);
#ifndef USE_FAST_UNMARSHALER
  int e1,e2;
  int OTI=unmarshalNumberRobust(bs,&e1);
  DSite* cs=unmarshalDSiteRobust(bs,&e2);
  sendSecondaryCredit(cs,myDSite,OTI,1);
  *error = e1 || e2;
#else
  int OTI=unmarshalNumber(bs);
  DSite* cs=unmarshalDSite(bs);
  sendSecondaryCredit(cs,myDSite,OTI,1);
#endif
  return OT->getOwner(OTI)->getValue();
}
