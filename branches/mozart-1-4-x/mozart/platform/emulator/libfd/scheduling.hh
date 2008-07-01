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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __SCHEDULING_HH__
#define __SCHEDULING_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class SchedCardPropagator : public Propagator_D_I_D_I {
  friend INIT_FUNC(sched_init);
private:
  static OZ_PropagatorProfile profile;
public:
  SchedCardPropagator(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd)
    : Propagator_D_I_D_I(x, xd, y, yd) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};


//-----------------------------------------------------------------------------

class CPIteratePropagator : public Propagator_VD_VI {
  friend INIT_FUNC(sched_init);
private:
  static OZ_PropagatorProfile profile;
public:
  CPIteratePropagator(OZ_Term x, OZ_Term durations) 
    : Propagator_VD_VI(x, durations) {}

  CPIteratePropagator(OZ_Term tasks, OZ_Term starts, OZ_Term durs);
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
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
  friend INIT_FUNC(sched_init);
private:
  // signals whether edge finding is used (1) or not (0)
  int reg_flag;
  static OZ_PropagatorProfile profile;
public:
  CPIteratePropagatorCap(OZ_Term x, OZ_Term durations, OZ_Term use, 
			 OZ_Term cap) 
    : Propagator_VD_VI_VI_I(x, durations, use, cap) {}

  CPIteratePropagatorCap(OZ_Term tasks, OZ_Term starts, OZ_Term durs,
			 OZ_Term use, OZ_Term cap, int flag);
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
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
  friend INIT_FUNC(sched_init);
private:
  static OZ_PropagatorProfile profile;
public:
  CPIteratePropagatorCapUp(OZ_Term x, OZ_Term durations, OZ_Term use, 
			   OZ_Term cap) 
    : Propagator_VD_VI_VI_I(x, durations, use, cap) {}

  CPIteratePropagatorCapUp(OZ_Term tasks, OZ_Term starts, OZ_Term durs,
			   OZ_Term use, OZ_Term cap);
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
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
  friend INIT_FUNC(sched_init);
private:
  static OZ_PropagatorProfile profile;
public:
  DisjunctivePropagator(OZ_Term x, OZ_Term durations) 
    : Propagator_VD_VI(x, durations) {}
  
  DisjunctivePropagator(OZ_Term tasks, OZ_Term starts, OZ_Term durs);
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
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

#endif // __SCHEDULING_HH__

