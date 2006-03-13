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

#include "misc.hh"

double * getDoubleVector(OZ_Term t, double * v)
{
  int i = 0;

  if (OZ_isLiteral(t)) {

    ;

  } if (OZ_isCons(t)) {

    for (; OZ_isCons(t); t = OZ_tail(t))
      v[i++] = OZ_floatToC(OZ_head(t));

  } else if (OZ_isTuple(t)) {

    for (int sz = OZ_width(t); i < sz; i += 1)
      v[i] = OZ_floatToC(OZ_getArg(t, i));

  } else if (OZ_isRecord(t)) {

    OZ_Term al = OZ_arityList(t);

    for (; OZ_isCons(al); al = OZ_tail(al))
      v[i++] = OZ_floatToC(OZ_subtree(t, OZ_head(al)));

  } else {
    OZ_warning("getFloatVector: Unexpected term, expected vector.");
    return NULL;
  }
  return v + i;
}

#ifdef LINUX_IEEE
void exception_handler(int i, siginfo_t * info, ucontext_mozart_t * fpu_state)
{
  static char *msg[6] = {"invalid operation", "denormal", "divide by zero",
                         "underflow", "overflow", "precision loss"};

        fprintf(stderr, "Floating point error: %s\n", msg[info->si_code]);
}
#endif

OZ_Term atom_row, atom_opt, atom_type, atom_rhs, atom_min, atom_max,
  atom_optimal, atom_infeasible, atom_unbounded,  atom_failure, atom_le,
  atom_ge, atom_eq, atom_oops, atom_solver, atom_mode, atom_avail, atom_config,
  atom_lpsolve, atom_cplex_primopt, atom_cplex_dualopt, atom_put, atom_get,
  atom_quiet, atom_verbose;
