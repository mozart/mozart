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


#endif /* __REIFIED_HH__ */

//-----------------------------------------------------------------------------
// eof
