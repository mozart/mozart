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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_bool.hh"
#endif


#include "var_bool.hh"
#include "var_fd.hh"
#include "unify.hh"

OZ_Return OzBoolVariable::bind(TaggedRef * vPtr, TaggedRef term, ByteCode *scp)
{
  Assert(!oz_isRef(term));
  if (!oz_isSmallInt(term)) return FAILED;

  int term_val = OZ_intToC(term);
  if (term_val < 0 || 1 < term_val) {
    return FAILED;
  }

  Bool isLocalVar = oz_isLocalVar(this);
  Bool isNotInstallingScript = !am.isInstallingScript();

#ifdef SCRIPTDEBUG
  printf("bool-int %s\n", isLocalVar ? "local" : "global"); fflush(stdout);
#endif

  if (scp==0 && (isNotInstallingScript || isLocalVar)) propagate();

  if (oz_isLocalVar(this)) {
    DoBind(vPtr, term);
    dispose();
  } else {
    DoBindAndTrail(vPtr, term);
  }

  return PROCEED;
}

// unify expects either two OzFDVariables or at least one
// OzFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
OZ_Return OzBoolVariable::unify(TaggedRef * vPtr, TaggedRef *tPtr,
                                ByteCode *scp)
{
#ifdef SCRIPTDEBUG
  printf(am.isInstallingScript()
         ? "bool installing script\n"
         : "bool NOT installing script\n");
  fflush(stdout);
#endif

  TaggedRef term = *tPtr;
  OzVariable *cv=tagged2CVar(term);
  switch (cv->getType()) {
  case OZ_VAR_BOOL:
    {
      Bool isConstrained = ! am.isInstallingScript();
      OzBoolVariable * termvar = (OzBoolVariable *)cv;

      Bool varIsLocal =  oz_isLocalVar(this);
      Bool termIsLocal = oz_isLocalVar(termvar);

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
            DoBind(vPtr, makeTaggedRef(tPtr));
            dispose();
          } else { // bind term to var
            termvar->propagate(pc_cv_unif);
            propagate(pc_cv_unif);
            termvar->relinkSuspListTo(this);
            DoBind(tPtr, makeTaggedRef(vPtr));
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
          DoBind(vPtr, makeTaggedRef(tPtr));
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
          DoBind(tPtr, makeTaggedRef(vPtr));
          termvar->dispose();
          break;
        }

      case FALSE + 2 * FALSE: // var and term is global
        {
#ifdef SCRIPTDEBUG
          printf("bool-bool global global\n"); fflush(stdout);
#endif

          OzBoolVariable * bool_var
            = new OzBoolVariable(oz_currentBoard());
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
        OZ_error("unexpected case in unifyBool Bool <--> Bool");
        break;
      } // switch
      return TRUE;
    }
  case OZ_VAR_FD:
    {
      OzFDVariable * termvar = (OzFDVariable *)cv;

      int intsct = termvar->intersectWithBool();

      if (intsct == -2) return FAILED;

      Bool isNotInstallingScript = ! am.isInstallingScript();
      Bool isConstrainedVar = isNotInstallingScript || (intsct != -1);
      Bool isConstrainedTerm = isNotInstallingScript;

      Bool varIsLocal =  oz_isLocalVar(this);
      Bool termIsLocal = oz_isLocalVar(termvar);

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
            DoBind(vPtr, int_var);
            DoBind(tPtr, int_var);
            dispose(); termvar->dispose();
          } else if (heapNewer(vPtr, tPtr)) { // bind var to term
            propagate(pc_cv_unif);
            termvar->propagate(fd_prop_bounds, pc_cv_unif);
            termvar->becomesBool();
            relinkSuspListTo(termvar);
            DoBind(vPtr, makeTaggedRef(tPtr));
            dispose();
          } else { // bind term to var
            termvar->propagate(fd_prop_bounds, pc_cv_unif);
            propagate(pc_cv_unif);
            termvar->relinkSuspListTo(this);
            DoBind(tPtr, makeTaggedRef(vPtr));
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
            DoBind(vPtr, int_var);
            DoBindAndTrail(tPtr, int_var);
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
            DoBind(tPtr, int_term);
            DoBindAndTrail(vPtr, int_term);
            termvar->dispose();
          } else {
            if (isConstrainedTerm)
              termvar->propagate(fd_prop_bounds, pc_cv_unif);
            if (isNotInstallingScript) propagate(pc_cv_unif);
            termvar->relinkSuspListTo(this, TRUE);
            DoBind(tPtr, makeTaggedRef(vPtr));
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
            DoBindAndTrail(vPtr, int_val);
            DoBindAndTrail(tPtr, int_val);
          } else {
            OzBoolVariable * bool_var
              = new OzBoolVariable(oz_currentBoard());
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
        OZ_error("unexpected case in unifyBool Bool <--> FD");
        break;
      } // switch
    }
    return TRUE;
  default:
    break;
  }

  return FALSE;
} // OzBoolVariable::unify



Bool OzBoolVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  if (oz_isSmallInt(val)) {
    int intval = OZ_intToC(val);
    return (intval == 0 || intval == 1);
  }
  return FALSE;
}

OZ_Return tellBasicBoolConstraint(OZ_Term t)
{
  OZ_FiniteDomain bool_dom(fd_bool);
  return tellBasicConstraint(t, &bool_dom);
}

#ifdef OUTLINE
#define inline
#include "var_bool.icc"
#undef inline
#endif
