/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include <iostream.h>
#include <limits.h>

#include "count.hh"
#include "auxcomp.hh"

//-----------------------------------------------------------------------------
// Exactly

OZ_C_proc_begin(fdp_exactly, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.spawn(new ExactlyPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return ExactlyPropagator::run(void)
{
  if (reg_l_sz == 0) return replaceByInt(reg_n, 0);

  int &v = reg_v, &l_sz = reg_l_sz;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  PropagatorController_VV_V P(l_sz, l, n_var);
  
  int tn = 0, tnn = 0, i;

  for (i = l_sz; i--; ) {
    l[i].read(reg_l[i]);
    if (*l[i] == fd_singl && l[i]->getSingleElem() == v) tn += 1;
    if (! l[i]->isIn(v)) tnn += 1;    
  }

  if (*n_var == fd_singl) {
    int n = n_var->getSingleElem();
    if (tn == n) {
      for (i = l_sz; i--; ) 
	if (*l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      return P.vanish(); 
    } else if (l_sz - tnn == n) {
      for (i = l_sz; i--; )
	if (l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
    } else if ( (l_sz - tnn < n) || (tn > n) ) {
      goto failure;
    }   
  } else {
    if (tn == n_var->getMaxElem()) {
      for (i = l_sz; i--; ) 
	if (*l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      FailOnEmpty(*n_var &= tn);
      return P.vanish(); 
    } else if (l_sz - tnn == n_var->getMinElem()) {
      for (i = l_sz; i--; )
	if (l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      FailOnEmpty(*n_var &= l_sz - tnn);
    } else if ( (l_sz - tnn < n_var->getMinElem()) || (tn > n_var->getMaxElem()) ) {
      goto failure;
    }   

    FailOnEmpty(*n_var <= l_sz - tnn);
    FailOnEmpty(*n_var >= tn);
  }

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// AtLeast

OZ_C_proc_begin(fdp_atLeast, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.spawn(new AtLeastPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return AtLeastPropagator::run(void)
{
  if (reg_l_sz == 0) return replaceByInt(reg_n, 0);

  int &v = reg_v, &l_sz = reg_l_sz;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  PropagatorController_VV_V P(l_sz, l, n_var);
  int tn = 0, tnn = 0, i;

  for (i = l_sz; i--; ) {
    l[i].read(reg_l[i]);
    if (*l[i] == fd_singl && l[i]->getSingleElem() == v) tn += 1;
    if (! l[i]->isIn(v)) tnn += 1;    
  }

  if (*n_var == fd_singl) {
    int n = n_var->getSingleElem();
    if (l_sz - tnn == n) {
      for (i = l_sz; i--; ) 
	if (l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      return P.vanish();
    } else if  (l_sz - tnn < n) {
      goto failure;
    } else if (tn >= n) {
      return P.vanish();
    }
  } else {
    if (l_sz - tnn == n_var->getMinElem()) {
      for (i = l_sz; i--; ) 
	if (l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      FailOnEmpty(*n_var <= l_sz - tnn);
      return P.vanish();
    } else if  (l_sz - tnn < n_var->getMinElem()) {
      goto failure;
    } else if (tn >= n_var->getMaxElem()) {
      return P.vanish();
    }
  
    FailOnEmpty(*n_var <= l_sz - tnn);
  }

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// AtMost

OZ_C_proc_begin(fdp_atMost, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.spawn(new AtMostPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return AtMostPropagator::run(void)
{
  if (reg_l_sz == 0) return PROCEED;

  int &v = reg_v, &l_sz = reg_l_sz;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  PropagatorController_VV_V P(l_sz, l, n_var);
  
  int tn = 0, tnn = 0, i;

  for (i = l_sz; i--; ) {
    l[i].read(reg_l[i]);
    if (*l[i] == fd_singl && l[i]->getSingleElem() == v) tn += 1;
    if (! l[i]->isIn(v)) tnn += 1;    
  }

  if (*n_var == fd_singl) {
    int n = n_var->getSingleElem();
    if (tn == n) {
      for (i = l_sz; i--; ) 
	if (*l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      return P.vanish();
    } else if  (tn > n) {
      goto failure;
    } else if (l_sz - tnn <= n) {
      return P.vanish();
    }
  } else {
    if (n_var->getMaxElem() == tn) {
      for (i = l_sz; i--; ) 
	if (*l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      FailOnEmpty(*n_var >= tn);
      return P.vanish();
    } else if  (tn > n_var->getMaxElem()) {
      goto failure;
    } else if (l_sz - tnn <= n_var->getMinElem()) {
      return P.vanish();
    }
    
    FailOnEmpty(*n_var >= tn);
  }
  
  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_element, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorInt);
  OZ_EXPECT(pe, 2, expectIntVarAny);

  return pe.spawn(new ElementPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return ElementPropagator::run(void)
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

OZ_CFun ExactlyPropagator::spawner = fdp_exactly;
OZ_CFun AtLeastPropagator::spawner = fdp_atLeast;
OZ_CFun AtMostPropagator::spawner = fdp_atMost;
OZ_CFun ElementPropagator::spawner = fdp_element;
