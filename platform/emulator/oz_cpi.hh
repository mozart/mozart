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

#define OZ_EXPECTED_TYPE(S) char * expectedType = S

#define OZ_EXPECT(O, P, F)                                                  \
  {                                                                         \
    OZ_expect_t r = O.F(OZ_args[P]);                                        \
    if (O.isFailing(r)) {                                                   \
      O.fail();                                                             \
      return OZ_typeError(OZ_self, OZ_args, OZ_arity, expectedType, P, ""); \
    } else if (O.isSuspending(r))                                           \
      return O.suspend(OZ_makeSuspendedThread(OZ_self,OZ_args,OZ_arity));   \
  }

#define OZ_EXPECT_SUSPEND(O, P, F, SC)                                      \
  {                                                                         \
    OZ_expect_t r = O.F(OZ_args[P]);                                        \
    if (O.isFailing(r)) {                                                   \
      O.fail();                                                             \
      return OZ_typeError(OZ_self, OZ_args, OZ_arity, expectedType, P, ""); \
    } else if (O.isSuspending(r))                                           \
      SC += 1;                                                              \
  }

#define OZ_EM_LIT     "literal"
#define OZ_EM_INT     "integer"
#define OZ_EM_FD      "finite domain integer"
#define OZ_EM_FDDESCR "finite domain integer description"
#define OZ_EM_VECT    "vector of "
#define OZ_EM_TNAME   "truth name"

//-----------------------------------------------------------------------------
// OZ_FiniteDomain

enum OZ_FDPropState {fd_det = 0, fd_bounds, fd_any};
enum OZ_FDState {fd_empty, fd_full, fd_bool, fd_singleton};

class OZ_FiniteDomain {
protected:
  int min_elem, max_elem, size;
  void * descr;
public:

  OZ_FiniteDomain(void) : descr((void *) 0) {}
  OZ_FiniteDomain(OZ_FDState state);
  OZ_FiniteDomain(const OZ_FiniteDomain &);
  OZ_FiniteDomain(OZ_Term);

  int initRange(int, int);
  int initSingleton(int);
  int initDescr(OZ_Term);
  int initFull(void);
  int initEmpty(void);
  int initBool(void);

  int getMidElem(void) const;
  int getNextSmallerEl(int v) const;
  int getNextLargerEl(int v) const;
  int getSize(void) const { return size; }
  int getMinElem(void) const { return min_elem; }
  int getMaxElem(void) const { return max_elem; }
  int getSingleElem(void) const;
  OZ_Term getDescr(void) const;

  const OZ_FiniteDomain &operator = (const OZ_FiniteDomain &fd);
  OZ_Boolean operator == (const OZ_FDState) const;
  OZ_Boolean operator == (const int) const;
  OZ_Boolean operator != (const OZ_FDState) const;
  OZ_Boolean operator != (const int) const;

  OZ_FiniteDomain operator & (const OZ_FiniteDomain &) const;
  OZ_FiniteDomain operator | (const OZ_FiniteDomain &) const;
  OZ_FiniteDomain operator ~ (void) const;

  int operator &= (const OZ_FiniteDomain &);
  int operator &= (const int);
  int operator += (const int);
  int operator -= (const int);
  int operator -= (const OZ_FiniteDomain &);
  int operator <= (const int);
  int operator >= (const int);

  int constrainBool(void);
  OZ_Boolean isIn(int i) const;
  ostream &print(ostream &) const;
  void copyExtension(void);
  void dispose(void);
};


inline
ostream &operator << (ostream &ofile, const OZ_FiniteDomain &fd) {
  return fd.print(ofile);
}

//-----------------------------------------------------------------------------
// class OZ_Propagator

// virtual base class; never create an object from this class
class OZ_Propagator {
public:
  OZ_Propagator(void) {}
  virtual ~OZ_Propagator(void) {}

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

  OZ_Propagator * gc(void);
  OZ_Boolean mayBeEqualVars(void);
  OZ_Return replaceBy(OZ_Propagator *);
  OZ_Return replaceBy(OZ_Term, OZ_Term);
  OZ_Return replaceByInt(OZ_Term, int);
  OZ_Return postpone(void);
  OZ_Boolean postOn(OZ_Term);

  virtual size_t sizeOf(void) = 0;
  virtual void gcRecurse(void) = 0;
  virtual OZ_Return run(void) = 0;
  virtual OZ_Term getArguments(void) const = 0;
  virtual OZ_CFun getSpawner(void) const = 0;
};

ostream& operator << (ostream& o, const OZ_Propagator &p);

//-----------------------------------------------------------------------------
// class OZ_Expect, etc.

struct OZ_expect_t {
  int size, accepted;
  OZ_expect_t(int s, int a) : size(s), accepted(a) {}
};

enum OZ_PropagatorFlags {NULL_flag, OFS_flag};

class OZ_Expect;

typedef OZ_expect_t (OZ_Expect::*OZ_ExpectMeth) (OZ_Term);

class OZ_Expect {
public:
  struct spawnVars_t {OZ_Term * var; OZ_FDPropState state;} * spawnVars;
private:
  OZ_Term ** suspendVars;
  int spawnVarsNumber,  suspendVarsNumber;
protected:
  void addSpawn(OZ_FDPropState, OZ_Term *);
  void addSuspend(OZ_Term *);
public:
  OZ_Expect(void);
  ~OZ_Expect(void);

  OZ_expect_t expectDomDescr(OZ_Term, int = 4);
  OZ_expect_t expectVar(OZ_Term t);
  OZ_expect_t expectRecordVar(OZ_Term);
  OZ_expect_t expectIntVar(OZ_Term, OZ_FDPropState);
  OZ_expect_t expectIntVarAny(OZ_Term t) { return expectIntVar(t, fd_any); }
  OZ_expect_t expectInt(OZ_Term);
  OZ_expect_t expectLiteral(OZ_Term);
  OZ_expect_t expectVector(OZ_Term, OZ_ExpectMeth);

  OZ_Return spawn(OZ_Propagator *, int prio = OZ_getPropagatorPrio(),
                  OZ_PropagatorFlags flags=NULL_flag);
  OZ_Return spawn(OZ_Propagator * p, OZ_PropagatorFlags flags) {
      return spawn(p, OZ_getPropagatorPrio(), flags);
  }
  OZ_Return suspend(OZ_Thread);
  OZ_Return fail(void);
  OZ_Boolean isSuspending(OZ_expect_t r) {
    return (r.accepted == 0 || (0 < r.accepted && r.accepted < r.size));
  }
  OZ_Boolean isFailing(OZ_expect_t r) {
    return (r.accepted == -1);
  }
};


//-----------------------------------------------------------------------------
// class OZ_FDIntVar

class OZ_FDIntVar {
private:
  OZ_FiniteDomain dom;
  OZ_FiniteDomain * domPtr;
  OZ_Term var;
  OZ_Term * varPtr;
  int initial_size;
  enum Sort_e {sgl_e = 1, bool_e = 2, int_e  = 3} sort;
  enum State_e {loc_e = 1, glob_e = 2, spec_e = 3} state;

  OZ_Boolean tell(void);
public:
  OZ_FDIntVar(void) {}
  OZ_FDIntVar(OZ_Term v) { enter(v); }

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

#ifndef _MSC_VER
  // mm2: portability ?
  static void * operator new[](size_t);
  static void operator delete[](void *, size_t);
#endif

  OZ_FiniteDomain &operator * (void) {return *domPtr;}
  OZ_FiniteDomain * operator -> (void) {return domPtr;}

  OZ_Boolean isTouched(void) const {return initial_size > domPtr->getSize();}
  OZ_Boolean isSort(Sort_e s) const {return s == sort;}
  void setSort(Sort_e s) {sort = s;}
  OZ_Boolean isState(State_e s) const {return s == state;}
  void setState(State_e s) {state = s;}

  void ask(OZ_Term);
  void enter(OZ_Term);
  void enterSpec(OZ_Term);
  OZ_Boolean leave(void) { return isSort(sgl_e) ? OZ_FALSE : tell(); }
  void fail(void);
};

//-----------------------------------------------------------------------------
// Miscellaneous

void OZ_gcTerm(OZ_Term &);

OZ_Boolean OZ_isPosSmallInt(OZ_Term val);

OZ_Term * OZ_hallocOzTerms(int);
int *     OZ_hallocCInts(int);
void      OZ_hfreeOzTerms(OZ_Term *, int);
void      OZ_hfreeCInts(int *, int);

int * OZ_findEqualVars(int, OZ_Term *); // static return value
OZ_Boolean OZ_isEqualVars(OZ_Term, OZ_Term);

OZ_Return OZ_typeError(OZ_CFun, OZ_Term [], int, char *, int, char *);

int OZ_getFDInf(void);
int OZ_getFDSup(void);

int OZ_vectorSize(OZ_Term);

#endif // __OZ_CPI_HH__
