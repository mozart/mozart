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
  disposeS();
  freeListDispose(this, sizeof(OzFSVariable));
}

#ifdef DEBUG_FSET
//#define DEBUG_FSUNIFY
//#define DEBUG_TELLCONSTRAINTS 
#endif


OZ_Return OzFSVariable::bind(OZ_Term * vptr, OZ_Term term)
{
  Assert(!oz_isRef(term));
  if (!oz_isFSetValue(term)) return FAILED;

#ifdef DEBUG_FSUNIFY 
  (*cpi_cout) << "fsunify(value): (" << _fset.toString() << " = " 
	      << *((FSetValue *)tagged2FSetValue(term)) << " )";
#endif
      
  if (! ((FSetConstraint *) &_fset)->valid(*(FSetValue *)tagged2FSetValue(term))) {
#ifdef DEBUG_FSUNIFY 
    (*cpi_cout) << "false" << endl << flush;
#endif
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
      
#ifdef DEBUG_FSUNIFY 
  (*cpi_cout) << " -> " <<  _fset.toString();
  (*cpi_cout) << toC(*vptr) << " true" << endl << flush;
#endif
  return TRUE;
}


OZ_Return OzFSVariable::unify(OZ_Term * vptr, OZ_Term *tptr)
{
  OZ_Term term = *tptr;
  OzVariable *cv=tagged2CVar(term);
  if (cv->getType() != OZ_VAR_FS) {
  f:
#ifdef DEBUG_FSUNIFY 
    (*cpi_cout) << "false" << endl << flush;
#endif
    return FALSE;
  }

  OzFSVariable * term_var = (OzFSVariable *)cv;
  OZ_FSetConstraint * t_fset = (OZ_FSetConstraint *) &term_var->getSet();
  OZ_FSetConstraint * fset = (OZ_FSetConstraint *) &getSet();
  OZ_FSetConstraint new_fset;

#ifdef DEBUG_FSUNIFY 
  (*cpi_cout) << "fsunify(var): (" << *fset << " = " << *t_fset << " )";
#endif
  new_fset = ((FSetConstraint *) t_fset)->unify(*(FSetConstraint *) fset);
  if (new_fset.getCardMin() == -1)
    goto f;

#ifdef DEBUG_FSUNIFY 
  (*cpi_cout) << " -> " << new_fset << " " << new_fset.isValue();
#endif
      
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

#ifdef DEBUG_FSUNIFY 
  (*cpi_cout) << toC(*vptr) << " true" << endl << flush;
#endif
  return TRUE;
}

OZ_Return tellBasicConstraint(OZ_Term v, OZ_FSetConstraint * fs)
{
#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tellBasicConstraint - in - : ";
  oz_print(v);
  if (fs) cout << " , " << *fs;
  cout << endl <<flush;
#endif

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
}

// inline DISABLED CS
void OzFSVariable::propagate(OZ_FSetPropState state,
				PropCaller prop_eq)
{
  if (prop_eq == pc_propagator) {
    switch (state) {
    case fs_prop_val: { // no break
      for (int i = fs_prop_any; i--; )
	if (fsSuspList[i])
	  OzVariable::propagate(fsSuspList[i], prop_eq);
    }
 	   case fs_prop_lub: case fs_prop_glb:
      if (fsSuspList[state])
	OzVariable::propagate(fsSuspList[state], prop_eq);
      break;
    case fs_prop_bounds:
      if (fsSuspList[fs_prop_lub])
	OzVariable::propagate(fsSuspList[fs_prop_lub], prop_eq);
      if (fsSuspList[fs_prop_glb])
	OzVariable::propagate(fsSuspList[fs_prop_glb], prop_eq);
      break;      
    default:
      break;
    }
  } else {
    for (int i = fs_prop_any; i--; )
      if (fsSuspList[i])
	OzVariable::propagate(fsSuspList[i], prop_eq);
  }
  if (suspList) 
    OzVariable::propagate(suspList, prop_eq);
}


#if defined(OUTLINE)
#define inline
#include "var_fs.icc"
#undef inline
#endif
