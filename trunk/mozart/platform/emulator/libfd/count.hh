/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __COUNT_HH__
#define __COUNT_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class ExactlyPropagator : public Propagator_D_VD_I {
private:
  static OZ_CFun spawner;
public:
  ExactlyPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VD_I(n, l, v) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};

//-----------------------------------------------------------------------------

class AtLeastPropagator : public Propagator_D_VD_I {
private:
  static OZ_CFun spawner;
public:
  AtLeastPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VD_I(n, l, v) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};

//-----------------------------------------------------------------------------

class AtMostPropagator : public Propagator_D_VD_I {
private:
  static OZ_CFun spawner;
public:
  AtMostPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VD_I(n, l, v) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};

//-----------------------------------------------------------------------------

class ElementPropagator : public Propagator_D_VI_D {
private:
  static OZ_CFun spawner;
public:
  ElementPropagator(OZ_Term n, OZ_Term l, OZ_Term v)
    : Propagator_D_VI_D(n, l, v) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};

#endif // __COUNT_HH__
