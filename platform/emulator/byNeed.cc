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

/* if `function' is a procedure or an object, we simply call it with a
 * new variable as argument: local Tmp in {F Tmp} This := Tmp end
 */


OZ_BI_define(BIbyNeedAssign,2,0)
{
  OZ_Term var = OZ_in(0);
  DEREF(var,varPtr,_);
  OZ_Term val = OZ_in(1);

  Assert(isByNeedVariable(var));
  oz_bind(varPtr,var,val);
  return PROCEED;
} OZ_BI_end

void
ByNeedVariable::kickLazy(TaggedRef *ptr)
{
  if (function!=0) {
    Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,GETBOARD(this));
    OZ_Term newvar=oz_newVar(GETBOARD(this));
    static RefsArray args = allocateStaticRefsArray(2);
    args[0]=makeTaggedRef(ptr);
    args[1]=newvar;
    thr->pushCFun(BIbyNeedAssign, args, 2, OK);
    thr->pushCall(function,newvar);
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

  oz_suspendOnPtr(vPtr);
  return SUSPEND;
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
