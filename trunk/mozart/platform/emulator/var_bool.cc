/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "fdbvar.hh"
#endif


#include "am.hh"
#include "genvar.hh"
#include "fdprofil.hh"


// unify expects either two GenFDVariables or at least one
// GenFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
Bool GenBoolVariable::unifyBool(TaggedRef * vPtr, TaggedRef var,
				TaggedRef * tPtr, TaggedRef term,
				Bool prop, Bool disp)
{
  TypeOfTerm tTag = tagTypeOf(term);
  
  switch (tTag) {
  case SMALLINT:
    {
      int term_val = OZ_intToC(term);
      if (term_val < 0 || 1 < term_val) {
	return FALSE;
      }
      if (prop) propagate(var, pc_propagator);

      if (prop && am.isLocalSVar(this)) {
	doBind(vPtr, term);
	if (disp) dispose();
      } else {
	am.doBindAndTrail(var, vPtr, term);
      }
      
      return TRUE;
    }


  case CVAR:
    {
      switch (tagged2CVar(term)->getType()) {
      case BoolVariable:
	{
	  GenBoolVariable * termvar = tagged2GenBoolVar(term);

	  Bool varIsLocal =  (prop && am.isLocalSVar(this));
	  Bool termIsLocal = (prop && am.isLocalSVar(termvar));
	  
	  switch (varIsLocal + 2 * termIsLocal) {
	  case TRUE + 2 * TRUE: // var and term are local
	    {
	      if (heapNewer(vPtr, tPtr)) { // bind var to term
		  propagate(var, pc_cv_unif);
		  termvar->propagate(term, pc_cv_unif);
		  relinkSuspListTo(termvar);
		  doBind(vPtr, makeTaggedRef(tPtr));
		  if (disp) dispose();
		} else { // bind term to var
		  termvar->propagate(term, pc_cv_unif);
		  propagate(var, pc_cv_unif);
		  termvar->relinkSuspListTo(this);
		  doBind(tPtr, makeTaggedRef(vPtr));
		  if (disp) termvar->dispose();
		}
		break;
	      }

	  case TRUE + 2 * FALSE: // var is local and term is global
	    {
	      termvar->propagate(term, pc_cv_unif);
	      propagate(var, pc_cv_unif);
	      relinkSuspListTo(termvar, TRUE);
	      doBind(vPtr, makeTaggedRef(tPtr));
	      if (disp) dispose();
	      break;
	    }
	    
	  case FALSE + 2 * TRUE: // var is global and term is local
	    {
	      termvar->propagate(term, pc_cv_unif);
	      propagate(var, pc_cv_unif);
	      termvar->relinkSuspListTo(this, TRUE);
	      doBind(tPtr, makeTaggedRef(vPtr));
	      if (disp) termvar->dispose();
	      break;
	    }
	    
	  case FALSE + 2 * FALSE: // var and term is global
	    {
	      GenBoolVariable * bool_var = new GenBoolVariable();
	      TaggedRef * var_val = newTaggedCVar(bool_var);

	      if (prop) {
		propagate(var, pc_cv_unif);
		termvar->propagate(term, pc_cv_unif);
	      }
	      am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(var_val),
				     bool_var, this, prop);
	      am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(var_val),
				     bool_var, termvar, prop);
	      
	      break;
	    }

	  default:
	    error("unexpected case in unifyBool Bool <--> Bool");
	    break;
	  } // switch
	}
	return TRUE;
      case FDVariable:
	{
	  GenFDVariable * termvar = tagged2GenFDVar(term);
	  
	  int intsct = termvar->intersectWithBool();
	  
	  if (intsct == -2) return FAILED;
	  
	  Bool varIsLocal =  (prop && am.isLocalSVar(this));
	  Bool termIsLocal = (prop && am.isLocalSVar(termvar));
       
	  switch (varIsLocal + 2 * termIsLocal) {
	  case TRUE + 2 * TRUE: // var and term are local
	    { 
	      if (intsct != -1) {
		TaggedRef int_var = OZ_CToInt(intsct);
		termvar->propagate(term, fd_det, pc_cv_unif);
		propagate(var, pc_cv_unif);
		doBind(vPtr, int_var);
		doBind(tPtr, int_var);
		if (disp) { dispose(); termvar->dispose(); }
	      } else if (heapNewer(vPtr, tPtr)) { // bind var to term
		propagate(var, pc_cv_unif);
		termvar->propagate(term, fd_bounds, pc_cv_unif);
		termvar->becomesBool();
		relinkSuspListTo(termvar);
		doBind(vPtr, makeTaggedRef(tPtr));
		if (disp) dispose();
	      } else { // bind term to var
		termvar->propagate(term, fd_bounds, pc_cv_unif);
		propagate(var, pc_cv_unif);
		termvar->relinkSuspListTo(this);
		doBind(tPtr, makeTaggedRef(vPtr));
		if (disp) termvar->dispose();
	      }
		break;
	    }

	    case TRUE + 2 * FALSE: // var is local and term is global
	      {
		if (intsct != -1) {
		  TaggedRef int_var = OZ_CToInt(intsct);
		  termvar->propagate(term, fd_det, pc_cv_unif);
		  propagate(var, pc_cv_unif);
		  doBind(vPtr, int_var);
		  am.doBindAndTrail(term, tPtr, int_var);
		  if (disp) dispose();
		} else {
		  termvar->propagate(term, fd_bounds, pc_cv_unif);
		  propagate(var, pc_cv_unif);
		  am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(vPtr),
					 this, termvar, prop);
		}
		break;
	      }

	    case FALSE + 2 * TRUE: // var is global and term is local
	      {
		if(intsct != -1) {
		  TaggedRef int_term = OZ_CToInt(intsct);
		  propagate(var, pc_cv_unif);
		  termvar->propagate(term, fd_det, pc_cv_unif);
		  doBind(tPtr, int_term);
		  am.doBindAndTrail(var, vPtr, int_term);
		  if (disp) termvar->dispose();
		} else {
		  termvar->propagate(term, fd_bounds, pc_cv_unif);
		  propagate(var, pc_cv_unif);
		  termvar->relinkSuspListTo(this, TRUE);
		  doBind(tPtr, makeTaggedRef(vPtr));
		  if (disp) termvar->dispose();
		}
		break;
	      }

	    case FALSE + 2 * FALSE: // var and term is global
	      {
		if (intsct != -1){
		  TaggedRef int_val = OZ_CToInt(intsct);
		  if (prop) {
		    propagate(var, pc_cv_unif);
		    termvar->propagate(term, fd_det, pc_cv_unif);
		  }
		  am.doBindAndTrail(var, vPtr, int_val);
		  am.doBindAndTrail(term, tPtr, int_val);
		} else {
		  GenBoolVariable * bool_var = new GenBoolVariable();
		  TaggedRef * var_val = newTaggedCVar(bool_var);
		  if (prop) {
		    propagate(var, pc_cv_unif);
		    termvar->propagate(term, fd_bounds, pc_cv_unif);
		  }
		  am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(var_val),
					 bool_var, this, prop);
		  am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(var_val),
					 bool_var, termvar, prop);
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
  default:
    break;
  } // switch (tTag) 
  
  return FALSE;  
} // GenBoolVariable::unify
  


Bool GenBoolVariable::valid(TaggedRef val)
{
  Assert(!isRef(val));
  if (isSmallInt(val)) {
    int intval = OZ_intToC(val);
    return (intval == 0 || intval == 1);
  }
  return FALSE;
}


#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline
#include "fdbvar.icc"
#undef inline
#endif
