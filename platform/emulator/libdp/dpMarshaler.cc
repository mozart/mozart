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
#include "dpResource.hh"
/* for now credit is a 32-bit word */

// from var_obj
TaggedRef newObjectProxy(Object*, GName*, GName*, TaggedRef);

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
// 	oe->returnCreditOwner(cred);
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
  b->moreCredit(); // The Borrow needs some of the real McCoys
  ob=b;
  return 0;
}

/**********************************************************************/
/*  header */
/**********************************************************************/

MessageType unmarshalHeader(MsgBuffer *bs){
  refTable->reset();
  MessageType mt= (MessageType) bs->get();
  mess_counter[mt].recv();
  return mt;}

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

void marshalObjectImpl(Object *o, MsgBuffer *bs, GName *gnclass)
{
  if (marshalTertiaryImpl(o,DIF_OBJECT,bs)) return;   /* ATTENTION */
  Assert(o->getGName1());
  marshalGName(globalizeConst(o,bs),bs);
  marshalGName(gnclass,bs);
  trailCycleOutLine(o,bs);
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

void marshalObjectImpl(ConstTerm* t, MsgBuffer *bs) 
{
  PD((MARSHAL,"object"));
  Object *o = (Object*) t;
  Assert(o->getType() == Co_Object);

  ObjectClass *oc = o->getClass();
  globalizeConst(o,bs);
  marshalObjectImpl(o,bs,globalizeConst(oc,bs));}

Bool marshalTertiaryImpl(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
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
      if (sd && borrowTable->getOriginSite(BTI)==sd) {
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
    return "object";
  case DIF_RESOURCE_T:
  case DIF_RESOURCE_N:
    return "resource";
  default:
    Assert(0);
    return "";
}}

OZ_Term unmarshalTertiaryImpl(MsgBuffer *bs, MarshalTag tag)
{
  OB_Entry* ob;
  int bi;
  OZ_Term val = unmarshalBorrow(bs,ob,bi);
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

      ob->changeToVar(val); 
      return val;}
  default:         
    Assert(0);
  }
  val=makeTaggedConst(tert);
  ob->changeToTertiary(tert); 
  (void)((BorrowEntry*)ob)->getSite()->installProbe(PROBE_TYPE_ALL, TIME_SLICE);
  if(((BorrowEntry*)ob)->getSite()->siteStatus()!=SITE_OK){
    deferEntityProblem(tert);}
  return val;
}

OZ_Term unmarshalOwnerImpl(MsgBuffer *bs,MarshalTag mt){
  if(mt==DIF_OWNER){
    int OTI=unmarshalNumber(bs);
    PD((UNMARSHAL,"OWNER o:%d",OTI));
    OwnerEntry* oe=OT->getOwner(OTI);
    oe->returnCreditOwner(1,OTI);
    OZ_Term oz=oe->getValue();
    return oz;}
  Assert(mt==DIF_OWNER_SEC);
  int OTI=unmarshalNumber(bs);
  DSite* cs=unmarshalDSite(bs);
  sendSecondaryCredit(cs,myDSite,OTI,1);
  return OT->getOwner(OTI)->getValue();
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

