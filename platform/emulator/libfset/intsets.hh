/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __INTSETS_HH__
#define __INTSETS_HH__

#include "fsstd.hh"

class FSetsMinPropagator : public Propagator_S_D {
private:
  static OZ_CFun header;
public:
  FSetsMinPropagator(OZ_Term v, OZ_Term i)
    : Propagator_S_D(v, i) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFun getHeaderFunc(void) const {
    return header;
  }
};

#endif /* __INTSETS_HH__ */


//-----------------------------------------------------------------------------
// eof
