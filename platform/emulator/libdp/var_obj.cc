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
#pragma implementation "var_obj.hh"
#endif

#include "var_obj.hh"
#include "dpMarshaler.hh"
#include "gname.hh"
#include "unify.hh"

void ObjectVar::marshal(MsgBuffer *bs)
{
  PD((MARSHAL,"var objectproxy"));
  int done=checkCycleOutLine(getObject(),bs);
  if (!done) {
    GName *classgn =  isObjectClassAvail()
      ? globalizeConst(getClass(),bs) : getGNameClass();
    marshalObjectImpl(getObject(),bs,classgn);
  }
}

/* --- ObjectProxis --- */

// mm2: deep as future!
OZ_Return ObjectVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  am.addSuspendVarList(lPtr);
  return SUSPEND;
}

// mm2: deep as future!
OZ_Return ObjectVar::unifyV(TaggedRef *lPtr, TaggedRef *rPtr)
{
  return oz_var_bind(tagged2CVar(*rPtr),rPtr,makeTaggedRef(lPtr));
}

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
  // mm2: this may be wrong, when gc deletes a dead thread!
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

void ObjectVar::disposeV()
{
  disposeS();
  if (isObjectClassNotAvail()) {
    deleteGName(u.gnameClass);
  }
  freeListDispose(this,sizeof(ObjectVar));
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
  oz_bindLocalVar(this,be->getPtr(),makeTaggedConst(o));
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
  oz_bindLocalVar(this,be->getPtr(),makeTaggedConst(o));
  be->changeToRef();
  BT->maybeFreeBorrowEntry(o->getIndex());
  o->localize();
}
