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

#include "std.hh"

//-----------------------------------------------------------------------------

class NotEqOffPropagator : public Propagator_D_D_I {
private:
  static OZ_CFunHeader spawner;
public:
  NotEqOffPropagator(OZ_Term x, OZ_Term y,  int c) 
    : Propagator_D_D_I(x, y, c) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class LessEqOffPropagator : public Propagator_D_D_I {
private:
  static OZ_CFunHeader spawner;
public:
  LessEqOffPropagator(OZ_Term x, OZ_Term y,  int c) 
    : Propagator_D_D_I(x, y, c) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class MinimumPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  MinimumPropagator(OZ_Term x, OZ_Term y,  OZ_Term z) 
    : Propagator_D_D_D(x, y, z) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class MaximumPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  MaximumPropagator(OZ_Term x, OZ_Term y,  OZ_Term z) 
    : Propagator_D_D_D(x, y, z) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class IntersectionPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  IntersectionPropagator(OZ_Term x, OZ_Term y,  OZ_Term z) 
    : Propagator_D_D_D(x, y, z) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class UnionPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  UnionPropagator(OZ_Term x, OZ_Term y,  OZ_Term z) 
    : Propagator_D_D_D(x, y, z) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class SubSetPropagator : public Propagator_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  SubSetPropagator(OZ_Term x, OZ_Term y) 
    : Propagator_D_D(x, y) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class DistinctPropagator : public Propagator_VD {
private:
  static OZ_CFunHeader spawner;
public:
  DistinctPropagator(OZ_Term x) : Propagator_VD(x) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
  /*
private:
  OZ_NonMonotonic _nm;
public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
  virtual size_t sizeOf(void) { return sizeof(*this); }
  */
};
 
//-----------------------------------------------------------------------------

class DistinctOffsetPropagator : public Propagator_VD_VI {
private:
  static OZ_CFunHeader spawner;
public:
  DistinctOffsetPropagator(OZ_Term l, OZ_Term offset) 
    : Propagator_VD_VI(l, offset) {}
  
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};
 
