/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "fsstd.hh"


class IsInPropagator : public Propagator_S_I_D {
private:
  static OZ_CFun spawner;
public:
  IsInPropagator(OZ_Term v, OZ_Term i, OZ_Term b)
    : Propagator_S_I_D(v, i, b) { }

  virtual OZ_Return propagate(void);

  virtual OZ_CFun getHeaderFunc(void) const {
    return spawner;
  }
};
