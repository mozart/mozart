/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "reified.hh"

OZ_C_proc_begin(fsp_includeR, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectFSetVarBounds);
  OZ_EXPECT(pe, 2, expectIntVarAny);
  
  return pe.impose(new IncludeRPropagator(OZ_args[1],
					  OZ_args[0],
					  OZ_args[2]));
} 
OZ_C_proc_end

OZ_CFunHeader IncludeRPropagator::header = fsp_includeR;

//*****************************************************************************

OZ_Return IncludeRPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FDIntVar r(_r);

  if (*r == fd_singl) {
    r.leave();
    return replaceBy((r->getSingleElem() == 1)
		     ? (OZ_Propagator*) new IncludePropagator(_s, _d)
		     : (OZ_Propagator*) new ExcludePropagator(_s, _d));
  }

  int r_val = 0;
  OZ_FSetVar s;
  OZ_FDIntVar d;

  s.readEncap(_s);
  d.readEncap(_d);
  
  {
    FailOnEmpty(*d <= (32 * fset_high - 1));
    
    if (*d == fd_singl) {
      FailOnInvalid(*s += d->getSingleElem());
    } else {
      
      for (int i = 32 * fset_high; i --; )
	if (s->isNotIn(i))
	  FailOnEmpty(*d -= i);
      
      if (*d == fd_singl) 
	FailOnInvalid(*s += d->getSingleElem());
    }
    if (!s.isTouched() && !d.isTouched()) {
      r_val = 1;
      goto entailment;
    }
  }
    
  r.leave(); s.leave(); d.leave();
  return SLEEP;

failure:
entailment:

  if (0 == (*r &= r_val)) {
    r.fail(); s.fail(); d.fail();
    return FAILED;
  } 

  r.leave(); s.leave(); d.leave();
  return OZ_ENTAILED;
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_isInR, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_INT "," OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectInt);
  OZ_EXPECT(pe, 1, expectFSetVarAny);
  OZ_EXPECT(pe, 2, expectIntVarAny);
  
  return pe.impose(new IsInRPropagator(OZ_args[1],
				       OZ_args[0],
				       OZ_args[2]));
} 
OZ_C_proc_end

OZ_CFunHeader IsInRPropagator::spawner = fsp_isInR;

OZ_Return IsInRPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FDIntVar b(_b);
  OZ_FSetVar v(_v);

  // TMUELLER if is singleton do the right things  
  if (v->isIn(_i)) {
    FailOnEmpty(*b &= 1);
    b.leave();
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }
  if (v->isNotIn(_i)) {
    FailOnEmpty(*b &= 0);
    b.leave();
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }
  if (*b == fd_singl) {
    if (b->getSingleElem() == 0) {
      FailOnInvalid(*v-= _i);
    } else {
      FailOnInvalid(*v += _i);
    }
    b.leave();
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }

  OZ_DEBUGPRINTTHIS("sleep: ");
  b.leave();
  v.leave();
  return SLEEP;

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  b.fail();
  v.fail();
  return FAILED;
}


//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_bounds, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSETVAL "," OZ_EM_FSET "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;

  int dummy;
  OZ_EXPECT(pe, 0, expectFSetValue);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, dummy);
  OZ_EXPECT(pe, 2, expectInt);
  OZ_EXPECT_SUSPEND(pe, 3, expectIntVarMinMax, dummy);
  OZ_EXPECT_SUSPEND(pe, 4, expectIntVarMinMax, dummy);
  
  return pe.impose(new BoundsPropagator(OZ_args[0],
					OZ_args[1],
					OZ_args[2],
					OZ_args[3],
					OZ_args[4]));
} 
OZ_C_proc_end

OZ_CFunHeader BoundsPropagator::header = fsp_bounds;

OZ_Return BoundsPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d), r(_r);
  PropagatorController_S_D_D P(s, d, r);
  OZ_Return retval = OZ_SLEEP;

  // the following code is executed once ..
  if (_s_ub) {
    OZ_FSetVar s_ub;
    s_ub.ask(_s_ub);
    _s_ub_card = s_ub->getCardMin();

    OZ_DEBUGPRINT(("once %s %d\n", s_ub->toString(), _s_ub_card));
    FailOnInvalid(*s <= *s_ub);
    FailOnEmpty(*d <= _d_ub);
    FailOnEmpty(r->constrainBool());

    _s_ub = 0; // .. because of that 
  }

  if (d->getMinElem() > 0 || 
      s->getGlbSet().getCard() > 0 ||
      *r == 1) {
    OZ_DEBUGPRINT(("a\n"));
    FailOnEmpty(*d &= _d_ub);
    FailOnEmpty(*r &= 1);
    FailOnInvalid(s->putCard(_s_ub_card, _s_ub_card));
    retval = OZ_ENTAILED;
  } else if (d->getMaxElem() < _d_ub || 
	     s->getLubSet().getCard() < _s_ub_card ||
	     *r == 0) {
    OZ_DEBUGPRINT(("b\n"));
    FailOnEmpty(*d &= 0);
    FailOnEmpty(*r &= 0);
    FailOnInvalid(s->putCard(0,0));
    retval = OZ_ENTAILED;
  } 

  P.leave();
  OZ_DEBUGPRINTTHIS("out");
  return OZ_DEBUGRETURNPRINT(retval);

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  P.fail();
  return FAILED;
}

//-----------------------------------------------------------------------------
// eof
