/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __ARITH_HH__
#define __ARITH_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class TwicePropagator : public Propagator_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  TwicePropagator(OZ_Term x, OZ_Term y) : Propagator_D_D(x, y) {}

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class SquarePropagator : public Propagator_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  SquarePropagator(OZ_Term x, OZ_Term y) : Propagator_D_D(x, y) {}

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class PlusPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  PlusPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_D_D(x, y, z) {}

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class MinusPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  MinusPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class TimesPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  TimesPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class DivPropagator : public Propagator_D_I_D {
private:
  static OZ_CFunHeader spawner;
public:
  DivPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_I_D(x, y, z) { };

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

class DivIPropagator : public Propagator_D_I_D {
private:
  static OZ_CFunHeader spawner;
public:
  DivIPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_I_D(x, y, z) { };

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

class ModPropagator : public Propagator_D_I_D {
private:
  static OZ_CFunHeader spawner;
public:
  ModPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_I_D(x, y, z) { };

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

class ModIPropagator : public Propagator_D_I_D {
private:
  static OZ_CFunHeader spawner;
public:
  ModIPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_I_D(x, y, z) { };

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class PowerPropagator : public Propagator_D_D_I {
private:
  static OZ_CFunHeader spawner;
public:
  PowerPropagator(OZ_Term x, OZ_Term y,  int c)
    : Propagator_D_D_I(x, y, c) {}

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Return propagate(void);
};

#endif
//-----------------------------------------------------------------------------
// eof
