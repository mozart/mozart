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
#include "propagators.hh"
#include "lp.hh"

//-----------------------------------------------------------------------------

RIDefinition * ri_definition;

int RIDefinition::_kind;

char ** RIDefinition::getNamesOfWakeUpLists(void)
{
  static char * names[2] = {"lower", "upper"};

  return names;
}


ri_float ri_precision = 1e-6;

void RIProfile::init(OZ_Ct * c)
{
  RI * ri = (RI *) c;
  _l = ri->_l;
  _u = ri->_u;
}

//-----------------------------------------------------------------------------

extern "C"
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table [] = {
      {"newVar",        3, 0, ri_newVar},
      {"declVar",       1, 0, ri_declVar},
      {"setPrecision",  1, 0, ri_setPrecision},
      {"lessEq",        2, 0, ri_lessEq},
      {"greater",       2, 0, ri_greater},
      {"getLowerBound", 2, 0, ri_getLowerBound},
      {"getUpperBound", 2, 0, ri_getUpperBound},
      {"getWidth",      2, 0, ri_getWidth},
      {"getInf",        1, 0, ri_getInf},
      {"getSup",        1, 0, ri_getSup},
      {"intBounds",     2, 0, ri_intBounds},
      {"intBoundsSPP",  2, 0, ri_intBoundsSPP},
      {"plus",          3, 0, ri_plus},
      {"times",         3, 0, ri_times},
      {"lpsolve",       5, 0, ri_lpsolve},
      {"lpsolve_conf",  2, 0, ri_lpsolve_conf},
      {0,0,0,0}
    };

#ifdef LINUX_IEEE
    fp_except m = fpsetmask(~FP_X_INV);
    sigfpe(FPE_FLTINV, exception_handler);
#endif

    static RIDefinition ri_def;
    ri_definition = &ri_def;

    atom_row        = OZ_atom("row");
    atom_opt        = OZ_atom("opt");
    atom_type       = OZ_atom("type");
    atom_rhs        = OZ_atom("rhs");
    atom_min        = OZ_atom("min");
    atom_max        = OZ_atom("max");
    atom_optimal    = OZ_atom("optimal");
    atom_infeasible = OZ_atom("infeasible");
    atom_unbounded  = OZ_atom("unbound");
    atom_failure    = OZ_atom("failure");
    atom_le         = OZ_atom("=<");
    atom_ge         = OZ_atom(">=");
    atom_eq         = OZ_atom("==");
    atom_oops       = OZ_atom("oops");

    atom_verbose       = OZ_atom("verbose");
    atom_quiet         = OZ_atom("quiet");
    atom_put           = OZ_atom("put");
    atom_get           = OZ_atom("get");
    atom_solver        = OZ_atom("solver");
    atom_mode          = OZ_atom("mode");
    atom_avail         = OZ_atom("avail");
    atom_config        = OZ_atom("config");
    atom_lpsolve       = OZ_atom("lpsolve");
    atom_cplex_primopt = OZ_atom("cplex_primopt");
    atom_cplex_dualopt = OZ_atom("cplex_dualopt");

    RILessEq::profile       = "ri_lessEq";
    RIGreater::profile      = "ri_greater";
    RIPlus::profile         = "ri_plus";
    RITimes::profile        = "ri_times";
    RIIntBounds::profile    = "ri_intBounds";
    RIIntBoundsSPP::profile = "ri_intBoundsSPP";

    RILPSolve::solver = RILPSolve::lpsolve;
    RILPSolve::mode   = RILPSolve::quiet;

#ifndef ALLWAYS_CLOSE_CPLEX
#ifdef CPLEX
    int status;
    CPLEX_env = CPXopenCPLEXdevelop(&status);

    if (CPLEX_env == NULL) {
      char  errmsg[1024];
      fprintf (stderr, "Could not open CPLEX environment.\n");
      CPXgeterrorstring (CPLEX_env, status, errmsg);
      fprintf (stderr, "%s", errmsg);
    }
#endif
#endif

    RIDefinition::_kind = OZ_getUniqueId();

    printf("\tModule `real interval constraints' from %s (%s) loaded.\n",
           __DATE__, __TIME__);

    return i_table;
  }
} /* extern "C" */

// End of File
//-----------------------------------------------------------------------------
