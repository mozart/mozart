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

#ifndef __FSSTD_HH__
#define __FSSTD_HH__

#include "fsaux.hh"

//-----------------------------------------------------------------------------

class Propagator_S_I_D : public OZ_Propagator {
protected:
  OZ_Term _v, _b;
  int _i;
public:
  Propagator_S_I_D(OZ_Term v, OZ_Term i, OZ_Term b)
    : _v(v), _i(OZ_intToC(i)), _b(b) { }
  
  virtual size_t sizeOf(void) {
    return sizeof(Propagator_S_I_D);
  }
  virtual void gCollect(void) {
    OZ_gCollectBlock(&_v, &_v, 2);
  }
  virtual void sClone(void) {
    OZ_sCloneBlock(&_v, &_v, 2);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_v, 
		   OZ_cons(OZ_int(_i),
			   OZ_cons(_b, OZ_nil())));
  }
};

//-----------------------------------------------------------------------------

class Propagator_S_S_D : public OZ_Propagator {
protected:
  OZ_Term _v1, _v2, _b;

public:
  Propagator_S_S_D(OZ_Term v1, OZ_Term v2, OZ_Term b)
    : _v1(v1), _v2(v2), _b(b) { }
  
  virtual size_t sizeOf(void) {
    return sizeof(Propagator_S_S_D);
  }
  virtual void gCollect(void) {
    OZ_gCollectBlock(&_v1, &_v1, 3);
  }
  virtual void sClone(void) {
    OZ_sCloneBlock(&_v1, &_v1, 3);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_v1, 
		   OZ_cons(_v2,
			   OZ_cons(_b, OZ_nil())));
  }
};

//-----------------------------------------------------------------------------

class Propagator_S_I : public OZ_Propagator {
protected:
  OZ_Term _v;
  int _i;
public:
  Propagator_S_I(OZ_Term v, OZ_Term i)
    : _v(v), _i(OZ_intToC(i)) {}

  virtual size_t sizeOf(void) {
    return sizeof(Propagator_S_I);
  }
  virtual void gCollect(void) {
    OZ_gCollectTerm(_v);
  }
  virtual void sClone(void) {
    OZ_sCloneTerm(_v);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_v, 
		   OZ_cons(OZ_int(_i),OZ_nil()));
  }
};

//-----------------------------------------------------------------------------

class Propagator_S_S : public OZ_Propagator {
protected:
  OZ_Term _x, _y;
public:
  Propagator_S_S(OZ_Term x, OZ_Term y)
    : _x(x), _y(y) {}
  
  virtual size_t sizeOf(void) {
    return sizeof(Propagator_S_S);
  }
  virtual void gCollect(void) {
    OZ_gCollectBlock(&_x,&_x,2);
  }
  virtual void sClone(void) {
    OZ_sCloneBlock(&_x,&_x,2);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_x, OZ_cons(_y, OZ_nil()));
  }
};

//-----------------------------------------------------------------------------

class Propagator_S_S_S : public OZ_Propagator {
protected:
  OZ_Term _x, _y, _z;
public:
  Propagator_S_S_S(OZ_Term x, OZ_Term y, OZ_Term z)
    : _x(x), _y(y), _z(z) {}
  
  virtual size_t sizeOf(void) {
    return sizeof(Propagator_S_S_S);
  }
  virtual void gCollect(void) {
    OZ_gCollectBlock(&_x, &_x, 3);
  }
  virtual void sClone(void) {
    OZ_sCloneBlock(&_x, &_x, 3);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_x, 
		   OZ_cons(_y,
			   OZ_cons(_z, OZ_nil())));
  }
};

//-----------------------------------------------------------------------------

class Propagator_S_D : public OZ_Propagator {
protected:
  OZ_Term _s, _d;
public:
  Propagator_S_D(OZ_Term s, OZ_Term d)
    : _s(s), _d(d) {}
  
  virtual size_t sizeOf(void) {
    return sizeof(Propagator_S_D);
  }
  virtual void gCollect(void) {
    OZ_gCollectBlock(&_s,&_s,2);
  }
  virtual void sClone(void) {
    OZ_sCloneBlock(&_s,&_s,2);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_s, OZ_cons(_d, OZ_nil()));
  }
};

//-----------------------------------------------------------------------------

class Propagator_S_D_D : public Propagator_S_D {
protected:
  OZ_Term _r;
public:
  Propagator_S_D_D(OZ_Term s, OZ_Term d, OZ_Term r) 
    : _r(r), Propagator_S_D(s, d) {}
  
  virtual size_t sizeOf(void) {
    return sizeof(Propagator_S_D_D);
  }
  virtual void gCollect(void) {
    OZ_gCollectTerm(_r);
    Propagator_S_D::gCollect();
  }
  virtual void sClone(void) {
    OZ_sCloneTerm(_r);
    Propagator_S_D::sClone();
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_s, OZ_cons(_d, OZ_cons(_r, OZ_nil())));
  }
};

//-----------------------------------------------------------------------------

class Propagator_S_VD : public OZ_Propagator {
protected:
  OZ_Term _s, * _vd;
  int _vd_size;
public:
  Propagator_S_VD(OZ_Term s, OZ_Term vd);
  ~Propagator_S_VD(void);
  
  virtual size_t sizeOf(void) { return sizeof(Propagator_S_VD); }
  virtual void gCollect(void);
  virtual void sClone(void);
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_VS : public OZ_Propagator {
protected:
  OZ_Term * _vs;
  int _vs_size;
public:
  Propagator_VS(OZ_Term vs);
  ~Propagator_VS(void);
  
  virtual size_t sizeOf(void) { return sizeof(Propagator_VS); }
  virtual void gCollect(void);
  virtual void sClone(void);
  virtual OZ_Term getParameters(void) const;

  OZ_Boolean hasEqualVars(void);
};

inline
OZ_Boolean Propagator_VS::hasEqualVars(void) 
{
  if (mayBeEqualVars()) {
    int * is = OZ_findEqualVars(_vs_size, _vs);
    
    for (int i = _vs_size; i--; )
      if (is[i] != -1 && is[i] != i) 
	return OZ_TRUE;
  }    
  return OZ_FALSE;
}


//-----------------------------------------------------------------------------
class Propagator_VS_S : public Propagator_VS {
protected:
  OZ_Term  _s;

public:
  Propagator_VS_S(OZ_Term vs, OZ_Term s);
  
  virtual size_t sizeOf(void) { return sizeof(Propagator_VS_S); }
  virtual void gCollect(void);
  virtual void sClone(void);
  virtual OZ_Term getParameters(void) const;
};


#endif /* __FSSTD_HH__ */

//-----------------------------------------------------------------------------
// eof
