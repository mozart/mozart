/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Konstantin Popov (2000)
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
#pragma implementation "var_class.hh"
#endif

#include "var_base.hh"
#include "var_class.hh"
#include "dpMarshaler.hh"
#include "dpBase.hh"
#include "gname.hh"
#include "unify.hh"
#include "fail.hh"

//
LazyType ClassVar::getLazyType()
{
  return (LT_CLASS);
}

//
void ClassVar::marshal(ByteBuffer *bs)
{
  PD((MARSHAL,"var classproxy"));
  // lazy classes are not first class now, so:
  OZ_error("marshaling a ClassVar??!");
}

//
void ClassVar::sendRequest()
{
  // lazy classes are not first class now, so:
  OZ_error("requesting the class of a ClassVar??!");
}

//
// Special for the lazy objects protocol: check whether a request to
// the side 'ds' should be sent. BT's entry is ignored (which may be
// absent at all);
Bool ClassVar::sendRequest(DSite *ds)
{
  if (!lookupDSite(ds)) {
    addDSite(ds);
    return (OK);
  } else {
    return (NO);
  }
}

void ClassVar::gCollectRecurseV(void)
{
  if (index != MakeOB_TIndex(-1))
    borrowIndex2borrowEntry(index)->gcPO();
  Assert(gname);
  gCollectGName(gname);
  setInfo(gcEntityInfoInternal(getInfo()));
}

void ClassVar::disposeV()
{
  disposeS();
  while (dsl) {
    DSiteList *ne = dsl->getNext();
    delete dsl;
    dsl = ne;
  }
  freeListDispose(sizeof(ClassVar));
}

//
TaggedRef newClassProxy(OB_TIndex bi, GName *gnclass)
{
  ClassVar *pvar = new ClassVar(oz_currentBoard(), bi, gnclass);
  TaggedRef val = makeTaggedRef(newTaggedVar(extVar2Var(pvar)));
  return (val);
}

void ClassVar::transfer(OZ_Term cl, OZ_Term *cvtp)
{
  Assert(cl);
  DebugCode(GName *gnobj = getGName());
  Assert(gnobj->getValue() == cl);

  //
  oz_bindLocalVar(extVar2Var(this), cvtp, cl);
}
