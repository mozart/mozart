/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __FDOMN_HH__
#define __FDOMN_HH__


#include "tagged.hh"
#include "value.hh"

#include "fddebug.hh"

#include "oz_cpi.hh"

const int fd_inf = 0;
const int fd_sup = OZ_smallIntMax() - 1;

const int fd_full_size = fd_sup + 1;

// TMUELLER: MAXFDBIARGS twice
#define MAXFDBIARGS 1000 // maximum number of arguments of fd built-ins

struct i_arr_type {int left; int right;};

// Invariants: high == 1 reduce to OZ_FiniteDomain
class FDIntervals {
friend class OZ_FiniteDomainImpl;
private:
  int high;
  OZ_Boolean isConsistent(void) const;
#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  struct _i_arr_type {
    i_arr_type _i_arr[1];
    i_arr_type &operator [] (int i) /*const*/ {
      AssertFD(0 <= i && i < *(((int *)this) - 1));
      return _i_arr[i];
    }
    i_arr_type operator [] (int i) const {
      AssertFD(0 <= i && i < *(((int *)this) - 1));
      return _i_arr[i];
    }
  } i_arr;
#else
  i_arr_type i_arr[1];
#endif

  int findPossibleIndexOf(int) const;
public:
  FDIntervals(int hi) {high = hi;}
  FDIntervals(const FDIntervals &);

  const FDIntervals &operator = (const FDIntervals &);

  static size_t sizeOf(int hi) {
    return (1 + 2 * hi) * sizeof(int);
  }
  size_t sizeOf(void) {
    return sizeOf(high);
  }
  void * operator new(size_t, int hi) {
    return heapMalloc(FDIntervals::sizeOf(hi));
  }
  void * operator new (size_t)  {
    error("Unexpected call of FDIntervals::new.");
    return NULL;
  }
  void operator delete(void *, size_t) {
    error("Unexpected call of FDIntervals::delete.");
  }
  void dispose(void) {
    freeListDispose(this, sizeOf());
  }
  int getHigh(void) { return high; }

  void print(ostream &, int = 0) const;
  void printLong(ostream &, int = 0) const;
  void printDebug(void) const;
  void printDebugLong(void) const;

  OZ_Boolean contains(int i) const;
  int findSize(void);
  int findMinElem(void);
  int findMaxElem(void);
  void initList(int list_len, int * list_left, int * list_right);
  void init(int l, int r) {i_arr[0].left = l; i_arr[0].right = r;}
  int nextSmallerElem(int v, int upper) const;
  int nextLargerElem(int v, int upper) const;
  int midElem(int i) const;
  int upperBound(int v) const;
  int lowerBound(int v) const;
  OZ_Term getAsList(void) const;
  FDIntervals * copy(void);
  int operator <= (const int);
  int operator >= (const int);
  FDIntervals * operator -= (const int);
  FDIntervals * operator += (const int);
  void init(int, int, int, int);
  FDIntervals * complement(FDIntervals *);
  FDIntervals * complement(int, int * , int *);
  int union_iv(const FDIntervals &, const FDIntervals &);
  int intersect_iv(FDIntervals &, const FDIntervals &);
  int subtract_iv (FDIntervals &, const FDIntervals &);
};


extern int fd_bv_max_high, fd_bv_max_elem, fd_bv_conv_max_high;
extern int * fd_bv_left_conv, * fd_bv_right_conv;

inline int div32(int n) { return n >> 5; }
inline int mod32(int n) { return n & 0x1f; }
inline int word32(int n) { n+=1; return mod32(n) ? div32(n) + 1 : div32(n); }

// Invariants: size < max_elem - min_elem + 1 otherwise
// reduce to OZ_FiniteDomain
class FDBitVector {
friend class OZ_FiniteDomainImpl;
private:
  int high;
#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  struct b_arr_t {
    int _b_arr[1];
    int &operator [] (int i) /*const*/ {
      AssertFD(0 <= i && i < *(((int *)this) - 1));
      return _b_arr[i];
    }
    int operator [] (int i) const {
      AssertFD(0 <= i && i < *(((int *)this) - 1));
      return _b_arr[i];
    }
  } b_arr;
#else
  int b_arr[1];
#endif
public:
  void * operator new (size_t) {
    error("Unexpected call of FDBitVector::new.");
    return NULL;
  }
  void operator delete(void *, size_t) {
    error("Unexpected call of FDBitVector::delete.");
  }
  FDBitVector(void) { error("Unexpected call of FDBitVector::FDBitVector."); }
  FDBitVector(int hi) : high(hi) { Assert(high <= word32(fd_bv_max_elem)); }

  static size_t sizeOf(int hi) { return (1 + hi) * sizeof(int); }
  size_t sizeOf(void) { return sizeOf(high); }

  int currBvMaxElem(void) const { return 32*high-1; }
  int getHigh(void) { return high; }
  void * operator new(size_t, int hi) {
    return heapMalloc(FDBitVector::sizeOf(hi));
  }
  void dispose(void) {
    freeListDispose(this, sizeOf());
  }

  void print(ostream &, int = 0) const;
  void printLong(ostream &, int = 0) const;
  void printDebug(void) const;
  void printDebugLong(void) const;

  OZ_Boolean contains(int i) const;

  void setEmpty(void);
  void setBit(int i);
  void resetBit(int i);
  void setFromTo(int, int);
  void addFromTo(int, int);

  int findSize(void);
  int findMinElem(void);
  int findMaxElem(void);
  void findHigh(int);
  void initList(int list_len, int * list_left, int * list_right);
  int nextSmallerElem(int v, int min_elem) const;
  int nextLargerElem(int v, int max_elem) const;
  int midElem(int i) const;
  int lowerBound(int v, int min_elem) const;
  int upperBound(int v, int max_elem) const;
  OZ_Term getAsList(void) const;
  FDBitVector * copy(void);
  int operator <= (const int);
  int operator >= (const int);
  int operator -= (const FDBitVector &);
  int mkRaw(int * list_left, int * list_right) const;
  int mkRawOutline(int * list_left, int * list_right) const;

  const FDBitVector &operator = (const FDBitVector &);

  int union_bv(const FDBitVector &, const FDBitVector &);
  int intersect_bv(const FDBitVector &, const FDBitVector &);
  int intersect_bv(const FDBitVector &);
};

typedef FDBitVector BitArray;

typedef int * intptr;

extern intptr fd_iv_left_sort[MAXFDBIARGS];
extern intptr fd_iv_right_sort[MAXFDBIARGS];

class OZ_FiniteDomainImpl : public OZ_FiniteDomain {
protected:
  enum descr_type {fd_descr = 0, iv_descr = 1, bv_descr = 2};

  int simplify(int list_len, int * list_left, int * list_right);
  descr_type getType(void) const {
    return (descr_type)ToInt32(andPointer(descr,3));
  }
  FDIntervals * get_iv(void) const {
    Assert(getType() == iv_descr);
    return (FDIntervals *)andPointer(descr,~3);
  }
  FDBitVector * get_bv(void) const {
    Assert(getType() == bv_descr);
    return (FDBitVector *)andPointer(descr,~3);
  }

  void setType(descr_type t);
  void setType(descr_type t, void * p);
  void setType(FDBitVector * p);
  void setType(FDIntervals * p);
  void set_iv(void * p);
  void set_bv(void * p);

  FDBitVector * provideBitVector(int) const;
  FDIntervals * provideIntervals(int) const;
  int findSize(void) const;
  OZ_Boolean isSingleInterval(void) const;
  FDBitVector * asBitVector(void) const;
  FDIntervals * asIntervals(void) const;

  OZ_Boolean isConsistent(void) const;
public:
  void disposeExtension(void);
  void FiniteDomainInit(void * d);

  OZ_FiniteDomainImpl(void);

  unsigned getDescrSize(void);

  OZ_FiniteDomainImpl(const OZ_FiniteDomainImpl &);
  const OZ_FiniteDomainImpl &operator = (const OZ_FiniteDomainImpl &fd);

  OZ_Boolean contains(int i) const;
  int initFull(void);
  int initEmpty(void);
  int initSingleton(int);
  int initList(int list_len, int * list_left, int * list_right,
               int list_min, int list_max);
  int initRange(int, int);
  int initDescr(OZ_Term);
  int initBool(void);

  int getSize(void) const {return size;}
  int minElem(void) const {return min_elem;}
  int maxElem(void) const {return max_elem;}
  int getWidth(void) const {return max_elem - min_elem;}
  int singl(void) const;

  OZ_Term getAsList(void) const;
  int midElem(void) const;
  int nextSmallerElem(int v) const;
  int nextLargerElem(int v) const;
  int lowerBound(int v) const;
  int upperBound(int v) const;
  int intersectWithBool(void);
  int constrainBool(void);

  // non-destructive operators
  OZ_FiniteDomainImpl operator & (const OZ_FiniteDomainImpl &) const; // intersection
  OZ_FiniteDomainImpl operator | (const OZ_FiniteDomainImpl &) const; // union
  OZ_FiniteDomainImpl operator ~ (void) const;                 // inversion

  // destructive operators
  int operator &= (const OZ_FiniteDomainImpl &);
  int operator &= (const int);
  int operator -= (const int);
  int operator -= (const OZ_FiniteDomainImpl &);
  int operator += (const int);
  int operator <= (const int);
  int operator >= (const int);

  OZ_Boolean operator == (const OZ_FDState) const;
  OZ_Boolean operator != (const OZ_FDState) const;
  OZ_Boolean operator == (const int) const;
  OZ_Boolean operator != (const int) const;

  void print(ostream &, int = 0) const;
  void printLong(ostream &, int = 0) const;
  void printDebug(void) const;
  void printDebugLong(void) const;

  void gc(void);
  void copyExtensionInline(void);
  void copyExtension(void);
};

void initFDs();
void reInitFDs(int);

#endif
