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

#include "oz.h"

class ostream;


class FDBitVector;
class FDIntervals;


enum FDPropState {fd_det = 0, fd_bounds, fd_any};
enum FDState {fd_empty, fd_full, fd_bool, fd_singleton};
#define MAXFDBIARGS 1000 // maximum number of arguments of fd built-ins
#define FDMAGICNUMBER MAXFDBIARGS
const int fd_inf = 0;
const int fd_sup = OZ_getMaxInt() - 1;

class OZ_FiniteDomain {
private:
  int simplify(int list_len, int * list_left, int * list_right);

  int min_elem;
  int max_elem;
  int size;
  
  enum descr_type {bv_descr = 0, iv_descr = 1, fd_descr = 2};
  void * descr;

  descr_type getType(void) const;
  void setType(descr_type t);
  void setType(descr_type t, void * p);
  void setType(FDBitVector * p);
  void setType(FDIntervals * p);
  void set_iv(void * p);
  FDIntervals * get_iv(void) const;
  void set_bv(void * p);
  FDBitVector * get_bv(void) const;

  FDBitVector * provideBitVector(void) const;
  FDIntervals * provideIntervals(int) const;
  int findSize(void) const;
  OZ_Boolean isSingleInterval(void) const;
  FDBitVector * asBitVector(void) const;
  FDIntervals * asIntervals(void) const;

  OZ_Boolean isConsistent(void) const;
  OZ_Boolean contains(int i) const;
public:
  void dispose(void);
  void FiniteDomainInit(void * d);

  OZ_FiniteDomain(void * d);
  OZ_FiniteDomain(void);

  unsigned getDescrSize(void);

  int setEmpty(void);
  int setFull(void);

  OZ_FiniteDomain(FDState state);

  OZ_FiniteDomain(const OZ_FiniteDomain &);
  const OZ_FiniteDomain &operator = (const OZ_FiniteDomain &fd);

  int initFull(void);
  int initEmpty(void);
  int initSingleton(int);
  int initList(int list_len, int * list_left, int * list_right,
	       int list_min, int list_max);
  int init(int, int);
  int init(OZ_Term);
  
  int setSingleton(int);
  int setBool(void);

  int getSize(void) const {return size;}
  int minElem(void) const {return min_elem;}
  int maxElem(void) const {return max_elem;}
  int singl(void) const;
   
  OZ_Boolean isIn(int i) const;
  OZ_Term getAsList(void) const;
  OZ_Boolean next(int i, int &n) const; 
  int nextBiggerElem(int v) const;
  int intersectWithBool(void);
  int constrainBool(void);
  
  // non-destructive operators
  OZ_FiniteDomain operator & (const OZ_FiniteDomain &) const; // intersection
  OZ_FiniteDomain operator | (const OZ_FiniteDomain &) const; // union
  OZ_FiniteDomain operator ~ (void) const;                 // inversion

  // destructive operators
  int operator &= (const OZ_FiniteDomain &);
  int operator &= (const int);
  int operator -= (const int); 
  int operator -= (const OZ_FiniteDomain &);
  int operator += (const int); 
  int operator <= (const int);    
  int operator >= (const int);

  OZ_Boolean operator == (const FDState) const;
  OZ_Boolean operator != (const FDState) const;
  OZ_Boolean operator == (const int) const;
  OZ_Boolean operator != (const int) const;

  void print(ostream &, int = 0) const;
  void printLong(ostream &, int = 0) const;
  void printDebug(void) const;
  void printDebugLong(void) const;

  void gc(void);
  void copyExtension(void);
};


typedef OZ_FiniteDomain * OZ_FiniteDomainPtr;

class FDIterator {
private:
  OZ_FiniteDomainPtr finiteDomain;
  int current;
  int size;
public:
  FDIterator(OZ_FiniteDomainPtr fd) : finiteDomain(fd) {}

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


#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdomn.icc"
#endif


inline
ostream &operator << (ostream &ofile, const OZ_FiniteDomain &fd) {
  fd.print(ofile);
  return ofile;
}


#endif
