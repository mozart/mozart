/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifdef FSETVAR

#if defined(INTERFACE)
#pragma implementation "fsgenvar.hh"
#endif

#include "am.hh"
#include "genvar.hh"

Bool GenFSetVariable::valid(TaggedRef val)
{
  Assert(!isRef(val));
  return (isFSetValue(val) && _fset.unify(*tagged2FSetValue(val)));
}

void GenFSetVariable::dispose(void) {
  //  _fset.disposeExtension();
  suspList->disposeList();
  freeListDispose(this, sizeof(GenFSetVariable));
}


Bool GenFSetVariable::unifyFSet(TaggedRef * vptr, TaggedRef var, 
				TaggedRef * tptr, TaggedRef term,
				Bool prop, /* propagate */
				Bool disp  /* dispose */)
{
  TypeOfTerm ttag = tagTypeOf(term);
  
  switch (ttag) {
  case FSETVALUE:
    {
      if (! _fset.unify(*tagged2FSetValue(term)))
	return FALSE;
      
      Bool isLocalVar = am.isLocalSVar(this);
      Bool isNotInstallingScript = !am.isInstallingScript();
      
      if (prop && (isNotInstallingScript || isLocalVar)) 
	propagate(var, fs_val);
      
      if (prop && isLocalVar) {
	doBind(vptr, term);
	if (disp) dispose();
      } else {
	am.doBindAndTrail(var, vptr, term);
      }
      
      return TRUE;
    }
  case CVAR:
    {
      switch(tagged2CVar(term)->getType()) {
      case FSetVariable: 
	{
	  GenFSetVariable * term_var = tagged2GenFSetVar(term);
	  OZ_FSetImpl &t_fset = term_var->getSet(), new_fset;
	  
	  if ((new_fset = t_fset.unify(_fset)).getCardMin() == -1)
	    return FALSE;
	  
	  Bool var_is_local =  (prop && am.isLocalSVar(this));
	  Bool term_is_local = (prop && am.isLocalSVar(term_var));

	  Bool is_not_installing_script = !am.isInstallingScript();
	  Bool var_is_constrained = is_not_installing_script ||
	                            _fset.isWeakerThan(new_fset);
	  Bool term_is_constrained = is_not_installing_script ||
	                             t_fset.isWeakerThan(new_fset);


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
	      if (t_fset.isWeakerThan(new_fset)) {
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
	      if (_fset.isWeakerThan(new_fset)){
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
	  return TRUE;
	}
      default: 
	break;
      }
    } // case CVAR;
  default:
    break;
  }
  return FALSE;
}

#if defined(OUTLINE)
#define inline
#include "fsgenvar.icc"
#undef inline
#endif

#endif /* FSETVAR */
