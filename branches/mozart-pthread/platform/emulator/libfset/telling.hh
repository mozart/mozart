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

#ifndef __TELLING_HH__
#define __TELLING_HH__

#include "fsstd.hh"

class IncludePropagator : public Propagator_S_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  IncludePropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}
  
  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class ExcludePropagator : public Propagator_S_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  ExcludePropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class FSetCardPropagator : public Propagator_S_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetCardPropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

#endif /* __TELLING_HH__ */

//-----------------------------------------------------------------------------
// eof
