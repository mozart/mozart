/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __TELLING_HH__
#define __TELLING_HH__

#include "fsstd.hh"

class TellIsInPropagator : public Propagator_S_D {
private:
  static OZ_CFun spawner;
public:
  TellIsInPropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}
  
  virtual OZ_Return propagate(void);
  
  virtual OZ_CFun getHeaderFunc(void) const {
    return spawner;
  }
};

class TellIsNotInPropagator : public Propagator_S_D {
private:
  static OZ_CFun spawner;
public:
  TellIsNotInPropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_CFun getHeaderFunc(void) const {
    return spawner;
  }
};

class FSetCardPropagator : public Propagator_S_D {
private:
  static OZ_CFun spawner;
public:
  FSetCardPropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_CFun getHeaderFunc(void) const {
    return spawner;
  }
};

#endif /* __TELLING_HH__ */

//-----------------------------------------------------------------------------
// eof
