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

#ifndef __STANDARD_HH__
#define __STANDARD_HH__

#include "fsstd.hh"

//*****************************************************************************

class FSetIntersectionPropagator : public Propagator_S_S_S {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetIntersectionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_S_S_S(x, y, z) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class FSetUnionPropagator : public Propagator_S_S_S {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetUnionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_S_S_S(x, y, z) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class FSetSubsumePropagator : public Propagator_S_S {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetSubsumePropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class FSetDisjointPropagator : public Propagator_S_S {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetDisjointPropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class FSetDistinctPropagator : public Propagator_S_S {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetDistinctPropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class FSetDiffPropagator : public Propagator_S_S_S {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetDiffPropagator(OZ_Term x,OZ_Term y,OZ_Term z)
    : Propagator_S_S_S(x,y,z) {}
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

//*****************************************************************************
#endif /* __STANDARD_HH__ */
// end of file


