/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include <string.h>

#include "sum.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include "arith.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_sum, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD","OZ_EM_LIT","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 1, expectLiteral);

  char * op = OZ_atomToC(OZ_args[1]);

  if (!strcmp(SUM_OP_NEQ, op)) {
    OZ_EXPECT(pe, 0, expectVectorIntVarSingl);
    OZ_EXPECT(pe, 2, expectIntVarSingl);

    return pe.spawn(new SumNeqPropagator(OZ_args[0], OZ_args[2]));
  } else {
    OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 2, expectIntVarMinMax);

    if (!strcmp(SUM_OP_EQ, op)) {
      return pe.spawn(new SumEqPropagator(OZ_args[0], OZ_args[2]));
    } else if (!strcmp(SUM_OP_LEQ, op)) {
      return pe.spawn(new SumLeqPropagator(OZ_args[0], OZ_args[2]));
    } else if (!strcmp(SUM_OP_LT, op)) {
      return pe.spawn(new SumLtPropagator(OZ_args[0], OZ_args[2]));
    } else if (!strcmp(SUM_OP_GEQ, op)) {
      return pe.spawn(new SumGeqPropagator(OZ_args[0], OZ_args[2]));
    } else if (!strcmp(SUM_OP_GT, op)) {
      return pe.spawn(new SumGtPropagator(OZ_args[0], OZ_args[2]));
    }
  }

  ERROR_UNEXPECTED_OPERATOR(1);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_sumC, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT","OZ_EM_VECT OZ_EM_FD","OZ_EM_LIT
                   ","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 2, expectLiteral);
  OZ_EXPECT(pe, 0, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  char * op = OZ_atomToC(OZ_args[2]);

  if (!strcmp(SUM_OP_NEQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarSingl);
    OZ_EXPECT(pe, 3, expectIntVarSingl);

    return pe.spawn(new SumCNeqPropagator(OZ_args[0], OZ_args[1], OZ_args[3]));
  } else {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax);

    if (!strcmp(SUM_OP_EQ, op)) {
      return pe.spawn(new SumCEqPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_LEQ, op)) {
      return pe.spawn(new SumCLeqPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_LT, op)) {
      return pe.spawn(new SumCLtPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_GEQ, op)) {
      return pe.spawn(new SumCGeqPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_GT, op)) {
      return pe.spawn(new SumCGtPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    }
  }

  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_sumCN, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT","OZ_EM_VECT OZ_EM_VECT OZ_EM_FD
                   ","OZ_EM_LIT","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 2, expectLiteral);
  OZ_EXPECT(pe, 0, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  char * op = OZ_atomToC(OZ_args[2]);

  if (!strcmp(SUM_OP_NEQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorVectorIntVarSingl);
    OZ_EXPECT(pe, 3, expectIntVarSingl);

    return pe.spawn(new SumCNNeqPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
  } else {
    OZ_EXPECT(pe, 1, expectVectorVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax);

    if (!strcmp(SUM_OP_EQ, op)) {
      return pe.spawn(new SumCN_EqPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_NEQ, op)) {
      return pe.spawn(new SumCNNeqPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_LEQ, op)) {
      return pe.spawn(new SumCNLeqPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_LT, op)) {
      return pe.spawn(new SumCNLtPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_GEQ, op)) {
      return pe.spawn(new SumCNGeqPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_GT, op)) {
      return pe.spawn(new SumCNGtPropagator(OZ_args[0],OZ_args[1],OZ_args[3]));
    }
  }

  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_Return LinNotEqPropagator::run(void)
{
  OZ_DEBUGPRINT("in " << *this);

  int &c = reg_c, &sz = reg_sz, * a = reg_a;

  simplify_on_equality();

  if (sz == 0) return (c == 0) ? FAILED : PROCEED;

  // if possible reduce to ternary propagator
  if (sz == 2 && mayBeEqualVars()) {
    if ((a[0] == 1) && (a[1] == -1)) {
      return replaceBy(new NotEqOffPropagator(reg_x[0], reg_x[1], -c));
    } else if ((a[0] == -1) && (a[1] == 1)) {
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

    if ((int(sum) % a[last_nonsingl]) != 0)
      return P.vanish();
    else
      sum /= -NUMBERCAST(a[last_nonsingl]);

    *x[last_nonsingl] -= int(sum);
    return P.vanish();
  }

  if (sz == num_of_singl) {
    NUMBERCAST ent = c;

    for (i = sz; i--; ) {
      ent += NUMBERCAST(a[i]) * x[i]->getSingleElem();
    }

    if (ent != 0) return P.vanish();
    return P.fail();
  }

  return P.leave();
}

//-----------------------------------------------------------------------------

OZ_Return LinLessEqPropagator::run(void)
{
  OZ_DEBUGPRINT("in " << *this);

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

  OZ_DEBUGPRINT("out " << *this);

  return entailment_test <= 0 ? P.vanish() : P.leave();

failure:
  OZ_DEBUGPRINT("fail " << endl);

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return NonLinEqPropagatorP::run(void)
{
  OZ_DEBUGPRINT("in " << *this);

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

  OZ_DEBUGPRINT("out " << *this);

  {
    DECL_DYN_ARRAY(int, single_var, sz);
    OZ_Boolean is_lin;
    OZ_Return r = P.leave(single_var, is_lin);
    return is_lin ? replaceBy(new LinEqPropagator(sz, smd_sz, single_var,
                                                  a, reg_x, c)) : r;
  }
failure:

  OZ_DEBUGPRINT("fail");

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return NonLinLessEqPropagatorP::run(void)
{
  OZ_DEBUGPRINT("in: " << *this);

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

  OZ_DEBUGPRINT("out: " << *this);

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
  OZ_DEBUGPRINT("fail");

  return P.fail();
}

//-----------------------------------------------------------------------------
// static members

OZ_CFun LinEqPropagator::spawner = fdp_sumC;
OZ_CFun LinNotEqPropagator::spawner = fdp_sumC;
OZ_CFun LinLessEqPropagator::spawner = fdp_sumC;
OZ_CFun NonLinEqPropagatorP::spawner = fdp_sumCN;
OZ_CFun NonLinLessEqPropagatorP::spawner = fdp_sumCN;
