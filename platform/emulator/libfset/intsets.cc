/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "intsets.hh"

OZ_C_proc_begin(fsp_min, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);

  int susp_count_dummy;
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count_dummy);
  
  return pe.impose(new FSetsMinPropagator(OZ_args[0],
					  OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun FSetsMinPropagator::header = fsp_min;

OZ_Return FSetsMinPropagator::propagate(void)
{
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  // This is a naive and quick-and-dirty implementation but since your
  // current problem is the number of propagators, it will suffice. A
  // better solution needs support from the class representing the
  // finite set intervals. I will do this later. Tobias.

  // card(s) > 0
  FailOnInvalid(s->putCard(1, 32*fset_high));

  for (int i = 0; i < 32*fset_high; i += 1) {
    // i < min(d) ==> i not in s
    if (i < d->getMinElem())
      FailOnInvalid(*s -= i);
    
    // i in s ==> min(d) <= i
    if (s->isIn(i)) 
      FailOnEmpty(*d <= i);
  }

  return P.leave1();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// eof
