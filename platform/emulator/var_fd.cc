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
#include "bignum.hh"
#include "fdprofil.hh"

// unify expects either two GenFDVariables or at least one
// GenFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
Bool GenFDVariable::unifyFD(TaggedRef *vPtr, TaggedRef var,
                            TaggedRef *tPtr, TaggedRef term,
                            Bool prop, Bool disp)
{
  TypeOfTerm vTag = tagTypeOf(var);
  TypeOfTerm tTag = tagTypeOf(term);

  switch (tTag){
  case SMALLINT:
    {
      if (! finiteDomain.contains(smallIntValue(term))) {
        PROFILE_CODE1(FDProfiles.inc_item(no_failed_fdunify_vars);)
        return FALSE;
      }
      if (prop) propagate(var, fd_det, term, pc_propagator);

      if (prop && isLocalVariable()) {
        doBind(vPtr, term);
        if (disp) dispose();
      } else {
        if (prop) addSuspension(new Suspension(am.currentBoard));
        doBindAndTrail(var, vPtr, term);
      }

      PROFILE_CODE1(if (FDVarsTouched.add(term))
                      FDProfiles.inc_item(no_touched_vars);
                    FDProfiles.inc_item(no_succ_fdunify_vars);
                    )
      return TRUE;
    }
  case CVAR:
    {
      if (tagged2CVar(term)->getType() != FDVariable) return FALSE;

// compute intersection of domains ...
      GenFDVariable* termVar = tagged2GenFDVar(term);
      FiniteDomain &termDom = termVar->finiteDomain;
      LocalFD intsct;

      if ((intsct = finiteDomain & termDom) == fd_empty) {
        PROFILE_CODE1(FDProfiles.inc_item(no_failed_fdunify_vars);)
        return FALSE;
      }
      FDPropState l_dom = intsct.checkAgainst(finiteDomain);
      FDPropState r_dom = intsct.checkAgainst(termDom);

      PROFILE_CODE1(if (l_dom != fd_any)
                      if (FDVarsTouched.add(var))
                        FDProfiles.inc_item(no_touched_vars);
                    if (r_dom != fd_any)
                      if (FDVarsTouched.add(term))
                        FDProfiles.inc_item(no_touched_vars);
                    FDProfiles.inc_item(no_succ_fdunify_vars);
                    )
// bind - trail - propagate
      Bool varIsLocal =  (prop && isLocalVariable());
      Bool termIsLocal = (prop && termVar->isLocalVariable());
      switch (varIsLocal + 2 * termIsLocal) {
      case TRUE + 2 * TRUE: // var and term are local
        {
          if (heapNewer(vPtr, tPtr)) { // bind var to term
            if (intsct == fd_singleton) {
              TaggedRef int_term = newSmallInt(intsct.singl());
              propagate(var, l_dom, int_term, pc_cv_unif);
              termVar->propagate(term, r_dom, int_term, pc_cv_unif);
              doBind(tPtr, int_term);
              doBind(vPtr, TaggedRef(tPtr));
              if (disp) {
                dispose();
                termVar->dispose();
              }
            } else {
              termVar->setDom(intsct);
              propagate(var, l_dom, TaggedRef(tPtr), pc_cv_unif);
              termVar->propagate(term, r_dom, TaggedRef(vPtr), pc_cv_unif);
              relinkSuspListTo(termVar);
              doBind(vPtr, TaggedRef(tPtr));
              if (disp) dispose();
            }
          } else { // bind term to var
            if (intsct == fd_singleton) {
              TaggedRef int_var = newSmallInt(intsct.singl());
              termVar->propagate(term, r_dom, int_var, pc_cv_unif);
              propagate(var, l_dom, int_var, pc_cv_unif);
              doBind(vPtr, int_var);
              doBind(tPtr, TaggedRef(vPtr));
              if (disp) {
                dispose();
                termVar->dispose();
              }
            } else {
              setDom(intsct);
              termVar->propagate(term, r_dom, TaggedRef(vPtr), pc_cv_unif);
              propagate(var, l_dom, TaggedRef(vPtr), pc_cv_unif);
              termVar->relinkSuspListTo(this);
              doBind(tPtr, TaggedRef(vPtr));
              if (disp) termVar->dispose();
            }
          }
          break;
        }
      case TRUE + 2 * FALSE: // var is local and term is global
        {
          if (intsct.getSize() != termDom.getSize()){
            if (intsct == fd_singleton) {
              TaggedRef int_var = newSmallInt(intsct.singl());
              termVar->propagate(term, r_dom, int_var, pc_cv_unif);
              propagate(var, l_dom, int_var, pc_cv_unif);
              termVar->addSuspension(new Suspension(am.currentBoard));
              doBind(vPtr, int_var);
              doBindAndTrail(term, tPtr, TaggedRef(vPtr));
              if (disp) dispose();
            } else {
              setDom(intsct);
              termVar->propagate(term, r_dom, TaggedRef(vPtr), pc_cv_unif);
              propagate(var, l_dom, TaggedRef(vPtr), pc_cv_unif);
              termVar->addSuspension(new Suspension(am.currentBoard));
              doBindAndTrail(term, tPtr, TaggedRef(vPtr));
            }
          } else {
            termVar->propagate(term, r_dom, TaggedRef(tPtr), pc_cv_unif);
            propagate(var, l_dom, TaggedRef(tPtr), pc_cv_unif);
            relinkSuspListTo(termVar, TRUE);
            doBind(vPtr, TaggedRef(tPtr));
            if (disp) dispose();
          }
          break;
        }
      case FALSE + 2 * TRUE: // var is global and term is local
        {
          if (intsct.getSize() != finiteDomain.getSize()){
            if(intsct == fd_singleton) {
              TaggedRef int_term = newSmallInt(intsct.singl());
              propagate(var, l_dom, int_term, pc_cv_unif);
              termVar->propagate(term, r_dom, int_term, pc_cv_unif);
              addSuspension(new Suspension(am.currentBoard));
              doBind(tPtr, int_term);
              doBindAndTrail(var, vPtr, TaggedRef(tPtr));
              if (disp) termVar->dispose();
            } else {
              termVar->setDom(intsct);
              propagate(var, l_dom, TaggedRef(tPtr), pc_cv_unif);
              termVar->propagate(term, r_dom, TaggedRef(tPtr), pc_cv_unif);
              addSuspension(new Suspension(am.currentBoard));
              doBindAndTrail(var, vPtr, TaggedRef(tPtr));
            }
          } else {
            termVar->propagate(term, r_dom, TaggedRef(vPtr), pc_cv_unif);
            propagate(var, l_dom, TaggedRef(vPtr), pc_cv_unif);
            termVar->relinkSuspListTo(this, TRUE);
            doBind(tPtr, TaggedRef(vPtr));
            if (disp) termVar->dispose();
          }
          break;
        }
      case FALSE + 2 * FALSE: // var and term is global
        {
          if (intsct == fd_singleton){
            TaggedRef int_val = newSmallInt(intsct.singl());
            if (prop) {
              propagate(var, l_dom, int_val, pc_cv_unif);
              termVar->propagate(term, r_dom, int_val, pc_cv_unif);
            }
            doBindAndTrail(var, vPtr, int_val);
            doBindAndTrail(term, tPtr, TaggedRef(vPtr));
          } else {
            TaggedRef pn = tagged2CVar(var)->getName();
            TaggedRef * var_val = newTaggedCVar(new GenFDVariable(intsct, pn));
            if (prop) {
              propagate(var, l_dom, TaggedRef(var_val), pc_cv_unif);
              termVar->propagate(term, r_dom, TaggedRef(var_val), pc_cv_unif);
            }
            doBindAndTrail(var, vPtr, TaggedRef(var_val));
            doBindAndTrail(term, tPtr, TaggedRef(var_val));
          }
          if (prop) {
            Suspension * susp = new Suspension(am.currentBoard);
            termVar->addSuspension(susp);
            addSuspension(susp);
          }
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
