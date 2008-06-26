/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (scheidhr@ps.uni-sb.de)
 *
 *  Contributors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *    Raphael Collet (raph@info.ucl.ac.be)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifdef INTERFACE
#pragma implementation
#pragma implementation "unify.hh"
#endif

#include "unify.hh"
#include "fset.hh"
#include "thr_int.hh"


/* -------------------------------------------------------------------------
 * Binding
 * -------------------------------------------------------------------------*/

#ifdef DEBUG_CHECK
static
Bool checkHome(TaggedRef *vPtr) {
  TaggedRef val = oz_deref(*vPtr);
  Assert(!oz_isRef(val));
  return (!oz_isVarOrRef(val) ||
	  oz_isBelow(oz_currentBoard(), GETBOARD(tagged2Var(val))));
}
#endif

/*
 * oz_bindVar: bind a variable to a value
 *   - trail global vars
 *   - wakeup suspensions
 *   - relink suspensions of local vars
 *   - dispose local vars
 *   - redirect REF
 */
void oz_bindGlobalVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term)
{
  Assert(tagged2Var(*varPtr)==ov);
  Assert(!oz_isLocalVar(ov));
  Assert(am.inEqEq() || checkHome(varPtr));
  oz_checkSuspensionList(ov, pc_std_unif);
  trail.pushBind(varPtr);
  doBind(varPtr,term);
}

void oz_bindLocalVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term)
{
  Assert(tagged2Var(*varPtr)==ov);
  Assert(oz_isLocalVar(ov));
  Assert(!am.inEqEq());
  oz_checkSuspensionList(ov, pc_std_unif);
  DEREF(term,termPtr);
  Assert(!oz_isRef(term));
  if (oz_isVarOrRef(term)) {
    OzVariable *sv=tagged2Var(term);
    Assert(sv!=ov);
    ov->relinkSuspListTo(sv);
    term=makeTaggedRef(termPtr);
  } 
  // kost@ : cut away so far;
  /*
    else if (oz_isUVar(term)){
      // mm2: problems with fsp_monitorIn, monitorArity,...
      // Assert(ov->isEmptySuspList());
      term = makeTaggedRef(termPtr);
    }
  */
  oz_var_dispose(ov);
  doBind(varPtr,term);
}

void oz_bind_global(TaggedRef var, TaggedRef term)
{
  DEREF(var,varPtr);
  if (!oz_isOptVar(var)) {
    OzVariable *ov=tagged2Var(var);
    oz_checkSuspensionList(ov, pc_all);
    DEREF(term,termPtr);
    Assert(!oz_isRef(term));
    if (oz_isVarOrRef(term)) {
      OzVariable *sv=tagged2Var(term);
      Assert(sv!=ov);
      ov->relinkSuspListTo(sv);
      term=makeTaggedRef(termPtr);
    } else {
      Assert(ov->isEmptySuspList());
    }
    oz_var_dispose(ov);
  }
  doBind(varPtr,term);
}

/* -------------------------------------------------------------------------
 * Unification
 * -------------------------------------------------------------------------*/

//
// kost@ : Variables with GREATER types are bound to variables with
// SMALLER types;
#define CMPVAR(v1, v2)		(v1->getType() - v2->getType())

const StackEntry mark=(StackEntry)-1;
class UnifyStack : public Stack {
public:
  UnifyStack() : Stack(100,Stack_WithMalloc) {}
  void pushMark() { push(mark); }
  Bool isMark() { return topElem()==mark; }
};

// global vars!!!
static UnifyStack unifyStack;
static Stack rebindTrail(100,Stack_WithMalloc);

inline
void rebind(TaggedRef *refPtr, TaggedRef term2)
{
  rebindTrail.ensureFree(2);
  rebindTrail.push(refPtr,NO);
  rebindTrail.push(ToPointer(*refPtr),NO);
  doBind(refPtr,term2);
}

#define PopRebindTrail(value,refPtr)                    \
    TaggedRef value   = ToInt32(rebindTrail.pop());     \
    TaggedRef *refPtr = (TaggedRef*) rebindTrail.pop();


// raph: Variable bindings that suspend do not need to stop the whole
// unification.  Instead we put them aside on a trail, and let the
// process carry on.  In case of success, we simplify the unification,
// and suspend on those variables.  This keeps less references alive,
// and enables parallel distributed bindings.

// hold suspended bindings
static Stack suspendTrail(100,Stack_WithMalloc);

inline
void suspendUnify(TaggedRef term1, TaggedRef term2) {
  suspendTrail.ensureFree(2);
  suspendTrail.push(ToPointer(term2), NO);
  suspendTrail.push(ToPointer(term1), NO);
}

inline
TaggedRef popSuspended() { // beware: they come one by one!
  return ToInt32(suspendTrail.pop());
}



OZ_Return oz_unify(TaggedRef t1, TaggedRef t2)
{
  unifyStack.pushMark();
  Assert(!oz_isVar(t1) && !oz_isVar(t2));

  OZ_Return result = FAILED;

  TaggedRef term1 = t1;
  TaggedRef term2 = t2;
  TaggedRef *termPtr1 = &term1;
  TaggedRef *termPtr2 = &term2;
  stag_t tag1, tag2;

loop:
  int argSize;

  _DEREF(term1,termPtr1);
  _DEREF(term2,termPtr2);

  // identical terms ?
  Assert(!oz_isRef(term1));
  if (oz_isVarOrRef(term1) ? termPtr1 == termPtr2 : term1 == term2) {
    goto next;
  }

  Assert(!oz_isRef(term1));
  Assert(!oz_isRef(term2));
  if (oz_isVarOrRef(term1)) {
    if (oz_isVarOrRef(term2)) {
      goto var_var;
    } else {
      goto var_nonvar;
    }
  } else {
    if (oz_isVarOrRef(term2)) {
      Swap(term1,term2,TaggedRef);
      Swap(termPtr1,termPtr2,TaggedRef*);
      goto var_nonvar;
    } else {
      goto nonvar_nonvar;
    }
  }


 /*************/
 var_nonvar:

  if (oz_isOptVar(term1)) {
    doBind(termPtr1, term2);
    goto next;
  } else {
    int res = oz_var_bind(tagged2Var(term1), termPtr1, term2);
    if (res == PROCEED) {
      goto next;
    } else if (res == SUSPEND) {
      suspendUnify(makeTaggedRef(termPtr1), term2);
      goto next;
    } else {
      result = res;
      goto fail;
    }
  }
  Assert(0);


 /*************/
 var_var:

  /*
   * Specially optimized: local simple vars without suspensions
   */
  {
    const OZ_Term ov = am.getCurrentOptVar();

    if (term1 == ov) {
      // Both have the same home, order them according to age
      if (term1 == term2 && heapNewer(termPtr2, termPtr1))
	doBind(termPtr2, makeTaggedRef(termPtr1));
      else
	doBind(termPtr1, makeTaggedRef(termPtr2));
     goto next;
    }

    if (term2 == ov) {
      // Both have the same home, order them according to age
      if (term1 == term2 && heapNewer(termPtr1, termPtr2))
	doBind(termPtr1, makeTaggedRef(termPtr2));
      else
	doBind(termPtr2, makeTaggedRef(termPtr1));
      goto next;
    }
  }

  /*
   * Prescribe in which order to bind:
   *  - home(term1) below home(term2) ``term1 is more local''
   *  - never bind the strange guys (see var_base.hh)
   */

  {
    OzVariable *v1 = tagged2Var(term1);
    OzVariable *v2 = tagged2Var(term2);
    Board* tb1 = v1->getBoardInternal()->derefBoard();
    Board* tb2 = v2->getBoardInternal()->derefBoard();
    DebugCode(term1 = (OZ_Term) -1;);
    DebugCode(term2 = (OZ_Term) -1;);

    if (tb1 == tb2) {
      if (CMPVAR(v1, v2) < 0) {
	Swap(termPtr1, termPtr2, TaggedRef*);
	v1 = v2;
	tb2 = tb1; DebugCode(tb1 = (Board *) -1);
      }
    } else if (oz_isBelow(tb2, tb1)) {            // strictly above;
      Swap(termPtr1, termPtr2, TaggedRef*);
      v1 = v2;
      tb2 = tb1; DebugCode(tb1 = (Board *) -1);
    } else {
      Assert(tb1 != tb2 && oz_isBelow(tb1, tb2)); // strictly below;
      DebugCode(tb1 = (Board *) -1);
    }
    // v1 might have been overwritten by v2

    //
    // Make the variable to which the binding is done fit!
    int res = oz_var_cast(termPtr2, tb2, v1->getType());
    // *termPtr2 can be changed!

    if (res != PROCEED) {
      result = res;
      goto fail;
    }
    
    res = oz_var_unify(tagged2Var(*termPtr1), termPtr1, termPtr2);
    
    if (res == PROCEED)
      goto next;
    if (res == SUSPEND) {
      suspendUnify(makeTaggedRef(termPtr1), makeTaggedRef(termPtr2));
      goto next;
    }
    result = res;
    goto fail;
  }

 /*************/
 nonvar_nonvar:

  tag1 = tagged2stag(term1);
  tag2 = tagged2stag(term2);

  if (tag1 != tag2)
    goto fail;

  switch ( tag1 ) {

  case STAG_LTUPLE:
    {
      LTuple *lt1 = tagged2LTuple(term1);
      LTuple *lt2 = tagged2LTuple(term2);

      rebind(termPtr2,term1);
      argSize = 2;
      termPtr1 = lt1->getRef();
      termPtr2 = lt2->getRef();
      goto push;
    }

  case STAG_SRECORD:
    {
      SRecord *sr1 = tagged2SRecord(term1);
      SRecord *sr2 = tagged2SRecord(term2);

      if (! sr1->compareFunctor(sr2))
	goto fail;

      rebind(termPtr2,term1);
      argSize  = sr1->getWidth();
      termPtr1 = sr1->getRef();
      termPtr2 = sr2->getRef();
      goto push;
    }

  case STAG_CONST:
    switch (tagged2Const(term1)->getType()) {
    case Co_Extension:
      {
	int res = tagged2Extension(term1)->eqV(term2);
	if (res == PROCEED)
	  goto next;
	result = res;
	goto fail;
      }

    case Co_Float:
      if (floatEq(term1,term2))
	goto next;
      break;
    case Co_FSetValue:
      if (((FSetValue *) tagged2FSetValue(term1))->unify(term2))
	goto next;
      break;
    case Co_BigInt:
      if (oz_isBigInt(term2) && bigIntEq(term1,term2))
	goto next;
      break;
    default:
      break;
    }
    goto fail;

  case STAG_TOKEN:
    Assert(term1 != term2);
    /* fall through */
  default:
    goto fail;
  }


 /*************/

next:
  if (unifyStack.isMark()) {
    result = PROCEED;
    goto exit;
  }

  termPtr2 = (TaggedRef*) unifyStack.pop();
  termPtr1 = (TaggedRef*) unifyStack.pop();
  argSize  = ToInt32(unifyStack.pop());
  // fall through

push:
  if (argSize>1) {
    unifyStack.ensureFree(3);
    unifyStack.push(ToPointer(argSize-1),NO);
    unifyStack.push(termPtr1+1,NO);
    unifyStack.push(termPtr2+1,NO);
  }
  term1=tagged2NonVariable(termPtr1);
  term2=tagged2NonVariable(termPtr2);
  goto loop;

fail:
  Assert(result!=PROCEED);
  while (!unifyStack.isMark()) {
    unifyStack.pop();
  }
  suspendTrail.mkEmpty();
  am.emptySuspendVarList();     // emulator invariant
  // fall through

exit:
  Assert(unifyStack.isMark());
  unifyStack.pop(); // pop mark

  // Temporary bindings must be restored if unify is
  // used for equality tests. Erik.
  // In a space, the bindings must either be undone or transferred
  // to the global trail to end up in the script.  For simplicity
  // we choose to undo them -- Denys&Christian
  // raph: Undo temporary bindings if suspensions are pending.
  if (result!=PROCEED || !oz_onToplevel() || am.inEqEq() ||
      !suspendTrail.isEmpty())
    while (!rebindTrail.isEmpty ()) {
      PopRebindTrail(value,refPtr);
    // kost@ : no need to restore temporary bindings if terms were
    //         successfully unified. Moreover, they should not be
    //         restored: that compactifies store and speeds up
    //         subsequent unifications! 
    //         The credit for this optimization goes to Per (Brand).
      doBind(refPtr, value);
    }
  else
    // Also, in case the rebindTrail can be ignored, it is not
    // necessary to pop each item: we can simply make the trail
    // empty directly -- Denys
    rebindTrail.mkEmpty();

  // handle pending suspensions
  if (!suspendTrail.isEmpty()) {
    // make two terms with the suspended bindings
    int n = suspendTrail.getUsed() / 2;
    if (n == 1) {
      term1 = popSuspended();
      term2 = popSuspended();
    } else {
      SRecord* rec1 = SRecord::newSRecord(AtomPair, n);
      SRecord* rec2 = SRecord::newSRecord(AtomPair, n);
      while (n > 0) {
	n--;
	rec1->setArg(n, popSuspended());
	rec2->setArg(n, popSuspended());
      }
      term1 = makeTaggedSRecord(rec1);
      term2 = makeTaggedSRecord(rec2);
    }

    // unification is reduced to "term1 = term2"
    am.prepareCall(BI_Unify, RefsArray::make(term1, term2));

    // the following ensures that suspension occurs after replacement
    if (am.isEmptySuspendVarList())
      am.addSuspendVarListInline(oz_newSimpleVar(oz_currentBoard()));

    return BI_REPLACEBICALL;
  }

  Assert(suspendTrail.isEmpty());

  return (result);
}


