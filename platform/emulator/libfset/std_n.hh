/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __STD_N_HH__
#define __STD_N_HH__

#include "fsstd.hh"

//*****************************************************************************

class FSetDisjointNPropagator : public Propagator_VS {
private:
  static OZ_CFunHeader header;

  OZ_FSetValue _u;

public:
  FSetDisjointNPropagator(OZ_Term vs) : Propagator_VS(vs), _u(fs_empty) {}

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { return sizeof(FSetDisjointNPropagator); }

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
};

//-----------------------------------------------------------------------------

class FSetUnionNPropagator : public Propagator_VS_S {
protected:
  static OZ_CFunHeader header;

  OZ_FSetConstraint * _aux, _u;
  int _aux_size;

  void _init_aux(void) { for (int i = _aux_size; i--; ) _aux[i].init(); }

public:
  FSetUnionNPropagator(OZ_Term vs, OZ_Term s) : Propagator_VS_S(vs, s)
  {
    _aux_size = _vs_size - 1;
    _aux = (OZ_FSetConstraint *)
      OZ_hallocChars(_aux_size * sizeof(OZ_FSetConstraint));
    _init_aux();
    _u.init(fs_empty);
  }

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { return sizeof(FSetUnionNPropagator); }

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }

  virtual void updateHeapRefs(OZ_Boolean dup) {
    Propagator_VS_S::updateHeapRefs(dup);

    OZ_FSetConstraint * new_aux = (OZ_FSetConstraint *)
      OZ_hallocChars(_aux_size * sizeof(OZ_FSetConstraint));

    for (int i = _aux_size; i--; )
      new_aux[i] = _aux[i];

    _aux = new_aux;
  }
};

//-----------------------------------------------------------------------------

class FSetPartitionPropagator : public FSetUnionNPropagator {
private:
  static OZ_CFunHeader header;

public:
  FSetPartitionPropagator(OZ_Term vs, OZ_Term s)
    : FSetUnionNPropagator(vs, s) {}

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { return sizeof(FSetPartitionPropagator); }

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
};

//*****************************************************************************
#endif /* __STD_N_HH__ */
// end of file
