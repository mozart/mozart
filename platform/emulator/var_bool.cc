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
#pragma implementation "var_bool.hh"
#endif


#include "var_bool.hh"
#include "var_fd.hh"
#include "unify.hh"

OZ_Return OzBoolVariable::bind(OZ_Term * vPtr, OZ_Term term)
{
  DEBUG_CONSTRAIN_VAR(("bindBool "));

  Assert(!oz_isRef(term));

  if (!oz_isSmallInt(term)) {
    DEBUG_CONSTRAIN_VAR(("FAILED\n"));
    return FAILED;
  }
  int term_val = tagged2SmallInt(term);
  if (term_val < 0 || 1 < term_val) {
    DEBUG_CONSTRAIN_VAR(("FAILED\n"));
    return FAILED;
  }

  Bool isLocalVar = oz_isLocalVar(this);

  propagate();

  if (isLocalVar) {
    bindLocalVarToValue(vPtr, term);
    dispose();
  } else {
    bindGlobalVarToValue(vPtr, term);
  }

  DEBUG_CONSTRAIN_VAR(("PROCEED\n"));
  return PROCEED;
}

// unify expects either two OzFDVariables or at least one
// OzFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
OZ_Return OzBoolVariable::unify(OZ_Term  * left_varptr, OZ_Term * right_varptr)
{
  DEBUG_CONSTRAIN_VAR(("unifyBool "));

  OZ_Term right_var       = *right_varptr;
  OzVariable *right_ov = tagged2Var(right_var);
  
  // left variable is a boolean variable. index unification on type of
  // right variable (boolean and finite domain are allowed)
  TypeOfVariable right_var_type = right_ov->getType();

  if (right_var_type == OZ_VAR_BOOL) {
    //
    // unify two boolean variables
    //
    OzBoolVariable * right_boolvar = (OzBoolVariable *) right_ov;
    Bool left_var_is_local  = oz_isLocalVar(this);
    Bool right_var_is_local = oz_isLocalVar(right_boolvar);

    if (left_var_is_local && right_var_is_local) {
      //
      // left and right variable are local
      //
      if (heapNewer(left_varptr, right_varptr)) {
	// bind left var to right var
	propagateUnify();
	right_boolvar->propagateUnify();
	relinkSuspListTo(right_boolvar);
	bindLocalVar(left_varptr, right_varptr);
	dispose();
      } else {
	// bind right var to left var
	right_boolvar->propagateUnify();
	propagateUnify();
	right_boolvar->relinkSuspListTo(this);
	bindLocalVar(right_varptr, left_varptr);
	right_boolvar->dispose();
      }
    } else if (left_var_is_local && !right_var_is_local) {
      //
      // left variable is local and right variable is global
      //
      right_boolvar->propagateUnify();
      propagateUnify();
      relinkSuspListTo(right_boolvar, TRUE);
      bindLocalVar(left_varptr, right_varptr);
      dispose();
    } else if (!left_var_is_local && right_var_is_local) {
      //
      // left variable is global and right variable is local
      right_boolvar->propagateUnify();
      propagateUnify();
      right_boolvar->relinkSuspListTo(this, TRUE);
      bindLocalVar(right_varptr, left_varptr);
      right_boolvar->dispose();
    }  else {
      //
      // left and right variable are global
      //
      Assert(!left_var_is_local && !right_var_is_local);
      //
      propagateUnify();
      right_boolvar->propagateUnify();
      //
      bindGlobalVar(left_varptr, right_varptr);
    }
  } else if (right_var_type == OZ_VAR_FD) {
    //
    // unify a boolean and a proper finite domain variable
    //
    OzFDVariable * right_fdvar = (OzFDVariable *) right_ov;
    int intersection = right_fdvar->intersectWithBool();

    if (intersection == -2) {
      goto failed;
    }

    Bool left_var_is_local  = oz_isLocalVar(this);
    Bool right_var_is_local = oz_isLocalVar(right_fdvar);

    if (left_var_is_local && right_var_is_local) {
      // 
      // left and right variable are local
      //
      if (intersection != -1) {
	// intersection is singleton
	OZ_Term int_var = makeTaggedSmallInt(intersection);
	right_fdvar->propagate(fd_prop_singl, pc_cv_unif);
	propagateUnify();
	bindLocalVarToValue(left_varptr, int_var);
	bindLocalVarToValue(right_varptr, int_var);
	dispose();
	right_fdvar->dispose();
      } else if (heapNewer(left_varptr, right_varptr)) {
	// intersection is boolean domain

	// bind left variable to right variable
	propagateUnify();
	right_fdvar->propagate(fd_prop_bounds, pc_cv_unif);
	right_fdvar->becomesBool();
	relinkSuspListTo(right_fdvar);
	bindLocalVar(left_varptr, right_varptr);
	dispose();
      } else {
	// bind right variable to left variable
	right_fdvar->propagate(fd_prop_bounds, pc_cv_unif);
	propagateUnify();
	right_fdvar->relinkSuspListTo(this);
	bindLocalVar(right_varptr, left_varptr);
	right_fdvar->dispose();
      }
    } else if (left_var_is_local && !right_var_is_local) {
      //
      // left variable is local and right variable is global
      //
      if (intersection != -1) {
	// intersection has singleton domain
	OZ_Term int_val = makeTaggedSmallInt(intersection);
	right_fdvar->propagate(fd_prop_singl, pc_cv_unif);
	propagate(pc_cv_unif);
	bindLocalVarToValue(left_varptr, int_val);
	bindGlobalVarToValue(right_varptr, int_val);
	dispose();
      } else {
	// intersection has boolean domain
	right_fdvar->propagate(fd_prop_bounds, pc_cv_unif);
	propagateUnify();
	Board * rightvarhome = right_fdvar->getBoardInternal();
	OzBoolVariable * right_boolvar = new OzBoolVariable(rightvarhome);
	OZ_Term * right_varptr_bool = newTaggedVar(right_boolvar);
	castGlobalVar(right_varptr, right_varptr_bool);
	bindLocalVar(left_varptr, right_varptr_bool);
	dispose();
      }
    } else if (!left_var_is_local && right_var_is_local) {
      //
      // left variable is global and right variable is local
      //
      if(intersection != -1) {
	// intersection is singleton
	OZ_Term int_val = makeTaggedSmallInt(intersection);
	propagateUnify();
	right_fdvar->propagate(fd_prop_singl, pc_cv_unif);
	bindLocalVarToValue(right_varptr, int_val);
	bindGlobalVarToValue(left_varptr, int_val);
	right_fdvar->dispose();
      } else {
	right_fdvar->propagate(fd_prop_bounds, pc_cv_unif);
	propagateUnify();
	right_fdvar->relinkSuspListTo(this, TRUE);
	bindLocalVar(right_varptr, left_varptr);
	right_fdvar->dispose();
      }
    } else {
      //
      // left and right variable are global
      //
      Assert(!left_var_is_local && !right_var_is_local);
      //
      if (intersection != -1) {
	// intersection is singleton
	OZ_Term int_val = makeTaggedSmallInt(intersection);
	//
	propagateUnify();
	right_fdvar->propagate(fd_prop_singl, pc_cv_unif);
	//
	bindGlobalVarToValue(left_varptr, int_val);
	bindGlobalVarToValue(right_varptr, int_val);
      } else {
	// intersection is boolean domain
	propagateUnify();
	right_fdvar->propagate(fd_prop_bounds, pc_cv_unif);
	// tmueller: left variable is more local
	Board * rightvarhome = right_fdvar->getBoardInternal();
	OzBoolVariable * right_boolvar = new OzBoolVariable(rightvarhome);
	OZ_Term * right_varptr_bool = newTaggedVar(right_boolvar);
	castGlobalVar(right_varptr, right_varptr_bool);
	bindGlobalVar(left_varptr, right_varptr_bool);
      }
    }
  }
  DEBUG_CONSTRAIN_VAR(("PROCEED\n"));
  return PROCEED;

 failed:
  DEBUG_CONSTRAIN_VAR(("FAILED\n"));
  return FALSE;
} // OzBoolVariable::unify

Bool OzBoolVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  if (oz_isSmallInt(val)) {
    int intval = tagged2SmallInt(val);
    return (intval == 0 || intval == 1);
  }
  return FALSE;
}

OZ_Return tellBasicBoolConstraint(OZ_Term t)
{
  OZ_FiniteDomain bool_dom(fd_bool);
  return tellBasicConstraint(t, &bool_dom);
}

#ifdef OUTLINE
#define inline
#include "var_bool.icc"
#undef inline
#endif
