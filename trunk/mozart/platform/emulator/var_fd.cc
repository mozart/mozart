/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#include "genvar.hh"
#include "fdgenvar.hh"
#include "bignum.hh"


// unify expects either two GenFDVariables or at least one
// GenFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
Bool GenFDVariable::unifyFD(TaggedRef* vPtr, TaggedRef var,  TypeOfTerm vTag,
			    TaggedRef* tPtr, TaggedRef term, TypeOfTerm tTag){
#ifdef PROFILE_FD
  FiniteDomain::unifyCalled++;
#endif
  
  switch (tTag){
  case SMALLINT:
    if (finiteDomain.isInDomain(smallIntValue(term)) == NO)
      return NO;
    propagate(var, det, term);
    bind(vPtr, var, tPtr, term);
    return OK;
    
  case CVAR:
    if (tagged2CVar(term)->getType() != FDVariable)
      return NO;
    
// compute intersection of domains ...
    GenFDVariable* termVar = tagged2GenFDVar(term);
    FiniteDomain &termDom = termVar->finiteDomain;
    LocalFD intersection;
    // don't change the order of the args
    intersection = finiteDomain & termDom; 
// ... and check if resulting domain is empty -> failure
    if (intersection == empty)
      return NO;
    
// bind - trail - propagate
    switch (isLocalVariable() + 2 * termVar->isLocalVariable()) {
    case OK + 2 * OK:
      {
	// var and term are local
	if (tPtr < vPtr){
	  // bind  var to term
	  TaggedRef auxterm = term;
	  if (intersection == singleton)
	    term = *tPtr = newSmallInt(intersection.getSingleton());
	  else
	    termVar->setDom(intersection);
	  propagate(var, FiniteDomain::leftDom, tPtr);
	  termVar->propagate(auxterm, FiniteDomain::rightDom, tPtr);
	  if (isCVar(term) == OK)
	    relinkSuspList(termVar);
	  bind(vPtr, var, tPtr, term);
	} else {
	  // bind term to  var
	  TaggedRef auxvar = var;
	  if (intersection == singleton)
	    var = *vPtr = newSmallInt(intersection.getSingleton());
	  else
	    setDom(intersection);
	  termVar->propagate(term, FiniteDomain::rightDom, vPtr);
	  propagate(auxvar, FiniteDomain::leftDom, vPtr);
	  if (isCVar(var) == OK)
	    termVar->relinkSuspList(this);
	  bind(tPtr, term, vPtr, var);
	}
	break;
      }
    case OK + 2 * NO:
      {
	// var is local and term is global
	if (intersection != termDom){
	  TaggedRef auxvar = var;
	  if (intersection == singleton)
	    var = *vPtr = newSmallInt(intersection.getSingleton());
	  else
	    setDom(intersection);
	  termVar->propagate(term, FiniteDomain::rightDom, vPtr);
	  propagate(auxvar, FiniteDomain::leftDom, vPtr);
	  bind(tPtr, term, vPtr, var);
	} else {
	  termVar->propagate(term, FiniteDomain::rightDom, tPtr);
	  propagate(var, FiniteDomain::leftDom, tPtr);
	  if (isCVar(term) == OK)
	    relinkSuspList(termVar);
	  bind(vPtr, var, tPtr, term);
	}
	break;
      }
    case NO + 2 * OK:
      {
	// var is global and term is local
	if (intersection != finiteDomain){
	  TaggedRef auxterm = term;
	  if(intersection == singleton)
	    term = *tPtr = newSmallInt(intersection.getSingleton());
	  else
	    termVar->setDom(intersection);
	  propagate(var, FiniteDomain::leftDom, tPtr);
	  termVar->propagate(auxterm, FiniteDomain::rightDom, tPtr);
	  bind(vPtr, var, tPtr, term);
	} else {
	  termVar->propagate(term, FiniteDomain::rightDom, vPtr);
	  propagate(var, FiniteDomain::leftDom, vPtr);
	  if (isCVar(var) == OK)
	    termVar->relinkSuspList(this);
	  bind(tPtr, term, vPtr, var);
	}
	break;
      }
    case NO + 2 * NO:
      {
	// var and term is global
	TaggedRef *aPtr, aux;
	if (intersection == singleton){
	  aPtr = NULL;
	  aux = newSmallInt(intersection.getSingleton());
	  propagate(var, FiniteDomain::leftDom, aux);
	  termVar->propagate(term, FiniteDomain::rightDom, aux);
	} else {
	  TaggedRef pn = tagged2CVar(var)->getName();
	  aPtr = newTaggedCVar(new GenFDVariable(intersection, pn));
	  aux = *aPtr;
	  propagate(var, FiniteDomain::leftDom, aPtr);
	  termVar->propagate(term, FiniteDomain::rightDom, aPtr);
	}
	bind(vPtr, var,  aPtr, aux);
	bind(tPtr, term, aPtr, aux);
	break;
      }
    default:
      error("Unexpected case at %s:%d.", __FILE__, __LINE__);
      break;
    } // switch
    return OK;
    
  default:
    break;
  } // switch(tTag)
  
  return NO;  
} // GenFDVariable::unify


// - 'table' holds the code to branch to when indexing
// - 'elseLabel' is the PC of the ELSE-branch, ie. in case there is no
//   clause to switch to
// How it works:
// If none of the numbers is member of the domain, no guard can ever be
// entailed therefore goto to the else-branch. Otherwise goto varLabel, which
// wait for determination of the variable. Usually if unconstrained variables
// get bound to each other, det-nodes are not reentered, but since
// unifying two fd variables may result in a singleton (ie. determined term),
// det-nodes are reentered and we achieve completeness.

ProgramCounter GenFDVariable::index(ProgramCounter elseLabel,
				    IHashTable* table)
{
  // if there are no integer guards goto else-branch
  if (table->numberTable) {
    HTEntry** aux_table = table->numberTable;
    int size = table->size;

    // if there is at least one integer member of the domain then goto varLabel
    for (int i = 0; i < size; i++) {
      HTEntry* aux_entry = aux_table[i];
      while (aux_entry) {
	if (isSmallInt(aux_entry->number))
	  if (finiteDomain.isInDomain(smallIntValue(aux_entry->number)))
	    return table->varLabel;
	aux_entry = aux_entry->getNext();
      } // while
    } // for
  }
  
  return elseLabel;
} // GenFDVariable::index


//-----------------------------------------------------------------------------
//                   Predicates for suspension lists


//typedef Bool (*CondFunc)(TaggedRef, TaggedRef, TaggedRef);

// Returns OK, if an unconstrained variable gets constrained in some way,
// otherwise NO
Bool isGettingConstrained(TaggedRef oldVal,TaggedRef newVal, TaggedRef){
  DEREF(newVal, newValPtr, newValTag);

  return (isNotCVar(newVal) == OK) ? NO : OK;
}


// Returns OK, if unconstrained variable gets bound to another
// variable or a term, otherwise NO.
Bool isGettingBound(TaggedRef oldVal, TaggedRef newVal, TaggedRef arg){
  DEREF(newVal, newValPtr, newValTag);

  return ((TaggedRef) newValPtr == arg) ? NO : OK;
}


// Returns OK, if a variable gets determined
Bool isDet(TaggedRef oldVal, TaggedRef newVal, TaggedRef){
  DEREF(newVal, newValPtr, newValTag);

  return (isAnyVar(newValTag) == OK) ? NO : OK;
}

// Returns OK, if the lower bound of newVal is bigger than arg.i,
// otherwise NO
Bool lowerBound(TaggedRef oldVal, TaggedRef newVal, TaggedRef arg){
  DEREF(newVal, newValPtr, newValTag);

  if (isNotCVar(newVal) == OK)
    return NO;
  if (isGenFDVar(newVal) == OK){
    return (tagged2GenFDVar(newVal)->getDom().minElem() > smallIntValue(arg))
      ? OK : NO;
  } else
    return OK;
}

// Returns  OK, if the uppper bound of newVal is smaller than arg.i,
// otherwise NO
Bool upperBound(TaggedRef oldVal, TaggedRef newVal, TaggedRef arg){
  DEREF(newVal, newValPtr, newValTag);

  if (isNotCVar(newVal) == OK)
    return NO;
  if (isGenFDVar(newVal) == OK){
    return (tagged2GenFDVar(newVal)->getDom().maxElem() < smallIntValue(arg))
      ? OK : NO;
  } else
    return OK;
}

// Return OK, if newValPtr == arg.trPtr (modulo dereferencing),
// otherwise NO
Bool eqVar(TaggedRef oldVal, TaggedRef newVal, TaggedRef arg){
  DEREF(newVal, newValPtr, newValTag);

  if (arg == 0) 
    return NO;
  DEREF(arg, argPtr, argTag);
  return ((newValPtr == argPtr) && isAnyVar(newValTag)) ? OK : NO;
}

// Returns OK, if cardinality was changed or term has no cardinality,
// otherwise NO
Bool hasSmallerCard(TaggedRef oldVal, TaggedRef newVal, TaggedRef arg){
  DEREF(newVal, newValPtr, newValTag);

  if (isNotCVar(newVal) == OK)
    return NO;
  if (isGenFDVar(newVal) == OK){
    return (tagged2GenFDVar(newVal)->getDom().getSize() < smallIntValue(arg))
      ? OK : NO;
  } else
    return OK;
}


OZ_Bool fdDomainConstrain(TaggedRef &var, TaggedRef* &varPtr,
			      FiniteDomain &domain)
{
  if (domain == empty) return FAILED;

  TaggedRef vName;
  
  switch (tagTypeOf(var)) { 
  case SMALLINT:
    return domain.isInDomain(smallIntValue(var)) == OK ? PROCEED : FAILED;

  case CVAR:
    if (isGenFDVar(var) == NO) goto do_unification;

    {
      GenFDVariable *fdvar = tagged2GenFDVar(var);
      FiniteDomain &vardom = fdvar->getDom();
      // don't change the order 
      domain = domain & vardom;
      if (domain == empty)
	return FAILED;
      
      if (domain == vardom)
	return PROCEED;
      
      if (fdvar->isLocalVariable() == OK){
	
	if (domain == singleton)
	  *varPtr = newSmallInt(domain.getSingleton());
	else 
	  vardom = domain;
	
	GenCVariable::unifyGenCVariables = NO;
	fdvar->propagate(var, FiniteDomain::rightDom, varPtr);
	GenCVariable::unifyGenCVariables = OK;
	return PROCEED;
      } else {
	vName = fdvar->getName();
	goto do_unification;
      } 
    }

  case SVAR:
    vName = tagged2SVar(var)->getName();
    goto do_unification;

  case UVAR:
    vName = AtomVoid; 
    goto do_unification;
    
  do_unification:
    {
      TaggedRef auxvar;
      
      if (domain == singleton) {
	auxvar = newSmallInt(domain.getSingleton());
      } else {
	auxvar = makeTaggedRef(newTaggedCVar(new GenFDVariable(domain,vName)));
#ifdef DEBUG_CHECK
	if (varPtr == NULL)
	  error("varPtr unexpected NULL.");
#endif
      }
      GenCVariable::unifyGenCVariables = NO;
      if (OZ_unify(auxvar, makeTaggedRef(varPtr))) {
	GenCVariable::unifyGenCVariables = OK;
	while (isRef(*varPtr)) varPtr = (TaggedRef*) *varPtr;
	var = *varPtr;
	return PROCEED;
      }
      GenCVariable::unifyGenCVariables = OK;
      return FAILED;
    }
  default:
    return FAILED;
  } // switch
} // fdDomainConstrain


#ifdef OUTLINE
#define inline
#include "fdgenvar.icc"
#undef inline
#endif
