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
  freeListDispose(this, sizeof(OzFSVariable));
}

#ifdef DEBUG_FSET
//#define DEBUG_FSUNIFY
//#define DEBUG_TELLCONSTRAINTS
#endif

#ifdef CORRECT_UNIFY
//-----------------------------------------------------------------------------
OZ_Return OzFSVariable::bind(OZ_Term * vptr, OZ_Term term)
{
  DEBUG_CONSTRAIN_CVAR(("bindFS "));

  Assert(!oz_isRef(term));

  if (!oz_isFSetValue(term)) {
    DEBUG_CONSTRAIN_CVAR(("FAILED\n"));
    return FAILED;
  }
  if (! ((FSetConstraint *) &_fset)->
      valid(* (FSetValue *) tagged2FSetValue(term))) {
    DEBUG_CONSTRAIN_CVAR(("FAILED\n"));
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

  DEBUG_CONSTRAIN_CVAR(("PROCEED\n"));
  return PROCEED;
}
//-----------------------------------------------------------------------------
#else
OZ_Return OzFSVariable::bind(OZ_Term * vptr, OZ_Term term)
{
  Assert(!oz_isRef(term));
  if (!oz_isFSetValue(term)) return FAILED;

  if (! ((FSetConstraint *) &_fset)->valid(*(FSetValue *)tagged2FSetValue(term))) {
    return FALSE;
  }

  Bool isLocalVar = oz_isLocalVar(this);
  Bool isNotInstallingScript = !am.isInstallingScript();

  if (!am.inEqEq() && (isNotInstallingScript || isLocalVar))
    propagate(fs_prop_val);

  if (isLocalVar) {
    DoBind(vptr, term);
    dispose();
  } else {
    DoBindAndTrail(vptr, term);
  }

  return TRUE;
}
#endif /* CORRECT_UNIFY */

#ifdef CORRECT_UNIFY
//-----------------------------------------------------------------------------
OZ_Return OzFSVariable::unify(OZ_Term * left_varptr, OZ_Term * right_varptr)
{
  DEBUG_CONSTRAIN_CVAR(("unifyFS "));
  //
  OZ_Term right_var       = * right_varptr;
  OzVariable * right_cvar = tagged2CVar(right_var);
  OzFSVariable * right_fsvar    = (OzFSVariable *) right_cvar;
  //
  Bool left_var_is_local  = oz_isLocalVar(this);
  Bool right_var_is_local = oz_isLocalVar(right_fsvar);
  //
  if (!left_var_is_local && right_var_is_local) {
    DEBUG_CONSTRAIN_CVAR(("global-local (swapping)"));
    //
    // left variable is global and right variable is local
    //
    // swap variables to be unified and recurse
    return unify(right_varptr, left_varptr);
  }
  //
  if (right_cvar->getType() != OZ_VAR_FS) {
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
      DEBUG_CONSTRAIN_CVAR(("local-local"));
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
      DEBUG_CONSTRAIN_CVAR(("local-global"));
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
      DEBUG_CONSTRAIN_CVAR(("global-global"));
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
  DEBUG_CONSTRAIN_CVAR(("SUCCEEDED\n"));
  return TRUE;
  //
 failed:
  DEBUG_CONSTRAIN_CVAR(("FAILED\n"));
  return FAILED;
}
//-----------------------------------------------------------------------------
#else
OZ_Return OzFSVariable::unify(OZ_Term * vptr, OZ_Term *tptr)
{
  OZ_Term term = *tptr;
  OzVariable *cv=tagged2CVar(term);
  if (cv->getType() != OZ_VAR_FS) {
  f:
    return FALSE;
  }

  OzFSVariable * term_var = (OzFSVariable *)cv;
  OZ_FSetConstraint * t_fset = (OZ_FSetConstraint *) &term_var->getSet();
  OZ_FSetConstraint * fset = (OZ_FSetConstraint *) &getSet();
  OZ_FSetConstraint new_fset;

  new_fset = ((FSetConstraint *) t_fset)->unify(*(FSetConstraint *) fset);
  if (new_fset.getCardMin() == -1)
    goto f;

  Bool var_is_local  = oz_isLocalVar(this);
  Bool term_is_local = oz_isLocalVar(term_var);
  Bool is_not_installing_script = !am.isInstallingScript();
  Bool var_is_constrained
    = (is_not_installing_script ||
       ((FSetConstraint *) fset)
       ->isWeakerThan(*((FSetConstraint *) &new_fset)));
  Bool term_is_constrained
    = (is_not_installing_script ||
       ((FSetConstraint *) t_fset)
       ->isWeakerThan(*((FSetConstraint *) &new_fset)));


  switch (var_is_local + 2 * term_is_local) {
  case TRUE + 2 * TRUE: // var and term are local
    {
      if (new_fset.isValue()) {
	OZ_Term new_fset_var = makeTaggedFSetValue(new FSetValue(*((FSetConstraint *) &new_fset)));
	term_var->propagateUnify();
	propagateUnify();
	DoBind(vptr, new_fset_var);
	DoBind(tptr, new_fset_var);
	dispose();
	term_var->dispose();
      } else if (heapNewer(vptr, tptr)) { // bind var to term
	term_var->setSet(new_fset);
	propagateUnify();
	term_var->propagateUnify();
	relinkSuspListTo(term_var);
	DoBind(vptr, makeTaggedRef(tptr));
	dispose();
      } else { // bind term to var
	setSet(new_fset);
	term_var->propagateUnify();
	propagateUnify();
	term_var->relinkSuspListTo(this);
	DoBind(tptr, makeTaggedRef(vptr));
	term_var->dispose();
      }
      break;
    } // TRUE + 2 * TRUE:
  case TRUE + 2 * FALSE: // var is local and term is global
    {
      if (((FSetConstraint *) t_fset)
	  ->isWeakerThan(*((FSetConstraint *) &new_fset))) {
	if (new_fset.isValue()) {
	  OZ_Term new_fset_var
	    = makeTaggedFSetValue(new FSetValue(*((FSetConstraint *) &new_fset)));
	  if (is_not_installing_script) term_var->propagateUnify();
	  if (var_is_constrained) propagateUnify();
	  DoBind(vptr, new_fset_var);
	  DoBindAndTrail(tptr, new_fset_var);
	  dispose();
	} else {
	  setSet(new_fset);
	  if (is_not_installing_script) term_var->propagateUnify();
	  if (var_is_constrained) propagateUnify();
	  DoBindAndTrailAndIP(tptr, makeTaggedRef(vptr),
			      this, term_var);
	}
      } else {
	if (is_not_installing_script) term_var->propagateUnify();
	if (var_is_constrained) propagateUnify();
	relinkSuspListTo(term_var, TRUE);
	DoBind(vptr, makeTaggedRef(tptr));
	dispose();
      }
      break;
    } // TRUE + 2 * FALSE:
  case FALSE + 2 * TRUE: // var is global and term is local
    {
      if (((FSetConstraint *) fset)
	  ->isWeakerThan(*((FSetConstraint *) &new_fset))) {
	if(new_fset.isValue()) {
	  OZ_Term new_fset_var
	    = makeTaggedFSetValue(new FSetValue(*((FSetConstraint *) &new_fset)));
	  if (is_not_installing_script) propagateUnify();
	  if (term_is_constrained) term_var->propagateUnify();
	  DoBind(tptr, new_fset_var);
	  DoBindAndTrail(vptr, new_fset_var);
	  term_var->dispose();
	} else {
	  term_var->setSet(new_fset);
	  if (is_not_installing_script) propagateUnify();
	  if (term_is_constrained) term_var->propagateUnify();
	  DoBindAndTrailAndIP(vptr, makeTaggedRef(tptr),
			      term_var, this);
	}
      } else {
	if (term_is_constrained) term_var->propagateUnify();
	if (is_not_installing_script) propagateUnify();
	term_var->relinkSuspListTo(this, TRUE);
	DoBind(tptr, makeTaggedRef(vptr));
	term_var->dispose();
      }
      break;
    } // FALSE + 2 * TRUE:
  case FALSE + 2 * FALSE: // var and term is global
    {
      if (new_fset.isValue()){
	OZ_Term new_fset_var
	  = makeTaggedFSetValue(new FSetValue(*((FSetConstraint *) &new_fset)));
	if (!am.inEqEq()) {
	  if (var_is_constrained) propagateUnify();
	  if (term_is_constrained) term_var->propagateUnify();
	}
	DoBindAndTrail(vptr, new_fset_var);
	DoBindAndTrail(tptr, new_fset_var);
      } else {
	OzFSVariable *c_var
	  = new OzFSVariable(new_fset,oz_currentBoard());
	TaggedRef * var_val = newTaggedCVar(c_var);
	if (!am.inEqEq()) {
	  if (var_is_constrained) propagateUnify();
	  if (term_is_constrained) term_var->propagateUnify();
	}
	DoBindAndTrailAndIP(vptr, makeTaggedRef(var_val),
			    c_var, this);
	DoBindAndTrailAndIP(tptr, makeTaggedRef(var_val),
			    c_var, term_var);
      }
      break;
    } // FALSE + 2 * FALSE:
  default:
    OZ_error("unexpected case in unifyFSet");
    break;
  } // switch (varIsLocal + 2 * termIsLocal)

  return TRUE;
}
#endif /* CORRECT_UNIFY */

#ifdef CORRECT_UNIFY
//-----------------------------------------------------------------------------
OZ_Return tellBasicConstraint(OZ_Term v, OZ_FSetConstraint * fs)
{
  DEBUG_CONSTRAIN_CVAR(("tellBasicConstraintFS "));
  //
  DEREF(v, vptr, vtag);
  //
  if (fs && !((FSetConstraint *) fs)->isValid()) {
    goto failed;
  }
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

      if (oz_isLocalVariable(vptr)) {
	if (!isUVar(vtag)) {
	  oz_checkSuspensionListProp(tagged2SVarPlus(v));
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

    OZ_Term *  tfsv = newTaggedCVar(fsv);

    if (oz_isLocalVariable(vptr)) {
      if (!isUVar(vtag)) {
	oz_checkSuspensionListProp(tagged2SVarPlus(v));
	fsv->setSuspList(tagged2SVarPlus(v)->unlinkSuspList());
      }
      bindLocalVar(vptr, tfsv);
    } else {
      bindGlobalVar(vptr, tfsv);
    }

    goto proceed;
  } else if (isGenFSetVar(v, vtag)) {
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
    if (!((FSetConstraint *) &fsvar->getSet())->isWeakerThan(*((FSetConstraint *) &set))) {
      printf("gaga\n"); fflush(stdout);
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
  } else if (isFSetValueTag(vtag)) {
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
  } else if (oz_isVariable(v)) {
    //
    // future stuff, no idea what is going on here
    TaggedRef newVar = oz_newVariable();
    OZ_Return ret = tellBasicConstraint(newVar, fs);
    Assert(ret == PROCEED);
    return oz_unify(makeTaggedRef(vptr), newVar);
  }

failed:
  DEBUG_CONSTRAIN_CVAR(("FAILED\n"));
  return FAILED;

proceed:
  DEBUG_CONSTRAIN_CVAR(("PROCEED\n"));
  return PROCEED;
}
//-----------------------------------------------------------------------------
#else
OZ_Return tellBasicConstraint(OZ_Term v, OZ_FSetConstraint * fs)
{
#ifdef TELLCONSTRAINTS_BY_UNIFICATION
  // create appropriate constrained variable
  OzFSVariable * fcv =
    fs ? new OzFSVariable(*fs,oz_currentBoard())
    : new OzFSVariable(oz_currentBoard());

    OZ_Term *  tcv = newTaggedCVar(fcv);

    return OZ_unify(v, makeTaggedRef(tcv));
#else

  DEREF(v, vptr, vtag);

  if (fs && !((FSetConstraint *) fs)->isValid())
    goto failed;


// tell finite set constraint to unconstrained variable
  if (oz_isFree(v)) { // mm3
    if (! fs) goto fsvariable;

    // fs denotes a set value --> v becomes set value
    if (fs->isValue()) {
      if (oz_isLocalVariable(vptr)) {
	if (!isUVar(vtag))
	  oz_checkSuspensionListProp(tagged2SVarPlus(v));
	DoBind(vptr, makeTaggedFSetValue(new FSetValue(*(FSetConstraint *) fs)));
      } else {
	DoBindAndTrail(vptr,
		       makeTaggedFSetValue(new FSetValue(*(FSetConstraint *) fs)));
      }
      goto proceed;
    }

    // create finite set variable
  fsvariable:
    OzFSVariable * fsv =
      fs ? new OzFSVariable(*fs,oz_currentBoard())
      : new OzFSVariable(oz_currentBoard());

    OZ_Term *  tfsv = newTaggedCVar(fsv);

    if (oz_isLocalVariable(vptr)) {
      if (!isUVar(vtag)) {
	oz_checkSuspensionListProp(tagged2SVarPlus(v));
	fsv->setSuspList(tagged2SVarPlus(v)->unlinkSuspList());
      }
      DoBind(vptr, makeTaggedRef(tfsv));
    } else {
      DoBindAndTrail(vptr, makeTaggedRef(tfsv));
    }

    goto proceed;
// tell finite set constraint to finite set variable
  } else if (isGenFSetVar(v, vtag)) {
    if (! fs) goto proceed;

    OzFSVariable * fsvar = tagged2GenFSetVar(v);
    OZ_FSetConstraint set = ((FSetConstraint *) ((OZ_FSetConstraint *) &fsvar->getSet()))->unify(* (FSetConstraint *) fs);

    if (!((FSetConstraint *) &set)->isValid())
      goto failed;

    if (!((FSetConstraint *) &fsvar->getSet())->isWeakerThan(*((FSetConstraint *) &set)))
      goto proceed;

    if (set.isValue()) {
      if (oz_isLocalVar(fsvar)) {
	fsvar->getSet() = set;
	fsvar->becomesFSetValueAndPropagate(vptr);
      } else {
	fsvar->propagate(fs_prop_val);
	DoBindAndTrail(vptr, makeTaggedFSetValue(new FSetValue(*((FSetConstraint *) &set))));
      }
    } else {
      fsvar->propagate(fs_prop_bounds);
      if (oz_isLocalVar(fsvar)) {
	fsvar->getSet() = set;
      } else {
	OzFSVariable * locfsvar
	  = new OzFSVariable(set,oz_currentBoard());
	OZ_Term * loctaggedfsvar = newTaggedCVar(locfsvar);
	DoBindAndTrailAndIP(vptr, makeTaggedRef(loctaggedfsvar),
			    locfsvar, tagged2GenFSetVar(v));
      }
    }
    goto proceed;
  } else if (isFSetValueTag(vtag)) {
    if (!fs) goto proceed;

    if (((FSetConstraint *) fs)->valid(*(FSetValue *) tagged2FSetValue(v)))
      goto proceed;
    goto failed;
  } else if (oz_isVariable(v)) {
    TaggedRef newVar = oz_newVariable();
    OZ_Return ret = tellBasicConstraint(newVar, fs);
    Assert(ret == PROCEED);
    return oz_unify(makeTaggedRef(vptr), newVar);
  }

failed:

  return FAILED;

proceed:

#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tellBasicConstraint - out - : ";
  if (vptr) oz_print(*vptr); else oz_print(v);
  if (fs) cout << " , " << *fs;
  cout << endl <<flush;
#endif

  return PROCEED;
#endif /* TELLCONSTRAINTS_BY_UNIFICATION */
}
#endif /* CORRECT_UNIFY */

// inline DISABLED CS
void OzFSVariable::propagate(OZ_FSetPropState state,
			     PropCaller prop_eq)
{
  if (prop_eq == pc_propagator) {
    switch (state) {
    case fs_prop_val:
      for (int i = fs_prop_any; i--; )
	if (fsSuspList[i])
	  OzVariable::propagateLocal(fsSuspList[i], prop_eq);
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
