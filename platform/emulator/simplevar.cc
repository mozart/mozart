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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "simplevar.hh"
#endif

#include "simplevar.hh"
#include "am.hh"
#include "marshaler.hh"

OZ_Return SimpleVar::bind(TaggedRef* vPtr, TaggedRef t, ByteCode* scp)
{
  Assert(!oz_isRef(t));
  oz_bindToNonvar(vPtr, t);
  return PROCEED;
}

OZ_Return SimpleVar::unify(TaggedRef* vPtr, TaggedRef t, ByteCode* scp)
{
  Assert(!oz_isRef(t)||oz_isVariable(*tagged2Ref(t)));

  if (isExported()) {
    OZ_Return aux = export(t);
    if (aux!=PROCEED) return aux;
  }

  if (oz_isRef(t)) {
    TaggedRef *tPtr=tagged2Ref(t);
    GenCVariable *tv=tagged2CVar(*tPtr);
    if (tv->getType()==OZ_VAR_SIMPLE
        && oz_isBelow(GETBOARD(tv),GETBOARD(this))
#ifdef VAR_BIND_NEWER
        // if both are local, then check heap
        && (!am.isLocalSVar(this) || heapNewer(tPtr,vPtr))
#endif
        ) {

      if (tagged2SimpleVar(*tPtr)->isExported())
        markExported();

      t    = makeTaggedRef(vPtr);
      vPtr = tPtr;
    }
    oz_bind(vPtr, t);
  } else {
    oz_bindToNonvar(vPtr, t);
  }
  return PROCEED;
}

GenCVariable *uvar2SimpleVar(TaggedRef *v)
{
  GenCVariable *sv = new SimpleVar(tagged2VarHome(*v));
  *v = makeTaggedCVar(sv);
  return sv;
}

void addSuspUVar(TaggedRef *v, Suspension susp, int unstable)
{
  uvar2SimpleVar(v)->addSuspSVar(susp, unstable);
}
