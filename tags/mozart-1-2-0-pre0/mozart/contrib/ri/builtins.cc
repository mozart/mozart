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

//-----------------------------------------------------------------------------

OZ_BI_define(ri_newVar, 3, 0)
{
  OZ_declareFloat(0, l);
  OZ_declareFloat(1, u);

  if (l > u)
    return OZ_FAILED;

  RI ri(l, u);

  return OZ_mkCtVar(OZ_in(2), &ri, ri_definition);
}
OZ_BI_end

OZ_BI_define(ri_declVar, 1, 0)
{
  RI ri(RI_FLOAT_MIN, RI_FLOAT_MAX);

  return OZ_mkCtVar(OZ_in(0), &ri, ri_definition);
}
OZ_BI_end

OZ_BI_define(ri_setPrecision, 1, 0)
{
  OZ_declareFloat(0, p);

  ri_precision = p;

  return PROCEED;
}
OZ_BI_end

OZ_BI_define(ri_getLowerBound, 2, 0)
{
  OZ_EXPECTED_TYPE(EM_RI ", FLOAT" );

  RIExpect pe;
  OZ_expect_t r = pe.expectRIVarMinMax(OZ_in(0));
  if (pe.isFailing(r)) {
    return OZ_typeErrorCPI(expectedType, 0, "");
  } else if (pe.isSuspending(r)) {
    return pe.suspend();
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
    return pe.suspend();
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
    return pe.suspend();
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
