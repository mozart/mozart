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

#ifdef BETA

#include "fdomn_beta.hh"

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

  FiniteDomain(BitArray* ba = NULL) : bitArray(setRange(ba)) {};
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


#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdomn.icc"
#endif

#endif

#endif
