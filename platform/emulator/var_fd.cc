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
      if (! finiteDomain.contains(smallIntValue(term))) return FALSE;
      propagate(var, fd_det, term, TRUE);
      Bool varIsLocal = isLocalVariable();
      if (! varIsLocal) addSuspension(new Suspension(am.currentBoard));

      doBindAndTrail(var, vPtr, term, varIsLocal);
      return TRUE;
    }
  case CVAR:
    {
#ifndef CVAR_ONLY_FDVAR
      if (tagged2CVar(term)->getType() != FDVariable) return FALSE;
#endif

// compute intersection of domains ...
      GenFDVariable* termVar = tagged2GenFDVar(term);
      FiniteDomain &termDom = termVar->finiteDomain;
      LocalFD intsct;

      if ((intsct = finiteDomain & termDom) == fd_empty) return FALSE;
      FDPropState left_dom = intsct.checkAgainst(finiteDomain);
      FDPropState right_dom = intsct.checkAgainst(termDom);

// bind - trail - propagate
      Bool varIsLocal = isLocalVariable();
      Bool termIsLocal = termVar->isLocalVariable();
      switch (varIsLocal + 2 * termIsLocal) {
      case TRUE + 2 * TRUE: // var and term are local
        {
          if (tPtr < vPtr) { // bind var to term
            if (intsct == fd_singleton) {
              TaggedRef int_term = newSmallInt(intsct.singl());
              propagate(var, left_dom, int_term, TRUE);
              termVar->propagate(term, right_dom, int_term, TRUE);
              doBind(vPtr, int_term);
            } else {
              termVar->setDom(intsct);
              propagate(var, left_dom, TaggedRef(tPtr), TRUE);
              termVar->propagate(term, right_dom, TaggedRef(vPtr), TRUE);
              relinkSuspList(termVar);
              doBind(vPtr, TaggedRef(tPtr));
            }
          } else { // bind term to var
            if (intsct == fd_singleton) {
              TaggedRef int_var = newSmallInt(intsct.singl());
              termVar->propagate(term, right_dom, int_var, TRUE);
              propagate(var, left_dom, int_var, TRUE);
              doBind(tPtr, int_var);
            } else {
              setDom(intsct);
              termVar->propagate(term, right_dom, TaggedRef(vPtr), TRUE);
              propagate(var, left_dom, TaggedRef(vPtr), TRUE);
              termVar->relinkSuspList(this);
              doBind(tPtr, TaggedRef(vPtr));
            }
          }
          break;
        }
      case TRUE + 2 * FALSE: // var is local and term is global
        {
          if (intsct.getSize() != termDom.getSize()){
            if (intsct == fd_singleton) {
              TaggedRef int_var = newSmallInt(intsct.singl());
              termVar->propagate(term, right_dom, int_var, TRUE);
              propagate(var, left_dom, int_var, TRUE);
              termVar->addSuspension(new Suspension(am.currentBoard));
              doBindAndTrail(term, tPtr, int_var);
            } else {
              setDom(intsct);
              termVar->propagate(term, right_dom, TaggedRef(vPtr), TRUE);
              propagate(var, left_dom, TaggedRef(vPtr), TRUE);
              termVar->addSuspension(new Suspension(am.currentBoard));
              doBindAndTrail(term, tPtr, TaggedRef(vPtr));
            }
          } else {
            termVar->propagate(term, right_dom, TaggedRef(tPtr), TRUE);
            propagate(var, left_dom, TaggedRef(tPtr), TRUE);
            doBind(vPtr, TaggedRef(tPtr));
          }
          break;
        }
      case FALSE + 2 * TRUE: // var is global and term is local
        {
          if (intsct.getSize() != finiteDomain.getSize()){
            if(intsct == fd_singleton) {
              TaggedRef int_term = newSmallInt(intsct.singl());
              propagate(var, left_dom, int_term, TRUE);
              termVar->propagate(term, right_dom, int_term, TRUE);
              addSuspension(new Suspension(am.currentBoard));
              doBindAndTrail(var, vPtr, int_term);
            } else {
              termVar->setDom(intsct);
              propagate(var, left_dom, TaggedRef(tPtr), TRUE);
              termVar->propagate(term, right_dom, TaggedRef(tPtr), TRUE);
              addSuspension(new Suspension(am.currentBoard));
              doBindAndTrail(var, vPtr, TaggedRef(tPtr));
            }
          } else {
            termVar->propagate(term, right_dom, TaggedRef(vPtr), TRUE);
            propagate(var, left_dom, TaggedRef(vPtr), TRUE);
            termVar->relinkSuspList(this);
            doBind(tPtr, TaggedRef(vPtr));
          }
          break;
        }
      case FALSE + 2 * FALSE: // var and term is global
        {
          if (intsct == fd_singleton){
            TaggedRef int_val = newSmallInt(intsct.singl());
            propagate(var, left_dom, int_val, TRUE);
            termVar->propagate(term, right_dom, int_val, TRUE);
            doBindAndTrail(var, vPtr, int_val);
            doBindAndTrail(term, tPtr, int_val);
          } else {
            TaggedRef pn = tagged2CVar(var)->getName();
            TaggedRef * var_val = newTaggedCVar(new GenFDVariable(intsct, pn));
            propagate(var, left_dom, TaggedRef(var_val), TRUE);
            termVar->propagate(term, right_dom, TaggedRef(var_val), TRUE);
            doBindAndTrail(var, vPtr, TaggedRef(var_val));
            doBindAndTrail(term, tPtr, TaggedRef(var_val));
          }
          Suspension * susp = new Suspension(am.currentBoard);
          termVar->addSuspension(susp);
          addSuspension(susp);
          break;
        }
      default:
        error("unexpected case in unifyFD");
        break;
      } // switch
      return TRUE;
    }
  default:
    break;
  } // switch

  return FALSE;
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
