/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __BOOL_HH__
#define __BOOL_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class ConjunctionPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  ConjunctionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class DisjunctionPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  DisjunctionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class XDisjunctionPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  XDisjunctionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class ImplicationPropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  ImplicationPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class EquivalencePropagator : public Propagator_D_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  EquivalencePropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_D_D_D(x, y, z) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class NegationPropagator : public Propagator_D_D {
private:
  static OZ_CFunHeader spawner;
public:
  NegationPropagator(OZ_Term x, OZ_Term y)
    : Propagator_D_D(x, y) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};


#endif
