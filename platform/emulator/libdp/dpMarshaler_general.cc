/*
 *  Authors:
 *    Andreas Sundstrom (andreas@sics.se)
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *
 *  Copyright:
 *    1997-1998 Konstantin Popov
 *    1997 Per Brand
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

//---------------------------------------------------------------------
// General unmarshaling procedures included in dpMarshaler.cc
//---------------------------------------------------------------------

#ifdef ROBUST_UNMARSHALER
OZ_Term unmarshalBorrowRobust(MsgBuffer *bs,OB_Entry *&ob,int &bi,int *error){
#else
OZ_Term unmarshalBorrow(MsgBuffer *bs,OB_Entry *&ob,int &bi){
#endif
  PD((UNMARSHAL,"Borrow"));
#ifdef ROBUST_UNMARSHALER
  DSite*  sd=unmarshalDSiteRobust(bs,error);
  if(*error) return 0;
  int si=unmarshalNumberRobust(bs,error);
  if(*error) return 0;
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
#ifdef ROBUST_UNMARSHALER
    cred = unmarshalCreditRobust(bs,error);
    if(*error) return 0;
#else
    cred = unmarshalCredit(bs);
#endif
    if(mt==DIF_PRIMARY){
      if(cred!=PERSISTENT_CRED)
        b->addPrimaryCredit(cred);
      else Assert(b->isPersistent());}
    else{
      Assert(mt==DIF_SECONDARY);
#ifdef ROBUST_UNMARSHALER
      DSite* s=unmarshalDSiteRobust(bs,error);
      if(*error) return 0;
#else
      DSite* s=unmarshalDSite(bs);
#endif
      b->addSecondaryCredit(cred,s);}
    ob = b;
    return b->getValue();}
#ifdef ROBUST_UNMARSHALER
  cred = unmarshalCreditRobust(bs,error);
  if(*error) return 0;
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
#ifdef ROBUST_UNMARSHALER
  DSite* site = unmarshalDSiteRobust(bs,error);
  if(*error) return 0;
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

#ifdef ROBUST_UNMARSHALER
OZ_Term unmarshalTertiaryRobustImpl(MsgBuffer *bs, MarshalTag tag, int *error)
#else
OZ_Term unmarshalTertiaryImpl(MsgBuffer *bs, MarshalTag tag)
#endif
{
  OB_Entry* ob;
  int bi;
#ifdef ROBUST_UNMARSHALER
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
#ifdef ROBUST_UNMARSHALER
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
#ifdef ROBUST_UNMARSHALER
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
    {
      TaggedRef obj;
      TaggedRef clas;
#ifdef ROBUST_UNMARSHALER
      GName *gnobj = unmarshalGNameRobust(&obj,bs,&e2);
      GName *gnclass = unmarshalGNameRobust(&clas,bs,&e3);
      *error = e1 || e2 || e3;
#else
      GName *gnobj = unmarshalGName(&obj,bs);
      GName *gnclass = unmarshalGName(&clas,bs);
#endif
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
#ifdef ROBUST_UNMARSHALER
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

#ifdef ROBUST_UNMARSHALER
OZ_Term unmarshalOwnerRobustImpl(MsgBuffer *bs,MarshalTag mt,int *error){
#else
OZ_Term unmarshalOwnerImpl(MsgBuffer *bs,MarshalTag mt){
#endif
  if(mt==DIF_OWNER){
#ifdef ROBUST_UNMARSHALER
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
#ifdef ROBUST_UNMARSHALER
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
