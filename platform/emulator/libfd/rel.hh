/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "std.hh"

//-----------------------------------------------------------------------------

class NotEqOffPropagator : public Propagator_D_D_I {
private:
  static OZ_CFun spawner;
public:
  NotEqOffPropagator(OZ_Term x, OZ_Term y,  int c)
    : Propagator_D_D_I(x, y, c) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class LessEqOffPropagator : public Propagator_D_D_I {
private:
  static OZ_CFun spawner;
public:
  LessEqOffPropagator(OZ_Term x, OZ_Term y,  int c)
    : Propagator_D_D_I(x, y, c) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class MinimumPropagator : public Propagator_D_D_D {
private:
  static OZ_CFun spawner;
public:
  MinimumPropagator(OZ_Term x, OZ_Term y,  OZ_Term z)
    : Propagator_D_D_D(x, y, z) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class MaximumPropagator : public Propagator_D_D_D {
private:
  static OZ_CFun spawner;
public:
  MaximumPropagator(OZ_Term x, OZ_Term y,  OZ_Term z)
    : Propagator_D_D_D(x, y, z) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class IntersectionPropagator : public Propagator_D_D_D {
private:
  static OZ_CFun spawner;
public:
  IntersectionPropagator(OZ_Term x, OZ_Term y,  OZ_Term z)
    : Propagator_D_D_D(x, y, z) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class UnionPropagator : public Propagator_D_D_D {
private:
  static OZ_CFun spawner;
public:
  UnionPropagator(OZ_Term x, OZ_Term y,  OZ_Term z)
    : Propagator_D_D_D(x, y, z) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class SubSetPropagator : public Propagator_D_D {
private:
  static OZ_CFun spawner;
public:
  SubSetPropagator(OZ_Term x, OZ_Term y)
    : Propagator_D_D(x, y) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class DistinctPropagator : public Propagator_VD {
private:
  static OZ_CFun spawner;
public:
  DistinctPropagator(OZ_Term x) : Propagator_VD(x) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class DistinctOffsetPropagator : public Propagator_VD_VI {
private:
  static OZ_CFun spawner;
public:
  DistinctOffsetPropagator(OZ_Term l, OZ_Term offset)
    : Propagator_VD_VI(l, offset) {}

  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Return propagate(void);
};
