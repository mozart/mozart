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

// FSP_MIN WAS WRITTEN BY TOBIAS AS AN EXAMPLE

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

  // DENYS: d < 32*fset_high
  FailOnEmpty(*d <= (32*fset_high - 1));

  // card(s) > 0
  FailOnInvalid(s->putCard(1, 32*fset_high));

  {
    for (int i = 0; i < 32*fset_high; i += 1) {
      // i < min(d) ==> i not in s
      if (i < d->getMinElem())
	FailOnInvalid(*s -= i);
      
      // i in s ==> min(d) <= i
      if (s->isIn(i)) {
	FailOnEmpty(*d <= i);
      }
      // DENYS: i not in s ==> d=/=i
      else if (s->isNotIn(i)) {
	FailOnEmpty(*d -= i);
      }
    }
  }

  // d is in s
  {
    int i = d->getSingleElem();
    if (i != -1)
      FailOnInvalid(*s += i);
  }

  return P.leave1();

failure:
  return P.fail();
}

// -------------------------------------------------------------------
// Max Propagator

OZ_C_proc_begin(fsp_max, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);

  int susp_count_dummy;
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count_dummy);
  
  return pe.impose(new FSetsMaxPropagator(OZ_args[0],
					  OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun FSetsMaxPropagator::header = fsp_max;

OZ_Return FSetsMaxPropagator::propagate(void)
{
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  // d < 32*fset_high
  FailOnEmpty(*d <= (32*fset_high - 1));

  // card(s) > 0
  FailOnInvalid(s->putCard(1, 32*fset_high));

  {
    for (int i = 0; i < 32*fset_high; i += 1) {
      // i > max(d) ==> i not in s
      if (i > d->getMaxElem())
	FailOnInvalid(*s -= i);
      
      // i in s ==> max(d) >= i
      if (s->isIn(i)) {
	FailOnEmpty(*d >= i);
      }
      // i not in s ==> d=/=i
      else if (s->isNotIn(i)) {
	FailOnEmpty(*d -= i);
      }
    }
  }

  // d is in s
  {
    int i = d->getSingleElem();
    if (i != -1)
      FailOnInvalid(*s += i);
  }

  return P.leave1();

failure:
  return P.fail();
}

//--------------------------------------------------------------------
// Convex Propagator

OZ_C_proc_begin(fsp_convex, 1)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET);

  PropagatorExpect pe;
  OZ_EXPECT(pe,0,expectFSetVarAny);
  return pe.impose(new FSetsConvexPropagator(OZ_args[0]));
}
OZ_C_proc_end

OZ_CFun FSetsConvexPropagator::header = fsp_convex;

OZ_Return FSetsConvexPropagator::propagate(void)
{
  OZ_FSetVar s(_s);
  int minelem,maxelem;
  for (int i=0;i<32*fset_high;i++) {
    if (s->isIn(i)) {
      minelem=maxelem=i;
      for(i++;i<32*fset_high;i++)
	if (s->isIn(i)) maxelem=i;
      // all ints between the smallest and largest known elements
      // must also be in
      for(int k=minelem+1;k<maxelem;k++)
	FailOnInvalid(*s += k);
      // find a non-element below minelem: all ints below are out
      for(i=minelem-1;i>=0;i--)
	if (s->isNotIn(i)) {
	  while (i--)
	    FailOnInvalid(*s -= i);
	  break;
	}
      // find a non-element above maxelem: all ints above are out
      for(i=maxelem+1;i<32*fset_high;i++)
	if (s->isNotIn(i)) {
	  while (++i < 32*fset_high)
	    FailOnInvalid(*s -= i);
	  break;
	}
      break;
    }
  }
  return s.leave()?SLEEP:OZ_ENTAILED;
failure:
  s.fail();
  return FAILED;
}

//-----------------------------------------------------------------------------
// eof
