/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE)
#pragma implementation "fsgenvar.hh"
#endif

#include "am.hh"
#include "genvar.hh"

Bool GenFSetVariable::valid(TaggedRef val)
{
  Assert(!isRef(val));
  return (isFSetValue(val) && ((OZ_FSetImpl *) &_fset)->unify(*(FSetValue *)tagged2FSetValue(val)));
}

void GenFSetVariable::dispose(void) {
  suspList->disposeList();
  freeListDispose(this, sizeof(GenFSetVariable));
}

#ifdef DEBUG_FSET
#define DEBUG_FSUNIFY
#define DEBUG_TELLCONSTRAINTS 
#endif

Bool GenFSetVariable::unifyFSet(TaggedRef * vptr, TaggedRef var, 
				TaggedRef * tptr, TaggedRef term,
				Bool prop, /* propagate */
				Bool disp  /* dispose */)
{
#ifdef DEBUG_FSUNIFY 
  cout << "entering fs unify" << endl << flush;
  taggedPrint(makeTaggedRef(vptr));
  taggedPrint(makeTaggedRef(tptr));
#endif

  TypeOfTerm ttag = tagTypeOf(term);
  
  switch (ttag) {
  case FSETVALUE:
    {
#ifdef DEBUG_FSUNIFY 
      cout << "fs = fs val" << endl << flush;
#endif

      if (! ((OZ_FSetImpl *) &_fset)->unify(*(FSetValue *)tagged2FSetValue(term)))
	goto f;

#ifdef DEBUG_FSUNIFY 
      cout << "succeeded" << endl << flush;
#endif
      
      Bool isLocalVar = am.isLocalSVar(this);
      Bool isNotInstallingScript = !am.isInstallingScript();
      
      if (prop && (isNotInstallingScript || isLocalVar)) 
	propagate(var, fs_prop_val);
      
      if (prop && isLocalVar) {
	doBind(vptr, term);
	if (disp) dispose();
      } else {
	am.doBindAndTrail(var, vptr, term);
      }
      
      goto t;
    }
  case CVAR:
    {
      switch(tagged2CVar(term)->getType()) {
      case FSetVariable: 
	{
#ifdef DEBUG_FSUNIFY 
	  cout << "fs = fs" << endl << flush;
#endif
	  GenFSetVariable * term_var = tagged2GenFSetVar(term);
	  OZ_FSetImpl * t_fset = (OZ_FSet *) &term_var->getSet();
	  OZ_FSetImpl * fset = (OZ_FSet *) &getSet();
	  OZ_FSetImpl new_fset;

	  if ((new_fset = t_fset->unify(*fset)).getCardMin() == -1)
	    goto f;

#ifdef DEBUG_FSUNIFY 
	  cout << "succeeded" << endl << flush;
#endif
	  Bool var_is_local = (prop && am.isLocalSVar(this));
	  Bool term_is_local = (prop && am.isLocalSVar(term_var));
	  Bool is_not_installing_script = !am.isInstallingScript();
	  Bool var_is_constrained = (is_not_installing_script ||
				     fset->isWeakerThan(new_fset));
	  Bool term_is_constrained = (is_not_installing_script ||
				      t_fset->isWeakerThan(new_fset));


	  switch (var_is_local + 2 * term_is_local) {
	  case TRUE + 2 * TRUE: // var and term are local
	    {
	      if (new_fset.isFSetValue()) {
		OZ_Term new_fset_var = makeTaggedFSetValue(new FSetValue(new_fset));
		term_var->propagateUnify(term);
		propagateUnify(var);
		doBind(vptr, new_fset_var);
		doBind(tptr, new_fset_var);
		if (disp) { 
		  dispose(); 
		  term_var->dispose(); 
		}
	      } else if (heapNewer(vptr, tptr)) { // bind var to term
		term_var->setSet(new_fset);
		propagateUnify(var);
		term_var->propagateUnify(term);
		relinkSuspListTo(term_var);
		doBind(vptr, makeTaggedRef(tptr));
		if (disp) 
		  dispose();
	      } else { // bind term to var
		setSet(new_fset);
		term_var->propagateUnify(term);
		propagateUnify(var);
		term_var->relinkSuspListTo(this);
		doBind(tptr, makeTaggedRef(vptr));
		if (disp) 
		  term_var->dispose();
	      }
	      break;
	    }
	  case TRUE + 2 * FALSE: // var is local and term is global
	    {
	      if (t_fset->isWeakerThan(new_fset)) {
		if (new_fset.isFSetValue()) {
		  OZ_Term new_fset_var = makeTaggedFSetValue(new FSetValue(new_fset));
		  if (is_not_installing_script) term_var->propagateUnify(term);
		  if (var_is_constrained) propagateUnify(var);
		  doBind(vptr, new_fset_var);
		  am.doBindAndTrail(term, tptr, new_fset_var);
		  if (disp) 
		    dispose();
		} else {
		  setSet(new_fset);
		  if (is_not_installing_script) term_var->propagateUnify(term);
		  if (var_is_constrained) propagateUnify(var);
		  am.doBindAndTrailAndIP(term, tptr, makeTaggedRef(vptr),
					 this, term_var, prop);
		}
	      } else {
		if (is_not_installing_script) term_var->propagateUnify(term);
		if (var_is_constrained) propagateUnify(var);
		relinkSuspListTo(term_var, TRUE);
		doBind(vptr, makeTaggedRef(tptr));
		if (disp) dispose();
	      }
	      break;
	    }
	  case FALSE + 2 * TRUE: // var is global and term is local
	    {
	      if (fset->isWeakerThan(new_fset)){
		if(new_fset.isFSetValue()) {
		  OZ_Term new_fset_var = makeTaggedFSetValue(new FSetValue(new_fset));
		  if (is_not_installing_script) propagateUnify(var);
		  if (term_is_constrained) term_var->propagateUnify(term);
		  doBind(tptr, new_fset_var);
		  am.doBindAndTrail(var, vptr, new_fset_var);
		  if (disp) 
		    term_var->dispose();
		} else {
		  term_var->setSet(new_fset);
		  if (is_not_installing_script) propagateUnify(var);
		  if (term_is_constrained) term_var->propagateUnify(term);
		  am.doBindAndTrailAndIP(var, vptr, makeTaggedRef(tptr),
					 term_var, this, prop);
		}
	      } else {
		if (term_is_constrained) term_var->propagateUnify(term);
		if (is_not_installing_script) propagateUnify(var);
		term_var->relinkSuspListTo(this, TRUE);
		doBind(tptr, makeTaggedRef(vptr));
		if (disp) 
		  term_var->dispose();
	      }
	      break;
	    }
	  case FALSE + 2 * FALSE: // var and term is global
	    {
	      if (new_fset.isFSetValue()){
		OZ_Term new_fset_var = makeTaggedFSetValue(new FSetValue(new_fset));
		if (prop) {
		  propagateUnify(var);
		  term_var->propagateUnify(term);
		}
		am.doBindAndTrail(var, vptr, new_fset_var);
		am.doBindAndTrail(term, tptr, new_fset_var);
	      } else {
		GenCVariable *c_var = new GenFSetVariable(new_fset);
		TaggedRef * var_val = newTaggedCVar(c_var);
		if (prop) {
		  propagateUnify(var);
		  term_var->propagateUnify(term);
		}
		am.doBindAndTrailAndIP(var, vptr, makeTaggedRef(var_val),
				       c_var, this, prop);
		am.doBindAndTrailAndIP(term, tptr, makeTaggedRef(var_val),
				       c_var, term_var, prop);
	      }
	      break;
	    }
	  default:
	    error("unexpected case in unifyFSet");
	    break;
	  } // switch (varIsLocal + 2 * termIsLocal) 
	  goto t;
	}
      default: 
	break;
      }
    } // case CVAR;
  default:
    break;
  }
t:
#ifdef DEBUG_FSUNIFY 
  cout << "leaving fs unify (true)" << flush;
  taggedPrint(makeTaggedRef(vptr));
  taggedPrint(makeTaggedRef(tptr));
  cout << endl << flush;
#endif
  return TRUE;

f:
#ifdef DEBUG_FSUNIFY 
  cout << "leaving fs unify (false)" << flush;
  taggedPrint(makeTaggedRef(vptr));
  taggedPrint(makeTaggedRef(tptr));
  cout << endl << flush;
#endif
  return FALSE;
}

OZ_Return tellBasicConstraint(OZ_Term v, OZ_FSet * fs)
{
#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tellBasicConstraint - in - : ";
  taggedPrint(v);
  if (fs) cout << " , " << *fs;
  cout << endl <<flush;
#endif

  DEREF(v, vptr, vtag);

  if (fs && !fs->isValid())
    goto failed;


// tell finite set constraint to unconstrained variable
  if (isNotCVar(vtag)) {
    if (! fs) goto fsvariable;

    // fs denotes a set value --> v becomes set value
    if (fs->isValue()) {
      if (am.isLocalVariable(v, vptr)) {
	if (isSVar(vtag))
	  am.checkSuspensionList(v);
	doBind(vptr, makeTaggedFSetValue(new FSetValue(*fs)));
      } else {
	am.doBindAndTrail(v, vptr, makeTaggedFSetValue(new FSetValue(*fs)));
      }
      goto proceed;
    }

    // create finite set variable
  fsvariable:
    GenFSetVariable * fsv = 
      fs ? new GenFSetVariable(*fs) : new GenFSetVariable();

    OZ_Term *  tfsv = newTaggedCVar(fsv);

    if (am.isLocalVariable(v, vptr)) {
      if (isSVar(vtag)) {
	am.checkSuspensionList(v);
	fsv->setSuspList(tagged2SVar(v)->getSuspList());
      }
      doBind(vptr, makeTaggedRef(tfsv));
    } else { 
      am.doBindAndTrail(v, vptr, makeTaggedRef(tfsv));
    }
    
    goto proceed;
// tell finite set constraint to finite set variable
  } else if (isGenFSetVar(v, vtag)) {
    if (! fs) goto proceed;

    GenFSetVariable * fsvar = tagged2GenFSetVar(v);
    OZ_FSet set = ((OZ_FSetImpl *) &fsvar->getSet())->unify(*fs);

    if (!set.isValid())
      goto failed;

    if (!((OZ_FSetImpl *) &fsvar->getSet())->isWeakerThan(set))
      goto proceed;

    if (set.isValue()) {
      if (am.isLocalCVar(v)) {
	fsvar->getSet() = set;
	fsvar->becomesFSetValueAndPropagate(vptr);
      } else {
	fsvar->propagate(v, fs_prop_val);
	am.doBindAndTrail(v, vptr, makeTaggedFSetValue(new FSetValue(set)));
      }
    } else {
      fsvar->propagate(v, fs_prop_bounds);
      if (am.isLocalCVar(v)) {
	fsvar->getSet() = set;
      } else {
	GenFSetVariable * locfsvar = new GenFSetVariable(set);
	OZ_Term * loctaggedfsvar = newTaggedCVar(locfsvar);
	am.doBindAndTrailAndIP(v, vptr,
			       makeTaggedRef(loctaggedfsvar),
			       locfsvar, tagged2GenFSetVar(v), OZ_FALSE);
      }
    }
    goto proceed;
  } else if (isFSetValue(vtag)) {
    if (!fs) goto proceed;
    
    if (fs->unify(*(FSetValue *) tagged2FSetValue(v)))
      goto proceed;
    goto failed;
  } 
  
failed:

  return FAILED;

proceed:

#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tellBasicConstraint - out - : ";
  if (vptr) taggedPrint(*vptr); else taggedPrint(v);
  if (fs) cout << " , " << *fs;
  cout << endl <<flush;
#endif

  return PROCEED;
}

#if defined(OUTLINE)
#define inline
#include "fsgenvar.icc"
#undef inline
#endif

