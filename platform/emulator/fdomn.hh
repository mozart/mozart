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

// CAUTION:
// Make sure that only references to LocalFD are returned otherwise
// you can get in big trouble!

// bit array covers the whole range, ie. conversion from range to bit array
// may not cause inaccuracy
#define BA_COVERS_R

// limits of a bit array
const int baSize = 8;

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


class BitArray {
private:
  int array[baSize];
  int size;
public:
  USEHEAPMEMORY;
  
  BitArray() {};
  BitArray(const BitArray *ba) {*this = *ba;}

  Bool isInDomain(int i) const { 
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
  
  void findLowerUpperSize(unsigned short &lower, unsigned short &upper);

  int getSize(void) {return size;};

  int &operator [](int i) {
    return array[i];
  }

  inline TaggedRef getAsList(void) const;
  inline Bool next(int i, int &n) const;

  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;  
}; // BitArray

#define RANGEFLAG 1

enum FDState {det=0, bounds, size, eqvar, any}; // any must always be the last

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
  
  inline BitArray rangeToBitArray(void) const;

public:
  USEHEAPMEMORY;

  FiniteDomain(BitArray* ba = NULL) : bitArray(setRange(ba)) {};
  inline FiniteDomain(int len, int intList[]);
  inline FiniteDomain(int intList[], int len); // constr for fdNotList
  inline FiniteDomain(int from, int to);
  inline FiniteDomain(LeGeFull_e type, int n = -1);
  inline FiniteDomain(const FiniteDomain &fd);

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

  inline FiniteDomain &operator &(const FiniteDomain &y) const; // intersection
  inline FiniteDomain &operator |(const FiniteDomain &y) const; // union
  inline FiniteDomain &operator ~(void) const;                  // inversion

  inline FiniteDomain &operator <=(const int y_upper) const;    
  inline FiniteDomain &operator >=(const int y_lower) const;
  
  inline FiniteDomain &operator -=(const int not_in); 
  inline FiniteDomain &operator +=(const int is_in); 
  inline FiniteDomain &operator &=(const int singl);
  
  int minElem(void) const {return lower;}
  int maxElem(void) const {return upper;}
  void getMinMax(int &l, int &u) const {l = lower; u = upper;}
  
  int getSize(void) const {
    return isRange() == OK ? upper - lower + 1 : bitArray->getSize();
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
      return (isInDomain(0) == OK || isInDomain(1) == OK) ? OK : NO;
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
      return (isInDomain(0) == OK || isInDomain(1) == OK) ? NO : OK;
    default:
      error("Unexpected case at %s:%d.", __FILE__, __LINE__);
      return NO;
    } //switch
  }


  inline Bool operator == (const FiniteDomain &) const;
  inline Bool operator != (const FiniteDomain &) const;

  inline TaggedRef getAsList(void) const;
  inline Bool next(int i, int &n) const;

  inline FDState checkDom(FiniteDomain &dom);
  
  inline Bool isInDomain(int i) const;
  int getSingleton(void) const {return lower;}

  void constrainBool(void)
  {
#ifdef DEBUG_CHECK
    if ((*this == boolish) == NO)
      error("Boolish finite domain expected.");
#endif
    Bool zero = isInDomain(0);
    Bool one = isInDomain(1);
    bitArray = setRange(bitArray);
    lower = zero == OK ? 0 : 1;
    upper = one == OK ? 1 : 0;
  }
  
  void print(ostream & = cout, int = 0) const;
  void printLong(ostream & = cout, int = 0) const;

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



class LocalFD : public FiniteDomain {
public:
  BitArray localBitArray;
  LocalFD() {bitArray = setRange(&localBitArray);}
  const LocalFD& operator =(const FiniteDomain &fd){
    FiniteDomain::operator=(fd);
  }
};


inline TaggedRef makeRangeTuple(int from, int to){
  OZ_Term s = OZ_makeTuple("#",2);
  OZ_putArg(s,1,OZ_intToTerm(from));
  OZ_putArg(s,2,OZ_intToTerm(to));
  return s;
} // makeRangeTupel


#ifndef OUTLINE
#include "fdomn.icc"
#endif

     
#endif
