/*
 *  Authors:
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Tobias Müller, 1999
 *    Christian Schulte, 1999
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
#pragma implementation "var_ct.hh"
#endif

#include "var_ct.hh"
#include "am.hh"
#include "unify.hh"

OZ_Return OzCtVariable::bind(OZ_Term * vptr, OZ_Term term)
{
  DEBUG_CONSTRAIN_VAR(("bindCT "));

  Assert(!oz_isRef(term));

  // bind temporarily to avoid looping in unification on cyclic terms
  OZ_Term trail = *vptr;
  *vptr = term;

  OZ_Boolean result_unify = _constraint->isInDomain(term);

  // undo binding
  *vptr = trail;

  if (! result_unify) {
    DEBUG_CONSTRAIN_VAR(("FAILED\n"));
    return FAILED;
  }
  Bool isLocalVar = oz_isLocalVar(this);
  //
  propagateUnify();
  //
  if (isLocalVar) {
    bindLocalVarToValue(vptr, term);
    dispose();
  } else {
    bindGlobalVarToValue(vptr, term);
  }
  //
  DEBUG_CONSTRAIN_VAR(("PROCEED\n"));
  return PROCEED;
}

// Unification is implemented such, that `OzCtVariable's of the same
// kind are compatible with each other.
OZ_Return OzCtVariable::unify(OZ_Term * left_varptr, OZ_Term * right_varptr)
{
  DEBUG_CONSTRAIN_VAR(("unifyCT "));

  OZ_Term right_var       = *right_varptr;
  OzVariable *right_ov = tagged2Var(right_var);
  //
  OzCtVariable * right_ctvar = (OzCtVariable *) right_ov;
  Bool left_var_is_local  = oz_isLocalVar(this);
  Bool right_var_is_local = oz_isLocalVar(right_ctvar);
  //
  if (!left_var_is_local && right_var_is_local) {
    DEBUG_CONSTRAIN_VAR(("global-local (swapping)"));
    //
    // left variable is global and right variable is local
    //
    // swap variables to be unified and recurse
    return unify(right_varptr, left_varptr);
  }
  if (right_ov->getType() != OZ_VAR_CT) {
    DEBUG_CONSTRAIN_VAR(("expected ct-var on right hand-side\n"));
    goto failed;
  }
  {
    OZ_Ct * right_constr       = right_ctvar->getConstraint();
    OZ_Ct * left_constr        = getConstraint();

    // bind temporarily to avoid looping in unification on cyclic terms
    OZ_Term trail = * left_varptr;
    *left_varptr = right_var;
    OZ_Ct * unified_constr = left_constr->intersectDomains(right_constr);

    // undo binding
    *left_varptr = trail;

    if (unified_constr->isEmpty()) {
      goto failed;
    }
    if (left_var_is_local && right_var_is_local) {
      DEBUG_CONSTRAIN_VAR(("local-local"));
      //
      // left and right variable are local
      //
      if (unified_constr->isValue()) {
	// unified_constr is a value
	OZ_Term unified_value = unified_constr->toValue();
	// wake up
	right_ctvar->propagateUnify();
	propagateUnify();
	//
	bindLocalVarToValue(left_varptr, unified_value);
	bindLocalVarToValue(right_varptr, unified_value);
	// dispose variables
	dispose();
	right_ctvar->dispose();
      } else if (heapNewer(left_varptr, right_varptr)) {
	// bind left variable to right variable
	right_ctvar->copyConstraint(unified_constr);
	// wake up
	propagateUnify();
	right_ctvar->propagateUnify();
	//
	relinkSuspListTo(right_ctvar);
	bindLocalVar(left_varptr, right_varptr);
	// dispose left variable
	dispose();
      } else {
	// bind right variable to left variable
	copyConstraint(unified_constr);
	// wake up
	right_ctvar->propagateUnify();
	propagateUnify();
	//
	right_ctvar->relinkSuspListTo(this);
	//
	bindLocalVar(right_varptr, left_varptr);
	// dispose right variable
	right_ctvar->dispose();
      }
    } else if (left_var_is_local && !right_var_is_local) {
	DEBUG_CONSTRAIN_VAR(("local-global"));
	//
	// left variable is local and right variable is global
	//
	if (unified_constr->isValue()) {
	  // unified_constr is a value
	  OZ_Term unified_value = unified_constr->toValue();
	  // wake up
	  right_ctvar->propagateUnify();
	  propagateUnify();
	  //
	  bindLocalVarToValue(left_varptr, unified_value);
	  bindGlobalVarToValue(right_varptr, unified_value);
	  // dispose left variable
	  dispose();
	} else {
	  // unified_constr is a not a value
	  // wake up
	  right_ctvar->propagateUnify();
	  propagateUnify();
	  //
	  bindLocalVar(left_varptr, right_varptr);
	  if (right_constr->isWeakerThan(unified_constr)) {
	    constrainGlobalVar(right_varptr, unified_constr);
	  }
	  // dispose left variable
	  dispose();
	}
      } else if (!left_var_is_local && !right_var_is_local) {
	DEBUG_CONSTRAIN_VAR(("global-global"));
	//
	// left variable and right variable are global
	//
	if (unified_constr->isValue()){
	  // unified_constr is a value
	  OZ_Term unified_value = unified_constr->toValue();
	  // wake up
	  propagateUnify();
	  right_ctvar->propagateUnify();
	  //
	  bindGlobalVarToValue(left_varptr, unified_value);
	  bindGlobalVarToValue(right_varptr, unified_value);
	} else {
	  // unified_constr is not a value
	  // wake up
	  propagateUnify();
	  right_ctvar->propagateUnify();
	  //
	  bindGlobalVar(left_varptr, right_varptr);
	  if (right_constr->isWeakerThan(unified_constr)) {
	    constrainGlobalVar(right_varptr, unified_constr);
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

OZ_Return tellBasicConstraint(OZ_Term v, OZ_Ct * constr, OZ_CtDefinition * def)
{
  DEBUG_CONSTRAIN_VAR(("tellBasicConstraintCT "));
  //
  DEREF(v, vptr);
  //
  if (constr && constr->isEmpty()) {
    goto failed;
  }
  Assert(!oz_isRef(v));
  if (oz_isFree(v)) {
    //
    // tell a finite set constraint to an unconstrained variable
    //
    if (! constr) {
      goto ctvariable;
    }
    // constr is a value and hence v becomes a value. otherwise ..
    if (constr->isValue()) {
      //
      if (oz_isLocalVariable(v)) {
	if (!oz_isOptVar(v))
	  oz_checkSuspensionListProp(tagged2Var(v));
	bindLocalVarToValue(vptr, constr->toValue());
      } else {
	bindGlobalVarToValue(vptr, constr->toValue());
      }
      goto proceed;
    }
    // .. create ct variable
  ctvariable:
    OzCtVariable * ctv =
      constr
      ? new OzCtVariable(constr, def, oz_currentBoard())
      :  new OzCtVariable(def->fullDomain(), def, oz_currentBoard());
    //
    OZ_Term *  tctv = newTaggedVar(ctv);
    //
    if (oz_isLocalVariable(v)) {
      if (!oz_isOptVar(v)) {
	oz_checkSuspensionListProp(tagged2Var(v));
      }
      bindLocalVar(vptr, tctv);
    } else {
      bindGlobalVar(vptr, tctv);
    }
    //
    goto proceed;
    //
  } else if (isGenCtVar(v)) {
    //
    // tell constraint to constrained variable
    //
    if (! constr) {
      goto proceed;
    }
    OzCtVariable * ctvar           = tagged2GenCtVar(v);
    OZ_Ct * old_constr             = ctvar->getConstraint();
    OZ_CtProfile * old_constr_prof = old_constr->getProfile();
    OZ_Ct * new_constr             = old_constr->intersectDomains(constr);

    if (new_constr->isEmpty()) {
      goto failed;
    }
    if (! ctvar->getConstraint()->isWeakerThan(new_constr)) {
      goto proceed;
    }
    if (new_constr->isValue()) {
      //
      // `new_constr' designates a value
      //
      ctvar->propagate(OZ_WAKEUP_ALL, pc_propagator);

      if (oz_isLocalVar(ctvar)) {
	bindLocalVarToValue(vptr, new_constr->toValue());
      } else {
	bindGlobalVarToValue(vptr, new_constr->toValue());
      }
    } else {
      //
      // `new_constr' does not designate a value
      //
      ctvar->propagate(new_constr->computeEvents(old_constr_prof),
		       pc_propagator);

      if (oz_isLocalVar(ctvar)) {
	ctvar->copyConstraint(new_constr);
      } else {
	constrainGlobalVar(vptr, new_constr);
      }
    }
    goto proceed;
  } else if (!oz_isVarOrRef(v)) {
    //
    // tell a ct constraint to a ct variable
    //
    if (! constr) {
      goto proceed;
    }
    if (constr->isInDomain(v)) {
      goto proceed;
    }
    goto failed;
  } else {
    Assert(oz_isVar(v));
    //
    // future stuff, no idea what is going on here
    TaggedRef newVar = oz_newVariable();
    OZ_Return ret = tellBasicConstraint(newVar, constr, def);
    Assert(ret == PROCEED);
    return oz_unify(makeTaggedRef(vptr), newVar);
  }

failed:
  return FAILED;

proceed:
  return PROCEED;
}

void OzCtVariable::propagate(OZ_CtWakeUp descr, PropCaller caller)
{
  int no_of_wakup_lists = _definition->getNoEvents();

  if (caller == pc_propagator) {
    // called by propagator
    for (int i = no_of_wakup_lists; i--; )
      if (descr.isWakeUp(i) && _susp_lists[i])
	OzVariable::propagateLocal(_susp_lists[i], caller);
  } else {
    // called by unification
    for (int i = no_of_wakup_lists; i--; )
      if (_susp_lists[i])
	OzVariable::propagateLocal(_susp_lists[i], caller);
  }
  if (suspList)
    OzVariable::propagate(suspList, caller);
}

void OzCtVariable::relinkSuspListTo(OzCtVariable * lv, Bool reset_local)
{
  OzVariable::relinkSuspListTo(lv, reset_local); // any

  // Ensure locality invariant

  if (reset_local) {
    for (int i = _definition->getNoEvents(); i--; )
      _susp_lists[i] =
	_susp_lists[i]->appendToAndUnlink(lv->suspList, reset_local);
  } else {
    for (int i = _definition->getNoEvents(); i--; )
      _susp_lists[i] =
	_susp_lists[i]->appendToAndUnlink(lv->_susp_lists[i], reset_local);
  }
}

void OzCtVariable::installPropagators(OzCtVariable * glob_var)
{
  installPropagatorsG(glob_var);
  Board * gb = glob_var->getBoardInternal();
  for (int i = _definition->getNoEvents(); i--; )
    _susp_lists[i] = oz_installPropagators(_susp_lists[i],
					   glob_var->_susp_lists[i],
					   gb);
}


/*
 * Trailing support
 *
 */

OzVariable * OzCtVariable::copyForTrail(void) {
  return new OzCtVariable(getConstraint(),
			  getDefinition(),
			  oz_currentBoard());
}

void OzCtVariable::restoreFromCopy(OzCtVariable * c) {
  OZ_Ct * cc           = c->getConstraint();
  OZ_CtDefinition * cd = c->getDefinition();
  c->_definition = _definition;
  c->_constraint = _constraint;
  _definition = cd;
  _constraint = cc;
}

#ifdef OUTLINE
#define inline
#include "var_ct.icc"
#undef inline
#endif

