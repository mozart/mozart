/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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

#ifndef __INTSETS_HH__
#define __INTSETS_HH__

#include "fsstd.hh"

class FSetsMinPropagator : public Propagator_S_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetsMinPropagator(OZ_Term v, OZ_Term i)
    : Propagator_S_D(v, i) {}
  
  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class FSetsMaxPropagator : public Propagator_S_D {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetsMaxPropagator(OZ_Term v, OZ_Term i)
    : Propagator_S_D(v, i) {}
  
  virtual OZ_Return propagate(void);
  
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};

class Propagator_S : public OZ_Propagator {
protected:
  OZ_Term _s;
public:
  Propagator_S(OZ_Term s)
    : _s(s) {}
  virtual size_t sizeOf(void) {
    return sizeof(Propagator_S);
  }
  virtual void gCollect(void) {
    OZ_gCollectTerm(_s);
  }
  virtual void sClone(void) {
    OZ_sCloneTerm(_s);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_s,OZ_nil());
  }
};

class FSetsConvexPropagator : public Propagator_S {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  FSetsConvexPropagator(OZ_Term v)
    : Propagator_S(v) {}
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
};


class FSetMatchPropagator : public Propagator_S_VD {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
  
  int _firsttime, _last_min, _last_max, _l, _k;

public:
  FSetMatchPropagator(OZ_Term s, OZ_Term vd)
    : _firsttime(1), Propagator_S_VD(s, vd) { }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }  
  virtual size_t sizeOf(void) {
    return sizeof(FSetMatchPropagator);
  }
};

class FSetMinNPropagator : public Propagator_S_VD {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
  
  int _firsttime, _last_min, _last_max, _l, _k;

public:
  FSetMinNPropagator(OZ_Term s, OZ_Term vd)
    : _firsttime(1), Propagator_S_VD(s, vd) { }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }  
  virtual size_t sizeOf(void) {
    return sizeof(FSetMinNPropagator);
  }
};

class FSetMaxNPropagator : public Propagator_S_VD {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
  
  int _firsttime, _last_min, _last_max, _l, _k;

public:
  FSetMaxNPropagator(OZ_Term s, OZ_Term vd)
    : _firsttime(1), Propagator_S_VD(s, vd) { }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }  
  virtual size_t sizeOf(void) {
    return sizeof(FSetMaxNPropagator);
  }
};

class FSetSeqPropagator : public Propagator_VS {
  friend INIT_FUNC(fsp_init);
private:
  static OZ_PropagatorProfile profile;
  
public:
  FSetSeqPropagator(OZ_Term s)
    : Propagator_VS(s) { }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }  
};

#endif /* __INTSETS_HH__ */


//-----------------------------------------------------------------------------
// eof
