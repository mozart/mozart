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
#pragma implementation "fdbvar.hh"
#endif


#include "am.hh"
#include "genvar.hh"
#include "fdprofil.hh"
#include "fdomn.hh"

// unify expects either two GenFDVariables or at least one
// GenFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
OZ_Return GenBoolVariable::unifyV(TaggedRef * vPtr, TaggedRef term,
				  ByteCode *scp)
{
#ifdef SCRIPTDEBUG
  printf(am.isInstallingScript()
	 ? "bool installing script\n"
	 : "bool NOT installing script\n");
  fflush(stdout);
#endif

  if (oz_isSmallInt(term)) {
    int term_val = OZ_intToC(term);
    if (term_val < 0 || 1 < term_val) {
      return FALSE;
    }

    Bool isLocalVar = am.isLocalSVar(this);
    Bool isNotInstallingScript = !am.isInstallingScript();

#ifdef SCRIPTDEBUG
    printf("bool-int %s\n", isLocalVar ? "local" : "global"); fflush(stdout);
#endif

    if (scp==0 && (isNotInstallingScript || isLocalVar)) propagate();

    if (am.isLocalSVar(this)) {
      doBind(vPtr, term);
      dispose();
    } else {
      am.doBindAndTrail(vPtr, term);
    }
      
    return TRUE;
  }

  if (oz_isRef(term)) {
    TaggedRef *tPtr=tagged2Ref(term);
    term = *tPtr;
    GenCVariable *cv=tagged2CVar(term);
    switch (cv->getType()) {
    case BoolVariable:
      {
	Bool isConstrained = ! am.isInstallingScript();
	GenBoolVariable * termvar = (GenBoolVariable *)cv;

	Bool varIsLocal =  am.isLocalSVar(this);
	Bool termIsLocal = am.isLocalSVar(termvar);
	  
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
	      doBind(vPtr, makeTaggedRef(tPtr));
	      dispose();
	    } else { // bind term to var
	      termvar->propagate(pc_cv_unif);
	      propagate(pc_cv_unif);
	      termvar->relinkSuspListTo(this);
	      doBind(tPtr, makeTaggedRef(vPtr));
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
	    doBind(vPtr, makeTaggedRef(tPtr));
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
	    doBind(tPtr, makeTaggedRef(vPtr));
	    termvar->dispose();
	    break;
	  }

	case FALSE + 2 * FALSE: // var and term is global
	  {
#ifdef SCRIPTDEBUG
	    printf("bool-bool global global\n"); fflush(stdout);
#endif

	    GenBoolVariable * bool_var = new GenBoolVariable();
	    TaggedRef * var_val = newTaggedCVar(bool_var);

	    if (scp==0) {
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
	  error("unexpected case in unifyBool Bool <--> Bool");
	  break;
	} // switch
	return TRUE;
      }
    case FDVariable:
      {
	GenFDVariable * termvar = (GenFDVariable *)cv;
	  
	int intsct = termvar->intersectWithBool();
	  
	if (intsct == -2) return FAILED;

	Bool isNotInstallingScript = ! am.isInstallingScript();
	Bool isConstrainedVar = isNotInstallingScript || (intsct != -1);
	Bool isConstrainedTerm = isNotInstallingScript; 

	Bool varIsLocal =  am.isLocalSVar(this);
	Bool termIsLocal = am.isLocalSVar(termvar);
       
	switch (varIsLocal + 2 * termIsLocal) {
	case TRUE + 2 * TRUE: // var and term are local
	  { 
#ifdef SCRIPTDEBUG
	    printf("bool-fd local local\n"); fflush(stdout);
#endif

	    if (intsct != -1) {
	      TaggedRef int_var = OZ_int(intsct);
	      termvar->propagate(fd_prop_singl, pc_cv_unif);
	      propagate(pc_cv_unif);
	      doBind(vPtr, int_var);
	      doBind(tPtr, int_var);
	      dispose(); termvar->dispose();
	    } else if (heapNewer(vPtr, tPtr)) { // bind var to term
	      propagate(pc_cv_unif);
	      termvar->propagate(fd_prop_bounds, pc_cv_unif);
	      termvar->becomesBool();
	      relinkSuspListTo(termvar);
	      doBind(vPtr, makeTaggedRef(tPtr));
	      dispose();
	    } else { // bind term to var
	      termvar->propagate(fd_prop_bounds, pc_cv_unif);
	      propagate(pc_cv_unif);
	      termvar->relinkSuspListTo(this);
	      doBind(tPtr, makeTaggedRef(vPtr));
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
	      TaggedRef int_var = OZ_int(intsct);
	      if (isNotInstallingScript) 
		termvar->propagate(fd_prop_singl, pc_cv_unif);
	      if (isConstrainedVar) propagate(pc_cv_unif);
	      doBind(vPtr, int_var);
	      am.doBindAndTrail(tPtr, int_var);
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
	      TaggedRef int_term = OZ_int(intsct);
	      if (isNotInstallingScript) propagate(pc_cv_unif);
	      if (isConstrainedTerm) 
		termvar->propagate(fd_prop_singl, pc_cv_unif);
	      doBind(tPtr, int_term);
	      am.doBindAndTrail(vPtr, int_term);
	      termvar->dispose();
	    } else {
	      if (isConstrainedTerm) 
		termvar->propagate(fd_prop_bounds, pc_cv_unif);
	      if (isNotInstallingScript) propagate(pc_cv_unif);
	      termvar->relinkSuspListTo(this, TRUE);
	      doBind(tPtr, makeTaggedRef(vPtr));
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
	      TaggedRef int_val = OZ_int(intsct);
	      if (scp==0) {
		propagate(pc_cv_unif);
		termvar->propagate(fd_prop_singl, pc_cv_unif);
	      }
	      am.doBindAndTrail(vPtr, int_val);
	      am.doBindAndTrail(tPtr, int_val);
	    } else {
	      GenBoolVariable * bool_var = new GenBoolVariable();
	      TaggedRef * var_val = newTaggedCVar(bool_var);
	      if (scp==0) {
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
	  error("unexpected case in unifyBool Bool <--> FD");
	  break;
	} // switch
      }
    return TRUE;
    default:
      break;
    }
  }
  
  return FALSE;  
} // GenBoolVariable::unify
  


Bool GenBoolVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  if (oz_isSmallInt(val)) {
    int intval = OZ_intToC(val);
    return (intval == 0 || intval == 1);
  }
  return FALSE;
}


#ifdef OUTLINE 
#define inline
#include "fdbvar.icc"
#undef inline
#endif
