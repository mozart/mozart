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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "simplevar.hh"
#endif

#include "simplevar.hh"
#include "runtime.hh"

OZ_Return SimpleVar::unifyV(TaggedRef* vPtr, TaggedRef t, ByteCode* scp) {
  TaggedRef v=*vPtr;
  Assert(!oz_isRef(t)||oz_isVariable(*tagged2Ref(t)));
  if (oz_isRef(t)) {
    TaggedRef *tPtr=tagged2Ref(t);
    GenCVariable *tv=tagged2CVar(*tPtr);
    if (tv->getType()==SimpleVarType
        && oz_isBelow(GETBOARD(tv),GETBOARD(this))
#ifdef VAR_BIND_NEWER
        // if both are local, then check heap
        && (!am.isLocalSVar(this) ||
            heapNewer(tPtr,vPtr))
#endif
        ) {
      t=    makeTaggedRef(vPtr);
      v=    *tPtr;
      vPtr= tPtr;
    }
    oz_bind(vPtr, v, t);
  } else {
    oz_bindToNonvar(vPtr, v, t);
  }
  return PROCEED;
}

#ifdef SIMPLEVAR
void addSuspUVar(TaggedRef *vPtr, Suspension susp, int unstable = TRUE)
{
  GenCVariable *cv = new SimpleVar(tagged2VarHome(*vPtr));
  *vPtr = makeTaggedCVar(cv);
  cv->addSuspSVar(susp, unstable);
}
#endif
