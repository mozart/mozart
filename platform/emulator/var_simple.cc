/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Michael Mehl (1998)
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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_simple.hh"
#endif

#include "var_simple.hh"
#include "unify.hh"

OZ_Return SimpleVar::bind(TaggedRef* vPtr, TaggedRef t)
{
  oz_bindVar(this,vPtr, t);
  return PROCEED;
}

// from marshaler.cc
OZ_Return export(OZ_Term t);

OZ_Return SimpleVar::unify(TaggedRef* vPtr, TaggedRef *tPtr)
{
  // mm2
  if (isExported()) {
    OZ_Return aux = export(makeTaggedRef(tPtr));
    if (aux!=PROCEED) return aux;
  }

  OzVariable *tv=tagged2CVar(*tPtr);
  if (tv->getType()==OZ_VAR_SIMPLE
      && oz_isBelow(tv->getBoardInternal(),GETBOARD(this))
#ifdef VAR_BIND_NEWER
      // if both are local, then check heap
      && (!oz_isLocalVar(this) || heapNewer(tPtr,vPtr))
#endif
      ) {

    if (tv->isExported())
      markExported();

    oz_bindVar(tv,tPtr, makeTaggedRef(vPtr));
  } else {
    oz_bindVar(this,vPtr, makeTaggedRef(tPtr));
  }
  return PROCEED;
}

OzVariable *oz_newSimpleVar(Board *bb)
{
  return new SimpleVar(bb);
}
