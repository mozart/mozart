/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller

  ------------------------------------------------------------------------
*/


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "fdgenvar.hh"
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
Bool GenFDVariable::unifyFD(TaggedRef * vPtr, TaggedRef var,
			    TaggedRef * tPtr, TaggedRef term,
			    ByteCode *scp, /* propagate */
			    Bool disp  /* dispose */ )
{
#ifdef SCRIPTDEBUG
  printf(am.isInstallingScript() ? "fd installing script\n" : "fd NOT installing script\n"); fflush(stdout);
#endif

  TypeOfTerm tTag = tagTypeOf(term);
  
  switch (tTag) {
  case SMALLINT:
    {
      if (! finiteDomain.isIn(OZ_intToC(term))) {
	PROFILE_CODE1(FDProfiles.inc_item(no_failed_fdunify_vars);)
	return FALSE;
      }
      
      Bool isLocalVar = am.isLocalSVar(this);
      Bool isNotInstallingScript = !am.isInstallingScript();

#ifdef SCRIPTDEBUG
      printf("fd-int %s\n", isLocalVar ? "local" : "global"); fflush(stdout);
#endif

      if (scp==0 && (isNotInstallingScript || isLocalVar)) 
	propagate(var, fd_prop_singl);

      if (isLocalVar) {
	doBind(vPtr, term);
	if (disp) dispose();
      } else {
	am.doBindAndTrail(var, vPtr, term);
      }
      
      PROFILE_CODE1(if (FDVarsTouched.add(term))
		      FDProfiles.inc_item(no_touched_vars);
		    FDProfiles.inc_item(no_succ_fdunify_vars);
		    )
      return TRUE;
    }
  case CVAR:
    {
      switch(tagged2CVar(term)->getType()) {
      case FDVariable: 
	{
	  // compute intersection of domains ...
	  GenFDVariable * termVar = tagged2GenFDVar(term);
	  OZ_FiniteDomain &termDom = termVar->finiteDomain;
	  OZ_FiniteDomain intsct;
	  
	  if ((intsct = finiteDomain & termDom) == fd_empty) {
	    PROFILE_CODE1(FDProfiles.inc_item(no_failed_fdunify_vars);)
	      return FALSE;
	  }
	  
	  // bind - trail - propagate
	  Bool varIsLocal =  am.isLocalSVar(this);
	  Bool termIsLocal = am.isLocalSVar(termVar);

	  Bool isNotInstallingScript = !am.isInstallingScript();
	  Bool varIsConstrained = isNotInstallingScript ||
	    (intsct.getSize() < finiteDomain.getSize());
	  Bool termIsConstrained = isNotInstallingScript ||
	    (intsct.getSize() < termDom.getSize());

	  switch (varIsLocal + 2 * termIsLocal) {
	  case TRUE + 2 * TRUE: // var and term are local
	    {
#ifdef SCRIPTDEBUG
	      printf("fd-fd local local\n"); fflush(stdout);
#endif
	      if (intsct == fd_singl) {
		TaggedRef int_var = OZ_int(intsct.getSingleElem());
		termVar->propagateUnify(term);
		propagateUnify(var);
		doBind(vPtr, int_var);
		doBind(tPtr, int_var);
		if (disp) { dispose(); termVar->dispose(); }
	      } else if (heapNewer(vPtr, tPtr)) { // bind var to term
		if (intsct == fd_bool) {
		  GenBoolVariable * tbvar = termVar->becomesBool();
		  propagateUnify(var);
		  tbvar->propagateUnify(term);
		  relinkSuspListTo(tbvar);
		  doBind(vPtr, makeTaggedRef(tPtr));
		} else {
		  termVar->setDom(intsct);
		  propagateUnify(var);
		  termVar->propagateUnify(term);
		  relinkSuspListTo(termVar);
		  doBind(vPtr, makeTaggedRef(tPtr));
		}
		if (disp) dispose();
	      } else { // bind term to var
		if (intsct == fd_bool) {
		  GenBoolVariable * bvar = becomesBool();
		  termVar->propagateUnify(term);
		  bvar->propagateUnify(var);
		  termVar->relinkSuspListTo(bvar);
		  doBind(tPtr, makeTaggedRef(vPtr));
		} else {
		  setDom(intsct);
		  termVar->propagateUnify(term);
		  propagateUnify(var);
		  termVar->relinkSuspListTo(this);
		  doBind(tPtr, makeTaggedRef(vPtr));
		}
		if (disp) termVar->dispose();
	      }
	      break;
	    }
	  case TRUE + 2 * FALSE: // var is local and term is global
	    {
#ifdef SCRIPTDEBUG
	      printf("fd-fd local global\n"); fflush(stdout);
#endif
	      if (intsct.getSize() != termDom.getSize()){
		if (intsct == fd_singl) {
		  TaggedRef int_var = OZ_int(intsct.getSingleElem());
		  if (isNotInstallingScript) termVar->propagateUnify(term);
		  if (varIsConstrained) propagateUnify(var);
		  doBind(vPtr, int_var);
		  am.doBindAndTrail(term, tPtr, int_var);
		  if (disp) dispose();
		} else {
		  if (intsct == fd_bool) {
		    GenBoolVariable * bvar = becomesBool();
		    if (isNotInstallingScript) termVar->propagateUnify(term);
		    if (varIsConstrained) bvar->propagateUnify(var);
		    am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(vPtr),
					   bvar, termVar);
		  } else {
		    setDom(intsct);
		    if (isNotInstallingScript) termVar->propagateUnify(term);
		    if (varIsConstrained) propagateUnify(var);
		    am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(vPtr),
					   this, termVar);
		  }
		}
	      } else {
		if (isNotInstallingScript) termVar->propagateUnify(term);
		if (varIsConstrained) propagateUnify(var);
		relinkSuspListTo(termVar, TRUE);
		doBind(vPtr, makeTaggedRef(tPtr));
		if (disp) dispose();
	      }
	      break;
	    }
	  case FALSE + 2 * TRUE: // var is global and term is local
	    {
#ifdef SCRIPTDEBUG
	      printf("fd-fd global local\n"); fflush(stdout);
#endif
	      if (intsct.getSize() != finiteDomain.getSize()){
		if(intsct == fd_singl) {
		  TaggedRef int_term = OZ_int(intsct.getSingleElem());
		  if (isNotInstallingScript) propagateUnify(var);
		  if (termIsConstrained) termVar->propagateUnify(term);
		  doBind(tPtr, int_term);
		  am.doBindAndTrail(var, vPtr, int_term);
		  if (disp) termVar->dispose();
		} else {
		  if (intsct == fd_bool) {
		    GenBoolVariable * tbvar = termVar->becomesBool();
		    if (isNotInstallingScript) propagateUnify(var);
		    if (termIsConstrained) tbvar->propagateUnify(term);
		    am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(tPtr),
					   tbvar, this);
		  } else {
		    termVar->setDom(intsct);
		    if (isNotInstallingScript) propagateUnify(var);
		    if (termIsConstrained) termVar->propagateUnify(term);
		    am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(tPtr),
					   termVar, this);
		  }
		}
	      } else {
		if (termIsConstrained) termVar->propagateUnify(term);
		if (isNotInstallingScript) propagateUnify(var);
		termVar->relinkSuspListTo(this, TRUE);
		doBind(tPtr, makeTaggedRef(vPtr));
		if (disp) termVar->dispose();
	      }
	      break;
	    }
	  case FALSE + 2 * FALSE: // var and term is global
	    {
#ifdef SCRIPTDEBUG
	      printf("fd-fd global global\n"); fflush(stdout);
#endif
	      if (intsct == fd_singl){
		TaggedRef int_val = OZ_int(intsct.getSingleElem());
		if (scp==0) {
		  propagateUnify(var);
		  termVar->propagateUnify(term);
		}
		am.doBindAndTrail(var, vPtr, int_val);
		am.doBindAndTrail(term, tPtr, int_val);
	      } else {
		GenCVariable *c_var;
		if (intsct == fd_bool) 
		  c_var = new GenBoolVariable();
		else
		  c_var = new GenFDVariable(intsct);
		TaggedRef * var_val = newTaggedCVar(c_var);
		if (scp==0) {
		  propagateUnify(var);
		  termVar->propagateUnify(term);
		}
		if (intsct == fd_bool) {
		  am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(var_val),
					 (GenBoolVariable *) c_var, 
					 this);
		  am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(var_val),
					 (GenBoolVariable *) c_var, 
					 termVar);
		} else {
		  am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(var_val),
					 (GenFDVariable *) c_var, 
					 this);
		  am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(var_val),
					 (GenFDVariable *) c_var, 
					 termVar);
		}
	      }
	      break;
	    }
	  default:
	    error("unexpected case in unifyFD");
	    break;
	  } // switch (varIsLocal + 2 * termIsLocal) {
	  return TRUE;
	} 
      case BoolVariable:
	{
	  return tagged2GenBoolVar(term)->unifyBool(tPtr, term, 
						    vPtr, var, 
						    scp, disp);
	}
      case LazyVariable:
	{
	  return
	    ((GenLazyVariable*)
	     tagged2CVar(term))->unifyLazy(tPtr,vPtr,scp);
	}
      default:
	return FALSE;
      } // switch(tagged2CVar(term)->getType())
    default:
      break;
    } // case CVAR
  } // switch( (tTag)
  return FALSE;  
} // GenFDVariable::unify

Bool GenFDVariable::valid(TaggedRef val)
{
  Assert(!isRef(val));
  return (isSmallInt(val) && finiteDomain.isIn(OZ_intToC(val)));
}

void GenFDVariable::relinkSuspListTo(GenBoolVariable * lv, Bool reset_local)
{
  GenCVariable::relinkSuspListTo(lv, reset_local); // any
  for (int i = 0; i < fd_prop_any; i += 1)
    fdSuspList[i] =
      fdSuspList[i]->appendToAndUnlink(lv->suspList, reset_local);
}


void GenFDVariable::relinkSuspListToItself(Bool reset_local)
{
  for (int i = 0; i < fd_prop_any; i += 1)
    fdSuspList[i]->appendToAndUnlink(suspList, reset_local);
}


void GenFDVariable::becomesBoolVarAndPropagate(TaggedRef * trPtr)
{
  if (isGenBoolVar(*trPtr)) return;

  Assert(this == tagged2SuspVar(*trPtr));

  propagate(*trPtr, fd_prop_bounds);
  becomesBool();
}

int GenFDVariable::intersectWithBool(void)
{
  return ((OZ_FiniteDomainImpl *) &finiteDomain)->intersectWithBool();
}

OZ_Return tellBasicConstraint(OZ_Term v, OZ_FiniteDomain * fd)
{
#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tellBasicConstraint - in - : ";
  taggedPrint(v);
  if (fd) cout << " , " << *fd;
  cout << endl <<flush;
#endif

  DEREF(v, vptr, vtag);

  if (fd && (*fd == fd_empty))
    goto failed;


// tell finite domain constraint to unconstrained variable
  if (isNotCVar(vtag)) {
    if (! fd) goto fdvariable;

    // fd is singleton domain --> v becomes integer
    if (fd->getSize() == 1) {
      if (am.isLocalVariable(v, vptr)) {
	if (isSVar(vtag))
	  am.checkSuspensionList(v);
	doBind(vptr, OZ_int(fd->getSingleElem()));
      } else {
	am.doBindAndTrail(v, vptr, OZ_int(fd->getSingleElem()));
      }
      goto proceed;
    }

    GenCVariable * cv;

    // create appropriate constrained variable
    if (*fd == fd_bool) {
      cv = (GenCVariable *) new GenBoolVariable();
    } else {
    fdvariable:
      cv = (GenCVariable *) fd ? new GenFDVariable(*fd) : new GenFDVariable();
    }
    OZ_Term *  tcv = newTaggedCVar(cv);

    if (am.isLocalVariable(v, vptr)) {
      if (isSVar(vtag)) {
	am.checkSuspensionList(v);
	cv->setSuspList(tagged2SVar(v)->getSuspList());
      }
      doBind(vptr, makeTaggedRef(tcv));
    } else { 
      am.doBindAndTrail(v, vptr, makeTaggedRef(tcv));
    }
    
    goto proceed;
// tell finite domain constraint to finite domain variable
  } else if (isGenFDVar(v, vtag)) {
    if (! fd) goto proceed;

    GenFDVariable * fdvar = tagged2GenFDVar(v);
    OZ_FiniteDomain dom = (fdvar->getDom() & *fd);
    
    if (dom == fd_empty) 
      goto failed;

    if (dom.getSize() == fdvar->getDom().getSize()) 
      goto proceed;

    if (dom == fd_singl) {
      if (am.isLocalCVar(v)) {
	fdvar->getDom() = dom;
	fdvar->becomesSmallIntAndPropagate(vptr);
      } else {
	int singl = dom.getSingleElem();
	fdvar->propagate(v, fd_prop_singl);
	am.doBindAndTrail(v, vptr, OZ_int(singl));
      }
    } else if (dom == fd_bool) {
      if (am.isLocalCVar(v)) {
	fdvar->becomesBoolVarAndPropagate(vptr);
      } else {
	fdvar->propagate(v, fd_prop_bounds);
	GenBoolVariable * newboolvar = new GenBoolVariable();
	OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
	am.doBindAndTrailAndIP(v, vptr,
			       makeTaggedRef(newtaggedboolvar),
			       newboolvar, tagged2GenBoolVar(v));
      }
    } else {
      fdvar->propagate(v, fd_prop_bounds);
      if (am.isLocalCVar(v)) {
	fdvar->getDom() = dom;
      } else {
	GenFDVariable * locfdvar = new GenFDVariable(dom);
	OZ_Term * loctaggedfdvar = newTaggedCVar(locfdvar);
	am.doBindAndTrailAndIP(v, vptr,
			       makeTaggedRef(loctaggedfdvar),
			       locfdvar, tagged2GenFDVar(v));
      }
    }
    goto proceed;
// tell finite domain constraint to boolean finite domain variable
  } else if (isGenBoolVar(v, vtag)) {
    if (! fd) goto proceed;

    int dom = fd->intersectWithBool();
    
    if (dom == -2) goto failed;
    if (dom == -1) goto proceed;

    GenBoolVariable * boolvar = tagged2GenBoolVar(v);
    if (am.isLocalCVar(v)) {
      boolvar->becomesSmallIntAndPropagate(vptr, dom);
    } else {
      boolvar->propagate(v);
      am.doBindAndTrail(v, vptr, OZ_int(dom));
    }
    goto proceed;
// tell finite domain constraint to integer, i.e. check for compatibility
  } else if (isSmallInt(vtag)) {
    if (! fd) goto proceed;
    
    if (fd->isIn(smallIntValue(v)))
      goto proceed;
  } 
  
failed:

  return FAILED;

proceed:

#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tellBasicConstraint - out - : ";
  if (vptr) taggedPrint(*vptr); else taggedPrint(v);
  if (fd) cout << " , " << *fd;
  cout << endl <<flush;
#endif

  return PROCEED;
}

#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline
#include "fdgenvar.icc"
#undef inline
#endif

