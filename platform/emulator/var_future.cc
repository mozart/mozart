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
Bool isFuture(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_FUTURE);
}

inline
Future *tagged2Future(TaggedRef t) {
  Assert(isFuture(t));
  return (Future *) tagged2CVar(t);
}

// this builtin is only internally available
OZ_BI_define(BIbyNeedAssign,2,0)
{
  OZ_Term var = OZ_in(0);
  DEREF(var,varPtr,_);
  OZ_Term val = OZ_in(1);

  Assert(isFuture(var));
  oz_bind(varPtr,var,val);
  return PROCEED;
} OZ_BI_end

/* call `function' as
 *    thread Tmp in {function Tmp} {`Assign` Future Tmp} end
 */
void Future::kick(TaggedRef *ptr)
{
  Assert(function!=0);
  Board* bb      = GETBOARD(this);
  Thread* thr    = oz_mkRunnableThread(DEFAULT_PRIORITY,bb);
  OZ_Term newvar = oz_newVar(bb);

  static RefsArray args = allocateStaticRefsArray(2);
  args[0]=makeTaggedRef(ptr);
  args[1]=newvar;

  thr->pushCFun(BIbyNeedAssign, args, 2, OK);
  thr->pushCall(function,newvar);
  am.threadsPool.scheduleThread(thr);
  function=0;
}

OZ_Return Future::unifyV(TaggedRef *vPtr, TaggedRef t, ByteCode*scp)
{
  if (function) kick(vPtr);

  oz_suspendOnPtr(vPtr);
  return SUSPEND;
}

void Future::addSuspV(Suspension susp, TaggedRef *tPtr, int unstable)
{
  if (function) kick(tPtr);

  addSuspSVar(susp, unstable);
}

void Future::printStreamV(ostream &out,int depth = 10)
{
  if (function) {
      out << "<future byNeed: ";
      oz_printStream(function,out,depth-1);
      out << ">";
  } else {
    out << "<future>";
  }
}

OZ_Term Future::inspectV()
{
  OZ_Term k=function ? OZ_mkTupleC("byNeed",1,function) : oz_atom("simple");
  return OZ_mkTupleC("future", 1, k);
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
  Assert(isFuture(f));
  oz_bind(fPtr,f,v);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIfuture,1,1)
{
  OZ_Term v = OZ_in(0);
  v = oz_safeDeref(v);
  if (oz_isRef(v)) {
    OZ_Term f = makeTaggedRef(newTaggedCVar(new Future()));
    RefsArray args = allocateRefsArray(2, NO);
    args[0]=v;
    args[1]=f;
    Thread *thr = (Thread *) OZ_makeSuspendedThread(VarToFuture,args,2);
    addSuspAnyVar(tagged2Ref(v),thr);
    OZ_RETURN(f);
  }
  OZ_RETURN(v);
} OZ_BI_end

OZ_BI_define(BIbyNeed,1,1)
{
  OZ_Term p = OZ_in(0);
  if (!OZ_isProcedure(p) && !OZ_isObject(p)) {
    oz_typeError(0,"Unary Procedure|Object");
  }
  OZ_RETURN(makeTaggedRef(newTaggedCVar(new Future(p))));
} OZ_BI_end
