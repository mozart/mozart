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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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

OZ_C_proc_begin(fdp_exactly, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.impose(new ExactlyPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

ExactlyPropagator::~ExactlyPropagator(void) 
{
  OZ_hfreeCInts(reg_oldDomSizes, reg_l_sz);
}

OZ_Return ExactlyPropagator::propagate(void)
{ 

  if (reg_l_sz == 0) return replaceByInt(reg_n, 0);

  int &v = reg_v, &l_sz = reg_l_sz;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  PropagatorController_VV_V P(l_sz, l, n_var); 
  /*
    tn denotes number items in list which are v and tnn number of
    items which are definitively not v
  */
  int tn = 0, tnn = 0, i;

  for (i = l_sz; i--; ) {
    l[i].read(reg_l[i]);
    if (l[i]->getSize() < reg_oldDomSizes[i]) {
      if (*l[i] == fd_singl && l[i]->getSingleElem() == v) 
	tn += 1;
      else { 
	if (! l[i]->isIn(v)) 
	  tnn += 1;    
      }
    }
  }


  reg_tn += tn; 
  reg_tnn += tnn;
  tn = reg_tn;
  tnn = reg_tnn;

loop:
  if (*n_var == fd_singl) {
    int n = n_var->getSingleElem();
    if ( (oldSize - tnn < n) || (tn > n) ) {
      goto failure;
    } 
    if (tn == n) {
      for (i = l_sz; i--; ) 
	if (*l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      return P.vanish(); 
    } else if (oldSize - tnn == n) {
      for (i = l_sz; i--; )
	if (l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      return P.vanish();
    }   
  } else {
    int oldIn=0, newIn=0;
    if (n_var->isIn(v)) oldIn = 1;
    if ( (oldSize - tnn < n_var->getMinElem()) || 
	 (tn > n_var->getMaxElem()) ) {
      goto failure;
    }   
    if (tn == n_var->getMaxElem()) {
      for (i = l_sz; i--; ) 
	if (*l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      FailOnEmpty(*n_var &= tn);
      return P.vanish(); 
    } else if (oldSize - tnn == n_var->getMinElem()) {
      for (i = l_sz; i--; )
	if (l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      FailOnEmpty(*n_var &= oldSize - tnn);
      return P.vanish();
    }
    FailOnEmpty(*n_var <= oldSize - tnn);
    FailOnEmpty(*n_var >= tn);
    if (n_var->isIn(v)) newIn = 1;
    if (newIn != oldIn) {
      reg_tnn++; tnn=reg_tnn;
      goto loop;
    }
    else {
      if ((oldIn==1) && (*n_var == fd_singl)) {
      reg_tn++; tn=reg_tn;
      goto loop;
      }
    }
  }

  int from, to;
  for (from = 0, to = 0; from < l_sz; from += 1) {
    if ((*l[from] == fd_singl) && (l[from]->getMinElem() == v)) {}
    else {
      if (!l[from]->isIn(v)) {}
      else {
	reg_oldDomSizes[to] = l[from]->getSize();
	reg_l[to] = reg_l[from];
	to++;
      }
    }
  }
  
  l_sz = to;

  // if n_var occurs in reg_l we must consider the case that
  // the list becomes empty but n_var is still a FD.
  if (l_sz == 0) {
    FailOnEmpty(*n_var &= n_var->getMinElem());
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

  return pe.impose(new AtLeastPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

AtLeastPropagator::~AtLeastPropagator(void) 
{
  OZ_hfreeCInts(reg_oldDomSizes, reg_l_sz);
}

OZ_Return AtLeastPropagator::propagate(void)
{
  if (reg_l_sz == 0) return replaceByInt(reg_n, 0);

  int &v = reg_v, &l_sz = reg_l_sz;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  PropagatorController_VV_V P(l_sz, l, n_var);

  int tn = 0, tnn = 0, i;

  for (i = l_sz; i--; ) {
    l[i].read(reg_l[i]);
    if (l[i]->getSize() < reg_oldDomSizes[i]) {
      if (*l[i] == fd_singl && l[i]->getSingleElem() == v) 
	tn += 1;
      else { 
	if (! l[i]->isIn(v)) 
	  tnn += 1;    
      }
    }
  }

  reg_tn += tn; 
  reg_tnn += tnn;
  tn = reg_tn;
  tnn = reg_tnn;

loop:
  if (*n_var == fd_singl) {
    int n = n_var->getSingleElem();
    if (oldSize - tnn == n) {
      for (i = l_sz; i--; ) 
	if (l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      return P.vanish();
    } else if  (oldSize - tnn < n) {
      goto failure;
    } else if (tn >= n) {
      return P.vanish();
    }
  } else {
    int oldIn=0, newIn=0;
    if (n_var->isIn(v)) oldIn = 1;
    if (oldSize - tnn == n_var->getMinElem()) {
      for (i = l_sz; i--; ) 
	if (l[i]->isIn(v))
	  FailOnEmpty(*l[i] &= v);
      FailOnEmpty(*n_var <= oldSize - tnn);
      return P.vanish();
    } else if  (oldSize - tnn < n_var->getMinElem()) {
      goto failure;
    } else if (tn >= n_var->getMaxElem()) {
      return P.vanish();
    }
  
    FailOnEmpty(*n_var <= oldSize - tnn);
    if (n_var->isIn(v)) newIn = 1;
    if (newIn != oldIn) {
      reg_tnn++; tnn=reg_tnn;
      goto loop;
    }
    else {
      if ((oldIn==1) && (*n_var == fd_singl)) {
      reg_tn++; tn=reg_tn;
      goto loop;
      }
    }
  }

  int from, to;
  for (from = 0, to = 0; from < l_sz; from += 1) {
    if ((*l[from] == fd_singl) && (l[from]->getMinElem() == v)) {}
    else {
      if (!l[from]->isIn(v)) {}
      else {
	reg_oldDomSizes[to] = l[from]->getSize();
	reg_l[to] = reg_l[from];
	to++;
      }
    }
  }
  
  l_sz = to;

  if (l_sz == 0) {
    FailOnEmpty(*n_var &= n_var->getMinElem());
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

  return pe.impose(new AtMostPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

AtMostPropagator::~AtMostPropagator(void) 
{
  OZ_hfreeCInts(reg_oldDomSizes, reg_l_sz);
}

OZ_Return AtMostPropagator::propagate(void)
{
  if (reg_l_sz == 0) return PROCEED;

  int &v = reg_v, &l_sz = reg_l_sz;
  OZ_FDIntVar n_var(reg_n);
  DECL_DYN_ARRAY(OZ_FDIntVar, l, l_sz);
  PropagatorController_VV_V P(l_sz, l, n_var);
  
  int tn = 0, tnn = 0, i;

  for (i = l_sz; i--; ) {
    l[i].read(reg_l[i]);
    if (l[i]->getSize() < reg_oldDomSizes[i]) {
      if (*l[i] == fd_singl && l[i]->getSingleElem() == v) 
	tn += 1;
      else { 
	if (! l[i]->isIn(v)) 
	  tnn += 1;    
      }
    }
  }

  reg_tn += tn; 
  reg_tnn += tnn;
  tn = reg_tn;
  tnn = reg_tnn;

loop:
  if (*n_var == fd_singl) {
    int n = n_var->getSingleElem();
    if (tn == n) {
      for (i = l_sz; i--; ) 
	if (*l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      return P.vanish();
    } else if  (tn > n) {
      goto failure;
    } else if (oldSize - tnn <= n) {
      return P.vanish();
    }
  } else {
    int oldIn=0, newIn=0;
    if (n_var->isIn(v)) oldIn = 1;
    if (n_var->getMaxElem() == tn) {
      for (i = l_sz; i--; ) 
	if (*l[i] != fd_singl)
	  FailOnEmpty(*l[i] -= v);
      FailOnEmpty(*n_var >= tn);
      return P.vanish();
    } else if  (tn > n_var->getMaxElem()) {
      goto failure;
    } else if (oldSize - tnn <= n_var->getMinElem()) {
      return P.vanish();
    }
    
    FailOnEmpty(*n_var >= tn);
    if (n_var->isIn(v)) newIn = 1;
    if (newIn != oldIn) {
      reg_tnn++; tnn=reg_tnn;
      goto loop;
    }
    else {
      if ((oldIn==1) && (*n_var == fd_singl)) {
      reg_tn++; tn=reg_tn;
      goto loop;
      }
    }
  }
  
  int from, to;
  for (from = 0, to = 0; from < l_sz; from += 1) {
    if ((*l[from] == fd_singl) && (l[from]->getMinElem() == v)) {}
    else {
      if (!l[from]->isIn(v)) {}
      else {
	reg_oldDomSizes[to] = l[from]->getSize();
	reg_l[to] = reg_l[from];
	to++;
      }
    }
  }
  
  l_sz = to;

  if (l_sz == 0) {
    return P.vanish();
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

  return pe.impose(new ElementPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

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

OZ_CFunHeader ExactlyPropagator::spawner = fdp_exactly;
OZ_CFunHeader AtLeastPropagator::spawner = fdp_atLeast;
OZ_CFunHeader AtMostPropagator::spawner = fdp_atMost;
OZ_CFunHeader ElementPropagator::spawner = fdp_element; 
