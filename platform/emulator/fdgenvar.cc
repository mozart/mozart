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
    {
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
    }
  case CVAR:
    {
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
      FDState left_dom = intersection.checkAgainst(finiteDomain);
      FDState right_dom = intersection.checkAgainst(termDom);

// bind - trail - propagate
      Bool varIsLocal = isLocalVariable();
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
            propagate(var, left_dom, tPtr, TRUE);
            termVar->propagate(auxterm, right_dom, tPtr, TRUE);
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
            termVar->propagate(term, right_dom, vPtr, TRUE);
            propagate(auxvar, left_dom, vPtr, TRUE);
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
            termVar->propagate(term, right_dom, vPtr, TRUE);
            propagate(auxvar, left_dom, vPtr, TRUE);
            termVar->addSuspension(new Suspension(am.currentBoard));
            bind(tPtr, term, termIsLocal, vPtr, var);
          } else {
            termVar->propagate(term, right_dom, tPtr, TRUE);
            propagate(var, left_dom, tPtr, TRUE);
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
            propagate(var, left_dom, tPtr, TRUE);
            termVar->propagate(auxterm, right_dom, tPtr, TRUE);
            addSuspension(new Suspension(am.currentBoard));
            bind(vPtr, var, varIsLocal, tPtr, term);
          } else {
            termVar->propagate(term, right_dom, vPtr, TRUE);
            propagate(var, left_dom, vPtr, TRUE);
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
            propagate(var, left_dom, aux, TRUE);
            termVar->propagate(term, right_dom, aux, TRUE);
          } else {
            TaggedRef pn = tagged2CVar(var)->getName();
            aPtr = newTaggedCVar(new GenFDVariable(intersection, pn));
            aux = *aPtr;
            propagate(var, left_dom, aPtr, TRUE);
            termVar->propagate(term, right_dom, aPtr, TRUE);
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
    }
  default:
    break;
  } // switch(tTag)

  return NO;
} // GenFDVariable::unify



Bool GenFDVariable::valid(TaggedRef val)
{
  Assert(!isRef(val));
  return (isSmallInt(val) && finiteDomain.contains(smallIntValue(val)));
}


#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline
#include "fdgenvar.icc"
#undef inline
#endif
