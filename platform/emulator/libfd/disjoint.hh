/*
 *  Authors:
 *    Jörg Würtz (wuertz@dfki.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Jörg Würtz, 1997
 *    Christian Schulte, 1999
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

#ifndef __DISJOINT_HH__
#define __DISJOINT_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class SchedCDPropagator : public Propagator_D_I_D_I {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  SchedCDPropagator(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd)
    : Propagator_D_I_D_I(x, xd, y, yd) {}

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

class SchedCDBPropagator : public Propagator_D_I_D_I_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  SchedCDBPropagator(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd, OZ_Term b)
    : Propagator_D_I_D_I_D(x, xd, y, yd, b) {}

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------

#endif // __DISJOINT_HH__
