/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    derived from lazyvar.cc by Denys Duchier (duchier@ps.uni-sb.de)
 *
 *  Copyright:
 *    Denys Duchier (1998)
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
#pragma implementation "future.hh"
#endif

#include "future.hh"

inline
Bool isPropFuture(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_FUTURE);
}

inline
PropFuture *tagged2PropFuture(TaggedRef t) {
  Assert(isPropFuture(t));
  return (PropFuture *) tagged2CVar(t);
}

OZ_Return PropFuture::unifyV(TaggedRef *vPtr, TaggedRef t, ByteCode*scp)
{
  oz_suspendOnPtr(vPtr);
  return SUSPEND;
}

void PropFuture::addSuspV(Suspension susp, TaggedRef *tPtr, int unstable)
{
  addSuspSVar(susp, unstable);
}

void PropFuture::printStreamV(ostream &out,int depth = 10)
{
  out << "<future>";
}

OZ_Term PropFuture::inspectV()
{
  return oz_atom("future");
}

// this builtin/propagator is only internally available
OZ_BI_define(VarToFuture,2,0)
{
  OZ_Term v = OZ_in(0);
  v = oz_safeDeref(v);
  if (oz_isRef(v)) {
    oz_suspendOnVar(v);
  }
  OZ_Term f = OZ_in(1);
  DEREF(f,fPtr,_);
  Assert(isPropFuture(f));
  oz_bind(fPtr,f,v);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIfuture,1,1)
{
  OZ_Term v = OZ_in(0);
  v = oz_safeDeref(v);
  if (oz_isRef(v)) {
    OZ_Term f = makeTaggedRef(newTaggedCVar(new PropFuture()));
    RefsArray args = allocateRefsArray(2, NO);
    args[0]=v;
    args[1]=f;
    Thread *thr = (Thread *) OZ_makeSuspendedThread(VarToFuture,args,2);
    addSuspAnyVar(tagged2Ref(v),thr);
    OZ_RETURN(f);
  }
  OZ_RETURN(v);
} OZ_BI_end
