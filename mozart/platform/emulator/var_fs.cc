/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_fs.hh"
#endif

#include "var_fs.hh"
#include "ozostream.hh"
#include "fddebug.hh"
#include "unify.hh"

Bool OzFSVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  return (oz_isFSetValue(val) && ((FSetConstraint *) &_fset)->valid(*(FSetValue *)tagged2FSetValue(val)));
}

void OzFSVariable::dispose(void) {
  _fset.disposeExtension();

  DISPOSE_SUSPLIST(fsSuspList[fs_prop_val]);
  DISPOSE_SUSPLIST(fsSuspList[fs_prop_lub]);
  DISPOSE_SUSPLIST(fsSuspList[fs_prop_glb]);

  disposeS();
  oz_freeListDispose(this, sizeof(OzFSVariable));
}

#ifdef DEBUG_FSET
//#define DEBUG_FSUNIFY
//#define DEBUG_TELLCONSTRAINTS
#endif

OZ_Return OzFSVariable::bind(OZ_Term * vptr, OZ_Term term)
{
  DEBUG_CONSTRAIN_VAR(("bindFS "));

  Assert(!oz_isRef(term));

  if (!oz_isFSetValue(term)) {
    DEBUG_CONSTRAIN_VAR(("FAILED\n"));
    return FAILED;
  }
  if (! ((FSetConstraint *) &_fset)->
      valid(* (FSetValue *) tagged2FSetValue(term))) {
    DEBUG_CONSTRAIN_VAR(("FAILED\n"));
    return FALSE;
  }

  Bool isLocalVar = oz_isLocalVar(this);

  propagate(fs_prop_val);

  if (isLocalVar) {
    bindLocalVarToValue(vptr, term);
    dispose();
  } else {
    bindGlobalVarToValue(vptr, term);
  }

  DEBUG_CONSTRAIN_VAR(("PROCEED\n"));
  return PROCEED;
}

OZ_Return OzFSVariable::unify(OZ_Term * left_varptr, OZ_Term * right_varptr)
{
  DEBUG_CONSTRAIN_VAR(("unifyFS "));
  //
  OZ_Term right_var       = * right_varptr;
  OzVariable *right_ov = tagged2Var(right_var);
  OzFSVariable * right_fsvar    = (OzFSVariable *) right_ov;
  //
  Bool left_var_is_local  = oz_isLocalVar(this);
  Bool right_var_is_local = oz_isLocalVar(right_fsvar);
  //
  if (!left_var_is_local && right_var_is_local) {
    DEBUG_CONSTRAIN_VAR(("global-local (swapping)"));
    //
    // left variable is global and right variable is local
    //
    // swap variables to be unified and recurse
    return unify(right_varptr, left_varptr);
  }
  //
  if (right_ov->getType() != OZ_VAR_FS) {
    goto failed;
  }
  //
  {
    // compute unified set constraint ...
    OZ_FSetConstraint * right_set =
      (OZ_FSetConstraint *) &right_fsvar->getSet();
    OZ_FSetConstraint * left_set  =
      (OZ_FSetConstraint *) &getSet();
    OZ_FSetConstraint unified_set =
      ((FSetConstraint *) left_set)->unify(*(FSetConstraint *) right_set);
    // ... and check if it is valid
    if (unified_set.getCardMin() == -1) {
      goto failed;
    }
    if (left_var_is_local && right_var_is_local) {
      DEBUG_CONSTRAIN_VAR(("local-local"));
      //
      // left and right variable are local
      //
      if (unified_set.isValue()) {
	// intersection is set value
	FSetValue * unified_set_value =
	  new FSetValue(*((FSetConstraint *) &unified_set));
	OZ_Term set_value           = makeTaggedFSetValue(unified_set_value);
	// wake up
	right_fsvar->propagateUnify();
	propagateUnify();
	// bind variables to set value
	bindLocalVarToValue(left_varptr, set_value);
	bindLocalVarToValue(right_varptr, set_value);
	// dispose varibles
	dispose();
	right_fsvar->dispose();
      } else if (heapNewer(right_varptr, left_varptr)) {
	// bind left_var to right_var
	right_fsvar->setSet(unified_set);
	// wake up
	propagateUnify();
	right_fsvar->propagateUnify();
	// move remaining suspensions to right variable
	relinkSuspListTo(right_fsvar);
	// bind left to right variable
	bindLocalVar(left_varptr, right_varptr);
	// dispose left variable
	dispose();
      } else {
	// bind right to left variable
	setSet(unified_set);
	// wake up
	propagateUnify();
	right_fsvar->propagateUnify();
	// move remaining suspensions to left variable
	right_fsvar->relinkSuspListTo(this);
	// bind right to left variable
	bindLocalVar(right_varptr, left_varptr);
	// dispose right variable
	right_fsvar->dispose();
      }
    } else if (left_var_is_local && !right_var_is_local ) {
      DEBUG_CONSTRAIN_VAR(("local-global"));
      //
      // left variable is local and right variable is global
      //
      if (unified_set.isValue()) {
	FSetValue * unified_set_value =
	  new FSetValue(*((FSetConstraint *) &unified_set));
	OZ_Term set_value = makeTaggedFSetValue(unified_set_value);
	// wake up
	right_fsvar->propagateUnify();
	propagateUnify();
	// bind variables to set value
	bindLocalVarToValue(left_varptr, set_value);
	bindGlobalVarToValue(right_varptr, set_value);
	// dispose left variable
	dispose();
      } else {
	// wake up and constrain right variable
	right_fsvar->propagateUnify();
	if (((FSetConstraint *) right_set)
	    ->isWeakerThan(*((FSetConstraint *) &unified_set))) {
	  constrainGlobalVar(right_varptr, unified_set);
	}
	// wake up left variable
	propagateUnify();
	// bind left to right variable 
	bindLocalVar(left_varptr, right_varptr);
	// dispose left variable
	dispose();
      }
    } else if (!left_var_is_local && !right_var_is_local) {
      DEBUG_CONSTRAIN_VAR(("global-global"));
      //
      // left variable and right variable are global
      //
      if (unified_set.isValue()){
	FSetValue * unified_set_value =
	  new FSetValue(*((FSetConstraint *) &unified_set));
	OZ_Term set_value = makeTaggedFSetValue(unified_set_value);
	// wake up
	propagateUnify();
	right_fsvar->propagateUnify();
	//
	bindGlobalVarToValue(left_varptr, set_value);
	bindGlobalVarToValue(right_varptr, set_value);
      } else {
	// wake up
	propagateUnify();
	right_fsvar->propagateUnify();
	//
	bindGlobalVar(left_varptr, right_varptr);
	if (((FSetConstraint *) right_set)
	    ->isWeakerThan(*((FSetConstraint *) &unified_set))) {
	  constrainGlobalVar(right_varptr, unified_set);
	}
      }
    }
  }
  //
  DEBUG_CONSTRAIN_VAR(("SUCCEEDED\n"));
  return TRUE;
  //
 failed:
  DEBUG_CONSTRAIN_VAR(("FAILED\n"));
  return FAILED;
}

OZ_Return tellBasicConstraint(OZ_Term v, OZ_FSetConstraint * fs)
{
  DEBUG_CONSTRAIN_VAR(("tellBasicConstraintFS "));
  //
  DEREF(v, vptr);
  //
  if (fs && !((FSetConstraint *) fs)->isValid()) {
    goto failed;
  }
  Assert(!oz_isRef(v));
  if (oz_isFree(v)) {
    //
    // tell a finite set constraint to an unconstrained variable
    //
    if (! fs) {
      goto fsvariable;
    }
    // fs is a set value and hence v becomes a set value. otherwise ..
    if (fs->isValue()) {
      FSetValue * set_value  = new FSetValue(*(FSetConstraint *) fs);
      OZ_Term set_value_term = makeTaggedFSetValue(set_value);

      if (oz_isLocalVariable(v)) {
	if (!oz_isOptVar(v)) {
	  oz_checkSuspensionListProp(tagged2Var(v));
	}
	bindLocalVarToValue(vptr, set_value_term);
      } else {
	bindGlobalVarToValue(vptr, set_value_term);
      }
      goto proceed;
    }

    // .. create finite set variable
  fsvariable:
    OzFSVariable * fsv =
      fs ? new OzFSVariable(*fs, oz_currentBoard())
      : new OzFSVariable(oz_currentBoard());

    OZ_Term *  tfsv = newTaggedVar(fsv);

    if (oz_isLocalVariable(v)) {
      if (!oz_isOptVar(v)) {
	oz_checkSuspensionListProp(tagged2Var(v));
      }
      bindLocalVar(vptr, tfsv);
    } else {
      bindGlobalVar(vptr, tfsv);
    }
    //
    goto proceed;
    //
  } else if (isGenFSetVar(v)) {
    //
    // tell finite set constraint to finite set variable
    //
    if (! fs) {
      goto proceed;
    }
    OzFSVariable * fsvar  = tagged2GenFSetVar(v);
    OZ_FSetConstraint set =
      ((FSetConstraint *) ((OZ_FSetConstraint *) &fsvar->getSet()))->
      unify(* (FSetConstraint *) fs);
    Board * fsvarhome = fsvar->getBoardInternal();

    if (!((FSetConstraint *) &set)->isValid()) {
      goto failed;
    }
    if (!((FSetConstraint *) &fsvar->getSet())
	->isWeakerThan(*((FSetConstraint *) &set))) {
      goto proceed;
    }
    if (set.isValue()) {
      // 
      // set value
      //
      if (oz_isLocalVar(fsvar)) {
	fsvar->getSet() = set;
	fsvar->becomesFSetValueAndPropagate(vptr);
      } else {
	FSetValue * set_value  = new FSetValue(*(FSetConstraint *) &set);
	OZ_Term set_value_term = makeTaggedFSetValue(set_value);
	//
	fsvar->propagate(fs_prop_val);
	bindGlobalVarToValue(vptr, set_value_term);
      }
    } else {
      //
      // set constraint
      //
      fsvar->propagate(fs_prop_bounds);
      if (oz_isLocalVar(fsvar)) {
	fsvar->getSet() = set;
      } else {
	constrainGlobalVar(vptr, set);
      }
    }
    goto proceed;
  } else if (oz_isFSetValue(v)) {
    //
    // tell a finite set constraint to a finite set value
    //
    if (!fs) {
      goto proceed;
    }
    if (((FSetConstraint *) fs)->valid(*(FSetValue *) tagged2FSetValue(v))) {
      goto proceed;
    }
    goto failed;
  } else if (oz_isVarOrRef(v)) {
    //
    // future stuff, no idea what is going on here
    TaggedRef newVar = oz_newVariable();
    OZ_Return ret = tellBasicConstraint(newVar, fs);
    Assert(ret == PROCEED);
    return oz_unify(makeTaggedRef(vptr), newVar);
  }

failed:
  DEBUG_CONSTRAIN_VAR(("FAILED\n"));
  return FAILED;

proceed:
  DEBUG_CONSTRAIN_VAR(("PROCEED\n"));
  return PROCEED;
}

// inline DISABLED CS
void OzFSVariable::propagate(OZ_FSetPropState state,
			     PropCaller prop_eq)
{
  if (prop_eq == pc_propagator) {
    switch (state) {
    case fs_prop_val:
      {
	for (int i = fs_prop_any; i--; )
	  if (fsSuspList[i])
	    OzVariable::propagateLocal(fsSuspList[i], prop_eq);
      }
      break;
    case fs_prop_lub: case fs_prop_glb:
      if (fsSuspList[state])
	OzVariable::propagateLocal(fsSuspList[state], prop_eq);
      break;
    case fs_prop_bounds:
      if (fsSuspList[fs_prop_lub])
	OzVariable::propagateLocal(fsSuspList[fs_prop_lub], prop_eq);
      if (fsSuspList[fs_prop_glb])
	OzVariable::propagateLocal(fsSuspList[fs_prop_glb], prop_eq);
      break;
    default:
      break;
    }
  } else {
    for (int i = fs_prop_any; i--; )
      if (fsSuspList[i])
	OzVariable::propagateLocal(fsSuspList[i], prop_eq);
  }
  if (suspList)
    OzVariable::propagate(suspList, prop_eq);
}

/*
 * Trailing support
 *
 */

OzVariable * OzFSVariable::copyForTrail(void) {
  return new OzFSVariable(_fset, oz_currentBoard());
}

void OzFSVariable::restoreFromCopy(OzFSVariable * c) {
  OZ_FSetConstraint tmp = _fset;
  _fset = c->_fset;
  c->_fset = tmp;
  tmp.disposeExtension();
}

#if defined(OUTLINE)
#define inline
#include "var_fs.icc"
#undef inline
#endif
