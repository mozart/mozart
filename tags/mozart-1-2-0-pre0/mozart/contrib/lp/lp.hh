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

#ifndef __LP_HH__
#define __LP_HH__

#include "misc.hh"
#include "ri.hh"

//-----------------------------------------------------------------------------

OZ_BI_proto(ri_lpsolve);
OZ_BI_proto(ri_lpsolve_conf);

//-----------------------------------------------------------------------------
// type-checking parameters

class LPExpect;

typedef OZ_expect_t (LPExpect::*LPPropagatorExpectMeth) (OZ_Term);

class LPExpect : public RIExpect {
public:
  OZ_expect_t expectRIVarMin(OZ_Term t) { 
    return expectGenCtVar(t, ri_definition, 
			  RIWakeUp::wakeupMin()); 
  }
  OZ_expect_t expectRIVarMax(OZ_Term t) { 
    return expectGenCtVar(t, ri_definition, 
			  RIWakeUp::wakeupMax()); 
  }
  OZ_expect_t expectRIVarMinMax(OZ_Term t) { 
    return expectGenCtVar(t, ri_definition, 
			  RIWakeUp::wakeupMinMax()); 
  }
  
  OZ_expect_t expectObjFnRecord(OZ_Term t) {
    static OZ_Term arity[] = {
      {atom_row}, {atom_opt}, {(OZ_Term) 0}
    };
    
    OZ_expect_t r = expectProperRecord(t, arity);
    if (isSuspending(r) || isFailing(r))
      return r;
    
    r = expectVectorRIVarMinMax(OZ_subtree(t, atom_row));
    if (isSuspending(r) || isFailing(r))
      return r;

    r = expectMinMaxLiteral(OZ_subtree(t, atom_opt));
    return r;
  }
  
  OZ_expect_t expectConstrRecord(OZ_Term t) {
    static OZ_Term arity[] = {
      {atom_row}, {atom_type}, {atom_rhs}, {(OZ_Term) 0}
    };
    OZ_expect_t r(1,1);
    
    r = expectProperRecord(t, arity);
    if (isSuspending(r) || isFailing(r))
      return r;
    
    r = expectVectorFloat(OZ_subtree(t, atom_row));   
    if (isSuspending(r) || isFailing(r))
      return r;
    
    r =  expectRhsLiteral(OZ_subtree(t, atom_type));
    if (isSuspending(r) || isFailing(r))
      return r;

    r = expectFloat(OZ_subtree(t, atom_rhs));            
    
    return r;
  }

  OZ_expect_t expectLPReturnLiteral(OZ_Term t) {
    static OZ_Term one_of[] = {
      {atom_optimal}, {atom_infeasible}, {atom_unbounded}, 
      {atom_failure}, {(OZ_Term) 0}
    };
    return expectLiteralOutOf(t, one_of);
  }
  OZ_expect_t expectMinMaxLiteral(OZ_Term t) {
    static OZ_Term one_of[] = {
      {atom_min}, {atom_max}, {(OZ_Term) 0}
    };
    return expectLiteralOutOf(t, one_of);
  }
  OZ_expect_t expectRhsLiteral(OZ_Term t) {
    static OZ_Term one_of[] = {
      {atom_le}, {atom_ge}, {atom_eq}, {(OZ_Term) 0}
    };
    return expectLiteralOutOf(t, one_of);
  }
  OZ_expect_t expectVector(OZ_Term t, LPPropagatorExpectMeth expectf) {
    return OZ_Expect::expectVector(t, (OZ_ExpectMeth) expectf);
  }
  OZ_expect_t expectVectorFloat(OZ_Term t) {
    return expectVector(t, (LPPropagatorExpectMeth) &OZ_Expect::expectFloat);
  }
  OZ_expect_t expectVectorRIVarMinMax(OZ_Term t) {
    return expectVector(t,  &LPExpect::expectRIVarMinMax);
  }
  OZ_expect_t expectVectorConstrRecord(OZ_Term t) {
    return expectVector(t,  &LPExpect::expectConstrRecord);
  }
  OZ_expect_t expectIntVarMinMax(OZ_Term t) {
    return expectIntVar(t, fd_prop_bounds); 
  }
};

//-----------------------------------------------------------------------------
// propagator controller

class PropagatorController_VRI_RI {
protected:
  int _size;
  RIVar * _vv;
  RIVar &_v;
public:
  PropagatorController_VRI_RI(int size, RIVar vv[], RIVar &v) 
    : _size(size), _vv(vv), _v(v) {}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = _v.leave();
    for (int i = _size; i--; vars_left |= _vv[i].leave());
    return vars_left ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    _v.leave();
    for (int i = _size; i--; _vv[i].leave());
    return PROCEED;
  }
  OZ_Return fail(void) {
    _v.fail();
    for (int i = _size; i--; _vv[i].fail());
    return FAILED;
  }
};
#endif /* __LP_HH__ */
