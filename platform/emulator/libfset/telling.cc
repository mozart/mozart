/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "telling.hh"

OZ_C_proc_begin(fsp_include, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FSET);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectFSetVarBounds);

  return pe.impose(new IncludePropagator(OZ_args[1],
                                         OZ_args[0]));
}
OZ_C_proc_end

OZ_CFun IncludePropagator::header = fsp_include;

OZ_C_proc_begin(fsp_exclude, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FSET );

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectFSetVarBounds);

  return pe.impose(new ExcludePropagator(OZ_args[1],
                                         OZ_args[0]));
}
OZ_C_proc_end

OZ_CFun ExcludePropagator::header = fsp_exclude;

OZ_C_proc_begin(fsp_card, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new FSetCardPropagator(OZ_args[0],
                                          OZ_args[1]));
}
OZ_C_proc_end

OZ_CFun FSetCardPropagator::header = fsp_card;

//*****************************************************************************

OZ_Return IncludePropagator::propagate(void)
{
  OZ_DEBUGPRINT("in: " << *this);

  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

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

  OZ_DEBUGPRINT("out: "<< *this);

  return P.leave1();

failure:
  OZ_DEBUGPRINT("fail: "<< *this);
  return P.fail();
}

OZ_Return ExcludePropagator::propagate(void)
{
  OZ_DEBUGPRINT("in: " << *this);

  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  if (*d == fd_singl) {
    FailOnInvalid(*s -= d->getSingleElem());
  } else {

    for (int i = 32 * fset_high; i --; )
      if (s->isIn(i))
        FailOnEmpty(*d -= i);

    if (*d == fd_singl)
      FailOnInvalid(*s -= d->getSingleElem());
  }

  OZ_DEBUGPRINT("out: "<< *this);

  return P.leave1();

failure:
  OZ_DEBUGPRINT("fail: "<< *this);
  return P.fail();
}

OZ_Return FSetCardPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in: " << *this);

  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);


  FailOnEmpty(*d >= s->getCardMin());
  FailOnEmpty(*d <= s->getCardMax());

  FailOnInvalid(s->putCard(d->getMinElem(), d->getMaxElem()));

  if (*d == fd_singl) {
    OZ_DEBUGPRINT("entailed: "<< *d << ' ' << *this);
    return P.vanish();
  }

  OZ_DEBUGPRINT("out: "<< *this);

  return P.leave();

failure:
  OZ_DEBUGPRINT("fail: "<< *this);
  return P.fail();
}
