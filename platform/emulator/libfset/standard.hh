/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __STANDARD_HH__
#define __STANDARD_HH__

#include "fsstd.hh"

//*****************************************************************************

class FSetIntersectionPropagator : public Propagator_S_S_S {
private:
  static OZ_CFun spawner;
public:
  FSetIntersectionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_S_S_S(x, y, z) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_CFun getHeaderFunc(void) const {
    return spawner;
  }
};

class FSetUnionPropagator : public Propagator_S_S_S {
private:
  static OZ_CFun spawner;
public:
  FSetUnionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_S_S_S(x, y, z) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_CFun getHeaderFunc(void) const {
    return spawner;
  }
};

class FSetSubsumePropagator : public Propagator_S_S {
private:
  static OZ_CFun spawner;
public:
  FSetSubsumePropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_CFun getHeaderFunc(void) const {
    return spawner;
  }
};

class FSetDisjointPropagator : public Propagator_S_S {
private:
  static OZ_CFun spawner;
public:
  FSetDisjointPropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_CFun getHeaderFunc(void) const {
    return spawner;
  }
};

class FSetDiffPropagator : public Propagator_S_S_S {
private:
  static OZ_CFun header;
public:
  FSetDiffPropagator(OZ_Term x,OZ_Term y,OZ_Term z)
    : Propagator_S_S_S(x,y,z) {}
  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const {
    return header;
  }
};

#endif /* __STANDARD_HH__ */

// end of file
//-----------------------------------------------------------------------------
