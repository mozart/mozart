/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __MOZART_CPI_HH__
#define __MOZART_CPI_HH__

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "mozart.h"

#define BIGFSET

// loeckelt:
// this might not be necessary anymore:
#ifdef BIGFSET
#ifdef FSET_HIGH
#undef FSET_HIGH
#endif
#endif

//-----------------------------------------------------------------------------
// misc macros

#define OZ_FAIL OZ_FAILED
#define OZ_ENTAIL OZ_ENTAILED

#define OZ_EXPECTED_TYPE(S) char * expectedType = S


#define __OZ_EXPECT(O, V, T, F)                         \
  {                                                     \
    OZ_expect_t r = O.F(V);                             \
    if (O.isFailing(r)) {                               \
      O.fail();                                         \
      return OZ_typeErrorCPI(T, 0, "");                 \
    } else if (O.isSuspending(r) || O.isExceptional(r)) \
      return O.suspend();                               \
  }

#define _OZ_EXPECT(O, V, A, F)                          \
  {                                                     \
    OZ_expect_t r = O.F(V);                             \
    if (O.isFailing(r)) {                               \
      O.fail();                                         \
      return OZ_typeErrorCPI(expectedType, A, "");      \
    } else if (O.isSuspending(r) || O.isExceptional(r)) \
      return O.suspend();                               \
  }

#define OZ_EXPECT(O, A, F)                      \
  {                                             \
    OZ_Term P = OZ_in(A);                       \
    _OZ_EXPECT(O, P, A, F)                      \
  }


#define _OZ_EXPECT_SUSPEND(O, V, A, F, SC)              \
  {                                                     \
    OZ_expect_t r = O.F(V);                             \
    if (O.isFailing(r)) {                               \
      O.fail();                                         \
      return OZ_typeErrorCPI(expectedType, A, "");      \
    } else if (O.isSuspending(r)) {                     \
      SC += 1;                                          \
    } else if (O.isExceptional(r)) {                    \
      return O.suspend();                               \
    }                                                   \
  }

#define OZ_EXPECT_SUSPEND(O, A, F, SC)          \
  {                                             \
    OZ_Term P = OZ_in(A);                       \
    _OZ_EXPECT_SUSPEND(O, P, A, F, SC)          \
  }


#define _OZ_EM_FDINF    "0"
#define _OZ_EM_FDSUP    "134 217 726"
#define _OZ_EM_FSETINF  "0"
// loeckelt: change?
#define _OZ_EM_FSETSUP  "134 217 726"
#define _OZ_EM_INTMAX   "134 217 727"

#define OZ_EM_LIT       "literal"
#define OZ_EM_FLOAT     "float"
#define OZ_EM_INT       "integer in [~"_OZ_EM_INTMAX"\\,...\\,"_OZ_EM_INTMAX"]"
#define OZ_EM_FD        "finite domain integer in {"_OZ_EM_FDINF"\\,...\\,"_OZ_EM_FDSUP"}"
#define OZ_EM_FDBOOL    "boolean finite domain integer in {0,1}"
#define OZ_EM_FDDESCR   "description of a finite domain integer"
#define OZ_EM_FSETVAL   "finite set of integers"
#define OZ_EM_FSET      "finite set of integers constraint"
#define OZ_EM_FSETDESCR "description of a finite set of integers"
#define OZ_EM_VECT      "vector of "
#define OZ_EM_RECORD    "record of "
#define OZ_EM_TNAME     "truth name"
#define OZ_EM_STREAM    "stream"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Dynamically sized arrays for compilers which do not provide alloca


#ifdef __GNUC__
#define USE_GCCALLOCA
#else
#define USE_INTVAR_NEW
#endif


#ifdef DEBUG_INDICES

#include <stdlib.h>

template <class T>
class IndexCheckArray {
private:
  int _size;
  T * _array;
public:
  IndexCheckArray(int s)  {
    _size = s;
    _array = (T *) malloc(s * sizeof(T));
  }
  // kost@ : where is the destructor function?
  inline
  T &operator [](int i) {
    OZ_ASSERT(0 <= i && i < _size);
    return _array[i];
  }
  inline
  operator T*() { return _array; } // conversion operator

  inline
  T * getArray(void) { return _array; }
};

#define GET_ARRAY(A) (A).getArray()

/* index-checked dynamic arrays */
#define _DECL_DYN_ARRAY(Type,Var,Size) IndexCheckArray<Type> Var(Size)

#else

#define GET_ARRAY(A) (A)

/* gcc supports dynamic sized arrays */
#ifdef USE_GCCALLOCA

#define _DECL_DYN_ARRAY(Type,Var,Size) Type Var[Size]

#endif

/* literally copied from fdaux.hh */
#ifdef USE_INTVAR_NEW

inline void * operator new(size_t, void * p) { return p; }

#define _DECL_DYN_ARRAY(Type,Var,Size)                          \
Type * Var;                                                     \
{                                                               \
  int __sz = Size;                                              \
  void *__aux = OZ_FDIntVar::operator new(sizeof(Type) * __sz); \
  Var = new (__aux) Type[__sz];                                 \
}

#endif

#ifdef USE_ALLOCA

inline void * operator new(size_t, void * p) { return p; }

#include <malloc.h>

#define _DECL_DYN_ARRAY(Type,Var,Size)          \
Type * Var;                                     \
{                                               \
  int __sz = Size;                              \
 void *__aux = alloca(sizeof(Type) * __sz);     \
 Var = new (__aux) Type[__sz];                  \
}

#endif

#endif /* DEBUG_INDICES */

/* cannot handle sometimes arrays of size 0 correctly */
#define DECL_DYN_ARRAY(Type,Var,Size) \
_DECL_DYN_ARRAY(Type,Var,Size==0?1:Size)



//-----------------------------------------------------------------------------
// OZ_FiniteDomain

class ozdeclspec OZ_FSetValue;

enum OZ_FDState {fd_empty, fd_full, fd_bool, fd_singl};

class ozdeclspec OZ_FiniteDomain {
protected:
  int min_elem, max_elem, size;
  void * descr;

public:

  OZ_FiniteDomain(void);
  OZ_FiniteDomain(OZ_FDState state);
  OZ_FiniteDomain(const OZ_FiniteDomain &);
  OZ_FiniteDomain(OZ_Term);
  OZ_FiniteDomain(const OZ_FSetValue &);

  int initRange(int, int);
  int initSingleton(int);
  int initDescr(OZ_Term);
  int initFull(void);
  int initEmpty(void);
  int initBool(void);

  int getMidElem(void) const;
  int getNextSmallerElem(int v) const;
  int getNextLargerElem(int v) const;
  int getLowerIntervalBd(int v) const;
  int getUpperIntervalBd(int v) const;
  int getSize(void) const;
  int getWidth(void) const;
  int getMinElem(void) const;
  int getMaxElem(void) const;
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
  int intersectWithBool(void);
  OZ_Boolean isIn(int i) const;
  void copyExtension(void);
  void disposeExtension(void);

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

#ifdef __GNUC__
  static void * operator new[](size_t);
  static void operator delete[](void *, size_t);
#endif

  char * toString(void) const;
};

inline OZ_FiniteDomain::OZ_FiniteDomain(void) : descr((void *) 0) {}
inline int OZ_FiniteDomain::getSize(void) const { return size; }
inline int OZ_FiniteDomain::getWidth(void) const { return max_elem-min_elem; }
inline int OZ_FiniteDomain::getMinElem(void) const { return min_elem; }
inline int OZ_FiniteDomain::getMaxElem(void) const { return max_elem; }

//-----------------------------------------------------------------------------
// OZ_FSetValue

enum OZ_FSetState {fs_empty, fs_full};


#ifdef BIGFSET
const int fs_sup = 134217726;
const int fsethigh32 = 134217727;
const int fset_high = 2;
#else

const int fset_high = 2;
const int fs_sup = 32*fset_high - 1;
//const int fset_high = 220;
const int fsethigh32 = 32*fset_high;
#endif

const int fs_max_card = fs_sup + 1;

class ozdeclspec OZ_FSetConstraint;
class OZ_FiniteDomainImpl;
class FSetValue;

#if defined(__GNUC__) && (defined(__CYGWIN32__) || defined(__MINGW32__))
// This patch was contributed by Jan van der Vorst <j.v.d.vorst@chello.nl>
//
// Patch for win32 gcc 3.3 and probably 3.4
// The gcc bug results in not exporting member functions of classes that are
// refered to as a friend of another class.
//
// This patch results in an emulator.dll with which the Select package
// builds.
// NO OTHER TESTS HAVE BEEN PERFORMED !
// I DO NOT EVEN KNOW IF MOZART STILL RUNS AFTER THIS PATCH !
//
// This function calls all non-const functions. Somehow all const functions
// were not affected by the gcc bug.
// Limitations: I have not been able to make the 'new' and 'delete' operators
// to show up in the exports of emulator.def

inline static void
gcc_bug_11005(void)
{
  OZ_FDState fds;
  OZ_Term ozt;
  void * ozfsv;
  OZ_FiniteDomain d, d1(fds), d2(d), d3(ozt), d4(*((OZ_FSetValue*)ozfsv));

  //   OZ_FiniteDomain(void);
  //   OZ_FiniteDomain(OZ_FDState state);
  //   OZ_FiniteDomain(const OZ_FiniteDomain &);
  //   OZ_FiniteDomain(OZ_Term);
  //   OZ_FiniteDomain(const OZ_FSetValue &);

  d.initRange(0, 0);
  d.initSingleton(0);
  d.initDescr((OZ_Term)NULL);
  d.initFull();
  d.initEmpty();
  d.initBool();

  //   int getMidElem(void) const;
  //   int getNextSmallerElem(int v) const;
  //   int getNextLargerElem(int v) const;
  //   int getLowerIntervalBd(int v) const;
  //   int getUpperIntervalBd(int v) const;
  //   int getSize(void) const;
  //   int getWidth(void) const;
  //   int getMinElem(void) const;
  //   int getMaxElem(void) const;
  //   int getSingleElem(void) const;
  //   OZ_Term getDescr(void) const;

  //  const OZ_FiniteDomain &operator = (const OZ_FiniteDomain &fd);
  d = d;
  //  OZ_Boolean operator == (const OZ_FDState) const;
  //   OZ_Boolean operator == (const int) const;
  //   OZ_Boolean operator != (const OZ_FDState) const;
  //   OZ_Boolean operator != (const int) const;
  //
  //   OZ_FiniteDomain operator & (const OZ_FiniteDomain &) const;
  //   OZ_FiniteDomain operator | (const OZ_FiniteDomain &) const;
  //   OZ_FiniteDomain operator ~ (void) const;

  //   int operator &= (const OZ_FiniteDomain &);
  d &= d;
  //   int operator &= (const int);
  d &= 0;
  //   int operator += (const int);
  d += 0;
  //   int operator -= (const int);
  d -= 0;
  //   int operator -= (const OZ_FiniteDomain &);
  d -= d;
  //   int operator <= (const int);
  d <= 0;
  //   int operator >= (const int);
  d >= 0;


  d.constrainBool();
  d.intersectWithBool();
  //   OZ_Boolean isIn(int i) const;
  d.copyExtension();
  d.disposeExtension();

  //   static void * operator new(size_t);
  //   static void operator delete(void *, size_t);
  //
  // #ifdef __GNUC__
  //   static void * operator new[](size_t);
  //   static void operator delete[](void *, size_t);
  // #endif

  //  char * toString(void) const;
}
#endif

class ozdeclspec OZ_FSetValue {

  friend class OZ_FiniteDomainImpl;
  friend class OZ_FiniteDomain;

protected:
  int _card;
#ifdef BIGFSET
  bool _other;
  OZ_FiniteDomain _IN;
  bool _normal;
#endif

  int _in[fset_high];

public:

  OZ_FSetValue(void);
  OZ_FSetValue(const OZ_FSetConstraint&);
  OZ_FSetValue(const OZ_Term);
  OZ_FSetValue(const OZ_FSetState);
  OZ_FSetValue(int, int);
  OZ_FSetValue(const OZ_FiniteDomain &);

  void init(const OZ_FSetState);

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

#ifdef __GNUC__
  static void * operator new[](size_t);
  static void operator delete[](void *, size_t);
#endif

  void copyExtension(void);
  void disposeExtension(void);

  int getCard(void) const;
  int getKnownNotIn(void) const;
  OZ_Boolean isIn(int) const;
  OZ_Boolean isNotIn(int) const;
  int getMinElem(void) const;
  int getMaxElem(void) const;
  int getNextLargerElem(int) const;
  int getNextSmallerElem(int) const;
  OZ_Term getKnownInList(void) const;
  OZ_Term getKnownNotInList(void) const;
  char * toString(void) const;

  // comparison
  OZ_Boolean operator == (const OZ_FSetValue &) const;
  OZ_Boolean operator <= (const OZ_FSetValue &) const;

  OZ_FSetValue operator & (const OZ_FSetValue &) const;
  OZ_FSetValue operator | (const OZ_FSetValue &) const;
  OZ_FSetValue operator - (const OZ_FSetValue &) const;
  OZ_FSetValue operator &= (const OZ_FSetValue &);
  OZ_FSetValue operator |= (const OZ_FSetValue &);
  OZ_FSetValue operator &= (const int);
  OZ_FSetValue operator += (const int);
  OZ_FSetValue operator -= (const int);
  OZ_FSetValue operator - (void) const;

  // these member functions support automatically generated set
  // propagators
  int card(void) const;
};

inline OZ_FSetValue::OZ_FSetValue(void) {}
inline int OZ_FSetValue::getCard(void) const { return _card; }
inline int OZ_FSetValue::getKnownNotIn(void) const {
  return fsethigh32 - _card;
}
inline int OZ_FSetValue::card(void) const {
  return getCard();
}


//-----------------------------------------------------------------------------
// OZ_FSetConstraint


enum OZ_FSetPropState {fs_prop_glb = 0, fs_prop_lub, fs_prop_val,
                       fs_prop_any, fs_prop_bounds};

class ozdeclspec OZ_FSetConstraint {
protected:
  int _card_min, _card_max;
  int _known_in, _known_not_in;

#ifdef BIGFSET
  bool _normal;
  bool _otherin;
  bool _otherout;
  OZ_FiniteDomain _IN;
  OZ_FiniteDomain _OUT;
#endif
  int _in[fset_high], _not_in[fset_high];


public:
  OZ_FSetConstraint(void);
  OZ_FSetConstraint(const OZ_FSetValue &);
  OZ_FSetConstraint(OZ_FSetState);

  OZ_FSetConstraint(const OZ_FSetConstraint &);
  OZ_FSetConstraint &operator = (const OZ_FSetConstraint &);

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

#ifdef __GNUC__
  static void * operator new[](size_t);
  static void operator delete[](void *, size_t);
#endif

  void copyExtension(void);
  void disposeExtension(void);

  int getKnownIn(void) const;
  int getKnownNotIn(void) const;
  int getUnknown(void) const;

  OZ_FSetValue getGlbSet(void) const;
  OZ_FSetValue getLubSet(void) const;
  OZ_FSetValue getUnknownSet(void) const;
  OZ_FSetValue getNotInSet(void) const;

  int getGlbCard(void) const;
  int getLubCard(void) const;
  int getNotInCard(void) const;
  int getUnknownCard(void) const;

  int getGlbMinElem(void) const;
  int getLubMinElem(void) const;
  int getNotInMinElem(void) const;
  int getUnknownMinElem(void) const;

  int getGlbMaxElem(void) const;
  int getLubMaxElem(void) const;
  int getNotInMaxElem(void) const;
  int getUnknownMaxElem(void) const;

  int getGlbNextSmallerElem(int) const;
  int getLubNextSmallerElem(int) const;
  int getNotInNextSmallerElem(int) const;
  int getUnknownNextSmallerElem(int) const;

  int getGlbNextLargerElem(int) const;
  int getLubNextLargerElem(int) const;
  int getNotInNextLargerElem(int) const;
  int getUnknownNextLargerElem(int) const;

  int getCardSize(void) const;
  int getCardMin(void) const;
  int getCardMax(void) const;

  OZ_Boolean putCard(int, int);
  OZ_Boolean isValue(void) const;

  void init(void);
  void init(const OZ_FSetValue &);
  void init(OZ_FSetState);

  OZ_Boolean isIn(int) const;
  OZ_Boolean isNotIn(int) const;
  OZ_Boolean isEmpty(void) const;
  OZ_Boolean isFull(void) const;
  OZ_Boolean isSubsumedBy(const OZ_FSetConstraint &) const;
  OZ_Term getKnownInList(void) const;
  OZ_Term getKnownNotInList(void) const;
  OZ_Term getUnknownList(void) const;
  OZ_Term getLubList(void) const;
  OZ_Term getCardTuple(void) const;
  OZ_FSetConstraint operator - (void) const;
  OZ_Boolean operator += (int);
  OZ_Boolean operator -= (int);
  OZ_Boolean operator <<= (const OZ_FSetConstraint &);
  OZ_Boolean operator % (const OZ_FSetConstraint &);
  OZ_FSetConstraint operator & (const OZ_FSetConstraint &) const;
  OZ_FSetConstraint operator | (const OZ_FSetConstraint &) const;
  OZ_FSetConstraint operator - (const OZ_FSetConstraint &) const;
  OZ_Boolean operator <= (const OZ_FSetConstraint &);
  OZ_Boolean operator >= (const OZ_FSetConstraint &);
  OZ_Boolean operator != (const OZ_FSetConstraint &);
  OZ_Boolean operator == (const OZ_FSetConstraint &) const;

  OZ_Boolean le(const int);
  OZ_Boolean ge(const int);

  // these member functions support automatically generated set
  // propagators: the following operators throw an execption of type
  // 'OZ_FSetConstraint' in case a failure occurs.
  OZ_Boolean operator <= (const int);
  OZ_Boolean operator >= (const int);
  OZ_Boolean operator |= (const OZ_FSetValue &);
  OZ_Boolean operator &= (const OZ_FSetValue &);
  int lbc(void) const;
  int ubc(void) const;
  OZ_FSetValue lb(void) const { return getGlbSet(); }
  OZ_FSetValue ub(void) const { return getLubSet(); }

  char * toString(void) const;
};

inline OZ_FSetConstraint::OZ_FSetConstraint(void) {}
inline int OZ_FSetConstraint::getKnownIn(void) const { return _known_in; }
inline int OZ_FSetConstraint::getKnownNotIn(void) const {
  return _known_not_in;
}
inline int OZ_FSetConstraint::getUnknown(void) const {
#ifdef BIGFSET
  return fs_sup - _known_in - _known_not_in + 1;
#else
  return fsethigh32 - _known_in - _known_not_in;
#endif
}
inline int OZ_FSetConstraint::getCardSize(void) const {
  return _card_max - _card_min + 1;
}
inline int OZ_FSetConstraint::getCardMin(void) const { return _card_min; }
inline int OZ_FSetConstraint::getCardMax(void) const { return _card_max; }
inline int OZ_FSetConstraint::lbc(void) const { return getCardMin(); }
inline int OZ_FSetConstraint::ubc(void) const { return getCardMax(); };

//-----------------------------------------------------------------------------
// class OZ_Propagator

class ozdeclspec OZ_NonMonotonic {
public:
  typedef unsigned order_t;
private:
  order_t _order;
  static order_t _next_order;
public:
  OZ_NonMonotonic(void);
  order_t getOrder(void) const;
};

inline  OZ_NonMonotonic::order_t OZ_NonMonotonic::getOrder(void) const {
  return _order;
}

class ozdeclspec OZ_PropagatorProfile {
private:
  OZ_PropagatorProfile * _next;
  static OZ_PropagatorProfile * _all_headers;
  char * _propagator_name;
  unsigned _calls, _samples, _heap;

public:
  OZ_PropagatorProfile(void);

  OZ_PropagatorProfile(char * propagator_name);

  void operator = (char * propagator_name);

  char * getPropagatorName(void);
  void incSamples(void);
  void incCalls(void);
  unsigned getSamples(void);
  unsigned getCalls(void);
  void incHeap(unsigned inc);
  unsigned getHeap(void);

  static OZ_PropagatorProfile * getFirst();
  OZ_PropagatorProfile * getNext(void);

  static void profileReset(void);
};

inline
char * OZ_PropagatorProfile::getPropagatorName() {
  return _propagator_name;
}
inline
void OZ_PropagatorProfile::incSamples(void)         { _samples++; }
inline
void OZ_PropagatorProfile::incCalls(void)           { _calls++; }
inline
unsigned OZ_PropagatorProfile::getSamples(void)     { return _samples; }
inline
unsigned OZ_PropagatorProfile::getCalls(void)       { return _calls; }
inline
void OZ_PropagatorProfile::incHeap(unsigned inc)    { _heap += inc; }
inline
unsigned OZ_PropagatorProfile::getHeap(void)        { return _heap; }

inline OZ_PropagatorProfile * OZ_PropagatorProfile::getFirst(void) {
  return OZ_PropagatorProfile::_all_headers;
}
inline OZ_PropagatorProfile * OZ_PropagatorProfile::getNext(void) {
  return _next;
}

//-----------------------------------------------------------------------------
// OZ_CtWakeUp

// there are not more than 32 wake up lists
class ozdeclspec OZ_CtWakeUp {
private:
  unsigned _wakeUpDescriptor;
public:
  // don't define any constructor
  void init(void);
  OZ_Boolean isEmpty(void);
  OZ_Boolean setWakeUp(int i);
  OZ_Boolean setEvent(int i);
  OZ_Boolean isWakeUp(int i);
  OZ_Boolean isEvent(int i);
  static OZ_CtWakeUp getWakeUpAll(void);
};

#define OZ_WAKEUP_ALL OZ_CtWakeUp::getWakeUpAll()

inline
void OZ_CtWakeUp::init(void) { _wakeUpDescriptor = 0; }
inline
OZ_Boolean OZ_CtWakeUp::isEmpty(void) { return (_wakeUpDescriptor == 0); }
inline
OZ_Boolean OZ_CtWakeUp::setWakeUp(int i) { return (_wakeUpDescriptor |= (1 << i)); }
inline
OZ_Boolean OZ_CtWakeUp::setEvent(int i) {return setWakeUp(i); };
inline
OZ_Boolean OZ_CtWakeUp::isWakeUp(int i) { return (_wakeUpDescriptor & (1 << i)); }
inline
OZ_Boolean OZ_CtWakeUp::isEvent(int i)  { return isWakeUp(i); };
inline
OZ_CtWakeUp OZ_CtWakeUp::getWakeUpAll(void) {
  OZ_CtWakeUp aux;
  aux._wakeUpDescriptor = 0xffff;
  return aux;
};

typedef OZ_CtWakeUp OZ_CtEvents;

//----------------------------------------------------------------------

class ozdeclspec OZ_CtDefinition;

enum OZ_FDPropState {fd_prop_singl = 0, fd_prop_bounds, fd_prop_any};

// virtual base class; never create an object from this class
class ozdeclspec OZ_Propagator {
  friend class Propagator;
public:
  OZ_Propagator(void);
  virtual ~OZ_Propagator(void);

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

  OZ_Boolean mayBeEqualVars(void) {
    extern int __OZ_rp_isUnify;
    return __OZ_rp_isUnify;
  }
  OZ_Return replaceBy(OZ_Propagator *);
  OZ_Return replaceBy(OZ_Term, OZ_Term);
  OZ_Return replaceByInt(OZ_Term, int);
  OZ_Return postpone(void);
  OZ_Boolean imposeOn(OZ_Term);
  OZ_Boolean addImpose(OZ_FDPropState s, OZ_Term v);
  OZ_Boolean expectInt(OZ_Term v, int &r) {
    r = 0;
    return 0;
  }
  int expectIntVarBounds(OZ_Term v, int &r) {
    r = addImpose(fd_prop_bounds, v);
    return 0;
  }
  OZ_Boolean addImpose(OZ_FSetPropState s, OZ_Term v);
  OZ_Boolean addImpose(OZ_CtWakeUp e, OZ_CtDefinition * d, OZ_Term v);

  OZ_Boolean expectFSetVar(OZ_FSetPropState s, OZ_Term v) {
    return addImpose(s, v);
  }
  OZ_Boolean expectFDIntVar(OZ_FDPropState s, OZ_Term v) {
    return addImpose(s, v);
  }
  OZ_Boolean expectCtVar(OZ_CtWakeUp e, OZ_CtDefinition * d, OZ_Term v) {
    return addImpose(e, d, v);
  }
  int impose(OZ_Propagator *);
  virtual size_t sizeOf(void) = 0;
  virtual void sClone(void) = 0;
  virtual void gCollect(void) = 0;
  virtual OZ_Return propagate(void) = 0;
  virtual OZ_Term getParameters(void) const = 0;
  virtual OZ_PropagatorProfile * getProfile(void) const = 0;

  // support for nonmonotonic propagator
  virtual OZ_Boolean isMonotonic(void) const;
  virtual OZ_NonMonotonic::order_t getOrder(void) const;

  char * toString(void) const;
};

inline OZ_Propagator::OZ_Propagator(void) {}

//-----------------------------------------------------------------------------
// class OZ_CPIVar

class ozdeclspec OZ_CPIVar {
  //
  friend class OZ_Expect;
  friend void initCPI(void);
  //
private:
  //
  static int _first_run;
  static OZ_Term _vars_removed;
  static void add_vars_removed(OZ_Term *);
  static int is_in_vars_removed(OZ_Term *);
  static void reset_vars_removed(void);
  static void set_vars_removed(void);
  //
protected:
  //
  enum State_e {empty_e = 0x00,
                // state
                loc_e   = 0x01,
                glob_e  = 0x02,
                encap_e = 0x04,
                // sort
                val_e   = 0x08,
                var_e   = 0x10,
                sgl_e   = val_e,
                bool_e  = 0x40,
                int_e   = var_e,
                drop_e  = 0x100} _state;
  //
  OZ_Boolean isState(State_e s) const {
    return _state & s;
  }
  void setState(State_e s) {
    _state = State_e(_state | s);
    Assert(!(isState(glob_e) && isState(loc_e)));
  }
  OZ_Boolean isSort(State_e s) const {
    return _state & s;
  }
  void setSort(State_e s) {
    _state = State_e(_state | s);
    Assert(!(isSort(sgl_e) && isSort(bool_e) && isSort(int_e)));
    Assert(!(isSort(val_e) && isSort(var_e)));
  }
  //
  int _nb_refs;
  OZ_Term var, * varPtr;
  //
public:
  //
  OZ_CPIVar(void)
    : _state(empty_e), _nb_refs(0) {}
  void dropParameter(void);
  int is_dropped(void) {  return isState(drop_e); }
  int is_value(void) {  return isState(val_e); }

  static void * operator new(size_t);
  static void operator delete(void *, size_t);

#ifdef __GNUC__
  // mm2: portability ?
  static void * operator new[](size_t);
  static void operator delete[](void *, size_t);
#endif

  // conversion operator: OZ_CPIVar -> OZ_Term
  operator OZ_Term () const { return varPtr == NULL ? var : (OZ_Term) varPtr; }
  int operator == (OZ_CPIVar &v) const { return v.var == var; } // tmueller: new stuff
  int operator != (OZ_CPIVar &v) const { return v.var != var; } // tmueller: new stuff
};


//-----------------------------------------------------------------------------
// class OZ_FDIntVar

class ozdeclspec OZ_FDIntVar : public OZ_CPIVar {
  OZ_FiniteDomain _copy, _encap;
  OZ_FiniteDomain * _domain;
  int initial_size, initial_width;
  OZ_Boolean tell(void);
public:
  OZ_FDIntVar(void);
  OZ_FDIntVar(OZ_Term v);

  OZ_FiniteDomain &operator * (void);
  OZ_FiniteDomain * operator -> (void);

  OZ_Boolean isTouched(void) const;

  OZ_Boolean operator == (OZ_FDIntVar &v) {
    return (_domain == v._domain);
  }

  void ask(OZ_Term);
  int read(OZ_Term);
  int readEncap(OZ_Term);
  OZ_Boolean leave(void);
  void fail(void);
};


inline
OZ_FDIntVar::OZ_FDIntVar(void)
  : OZ_CPIVar() {}
inline
OZ_FDIntVar::OZ_FDIntVar(OZ_Term v)
  : OZ_CPIVar() {
  read(v);
}
inline
OZ_FiniteDomain &OZ_FDIntVar::operator * (void) { return *_domain; }
inline
OZ_FiniteDomain * OZ_FDIntVar::operator -> (void) { return _domain; }
inline
OZ_Boolean OZ_FDIntVar::isTouched(void) const {
  return initial_size > _domain->getSize();
}

inline
OZ_Boolean OZ_FDIntVar::leave(void) {
  return isSort(sgl_e) ? OZ_FALSE : tell();
}

//-----------------------------------------------------------------------------
// class OZ_FSetVar

class ozdeclspec OZ_FSetVar : public OZ_CPIVar {
private:
  OZ_FSetConstraint _copy, _encap;
  OZ_FSetConstraint * _set;
  int known_in, known_not_in, card_size;
  OZ_Boolean tell(void);
public:
  OZ_FSetVar(void);
  OZ_FSetVar(OZ_Term v);

  OZ_FSetConstraint &operator * (void);
  OZ_FSetConstraint * operator -> (void);

  OZ_Boolean isTouched(void) const;

  void ask(OZ_Term);
  void read(OZ_Term);
  void readEncap(OZ_Term);
  OZ_Boolean leave(void);
  void fail(void);
};

inline
OZ_FSetVar::OZ_FSetVar(void)
  : OZ_CPIVar() {}
inline
OZ_FSetVar::OZ_FSetVar(OZ_Term v)
  : OZ_CPIVar() {
  read(v);
}
inline
OZ_FSetConstraint &OZ_FSetVar::operator * (void) {return *_set;}
inline
OZ_FSetConstraint * OZ_FSetVar::operator -> (void) {return _set;}

inline
OZ_Boolean OZ_FSetVar::leave(void) {
  return isSort(val_e) ? OZ_FALSE : tell();
}

//-----------------------------------------------------------------------------
// class OZ_Stream

class ozdeclspec OZ_Stream {
private:
  OZ_Boolean closed, eostr, valid;
  OZ_Term tail;

  void setFlags(void);
public:
  OZ_Stream(OZ_Term st);
  OZ_Boolean isEostr(void);
  OZ_Boolean isClosed(void);
  OZ_Boolean isValid(void);

  OZ_Term get(void);
  OZ_Term getTail(void);
  OZ_Term put(OZ_Term, OZ_Term);

  OZ_Boolean leave(void);
  void fail(void);
};

inline
OZ_Stream::OZ_Stream(OZ_Term st) : tail(st) { setFlags(); }
inline
OZ_Boolean OZ_Stream::isEostr(void) { return eostr; }
inline
OZ_Boolean OZ_Stream::isClosed(void) { return closed; }
inline
OZ_Boolean OZ_Stream::isValid(void) { return valid; }
inline
OZ_Term OZ_Stream::getTail(void) { return tail; }

//-----------------------------------------------------------------------------
// Miscellaneous I

// Allocation
_FUNDECL(OZ_Term *,OZ_hallocOzTerms,(int));
_FUNDECL(int *,OZ_hallocCInts,(int));
_FUNDECL(char *,OZ_hallocChars,(int));

// Copying
inline
int * OZ_copyCInts(int n, int * frm) {
  if (n>0) {
    return (int *) memcpy(OZ_hallocCInts(n), frm, n*sizeof(int));
  } else {
    return ((int *) 0);
  }
}

_FUNDECL(char *,OZ_copyChars,(int, char *));

// Garbage collection
_FUNDECL(void,OZ_gCollectBlock,(OZ_Term *, OZ_Term *, int));
inline
void OZ_gCollectTerm(OZ_Term &t) {
  OZ_gCollectBlock(&t, &t, 1);
}
_FUNDECL(OZ_Term *,OZ_gCollectAllocBlock,(int, OZ_Term *));

// Cloning
_FUNDECL(void,OZ_sCloneBlock,(OZ_Term *, OZ_Term *, int));
inline
void OZ_sCloneTerm(OZ_Term &t) {
  OZ_sCloneBlock(&t, &t, 1);
}
_FUNDECL(OZ_Term *,OZ_sCloneAllocBlock,(int, OZ_Term *));


_FUNDECL(OZ_Boolean,OZ_isPosSmallInt,(OZ_Term val));

// Free

_FUNDECL(void,OZ_hfreeOzTerms,(OZ_Term *, int));
_FUNDECL(void,OZ_hfreeCInts,(int *, int));
_FUNDECL(void,OZ_hfreeChars,(char *, int));

_FUNDECL(OZ_Boolean,OZ_hasEqualVars,(int, OZ_Term *));
_FUNDECL(int *,OZ_findEqualVars,(int, OZ_Term *)); // static return value
_FUNDECL(int *,OZ_findSingletons,(int, OZ_Term *)); // static return value

#define __OZ_CPI_isRef(v)           !((v) & 3)
#define __OZ_CPI_tagged2Ref(v)      ((OZ_Term *) (v))
#define __OZ_CPI_unstag_ptr(c,t,st) ((c) (((OZ_Term)(t))-(st)))
#define __OZ_CPI_hasStag(t,st)      !(__OZ_CPI_unstag_ptr(int,t,st) & 7)
#define __OZ_CPI_isVar(t)           __OZ_CPI_hasStag(t,1)

inline
OZ_Boolean OZ_isEqualVars(OZ_Term v1, OZ_Term v2)
{
  OZ_Term * v1_ptr;
  if (__OZ_CPI_isRef(v1)) {
    do {
      v1_ptr = __OZ_CPI_tagged2Ref(v1);
      v1     = *v1_ptr;
    } while (__OZ_CPI_isRef(v1));
    if (!__OZ_CPI_isVar(v1))
      return OZ_FALSE;
    OZ_Term * v2_ptr;
    if (__OZ_CPI_isRef(v2)) {
      do {
        v2_ptr = __OZ_CPI_tagged2Ref(v2);
        v2     = *v2_ptr;
      } while (__OZ_CPI_isRef(v2));
      return v1_ptr == v2_ptr;
    }
  }
  return OZ_FALSE;
}

#undef __OZ_CPI_isRef
#undef __OZ_CPI_tagged2Ref
#undef __OZ_CPI_unstag_ptr
#undef __OZ_CPI_hasStag
#undef __OZ_CPI_isVar

_FUNDECL(OZ_Return,OZ_typeErrorCPI,(char *, int, char *));

#define OZ_getFDInf() (0)
#define OZ_getFDSup() (134217727-1)

_FUNDECL(int,OZ_getFSetInf,(void));
_FUNDECL(int,OZ_getFSetSup,(void));

_FUNDECL(int,OZ_vectorSize,(OZ_Term));

_FUNDECL(OZ_Term *,OZ_getOzTermVector,(OZ_Term, OZ_Term *));
_FUNDECL(int *,OZ_getCIntVector,(OZ_Term, int *));

_FUNDECL(OZ_Term,OZ_fsetValue,(OZ_FSetValue *));
_FUNDECL(OZ_FSetValue *,OZ_fsetValueToC,(OZ_Term));

//-----------------------------------------------------------------------------
// Interface to Generic Constraint Systems

class ozdeclspec OZ_Ct;

//-----------------------------------------------------------------------------
// OZ_CtDefinition

class ozdeclspec OZ_CtDefinition {
public:
  virtual int getId(void) = 0;
  virtual int getNoEvents(void) = 0;
  virtual char ** getEventNames(void) = 0;
  virtual char * getName(void) = 0;
  virtual OZ_Ct * fullDomain(void) = 0;
  virtual OZ_Boolean isValueOfDomain(OZ_Term) = 0;

};

typedef OZ_CtDefinition OZ_CtDescr;

/*
class ozdeclspec OZ_CtDefinition {
public:
  virtual int getKind(void) = 0;
  virtual int getNoOfWakeUpLists(void) = 0;
  virtual char ** getNamesOfWakeUpLists(void) = 0;
  virtual char * getName(void) = 0;
  virtual OZ_Ct * leastConstraint(void) = 0;
  virtual OZ_Boolean isValidValue(OZ_Term) = 0;

};
*/

//-----------------------------------------------------------------------------
// OZ_CtProfile

class ozdeclspec OZ_CtProfile {
public:
  OZ_CtProfile(void) {}
  virtual void init(OZ_Ct *) = 0;
};

//-----------------------------------------------------------------------------
// OZ_Ct

class ozdeclspec OZ_Ct {

public:
  OZ_Ct(void) {}

  virtual OZ_Boolean isWeakerThan(OZ_Ct *) = 0; // becomes redundant soon

  virtual OZ_Boolean isValue(void) = 0;
  virtual OZ_Term toValue(void) = 0;

  virtual OZ_Boolean isEmpty(void) = 0;
  virtual OZ_Boolean operator == (OZ_Ct *) = 0;

  virtual OZ_Ct * intersectDomains(OZ_Ct *) = 0;
  virtual OZ_Boolean isInDomain(OZ_Term) = 0;

  virtual OZ_CtProfile * getProfile(void) = 0;
  virtual OZ_CtEvents computeEvents(OZ_CtProfile *) = 0;
  virtual OZ_CtEvents computeEvents(OZ_Ct *) = 0;

  virtual char * toString(int) = 0;
  virtual OZ_Ct * copy(void) = 0;
  virtual size_t sizeOf(void) = 0;

  static void * operator new(size_t, int align = sizeof(void *));
  static void operator delete(void *, size_t);
};
/*
class ozdeclspec OZ_Ct {

public:
  OZ_Ct(void) {}
  virtual OZ_Boolean isValue(void) = 0;
  virtual OZ_Term toValue(void) = 0;
  virtual OZ_Boolean isValid(void) = 0;
  virtual OZ_Boolean isWeakerThan(OZ_Ct *) = 0;
  virtual OZ_Ct * unify(OZ_Ct *) = 0;
  virtual OZ_Boolean unify(OZ_Term) = 0;
  virtual size_t sizeOf(void) = 0;
  virtual OZ_CtProfile * getProfile(void) = 0;
  virtual OZ_CtWakeUp getWakeUpDescriptor(OZ_CtProfile *) = 0;
  virtual char * toString(int) = 0;
  virtual OZ_Ct * copy(void) = 0;

  static void * operator new(size_t, int align = sizeof(void *));
  static void operator delete(void *, size_t);
};
*/
//-----------------------------------------------------------------------------
// OZ_CtVar

class ozdeclspec OZ_CtVar : public OZ_CPIVar {
private:

  OZ_CtProfile * _profile; // necessary ?
  OZ_CtDefinition * _definition;
  OZ_Boolean tell(void);

  OZ_CtWakeUp ctGetWakeUpDescriptor(void);

protected:

  virtual void ctSetValue(OZ_Term) = 0;

  virtual OZ_Ct * ctRefConstraint(OZ_Ct *) = 0;
  virtual OZ_Ct * ctSaveConstraint(OZ_Ct *) = 0;
  virtual OZ_Ct * ctSaveEncapConstraint(OZ_Ct *) = 0;
  virtual void ctRestoreConstraint() = 0;
  virtual void ctSetConstraintProfile(void) = 0;
  virtual OZ_CtProfile * ctGetConstraintProfile(void) = 0;

  virtual OZ_Ct * ctGetConstraint(void) = 0;

public:

  OZ_CtVar(void);

  virtual OZ_Boolean isTouched(void) const = 0;

  void ask(OZ_Term);
  void read(OZ_Term);
  void readEncap(OZ_Term);
  OZ_Boolean leave(void);
  void fail(void);
};


inline
OZ_CtVar::OZ_CtVar(void)
  : OZ_CPIVar() {}

inline
OZ_CtWakeUp OZ_CtVar::ctGetWakeUpDescriptor(void) {
  return ctGetConstraint()->computeEvents(ctGetConstraintProfile());
}

inline
OZ_Boolean OZ_CtVar::leave(void) {
  return isSort(val_e) ? OZ_FALSE : tell();
}

//-----------------------------------------------------------------------------
// Miscellaneous II

_FUNDECL(OZ_Return,OZ_mkOZ_VAR_CT,(OZ_Term, OZ_Ct *, OZ_CtDefinition *));
_FUNDECL(OZ_Return,OZ_mkCtVar,(OZ_Term, OZ_Ct *, OZ_CtDefinition *));

//-----------------------------------------------------------------------------
// class OZ_Expect, etc.

class ozdeclspec OZ_expect_t {
public:
  int size, accepted;
  OZ_expect_t(int s, int a) : size(s), accepted(a) {}
};

class ozdeclspec OZ_Expect;

typedef OZ_expect_t (OZ_Expect::*OZ_ExpectMeth) (OZ_Term);

#define __E(ME, EM)                                     \
{                                                       \
  OZ_expect_t e = ME(v);                                \
  if (isFailing(e)) {                                   \
    fail();                                             \
    r = OZ_typeErrorCPI(EM, 0, "");                     \
    return 1;                                           \
  } else if (isSuspending(e) || isExceptional(e)) {     \
    r = suspend();                                      \
    return 2;                                           \
  }                                                     \
  return 0;                                             \
}

class ozdeclspec OZ_Expect {
private:
  OZ_Boolean collect;

  OZ_expect_t _expectFSetDescr(OZ_Term descr, int level);

protected: //tmueller: protected?
  void addSpawnBool(OZ_Term *);
  void addSpawn(OZ_FDPropState, OZ_Term *);
  void addSpawn(OZ_FSetPropState, OZ_Term *);
  void addSpawn(OZ_CtDefinition *, OZ_CtWakeUp, OZ_Term *);

  void addSuspend(OZ_Term *);
  void addSuspendBool(OZ_Term *);
  void addSuspend(OZ_FDPropState, OZ_Term *);
  void addSuspend(OZ_FSetPropState, OZ_Term *);
  void addSuspend(OZ_CtDefinition *, OZ_CtWakeUp, OZ_Term *);

public:
  OZ_Expect(void);
  ~OZ_Expect(void);

  void collectVarsOn(void);
  void collectVarsOff(void);

  OZ_Boolean isSuspending(OZ_expect_t r) {
    return (r.accepted == 0 || (0 < r.accepted && r.accepted < r.size));
  }
  OZ_Boolean isFailing(OZ_expect_t r) {
    return (r.accepted == -1);
  }
  OZ_Boolean isExceptional(OZ_expect_t r) {
    return (r.accepted == -2);
  }

  OZ_expect_t expectVector(OZ_Term, OZ_ExpectMeth);
  OZ_expect_t expectDomDescr(OZ_Term descr, int level = 4);
  OZ_expect_t expectSetDescr(OZ_Term descr, int level = 4) {
    return expectDomDescr(descr, level);
  }
  OZ_expect_t expectFSetDescr(OZ_Term descr, int level = 4);
  OZ_expect_t expectVar(OZ_Term t);
  OZ_expect_t expectRecordVar(OZ_Term);
  OZ_expect_t expectBoolVar(OZ_Term);
  OZ_expect_t expectIntVar(OZ_Term, OZ_FDPropState = fd_prop_any);
  OZ_expect_t expectIntVarMinMax(OZ_Term t) {
    return expectIntVar(t, fd_prop_bounds);
  }
  OZ_expect_t expectVectorIntVarMinMax(OZ_Term t) {
    return expectVector(t, &OZ_Expect::expectIntVarMinMax);
  }
  OZ_expect_t expectVectorInt(OZ_Term t) {
    return expectVector(t, &OZ_Expect::expectInt);
  }
  int expectBoolVar(OZ_Term v, OZ_Return &r)
    __E(expectBoolVar, OZ_EM_FDBOOL)
  int expectIntVarBounds(OZ_Term v, OZ_Return &r)
    __E(expectIntVarMinMax, OZ_EM_FD)
  int expectVectorIntVarBounds(OZ_Term v, OZ_Return &r)
    __E(expectVectorIntVarMinMax, OZ_EM_VECT OZ_EM_FD)
  int expectInt(OZ_Term v, OZ_Return &r)
    __E(expectInt, OZ_EM_FD)
  int expectVectorInt(OZ_Term v, OZ_Return &r)
    __E(expectVectorInt, OZ_EM_VECT OZ_EM_FD)
  OZ_expect_t expectFSetVar(OZ_Term, OZ_FSetPropState = fs_prop_any);
  OZ_expect_t expectInt(OZ_Term);
  OZ_expect_t expectFloat(OZ_Term);
  OZ_expect_t expectFSetValue(OZ_Term);
  OZ_expect_t expectLiteral(OZ_Term);
  OZ_expect_t expectLiteralOutOf(OZ_Term, OZ_Term *);
  OZ_expect_t expectProperRecord(OZ_Term, OZ_ExpectMeth);
  OZ_expect_t expectProperRecord(OZ_Term, OZ_Term *);
  OZ_expect_t expectProperTuple(OZ_Term, OZ_ExpectMeth);
  OZ_expect_t expectList(OZ_Term, OZ_ExpectMeth);
  OZ_expect_t expectStream(OZ_Term st);
  OZ_expect_t expectCtVar(OZ_Term x, OZ_CtDefinition * d, OZ_CtWakeUp e) {
    return expectGenCtVar(x, d, e);
  }
  OZ_expect_t expectGenCtVar(OZ_Term, OZ_CtDefinition *, OZ_CtWakeUp);

  OZ_Return impose(OZ_Propagator * p);
  OZ_Return suspend(void);
  OZ_Return fail(void);
};

typedef OZ_Expect OZ_CreateProp;

template <class RTYPE>
class _OZ_ParamIterator {
public:
  virtual RTYPE leave(int vars_left = 0) = 0;
  virtual RTYPE fail(void) = 0;
  virtual RTYPE vanish(void) = 0;
};

typedef _OZ_ParamIterator<OZ_Return> OZ_ParamIterator;

//#define FILTER_DEBUG

#ifdef FILTER_DEBUG

#define DSP(A) printf A

#else

#define DSP(A)

#endif

//-----------------------------------------------------------------------------

#define FailOnInvalidTouched(R, V, E)           \
{                                               \
  FSetTouched __t = V;                          \
  if(!(E)) goto failure;                        \
  R |= (__t <= V);                              \
}

//-----------------------------------------------------------------------------

OZ_CPIVar * _getCPIVar(OZ_Term v);
#define getCPIVar(T,V) ((T) _getCPIVar(V))

// a CPIVector can be compressed, i.e., it can drop `dropped'
// parameters by the methods `compress(void)' and `compress(int *)'
template <class T>
class CPIVector {
private:
  T * _vector;
  T ** _vector_elems;
  int &_size, _init_size;
  OZ_Term ** _ot_vector;
public:
  CPIVector(int &size, T * vector, OZ_Term ** ot_vector)
    : _size(size), _init_size(size),
    _vector(vector), _ot_vector(ot_vector) {
    _vector_elems = (T **) OZ_CPIVar::operator new(sizeof(T*) * _size);
    for (int i = _init_size; i--; )
      _vector_elems[i] = &_vector[i];
  }
  T & operator [] (int i) { return *_vector_elems[i]; }
  int getHigh(void) { return _size; }
  OZ_Term getOzTermVector(void) {
    OZ_Term r = OZ_nil();
    for (int i  = _size; i--; ) {
      if (! _vector[i].is_dropped()) {
        r = OZ_cons((*_ot_vector)[i], r);
      }
    }
    return r;
  }
  void find_equals(int * pa) {
    for (int i = 0; i < _size; i += 1) {
      if (_vector[i].is_value()) {
        pa[i] = -1;
      } else {
        pa[i] = getCPIVar(T*, _vector[i]) - _vector;
      }
    }
  }
  int * find_equals(void) { // obsolete
    return OZ_findEqualVars(_size, *_ot_vector);
  }
  int compress(int * a) {
    int t = 0;
    for (int f = 0; f < _size; f += 1) {
      if (!_vector_elems[f]->is_dropped()) {
        _vector_elems[t] = _vector_elems[f];
        (*_ot_vector)[t] = (*_ot_vector)[f];
        if (a) {
          a[t] = a[f];
        }
        t += 1;
      }
    }
    return _size = t;
  }
  int compress(void) {
     return compress((int *) NULL);
  }
};

typedef CPIVector<OZ_FSetVar> OZ_FSetVarVector;
typedef CPIVector<OZ_FDIntVar> OZ_FDIntVarVector;
typedef CPIVector<OZ_CtVar> OZ_CtVarVector;

//-----------------------------------------------------------------------------
// OZ_PersistentFilter

class OZ_PersistentFilter {
public:
  virtual void sClone(void) = 0;
  virtual void gCollect(void) = 0;
};

//-----------------------------------------------------------------------------
// OZ_Filter

#include <stdarg.h>

typedef OZ_Return (*make_prop_fn_2)(OZ_Term, OZ_Term);
typedef OZ_Return (*make_prop_fn_3)(OZ_Term, OZ_Term, OZ_Term);
typedef OZ_Return (*make_prop_fn_4)(OZ_Term, OZ_Term, OZ_Term, OZ_Term);


/*
 * This is not as the Microsoft Visual C++ compiler
 * is buggy in that it mistakes this inside a class
 * as something being pure virtual...
 *
 */
const int OZ_Filter_max_actions = 10;

template <class PROPAGATOR>
class OZ_Filter {
private:
  int _closed;
  PROPAGATOR * _prop;
  OZ_ParamIterator * _iter;
  struct _actions_t {
    enum {
      _serv_failed = 0,
      _serv_entailed,
      _serv_leave,
      _serv_replace,
      _serv_equate} _what;
    union _action_params_t {
      int _vars_left;
      PROPAGATOR * _replacement;
      struct { OZ_Term _x, _y; } _equat;
    } _action_params;
  } _actions[OZ_Filter_max_actions];
  int _nb_actions;
  OZ_Return update_return(OZ_Return  o, OZ_Return n) {
    if (o == OZ_ENTAILED) {
      return n;
    } else if (o == OZ_SLEEP) {
      return (n == OZ_FAILED ? n : o);
    } else if (o == OZ_FAILED) {
      return o;
    } else {
      Assert(0);
      return o;
    }
  }
public:  //
  OZ_Filter(PROPAGATOR * prop, OZ_ParamIterator * iter)
    : _closed(0), _prop(prop), _iter(iter), _nb_actions(0) {}
  //
  // sleep is default, after one of these operations, the object is
  // closed
  OZ_Filter &leave(int vars_left = 0) {
    DSP(("request leave\n"));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_leave;
      _actions[_nb_actions]._action_params._vars_left = vars_left;
      _nb_actions += 1;
      Assert(_nb_actions <= OZ_Filter_max_actions);
    }
    _closed = 1;
    return *this;
  }
  OZ_Filter &entail(void) {
    DSP(("request entail\n"));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_entailed;
      _nb_actions += 1;
      Assert(_nb_actions <= OZ_Filter_max_actions);
    }
    _closed = 1;
    return *this;
  }
  OZ_Filter &fail(void) {
    DSP(("request fail\n"));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_failed;
      _nb_actions += 1;
      Assert(_nb_actions <= OZ_Filter_max_actions);
    }
    _closed = 1;
    return *this;
  }
  OZ_Filter &equate(OZ_Term x, OZ_Term y) {
    DSP(("request equate %x %x\n", x, y));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_equate;
      _actions[_nb_actions]._action_params._equat._x = x;
      _actions[_nb_actions]._action_params._equat._y = y;
      _nb_actions += 1;
      Assert(_nb_actions <= OZ_Filter_max_actions);
    }
    _closed = 1;
    return *this;
  }
  OZ_Filter &add_parameter(OZ_CPIVar &, int event) {
    Assert(0);
    return *this;
  }
  OZ_Filter &drop_parameter(OZ_CPIVar &) {
    Assert(0);
    return *this;
  }
  // propagator is set `scheduled'
  OZ_Filter &impose_propagator(make_prop_fn_2,
                                OZ_Term, OZ_Term);
  OZ_Filter &impose_propagator(make_prop_fn_3,
                                OZ_Term, OZ_Term, OZ_Term);
  OZ_Filter &impose_propagator(make_prop_fn_4,
                                OZ_Term, OZ_Term, OZ_Term, OZ_Term);
  // replacing a propagator by another one happens frequently, hence a
  // dedicated fucntion is introduced, not that ununsed parameters
  // have to be passed as arguments and the replacment react on the
  // same events at the respective parameters
  OZ_Filter &replace_propagator(PROPAGATOR * prop )
  {
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_replace;
      _actions[_nb_actions]._action_params._replacement = prop;
      _nb_actions += 1;
    }
    //
    _closed = 1;
    return *this;
  }
  // changes state of propagator, propagator shall
  // not be set `scheduled' (hence, `impose_propagator' does not work)
  void condens_vector(OZ_FDIntVarVector &);
  void condens_vector(OZ_FSetVarVector &);
  void condens_vector(OZ_CtVarVector &);
  OZ_Return operator ()() {
    DSP(("OZ_Filter %d\n", _nb_actions));
    OZ_Return r = OZ_ENTAILED;
    int do_leave = 1;
    //
    for (int i = 0; i < _nb_actions; i += 1) {
      typename _actions_t::_action_params_t &a = _actions[i]._action_params;
      //
      if (r == OZ_FAILED) {
        break;
      }
      switch (_actions[i]._what) {
      case _actions_t::_serv_failed:
        DSP(("\tfailed\n"));
        do_leave = 0;
        r = _iter->fail();
        break;
      case _actions_t::_serv_leave:
        DSP(("\tleave\n"));
        do_leave = 0;
        r = _iter->leave(a._vars_left);
        break;
      case _actions_t::_serv_entailed:
        DSP(("\tentailed\n"));
        do_leave = 0;
        r = _iter->vanish();
        break;
      case _actions_t::_serv_replace:
        DSP(("\tleave\n"));
        do_leave = 0;
        _iter->vanish();
        r = _prop->replaceBy(a._replacement);
        break;
      case _actions_t::_serv_equate:
        DSP(("\tequate\n"));
        do_leave = 0;
        _iter->vanish();
        DSP(("request equate %x %x\n", a._equat._x, a._equat._y));
        r = _prop->replaceBy(a._equat._x, a._equat._y);
        break;
      default:
        Assert(0);
      }
    }
    if (do_leave) {
      r = _iter->leave();
    }
    return r;
  }
  PROPAGATOR &operator *(void) { return *_prop; }
};

#endif // __MOZART_CPI_HH__
//
//-----------------------------------------------------------------------------
