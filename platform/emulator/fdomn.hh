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

// limits of a bit array
//const int baSize = 8;
const int baSize = 4;

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

enum LeGeFull_e {em, le, ge, fu, no, bo, si};
enum DFlag_e {empty, full, discrete, boolish, singleton};

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
  BitArray(const BitArray *ba) {*this = *ba;}

  Bool contains(int i) const {
    if (i > fdMaxBA || i < fdMinBA)
      return NO;
    else
      return (array[i / bits] & (1 << (i % bits))) ? OK  :  NO;
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

  int &operator [](int i) {
    return array[i];
  }

  int operator <=(const int y_upper);
  int operator >=(const int y_lower);

  TaggedRef getAsList(void) const;
  Bool next(int i, int &n) const;
  int nextBiggerElem(int v, int upper) const;

  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;
}; // BitArray

#define RANGEFLAG 1

enum FDState {det=0, bounds, size, eqvar, any};

class FiniteDomain {
protected:
  unsigned short lower;
  unsigned short upper;
  BitArray *bitArray;

  Bool isRange(void) const {
    return ((int)bitArray & RANGEFLAG) ? OK : NO;
  }

  BitArray* setRange(BitArray *ba) const {
    return (BitArray*)((int)ba | RANGEFLAG);
  }

  BitArray* resetRange(BitArray *ba) const {
    return (BitArray*)((int)ba & ~RANGEFLAG);
  }

  void setEmpty(void) {
    lower = 1; upper = 0; bitArray = setRange(bitArray);
  }

  void allocateBitArray(void) {
    bitArray = resetRange(bitArray);
    if (bitArray == NULL)
      bitArray = new BitArray;
  }

  BitArray rangeToBitArray(void) const;
  BitArray * rangeToBitArray(BitArray * ba) const;

  int getRangeSize(void) const {
    DebugCheck(isRange() == NO, error("Range expected."));
    return upper - lower + 1;
  }

  Bool containsRange(int i) const {
    return (i >= lower && i <= upper) ? OK : NO;
  }

  BitArray * becomeBitArray(void);
public:
  USEHEAPMEMORY;

  FiniteDomain(BitArray* ba = NULL) : bitArray(setRange(ba)) {};
  FiniteDomain(const FiniteDomain &fd);

  const FiniteDomain &operator =(const FiniteDomain &fd) {
    if (this != &fd) {
//    FiniteDomain(fd);
      lower = fd.lower;
      upper = fd.upper;
      if (fd.isRange() == OK) {
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
  void init(LeGeFull_e type, int n = -1);

  FiniteDomain &operator &(const FiniteDomain &y) const; // intersection
  FiniteDomain &operator |(const FiniteDomain &y) const; // union
  FiniteDomain &operator ~(void) const;                  // inversion

  FiniteDomain &operator <=(const int y_upper) const;
  FiniteDomain &operator >=(const int y_lower) const;

  int operator <(const int y_upper);
  int operator >(const int y_lower);
  int operator &=(const FiniteDomain &y);

  FiniteDomain &operator -=(const int not_in);
  FiniteDomain &operator +=(const int is_in);
  FiniteDomain &operator &=(const int singl);


  int minElem(void) const {return lower;}
  int maxElem(void) const {return upper;}
  void getMinMax(int &l, int &u) const {l = lower; u = upper;}

  int getSize(void) const {
    return isRange() == OK ? getRangeSize() : bitArray->getSize();
  }

  Bool contains(int i) const {
    if (isRange() == OK)
      return containsRange(i);
    else
      return bitArray->contains(i);
  }


  Bool operator == (const DFlag_e flag) const
  {
    switch (flag){
    case empty:
      return getSize() == 0 ? OK : NO;
    case full:
      if (isRange() == OK)
        return (lower == fdMinR && upper == fdMaxR) ? OK : NO;
      return (lower == fdMinBA && upper == fdMaxBA &&
              bitArray->getSize() == upper - lower + 1) ? OK : NO;
    case discrete:
      if (isRange() == OK)
        return (lower != fdMinR || upper != fdMaxR) ? OK : NO;
      return (lower != fdMinBA || upper != fdMaxBA ||
              bitArray->getSize() != upper - lower + 1) ? OK : NO;
    case singleton:
      return (lower == upper) ? OK : NO;
    case boolish:
      return (contains(0) == OK || contains(1) == OK) ? OK : NO;
    default:
      error("Unexpected case at %s:%d.", __FILE__, __LINE__);
      return NO;
    } //switch
  }

  Bool operator != (const DFlag_e flag) const
  {
    switch (flag){
    case empty:
      return getSize() ? OK : NO;
    case full:
      if (isRange() == OK)
        return (lower != fdMinR || upper != fdMaxR) ? OK : NO;
      return (lower != fdMinBA || upper != fdMaxBA ||
              bitArray->getSize() != upper - lower + 1) ? OK : NO;
    case discrete:
      if (isRange() == OK)
        return (lower == fdMinR && upper == fdMaxR) ? NO : OK;
      return (lower == fdMinBA || upper == fdMaxBA ||
              bitArray->getSize() == upper - lower + 1) ? OK : NO;
    case singleton:
      return (lower == upper) ? NO : OK;
    case boolish:
      return (contains(0) == OK || contains(1) == OK) ? NO : OK;
    default:
      error("Unexpected case at %s:%d.", __FILE__, __LINE__);
      return NO;
    } //switch
  }


  Bool operator == (const FiniteDomain &) const;
  Bool operator != (const FiniteDomain &) const;

  TaggedRef getAsList(void) const;
  Bool next(int i, int &n) const;
  int nextBiggerElem(int v) const;

  FDState checkDom(FiniteDomain &dom);

  int singl(void) const {
    DebugCheck(getSize() != 1, error("Domain must be singletons"));
    return lower;
  }

  void constrainBool(void)
  {
#ifdef DEBUG_CHECK
    if ((*this == boolish) == NO)
      error("Boolish finite domain expected.");
#endif
    Bool zero = contains(0);
    Bool one = contains(1);
    bitArray = setRange(bitArray);
    lower = (zero == OK) ? 0 : 1;
    upper = (one == OK) ? 1 : 0;
  }

  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;
  void printDebug(void) const;

  void gc(void) {
    if (isRange() == OK)
      bitArray = setRange(NULL);
    else
      bitArray = new BitArray(bitArray);
  }

  static FDState leftDom;
  static FDState rightDom;

#ifdef PROFILE_FD
  static unsigned constrCalled;
  static unsigned unifyCalled;
  static unsigned varsCreated;
#endif //PROFILE_FD
}; // FiniteDomain


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
  LocalFD() {bitArray = setRange(&localBitArray);}
  const LocalFD& operator =(const FiniteDomain &fd){
    FiniteDomain::operator=(fd);
    return *this;
  }
};


TaggedRef makeRangeTuple(int from, int to);


#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdomn.icc"
#endif


#endif
