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

OZ_CFun IncludeRPropagator::header = fsp_includeR;

//*****************************************************************************

OZ_Return IncludeRPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in: " << *this);

  OZ_FDIntVar r(_r);

  if (*r == fd_singl) {
    r.leave();
    return replaceBy((r->getSingleElem() == 1)
                     ? new IncludePropagator(_s, _d)
                     : new ExcludePropagator(_s, _d));
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
  return ENTAILED;
}
