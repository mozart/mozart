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
OZ_BI_define(BIbindFuture,2,0)
{
  OZ_Term var = OZ_in(0);
  DEREF(var,varPtr);
  OZ_Term val = OZ_in(1);

  oz_bindFuture(varPtr,val);
  return PROCEED;
} OZ_BI_end

/* call `function' as
 *    thread Tmp in {function Tmp} {`Assign` Future Tmp} end
 * return PROCEED, if ptr is bound during kick
 * return SUSPEND, if ptr remains unbound
 * return RAISE, if kicking raises an exception
 */
OZ_Return Future::kick(TaggedRef *ptr)
{
  if (!function) return SUSPEND;

  Board* bb      = GETBOARD(this);
  
  if (oz_isProcedure(function)) {
    Thread* thr    = oz_newThreadInject(bb);
    OZ_Term newvar = oz_newVariable(bb);
    thr->pushCall(BI_bindFuture,RefsArray::make(makeTaggedRef(ptr),newvar));
    thr->pushCall(function,RefsArray::make(newvar));
  } else {
    Assert(oz_isTuple(function));
    if (oz_eq(OZ_label(function),AtomDot)) {
      OZ_Term fut=oz_arg(function,0);
      OZ_Term fea=oz_arg(function,1);

      if (oz_currentBoard()==bb) {
	OZ_Term aux=0;
	OZ_Term save=am.getSuspendVarList();
	OZ_Return ret=dotInline(fut,fea,aux);
	if (ret == PROCEED) {
	  oz_bindFuture(ptr,aux);
	  return PROCEED;
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

      OZ_Term newvar = oz_newVariable(bb);
      Thread *thr = oz_newThreadInject(bb);
      thr->pushCall(BI_bindFuture,RefsArray::make(makeTaggedRef(ptr),newvar));
      thr->pushCall(BI_dot,RefsArray::make(fut,fea,newvar));
    } else {
      Assert(oz_eq(OZ_label(function),AtomFail));
      OZ_Term exn=oz_arg(function,0);

      return OZ_raiseDebug(exn);
    }
  }
  function=0;
  return SUSPEND;
}

OZ_Return Future::bind(TaggedRef *vPtr, TaggedRef t)
{
  switch (kick(vPtr)) {
  case PROCEED:
    // redo unification, because vPtr is bound
    return oz_unify(makeTaggedRef(vPtr),t);
  case RAISE:
    return RAISE;
  }

  if (oz_isLocalVar(this)) {
    return am.addSuspendVarListInline(vPtr);
  } else {
    oz_bindVar(this,vPtr, t);
    return PROCEED;
  }
}

OZ_Return Future::forceBind(TaggedRef *vPtr, TaggedRef t)
{
  if (*vPtr != oz_deref(t))
    oz_bindVar(this,vPtr,t);
  return PROCEED;
}

OZ_Return Future::unify(TaggedRef *vPtr, TaggedRef *tPtr)
{
  return bind(vPtr,makeTaggedRef(tPtr));
}

OZ_Return Future::addSusp(TaggedRef *tPtr, Suspendable * susp) {
  OZ_Return ret = kick(tPtr);
  if (ret == SUSPEND) {
    addSuspSVar(susp);
  }
  return ret;
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

inline Bool Future::isFailed()
{
  return function && oz_isTuple(function) && oz_eq(OZ_label(function),AtomFail);
}

// this builtin/propagator is only internally available
OZ_BI_define(BIvarToFuture,2,0)
{
  oz_declareDerefIN(0,v);
  if (oz_isVarOrRef(v)) {
    if (oz_isFailed(v)) {   // added by raph
      v = makeTaggedRef(vPtr);
      goto bind_fut;
    }
    if (oz_isFuture(v)) {
      Assert(tagged2Var(v)->getType() == OZ_VAR_FUTURE);
      if (((Future*)tagged2Var(v))->isFailed()) {
	v = makeTaggedRef(vPtr);
	goto bind_fut;
      }
      else {
	((Future*)tagged2Var(v))->addSuspSVar(oz_currentThread());
	return SUSPEND;
      }
    }
    else {
      oz_suspendOnPtr(vPtr);
    }
  }
 bind_fut:
  oz_declareDerefIN(1,f);
  oz_bindFuture(fPtr,v);
  return PROCEED;
} OZ_BI_end

// deprecated
OZ_BI_define(BIfuture,1,1)
{
  Assert(0);   // raph: should no longer be used
  TaggedRef v = OZ_in(0);
  v = oz_safeDeref(v);
  if (oz_isRef(v)) {
    OZ_Term *vPtr = tagged2Ref(v);
    if (oz_isFuture(*vPtr)) OZ_RETURN(v);
    if (oz_isFailed(*vPtr)) OZ_RETURN(v);   // added by raph
    OzVariable *ov = tagged2Var(*vPtr);
    Board *bb = GETBOARD(ov);
    TaggedRef f = oz_newFuture(bb);
    RefsArray * args = RefsArray::make(v,f);
    if (bb != oz_currentBoard()) {
      Thread *thr = oz_newThreadInject(bb);
      thr->pushCall(BI_varToFuture, args);
    } else { // optimize: immediately suspend thread
      Thread *thr = oz_newThreadSuspended();
      thr->pushCall(BI_varToFuture, args);
      OZ_Return ret = oz_var_addSusp(vPtr, thr);
      Assert(ret==SUSPEND);
    }
    OZ_RETURN(f);
  }
  OZ_RETURN(v);
} OZ_BI_end

OZ_BI_define(BIbyNeedFuture,1,1)
{
  oz_declareNonvarIN(0,p);
  if (oz_isProcedure(p) && oz_procedureArity(p)==1) {
      OZ_RETURN(makeTaggedRef(newTaggedVar(new Future(oz_currentBoard(),p))));
  }
  oz_typeError(0,"Unary Procedure");
} OZ_BI_end

// deprecated
OZ_BI_define(BIbyNeedDot,2,1)
{
  Assert(0);   // raph: should no longer be used
  oz_declareSafeDerefIN(0,fut);
  oz_declareNonvarIN(1,fea);
  if (!oz_isFeature(fea)) oz_typeError(1,"Feature");
  if (oz_isRef(fut)) {
    Future *newFut = new Future(oz_currentBoard(),
				OZ_mkTuple(AtomDot,2,fut,fea));
    OZ_RETURN(makeTaggedRef(newTaggedVar(newFut)));
  } else {
    OZ_Term aux=0;
    OZ_Return ret=dotInline(fut,fea,aux);
    if (ret==RAISE) {
      Future *newFut =
	new Future(oz_currentBoard(),
		   OZ_mkTuple(AtomFail,1,am.getExceptionValue()));
      OZ_RETURN(makeTaggedRef(newTaggedVar(newFut)));
    }
    Assert(ret!=SUSPEND);
    OZ_RETURN(aux);
  }
} OZ_BI_end

// deprecated
OZ_BI_define(BIbyNeedFail,1,1)
{
  Assert(0);   // raph: should no longer be used
  Future *newFut = new Future(oz_currentBoard(),
			      OZ_mkTuple(AtomFail,1,OZ_in(0)));
  OZ_RETURN(makeTaggedRef(newTaggedVar(newFut)));
} OZ_BI_end
