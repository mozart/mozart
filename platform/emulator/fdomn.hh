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

#define BETA

#ifdef BETA

#include "types.hh"
#include "term.hh"
#include "bignum.hh"


enum FDPropState {fd_det = 0, fd_bounds, fd_size, fd_eqvar, fd_any};
enum FDState {fd_empty, fd_full, fd_discrete, fd_singleton};


#define FD_NOI 4 // number of intervals
#define MAXFDBIARGS 1000 // maximum number of arguments of fd built-ins

const int fd_iv_max_high = FD_NOI;
const int fd_iv_max_elem = OzMaxInt;
const int fd_full_size = fd_iv_max_elem + 1;

// Invariants: high == 1 other reduce to FiniteDomain
class FDIntervals {
friend class FiniteDomain;
private:
  int high;
  struct i_arr_type {int left; int right;} i_arr[fd_iv_max_high];
  int findPossibleIndexOf(int) const;
#ifdef DEBUG_CHECK
  Bool isConsistent(void) {
    for (int i = 0; i < high; i++) {
      if (i_arr[i].left > i_arr[i].right)
        return FALSE;
      if ((i + 1 < high) && (i_arr[i].right >= i_arr[i + 1].left))
        return FALSE;
    }
    return TRUE;
  }
#endif
public:
  FDIntervals(int hi) {high = hi;}
  FDIntervals(const FDIntervals &);

  const FDIntervals &operator = (const FDIntervals &);

  void * operator new (size_t s) {return heapMalloc(s);}
  void * operator new (size_t s, int hi) {
    return heapMalloc(s + 2 * (hi - fd_iv_max_high) * sizeof(int));
  }
  void operator delete(void *, size_t) {
    error("Unexpected call of FDIntervals::delete.");
  }
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
  FDIntervals * complement(int * , int *);
  void copy(FDIntervals *);
  int union_iv(const FDIntervals &, const FDIntervals &);
  int intersect_iv(FDIntervals &, const FDIntervals &);
};

const int fd_bv_max_high = 2 * FD_NOI + 1;
const int fd_bv_max_elem = 32 * fd_bv_max_high - 1;
const int fd_bv_conv_max_high = (fd_bv_max_elem) / 2 + 2;

int * fd_bv_left_conv;
int * fd_bv_right_conv;

// Invariants: size < max_elem - min_elem + 1 otherwise reduce to FiniteDomain
class FDBitVector {
private:
  int b_arr[fd_bv_max_high];
public:
  void * operator new (size_t s) {return heapMalloc(s);}
  void operator delete(void *, size_t) {
    error("Unexpected call of FDBitVector::delete.");
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
  int mkRaw(int * list_left, int * list_right) const;
  int union_bv(const FDBitVector &, const FDBitVector &);
  int intersect_bv(FDBitVector &, const FDBitVector &);
};

typedef FDBitVector BitArray;

typedef int * intptr;

intptr * fd_iv_left_sort;
intptr * fd_iv_right_sort;

class FiniteDomain {
private:
  int simplify(int list_len, int * list_left, int * list_right);

  int min_elem;
  int max_elem;
  int size;

  enum descr_type {bv_descr = 0, iv_descr = 1, fd_descr = 2};
  static char * descr_type_text[3];
  void * descr;
  descr_type getType(void) const {return (descr_type)(((int) descr) & 3);}
  void setType(descr_type t) {descr = (void *) (((int) descr & ~3) | t);}
  void setType(descr_type t, void * p) {descr = (void *) (((int) p) | t);}
  void set_iv(void * p) {descr = (void *) (((int) p) | iv_descr);}
  FDIntervals * get_iv(void) const {return (FDIntervals *)(((int)descr) & ~3);}
  void set_bv(void * p) {descr = p;}
  FDBitVector * get_bv(void) const {return (FDBitVector *)(((int)descr) & ~3);}

  int setEmpty(void);
  FDBitVector * provideBitVector(void);
  FDIntervals * provideIntervals(int);
  int findSize(void) {return max_elem - min_elem + 1;}
  Bool isSingleInterval(void) const {return size == (max_elem - min_elem + 1);}
  FDBitVector * asBitVector(void) const;
  FDIntervals * asIntervals(void) const;
public:
  void * operator new (size_t s) {return heapMalloc(s);}
  void operator delete(void *, size_t) {
    error("Unexpected call of FiniteDomain::delete.");
  }

  void FiniteDomainInit(void * d = NULL) {setType(fd_descr, d);};

  FiniteDomain(void * d = NULL) {FiniteDomainInit(d);}

  FiniteDomain(const FiniteDomain &);
  const FiniteDomain &operator = (const FiniteDomain &fd);
  void gc(void);

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

  // non-destructive operators
  FiniteDomain &operator & (const FiniteDomain &) const; // intersection
  FiniteDomain &operator | (const FiniteDomain &) const; // union
  FiniteDomain &operator ~ (void) const;                 // inversion

  // destructive operators
  int operator &= (const FiniteDomain &);
  int operator &= (const int);
  int operator -= (const int);
  int operator += (const int);
  int operator <= (const int);
  int operator >= (const int);

  Bool operator == (const FDState) const;
  Bool operator != (const FDState) const;

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
TaggedRef mkTuple(int from, int to){
  OZ_Term s = OZ_tuple(OZ_CToAtom("#"), 2);
  OZ_putArg(s, 1, OZ_CToInt(from));
  OZ_putArg(s, 2, OZ_CToInt(to));
  return s;
}





//=============================================================================
#else

#if defined(__GNUC__)
#pragma interface
#endif

#include "types.hh"
#include "term.hh"
#include "bignum.hh"

// CAUTION:
// Make sure that only references to LocalFD are returned otherwise
// you can get in big trouble!

// bit array covers the whole range, ie. conversion from range to bit array
// may not cause inaccuracy
#define BA_COVERS_R
#define MAXFDBIARGS 1000 // maximum number of arguments of fd built-ins
// limits of a bit array
const int baSize = 8;
//const int baSize = 4;

//const int baSize = 32;

const int bits = 32;
const int fdMaxBA = baSize * bits - 1;
const int fdMinBA = 0;
// limits of a range
#ifdef BA_COVERS_R
const int fdMaxR = fdMaxBA;
#else
const int fdMaxR = USHRT_MAX;
#endif
const int fdMinR = 0;

enum DFlag_e {fd_empty, fd_full, fd_discrete, fd_singleton};

extern unsigned char numOfBitsInByte[];
extern int toTheLowerEnd[];
extern int toTheUpperEnd[];

class BitArray {
private:
  int array[baSize];
  int size;
public:
  USEHEAPMEMORY;

  BitArray() {};
  BitArray(const BitArray * ba) {*this = *ba;}

  Bool contains(int i) const {
    return (i > fdMaxBA || i < fdMinBA)
      ? FALSE : array[i / bits] & (1 << (i % bits));
  }

  void empty(void) {
    size = 0; int i = baSize; while (i--) array[i] = 0;
  }
  void full(void) {
    size = fdMaxBA - fdMinBA + 1; int i = baSize; while (i--) array[i] = ~0;
  }

  void setBit(int i) {
    if (i > fdMaxBA ||  i < fdMinBA)
      return;
    array[i / bits] |= (1 << (i % bits));
  }

  void setFromTo(int from, int to);

  void resetBit(int i) {
    if (i > fdMaxBA ||  i < fdMinBA)
      return;
    array[i / bits] &= ~(1 << (i % bits));
  }

  int findLowerUpperSize(unsigned short &lower, unsigned short &upper);
  unsigned short findLower(void);
  unsigned short findUpper(void);
  void findSize(void);

  int getSize(void) {return size;}

  void adjustSizeBy(int s) {size += s;}

  int &operator [](int i) {return array[i];}

  int operator <=(const int y_upper);
  int operator >=(const int y_lower);

  TaggedRef getAsList(void) const;
  Bool next(int i, int &n) const;
  int nextBiggerElem(int v, int upper) const;

  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;
}; // BitArray

#define RANGEFLAG 1

enum FDPropState {fd_det=0, fd_bounds, fd_size, fd_eqvar, fd_any};

class FiniteDomain {
protected:
  unsigned short lower;
  unsigned short upper;
  BitArray *bitArray;

  Bool isRange(void) const {return ((int)bitArray & RANGEFLAG);}

  BitArray* setRange(BitArray *ba) const {
    return (BitArray*)((int)ba | RANGEFLAG);
  }

  BitArray* resetRange(BitArray *ba) const {
    return (BitArray*)((int)ba & ~RANGEFLAG);
  }

  void allocateBitArray(void) {
    bitArray = resetRange(bitArray);
    if (bitArray == NULL)
      bitArray = new BitArray;
  }

  BitArray rangeToBitArray(void) const;
  BitArray * rangeToBitArray(BitArray * ba) const;

  int setEmpty(void) {
    lower = 1; upper = 0; bitArray = setRange(bitArray); return 0;
  }

  int getRangeSize(void) const {
    DebugCheck(!isRange(), error("Range expected."));
    return upper - lower + 1;
  }

  Bool containsRange(int i) const {return (i >= lower && i <= upper);}

  BitArray * becomeBitArray(void);
public:
  USEHEAPMEMORY;

  void FiniteDomainInit(BitArray* ba = NULL) { bitArray = setRange(ba); };
  FiniteDomain(BitArray* ba = NULL) { FiniteDomainInit(ba);};
  FiniteDomain(const FiniteDomain &fd);

  const FiniteDomain &operator =(const FiniteDomain &fd) {
    if (this != &fd) {
      lower = fd.lower;
      upper = fd.upper;
      if (fd.isRange()) {
        bitArray = setRange(bitArray);
      } else {
        if ((bitArray = resetRange(bitArray)) == NULL)
          bitArray = new BitArray(fd.bitArray);
        else {
#ifdef DEBUG_FD
          if (bitArray == NULL)
            error("bitArray may never be NULL.");
#endif
          *bitArray = *fd.bitArray;
        }
      }
    }
    return *this;
  }

  void init(int len, int intList[]);
  void init(int intList[], int len); // constr for fdNotList
  void init(int from, int to);
  void init(DFlag_e type, int n = -1);
  void initFull(void) {init(fd_full);}
  void initEmpty(void) {init(fd_empty);}
  void initSingleton(int i) {init(fd_singleton, i);}

  void setFull(void) {
    bitArray = setRange(NULL);
    lower = fdMinR;
    upper = fdMaxR;
  }
  void setSingleton(int n) {
    if (n > fdMaxR || n < fdMinR){
      setEmpty();
      return;
    }
    bitArray = setRange(NULL);
    lower = upper = n;
  }

  // non-destructive operators
  FiniteDomain &operator &(const FiniteDomain &y) const; // intersection
  FiniteDomain &operator |(const FiniteDomain &y) const; // union
  FiniteDomain &operator ~(void) const;                  // inversion

  // destructive operators
  int operator &=(const FiniteDomain &y);
  int operator &=(const int singl);
  int operator -=(const int not_in);
  int operator +=(const int is_in);
  int operator <=(const int y_upper);
  int operator >=(const int y_lower);


  int minElem(void) const {return lower;}
  int maxElem(void) const {return upper;}

  int getSize(void) const {
    return isRange() ? getRangeSize() : bitArray->getSize();
  }

  Bool contains(int i) const {
    return isRange() ? containsRange(i) : bitArray->contains(i);
  }

  Bool operator == (const DFlag_e flag) const
  {
    switch (flag){
    case fd_empty:
      return (getSize() == 0);
    case fd_full:
      if (isRange())
        return (lower == fdMinR && upper == fdMaxR);
      return (lower == fdMinBA && upper == fdMaxBA &&
              bitArray->getSize() == upper - lower + 1);
    case fd_discrete:
      if (isRange())
        return (lower != fdMinR || upper != fdMaxR);
      return (lower != fdMinBA || upper != fdMaxBA ||
              bitArray->getSize() != upper - lower + 1);
    case fd_singleton:
      return (lower == upper);
    default:
      error("Unexpected case at %s:%d.", __FILE__, __LINE__);
      return FALSE;
    }
  }

  Bool operator != (const DFlag_e flag) const
  {
    switch (flag){
    case fd_empty:
      return (getSize() != 0);
    case fd_full:
      if (isRange())
        return (lower != fdMinR || upper != fdMaxR);
      return (lower != fdMinBA || upper != fdMaxBA ||
              bitArray->getSize() != upper - lower + 1);
    case fd_discrete:
      if (isRange())
        return !(lower == fdMinR && upper == fdMaxR);
      return (lower == fdMinBA || upper == fdMaxBA ||
              bitArray->getSize() == upper - lower + 1);
    case fd_singleton:
      return (lower != upper);
    default:
      error("Unexpected case at %s:%d.", __FILE__, __LINE__);
      return FALSE;
    }
  }


  Bool operator == (const FiniteDomain &) const;
  Bool operator != (const FiniteDomain &) const;

  TaggedRef getAsList(void) const;
  Bool next(int i, int &n) const;
  int nextBiggerElem(int v) const;

  FDPropState checkAgainst(FiniteDomain &dom);

  int singl(void) const {
    DebugCheck(getSize() != 1, error("Domain must be singletons"));
    return lower;
  }

  int constrainBool(void) {
    lower = contains(0) ? 0 : 1;
    upper = contains(1) ? 1 : 0;
    bitArray = setRange(bitArray);
    return getRangeSize();
  }

  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;
  void printDebug(void) const;

  void gc(void) {
    bitArray = isRange() ? setRange(NULL) : new BitArray(bitArray);
  }

}; // FiniteDomain

typedef FiniteDomain * FiniteDomainPtr;

class FDIterator {
private:
  FiniteDomain* finiteDomain;
  int current;
  int size;
public:
  FDIterator(FiniteDomain* fd) : finiteDomain(fd) {}

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
public:
  BitArray localBitArray;
  LocalFD() {
    CHECK_POINTERLSB(&localBitArray);
    bitArray = setRange(&localBitArray);
  }
  const LocalFD& operator =(const FiniteDomain &fd){
    FiniteDomain::operator=(fd);
    return *this;
  }
};


extern TaggedRef mkTuple(int from, int to);

#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdomn.icc"
#endif

#endif

#endif
