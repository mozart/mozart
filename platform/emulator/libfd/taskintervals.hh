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

class TaskIntervalsPropagator : public Propagator_VD_VI {
private:
  static OZ_CFunHeader spawner;
public:
  TaskIntervalsPropagator (OZ_Term x, OZ_Term durations) 
    : Propagator_VD_VI(x, durations) {}

  TaskIntervalsPropagator(OZ_Term tasks, OZ_Term starts, OZ_Term durs);

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
class CPIteratePropagatorCumTI : public Propagator_VD_VI_VI_I {
private:
  static OZ_CFunHeader spawner;
public:
  CPIteratePropagatorCumTI(OZ_Term x, OZ_Term durations, OZ_Term use, 
			 OZ_Term cap) 
    : Propagator_VD_VI_VI_I(x, durations, use, cap) {}

  CPIteratePropagatorCumTI(OZ_Term tasks, OZ_Term starts, OZ_Term durs,
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


#endif // __SCHED_HH__
