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

#include "propagators.hh"

//-----------------------------------------------------------------------------

RILPSolve::solvers_e RILPSolve::solver;
RILPSolve::mode_e RILPSolve::mode;

//-----------------------------------------------------------------------------

OZ_BI_define(ri_lpsolve, 5, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT EM_RI "," EM_LP_OBJFN "," 
		   EM_LP_CONSTR "," EM_RI "," OZ_EM_LIT);

  LPExpect pe;

  OZ_EXPECT(pe, 0, expectVectorRIVarMinMax);

  OZ_EXPECT(pe, 1, expectObjFnRecord);

  OZ_EXPECT(pe, 2, expectVectorConstrRecord);

  int dummy;

  OZ_EXPECT_SUSPEND(pe, 3, expectRIVarMinMax, dummy);

  pe.collectVarsOff();

  OZ_EXPECT_SUSPEND(pe, 4, expectLPReturnLiteral, dummy);

  return pe.impose(new RILPSolve(OZ_in(0), OZ_in(1), 
				 OZ_in(2), OZ_in(3), OZ_in(4)));
}
OZ_BI_end

OZ_PropagatorProfile RILPSolve::profile;

#include "lpsolve.cc"

#ifdef INCLUDE_CPLEX

#include "cplex.cc"

#endif
