/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __FDOMN__H__
#define __FDOMN__H__


#ifdef __GNUC__
#pragma interface
#endif

#include "types.hh"
#include "term.hh"
#include "bignum.hh"

enum FDPropState {fd_det = 0, fd_bounds, fd_any};
enum FDState {fd_empty, fd_full, fd_discrete, fd_singleton};


#define FD_NOI 4 // number of intervals
#define MAXFDBIARGS 1000 // maximum number of arguments of fd built-ins

const int fd_iv_max_high = FD_NOI;
const int fd_iv_max_elem = OzMaxInt;
const int fd_full_size = fd_iv_max_elem + 1;

// Invariants: high == 1 reduce to FiniteDomain
class FDIntervals {
friend class FiniteDomain;
private:
  int high;
  struct i_arr_type {int left; int right;};

#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  Bool isConsistent(void) const;
  struct _i_arr_type {
    i_arr_type _i_arr[fd_iv_max_high];
    i_arr_type &operator [] (int i) /*const*/ {
      Assert(0 <= i && i < *(((int *)this) - 1));
      return _i_arr[i];
    }
    i_arr_type operator [] (int i) const {
      Assert(0 <= i && i < *(((int *)this) - 1));
      return _i_arr[i];
    }
  } i_arr;
#else
  i_arr_type i_arr[fd_iv_max_high];
#endif

  int findPossibleIndexOf(int) const;
public:
  FDIntervals(int hi) {high = hi;}
  FDIntervals(const FDIntervals &);

  const FDIntervals &operator = (const FDIntervals &);

  size_t memory_required(int hi) { // used for profiling
    return sizeof(int) + 2 * hi * sizeof(int);
  }
  void * operator new (size_t s) {return freeListMalloc(s);}
  void * operator new (size_t s, int hi) {
    return heapMalloc(s + 2 * (hi - fd_iv_max_high) * sizeof(int));
  }
  void operator delete(void *, size_t) {
    error("Unexpected call of FDIntervals::delete.");
  }
  void dispose(void) {
    if (high <= fd_iv_max_high) freeListDispose(this, sizeof(FDIntervals));
  }
  int getHigh(void) { return high; }

  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;
  void printDebug(void) const;
  void printDebugLong(void) const;

  Bool contains(int i) const;
  int findSize(void);
  int findMinElem(void);
  int findMaxElem(void);
  void initList(int list_len, int * list_left, int * list_right);
  void init(int l, int r) {i_arr[0].left = l; i_arr[0].right = r;}
  int nextBiggerElem(int v, int upper) const;
  Bool next(int i, int &n) const;
  TaggedRef getAsList(void) const;
  FDIntervals * copy(void);
  int operator <= (const int);
  int operator >= (const int);
  FDIntervals * operator -= (const int);
  FDIntervals * operator += (const int);
  void init(int, int, int, int);
  FDIntervals * complement(FDIntervals *);
  FDIntervals * complement(int, int * , int *);
  void copy(FDIntervals *);
  int union_iv(const FDIntervals &, const FDIntervals &);
  int intersect_iv(FDIntervals &, const FDIntervals &);
  int subtract_iv (FDIntervals &, const FDIntervals &);
};

const int fd_bv_max_high = 2 * FD_NOI + 1;
const int fd_bv_max_elem = 32 * fd_bv_max_high - 1;
const int fd_bv_conv_max_high = (fd_bv_max_elem) / 2 + 2;

extern int fd_bv_left_conv[fd_bv_conv_max_high];
extern int fd_bv_right_conv[fd_bv_conv_max_high];

// Invariants: size < max_elem - min_elem + 1 otherwise reduce to FiniteDomain
class FDBitVector {
private:
#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  struct {
    int _b_arr[fd_bv_max_high];
    int &operator [] (int i) /*const*/ {
      Assert(0 <= i && i < fd_bv_max_high);
      return _b_arr[i];
    }
    int operator [] (int i) const {
      Assert(0 <= i && i < fd_bv_max_high);
      return _b_arr[i];
    }
  } b_arr;
#else
  int b_arr[fd_bv_max_high];
#endif
public:
  void * operator new (size_t s) {return freeListMalloc(s);}
  void operator delete(void *, size_t) {
    error("Unexpected call of FDBitVector::delete.");
  }
  void dispose(void) {freeListDispose(this, sizeof(FDBitVector));}

  size_t memory_required(void) { // used for profiling
    return 4 * size_t(ceil(float(findMaxElem())/32));
  }
  FDBitVector(void){}
  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;
  void printDebug(void) const;
  void printDebugLong(void) const;

  Bool contains(int i) const;

  void setEmpty(void);
  void setBit(int i);
  void resetBit(int i);
  void setFromTo(int, int);
  void addFromTo(int, int);

  int findSize(void);
  int findMinElem(void);
  int findMaxElem(void);
  void initList(int list_len, int * list_left, int * list_right);
  int nextBiggerElem(int v, int upper) const;
  Bool next(int i, int &n) const;
  TaggedRef getAsList(void) const;
  FDBitVector * copy(void);
  int operator <= (const int);
  int operator >= (const int);
  int operator -= (const FDBitVector &);
  int mkRaw(int * list_left, int * list_right) const;
  int union_bv(const FDBitVector &, const FDBitVector &);
  int intersect_bv(FDBitVector &, const FDBitVector &);
};

typedef FDBitVector BitArray;

typedef int * intptr;

extern intptr fd_iv_left_sort[MAXFDBIARGS];
extern intptr fd_iv_right_sort[MAXFDBIARGS];

class FiniteDomain {
private:
  int simplify(int list_len, int * list_left, int * list_right);

  int min_elem;
  int max_elem;
  int size;

  enum descr_type {bv_descr = 0, iv_descr = 1, fd_descr = 2};
  static char * descr_type_text[3];
  void * descr;
  descr_type getType(void) const {return (descr_type)ToInt32(andPointer(descr,3));}
  void setType(descr_type t) {descr = orPointer(andPointer(descr,~3),t);}
  void setType(descr_type t, void * p) {descr = orPointer(p,t);}
  void setType(FDBitVector * p) {descr = orPointer(p,bv_descr);}
  void setType(FDIntervals * p) {descr = orPointer(p,iv_descr);}
  void set_iv(void * p) {descr = orPointer(p,iv_descr);}
  FDIntervals * get_iv(void) const {return (FDIntervals *)andPointer(descr,~3);}
  void set_bv(void * p) {descr = p;}
  FDBitVector * get_bv(void) const {return (FDBitVector *)andPointer(descr,~3);}

  FDBitVector * provideBitVector(void) const;
  FDIntervals * provideIntervals(int) const;
  int findSize(void) const {return max_elem - min_elem + 1;}
  Bool isSingleInterval(void) const {return size == (max_elem - min_elem + 1);}
  FDBitVector * asBitVector(void) const;
  FDIntervals * asIntervals(void) const;

#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  Bool isConsistent(void) const;
#endif

public:
  void dispose(void) {
    switch (getType()) {
    case iv_descr: get_iv()->dispose(); return;
    case bv_descr: get_bv()->dispose(); return;
    default: return;
    }
  }
  void FiniteDomainInit(void * d = NULL) {setType(fd_descr, d);};

  FiniteDomain(void * d = NULL) {FiniteDomainInit(d);}

  FiniteDomain(FDState state) {
    switch (state) {
    case fd_empty:
      setEmpty();
      break;
    case fd_full:
      setFull();
      break;
    default:
      error("Unexpected FDState.");
      break;
    }
  }

  FiniteDomain(const FiniteDomain &);
  const FiniteDomain &operator = (const FiniteDomain &fd);
  void gc(void);

  int setEmpty(void);

  int initFull(void);
  int initEmpty(void);
  int initSingleton(int);
  int initList(int list_len, int * list_left, int * list_right,
               int list_min, int list_max);
  int init(int, int);

  int setFull(void);
  int setSingleton(int);

  int getSize(void) const {return size;}
  int minElem(void) const {return min_elem;}
  int maxElem(void) const {return max_elem;}
  int singl(void) const;

  Bool contains(int i) const;
  FDPropState checkAgainst(FiniteDomain &dom);
  TaggedRef getAsList(void) const;
  Bool next(int i, int &n) const;
  int nextBiggerElem(int v) const;
  int constrainBool(void);
  Bool isBool(void) {
    return (min_elem == 0) && (max_elem == 1);
  }

  // non-destructive operators
  FiniteDomain &operator & (const FiniteDomain &) const; // intersection
  FiniteDomain &operator | (const FiniteDomain &) const; // union
  FiniteDomain &operator ~ (void) const;                 // inversion

  // destructive operators
  int operator &= (const FiniteDomain &);
  int operator &= (const int);
  int operator -= (const int);
  int operator -= (const FiniteDomain &);
  int operator += (const int);
  int operator <= (const int);
  int operator >= (const int);

  Bool operator == (const FDState) const;
  Bool operator != (const FDState) const;
  Bool operator == (int) const;
  Bool operator != (int) const;

  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;
  void printDebug(void) const;
  void printDebugLong(void) const;
};


typedef FiniteDomain * FiniteDomainPtr;

class FDIterator {
private:
  FiniteDomainPtr finiteDomain;
  int current;
  int size;
public:
  FDIterator(FiniteDomainPtr fd) : finiteDomain(fd) {}

  int reset(void) {
    size = finiteDomain->getSize() - 1;
    return current = finiteDomain->minElem();
  }

  int next(void) {
    if (size > 0) {
      size -= 1;
      return current = finiteDomain->nextBiggerElem(current);
    } else {
      return -1;
    }
  }
};


class LocalFD : public FiniteDomain {
private:
  char iv_bv_descr[sizeof(FDIntervals)];
public:
  LocalFD() : FiniteDomain(&iv_bv_descr) {
    Assert(sizeof(FDIntervals) == sizeof(FDBitVector));
  }
  const LocalFD &operator =(const FiniteDomain &fd) {
    FiniteDomain::operator=(fd);
    return *this;
  }
};


inline
ostream &operator << (ostream &ofile, const FiniteDomain &fd) {
  fd.print(ofile);
  return ofile;
}


inline
TaggedRef mkTuple(int from, int to){
  OZ_Term s = OZ_tuple(OZ_CToAtom("#"), 2);
  OZ_putArg(s, 1, OZ_CToInt(from));
  OZ_putArg(s, 2, OZ_CToInt(to));
  return s;
}

#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdomn.icc"
#endif

#endif
