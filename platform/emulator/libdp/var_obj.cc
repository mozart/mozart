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
#pragma implementation "var_obj.hh"
#endif

#include "var_obj.hh"
#include "dpMarshaler.hh"
#include "dpBase.hh"
#include "gname.hh"
#include "unify.hh"
#include "fail.hh"


//
void ObjectVar::marshal(ByteBuffer *bs)
{
  PD((MARSHAL,"var objectproxy"));
  GName *classgn = isObjectClassAvail()
    ? ((ObjectClass *) tagged2Const(getClass()))->getGName()
    : getGNameClass();
  marshalVarObject(bs, index, gname, classgn);
}

/* --- ObjectProxis --- */

//
TaggedRef newObjectProxy(int bi, GName *gnobj, GName *gnclass)
{
  ObjectVar *pvar;
  pvar = new ObjectVar(oz_currentBoard(), bi, gnobj, gnclass);
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  return (val);
}

//
TaggedRef newObjectProxy(int bi, GName *gnobj, TaggedRef clas)
{
  ObjectVar *pvar;
  pvar = new ObjectVar(oz_currentBoard(), bi, gnobj,
                       tagged2ObjectClass(oz_deref(clas)));
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  return (val);
}

LazyType ObjectVar::getLazyType()
{
  return (LT_OBJECT);
}

void ObjectVar::sendRequest()
{
  // There could be an optimization: avoiding retrieving the same
  // class twice from different objects (not done);
  LazyFlag sendClass = isObjectClassNotAvail() ? OBJECT_AND_CLASS : OBJECT;
  BorrowEntry *be=BT->getBorrow(index);

  NetAddress* na=be->getNetAddress();
  // MarshalerBuffer *bs=msgBufferManager->getMarshalerBuffer(na->site);

  //
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->setImplicitMessageCredit(be->getOneMsgCredit());

  //
  msgC->put_M_GET_LAZY(na->index, sendClass, myDSite);
  SendTo(na->site, msgC, 3);
}

void ObjectVar::gCollectRecurseV(void)
{
  BT->getBorrow(index)->gcPO();
  if (isObjectClassAvail()) {
    oz_gCollectTerm(u.aclass, u.aclass);
  } else {
    Assert(u.gnameClass);
    gCollectGName(u.gnameClass);
  }
  Assert(gname);
  gCollectGName(gname);
  setInfo(gcEntityInfoInternal(getInfo()));
}

void ObjectVar::disposeV()
{
  disposeS();
  // PER-LOOK
  // kost@ : ... so what? found something?
  if (isObjectClassNotAvail())
    deleteGName(u.gnameClass);
  // Don't touch gname, since it appears in the object itself!!!
  freeListDispose(this,sizeof(ObjectVar));
}

void ObjectVar::transfer(Object *o, BorrowEntry *be)
{
  Assert(o);
  Assert(isObjectClassAvail());
  GName *gnobj = getGName();
  gnobj->setValue(makeTaggedConst(o));

  //
  EntityInfo *savedInfo = info; // bind disposes this!
  Assert(be->isVar());
  oz_bindLocalVar(this, be->getPtr(), makeTaggedConst(o));

  //
  be->changeToRef();
  maybeHandOver(savedInfo, makeTaggedConst(o));
  (void) BT->maybeFreeBorrowEntry(index);
}
