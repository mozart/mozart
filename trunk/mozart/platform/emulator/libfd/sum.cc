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

#include <string.h>

#include "sum.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include "arith.hh"

#define TMUELLER
#define NEW
//-----------------------------------------------------------------------------

OZ_BI_define(fdp_sum, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD","OZ_EM_LIT","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 1, expectLiteral);

  sum_ops op = getSumOps(OZ_in(1));

  if (op == sum_ops_neq) {
    OZ_EXPECT(pe, 0, expectVectorIntVarSingl);
    OZ_EXPECT(pe, 2, expectIntVarSingl);
    
    return pe.impose(new SumNeqPropagator(OZ_in(0), OZ_in(2)));
  } else {
    OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 2, expectIntVarMinMax);

    switch (op) {
    case sum_ops_eq:
      return pe.impose(new SumEqPropagator(OZ_in(0), OZ_in(2)));
    case sum_ops_leq:
      return pe.impose(new SumLeqPropagator(OZ_in(0), OZ_in(2)));
    case sum_ops_lt:
      return pe.impose(new SumLtPropagator(OZ_in(0), OZ_in(2)));
    case sum_ops_geq:
      return pe.impose(new SumGeqPropagator(OZ_in(0), OZ_in(2)));
    case sum_ops_gt:
      return pe.impose(new SumGtPropagator(OZ_in(0), OZ_in(2)));
    default: ;
    } 
  }

  ERROR_UNEXPECTED_OPERATOR(1);
}
OZ_BI_end

//-----------------------------------------------------------------------------

#include "sum_filter.hh"

OZ_BI_define(fdp_sumC, 4, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT","OZ_EM_VECT OZ_EM_FD","OZ_EM_LIT
		   ","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 2, expectLiteral);

  sum_ops op = getSumOps(OZ_in(2));


#ifdef NEW
  if (op == sum_ops_leq) {
    OZ_Return r; 
    return make_lessEqOffsetN(r, pe, OZ_in(0), OZ_in(1), OZ_in(3));
  }
#endif

  OZ_EXPECT(pe, 0, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  if (op == sum_ops_neq) {
    OZ_EXPECT(pe, 1, expectVectorIntVarSingl);
    OZ_EXPECT(pe, 3, expectIntVarSingl);

    return pe.impose(new SumCNeqPropagator(OZ_in(0), OZ_in(1), OZ_in(3)));
  } else {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax);

    switch (op) {
    case sum_ops_eq:
      return pe.impose(new SumCEqPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    case sum_ops_leq:
      return pe.impose(new SumCLeqPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    case sum_ops_lt:
      return pe.impose(new SumCLtPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    case sum_ops_geq:
      return pe.impose(new SumCGeqPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    case sum_ops_gt:
      return pe.impose(new SumCGtPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    default: ;
    } 
  }

  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_sumCN, 4, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT","OZ_EM_VECT OZ_EM_VECT OZ_EM_FD
		   ","OZ_EM_LIT","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 2, expectLiteral);
  OZ_EXPECT(pe, 0, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  sum_ops op = getSumOps(OZ_in(2));

  if (op == sum_ops_neq) {
    OZ_EXPECT(pe, 1, expectVectorLinearVectorIntVarSingl);
    OZ_EXPECT(pe, 3, expectIntVarSingl);

    return pe.impose(new SumCNNeqPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
  } else {
    OZ_EXPECT(pe, 1, expectVectorVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax);

    switch (op) {
    case sum_ops_eq:
      return pe.impose(new SumCN_EqPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    case sum_ops_leq:
      return pe.impose(new SumCNLeqPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    case sum_ops_lt:
      return pe.impose(new SumCNLtPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    case sum_ops_geq:
      return pe.impose(new SumCNGeqPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    case sum_ops_gt:
      return pe.impose(new SumCNGtPropagator(OZ_in(0),OZ_in(1),OZ_in(3)));
    default: ;
    } 
  }
  
  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_BI_end

//-----------------------------------------------------------------------------

OZ_Return LinNotEqPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  int &c = reg_c, &sz = reg_sz, * a = reg_a;

  simplify_on_equality();

  if (sz == 0) return (c == 0) ? FAILED : PROCEED;

  // if possible reduce to ternary propagator 
  if (sz == 2 && mayBeEqualVars()) {
    if ((a[0] == 1) && (a[1] == -1)) {
      OZ_DEBUGPRINTTHIS("out (replace 1)");
      return replaceBy(new NotEqOffPropagator(reg_x[0], reg_x[1], -c));
    } else if ((a[0] == -1) && (a[1] == 1)) {
      OZ_DEBUGPRINTTHIS("out (replace 2)");
      return replaceBy(new NotEqOffPropagator(reg_x[1], reg_x[0], -c));
    }
  }

  DECL_DYN_ARRAY(OZ_FDIntVar, x, sz);

  PropagatorController_VV P(sz, x);
  int  num_of_singl = 0, last_nonsingl = 0;

  int i;
  for (i = sz; i--; ) {
    x[i].read(reg_x[i]);
    if (*x[i] == fd_singl) 
      num_of_singl += 1;
    else
      last_nonsingl = i;
  }

  if (sz == (num_of_singl + 1)) {
    NUMBERCAST sum = c;
    for (i = sz; i--; )
      if (last_nonsingl != i)
	sum += NUMBERCAST(a[i]) * x[i]->getSingleElem();

    if ((int(sum) % a[last_nonsingl]) != 0) {
      OZ_DEBUGPRINTTHIS("out (1) ");
      return P.vanish();
    } else 
      sum /= -NUMBERCAST(a[last_nonsingl]);

    *x[last_nonsingl] -= int(sum);    
    
    OZ_DEBUGPRINTTHIS("out (2) ");
    return P.vanish();
  }

  if (sz == num_of_singl) {
    NUMBERCAST ent = c;
    
    for (i = sz; i--; ) {
      ent += NUMBERCAST(a[i]) * x[i]->getSingleElem();
    }

    if (ent != 0) {
      OZ_DEBUGPRINTTHIS("out (3) ");
      return P.vanish();
    }
    OZ_DEBUGPRINTTHIS("out (fail) ");
    return P.fail();
  }
  
  OZ_DEBUGPRINTTHIS("out (4) ");
  return P.leave();
}

//-----------------------------------------------------------------------------
#ifdef TMUELLER
#ifdef NEW
#include "sum_filter.hh"

OZ_Return LinLessEqPropagator::propagate(void)
{
  //
  DECL_DYN_ARRAY(OZ_FDIntVar, xs, reg_sz);
  PropagatorController_VV P(reg_sz, xs);
  for (int i = reg_sz; i--; )
    xs[i].read(reg_x[i]);
  OZ_FDIntVarVector x(reg_sz, xs, &reg_x);
  OZ_Filter<OZ_Propagator> s(this, &P);
  OZ_Return r = filter_lessEqOffsetN(s, reg_a, x, reg_c)();
  return r;
}

#else
OZ_Return LinLessEqPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");
  //
  int &c = reg_c, &sz = reg_sz, * a = reg_a;
  //
  simplify_on_equality();
  //
  if (sz == 0) {
    return (c <= 0) ? PROCEED: FAILED;
  }
  // if possible reduce to ternary propagator 
  if (sz == 2) {
    if ((a[0] == 1) && (a[1] == -1)) {
      return replaceBy(new LessEqOffPropagator(reg_x[0], reg_x[1], -c));
    } else if ((a[0] == -1) && (a[1] == 1)) {
      return replaceBy(new LessEqOffPropagator(reg_x[1], reg_x[0], -c));
    }
  }
  //
  DECL_DYN_ARRAY(OZ_FDIntVar, x, sz);
  PropagatorController_VV P(sz, x);
  int i;
  //
  for (i = sz; i--; )
    x[i].read(reg_x[i]);
  //
  DECL_DYN_ARRAY(double, v, sz);
  DECL_DYN_ARRAY(double, w, sz);
  v[0] = 0;
  OZ_DEBUGCODE(printf("v[%d]=%f\n", 0, v[0]);)
  for (i = 1; i < sz; i += 1) {
    v[i] = v[i-1] + a[i-1] * (a[i-1] > 0
			      ? x[i-1]->getMinElem()
			      : x[i-1]->getMaxElem());
    OZ_DEBUGCODE(printf("v[%d]=%f\n", i, v[i]);)
  }
  //
  w[sz-1] = 0;
  OZ_DEBUGCODE(printf("w[%d]=%f\n", sz-1, w[sz-1]);)
  for (i = sz; i > 1; i -= 1) {
    w[i-2] = w[i-1] + a[i-1]* (a[i-1] > 0
			       ? x[i-1]->getMinElem()
			       : x[i-1]->getMaxElem());
    OZ_DEBUGCODE(printf("w[%d]=%f\n", i-2, w[i-2]);)
  }
  //
  OZ_DEBUGCODE(
		DECL_DYN_ARRAY(double, tx, sz);
		for (i = sz; i--; ) tx[i] = calc_tx_lin(i, sz, a, x, c);
		);
  double sum = c;
  //
  for (i = 0; i < sz; i += 1) {
    if (a[i] > 0) {
      int ub = doubleToInt(floor(double(v[i]+w[i]+c)/-a[i]));
      OZ_DEBUGCODE(
		   if (ub != doubleToInt(floor(tx[i]))) {
		     printf("--> ub %d != %d\n", ub, doubleToInt(floor(tx[i])));
		     printf("error %s\n", this->toString());
		   }
		   printf("ub=%d\n", ub);
		   );
      FailOnEmpty(*x[i] <= ub);
      sum += a[i] * x[i]->getMaxElem();
    } else {
      int lb = doubleToInt(ceil(double(v[i]+w[i]+c)/-a[i]));
      OZ_DEBUGCODE(
		   if (lb != doubleToInt(ceil(tx[i]))) {
		     printf("--> lb %d != %d\n", lb, doubleToInt(ceil(tx[i])));
		     printf("error %s\n", this->toString());
		   }
		   printf("lb=%d\n", lb);
		   );
      FailOnEmpty(*x[i] >= lb);
      sum += a[i] * x[i]->getMinElem();
    }
  }
  //
  OZ_DEBUGPRINTTHIS("out ");
  //
  return sum <= 0 ? P.vanish() : P.leave();
  //
failure:
  OZ_DEBUGPRINT(("fail \n"));
  return P.fail();
}
#endif
#else
OZ_Return LinLessEqPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  int &c = reg_c, &sz = reg_sz, * a = reg_a;
  
  simplify_on_equality();
  
  if (sz == 0) return (c <= 0) ? PROCEED: FAILED;
  
  // if possible reduce to ternary propagator 
  if (sz == 2 && mayBeEqualVars()) {
    if ((a[0] == 1) && (a[1] == -1)) {
      return replaceBy(new LessEqOffPropagator(reg_x[0], reg_x[1], -c));
    } else if ((a[0] == -1) && (a[1] == 1)) {
      return replaceBy(new LessEqOffPropagator(reg_x[1], reg_x[0], -c));
    }
  }

  DECL_DYN_ARRAY(OZ_FDIntVar, x, sz);
  PropagatorController_VV P(sz, x);
  int i;
  DECL_DYN_ARRAY(double, tx, sz);
    
  for (i = sz; i--; ) 
    x[i].read(reg_x[i]);

  for (i = sz; i--; )
    tx[i] = calc_tx_lin(i, sz, a, x, c);
  
  NUMBERCAST entailment_test = c;
  
  for (i = sz; i--; ) 
    if (a[i] >= 0) {
      FailOnEmpty(*x[i] <= doubleToInt(floor(tx[i])));
      entailment_test += NUMBERCAST(a[i]) * x[i]->getMaxElem();
    } else {
      FailOnEmpty(*x[i] >= doubleToInt(ceil(tx[i])));
      entailment_test += NUMBERCAST(a[i]) * x[i]->getMinElem();
    }
  
  OZ_DEBUGPRINTTHIS("out ");
  
  return entailment_test <= 0 ? P.vanish() : P.leave();
  
failure:
  OZ_DEBUGPRINT(("fail \n"));

  return P.fail();
}
#endif

//-----------------------------------------------------------------------------

OZ_Return NonLinEqPropagatorP::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");
  
  int &c = reg_c, &sz = reg_sz, * a = reg_a, * smd_sz = reg_smd_sz;

  if (sz == 0 && c != 0) return FAILED;
  
  if (sz == 1 && smd_sz[0] == 2 && OZ_isEqualVars(reg_x[0], reg_x[1])) {
    // x^2 = c
    return replaceBy(new SquarePropagator(reg_x[0], OZ_int(-reg_c)));
  }
  DECL_DYN_ARRAY(OZ_FDIntVar *, xptr, sz);
  FDIntVarArr2 x(sz, xptr, smd_sz);
  FDIntVarArr2 &P = x;
  int i, j, k, l;

  for (i = 0, k = 0; i < sz; i += 1)
    for (j = 0; j < smd_sz[i]; j += 1)
      x[i][j].read(reg_x[k++]);

 loop:
  for (i = sz; i--; ) {
    for (j = smd_sz[i]; j--; ) {
      int txl_i_j = calc_txl_nonlin(i, sz, j, smd_sz, a, x, c);
      
      if (txl_i_j > x[i][j]->getMinElem()) {
	FailOnEmpty(*x[i][j] >= txl_i_j);
	goto loop;
      }
    }
    
    for (j = smd_sz[i]; j--; ) {
      int txu_i_j = calc_txu_nonlin(i, sz, j, smd_sz, a, x, c);	
      if (txu_i_j < x[i][j]->getMaxElem()) {
	FailOnEmpty(*x[i][j] <= txu_i_j);
	goto loop;
      }
    }
  } 
  
  OZ_DEBUGPRINTTHIS("out ");

  {
    DECL_DYN_ARRAY(int, single_var, sz);
    OZ_Boolean is_lin;
    OZ_Return r = P.leave(single_var, is_lin);
    return is_lin ? replaceBy(new LinEqPropagator(sz, smd_sz, single_var,
						  a, reg_x, c)) : r;
  }
failure:

  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return NonLinLessEqPropagatorP::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &c = reg_c, &sz = reg_sz, * a = reg_a, * smd_sz = reg_smd_sz;

  if (sz == 0) return (c <= 0) ? PROCEED : FAILED;
  
  DECL_DYN_ARRAY(OZ_FDIntVar *, xptr, sz);
  FDIntVarArr2 x(sz, xptr, smd_sz);
  FDIntVarArr2 &P = x;
  int i, j, k, l;

  for (i = 0, k = 0; i < sz; i += 1) 
    for (j = 0; j < smd_sz[i]; j += 1) 
      x[i][j].read(reg_x[k++]);

 loop:
  NUMBERCAST sum = c;
  
  for (i = sz; i--; ) {
    NUMBERCAST prod = 1;

    if (a[i] >= 0) {
      for (j = smd_sz[i]; j--; ) {
	int tx_floor = doubleToInt(floor(calc_tx_nonlin(i, sz, j, smd_sz, a, x, c)));
	if (x[i][j]->getMaxElem() > tx_floor) {
	  FailOnEmpty(*x[i][j] <= tx_floor);
	  goto loop;
	}
	prod *= x[i][j]->getMaxElem();
      }
    } else {
      for (j = smd_sz[i]; j--; ) {
	int tx_ceil = doubleToInt(ceil(calc_tx_nonlin(i, sz, j, smd_sz, a, x, c)));
	if (tx_ceil > x[i][j]->getMinElem()) {
	  FailOnEmpty(*x[i][j] >= tx_ceil);
	  goto loop;
	}
	prod *= x[i][j]->getMinElem();
      }
    }
    sum += a[i] * prod;
  }
  
  OZ_DEBUGPRINTTHIS("out: ");

  if (sum <= 0) {
    return P.vanish();
  } else {
    DECL_DYN_ARRAY(int, single_var, sz);
    OZ_Boolean is_lin;
    OZ_Return r = P.leave(single_var, is_lin); 
    return is_lin ? replaceBy(new LinLessEqPropagator(sz, smd_sz, single_var,
						      a, reg_x, c)) : r;
  }

failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------
// static members

OZ_PropagatorProfile LinEqPropagator::profile;
OZ_PropagatorProfile LinNotEqPropagator::profile;
OZ_PropagatorProfile LinLessEqPropagator::profile;
OZ_PropagatorProfile NonLinEqPropagatorP::profile;
OZ_PropagatorProfile NonLinLessEqPropagatorP::profile;
