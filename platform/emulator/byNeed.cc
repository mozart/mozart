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
#pragma implementation "byNeed.hh"
#endif

#include "byNeed.hh"

inline
Bool isByNeedVariable(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_BYNEED);
}

inline
ByNeedVariable *tagged2ByNeedVariable(TaggedRef t) {
  Assert(isByNeedVariable(t));
  return (ByNeedVariable *) tagged2CVar(t);
}

// this builtin is only internally available
OZ_BI_define(BIbyNeedAssign,2,0)
{
  OZ_Term var = OZ_in(0);
  DEREF(var,varPtr,_);
  OZ_Term val = OZ_in(1);

  Assert(isByNeedVariable(var));
  oz_bind(varPtr,var,val);
  return PROCEED;
} OZ_BI_end

/* call `function' as
 *    local Tmp in {function Tmp} This := Tmp end
 */

void ByNeedVariable::kick(TaggedRef *ptr)
{
  if (function!=0) {
    Board* bb      = GETBOARD(this);
    Thread* thr    = am.mkRunnableThread(DEFAULT_PRIORITY,bb);
    OZ_Term newvar = oz_newVar(bb);

    static RefsArray args = allocateStaticRefsArray(2);
    args[0]=makeTaggedRef(ptr);
    args[1]=newvar;

    thr->pushCFun(BIbyNeedAssign, args, 2, OK);
    thr->pushCall(function,newvar);
    am.scheduleThread(thr);
    function=0;
  }
}

OZ_Return ByNeedVariable::unifyV(TaggedRef *vPtr, TaggedRef t, ByteCode*scp)
{
  kick(vPtr);

  oz_suspendOnPtr(vPtr);
  return SUSPEND;
}

void ByNeedVariable::addSuspV(Suspension susp, TaggedRef *tPtr, int unstable)
{
  kick(tPtr);
  addSuspSVar(susp, unstable);
}

void ByNeedVariable::printStreamV(ostream &out,int depth = 10) {
    OZ_Term f = getFunction();
    if (f==0) {
      out << "<future byNeed: requested>";
    } else {
      out << "<future byNeed: ";
      oz_printStream(f,out,depth-1);
      out << ">";
    }
  }

OZ_Term ByNeedVariable::inspectV()
{
  // future(byNeed(requested|F))
  OZ_Term f = getFunction();
  if (f==0) {
    f=oz_atom("requested");
  }
  return OZ_mkTupleC("future",1, OZ_mkTupleC("byNeed",1,f));
}

OZ_BI_define(BIbyNeed,1,1)
{
  OZ_Term oz_fun = OZ_in(0);
  if (!OZ_isProcedure(oz_fun) && !OZ_isObject(oz_fun)) {
    oz_typeError(0,"Unary Procedure|Object");
  }
  ByNeedVariable *lazy = new ByNeedVariable(oz_fun);
  OZ_RETURN(makeTaggedRef(newTaggedCVar(lazy)));
} OZ_BI_end

