/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 2001
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

#ifndef __BOUNDSALLDIST_HH__
#define __BOUNDSALLDIST_HH__

#include "std.hh"

class BoundsDistinctPropagator : public Propagator_VD {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
  OZ_NonMonotonic _nm;
public:
  BoundsDistinctPropagator(OZ_Term x) : Propagator_VD(x) {}
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);

  virtual OZ_Boolean isMonotonic(void) const { 
    return OZ_FALSE; 
  }

  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }

};
 
#endif
