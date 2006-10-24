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

#include "std.hh"

class DistancePropagatorLeq : public Propagator_D_D_D_I {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  DistancePropagatorLeq(OZ_Term x, OZ_Term y,  OZ_Term z, int c) 
    : Propagator_D_D_D_I(x, y, z, c) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Term getParameters(void) const { return Propagator_D_D_D_I::getParametersC(SUM_OP_LEQ); }
};

class DistancePropagatorGeq : public Propagator_D_D_D_I {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  DistancePropagatorGeq(OZ_Term x, OZ_Term y,  OZ_Term z, int c) 
    : Propagator_D_D_D_I(x, y, z, c) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Term getParameters(void) const { return Propagator_D_D_D_I::getParametersC(SUM_OP_GEQ); }
};

class DistancePropagatorEq : public Propagator_D_D_D_I {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  DistancePropagatorEq(OZ_Term x, OZ_Term y,  OZ_Term z, int c) 
    : Propagator_D_D_D_I(x, y, z, c) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Term getParameters(void) const { return Propagator_D_D_D_I::getParametersC(SUM_OP_EQ); }
};

class DistancePropagatorNeq : public Propagator_D_D_D_I {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  DistancePropagatorNeq(OZ_Term x, OZ_Term y,  OZ_Term z, int c) 
    : Propagator_D_D_D_I(x, y, z, c) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Term getParameters(void) const { return Propagator_D_D_D_I::getParametersC(SUM_OP_NEQ); }
};
