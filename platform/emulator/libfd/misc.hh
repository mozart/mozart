#ifndef __MISC_HH__
#define __MISC_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class Counter : public OZ_Propagator {
private:
  int c;
  OZ_Term s;
  static OZ_CFun spawner;
public:
  Counter(OZ_Term i, OZ_Term st) : c(OZ_intToC(i)), s(st) {}

  virtual size_t sizeOf(void) { return sizeof(Counter); }
  virtual void updateHeapRefs(OZ_Boolean) { OZ_updateHeapTerm(s); }
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const { RETURN_LIST2(OZ_int(c), s); }
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};


class FirstFail : public OZ_Propagator {
private:
  int size;
  OZ_Term * reg_fds;
  OZ_Term stream;
  static OZ_CFun spawner;
public:
  //  FirstFail(OZ_Term l, OZ_Term stream) : FirstFail(l, stream) {};
  FirstFail(OZ_Term, OZ_Term);
  virtual size_t sizeOf(void) { return sizeof(FirstFail); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const { RETURN_LIST2(OZ_int(size), stream); }
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};

//-----------------------------------------------------------------------------

class SpawnLess : public OZ_Propagator {
private:
  OZ_Term a, b;
  int c;
public:
  static OZ_CFun spawner;
public:
  SpawnLess(OZ_Term i1, OZ_Term i2) : a(i1), b(i2), c(2) {}

  virtual size_t sizeOf(void) { return sizeof(SpawnLess); }
  virtual void updateHeapRefs(OZ_Boolean) { OZ_updateHeapTerm(a); OZ_updateHeapTerm(b); }
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const { RETURN_LIST2(a, b); }
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
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
  
  virtual OZ_CFun getHeaderFunc(void) const { return SpawnLess::spawner; }
  
  virtual OZ_Return propagate(void);
};




#endif // __MISC_HH__
