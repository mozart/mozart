/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __REIFIED_HH__
#define __REIFIED_HH__

#include "fsstd.hh"
#include "telling.hh"
#include "indexset.hh"

class IncludeRPropagator : public Propagator_S_D_D {
private:
  static OZ_CFunHeader header;
public:
  IncludeRPropagator(OZ_Term s, OZ_Term d, OZ_Term r)
    : Propagator_S_D_D(s, d, r) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
};

//-----------------------------------------------------------------------------

class IsInRPropagator : public Propagator_S_I_D {
private:
  static OZ_CFunHeader spawner;
public:
  IsInRPropagator(OZ_Term v, OZ_Term i, OZ_Term b)
    : Propagator_S_I_D(v, i, b) { }

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &spawner;
  }
};

//-----------------------------------------------------------------------------

class BoundsPropagator : public OZ_Propagator {
private:
  static OZ_CFunHeader header;

  int _s_ub_card, _d_ub, _s_ub;
  OZ_Term _s, _d, _r;

public:
  BoundsPropagator(OZ_Term s_ub, OZ_Term s, OZ_Term d_ub, OZ_Term d, OZ_Term r)
    : _d_ub(OZ_intToC(d_ub)), _s_ub(s_ub), _s(s), _d(d), _r(r) { }

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
  virtual size_t sizeOf(void) {
    return sizeof(BoundsPropagator);
  }
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_s);
    OZ_updateHeapTerm(_d);
    OZ_updateHeapTerm(_r);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_s,
                   OZ_cons(_d,
                           OZ_cons(_r,
                                   OZ_nil())));
  }
};

//-----------------------------------------------------------------------------

class BoundsNPropagator : public OZ_Propagator {
private:
  static OZ_CFunHeader header;

  int _size, * _d_ub;
  OZ_Term * _s, * _d, * _r;
  union _s_ub_t {int * s_ub_card; OZ_Term * s_ub;} _s_ub;
  int first;
public:
  BoundsNPropagator(OZ_Term s_ub, OZ_Term s, OZ_Term d_ub,
                    OZ_Term d, OZ_Term r) : first(1)
  {
    _size = OZ_vectorSize(d);

    _d = OZ_hallocOzTerms(_size);
    OZ_getOzTermVector(d, _d);

    _s = OZ_hallocOzTerms(_size);
    OZ_getOzTermVector(s, _s);

    _r = OZ_hallocOzTerms(_size);
    OZ_getOzTermVector(r, _r);

    _d_ub = OZ_hallocCInts(_size);
    OZ_getCIntVector(d_ub, _d_ub);

    _s_ub.s_ub = OZ_hallocOzTerms(_size);
    OZ_getOzTermVector(s_ub, _s_ub.s_ub);
  }


  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
  virtual size_t sizeOf(void) {
    return sizeof(BoundsNPropagator);
  }
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_Term * new_d  = OZ_hallocOzTerms(_size);
    OZ_Term * new_s  = OZ_hallocOzTerms(_size);
    OZ_Term * new_r  = OZ_hallocOzTerms(_size);
    int * new_d_ub   = OZ_hallocCInts(_size);
    _s_ub_t new_s_ub = { OZ_hallocCInts(_size) };

    for (int i = _size; i--; ) {
      new_d[i] = _d[i];
      OZ_updateHeapTerm(new_d[i]);

      new_s[i] = _s[i];
      OZ_updateHeapTerm(new_s[i]);

      new_r[i] = _r[i];
      OZ_updateHeapTerm(new_r[i]);

      new_d_ub[i] = _d_ub[i];
      new_s_ub.s_ub_card[i] = _s_ub.s_ub_card[i];
    }
    _d    = new_d;
    _s    = new_s;
    _r    = new_r;
    _d_ub = new_d_ub;
    _s_ub = new_s_ub;
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_nil();
  }
};

//-----------------------------------------------------------------------------

class PartitionReifiedPropagator : public OZ_Propagator {
protected:
  static OZ_CFunHeader header;

  IndexSets * _i_sets;

  OZ_FSetValue * _vs;
  int _size;

  int _u_max_elem;

  OZ_Term * _vd;
  int _first;
public:
  PartitionReifiedPropagator(OZ_Term vs, OZ_Term s, OZ_Term vd);

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { return sizeof(PartitionReifiedPropagator); }

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }

  virtual void updateHeapRefs(OZ_Boolean) {
    // copy index sets
    _i_sets = _i_sets->copy();

    // copy subsets
    int vs_chars = _size * sizeof(OZ_FSetValue);

    OZ_FSetValue * new_vs = (OZ_FSetValue *) (void *) OZ_hallocChars(vs_chars);

    memcpy(new_vs, _vs, vs_chars);

    _vs = new_vs;

    // copy bools
    OZ_Term * new_vd = OZ_hallocOzTerms(_size);

    for (int i = _size; i--; ) {
      new_vd[i] = _vd[i];
      OZ_updateHeapTerm(new_vd[i]);
    }
    _vd = new_vd;

  }
  virtual OZ_Term getParameters(void) const {
    return OZ_nil();
  }

};

//-----------------------------------------------------------------------------

class PartitionReified1Propagator : public PartitionReifiedPropagator {
protected:
  static OZ_CFunHeader header;

  OZ_Term _cost;

  int * _min_cost_per_elem;
public:
  PartitionReified1Propagator(OZ_Term vs, OZ_Term s, OZ_Term vd, OZ_Term cost);

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { return sizeof(PartitionReified1Propagator); }

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }

  virtual void updateHeapRefs(OZ_Boolean gc) {
    PartitionReifiedPropagator::updateHeapRefs(gc);

    OZ_updateHeapTerm(_cost);

    int * tmp_min_cost_per_elem = OZ_hallocCInts(_u_max_elem+1);
    for (int i = _u_max_elem+1; i--; )
      tmp_min_cost_per_elem[i] = _min_cost_per_elem[i];
    _min_cost_per_elem = tmp_min_cost_per_elem;
  }

};


#endif /* __REIFIED_HH__ */

//-----------------------------------------------------------------------------
// eof
