/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __OZ_CPI_HH__
#define __OZ_CPI_HH__

#include <stddef.h>
#include <iostream.h>

#include "oz.h"

//-----------------------------------------------------------------------------
// misc macros

#define EXPECTED_TYPE(S) char * expectedType = S

#define EXPECT(O, P, F)                                                     \
  {                                                                         \
    OZ_expect_t r = O.F(OZ_args[P]);                                        \
    if (O.isFailing(r)) {                                                   \
      O.fail();                                                             \
      return OZ_typeError(OZ_self, OZ_args, OZ_arity, expectedType, P, ""); \
    } else if (O.isSuspending(r))                                           \
      return O.suspend(OZ_makeSuspendedThread(OZ_self,OZ_args,OZ_arity));   \
  }

#define EXPECT_SUSPEND(O, P, F, SC)                                         \
  {                                                                         \
    OZ_expect_t r = O.F(OZ_args[P]);                                        \
    if (O.isFailing(r)) {                                                   \
      O.fail();                                                             \
      return OZ_typeError(OZ_self, OZ_args, OZ_arity, expectedType, P, ""); \
    } else if (O.isSuspending(r))                                           \
      SC += 1;                                                              \
  }

#define EXPECT_SAMELENGTH_VECTORS(I, J)                                 \
if (OZ_width(OZ_args[I]) != OZ_width(OZ_args[J]))                       \
     return OZ_typeError(OZ_self, OZ_args, OZ_arity, expectedType, J,   \
                         "Vectors must have same size.");

//-----------------------------------------------------------------------------
// OZ_FiniteDomain

enum OZ_FDPropState {fd_det = 0, fd_bounds, fd_any};
enum OZ_FDState {fd_empty, fd_full, fd_bool, fd_singleton};

class OZ_FiniteDomain {
protected:
  int min_elem, max_elem, size;
  void * descr;
public:
  void FiniteDomainInit(void * d);

  OZ_FiniteDomain(void) : descr((void *) 0) {}
  OZ_FiniteDomain(void * d);
  OZ_FiniteDomain(OZ_FDState state);
  OZ_FiniteDomain(const OZ_FiniteDomain &);
  OZ_FiniteDomain(OZ_Term);

  int constrainBool(void);
  void copyExtension(void);
  void dispose(void);
  OZ_Term getAsList(void) const;

  int init(int, int);
  int initSingleton(int);
  int init(OZ_Term);
  int initFull(void);
  int initEmpty(void);
  int initBool(void);

  OZ_Boolean isIn(int i) const;
  int next(int i) const;
  int nextBiggerElem(int v) const;

  OZ_Boolean operator != (const OZ_FDState) const;
  OZ_Boolean operator != (const int) const;
  OZ_FiniteDomain operator & (const OZ_FiniteDomain &) const;
  int operator &= (const OZ_FiniteDomain &);
  int operator &= (const int);
  int operator += (const int);
  int operator -= (const int);
  int operator -= (const OZ_FiniteDomain &);
  int operator <= (const int);
  const OZ_FiniteDomain &operator = (const OZ_FiniteDomain &fd);
  OZ_Boolean operator == (const OZ_FDState) const;
  OZ_Boolean operator == (const int) const;
  int operator >= (const int);
  OZ_FiniteDomain operator | (const OZ_FiniteDomain &) const;
  OZ_FiniteDomain operator ~ (void) const;

  ostream &print(ostream &) const;
  int singl(void) const;

  int getSize(void) const { return size; }
  int minElem(void) const { return min_elem; }
  int maxElem(void) const { return max_elem; }
};


inline
ostream &operator << (ostream &ofile, const OZ_FiniteDomain &fd) {
  return fd.print(ofile);
}

//-----------------------------------------------------------------------------
// class Propagator

// virtual base class; never create an object of this class
class OZ_Propagator {
public:
  OZ_Propagator(void) {}
  virtual ~OZ_Propagator(void) {}

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

  OZ_Propagator * gc(void);

  virtual size_t sizeOf(void) = 0;
  virtual void gcRecurse(void) = 0;
  virtual OZ_Return run(void) = 0;
  virtual ostream& print(ostream&) const = 0;

  OZ_Boolean isEqualVarsPossible(void);
  OZ_Return replaceBy(OZ_Propagator *);
  OZ_Return replaceBy(OZ_Term, OZ_Term);
  OZ_Return replaceByInt(OZ_Term, int);
  OZ_Return postpone(void);
};

ostream& operator << (ostream& o, const OZ_Propagator &p);

//-----------------------------------------------------------------------------
// class OZ_PropagatorExpect, etc.

struct OZ_expect_t {
  int size, accepted;
  OZ_expect_t(int s, int a) : size(s), accepted(a) {}
};

enum OZ_PropagatorFlags {
   NULL_flag,
   OFS_flag
};

class OZ_PropagatorExpect {
private:
  struct spawnVars_t {OZ_Term * var; OZ_FDPropState state;} * spawnVars;
  OZ_Term ** suspendVars;
  int spawnVarsNumber,  suspendVarsNumber;
protected:
  void addSpawn(OZ_FDPropState, OZ_Term *);
  void addSuspend(OZ_Term *);
public:
  OZ_PropagatorExpect(void);
  ~OZ_PropagatorExpect(void);

  OZ_Return spawn(OZ_Propagator *, int prio = OZ_getMaxPrio(),
                  OZ_PropagatorFlags flags=NULL_flag);
  OZ_Return spawn(OZ_Propagator * p, OZ_PropagatorFlags flags) {
      return spawn(p, OZ_getMaxPrio(), flags);
  }
  OZ_Return suspend(OZ_Thread);
  OZ_Return fail(void);

  OZ_expect_t expectDomainDescription(OZ_Term, int = 4);
  OZ_expect_t expectVar(OZ_Term t);
  OZ_expect_t expectRecordVar(OZ_Term);
  OZ_expect_t expectIntVar(OZ_Term, OZ_FDPropState);
  OZ_expect_t expectIntVarAny(OZ_Term t) {return expectIntVar(t, fd_any);}
  OZ_expect_t expectInt(OZ_Term);
  OZ_expect_t expectTruthVar(OZ_Term);
  OZ_expect_t expectTuple(OZ_Term,
                          OZ_expect_t (OZ_PropagatorExpect::*) (OZ_Term));

  OZ_Boolean isSuspending(OZ_expect_t r) {
    return (r.accepted == 0 || (0 < r.accepted && r.accepted < r.size));
  }
  OZ_Boolean isFailing(OZ_expect_t r) {
    return (r.accepted == -1);
  }
};

//-----------------------------------------------------------------------------
// class OZ_FDIntVar, etc.

class OZ_VarState {
private:
  enum State_e {loc_e = 1, glob_e = 2, spec_e = 3} state;
public:
  OZ_Boolean isState(State_e s) {return s == state;}
  void setState(State_e s) {state = s;}
};

class OZ_FDIntVar : public OZ_VarState {
private:
  OZ_FiniteDomain dom;
  OZ_FiniteDomain * domPtr;
  OZ_Term var;
  OZ_Term * varPtr;
  int initial_size;
  enum Sort_e {sgl_e = 1, bool_e = 2, int_e  = 3} sort;

  OZ_Boolean tell(void);
public:
  OZ_FDIntVar(void) {}
  OZ_FDIntVar(OZ_Term v) { enter(v); }

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

  OZ_FiniteDomain &operator * (void) {return *domPtr;}
  OZ_FiniteDomain * operator -> (void) {return domPtr;}
  OZ_Boolean isTouched(void) {return initial_size > domPtr->getSize();}
  OZ_Boolean isSort(Sort_e s) {return s == sort;}
  void setSort(Sort_e s) {sort = s;}

  void enter(OZ_Term);
  void enterSpec(OZ_Term);
  OZ_Boolean leave(void) { return isSort(sgl_e) ? OZ_FALSE : tell(); }
  void fail(void);
};

class OZ_ToStream {
public:
  OZ_Term term;
  OZ_ToStream(OZ_Term t) : term(t) {};
};

ostream& operator << (ostream&, const OZ_ToStream &);
inline OZ_ToStream OZ_toStream(OZ_Term t) { return OZ_ToStream(t); }

//-----------------------------------------------------------------------------
// Miscellaneous

void OZ_gcTerm(OZ_Term &);

OZ_Boolean OZ_isPosSmallInt(OZ_Term val);

OZ_Term * OZ_allocOzTermsOnHeap(int);
int * OZ_allocCIntsOnHeap(int);
void OZ_deallocOzTermsFromHeap(OZ_Term *, int);
void OZ_deallocCIntsFromHeap(int *, int);

int * OZ_findEqualVars(int, OZ_Term *); // static return value
OZ_Boolean OZ_isEqualVars(OZ_Term, OZ_Term);

OZ_Return OZ_typeError(OZ_CFun, OZ_Term [], int, char *, int, char *);

int OZ_getFDInf(void);
int OZ_getFDSup(void);

#endif // __OZ_CPI_HH__
