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
#pragma implementation "var_future.hh"
#endif

#include "var_future.hh"
#include "dpInterface.hh"
#include "builtins.hh"
#include "unify.hh"
#include "thr_int.hh"
#include "value.hh"
#include "atoms.hh"

// this builtin is only internally available
OZ_BI_define(BIbyNeedAssign,2,0)
{
  OZ_Term var = OZ_in(0);
  DEREF(var,varPtr,_);
  OZ_Term val = OZ_in(1);

  oz_bindFuture(varPtr,val);
  return PROCEED;
} OZ_BI_end

/* call `function' as
 *    thread Tmp in {function Tmp} {`Assign` Future Tmp} end
 * return TRUE, if ptr is bound during kick
 */
Bool Future::kick(TaggedRef *ptr)
{
  if (!function) return FALSE;

  if (oz_isProcedure(function)) {
    Board* bb      = GETBOARD(this);
    Thread* thr    = oz_newThreadInject(bb);
    OZ_Term newvar = oz_newVar(bb);
    OZ_Term BI_ByNeedAssign=
      makeTaggedConst(new Builtin("byNeedAssign", 2, 0, BIbyNeedAssign, OK));
    thr->pushCall(BI_ByNeedAssign,makeTaggedRef(ptr),newvar);
    thr->pushCall(function,newvar);
  } else {
    Assert(oz_isTuple(function) && oz_eq(OZ_label(function),AtomDot));
    OZ_Term fut=oz_arg(function,0);
    OZ_Term fea=oz_arg(function,1);
    Board *bb = GETBOARD(this);
    if (oz_currentBoard()==bb) {
      OZ_Term aux=0;
      OZ_Term save=am.getSuspendVarList();
      OZ_Return ret=dotInline(fut,fea,aux);
      if (ret == PROCEED) {
        oz_bindFuture(ptr,aux);
        return TRUE;
      } else {
        switch (ret) {
        case SUSPEND:
          am.emptySuspendVarList();
          am.putSuspendVarList(save);
          break;
        case BI_REPLACEBICALL:
          am.emptyPreparedCalls();
          break;
        default:
          break;
        }
      }
      // fall through
    }

    OZ_Term newvar = oz_newVar(bb);
    Thread *thr = oz_newThreadInject(bb);
    OZ_Term BI_ByNeedAssign=
      makeTaggedConst(new Builtin("byNeedAssign", 2, 0, BIbyNeedAssign, OK));
    thr->pushCall(BI_ByNeedAssign,makeTaggedRef(ptr),newvar);
    thr->pushCall(BI_dot,fut,fea,newvar);
  }
  function=0;
  return FALSE;
}

OZ_Return Future::bind(TaggedRef *vPtr, TaggedRef t)
{
  if (kick(vPtr)) {
    // redo unification, because vPtr is bound
    return oz_unify(makeTaggedRef(vPtr),t);
  }

  if (oz_isLocalVar(this)) {
    am.addSuspendVarList(vPtr);
    return SUSPEND;
  } else {
    oz_bindVar(this,vPtr, t);
    return PROCEED;
  }
}

OZ_Return Future::forceBind(TaggedRef *vPtr, TaggedRef t)
{
  oz_bindVar(this,vPtr,t);
  return PROCEED;
}

OZ_Return Future::unify(TaggedRef *vPtr, TaggedRef *tPtr)
{
  return bind(vPtr,makeTaggedRef(tPtr));
}

OZ_Return Future::addSusp(TaggedRef *tPtr, Suspension susp, int unstable)
{
  if (kick(tPtr)) {
    return PROCEED;
  }
  addSuspSVar(susp, unstable);
  return SUSPEND;
}

void Future::printStream(ostream &out,int depth)
{
  if (function) {
      out << "<future byNeed: ";
      oz_printStream(function,out,depth-1);
      out << ">";
  } else {
    out << "<future>";
  }
}

OZ_Term Future::inspect()
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
  oz_bindFuture(fPtr,v);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIfuture,1,1)
{
  OZ_Term v = OZ_in(0);
  v = oz_safeDeref(v);
  if (oz_isRef(v)) {
    OZ_Term *vPtr = tagged2Ref(v);
    if (oz_isFuture(*vPtr)) OZ_RETURN(v);
    OzVariable *ov=oz_getVar(vPtr);
    Board *bb=GETBOARD(ov);
    OZ_Term f = oz_newFuture(bb);
    RefsArray args = allocateRefsArray(2, NO);
    args[0]=v;
    args[1]=f;
    if (bb!=oz_currentBoard()) {
      Thread *thr = oz_newThreadInject(bb);
      thr->pushCFun(VarToFuture,args,2);
    } else { // optimize: immediately suspend thread
      Thread *thr = oz_newThreadSuspended();
      thr->pushCFun(VarToFuture,args,2);
      OZ_Return ret = oz_var_addSusp(vPtr, thr);
      Assert(ret==SUSPEND);
    }
    OZ_RETURN(f);
  }
  OZ_RETURN(v);
} OZ_BI_end

OZ_BI_define(BIwaitQuiet,1,0)
{
  oz_declareDerefIN(0,fut);
  if (oz_isVariable(fut)) {
    if (oz_isFuture(fut)) {
      oz_getVar(futPtr)->addSuspSVar(oz_currentThread(),TRUE);
      return SUSPEND;
    }
    oz_suspendOnPtr(futPtr);
  }
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIbyNeed,1,1)
{
  oz_declareNonvarIN(0,p);
  if (oz_isProcedure(p) && oz_procedureArity(p)==1) {
      OZ_RETURN(makeTaggedRef(newTaggedCVar(new Future(oz_currentBoard(),p))));
  }
  oz_typeError(0,"Unary Procedure");
} OZ_BI_end

OZ_BI_define(BIbyNeedDot,2,1)
{
  oz_declareSafeDerefIN(0,fut);
  oz_declareNonvarIN(1,fea);
  if (!oz_isFeature(fea)) oz_typeError(1,"Feature");
  if (oz_isRef(fut)) {
    Future *newFut = new Future(oz_currentBoard(),
                                OZ_mkTuple(AtomDot,2,fut,fea));
    OZ_RETURN(makeTaggedRef(newTaggedCVar(newFut)));
  } else {
    OZ_Term aux=0;
    OZ_Return ret=dotInline(fut,fea,aux);
    OZ_result(aux);
    Assert(ret!=SUSPEND);
    return ret;
  }
} OZ_BI_end
