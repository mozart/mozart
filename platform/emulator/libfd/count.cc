/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Joerg Wuertz (wuertz@ps.uni-sb.de)
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

#include <limits.h>

#if defined(INTERFACE)
#pragma implementation "count.hh"
#endif

#include "base.hh"
#include "count.hh"
#include "auxcomp.hh"

template class CountPropagator<true,true>;
template class CountPropagator<false,true>;
template class CountPropagator<true,false>;

//-----------------------------------------------------------------------------
// BaseCount

BaseCountPropagator::~BaseCountPropagator()
{
  OZ_hfreeCInts(reg_oldDomSizes, reg_l_sz);
}

//-----------------------------------------------------------------------------
// Exactly

OZ_BI_define(fdp_exactly, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.impose(new ExactlyPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

//-----------------------------------------------------------------------------
// AtLeast

OZ_BI_define(fdp_atLeast, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.impose(new AtLeastPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

//-----------------------------------------------------------------------------
// AtMost

OZ_BI_define(fdp_atMost, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.impose(new AtMostPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_element, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorInt);
  OZ_EXPECT(pe, 2, expectIntVarAny);

  return pe.impose(new ElementPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_Return ElementPropagator::propagate(void)
{
  if (reg_l_sz == 0) return FAILED;

  int &l_sz = reg_l_sz;
  int * l = reg_l;

  OZ_FDIntVar n_var(reg_n), v_var(reg_v);
  PropagatorController_V_V P(n_var, v_var);
  OZ_FiniteDomain v_dom_new(fd_empty);

  if (n_var->getMinElem() == 0)
    FailOnEmpty(*n_var >= 1);
  if (n_var->getMaxElem() > l_sz)
    FailOnEmpty(*n_var <= l_sz);

  {
    for (int i = 0; i < l_sz; i += 1)
      if (l[i] != INT_MIN) {
	int l1 = i + 1;
	if (n_var->isIn(l1)) {
	  if (! v_var->isIn(l[i])) {
	    FailOnEmpty(*n_var -= l1);
	  } else {
	    v_dom_new += l[i];
	  }
	} else {
	  l[i] = INT_MIN;
	}
      }
  }

  if (*n_var == fd_singl) {
    FailOnEmpty(*v_var &= l[n_var->getSingleElem() - 1]);
    return P.vanish();
  } else if (v_dom_new == fd_empty) {
    goto failure;
  } else {
    *v_var &= v_dom_new;
  }
  return (*v_var == fd_singl) ? P.vanish() : P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// static member

OZ_PropagatorProfile ExactlyPropagator::profile;
OZ_PropagatorProfile AtLeastPropagator::profile;
OZ_PropagatorProfile AtMostPropagator::profile;
OZ_PropagatorProfile ElementPropagator::profile;
