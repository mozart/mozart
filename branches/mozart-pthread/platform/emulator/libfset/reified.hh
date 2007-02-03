/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __REIFIED_HH__
#define __REIFIED_HH__

#include "fsstd.hh"
#include "telling.hh"
#include "indexset.hh"
#include <string.h>

class IncludeRPropagator : public Propagator_S_D_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  IncludeRPropagator(OZ_Term s, OZ_Term d, OZ_Term r)
    : Propagator_S_D_D(s, d, r) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

//-----------------------------------------------------------------------------

class EqualRPropagator : public Propagator_S_S_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  EqualRPropagator(OZ_Term x, OZ_Term y, OZ_Term r)
    : Propagator_S_S_D(x, y, r) {}

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

//-----------------------------------------------------------------------------

class IsInRPropagator : public Propagator_S_I_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  IsInRPropagator(OZ_Term v, OZ_Term i, OZ_Term b)
    : Propagator_S_I_D(v, i, b) { }

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

//-----------------------------------------------------------------------------

class BoundsPropagator : public OZ_Propagator {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;

  int _s_ub_card, _d_ub, _s_ub;
  OZ_Term _s, _d, _r;

public:
  BoundsPropagator(OZ_Term s_ub, OZ_Term s, OZ_Term d_ub, OZ_Term d, OZ_Term r)
    : _d_ub(OZ_intToC(d_ub)), _s_ub(s_ub), _s(s), _d(d), _r(r) { }

  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
  virtual size_t sizeOf(void) {
    return sizeof(BoundsPropagator);
  }
  virtual void gCollect(void) {
    OZ_gCollectBlock(&_s,&_s,3);
  }
  virtual void sClone(void) {
    OZ_sCloneBlock(&_s,&_s,3);
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
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;

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
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
  virtual size_t sizeOf(void) {
    return sizeof(BoundsNPropagator);
  }
  virtual void gCollect(void) {
    _d = OZ_gCollectAllocBlock(_size, _d);
    _s = OZ_gCollectAllocBlock(_size, _s);
    _r = OZ_gCollectAllocBlock(_size, _r);

    _d_ub = OZ_copyCInts(_size, _d_ub);

    _s_ub_t new_s_ub = { OZ_hallocCInts(_size) };
    
    for (int i = _size; i--; )
      new_s_ub.s_ub_card[i] = _s_ub.s_ub_card[i];
    
    _s_ub = new_s_ub;
  }
  virtual void sClone(void) {
    _d = OZ_sCloneAllocBlock(_size, _d);
    _s = OZ_sCloneAllocBlock(_size, _s);
    _r = OZ_sCloneAllocBlock(_size, _r);

    _d_ub = OZ_copyCInts(_size, _d_ub);

    _s_ub_t new_s_ub = { OZ_hallocCInts(_size) };
    
    for (int i = _size; i--; )
      new_s_ub.s_ub_card[i] = _s_ub.s_ub_card[i];
    
    _s_ub = new_s_ub;
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_nil();
  }
};

//-----------------------------------------------------------------------------

class PartitionReifiedPropagator : public OZ_Propagator {
  friend INIT_FUNC(fsp_init);
protected:
  static OZ_PropagatorProfile profile;

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

  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }

  virtual void gCollect(void) {
    // copy index sets
    _i_sets = _i_sets->copy();

    // copy subsets
    int vs_chars = _size * sizeof(OZ_FSetValue);

    OZ_FSetValue * new_vs = (OZ_FSetValue *) (void *) OZ_hallocChars(vs_chars);
    
    memcpy(new_vs, _vs, vs_chars);

    for (int i = _size; i --; ) {
      new_vs[i].copyExtension();
    }

    _vs = new_vs;

    // copy bools
    _vd = OZ_gCollectAllocBlock(_size, _vd);
  
  }
  virtual void sClone(void) {
    // copy index sets
    _i_sets = _i_sets->copy();

    // copy subsets
    int vs_chars = _size * sizeof(OZ_FSetValue);

    OZ_FSetValue * new_vs = (OZ_FSetValue *) (void *) OZ_hallocChars(vs_chars);
    
    memcpy(new_vs, _vs, vs_chars);

    for (int i = _size; i --; ) {
      new_vs[i].copyExtension();
    }

    _vs = new_vs;

    // copy bools
    _vd = OZ_sCloneAllocBlock(_size, _vd);
  
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_nil();
  }

};

//-----------------------------------------------------------------------------

class PartitionReified1Propagator : public PartitionReifiedPropagator {
  friend INIT_FUNC(fsp_init);
protected:
  static OZ_PropagatorProfile profile;

  OZ_Term _cost;

  int * _min_cost_per_elem;
public:
  PartitionReified1Propagator(OZ_Term vs, OZ_Term s, OZ_Term vd, OZ_Term cost);

  virtual OZ_Return propagate(void);
  
  virtual size_t sizeOf(void) { return sizeof(PartitionReified1Propagator); }

  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }

  virtual void gCollect(void) {
    PartitionReifiedPropagator::gCollect();
    
    OZ_gCollectTerm(_cost);

    _min_cost_per_elem = OZ_copyCInts(_u_max_elem+1, _min_cost_per_elem);
  }

  virtual void sClone(void) {
    PartitionReifiedPropagator::sClone();
    
    OZ_sCloneTerm(_cost);

    _min_cost_per_elem = OZ_copyCInts(_u_max_elem+1, _min_cost_per_elem);
  }

private:
  OZ_NonMonotonic _nm;
public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
};


#endif /* __REIFIED_HH__ */

//-----------------------------------------------------------------------------
// eof
