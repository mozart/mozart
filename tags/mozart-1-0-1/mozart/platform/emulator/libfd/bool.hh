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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __BOOL_HH__
#define __BOOL_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class ConjunctionPropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  ConjunctionPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------

class DisjunctionPropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  DisjunctionPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) { };
  
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------

class XDisjunctionPropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  XDisjunctionPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------

class ImplicationPropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  ImplicationPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------

class EquivalencePropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  EquivalencePropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------

class NegationPropagator : public Propagator_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  NegationPropagator(OZ_Term x, OZ_Term y) 
    : Propagator_D_D(x, y) { };

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};


#endif
