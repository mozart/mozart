/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "card.hh"
#include "sum.hh"
#include "auxcomp.hh"

#define Reify(C, V) if (C) {FailOnEmpty(*b &= V); return P.vanish();}
#define ReifyOnFailure(X, V) Reify((X) == 0, V)


//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_sumR, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_LIT "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectLiteral);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectIntVarMinMax);

  char * op = OZ_atomToC(OZ_args[1]);
  if (!strcmp(SUM_OP_EQ, op)) {
    return pe.impose(new SumREqPropagator(OZ_args[0], OZ_args[2], OZ_args[3]));
  } else if (!strcmp(SUM_OP_NEQ, op)) {
    return pe.impose(new SumRNeqPropagator(OZ_args[0], OZ_args[2], OZ_args[3]));
  } else if (!strcmp(SUM_OP_LEQ, op)) {
    return pe.impose(new SumRLeqPropagator(OZ_args[0], OZ_args[2], OZ_args[3]));
  } else if (!strcmp(SUM_OP_LT, op)) {
    return pe.impose(new SumRLtPropagator(OZ_args[0], OZ_args[2], OZ_args[3]));
  } else if (!strcmp(SUM_OP_GEQ, op)) {
    return pe.impose(new SumRGeqPropagator(OZ_args[0], OZ_args[2], OZ_args[3]));
  } else if (!strcmp(SUM_OP_GT, op)) {
    return pe.impose(new SumRGtPropagator(OZ_args[0], OZ_args[2], OZ_args[3]));
  } 
  
  ERROR_UNEXPECTED_OPERATOR(1);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_sumCR, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_LIT "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorInt);
  OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 2, expectLiteral);
  OZ_EXPECT(pe, 3, expectIntVarMinMax);
  OZ_EXPECT(pe, 4, expectIntVarMinMax);

  char * op = OZ_atomToC(OZ_args[2]);
  if (!strcmp(SUM_OP_EQ, op)) {
    return pe.impose(new SumCREqPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_NEQ, op)) {
    return pe.impose(new SumCRNeqPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_LEQ, op)) {
    return pe.impose(new SumCRLeqPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_LT, op)) {
    return pe.impose(new SumCRLtPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_GEQ, op)) {
    return pe.impose(new SumCRGeqPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_GT, op)) {
    return pe.impose(new SumCRGtPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } 
  
  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_sumCNR, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT "," OZ_EM_VECT OZ_EM_VECT OZ_EM_FD "," OZ_EM_LIT "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorInt);
  OZ_EXPECT(pe, 1, expectVectorVectorIntVarMinMax);
  OZ_EXPECT(pe, 2, expectLiteral);
  OZ_EXPECT(pe, 3, expectIntVarMinMax);
  OZ_EXPECT(pe, 4, expectIntVarMinMax);

  char * op = OZ_atomToC(OZ_args[2]);
  if (!strcmp(SUM_OP_EQ, op)) {
    return pe.impose(new SumCNREqPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_NEQ, op)) {
    return pe.impose(new SumCNRNeqPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_LEQ, op)) {
    return pe.impose(new SumCNRLeqPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_LT, op)) {
    return pe.impose(new SumCNRLtPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_GEQ, op)) {
    return pe.impose(new SumCNRGeqPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } else if (!strcmp(SUM_OP_GT, op)) {
    return pe.impose(new SumCNRGtPropagator(OZ_args[0], OZ_args[1], OZ_args[3], OZ_args[4]));
  } 
  
  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_C_proc_end

//=============================================================================

OZ_Return LinEqBPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  int &c = reg_c, &sz = reg_sz, * a = reg_a;

  simplify_on_equality();

  DECL_DYN_ARRAY(OZ_FDIntVar, x, sz);
  OZ_FDIntVar b(reg_b);
  PropagatorController_VV_V P(sz, x, b);
  DECL_DYN_ARRAY(OZ_Boolean, flag_txl, sz);
  DECL_DYN_ARRAY(OZ_Boolean, flag_txu, sz); 
  int dom_size = 0, i;

  for (i = sz; i--; ) {
    x[i].readEncap(reg_x[i]);
    dom_size += x[i]->getSize();
    flag_txl[i] = flag_txu[i] = OZ_TRUE;
  }

  Reify(sz == 0 && c != 0, 0);
  Reify(sz == 0 && c == 0, 1);

 loop:
  for (i = sz; i--; ) {
    if (flag_txl[i]) {
      int txl_i = calc_txl_lin(i, sz, a, x, c);
      
      if (txl_i > x[i]->getMinElem()) {
	ReifyOnFailure(*x[i] >= txl_i, 0);
	for (int j = sz; j--; ) {
	  flag_txl[j] |= is_recalc_txl_lower(j, i, a);
	  flag_txu[j] |= is_recalc_txu_lower(j, i, a);
	}
	flag_txl[i] = OZ_FALSE;
	goto loop;
      }
      flag_txl[i] = OZ_FALSE;
    }
    if (flag_txu[i]) {
      int txu_i = calc_txu_lin(i, sz, a, x, c);
      
      if (txu_i < x[i]->getMaxElem()) {
	ReifyOnFailure(*x[i] <= txu_i, 0);
	for (int j = sz; j--; ) {
	  flag_txl[j] |= is_recalc_txl_upper(j, i, a);
	  flag_txu[j] |= is_recalc_txu_upper(j, i, a);
	}
	flag_txu[i] = OZ_FALSE;
	goto loop;
      }
      flag_txu[i] = OZ_FALSE;
    }
  } 

  // if every variable was already a singleton and no failure occured,
  // the reified equation is entailed.
  if (dom_size == sz) {
    FailOnEmpty(*b &= 1);
    return P.vanish();
  }
    
  if (*b == fd_singl) {
    OZ_DEBUGPRINT(("imposing (not) eq %d",b->getSingleElem()));
    P.vanish();
    OZ_Propagator *prop; 
    if (b->getSingleElem() == 1)
      prop = new LinEqPropagator(*this);
    else
      prop = new LinNotEqPropagator(*this);
    return replaceBy(prop);
  }

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return LinLessEqBPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  int &c = reg_c, &sz = reg_sz, * a = reg_a;

  simplify_on_equality();

  DECL_DYN_ARRAY(OZ_FDIntVar, x, sz);
  OZ_FDIntVar b(reg_b);
  PropagatorController_VV_V P(sz, x, b);
  DECL_DYN_ARRAY(OZ_Boolean, flag_txl, sz);
  DECL_DYN_ARRAY(OZ_Boolean, flag_txu, sz);
  int i;

  for (i = sz; i--; ) {
    x[i].readEncap(reg_x[i]);
    flag_txl[i] = flag_txu[i] = OZ_TRUE;
  }

  Reify(sz == 0, (c > 0) ? 0 : 1);
  
  Reify(check_calc_txl(sz, a, x, c) > 0, 0); 
  
  Reify(check_calc_txu(sz, a, x, c) <= 0, 1);
  
  if (*b == fd_singl) {
    OZ_DEBUGPRINT(("imposing lesseq %d", b->getSingleElem()));
    P.vanish();
    if (b->getSingleElem() == 1) {
      return replaceBy(new LinLessEqPropagator(*this));
    } else {
      // invert coefficients!
      for (i = sz; i--; )
	a[i] = - a[i];
      reg_c = 1 - reg_c;
      return replaceBy(new LinLessEqPropagator(*this));
    }
  }

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------

#define FailOnEmptySpeculativelyNeg(X)                                        \
if ((X) == 0) {FailOnEmpty(x[ts] &= 1); return x.entailment();}

OZ_Return LinNotEqBPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");
  int &c = reg_c, &sz = reg_sz, * a = reg_a;

  simplify_on_equality();

  DECL_DYN_ARRAY(OZ_FDIntVar, x, sz);
  OZ_FDIntVar b(reg_b);
  PropagatorController_VV_V P(sz, x, b);
  DECL_DYN_ARRAY(OZ_Boolean, flag_txl, sz);
  DECL_DYN_ARRAY(OZ_Boolean, flag_txu, sz);
  int dom_size = 0, i;

  for (i = sz; i--; ) {
    x[i].readEncap(reg_x[i]);
    dom_size += x[i]->getSize();
    flag_txl[i] = flag_txu[i] = OZ_TRUE;
  }

  Reify((sz == 0 && c != 0), 1);
  
  Reify((sz == 0 && c == 0), 0);

 loop:
  for (i = sz; i--; ) {
    if (flag_txl[i]) {
      int txl_i = calc_txl_lin(i, sz, a, x, c);
      
      if (txl_i > x[i]->getMinElem()) {
	ReifyOnFailure(*x[i] >= txl_i, 1);
	for (int j = sz; j--; ) {
	  flag_txl[j] |= is_recalc_txl_lower(j, i, a);
	  flag_txu[j] |= is_recalc_txu_lower(j, i, a);
	}
	flag_txl[i] = OZ_FALSE;
	goto loop;
      }
      flag_txl[i] = OZ_FALSE;
    }
    if (flag_txu[i]) {
      int txu_i = calc_txu_lin(i, sz, a, x, c);
      
      if (txu_i < x[i]->getMaxElem()) {
	ReifyOnFailure(*x[i] <= txu_i, 1);
	for (int j = sz; j--; ) {
	  flag_txl[j] |= is_recalc_txl_upper(j, i, a);
	  flag_txu[j] |= is_recalc_txu_upper(j, i, a);
	}
	flag_txu[i] = OZ_FALSE;
	goto loop;
      }
      flag_txu[i] = OZ_FALSE;
    }
  } 

  // if every variable was already a singleton and no failure occured,
  // the reified equation is disentailed.
  if (dom_size == sz) {
    FailOnEmpty(*b &= 0);
    return P.vanish();
  }
    
  if (*b == fd_singl) {
    OZ_DEBUGPRINT(("imposing (not) eq %d", b->getSingleElem()));
    P.vanish();
    OZ_Propagator *prop; 
    if (b->getSingleElem() == 0)
      prop = new LinEqPropagator(*this);
    else
      prop = new LinNotEqPropagator(*this);
    return replaceBy(prop);
  }

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_intR, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FDDESCR "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectDomDescr);
  OZ_EXPECT(pe, 2, expectIntVarAny);

  return pe.impose(new InBPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return InBPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_FiniteDomain &domain = reg_domain;
  OZ_FDIntVar v(reg_v), b(reg_b);
  PropagatorController_V_V P(v, b);

  if (*b == fd_singl) {
    FailOnEmpty(*v &= b->getSingleElem() ? domain : ~domain);
    OZ_DEBUGPRINTTHIS("vanish out ");
    return P.vanish();
  }

  if ((*v & domain) == fd_empty) {
    FailOnEmpty(*b &= 0);
    OZ_DEBUGPRINTTHIS("vanish out ");
    return P.vanish();
  }

  if ((*v & domain).getSize() == v->getSize()) {
    FailOnEmpty(*b &= 1);
    OZ_DEBUGPRINTTHIS("vanish out ");
    return P.vanish();
  } 
  
  OZ_DEBUGPRINTTHIS("out ");
  return P.leave();
  
failure:
  OZ_DEBUGPRINTTHIS("failure out ");

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_card, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectIntVarMinMax);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectIntVarMinMax);

  return pe.impose(new CardBPropagator(OZ_args[0], OZ_args[1], OZ_args[2], OZ_args[3]));
}
OZ_C_proc_end

OZ_Return CardBPropagator::propagate(void)
{
  int &v_sz = reg_v_sz;
  
  if (v_sz == 0) return FAILED;
  
  DECL_DYN_ARRAY(OZ_FDIntVar, v, v_sz);
  OZ_FDIntVar low(reg_low), up(reg_up), b(reg_b);
  PropagatorController_VV_V_V_V P(v_sz, v, low, up, b);
  int i, ones, zeroes, possibles;
  
  for (i = v_sz; i--; )
    v[i].read(reg_v[i]);
  
  int lowl = low->getMinElem(), lowu = low->getMaxElem();
  int upl = up->getMinElem(), upu = up->getMaxElem();
  
  if (*b == fd_singl) {
    if (b->getSingleElem() == 1) {
      FailOnEmpty(*low <= upu);
      FailOnEmpty(*up >= lowl);
      lowu = low->getMaxElem();
      upl = up->getMinElem();
    }
  } else if (lowl > upu) {
    FailOnEmpty(*b &= 0);
    return P.vanish();
  }
  
  ones = 0;
  zeroes = 0;
  
  for (i = v_sz; i--; ) 
    if (*v[i] == fd_singl)
      if (v[i]->getSingleElem() == 0)
	zeroes++;
      else
	ones++;

  possibles = v_sz - zeroes;
  
  if ((ones > upu) || (possibles < lowl)) {
    FailOnEmpty(*b &= 0);
    return P.vanish();
  }

  if ((ones >= lowu) && (possibles <= upl)) {
    FailOnEmpty(*b &= 1);
    return P.vanish();
  }

  if (*b == fd_singl) {
    if (b->getSingleElem() == 1) {
      if ((ones == upu) && (possibles - ones > 0)) {
	// impose negatively
	for (i = v_sz; i--; ) 
	  if (*v[i] != fd_singl) 
	    FailOnEmpty(*v[i] &= 0);
	return P.vanish();
      }
      
      if ((possibles == lowl) && (possibles - ones > 0)) {
	// impose positively
	for (i = v_sz; i--; ) 
	  if (*v[i] != fd_singl)
	    FailOnEmpty(*v[i] &= 1);
	return P.vanish();
      }
      FailOnEmpty(*low <= possibles);
      FailOnEmpty(*up >= ones);
    } else if (b->getSingleElem() == 0) {
      if ((ones == lowl - 1) && (possibles <= upl) && (possibles - ones > 0)) {
	// impose negatively
	for (i = v_sz; i--; ) 
	  if (*v[i] != fd_singl) 
	    FailOnEmpty(*v[i] &= 0);
	return P.vanish();
      }
      
      if ((possibles == upl + 1) && (ones >= lowu) && (possibles - ones > 0)) {
	// impose positively
	for (i = v_sz; i--; ) 
	  if (*v[i] != fd_singl)
	    FailOnEmpty(*v[i] &= 1);
	return P.vanish();
      }
      
      if (lowu <= ones) FailOnEmpty(*up <= possibles - 1);
      if (upl >= possibles) FailOnEmpty(*low >= ones + 1);
    }
  }
  
  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// static member

OZ_CFunHeader LinEqBPropagator::spawner = fdp_sumCR;
OZ_CFunHeader LinNotEqBPropagator::spawner = fdp_sumCR;
OZ_CFunHeader LinLessEqBPropagator::spawner = fdp_sumCR;
OZ_CFunHeader InBPropagator::spawner = fdp_intR;
OZ_CFunHeader CardBPropagator::spawner = fdp_card;
