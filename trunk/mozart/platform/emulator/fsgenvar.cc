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

#if defined(INTERFACE)
#pragma implementation "fsgenvar.hh"
#endif

#include "ozostream.hh"
#include "fddebug.hh"
#include "am.hh"
#include "genvar.hh"

Bool GenFSetVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  return (oz_isFSetValue(val) && ((FSetConstraint *) &_fset)->valid(*(FSetValue *)tagged2FSetValue(val)));
}

void GenFSetVariable::dispose(void) {
  suspList->disposeList();
  freeListDispose(this, sizeof(GenFSetVariable));
}

#ifdef DEBUG_FSET
//#define DEBUG_FSUNIFY
//#define DEBUG_TELLCONSTRAINTS 
#endif


OZ_Return GenFSetVariable::unifyV(OZ_Term * vptr, OZ_Term term, ByteCode * scp)
{
  if (oz_isFSetValue(term)) {
#ifdef DEBUG_FSUNIFY 
    (*cpi_cout) << "fsunify(value): (" << _fset.toString() << " = " 
		<< *((FSetValue *)tagged2FSetValue(term)) << " )";
#endif
      
    if (! ((FSetConstraint *) &_fset)->valid(*(FSetValue *)tagged2FSetValue(term)))
      goto f;

    Bool isLocalVar = am.isLocalSVar(this);
    Bool isNotInstallingScript = !am.isInstallingScript();
      
    if (scp==0 && (isNotInstallingScript || isLocalVar)) 
      propagate(fs_prop_val);
      
    if (isLocalVar) {
      doBind(vptr, term);
      dispose();
    } else {
      am.doBindAndTrail(vptr, term);
    }
      
#ifdef DEBUG_FSUNIFY 
    (*cpi_cout) << " -> " <<  _fset.toString();
#endif
      
    goto t;
  } // case FSETVALUE:

  if (oz_isRef(term)) {
    TaggedRef *tptr=tagged2Ref(term);
    term = *tptr;
    GenCVariable *cv=tagged2CVar(term);
    if (cv->getType() == FSetVariable) {
      GenFSetVariable * term_var = (GenFSetVariable *)cv;
      OZ_FSetConstraint * t_fset = (OZ_FSetConstraint *) &term_var->getSet();
      OZ_FSetConstraint * fset = (OZ_FSetConstraint *) &getSet();
      OZ_FSetConstraint new_fset;

#ifdef DEBUG_FSUNIFY 
      (*cpi_cout) << "fsunify(var): (" << *fset << " = " << *t_fset << " )";
#endif
      
      if ((new_fset = ((FSetConstraint *) t_fset)->unify(*(FSetConstraint *) fset)).getCardMin() == -1)
	goto f;

#ifdef DEBUG_FSUNIFY 
      (*cpi_cout) << " -> " << new_fset << " " << new_fset.isValue();
#endif
      
      Bool var_is_local  = am.isLocalSVar(this);
      Bool term_is_local = am.isLocalSVar(term_var);
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
	    doBind(vptr, new_fset_var);
	    doBind(tptr, new_fset_var);
	    dispose(); 
	    term_var->dispose(); 
	  } else if (heapNewer(vptr, tptr)) { // bind var to term
	    term_var->setSet(new_fset);
	    propagateUnify();
	    term_var->propagateUnify();
	    relinkSuspListTo(term_var);
	    doBind(vptr, makeTaggedRef(tptr));
	    dispose();
	  } else { // bind term to var
	    setSet(new_fset);
	    term_var->propagateUnify();
	    propagateUnify();
	    term_var->relinkSuspListTo(this);
	    doBind(tptr, makeTaggedRef(vptr));
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
	      doBind(vptr, new_fset_var);
	      am.doBindAndTrail(tptr, new_fset_var);
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
	    doBind(vptr, makeTaggedRef(tptr));
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
	      doBind(tptr, new_fset_var);
	      am.doBindAndTrail(vptr, new_fset_var);
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
	    doBind(tptr, makeTaggedRef(vptr));
	    term_var->dispose();
	  }
	  break;
	} // FALSE + 2 * TRUE:
      case FALSE + 2 * FALSE: // var and term is global
	{
	  if (new_fset.isValue()){
	    OZ_Term new_fset_var
	      = makeTaggedFSetValue(new FSetValue(*((FSetConstraint *) &new_fset)));
	    if (scp==0) {
	      if (var_is_constrained) propagateUnify();
	      if (term_is_constrained) term_var->propagateUnify();
	    }
	    am.doBindAndTrail(vptr, new_fset_var);
	    am.doBindAndTrail(tptr, new_fset_var);
	  } else {
	    GenCVariable *c_var = new GenFSetVariable(new_fset);
	    TaggedRef * var_val = newTaggedCVar(c_var);
	    if (scp==0) {
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
	error("unexpected case in unifyFSet");
	break;
      } // switch (varIsLocal + 2 * termIsLocal) 
      goto t;
    } // case FSetVariable:
    goto f;
  } // if (tagged2CVar(term)->getType() == FSetVariable)

  goto f;

t:
#ifdef DEBUG_FSUNIFY 
  
  (*cpi_cout) << toC(*vptr) << " true" << endl << flush;
#endif
  return TRUE;

f:
#ifdef DEBUG_FSUNIFY 
  (*cpi_cout) << "false" << endl << flush;
#endif
  return FALSE;
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
  if (isNotCVar(vtag)) {
    if (! fs) goto fsvariable;

    // fs denotes a set value --> v becomes set value
    if (fs->isValue()) {
      if (am.isLocalVariable(v, vptr)) {
	if (isSVar(vtag))
	  am.checkSuspensionList(v);
	doBind(vptr, makeTaggedFSetValue(new FSetValue(*(FSetConstraint *) fs)));
      } else {
	am.doBindAndTrail(vptr,
			  makeTaggedFSetValue(new FSetValue(*(FSetConstraint *) fs)));
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
      am.doBindAndTrail(vptr, makeTaggedRef(tfsv));
    }
    
    goto proceed;
// tell finite set constraint to finite set variable
  } else if (isGenFSetVar(v, vtag)) {
    if (! fs) goto proceed;

    GenFSetVariable * fsvar = tagged2GenFSetVar(v);
    OZ_FSetConstraint set = ((FSetConstraint *) ((OZ_FSetConstraint *) &fsvar->getSet()))->unify(* (FSetConstraint *) fs);

    if (!((FSetConstraint *) &set)->isValid())
      goto failed;

    if (!((FSetConstraint *) &fsvar->getSet())->isWeakerThan(*((FSetConstraint *) &set)))
      goto proceed;

    if (set.isValue()) {
      if (am.isLocalSVar(v)) {
	fsvar->getSet() = set;
	fsvar->becomesFSetValueAndPropagate(vptr);
      } else {
	fsvar->propagate(fs_prop_val);
	am.doBindAndTrail(vptr, makeTaggedFSetValue(new FSetValue(*((FSetConstraint *) &set))));
      }
    } else {
      fsvar->propagate(fs_prop_bounds);
      if (am.isLocalSVar(v)) {
	fsvar->getSet() = set;
      } else {
	GenFSetVariable * locfsvar = new GenFSetVariable(set);
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

#if defined(OUTLINE)
#define inline
#include "fsgenvar.icc"
#undef inline
#endif
