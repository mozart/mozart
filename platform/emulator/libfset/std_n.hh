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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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
#define EXPERIMENT

class FSetUnionNPropagator : public Propagator_VS_S {
#ifdef EXPERIMENT
friend void sortVars(FSetUnionNPropagator  &p);
#endif
protected:
  static OZ_CFunHeader header;

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

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }

  virtual void updateHeapRefs(OZ_Boolean dup) {
    Propagator_VS_S::updateHeapRefs(dup);

    OZ_FSetConstraint * new_aux = (OZ_FSetConstraint *) (void*)
      OZ_hallocChars(_vs_size * sizeof(OZ_FSetConstraint));
  
    for (int i = _vs_size; i--; )
      new_aux[i] = _aux[i];
    
    _aux = new_aux;
  }
  };


#ifdef EXPERIMENT
#include <stdlib.h>
int sortVarsOrder(const void * _a, const void  * _b) {
  OZ_FSetVar a, b;
  a.ask(* (const OZ_Term *) _a);
  b.ask(* (const OZ_Term *) _b);
  return a->getKnownNotIn() -  b->getKnownNotIn();
}

void sortVars(FSetUnionNPropagator  &p)
{
  qsort(p._vs, p._vs_size, sizeof(OZ_Term), sortVarsOrder);
}
#endif    
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
private:
  OZ_NonMonotonic _nm;
public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
};

//*****************************************************************************
#endif /* __STD_N_HH__ */
// end of file

