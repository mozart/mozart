/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
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
Bool GenBoolVariable::unifyBool(TaggedRef * vPtr, TaggedRef var,
                                TaggedRef * tPtr, TaggedRef term,
                                ByteCode *scp, Bool disp)
{
#ifdef SCRIPTDEBUG
  printf(am.isInstallingScript() ? "bool installing script\n" : "bool NOT installing script\n"); fflush(stdout);
#endif

  TypeOfTerm tTag = tagTypeOf(term);

  switch (tTag) {
  case SMALLINT:
    {
      int term_val = OZ_intToC(term);
      if (term_val < 0 || 1 < term_val) {
        return FALSE;
      }

      Bool isLocalVar = am.isLocalSVar(this);
      Bool isNotInstallingScript = !am.isInstallingScript();

#ifdef SCRIPTDEBUG
      printf("bool-int %s\n", isLocalVar ? "local" : "global"); fflush(stdout);
#endif

      if (scp==0 && (isNotInstallingScript || isLocalVar)) propagate(var);

      if (am.isLocalSVar(this)) {
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
          Bool isConstrained = ! am.isInstallingScript();
          GenBoolVariable * termvar = tagged2GenBoolVar(term);

          Bool varIsLocal =  am.isLocalSVar(this);
          Bool termIsLocal = am.isLocalSVar(termvar);

          switch (varIsLocal + 2 * termIsLocal) {
          case TRUE + 2 * TRUE: // var and term are local
            {
#ifdef SCRIPTDEBUG
              printf("bool-bool local local\n"); fflush(stdout);
#endif

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
#ifdef SCRIPTDEBUG
              printf("bool-bool local global\n"); fflush(stdout);
#endif

              if (isConstrained) {
                termvar->propagate(term, pc_cv_unif);
                propagate(var, pc_cv_unif);
              }
              relinkSuspListTo(termvar, TRUE);
              doBind(vPtr, makeTaggedRef(tPtr));
              if (disp) dispose();
              break;
            }

          case FALSE + 2 * TRUE: // var is global and term is local
            {
#ifdef SCRIPTDEBUG
              printf("bool-bool global local\n"); fflush(stdout);
#endif

              if (isConstrained) {
                termvar->propagate(term, pc_cv_unif);
                propagate(var, pc_cv_unif);
              }
              termvar->relinkSuspListTo(this, TRUE);
              doBind(tPtr, makeTaggedRef(vPtr));
              if (disp) termvar->dispose();
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
                propagate(var, pc_cv_unif);
                termvar->propagate(term, pc_cv_unif);
              }
              am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(var_val),
                                     bool_var, this);
              am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(var_val),
                                     bool_var, termvar);

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
                termvar->propagate(term, fd_prop_singl, pc_cv_unif);
                propagate(var, pc_cv_unif);
                doBind(vPtr, int_var);
                doBind(tPtr, int_var);
                if (disp) { dispose(); termvar->dispose(); }
              } else if (heapNewer(vPtr, tPtr)) { // bind var to term
                propagate(var, pc_cv_unif);
                termvar->propagate(term, fd_prop_bounds, pc_cv_unif);
                termvar->becomesBool();
                relinkSuspListTo(termvar);
                doBind(vPtr, makeTaggedRef(tPtr));
                if (disp) dispose();
              } else { // bind term to var
                termvar->propagate(term, fd_prop_bounds, pc_cv_unif);
                propagate(var, pc_cv_unif);
                termvar->relinkSuspListTo(this);
                doBind(tPtr, makeTaggedRef(vPtr));
                if (disp) termvar->dispose();
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
                    termvar->propagate(term, fd_prop_singl, pc_cv_unif);
                  if (isConstrainedVar) propagate(var, pc_cv_unif);
                  doBind(vPtr, int_var);
                  am.doBindAndTrail(term, tPtr, int_var);
                  if (disp) dispose();
                } else {
                  if (isNotInstallingScript)
                    termvar->propagate(term, fd_prop_bounds, pc_cv_unif);
                  if (isConstrainedVar) propagate(var, pc_cv_unif);
                  am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(vPtr),
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
                  if (isNotInstallingScript) propagate(var, pc_cv_unif);
                  if (isConstrainedTerm)
                    termvar->propagate(term, fd_prop_singl, pc_cv_unif);
                  doBind(tPtr, int_term);
                  am.doBindAndTrail(var, vPtr, int_term);
                  if (disp) termvar->dispose();
                } else {
                  if (isConstrainedTerm)
                    termvar->propagate(term, fd_prop_bounds, pc_cv_unif);
                  if (isNotInstallingScript) propagate(var, pc_cv_unif);
                  termvar->relinkSuspListTo(this, TRUE);
                  doBind(tPtr, makeTaggedRef(vPtr));
                  if (disp) termvar->dispose();
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
                    propagate(var, pc_cv_unif);
                    termvar->propagate(term, fd_prop_singl, pc_cv_unif);
                  }
                  am.doBindAndTrail(var, vPtr, int_val);
                  am.doBindAndTrail(term, tPtr, int_val);
                } else {
                  GenBoolVariable * bool_var = new GenBoolVariable();
                  TaggedRef * var_val = newTaggedCVar(bool_var);
                  if (scp==0) {
                    propagate(var, pc_cv_unif);
                    termvar->propagate(term, fd_prop_bounds, pc_cv_unif);
                  }
                  am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(var_val),
                                         bool_var, this);
                  am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(var_val),
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
      case LazyVariable:
        {
          return
            ((GenLazyVariable*)
             tagged2CVar(term))->unifyLazy(tPtr,vPtr,scp);
        }
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
