/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#ifndef __COUNT_HH__
#define __COUNT_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class ExactlyPropagator : public Propagator_D_VD_I {
private:
  int reg_tn, reg_tnn, oldSize;
  int * reg_oldDomSizes;
  void init_l_u(void)
  {
    reg_tn=0;
    reg_tnn=0;
    oldSize = reg_l_sz;
    reg_oldDomSizes = OZ_hallocCInts(reg_l_sz);
    for (int i = reg_l_sz; i--; )
      reg_oldDomSizes[i] = OZ_getFDSup() + 1;
  }
  static OZ_CFunHeader spawner;
public:
  ExactlyPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    //    : Propagator_D_VD_I(n, l, v) {}
    : Propagator_D_VD_I(n, l, v) {init_l_u();}
  virtual void updateHeapRefs(OZ_Boolean d)
  {
    Propagator_D_VD_I::updateHeapRefs(d);
    int * new_reg_oldDomSizes = OZ_hallocCInts(reg_l_sz);
    for (int i = reg_l_sz; i--; )
      new_reg_oldDomSizes[i] = reg_oldDomSizes[i];
    reg_oldDomSizes = new_reg_oldDomSizes;
  }

  virtual ~ExactlyPropagator(void);
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual size_t sizeOf(void) { return sizeof(*this); }
};

//-----------------------------------------------------------------------------

class AtLeastPropagator : public Propagator_D_VD_I {
private:
  int reg_tn, reg_tnn, oldSize;
  int * reg_oldDomSizes;
  void init_l_u(void)
  {
    reg_tn=0;
    reg_tnn=0;
    oldSize = reg_l_sz;
    reg_oldDomSizes = OZ_hallocCInts(reg_l_sz);
    for (int i = reg_l_sz; i--; )
      reg_oldDomSizes[i] = OZ_getFDSup() + 1;
  }
  static OZ_CFunHeader spawner;
public:
  AtLeastPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
   //    : Propagator_D_VD_I(n, l, v) {}
    : Propagator_D_VD_I(n, l, v) {init_l_u();}
  virtual void updateHeapRefs(OZ_Boolean d)
  {
    Propagator_D_VD_I::updateHeapRefs(d);
    int * new_reg_oldDomSizes = OZ_hallocCInts(reg_l_sz);
    for (int i = reg_l_sz; i--; )
      new_reg_oldDomSizes[i] = reg_oldDomSizes[i];
    reg_oldDomSizes = new_reg_oldDomSizes;
  }
  virtual ~AtLeastPropagator(void);
  virtual size_t sizeOf(void) { return sizeof(*this); }
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class AtMostPropagator : public Propagator_D_VD_I {
private:
  int reg_tn, reg_tnn, oldSize;
  int * reg_oldDomSizes;
  void init_l_u(void)
  {
    reg_tn=0;
    reg_tnn=0;
    oldSize = reg_l_sz;
    reg_oldDomSizes = OZ_hallocCInts(reg_l_sz);
    for (int i = reg_l_sz; i--; )
      reg_oldDomSizes[i] = OZ_getFDSup() + 1;
  }
static OZ_CFunHeader spawner;
public:
  AtMostPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VD_I(n, l, v) {init_l_u();}
  virtual void updateHeapRefs(OZ_Boolean d)
  {
    Propagator_D_VD_I::updateHeapRefs(d);
    int * new_reg_oldDomSizes = OZ_hallocCInts(reg_l_sz);
    for (int i = reg_l_sz; i--; )
      new_reg_oldDomSizes[i] = reg_oldDomSizes[i];
    reg_oldDomSizes = new_reg_oldDomSizes;
  }
  virtual ~AtMostPropagator(void);
  virtual size_t sizeOf(void) { return sizeof(*this); }

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class ElementPropagator : public Propagator_D_VI_D {
private:
  static OZ_CFunHeader spawner;
public:
  ElementPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VI_D(n, l, v) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

#endif // __COUNT_HH__
