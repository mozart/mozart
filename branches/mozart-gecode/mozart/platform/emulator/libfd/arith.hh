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

#ifndef __ARITH_HH__
#define __ARITH_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class TwicePropagator : public Propagator_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  TwicePropagator(OZ_Term x, OZ_Term y) : Propagator_D_D(x, y) {}
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class SquarePropagator : public Propagator_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  SquarePropagator(OZ_Term x, OZ_Term y) : Propagator_D_D(x, y) {}
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class PlusPropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  PlusPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class TimesPropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  TimesPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) { };
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class DivPropagator : public Propagator_D_I_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  DivPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_I_D(x, y, z) { };
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

class DivIPropagator : public Propagator_D_I_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  DivIPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_I_D(x, y, z) { };
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

class ModPropagator : public Propagator_D_I_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  ModPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_I_D(x, y, z) { };
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

class ModIPropagator : public Propagator_D_I_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  ModIPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_I_D(x, y, z) { };
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class PowerPropagator : public Propagator_D_D_I {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  PowerPropagator(OZ_Term x, OZ_Term y,  int c) 
    : Propagator_D_D_I(x, y, c) {}
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//=============================================================================
// domain consistent constraints

class TwiceDPropagator : public Propagator_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  TwiceDPropagator(OZ_Term x, OZ_Term y) : Propagator_D_D(x, y) {}
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class SquareDPropagator : public Propagator_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  SquareDPropagator(OZ_Term x, OZ_Term y) : Propagator_D_D(x, y) {}
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class PlusDPropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  PlusDPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class TimesDPropagator : public Propagator_D_D_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  TimesDPropagator(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_D_D_D(x, y, z) { };
  
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual OZ_Return propagate(void);
};


#endif
//-----------------------------------------------------------------------------
// eof
