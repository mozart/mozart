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

#ifdef TMUELLER
//-----------------------------------------------------------------------------
OZ_Return OzBoolVariable::bind(TaggedRef * vPtr, TaggedRef term)
{
  Assert(!oz_isRef(term));
  if (!oz_isSmallInt(term)) return FAILED;

  int term_val = smallIntValue(term);
  if (term_val < 0 || 1 < term_val) {
    return FAILED;
  }

  Bool isLocalVar = oz_isLocalVar(this);
  Bool isNotInstallingScript = !am.isInstallingScript();

#ifdef SCRIPTDEBUG
  printf("bool-int %s\n", isLocalVar ? "local" : "global"); fflush(stdout);
#endif

  if (!am.inEqEq() && (isNotInstallingScript || isLocalVar)) propagate();

  if (oz_isLocalVar(this)) {
    DoBind(vPtr, term);
    dispose();
  } else {
    DoBindAndTrail(vPtr, term);
  }

  return PROCEED;
}
//-----------------------------------------------------------------------------
#else
OZ_Return OzBoolVariable::bind(OZ_Term * vPtr, OZ_Term term)
{
  Assert(!oz_isRef(term));

  if (!oz_isSmallInt(term)) {
    return FAILED;
  }
  int term_val = smallIntValue(term);
  if (term_val < 0 || 1 < term_val) {
    return FAILED;
  }

  Bool isLocalVar = oz_isLocalVar(this);

  if (!am.inEqEq() && isLocalVar) {
    propagate();
  }
  if (oz_isLocalVar(this)) {
    bindLocalVarToValue(vPtr, term);
    dispose();
  } else {
    bindGlobalVarToValue(vPtr, term);
  }

  return PROCEED;
}
#endif

// unify expects either two OzFDVariables or at least one
// OzFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
#ifdef TMUELLER
OZ_Return OzBoolVariable::unify(OZ_Term  * left_varptr, OZ_Term * right_varptr)
{
  OZ_Term right_var       = *right_varptr;
  OzVariable * right_cvar = tagged2CVar(right_var);

  // left variable is a boolean variable. index unification on type of
  // right variable (boolean and finite domain are allowed)
  TypeOfVariable right_cvar_type = right_cvar->getType();

  if (right_cvar_type == OZ_VAR_BOOL) {
    //
    // unify two boolean variables
    //
    OzBoolVariable * right_boolvar = (OzBoolVariable *) right_cvar;
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
      if (!am.inEqEq()) {
        propagateUnify();
        right_boolvar->propagateUnify();
      }
      bindGlobalVar(left_varptr, right_varptr);
    }
  } else if (right_cvar_type == OZ_VAR_FD) {
    //
    // unify a boolean and a proper finite domain variable
    //
    OzFDVariable * right_fdvar = (OzFDVariable *) right_cvar;
    int intersection = right_fdvar->intersectWithBool();

    if (intersection == -2) {
      return FAILED;
    }

    Bool right_var_is_constrained = 1;

    Bool left_var_is_local  = oz_isLocalVar(this);
    Bool right_var_is_local = oz_isLocalVar(right_fdvar);

    if (left_var_is_local && right_var_is_local) {
      //
      // left and right variable are local
      //
      if (intersection != -1) {
        // intersection is singleton
        OZ_Term int_var = newSmallInt(intersection);
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
        OZ_Term int_val = newSmallInt(intersection);
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
        OZ_Term * right_varptr_bool = newTaggedCVar(right_boolvar);
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
        OZ_Term int_val = newSmallInt(intersection);
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
        OZ_Term int_val = newSmallInt(intersection);
        if (!am.inEqEq()) {
          propagateUnify();
          right_fdvar->propagate(fd_prop_singl, pc_cv_unif);
        }
        bindGlobalVarToValue(left_varptr, int_val);
        bindGlobalVarToValue(right_varptr, int_val);
      } else {
        // intersection is boolean domain
        if (!am.inEqEq()) {
          propagateUnify();
          right_fdvar->propagate(fd_prop_bounds, pc_cv_unif);
        }
        // tmueller: left variable is more local
        Board * rightvarhome = right_fdvar->getBoardInternal();
        OzBoolVariable * right_boolvar = new OzBoolVariable(rightvarhome);
        OZ_Term * right_varptr_bool = newTaggedCVar(right_boolvar);
        castGlobalVar(right_varptr, right_varptr_bool);
        bindGlobalVar(left_varptr, right_varptr_bool);
      }
    }
  }
  return FALSE;
} // OzBoolVariable::unify
//-----------------------------------------------------------------------------
#else
OZ_Return OzBoolVariable::unify(TaggedRef * vPtr, TaggedRef *tPtr)
{
#ifdef SCRIPTDEBUG
  printf(am.isInstallingScript()
         ? "bool installing script\n"
         : "bool NOT installing script\n");
  fflush(stdout);
#endif

  TaggedRef term = *tPtr;
  OzVariable *cv=tagged2CVar(term);
  switch (cv->getType()) {
  case OZ_VAR_BOOL:
    {
      Bool isConstrained = ! am.isInstallingScript();
      OzBoolVariable * termvar = (OzBoolVariable *)cv;

      Bool varIsLocal =  oz_isLocalVar(this);
      Bool termIsLocal = oz_isLocalVar(termvar);

      switch (varIsLocal + 2 * termIsLocal) {
      case TRUE + 2 * TRUE: // var and term are local
        {
#ifdef SCRIPTDEBUG
          printf("bool-bool local local\n"); fflush(stdout);
#endif

          if (heapNewer(vPtr, tPtr)) { // bind var to term
            propagate(pc_cv_unif);
            termvar->propagate(pc_cv_unif);
            relinkSuspListTo(termvar);
            DoBind(vPtr, makeTaggedRef(tPtr));
            dispose();
          } else { // bind term to var
            termvar->propagate(pc_cv_unif);
            propagate(pc_cv_unif);
            termvar->relinkSuspListTo(this);
            DoBind(tPtr, makeTaggedRef(vPtr));
            termvar->dispose();
          }
          break;
        }

      case TRUE + 2 * FALSE: // var is local and term is global
        {
#ifdef SCRIPTDEBUG
          printf("bool-bool local global\n"); fflush(stdout);
#endif

          if (isConstrained) {
            termvar->propagate(pc_cv_unif);
            propagate(pc_cv_unif);
          }
          relinkSuspListTo(termvar, TRUE);
          DoBind(vPtr, makeTaggedRef(tPtr));
          dispose();
          break;
        }

      case FALSE + 2 * TRUE: // var is global and term is local
        {
#ifdef SCRIPTDEBUG
          printf("bool-bool global local\n"); fflush(stdout);
#endif
          if (isConstrained) {
            termvar->propagate(pc_cv_unif);
            propagate(pc_cv_unif);
          }
          termvar->relinkSuspListTo(this, TRUE);
          DoBind(tPtr, makeTaggedRef(vPtr));
          termvar->dispose();
          break;
        }

      case FALSE + 2 * FALSE: // var and term is global
        {
#ifdef SCRIPTDEBUG
          printf("bool-bool global global\n"); fflush(stdout);
#endif

          OzBoolVariable * bool_var
            = new OzBoolVariable(oz_currentBoard());
          TaggedRef * var_val = newTaggedCVar(bool_var);

          if (!am.inEqEq()) {
            propagate(pc_cv_unif);
            termvar->propagate(pc_cv_unif);
          }
          DoBindAndTrailAndIP(vPtr, makeTaggedRef(var_val),
                              bool_var, this);
          DoBindAndTrailAndIP(tPtr, makeTaggedRef(var_val),
                              bool_var, termvar);
          break;
        }

      default:
        OZ_error("unexpected case in unifyBool Bool <--> Bool");
        break;
      } // switch
      return TRUE;
    }
  case OZ_VAR_FD:
    {
      OzFDVariable * termvar = (OzFDVariable *)cv;

      int intsct = termvar->intersectWithBool();

      if (intsct == -2) return FAILED;

      Bool isNotInstallingScript = ! am.isInstallingScript();
      Bool isConstrainedVar = isNotInstallingScript || (intsct != -1);
      Bool isConstrainedTerm = isNotInstallingScript;

      Bool varIsLocal =  oz_isLocalVar(this);
      Bool termIsLocal = oz_isLocalVar(termvar);

      switch (varIsLocal + 2 * termIsLocal) {
      case TRUE + 2 * TRUE: // var and term are local
        {
#ifdef SCRIPTDEBUG
          printf("bool-fd local local\n"); fflush(stdout);
#endif

          if (intsct != -1) {
            TaggedRef int_var = newSmallInt(intsct);
            termvar->propagate(fd_prop_singl, pc_cv_unif);
            propagate(pc_cv_unif);
            DoBind(vPtr, int_var);
            DoBind(tPtr, int_var);
            dispose(); termvar->dispose();
          } else if (heapNewer(vPtr, tPtr)) { // bind var to term
            propagate(pc_cv_unif);
            termvar->propagate(fd_prop_bounds, pc_cv_unif);
            termvar->becomesBool();
            relinkSuspListTo(termvar);
            DoBind(vPtr, makeTaggedRef(tPtr));
            dispose();
          } else { // bind term to var
            termvar->propagate(fd_prop_bounds, pc_cv_unif);
            propagate(pc_cv_unif);
            termvar->relinkSuspListTo(this);
            DoBind(tPtr, makeTaggedRef(vPtr));
            termvar->dispose();
          }
          break;
        }

      case TRUE + 2 * FALSE: // var is local and term is global
        {
#ifdef SCRIPTDEBUG
          printf("bool-fd local global\n"); fflush(stdout);
#endif
          if (intsct != -1) {
            TaggedRef int_var = newSmallInt(intsct);
            if (isNotInstallingScript)
              termvar->propagate(fd_prop_singl, pc_cv_unif);
            if (isConstrainedVar) propagate(pc_cv_unif);
            DoBind(vPtr, int_var);
            DoBindAndTrail(tPtr, int_var);
            dispose();
          } else {
            if (isNotInstallingScript)
              termvar->propagate(fd_prop_bounds, pc_cv_unif);
            if (isConstrainedVar) propagate(pc_cv_unif);
            DoBindAndTrailAndIP(tPtr, makeTaggedRef(vPtr),
                                this, termvar);
          }
          break;
        }

      case FALSE + 2 * TRUE: // var is global and term is local
        {
#ifdef SCRIPTDEBUG
          printf("bool-fd global local\n"); fflush(stdout);
#endif

          if(intsct != -1) {
            TaggedRef int_term = newSmallInt(intsct);
            if (isNotInstallingScript) propagate(pc_cv_unif);
            if (isConstrainedTerm)
              termvar->propagate(fd_prop_singl, pc_cv_unif);
            DoBind(tPtr, int_term);
            DoBindAndTrail(vPtr, int_term);
            termvar->dispose();
          } else {
            if (isConstrainedTerm)
              termvar->propagate(fd_prop_bounds, pc_cv_unif);
            if (isNotInstallingScript) propagate(pc_cv_unif);
            termvar->relinkSuspListTo(this, TRUE);
            DoBind(tPtr, makeTaggedRef(vPtr));
            termvar->dispose();
          }
          break;
        }

      case FALSE + 2 * FALSE: // var and term is global
        {
#ifdef SCRIPTDEBUG
          printf("bool-fd global global\n"); fflush(stdout);
#endif

          if (intsct != -1){
            TaggedRef int_val = newSmallInt(intsct);
            if (!am.inEqEq()) {
              propagate(pc_cv_unif);
              termvar->propagate(fd_prop_singl, pc_cv_unif);
            }
            DoBindAndTrail(vPtr, int_val);
            DoBindAndTrail(tPtr, int_val);
          } else {
            OzBoolVariable * bool_var
              = new OzBoolVariable(oz_currentBoard());
            TaggedRef * var_val = newTaggedCVar(bool_var);
            if (!am.inEqEq()) {
              propagate(pc_cv_unif);
              termvar->propagate(fd_prop_bounds, pc_cv_unif);
            }
            DoBindAndTrailAndIP(vPtr, makeTaggedRef(var_val),
                                bool_var, this);
            DoBindAndTrailAndIP(tPtr, makeTaggedRef(var_val),
                                bool_var, termvar);
          }
          break;
        }
      default:
        OZ_error("unexpected case in unifyBool Bool <--> FD");
        break;
      } // switch
    }
    return TRUE;
  default:
    break;
  }

  return FALSE;
} // OzBoolVariable::unify
#endif

Bool OzBoolVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  if (oz_isSmallInt(val)) {
    int intval = smallIntValue(val);
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
