/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Per Brand (perbrand@sics.se)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Erik Klintskog (erik@sics.se)
 *    Konstantin Popov <kost@sics.se>
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

#include "dpBase.hh"
#include "var_obj.hh"
#include "msgContainer.hh"
#include "dpMarshaler.hh"
#include "gname.hh"
#include "unify.hh"
#include "fail.hh"


//
void ObjectVar::marshal(ByteBuffer *bs)
{
  PD((MARSHAL,"var objectproxy"));
  GName *classgn = getGNameClass();
  marshalVarObject(bs, index, gname, classgn);
}

//
TaggedRef newObjectProxy(int bi, GName *gnobj, TaggedRef cl)
{
  ObjectVar *pvar = new ObjectVar(oz_currentBoard(), bi, gnobj, cl);
  TaggedRef val = makeTaggedRef(newTaggedVar(extVar2Var(pvar)));
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
  LazyFlag sendClass = isObjectClassAvail() ? OBJECT : OBJECT_AND_CLASS;
  BorrowEntry *be=BT->getBorrow(index);      

  NetAddress* na=be->getNetAddress();
  // MarshalerBuffer *bs=msgBufferManager->getMarshalerBuffer(na->site);

  //
  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);

  //
  msgC->put_M_GET_LAZY(na->index, sendClass, myDSite);
  send(msgC, -1);
}

void ObjectVar::gCollectRecurseV(void)
{
  BT->getBorrow(index)->gcPO();
  oz_gCollectTerm(aclass, aclass);
  Assert(gname);
  gCollectGName(gname);
  setInfo(gcEntityInfoInternal(getInfo()));
}

void ObjectVar::disposeV()
{
  disposeS();
  // PER-LOOK
  // kost@ : ... so what? found something?
  // Don't touch gname, since it appears in the object itself!!!
  freeListDispose(sizeof(ObjectVar));
}

void ObjectVar::transfer(Object *o, BorrowEntry *be)
{
  Assert(o);
  Assert(isObjectClassAvail());
  DebugCode(GName *gnobj = getGName());
  Assert(gnobj->getValue() == makeTaggedConst(o));

  //
  EntityInfo *savedInfo = info; // bind disposes this!
  Assert(be->isVar());
  oz_bindLocalVar(extVar2Var(this), be->getPtr(), makeTaggedConst(o));

  //
  be->changeToRef();
  maybeHandOver(savedInfo, makeTaggedConst(o));
  (void) BT->maybeFreeBorrowEntry(index);
}



