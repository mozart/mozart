/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
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

#ifndef __SCHED_HH__
#define __SCHED_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class SchedCardPropagator : public Propagator_D_I_D_I {
private:
  static OZ_CFunHeader spawner;
public:
  SchedCardPropagator(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd)
    : Propagator_D_I_D_I(x, xd, y, yd) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};


//-----------------------------------------------------------------------------

class CPIteratePropagator : public Propagator_VD_VI {
private:
  static OZ_CFunHeader spawner;
public:
  CPIteratePropagator(OZ_Term x, OZ_Term durations)
    : Propagator_VD_VI(x, durations) {}

  CPIteratePropagator(OZ_Term tasks, OZ_Term starts, OZ_Term durs);

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
private:
  OZ_NonMonotonic _nm;
public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
  virtual size_t sizeOf(void) { return sizeof(*this); }
};

//-----------------------------------------------------------------------------
class CPIteratePropagatorCap : public Propagator_VD_VI_VI_I {
private:
  // signals whether edge finding is used (1) or not (0)
  int reg_flag;
  static OZ_CFunHeader spawner;
public:
  CPIteratePropagatorCap(OZ_Term x, OZ_Term durations, OZ_Term use,
                         OZ_Term cap)
    : Propagator_VD_VI_VI_I(x, durations, use, cap) {}

  CPIteratePropagatorCap(OZ_Term tasks, OZ_Term starts, OZ_Term durs,
                         OZ_Term use, OZ_Term cap, int flag);

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
private:
  OZ_NonMonotonic _nm;
public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
  virtual size_t sizeOf(void) { return sizeof(*this); }
};

//-----------------------------------------------------------------------------

class CPIteratePropagatorCapUp : public Propagator_VD_VI_VI_I {
private:
  static OZ_CFunHeader spawner;
public:
  CPIteratePropagatorCapUp(OZ_Term x, OZ_Term durations, OZ_Term use,
                           OZ_Term cap)
    : Propagator_VD_VI_VI_I(x, durations, use, cap) {}

  CPIteratePropagatorCapUp(OZ_Term tasks, OZ_Term starts, OZ_Term durs,
                           OZ_Term use, OZ_Term cap);

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
private:
  OZ_NonMonotonic _nm;
public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
  virtual size_t sizeOf(void) { return sizeof(*this); }
};


//-----------------------------------------------------------------------------

class DisjunctivePropagator : public Propagator_VD_VI {
private:
  static OZ_CFunHeader spawner;
public:
  DisjunctivePropagator(OZ_Term x, OZ_Term durations)
    : Propagator_VD_VI(x, durations) {}

  DisjunctivePropagator(OZ_Term tasks, OZ_Term starts, OZ_Term durs);

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
private:
  OZ_NonMonotonic _nm;
public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
  virtual size_t sizeOf(void) { return sizeof(*this); }
};

//-----------------------------------------------------------------------------

#endif // __SCHED_HH__
