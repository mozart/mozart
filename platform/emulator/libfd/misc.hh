#ifndef __MISC_HH__
#define __MISC_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class Counter : public OZ_Propagator {
private:
  int c;
  OZ_Term s;
  static OZ_CFunHeader spawner;
public:
  Counter(OZ_Term i, OZ_Term st) : c(OZ_intToC(i)), s(st) {}

  virtual size_t sizeOf(void) { return sizeof(Counter); }
  virtual void updateHeapRefs(OZ_Boolean) { OZ_updateHeapTerm(s); }
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { RETURN_LIST2(OZ_int(c), s); }
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};


class FirstFail : public OZ_Propagator {
private:
  int size;
  OZ_Term * reg_fds;
  OZ_Term stream;
  static OZ_CFunHeader spawner;
public:
  //  FirstFail(OZ_Term l, OZ_Term stream) : FirstFail(l, stream) {};
  FirstFail(OZ_Term, OZ_Term);
  virtual size_t sizeOf(void) { return sizeof(FirstFail); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { RETURN_LIST2(OZ_int(size), stream); }
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class SpawnLess : public OZ_Propagator {
private:
  OZ_Term a, b;
  int c;
public:
  static OZ_CFunHeader spawner;
public:
  SpawnLess(OZ_Term i1, OZ_Term i2) : a(i1), b(i2), c(2) {}

  virtual size_t sizeOf(void) { return sizeof(SpawnLess); }
  virtual void updateHeapRefs(OZ_Boolean) { OZ_updateHeapTerm(a); OZ_updateHeapTerm(b); }
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { RETURN_LIST2(a, b); }
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class Less : public OZ_Propagator {
private:
OZ_Term _x, _y;
public:
  Less(OZ_Term x, OZ_Term y) : _x(x), _y(y) {}

  void updateHeapRefs(OZ_Boolean) {OZ_updateHeapTerm(_x);OZ_updateHeapTerm(_y);}

  virtual size_t sizeOf(void) { return sizeof(Less); }

  OZ_Term getParameters(void) const { RETURN_LIST2(_x, _y);}

  virtual OZ_CFunHeader * getHeader(void) const { return &SpawnLess::spawner; }

  virtual OZ_Return propagate(void);
};

//-----------------------------------------------------------------------------

class TestGenSum : public Propagator_VD {
private:
  static OZ_CFunHeader header;
  OZ_Term _v;

  OZ_FiniteDomain * _aux;

  void _init_aux(void) {
    for (int i = reg_l_sz; i--; )
      _aux[i].initFull();
    if(reg_l_sz)
      _aux[0] &= 0;
  }
public:
  TestGenSum(OZ_Term vd, OZ_Term v) : Propagator_VD(vd), _v(v)
  {
    _aux = (OZ_FiniteDomain *)
      OZ_hallocChars(reg_l_sz * sizeof(OZ_FiniteDomain));
    _init_aux();
  }

  virtual size_t sizeOf(void) { return sizeof(TestGenSum); }
  virtual OZ_CFunHeader * getHeader(void) const { return &header; }

  virtual OZ_Return propagate(void);

  virtual void updateHeapRefs(OZ_Boolean dup) {
    Propagator_VD::updateHeapRefs(dup);

    OZ_FiniteDomain * new_aux = (OZ_FiniteDomain *)
      OZ_hallocChars(reg_l_sz * sizeof(OZ_FiniteDomain));

    OZ_updateHeapTerm(_v);

    for (int i = reg_l_sz; i--; )
      new_aux[i] = _aux[i];

    _aux = new_aux;
  }
  virtual OZ_Term getParameters(void) const {
    TERMVECTOR2LIST(reg_l, reg_l_sz, vd);
    RETURN_LIST2(vd, _v);
  }
};


#endif // __MISC_HH__
