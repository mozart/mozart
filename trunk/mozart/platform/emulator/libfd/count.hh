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

class ExactlyPropagator : public Propagator_D_VD_I {
  friend INIT_FUNC(fdp_init);
private:
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
  ExactlyPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    //    : Propagator_D_VD_I(n, l, v) {}
    : Propagator_D_VD_I(n, l, v) {init_l_u();}
  virtual void updateHeapRefs(OZ_Boolean d)
  {
    Propagator_D_VD_I::updateHeapRefs(d);
    reg_oldDomSizes = OZ_copyCInts(reg_l_sz, reg_oldDomSizes);
  }

  virtual ~ExactlyPropagator(void);
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual size_t sizeOf(void) { return sizeof(*this); }
};

//-----------------------------------------------------------------------------

class AtLeastPropagator : public Propagator_D_VD_I {
  friend INIT_FUNC(fdp_init);
private:
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
  AtLeastPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
   //    : Propagator_D_VD_I(n, l, v) {}
    : Propagator_D_VD_I(n, l, v) {init_l_u();}
  virtual void updateHeapRefs(OZ_Boolean d)
  {
    Propagator_D_VD_I::updateHeapRefs(d);
    reg_oldDomSizes = OZ_copyCInts(reg_l_sz, reg_oldDomSizes);
  }
  virtual ~AtLeastPropagator(void);
  virtual size_t sizeOf(void) { return sizeof(*this); }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------

class AtMostPropagator : public Propagator_D_VD_I {
  friend INIT_FUNC(fdp_init);
private:
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
  AtMostPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VD_I(n, l, v) {init_l_u();}
  virtual void updateHeapRefs(OZ_Boolean d)
  {
    Propagator_D_VD_I::updateHeapRefs(d);
    reg_oldDomSizes = OZ_copyCInts(reg_l_sz, reg_oldDomSizes);
  }
  virtual ~AtMostPropagator(void);
  virtual size_t sizeOf(void) { return sizeof(*this); }

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
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
