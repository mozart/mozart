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
#pragma implementation "ctgenvar.hh"
#endif

#include "am.hh"
#include "genvar.hh"

// `var' and `vptr' belongs to `this', i.e., is a `GenCtVariable
// `term' and `tptr' are either values or other constrained variable
// (preferably `GenCtVariable' of the right kind. Unification is
// implemented such, that `GenCtVariable's of the sam ekind are
// compatible with each other.
OZ_Return GenCtVariable::unifyV(OZ_Term * vptr, OZ_Term term, 
				ByteCode * scp)
{
  TypeOfTerm ttag = tagTypeOf(term);
  
  Assert(vptr);

  if (! oz_isRef(ttag)) {
    // `term' and `tptr' refer to a value

    // bind temporarily to avoid looping in unification on cyclic terms
    OZ_Term trail = *vptr;
    *vptr = term;

    OZ_Boolean result_unify = _constraint->unify(term);

    // undo binding
    *vptr = trail;

    if (! result_unify)
      goto f;

    {
      Bool isLocalVar = am.isLocalSVar(this);
      Bool isNotInstallingScript = !am.isInstallingScript();
      
      if (scp == 0 && (isNotInstallingScript || isLocalVar)) {
	propagateUnify();
      }
      if (isLocalVar) {
	doBind(vptr, term);
	disposeV();
      } else {
	am.doBindAndTrail(vptr, term);
      }
    }
    goto t;
  } else {
    OZ_Term  * tptr = tagged2Ref(term);
    term = *tptr;
    GenCVariable *cv = tagged2CVar(term);
    if (cv->getType() == CtVariable) {
      // `t' and `tptr' refer to another `GenCtVariable'
    
      GenCtVariable * term_var = (GenCtVariable *)cv;
      OZ_GenConstraint * t_constr = term_var->getConstraint();
      OZ_GenConstraint * constr = getConstraint();

      // bind temporarily to avoid looping in unification on cyclic terms
      OZ_Term trail = *vptr;
      *vptr = term;
      OZ_GenConstraint * new_constr = constr->unify(t_constr);
    
      // undo binding
      *vptr = trail;

      if (! new_constr->isValid())
	goto f;
    
      Bool var_is_local  = am.isLocalSVar(this);
      Bool term_is_local = am.isLocalSVar(term_var);
      Bool is_not_installing_script = !am.isInstallingScript();
      Bool var_is_constrained = (is_not_installing_script ||
				 (constr->isWeakerThan(new_constr)));
      Bool term_is_constrained = (is_not_installing_script ||
				  (t_constr->isWeakerThan(new_constr)));
   
      switch (var_is_local + 2 * term_is_local) {
      case TRUE + 2 * TRUE: // var and term are local
	{
	  if (new_constr->isValue()) {
	    // `new_constr' designates a value
	    OZ_Term new_value = new_constr->toValue();
	    term_var->propagateUnify();
	    propagateUnify();
	    doBind(vptr, new_value);
	    doBind(tptr, new_value);
	    disposeV(); 
	    term_var->disposeV(); 
	  } else if (heapNewer(vptr, tptr)) { // bind var to term
	    term_var->copyConstraint(new_constr);
	    propagateUnify();
	    term_var->propagateUnify();
	    relinkSuspListTo(term_var);
	    doBind(vptr, makeTaggedRef(tptr));
	    disposeV();
	  } else { // bind term to var
	    copyConstraint(new_constr);
	    term_var->propagateUnify();
	    propagateUnify();
	    term_var->relinkSuspListTo(this);
	    doBind(tptr, makeTaggedRef(vptr));
	    term_var->disposeV();
	  }
	} // TRUE + 2 * TRUE:
      break;

      case TRUE + 2 * FALSE: // var is local and term is global
	{
	  if (t_constr->isWeakerThan(new_constr)) {
	    if (new_constr->isValue()) {
	      // `new_constr' designates a value
	      OZ_Term new_value = new_constr->toValue();
	      if (is_not_installing_script) 
		term_var->propagateUnify();
	      if (var_is_constrained) 
		propagateUnify();
	      doBind(vptr, new_value);
	      am.doBindAndTrail(tptr, new_value);
	      disposeV();
	    } else {
	      copyConstraint(new_constr);
	      if (is_not_installing_script) 
		term_var->propagateUnify();
	      if (var_is_constrained) 
		propagateUnify();
	      DoBindAndTrailAndIP(tptr, makeTaggedRef(vptr),
				  this, term_var);
	    }
	  } else {
	    if (is_not_installing_script) 
	      term_var->propagateUnify();
	    if (var_is_constrained) 
	      propagateUnify();
	    relinkSuspListTo(term_var, TRUE);
	    doBind(vptr, makeTaggedRef(tptr));
	    disposeV();
	  }
	} // TRUE + 2 * FALSE:
      break;

      case FALSE + 2 * TRUE: // var is global and term is local
	{
	  if (constr->isWeakerThan(new_constr)) {
	    if(new_constr->isValue()) {
	      // `new_constr' designates a value
	      OZ_Term new_value = new_constr->toValue();
	      if (is_not_installing_script) 
		propagateUnify();
	      if (term_is_constrained) 
		term_var->propagateUnify();
	      doBind(tptr, new_value);
	      am.doBindAndTrail(vptr, new_value);
	      term_var->disposeV();
	    } else {
	      term_var->copyConstraint(new_constr);
	      if (is_not_installing_script) 
		propagateUnify();
	      if (term_is_constrained) 
		term_var->propagateUnify();
	      DoBindAndTrailAndIP(vptr, makeTaggedRef(tptr),
				  term_var, this);
	    }
	  } else {
	    if (term_is_constrained) 
	      term_var->propagateUnify();
	    if (is_not_installing_script) 
	      propagateUnify();
	    term_var->relinkSuspListTo(this, TRUE);
	    doBind(tptr, makeTaggedRef(vptr));
	    term_var->disposeV();
	  }
	} // FALSE + 2 * TRUE:
      break;

      case FALSE + 2 * FALSE: // var and term is global
	{
	  if (new_constr->isValue()){
	    // `new_constr' designates a value
	    OZ_Term new_value = new_constr->toValue();
	    if (scp == 0) {
	      if (var_is_constrained)
		propagateUnify();
	      if (term_is_constrained)
		term_var->propagateUnify();
	    }
	    am.doBindAndTrail(vptr, new_value);
	    am.doBindAndTrail(tptr, new_value);
	  } else { 
	    GenCVariable * cv = new GenCtVariable(new_constr, getDefinition());
	    OZ_Term * cvar = newTaggedCVar(cv);
	    if (scp == 0) {
	      if (var_is_constrained)
		propagateUnify();
	      if (term_is_constrained)
		term_var->propagateUnify();
	    }
	    DoBindAndTrailAndIP(vptr, makeTaggedRef(cvar),
				cv, this);
	    DoBindAndTrailAndIP(tptr, makeTaggedRef(cvar),
				cv, term_var);
	  }
	} // FALSE + 2 * FALSE: 
      break;

      default:
	error("unexpected case in unifyCt");
	break;
      } // switch (varIsLocal + 2 * termIsLocal) 
      goto t;

    }// if (! is_ct variable(ttag))
  }
  goto f;

t:
  return TRUE;
  
f:
  return FALSE;
}

OZ_Return tellBasicConstraint(OZ_Term v, 
			      OZ_GenConstraint * constr, 
			      OZ_GenDefinition * def)
{
  DEREF(v, vptr, vtag);


  // a constraint has to be valid
  if (constr && ! constr->isValid())
    goto failed;

  // tell constraint to unconstrained variable
  if (oz_isFree(v)) {
    if (! constr) goto ctvariable;

    // constr denotes a value --> v becomes value
    if (constr->isValue()) {
      if (am.isLocalVariable(v, vptr)) {
	if (isSVar(vtag))
	  am.checkSuspensionList(v);
	doBind(vptr, constr->toValue());
      } else {
	am.doBindAndTrail(vptr, constr->toValue());
      }
      goto proceed;
    } else {
    ctvariable:
      GenCtVariable * ctv = 
	constr 
	? new GenCtVariable(constr, def) 
	:  new GenCtVariable(def->leastConstraint(), def);
      
      OZ_Term *  tctv = newTaggedCVar(ctv);
      
      if (am.isLocalVariable(v, vptr)) {
	if (isSVar(vtag)) {
	  am.checkSuspensionList(v);
	  ctv->setSuspList(tagged2SVar(v)->getSuspList());
	}
	doBind(vptr, makeTaggedRef(tctv));
      } else { 
	am.doBindAndTrail(vptr, makeTaggedRef(tctv));
      }
      goto proceed;
    }
  } else if (isGenCtVar(v, vtag)) {
    // tell constraint to constrained variable
    if (! constr) goto proceed;

    GenCtVariable * ctvar = tagged2GenCtVar(v);
    OZ_GenConstraint * old_constr = ctvar->getConstraint();
    OZ_GenConstraintProfile * old_constr_prof = old_constr->getProfile();
    OZ_GenConstraint * new_constr = old_constr->unify(constr);

    if (! new_constr->isValid())
      goto failed;

    if (! ctvar->getConstraint()->isWeakerThan(new_constr))
      goto proceed;

    if (new_constr->isValue()) {
      // `new_constr' designates a value
      
      ctvar->propagate(OZ_WAKEUP_ALL, pc_propagator);

      if (am.isLocalSVar(v)) {
	doBind(vptr, new_constr->toValue());
      } else {
	am.doBindAndTrail(vptr, new_constr->toValue());
      }
    } else {
      // `new_constr' does not designate a value
      ctvar->propagate(new_constr->getWakeUpDescriptor(old_constr_prof),
		       pc_propagator);

      if (am.isLocalSVar(v)) {
	ctvar->copyConstraint(new_constr);
      } else {
	GenCtVariable * locctvar = new GenCtVariable(new_constr, def);
	OZ_Term * loctaggedctvar = newTaggedCVar(locctvar);
	DoBindAndTrailAndIP(vptr, makeTaggedRef(loctaggedctvar),
			    locctvar, ctvar);
      }
    }
    goto proceed;
  } else if (! oz_isVariable(vtag)) {
    // trying to constrain a value, i.e., check if constraint is
    // consistent with value
    if (! constr) goto proceed;

    if (constr->unify(v))
      goto proceed;
    goto failed;
  } 
  
failed:
  return FAILED;

proceed:
  return PROCEED;
}


void GenCtVariable::propagate(OZ_GenWakeUpDescriptor descr, 
			      PropCaller caller)
{
  int no_of_wakup_lists = _definition->getNoOfWakeUpLists();

  if (caller == pc_propagator) {
    // called by propagator
    for (int i = no_of_wakup_lists; i--; )
      if (descr.isWakeUp(i) && _susp_lists[i])
	GenCVariable::propagate(_susp_lists[i], caller);
  } else {
    // called by unification
    for (int i = no_of_wakup_lists; i--; )
      if (_susp_lists[i])
	GenCVariable::propagate(_susp_lists[i], caller);
  }
  if (suspList) 
    GenCVariable::propagate(suspList, caller);
}

void GenCtVariable::relinkSuspListTo(GenCtVariable * lv, Bool reset_local)
{
  GenCVariable::relinkSuspListTo(lv, reset_local); // any

  for (int i = _definition->getNoOfWakeUpLists(); i--; )
    _susp_lists[i] = 
      _susp_lists[i]->appendToAndUnlink(lv->_susp_lists[i], reset_local);
}

void GenCtVariable::installPropagators(GenCtVariable * glob_var, 
				       Board * glob_home)
{
  for (int i = _definition->getNoOfWakeUpLists(); i--; )
    _susp_lists[i] = oz_installPropagators(_susp_lists[i],
					  glob_var->_susp_lists[i],
					  glob_home);
}

//-----------------------------------------------------------------------------
// built-ins

#define OZ_getINDeref(N, V, VPTR, VTAG)		\
  OZ_Term V = OZ_in(N);				\
  DEREF(V, VPTR, VTAG);

OZ_BI_define(BIIsGenCtVarB, 1,1)
{
  OZ_getINDeref(0, v, vptr, vtag);

  OZ_RETURN(isGenCtVar(v, vtag) ? NameTrue : NameFalse);
} OZ_BI_end

OZ_C_proc_begin(BIGetCtVarConstraintAsAtom, 2)
{ 
  ExpectedTypes("GenCtVariable<ConstraintData>,Atom");
  
  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! oz_isVariable(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));   
  } else if (isGenCtVar(var, vartag)) {
    return OZ_unify(makeTaggedAtom(((GenCtVariable *) tagged2CVar(var))->getConstraint()->toString(ozconf.printDepth)),
		    OZ_getCArg(1));
  } else if (oz_isFree(var)) {
    OZ_addThread(makeTaggedRef(varptr),
		 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIGetCtVarNameAsAtom, 2)
{ 
  ExpectedTypes("GenCtVariable<ConstraintData>,Atom");
  
  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! oz_isVariable(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));   
  } else if (isGenCtVar(var, vartag)) {
    return
      OZ_unify(makeTaggedAtom(((GenCtVariable*)tagged2CVar(var))->getDefinition()->getName()),
	       OZ_getCArg(1));   
  } else if (oz_isFree(var)) {
    OZ_addThread(makeTaggedRef(varptr),
		 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end



#ifdef OUTLINE 
#define inline
#include "ctgenvar.icc"
#undef inline
#endif

//-----------------------------------------------------------------------------

#include "ctgenvar.dcl"
static
BIspec fdSpec[] = {
#include "ctgenvar.tbl"
  {0,0,0,0}
};

void BIinitCtVar(void)
{
  BIaddSpec(fdSpec);
}

// eof
//-----------------------------------------------------------------------------
