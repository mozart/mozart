/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(__GNUC__)
#pragma implementation "fdgenvar.hh"
#endif

#include "genvar.hh"
#include "fdgenvar.hh"
#include "bignum.hh"


// unify expects either two GenFDVariables or at least one
// GenFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
Bool GenFDVariable::unifyFD(TaggedRef * vPtr, TaggedRef var,  TypeOfTerm vTag,
                            TaggedRef * tPtr, TaggedRef term, TypeOfTerm tTag)
{
  switch (tTag){
  case SMALLINT:
    if (finiteDomain.contains(smallIntValue(term)) == NO)
      return NO;
    propagate(var, det, term);
    Bool varIsLocal = isLocalVariable();
    if (varIsLocal == NO) {
      Suspension * susp = new Suspension(am.currentBoard);
      addSuspension(susp);
    }
    bind(vPtr, var, varIsLocal, tPtr, term);
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
    varIsLocal = isLocalVariable();
    Bool termIsLocal = termVar->isLocalVariable();
    switch (varIsLocal + 2 * termIsLocal) {
    case OK + 2 * OK:
      {
        // var and term are local
        if (tPtr < vPtr){
          // bind  var to term
          TaggedRef auxterm = term;
          if (intersection == singleton)
            term = *tPtr = newSmallInt(intersection.singl());
          else
            termVar->setDom(intersection);
          propagate(var, FiniteDomain::leftDom, tPtr);
          termVar->propagate(auxterm, FiniteDomain::rightDom, tPtr);
          if (isCVar(term) == OK)
            relinkSuspList(termVar);
          bind(vPtr, var, varIsLocal, tPtr, term);
        } else {
          // bind term to  var
          TaggedRef auxvar = var;
          if (intersection == singleton)
            var = *vPtr = newSmallInt(intersection.singl());
          else
            setDom(intersection);
          termVar->propagate(term, FiniteDomain::rightDom, vPtr);
          propagate(auxvar, FiniteDomain::leftDom, vPtr);
          if (isCVar(var) == OK)
            termVar->relinkSuspList(this);
          bind(tPtr, term, termIsLocal, vPtr, var);
        }
        break;
      }
    case OK + 2 * NO:
      {
        // var is local and term is global
        if (intersection != termDom){
          TaggedRef auxvar = var;
          if (intersection == singleton)
            var = *vPtr = newSmallInt(intersection.singl());
          else
            setDom(intersection);
          termVar->propagate(term, FiniteDomain::rightDom, vPtr);
          propagate(auxvar, FiniteDomain::leftDom, vPtr);
          termVar->addSuspension(new Suspension(am.currentBoard));
          bind(tPtr, term, termIsLocal, vPtr, var);
        } else {
          termVar->propagate(term, FiniteDomain::rightDom, tPtr);
          propagate(var, FiniteDomain::leftDom, tPtr);
          if (isCVar(term) == OK)
            relinkSuspList(termVar);
          bind(vPtr, var, varIsLocal, tPtr, term);
        }
        break;
      }
    case NO + 2 * OK:
      {
        // var is global and term is local
        if (intersection != finiteDomain){
          TaggedRef auxterm = term;
          if(intersection == singleton)
            term = *tPtr = newSmallInt(intersection.singl());
          else
            termVar->setDom(intersection);
          propagate(var, FiniteDomain::leftDom, tPtr);
          termVar->propagate(auxterm, FiniteDomain::rightDom, tPtr);
          addSuspension(new Suspension(am.currentBoard));
          bind(vPtr, var, varIsLocal, tPtr, term);
        } else {
          termVar->propagate(term, FiniteDomain::rightDom, vPtr);
          propagate(var, FiniteDomain::leftDom, vPtr);
          if (isCVar(var) == OK)
            termVar->relinkSuspList(this);
          bind(tPtr, term, termIsLocal, vPtr, var);
        }
        break;
      }
    case NO + 2 * NO:
      {
        // var and term is global
        TaggedRef *aPtr, aux;
        if (intersection == singleton){
          aPtr = NULL;
          aux = newSmallInt(intersection.singl());
          propagate(var, FiniteDomain::leftDom, aux);
          termVar->propagate(term, FiniteDomain::rightDom, aux);
        } else {
          TaggedRef pn = tagged2CVar(var)->getName();
          aPtr = newTaggedCVar(new GenFDVariable(intersection, pn));
          aux = *aPtr;
          propagate(var, FiniteDomain::leftDom, aPtr);
          termVar->propagate(term, FiniteDomain::rightDom, aPtr);
        }
        Suspension* susp = new Suspension(am.currentBoard);
        termVar->addSuspension(susp);
        addSuspension(susp);
        bind(vPtr, var,  varIsLocal, aPtr, aux);
        bind(tPtr, term, termIsLocal, aPtr, aux);
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
          if (finiteDomain.contains(smallIntValue(aux_entry->number)))
            return table->varLabel;
        aux_entry = aux_entry->getNext();
      } // while
    } // for
  }

  return elseLabel;
} // GenFDVariable::index


OZ_Bool fdDomainConstrain(TaggedRef &var, TaggedRef* &varPtr,
                              FiniteDomain &domain)
{
  if (domain == empty) return FAILED;

  TaggedRef vName;

  switch (tagTypeOf(var)) {
  case SMALLINT:
    return domain.contains(smallIntValue(var)) == OK ? PROCEED : FAILED;

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
          *varPtr = newSmallInt(domain.singl());
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
        auxvar = newSmallInt(domain.singl());
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


#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline
#include "fdgenvar.icc"
#undef inline
#endif
