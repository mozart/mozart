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
      {0,0,0,0}
    };

#ifdef LINUX_IEEE
    fp_except m = fpsetmask(~FP_X_INV);
    sigfpe(FPE_FLTINV, exception_handler);
#endif

    static RIDefinition ri_def;
    ri_definition = &ri_def;

    RILessEq::profile       = "ri_lessEq";
    RIGreater::profile      = "ri_greater";
    RIPlus::profile         = "ri_plus";
    RITimes::profile        = "ri_times";
    RIIntBounds::profile    = "ri_intBounds";
    RIIntBoundsSPP::profile = "ri_intBoundsSPP";

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

    return i_table;
  }
} /* extern "C" */

// End of File
//-----------------------------------------------------------------------------
