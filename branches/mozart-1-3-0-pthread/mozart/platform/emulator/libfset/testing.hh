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

#ifndef __TESTING_HH__
#define __TESTING_HH__

#include "fsstd.hh" 

class IsInPropagator : public Propagator_S_I_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  IsInPropagator(OZ_Term v, OZ_Term i, OZ_Term b)
    : Propagator_S_I_D(v, i, b) { }

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

#endif /* __TESTING_HH__ */

//-----------------------------------------------------------------------------
// eof
