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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __PROPAGATORS_HH__
#define __PROPAGATORS_HH__

#include "ri.hh"

//-----------------------------------------------------------------------------

class RILessEq : public Propagator_RI_RI {

  friend INIT_FUNC_RI;

private:

  static OZ_PropagatorProfile profile;

public:
  RILessEq(OZ_Term x, OZ_Term y) : Propagator_RI_RI(x, y) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }

  virtual OZ_Return propagate(void);

};

//-----------------------------------------------------------------------------

class RIGreater : public Propagator_RI_RI {

  friend INIT_FUNC_RI;

private:

  static OZ_PropagatorProfile profile;

public:
  RIGreater(OZ_Term x, OZ_Term y) : Propagator_RI_RI(x, y) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }

  virtual OZ_Return propagate(void);

};

//-----------------------------------------------------------------------------

class RIPlus : public Propagator_RI_RI_RI {

  friend INIT_FUNC_RI;

private:

  static OZ_PropagatorProfile profile;

public:
  RIPlus(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_RI_RI_RI(x, y, z) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }

  virtual OZ_Return propagate(void);

};

//-----------------------------------------------------------------------------

class RITimes : public Propagator_RI_RI_RI {

  friend INIT_FUNC_RI;

private:

  static OZ_PropagatorProfile profile;

public:
  RITimes(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_RI_RI_RI(x, y, z) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }

  virtual OZ_Return propagate(void);

};

//-----------------------------------------------------------------------------

class RIIntBounds : public Propagator_RI_D {

  friend INIT_FUNC_RI;

private:

  static OZ_PropagatorProfile profile;

public:
  RIIntBounds(OZ_Term ri, OZ_Term d) : Propagator_RI_D(ri, d) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }

  virtual OZ_Return propagate(void);

};

//-----------------------------------------------------------------------------

class RIIntBoundsSPP : public Propagator_RI_D {

  friend INIT_FUNC_RI;

private:

  static OZ_PropagatorProfile profile;

public:
  RIIntBoundsSPP(OZ_Term ri, OZ_Term d) : Propagator_RI_D(ri, d) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }

  virtual OZ_Return propagate(void);

};

#endif /* __PROPAGATORS_HH__ */
