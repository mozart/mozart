/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#ifndef __STD_N_HH__
#define __STD_N_HH__

#include "fsstd.hh"

//*****************************************************************************

class FSetDisjointNPropagator : public Propagator_VS {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;

  OZ_FSetValue _u;

public:
  FSetDisjointNPropagator(OZ_Term vs) : Propagator_VS(vs), _u(fs_empty) {}

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { return sizeof(FSetDisjointNPropagator); }

  virtual void gCollect(void) {
    Propagator_VS::gCollect();
    _u.copyExtension();
  }

  virtual void sClone(void) {
    Propagator_VS::sClone();
    _u.copyExtension();
  }

  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

//-----------------------------------------------------------------------------
//#define EXPERIMENT

class FSetUnionNPropagator : public Propagator_VS_S {
  friend INIT_FUNC(fsp_init);

#ifdef EXPERIMENT
friend void sortVars(FSetUnionNPropagator  &p);
#endif

protected:
  static OZ_PropagatorProfile profile;

  OZ_FSetConstraint * _aux;

  void _init_aux(void) { 
    for (int i = _vs_size; i--; ) _aux[i].init(); 
    if (_vs_size) 
      _aux[0].init(fs_empty);    
  }

public:
  FSetUnionNPropagator(OZ_Term vs, OZ_Term s) : Propagator_VS_S(vs, s) 
  {
    _aux = (OZ_FSetConstraint *) (void*)
      OZ_hallocChars(_vs_size * sizeof(OZ_FSetConstraint));
    _init_aux();

#ifdef EXPERIMENT
    sortVars(*this);
#endif

  }

  virtual OZ_Return propagate(void);
  
  virtual size_t sizeOf(void) { return sizeof(FSetUnionNPropagator); }

  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }

  virtual void gCollect(void) {
    Propagator_VS_S::gCollect();

    OZ_FSetConstraint * new_aux = (OZ_FSetConstraint *) (void*)
      OZ_hallocChars(_vs_size * sizeof(OZ_FSetConstraint));
  
    for (int i = _vs_size; i--; )
      new_aux[i] = _aux[i];
    
    _aux = new_aux;
  }
  virtual void sClone(void) {
    Propagator_VS_S::sClone();

    OZ_FSetConstraint * new_aux = (OZ_FSetConstraint *) (void*)
      OZ_hallocChars(_vs_size * sizeof(OZ_FSetConstraint));
  
    for (int i = _vs_size; i--; )
      new_aux[i] = _aux[i];
    
    _aux = new_aux;
  }
};


//-----------------------------------------------------------------------------

class FSetPartitionPropagator : public FSetUnionNPropagator {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;

public:
  FSetPartitionPropagator(OZ_Term vs, OZ_Term s) 
    : FSetUnionNPropagator(vs, s) {}

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { return sizeof(FSetPartitionPropagator); }

  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
private:
  OZ_NonMonotonic _nm;
public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
};

//-----------------------------------------------------------------------------

class FSetIntersectionNPropagator : public Propagator_VS_S {
  friend INIT_FUNC(fsp_init);

protected:
  static OZ_PropagatorProfile profile;

public:
  FSetIntersectionNPropagator(OZ_Term vs, OZ_Term s)
    : Propagator_VS_S(vs, s) { }

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { return sizeof(FSetIntersectionNPropagator); }

  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }

  virtual void gCollect(void) {
    Propagator_VS_S::gCollect();
  }
  virtual void sClone(void) {
    Propagator_VS_S::sClone();
  }
};

#endif /* __STD_N_HH__ */
// end of file

