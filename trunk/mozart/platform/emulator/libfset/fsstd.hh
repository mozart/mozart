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
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_v);
    OZ_updateHeapTerm(_b);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_v, 
		   OZ_cons(OZ_int(_i),
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
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_v);
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
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_x);
    OZ_updateHeapTerm(_y);
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
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_x);
    OZ_updateHeapTerm(_y);
    OZ_updateHeapTerm(_z);
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
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_s);
    OZ_updateHeapTerm(_d);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_s, OZ_cons(_d, OZ_nil()));
  }
};

