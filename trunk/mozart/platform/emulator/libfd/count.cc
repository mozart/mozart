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

#include "count.hh"
#include "auxcomp.hh"

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

ExactlyPropagator::~ExactlyPropagator(void)
{
  OZ_hfreeCInts(reg_oldDomSizes, reg_l_sz);
}

OZ_Return ExactlyPropagator::propagate(void)
{
  if (reg_l_sz == 0) return replaceByInt(reg_n, 0);

  int &v = reg_v, &l_sz = reg_l_sz, n_in_l = 0;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  CountPropagatorController P(l_sz, l, reg_oldDomSizes, n_var);

  // tn  is the number of entailed equations
  // tnn is the number of disentailed equations

  int tn  = reg_tn;
  int tnn = reg_tnn;

  // only check a var if it has changed
  // this is achieved by caching the sizes of the
  // domains of the FD vars in L from one run of
  // propagate to the next
  // the cached size is -1 when the var has been dropped
  // -2 when it it is about to be dropped

 recheck:
  for (int i = l_sz; i--; ) {
    int sz = reg_oldDomSizes[i];
    if (sz<0) continue;
    l[i].read(reg_l[i]);
    // find out if n_var occurs in l
    if (n_in_l==0 && (&(*n_var)==&(*l[i]))) n_in_l = 1;
    int li_sz = l[i]->getSize();
    if (li_sz < sz) {
      if (li_sz == 1) {
	if (l[i]->getSingleElem() == v)
	  tn += 1;
	else
	  tnn += 1;
	// we never need to check this one again
	reg_oldDomSizes[i] = -2;
      }
      else {
	if (! l[i]->isIn(v)) {
	  tnn += 1;
	  // we never need to check this one again
	  reg_oldDomSizes[i] = -2;
	  // and we should not suspend on it anymore
	  l[i].dropParameter();
	  reg_l[i] = OZ_nil();
	}
      }
    }
  }

  // write back the updated results
  reg_tn  = tn;
  reg_tnn = tnn;

  // frequent special case: N determined
  if (*n_var == fd_singl) {
  N_det:
    int n = n_var->getSingleElem();
    if ((oldSize - tnn < n) || (tn > n)) {
      goto failure;
    }
    if (tn == n) {
      for (int i = l_sz; i--; )
	if (reg_oldDomSizes[i]>=0 && *l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      return P.vanish();
    } else if (oldSize - tnn == n) {
      for (int i = l_sz; i--; )
	if (reg_oldDomSizes[i]>=0 && l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      return P.vanish();
    }
  } else {
    // propagate into the index
    int sz_before = n_var->getSize();
    FailOnEmpty(*n_var >= tn);
    int sz = (*n_var <= (oldSize - tnn));
    if (sz==0) goto failure;
    if (n_in_l && sz_before!=sz) goto recheck;
    if (sz==1) goto N_det;
  }

  // we fall through to here when we need to suspend again
  // we need to update the cached sizes of the domains
  for (int i=l_sz; i--;)
    if (reg_oldDomSizes[i] >= 0) {
      reg_oldDomSizes[i] = l[i]->getSize();
    }

  return P.leave();

failure:
  return P.fail();
}

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

AtLeastPropagator::~AtLeastPropagator(void)
{
  OZ_hfreeCInts(reg_oldDomSizes, reg_l_sz);
}

// {FD.atLeast N L V}
OZ_Return AtLeastPropagator::propagate(void)
{
  if (reg_l_sz == 0) return replaceByInt(reg_n, 0);

  int &v = reg_v, &l_sz = reg_l_sz, n_in_l = 0;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  CountPropagatorController P(l_sz, l, reg_oldDomSizes, n_var);

  // tn  is the number of entailed equations
  // tnn is the number of disentailed equations

  int tn  = reg_tn;
  int tnn = reg_tnn;

  // only check a var if it has changed
  // this is achieved by caching the sizes of the
  // domains of the FD vars in L from one run of
  // propagate to the next
  // the cached size is -1 when the var has been dropped
  // -2 when it it is about to be dropped

 recheck:
  for (int i = l_sz; i--; ) {
    int sz = reg_oldDomSizes[i];
    if (sz<0) continue;
    l[i].read(reg_l[i]);
    // find out if n_var occurs in l
    if (n_in_l==0 && (&(*n_var)==&(*l[i]))) n_in_l = 1;
    int li_sz = l[i]->getSize();
    if (li_sz < sz) {
      if (li_sz == 1) {
	if (l[i]->getSingleElem() == v)
	  tn += 1;
	else
	  tnn += 1;
	// we never need to check this one again
	reg_oldDomSizes[i] = -2;
      }
      else {
	if (! l[i]->isIn(v)) {
	  tnn += 1;
	  // we never need to check this one again
	  reg_oldDomSizes[i] = -2;
	  // and we should not suspend on it anymore
	  l[i].dropParameter();
	  reg_l[i] = OZ_nil();
	}
      }
    }
  }

  // write back the updated results
  reg_tn  = tn;
  reg_tnn = tnn;

  // frequent special case: N determined
  if (*n_var == fd_singl) {
  N_det:
    int n = n_var->getSingleElem();
    if (oldSize - tnn == n) {
      for (int i = l_sz; i--; )
	if (reg_oldDomSizes[i]>=0 && l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      return P.vanish();
    } else if  (oldSize - tnn < n) {
      goto failure;
    } else if (tn >= n) {
      return P.vanish();
    }
  } else {
    // propagate into the index
    int sz_before = n_var->getSize();
    int sz = (*n_var <= (oldSize - tnn));
    if (sz==0) goto failure;
    // if n occurs in l, constraining n may have changed the count
    if (n_in_l && sz_before!=sz) goto recheck;
    // if n is now determined: branch to special case
    if (sz==1) goto N_det;
  }

  // we fall through to here when we need to suspend again
  // we need to update the cached sizes of the domains
  for (int i=l_sz; i--;)
    if (reg_oldDomSizes[i] >= 0) {
      reg_oldDomSizes[i] = l[i]->getSize();
    }

  return P.leave();

failure:
  return P.fail();
}

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

AtMostPropagator::~AtMostPropagator(void)
{
  OZ_hfreeCInts(reg_oldDomSizes, reg_l_sz);
}

// {FS.atMost N L V}
OZ_Return AtMostPropagator::propagate(void)
{
  if (reg_l_sz == 0) return PROCEED;

  int &v = reg_v, &l_sz = reg_l_sz, n_in_l = 0;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  CountPropagatorController P(l_sz, l, reg_oldDomSizes, n_var);

  // tn  is the number of entailed equations
  // tnn is the number of disentailed equations

  int tn  = reg_tn;
  int tnn = reg_tnn;

  // only check a var if it has changed
  // this is achieved by caching the sizes of the
  // domains of the FD vars in L from one run of
  // propagate to the next
  // the cached size is -1 when the var has been dropped
  // -2 when it it is about to be dropped

 recheck:
  for (int i = l_sz; i--; ) {
    int sz = reg_oldDomSizes[i];
    if (sz<0) continue;
    l[i].read(reg_l[i]);
    // find out if n_var occurs in l
    if (n_in_l==0 && (&(*n_var)==&(*l[i]))) n_in_l = 1;
    int li_sz = l[i]->getSize();
    if (li_sz < sz) {
      if (li_sz == 1) {
	if (l[i]->getSingleElem() == v)
	  tn += 1;
	else
	  tnn += 1;
	// we never need to check this one again
	reg_oldDomSizes[i] = -2;
      }
      else {
	if (! l[i]->isIn(v)) {
	  tnn += 1;
	  // we never need to check this one again
	  reg_oldDomSizes[i] = -2;
	  // and we should not suspend on it anymore
	  l[i].dropParameter();
	  reg_l[i] = OZ_nil();
	}
      }
    }
  }

  // write back the updated results
  reg_tn  = tn;
  reg_tnn = tnn;

  // frequent special case: N determined
  if (*n_var == fd_singl) {
  N_det:
    int n = n_var->getSingleElem();
    if (tn == n) {
      for (int i = l_sz; i--; )
	if (reg_oldDomSizes[i]>=0 && *l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      return P.vanish();
    } else if  (tn > n) {
      goto failure;
    } else if (oldSize - tnn <= n) {
      return P.vanish();
    }
  } else {
    // propagate into the index
    int sz_before = n_var->getSize();
    int sz = (*n_var >= tn);
    if (sz == 0) goto failure;
    // if n occurs in l, constraining n may have changed the count
    if (n_in_l && sz_before!=sz) goto recheck;
    // if N is now determined: branch to special case
    if (sz==1) goto N_det;
  }

  // we fall through to here when we need to suspend again
  // we need to update the cached sizes of the domains
  for (int i=l_sz; i--;)
    if (reg_oldDomSizes[i] >= 0) {
      reg_oldDomSizes[i] = l[i]->getSize();
    }

  return P.leave();

failure:
  return P.fail();
}

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
