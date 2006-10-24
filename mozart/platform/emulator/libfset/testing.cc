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

#include "testing.hh"


OZ_BI_define(fsp_isIn, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_INT "," OZ_EM_FSET "," OZ_EM_TNAME);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectInt);
  OZ_EXPECT(pe, 1, expectFSetVarAny);
  
  if (!OZ_isVariable(OZ_in(2)) && 
      !(OZ_isTrue(OZ_in(2)) || OZ_isFalse(OZ_in(2)))) {
    pe.fail();                                             
    return OZ_typeErrorCPI(expectedType, 2, "");           
  }
  
  return pe.impose(new IsInPropagator(OZ_in(1),
				      OZ_in(0),
				      OZ_in(2)));
} 
OZ_BI_end

OZ_Return IsInPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FSetVar v(_v);

  if (v->isIn(_i)) {
    if (OZ_unify(_b, OZ_true()) == OZ_FAILED) // mm_u
      goto failure;
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }
  if (v->isNotIn(_i)) {
    if (OZ_unify(_b, OZ_false())  == OZ_FAILED) // mm_u
      goto failure;
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }
  OZ_DEBUGPRINTTHIS("sleep: ");
  v.leave();
  return SLEEP;

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  v.fail();
  return FAILED;
}

OZ_PropagatorProfile IsInPropagator::profile;

