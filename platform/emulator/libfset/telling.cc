#include "telling.hh"

OZ_C_proc_begin(fsp_tellIsIn, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarBounds);
  OZ_EXPECT(pe, 1, expectInt);
  
  return pe.spawn(new TellIsInPropagator(OZ_args[0],
					 OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun TellIsInPropagator::spawner = fsp_tellIsIn;

OZ_C_proc_begin(fsp_tellIsNotIn, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarBounds);
  OZ_EXPECT(pe, 1, expectInt);
  
  return pe.spawn(new TellIsNotInPropagator(OZ_args[0],
					    OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun TellIsNotInPropagator::spawner = fsp_tellIsNotIn;

OZ_C_proc_begin(fsp_card, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  
  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.spawn(new FSetCardPropagator(OZ_args[0],
					 OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun FSetCardPropagator::spawner = fsp_card;

//*****************************************************************************

OZ_Return TellIsInPropagator::run(void)
{
  OZ_DEBUGPRINT("in: " << *this);
  
  OZ_FSetVar v(_v);
  
  FailOnInvalid(*v += _i);

  OZ_DEBUGPRINT("out: "<< *this);
  v.leave(); 
  return ENTAILED;

failure:
  OZ_DEBUGPRINT("fail: "<< *this);
  v.fail();
  return FAILED;
}

OZ_Return TellIsNotInPropagator::run(void)
{
  OZ_DEBUGPRINT("in: " << *this);
  
  OZ_FSetVar v(_v);
  
  FailOnInvalid(*v -= _i);

  OZ_DEBUGPRINT("out: "<< *this);
  v.leave();
  return ENTAILED;

failure:
  OZ_DEBUGPRINT("fail: "<< *this);
  v.fail();
  return FAILED;
}

OZ_Return FSetCardPropagator::run(void)
{
  OZ_DEBUGPRINT("in: " << *this);
  
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  
  FailOnEmpty(*d >= s->getCardMin());
  FailOnEmpty(*d <= s->getCardMax());

  FailOnInvalid(s->putCard(d->getMinElem(), d->getMaxElem()));
  
  if (*d == fd_singl) {
    OZ_DEBUGPRINT("entailed: "<< *this);
    return P.vanish();
  }
    
  OZ_DEBUGPRINT("out: "<< *this);
  
  return P.leave();

failure:
  OZ_DEBUGPRINT("fail: "<< *this);
  return P.fail();
}
