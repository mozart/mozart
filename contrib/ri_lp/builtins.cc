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

#include "ri.hh"
#include "lp.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(ri_newVar, 3)
{
  DECLARE_FLOAT(0, l);
  DECLARE_FLOAT(1, u);

  if (l > u)
    return OZ_FAILED;

  RI ri(l, u);

  return OZ_mkOZ_VAR_CT(OZ_args[2], &ri, ri_definition);
}
OZ_C_proc_end

OZ_C_proc_begin(ri_declVar, 1)
{
  RI ri(RI_FLOAT_MIN, RI_FLOAT_MAX);

  return OZ_mkOZ_VAR_CT(OZ_args[0], &ri, ri_definition);
}
OZ_C_proc_end

OZ_C_proc_begin(ri_setPrecision, 1)
{
  DECLARE_FLOAT(0, p);

  ri_precision = p;

  return PROCEED;
}
OZ_C_proc_end

OZ_BI_define(ri_getLowerBound, 2, 0)
{
  OZ_EXPECTED_TYPE(EM_RI ", FLOAT" );

  RIExpect pe;
  OZ_expect_t r = pe.expectRIVarMinMax(OZ_in(0));
  if (pe.isFailing(r)) {
    return OZ_typeErrorCPI(expectedType, 0, "");
  } else if (pe.isSuspending(r)) {
    return pe.suspend(NULL);
  }

  RIVar ri;

  ri.ask(OZ_in(0));

  double l = ri->lowerBound();

  return OZ_unifyFloat(OZ_in(1), l);
}
OZ_BI_end


OZ_BI_define(ri_getUpperBound, 2, 0)
{
  OZ_EXPECTED_TYPE(EM_RI ", FLOAT" );

  RIExpect pe;
  OZ_expect_t r = pe.expectRIVarMinMax(OZ_in(0));
  if (pe.isFailing(r)) {
    return OZ_typeErrorCPI(expectedType, 0, "");
  } else if (pe.isSuspending(r)) {
    return pe.suspend(NULL);
  }

  RIVar ri;

  ri.ask(OZ_in(0));

  double u = ri->upperBound();

  return OZ_unifyFloat(OZ_in(1), u);
}
OZ_BI_end


OZ_BI_define(ri_getWidth, 2, 0)
{
  OZ_EXPECTED_TYPE(EM_RI ", FLOAT" );

  RIExpect pe;
  OZ_expect_t r = pe.expectRIVarMinMax(OZ_in(0));
  if (pe.isFailing(r)) {
    return OZ_typeErrorCPI(expectedType, 0, "");
  } else if (pe.isSuspending(r)) {
    return pe.suspend(NULL);
  }

  RIVar ri;

  ri.ask(OZ_in(0));

  double w = ri->getWidth();

  return OZ_unifyFloat(OZ_in(1), w);
}
OZ_BI_end


OZ_BI_define(ri_getInf, 1, 0)
{
  return OZ_unifyFloat(OZ_in(0), RI_FLOAT_MIN );
}
OZ_BI_end


OZ_BI_define(ri_getSup, 1, 0)
{
  return OZ_unifyFloat(OZ_in(0), RI_FLOAT_MAX );
}
OZ_BI_end

#define MKARITY(Arity, ArityDef)                        \
OZ_Term Arity = OZ_nil();                               \
for (int i = 0; ArityDef[i] != (OZ_Term) 0; i += 1)     \
  Arity = OZ_cons(ArityDef[i], Arity);

OZ_C_proc_begin(ri_lpsolve_conf, 2)
{
  // check if 1st arg is either `put' or `get'
  int is_put = 0;
  {
    OZ_Term arg1 = OZ_args[0];
    if (OZ_isVariable(arg1)) {
      OZ_suspendOn(arg1);
    } else if (! OZ_isAtom(arg1)) {
      return OZ_typeError(0, "Atom");
    } else {
      arg1 = OZ_deref(arg1);

      if (arg1 == atom_put) {
        is_put = 1;
      } else if (arg1 != atom_get) {
        return OZ_typeError(0, "Atom [`put'|`get']");
      }
    }
  }

  if (is_put) {
    if (OZ_isVariable(OZ_args[1])) {
      OZ_suspendOn(OZ_args[1]);
    }

    OZ_Term mode_term = OZ_subtree(OZ_args[1], atom_mode);
    if (mode_term) {
      if (OZ_isVariable(mode_term)) {
        OZ_suspendOn(mode_term);
      }
      RILPSolve::putModeAtom(mode_term);
    }

    OZ_Term solver_term = OZ_subtree(OZ_args[1], atom_solver);
    if (solver_term) {
      if (OZ_isVariable(solver_term)) {
        OZ_suspendOn(solver_term);
      }
      RILPSolve::putSolverAtom(solver_term);
    }

  } else { // `get' - branch
    int cplex_avail = RILPSolve::cplex_avail();

    OZ_Term solver_list = OZ_cons(atom_lpsolve, OZ_nil());

    if (cplex_avail) {
      solver_list = OZ_cons(atom_cplex_primopt,
                            OZ_cons(atom_cplex_dualopt, solver_list));
    }

    OZ_Term arity_def[] = {
      {OZ_pair2(atom_solver, RILPSolve::getSolverAtom())},
      {OZ_pair2(atom_mode,   RILPSolve::getModeAtom())},
      {OZ_pair2(atom_avail,  solver_list)},
      {(OZ_Term) 0}
    };

    MKARITY(arity, arity_def);

    OZ_Term r = OZ_recordInit(atom_config, arity);

    return OZ_unify(OZ_args[1], r);
  }

  return PROCEED;
}
OZ_C_proc_end
