/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#ifndef __COUNT_HH__
#define __COUNT_HH__

#include "std.hh"

//-----------------------------------------------------------------------------
// controller for implementations of FD.exactly, FD.atLeast, FD.atMost
//
// dd[i]=-1 indicates a dropped parameter
// dd[i]=-2 indicates a parameter that is being dropped

class CountPropagatorController {
protected:
  OZ_FDIntVar &v;
  OZ_FDIntVar * vv;
  int * dd;
  int size;
public:
  CountPropagatorController(int s, OZ_FDIntVar i1[], int d1[], OZ_FDIntVar &i2)
    : size(s), vv(i1), dd(d1), v(i2) {}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = v.leave();
    for (int i = size; i--;) {
      int d=dd[i];
      if (d == -1) {}
      else if (d == -2) {
	vv[i].leave();
	dd[i] = -1;
      } else {
	vars_left |= vv[i].leave();
      }
    }
    return vars_left ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    v.leave();
    for (int i = size; i--;)
      if (dd[i] != -1) vv[i].leave();
    return PROCEED;
  }
  OZ_Return fail(void) {
    v.fail();
    for (int i = size; i--;)
      if (dd[i] != -1) vv[i].fail();
    return FAILED;
  }
};

//-----------------------------------------------------------------------------
// common base class for implementation of FD.exactly, FD.atLeast, FD.atMost

class BaseCountPropagator : public Propagator_D_VD_I {
  friend INIT_FUNC(fdp_init);
protected:
  // the naming scheme for class members is inherited from Tobias
  // reg_tn  is the number of entailed equations so far
  // reg_tnn is the number of disentaield equations so far
  // when an equation is either entailed or disentailed, the corresponding
  // parameter is dropped and replaced by nil in the vector
  // oldSize is the size of the vector
  // reg_oldDomSizes is a vector caching the previous sizes of parameters
  //     this is used to avoid reprocessing a parameter which hasn't changed
  //     also it is used to indicate dropped parameters:
  //         -1 indicates a dropped parameter
  //         -2 indicates a parameter that is being dropped: it still needs
  //            a call to leave() and then to be replaced by nil in the vector
  //            and a -1 is put in place of -2
  int reg_tn, reg_tnn, oldSize;
  int * reg_oldDomSizes;
  void init_l_u(void)
  {
    reg_tn=0;
    reg_tnn=0;
    oldSize = reg_l_sz;
    reg_oldDomSizes = OZ_hallocCInts(reg_l_sz);
    int fds1 = OZ_getFDSup() + 1;
    for (int i = reg_l_sz; i--; ) 
      reg_oldDomSizes[i] = fds1;
  }
  static OZ_PropagatorProfile profile;
public:
  BaseCountPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VD_I(n, l, v) {init_l_u();}

  virtual void gCollect(void) {
    Propagator_D_VD_I::gCollect();
    reg_oldDomSizes = OZ_copyCInts(reg_l_sz, reg_oldDomSizes);
  }
  virtual void sClone(void) {
    Propagator_D_VD_I::sClone();
    reg_oldDomSizes = OZ_copyCInts(reg_l_sz, reg_oldDomSizes);
  }

  virtual ~BaseCountPropagator(void);
};

//-----------------------------------------------------------------------------
// template to generate the implementations for FD.exactly, FD.atLeast, and
// FD.atMost.  It has two boolean parameters: atleast and atmost.

template<const bool atleast,const bool atmost>
class CountPropagator : public BaseCountPropagator
{
public:
  CountPropagator(OZ_Term n,OZ_Term l,OZ_Term v)
    : BaseCountPropagator(n,l,v) {}
  virtual OZ_Return propagate(void);
};

template<const bool atleast,const bool atmost>
OZ_Return CountPropagator<atleast,atmost>::propagate(void)
{
  if (reg_l_sz == 0)
    return (atleast)?replaceByInt(reg_n, 0):PROCEED;

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

  int i;

 recheck:
  for (i = l_sz; i--; ) {
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
    // satisfaction surplus: upper and lower bounds
    int ss_hi = oldSize - tnn - n;
    int ss_lo = tn - n;
    if (ss_hi <  0) { if (atleast) goto failure; else goto vanish; }
    if (ss_hi == 0) {
      if (atleast) {
	for (i = l_sz; i--; )
	  if (reg_oldDomSizes[i]>=0 && l[i]->isIn(v))
	    FailOnEmpty(*l[i] &= v);
      }
      goto vanish;
    }
    if (ss_lo > 0) { if (atmost) goto failure; else goto vanish; }
    if (ss_lo == 0) {
      if (atmost) {
	for (i = l_sz; i--; )
	  if (reg_oldDomSizes[i]>=0 && *l[i] != fd_singl)
	    FailOnEmpty(*l[i] -= v);
      }
      goto vanish;
    }
  } else {
    // propagate into the index
    int sz_before = n_var->getSize();
    int sz;
    if (atmost)  { FailOnEmpty((sz = (*n_var >= tn))); }
    if (atleast) { FailOnEmpty((sz = (*n_var <= (oldSize - tnn)))); }
    if (n_in_l && sz_before!=sz) goto recheck;
    if (sz==1) goto N_det;
  }

  // we fall through to here when we need to suspend again
  // we need to update the cached sizes of the domains
  for (i=l_sz; i--;)
    if (reg_oldDomSizes[i] >= 0) {
      reg_oldDomSizes[i] = l[i]->getSize();
    }

  return P.leave();

 vanish:
  return P.vanish();

 failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

class ExactlyPropagator : public CountPropagator<true,true> {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  ExactlyPropagator(OZ_Term n,OZ_Term l,OZ_Term v)
    : CountPropagator<true,true>(n,l,v) {}
  virtual OZ_PropagatorProfile *getProfile(void) const { return &profile; }
  virtual size_t sizeOf(void) { return sizeof(ExactlyPropagator); }
};

class AtLeastPropagator : public CountPropagator<true,false> {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  AtLeastPropagator(OZ_Term n,OZ_Term l,OZ_Term v)
    : CountPropagator<true,false>(n,l,v) {}
  virtual OZ_PropagatorProfile *getProfile(void) const { return &profile; }
  virtual size_t sizeOf(void) { return sizeof(ExactlyPropagator); }
};

class AtMostPropagator : public CountPropagator<false,true> {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  AtMostPropagator(OZ_Term n,OZ_Term l,OZ_Term v)
    : CountPropagator<false,true>(n,l,v) {}
  virtual OZ_PropagatorProfile *getProfile(void) const { return &profile; }
  virtual size_t sizeOf(void) { return sizeof(ExactlyPropagator); }
};

//-----------------------------------------------------------------------------

class ElementPropagator : public Propagator_D_VI_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  ElementPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VI_D(n, l, v) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

#endif // __COUNT_HH__
