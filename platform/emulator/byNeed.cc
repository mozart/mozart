/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
#pragma implementation "byNeed.hh"
#endif

#include "byNeed.hh"

// if `function' is a procedure or an object, we simply call it
// with the variable itself as argument.
//
// for extension, `function' may also be a tuple, where typically
// the first element is a small integer that determines the
// operational interpretation:
//
//      1#P     ==> thread {P ME} end
//      2#X     ==> force request of X
//      3#URL   ==> thread {Load URL ME} end
//      4#call(P X1 ... Xn) ==> thread {P X1 ... Xn ME} end

void
ByNeedVariable::kickLazy(TaggedRef *ptr)
{
  if (function!=0) {
    Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
    thr->pushCall(function,makeTaggedRef(ptr));
    am.scheduleThread(thr);
    function=0;
  }
}

OZ_Return
ByNeedVariable::unifyV(TaggedRef *vPtr, TaggedRef t, ByteCode*scp)
{
  // if x:lazy=y:var y<-x if x is global, then trail
  // ^^^DONE AUTOMATICALLY
  // else x.kick() x=y

  kickLazy(vPtr);

  oz_bind(vPtr,*vPtr,t);
  return PROCEED;
}

void
ByNeedVariable::addSuspV(Suspension susp, TaggedRef *tPtr, int unstable)
{
  kickLazy(tPtr);
  addSuspSVar(susp, unstable);
}

OZ_BI_define(BIbyNeed,1,1)
{
  OZ_Term oz_fun = OZ_in(0);
  if (!OZ_isProcedure(oz_fun) && !OZ_isObject(oz_fun))
    return OZ_typeError(0,"Unary Procedure|Object");
  ByNeedVariable *lazy = new ByNeedVariable(oz_fun);
  OZ_RETURN(makeTaggedRef(newTaggedCVar(lazy)));
} OZ_BI_end
