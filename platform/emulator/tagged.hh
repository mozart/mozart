/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __TAGGEDH
#define __TAGGEDH

#ifdef INTERFACE
#pragma interface
#endif

#include <stdio.h>

#include "base.hh"
#include "mem.hh"
#include "gc.hh"

// ---------------------------------------------------------------------------
// ---------- TAGGED REF -----------------------------------------------------
// ---------------------------------------------------------------------------

// --- TaggedRef: Type Declaration

enum TypeOfTerm {
  REF              =  0,   // 0000
  REFTAG2          =  4,   // 0100
  REFTAG3          =  8,   // 1000
  REFTAG4          = 12,   // 1100

  UVAR             =  1,   // 0001
  UNUSED_VAR       =  9,   // 1001
  CVAR             =  5,   // 0101

  GCTAG            =  13,  // 1101    --> !!! oz_isVariable(GCTAG) = 1 !!!

  LTUPLE           =  2,   // 0010
  FSETVALUE        = 14,   // 1110
  SRECORD          =  3,   // 0011

  LITERAL          = 15,   // 1111

  OZCONST          = 10,   // 1010

  SMALLINT         =  6,   // 0110
  EXT              =  7,   // 0111
  OZFLOAT          = 11    // 1011
};

// ---------------------------------------------------------------------------
// --- TaggedRef: CLASS / BASIC ACCESS FUNCTIONS


#define tagSize 4
#define tagMask 0xF


// ------------------------------------------------------
// Basic macros
#define _tagTypeOf(ref)          ((TypeOfTerm)(ref&tagMask))
#define TaggedToPointer(t)       ((void*) (mallocBase|t))

#define lostPtrBits (tagSize-2)
#define _tagValueOf2(tag,ref)    TaggedToPointer(((ref)>>lostPtrBits) - ((tag)>>2))
#define _tagValueOf(ref)         TaggedToPointer(((ref)>>lostPtrBits)&~3)
#define _tagValueOfVerbatim(ref) ((void*)(((ref) >> lostPtrBits)&~3))
#define _makeTaggedRef2(tag,i)   ((i << lostPtrBits) | tag)

#define _makeTaggedRef2i(tag,ptr) _makeTaggedRef2(tag,(int32)ToInt32(ptr))

/* small ints are the only TaggedRefs that do not
 * contain a pointer in the value part */
#define _makeTaggedSmallInt(s) ((s << tagSize) | SMALLINT)

/* new tagging unused so far */
#define OLD_TAGGING

#ifdef OLD_TAGGING
#define _makeTaggedRef(s) ((TaggedRef) ToInt32(s))
#define _isRef(term)      ((term & 3) == 0)
#define _tagged2Ref(ref)  ((TaggedRef *) ToPointer(ref))
#else
#define _makeTaggedRef(s) _makeTaggedRef2i(REF,s)
#define _isRef(term)      (tagTypeOf(term)==REF)
#define _tagged2Ref(ref)  ((TaggedRef *) tagValueOf2(REF,ref))
#endif

// ------------------------------------------------------
// Debug macros for debugging outside of gc

#ifdef DEBUG_GC
#define GCDEBUG(X)                                      \
  if (!isCollecting && (_tagTypeOf(X)==GCTAG ) )        \
   OZ_error("GcTag unexpectedly found.");
#else
#define GCDEBUG(X)
#endif


#ifdef DEBUG_CHECK
inline
void *tagValueOf(TaggedRef ref)
{
  GCDEBUG(ref);
  return _tagValueOf(ref);
}
inline
void *tagValueOf2(TypeOfTerm tag, TaggedRef ref)
{
  GCDEBUG(ref);
  return _tagValueOf2(tag,ref);
}

inline
void *tagValueOfVerbatim(TaggedRef ref)
{
  GCDEBUG(ref);
  return _tagValueOfVerbatim(ref);
}

inline
TypeOfTerm tagTypeOf(TaggedRef ref)
{
  GCDEBUG(ref);
  return _tagTypeOf(ref);
}

inline
TaggedRef makeTaggedRef2i(TypeOfTerm tag, int32 i)
{
  Assert((i&3) == 0);
  return _makeTaggedRef2(tag,i);
}

inline
TaggedRef makeTaggedRef2p(TypeOfTerm tag, void *ptr)
{
  return _makeTaggedRef2i(tag,ptr);
}

#else

#define tagValueOf(ref)         _tagValueOf((TaggedRef)ref)
#define tagValueOf2(tag,ref)    _tagValueOf2(tag,(TaggedRef)ref)
#define tagValueOfVerbatim(ref) _tagValueOfVerbatim((TaggedRef)ref)
#define tagTypeOf(ref)          _tagTypeOf((TaggedRef)ref)
#define makeTaggedRef2i(tag,i)  _makeTaggedRef2(tag,i)
#define makeTaggedRef2p(tag,i)  _makeTaggedRef2i(tag,i)

#endif

// ---------------------------------------------------------------------------
// --- TaggedRef: CHECK_xx

#define CHECK_NONVAR(term) Assert(oz_isRef(term) || !oz_isVariable(term))
#define CHECK_ISVAR(term)  Assert(oz_isVariable(term))
#define CHECK_DEREF(term)  Assert(!oz_isRef(term) && !oz_isVariable(term))
#define CHECK_POINTER(s)   Assert(!(ToInt32(s) & 3))
#define CHECK_POINTER_N(s) Assert(s != NULL && !(ToInt32(s) & 3))
#define CHECK_STRPTR(s)    Assert(s != NULL)
#define CHECKTAG(Tag)      Assert(tagTypeOf(ref) == Tag)


// ---------------------------------------------------------------------------
// --- REF

// mm2: compile with CUSR="-DDEBUG_REF allows to visualize
//      tagged2Ref '*', makeTaggedRef 'm', and tagged2NonVar 'n'
//  can be removed if the new tagging scheme is used ...
#ifdef DEBUG_REF
#define NO_MACROS
extern int debugRef;
#define DebugRef(s) do { if (debugRef) { s ; } } while (0)
#else
#define DebugRef(s)
#endif

#ifdef DEBUG_DET
#define NO_MACROS
#endif

#ifdef NO_MACROS
inline
TaggedRef makeTaggedRef(TaggedRef *s)
{
  DebugRef(printf("m"));
  CHECK_POINTER_N(s);
  return _makeTaggedRef(s);
}

inline
Bool oz_isRef(TaggedRef term) {
  GCDEBUG(term);
  return _isRef(term);
}

inline
TaggedRef *tagged2Ref(TaggedRef ref)
{
  DebugRef(printf("*"));
  GCDEBUG(ref);
  Assert(oz_isRef(ref));
  return _tagged2Ref(ref);
}
#else

#define makeTaggedRef(s) _makeTaggedRef(s)
#define oz_isRef(t)      _isRef(t)
#define tagged2Ref(ref)  _tagged2Ref(ref)

#endif

// ---------------------------------------------------------------------------
// --- Variables

// ---------------------------------------------------------------------------
// Tests for variables and their semantics:
//                           tag
// unconstrained var         0001 (UVAR:1)  isUVar              oz_isVariable
// ???                       1001 (????:9)                      oz_isVariable
// constrained-susp. var     0101 (CVAR:5)           isCVar     oz_isVariable
// ---------------------------------------------------------------------------


// mm2: isCVar-> isVar
inline
Bool isCVar(TypeOfTerm tag) {
  return (tag == CVAR);
}

inline
Bool isCVar(TaggedRef term) {
  GCDEBUG(term);
  Assert(!oz_isRef(term));
  return isCVar(tagTypeOf(term));
}


/*
 * Optimized tests for some most often used types: no untagging needed
 * for type tests!
 * Use macros if not DEBUG_CHECK, since gcc creates awful code
 * for inline function version
 */

#define _oz_isVariable(val) (((TaggedRef) val&2)==0)       /* mask = 0010 */
#define _isUVar(val)        (((TaggedRef) val&14)==0)      /* mask = 1110 */

#ifdef DEBUG_CHECK

inline Bool isUVar(TypeOfTerm tag) { return _isUVar(tag);}

inline
Bool isUVar(TaggedRef term) {
  GCDEBUG(term);
  Assert(!oz_isRef(term));
  return _isUVar(term);
}

inline
Bool isVariableTag(TypeOfTerm tag) { return _oz_isVariable(tag); }

inline
Bool oz_isVariable(TaggedRef term) {
  GCDEBUG(term);
  Assert(!oz_isRef(term));
  return _oz_isVariable(term);
}
inline
TaggedRef makeTaggedUVar(Board *s)
{
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(UVAR,s);
}
inline
TaggedRef makeTaggedCVar(OzVariable *s) {
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(CVAR, s);
}
#else
#define isVariableTag(term)    _oz_isVariable(term)
#define oz_isVariable(term)    _oz_isVariable(term)
#define isUVar(term)           _isUVar(term)
#define makeTaggedUVar(s)      makeTaggedRef2p(UVAR,s)
#define makeTaggedCVar(s)      makeTaggedRef2p(CVAR,s)
#endif

// mm2: obsolete
inline
OzVariable *tagged2SVarPlus(TaggedRef ref) {
  GCDEBUG(ref);
  Assert(isCVar(ref));
  return (OzVariable *) tagValueOf(ref);
}

inline
OzVariable *tagged2CVar(TaggedRef ref) {
  GCDEBUG(ref);
  CHECKTAG(CVAR);
  return (OzVariable *) tagValueOf2(CVAR,ref);
}

inline
TaggedRef *newTaggedUVar(TaggedRef proto)
{
  TaggedRef *ref = (TaggedRef *) int32Malloc(sizeof(TaggedRef));
  *ref = proto;
  return ref;
}

inline
TaggedRef *newTaggedUVar(Board *c)
{
  return newTaggedUVar(makeTaggedUVar(c));
}

#define oz_newVar(bb)            makeTaggedRef(newTaggedUVar(bb))

inline
TaggedRef *newTaggedCVar(OzVariable *c) {
  TaggedRef *ref = (TaggedRef *) int32Malloc(sizeof(TaggedRef));
  *ref = makeTaggedCVar(c);
  return ref;
}

/* does not deref home pointer! */
inline
Board *tagged2VarHome(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(UVAR);
  return (Board *) tagValueOf2(UVAR,ref);
}

inline
OzVariable *oz_getVar(TaggedRef *v) {
  if (isUVar(*v)) {
    OzVariable *sv = oz_newSimpleVar(tagged2VarHome(*v));
    *v = makeTaggedCVar(sv);
    return sv;
  } else {
    return tagged2CVar(*v);
  }
}

// ---------------------------------------------------------------------------
// --- TaggedRef: BASIC TYPE TESTS

// if you want to test a term:
//   1. if you have the tag: is<Type>(tag)
//   2. if you have the term: oz_is<Type>(term)
//   3. if you need many tests:
//        switch(typeOf(term)) { ... }
//    or  switch(tag) { ... }


// REFs

/*
 * Optimized tests for some most often used types: no untagging needed
 * for type tests!
 * Use macros if not DEBUG_CHECK, since gcc creates awful code
 * for inline function version
 */
#define _isLTuple(val)      (((TaggedRef) val&13)==0)      /* mask = 1101 */

#ifdef DEBUG_CHECK

inline
Bool isLTupleTag(TypeOfTerm tag) { return _isLTuple(tag);}

inline
Bool oz_isLTuple(TaggedRef term) {
  GCDEBUG(term);
  Assert(!oz_isRef(term));
  return _isLTuple(term);
}

#else

#define isLTupleTag(term)      _isLTuple(term)
#define oz_isLTuple(term)      _isLTuple(term)

#endif


inline
Bool isFSetValueTag(TypeOfTerm tag) {
  return tag == FSETVALUE;
}

inline
Bool oz_isFSetValue(TaggedRef term) {
  GCDEBUG(term);
  return isFSetValueTag(tagTypeOf(term));
}

inline
Bool isLiteralTag(TypeOfTerm tag) {
  return tag == LITERAL;
}

inline
Bool oz_isLiteral(TaggedRef term) {
  GCDEBUG(term);
  return isLiteralTag(tagTypeOf(term));
}

inline
Bool isSRecordTag(TypeOfTerm tag) {
  return tag == SRECORD;
}

inline
Bool oz_isSRecord(TaggedRef term) {
  GCDEBUG(term);
  return isSRecordTag(tagTypeOf(term));
}

inline
Bool isFloatTag(TypeOfTerm tag) {
  return (tag == OZFLOAT);
}

inline
Bool oz_isFloat(TaggedRef term) {
  GCDEBUG(term);
  return isFloatTag(tagTypeOf(term));
}

inline
Bool isSmallIntTag(TypeOfTerm tag) {
  return (tag == SMALLINT);
}

inline
Bool oz_isSmallInt(TaggedRef term) {
  return isSmallIntTag(tagTypeOf(term));
}

inline
Bool isConstTag(TypeOfTerm tag) {
  return (tag == OZCONST);
}

inline
Bool oz_isConst(TaggedRef term) {
  GCDEBUG(term);
  return isConstTag(tagTypeOf(term));
}

// ---------------------------------------------------------------------------
// --- TaggedRef: create: makeTagged<Type>

// this function should be used, if tagged references are to be initialized
#ifdef DEBUG_CHECK
inline
TaggedRef makeTaggedNULL()
{
  return makeTaggedRef2p((TypeOfTerm)0,(void*)NULL);
}

inline
TaggedRef makeTaggedFSetValue(OZ_FSetValue * s)
{
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(FSETVALUE, s);
}

inline
TaggedRef makeTaggedLTuple(LTuple *s)
{
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(LTUPLE,s);
}

inline
TaggedRef makeTaggedSRecord(SRecord *s)
{
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(SRECORD,s);
}


inline
TaggedRef makeTaggedLiteral(Literal *s)
{
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(LITERAL,s);
}

inline
TaggedRef makeTaggedFloat(Float *s)
{
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(OZFLOAT,s);
}


inline
TaggedRef makeTaggedConst(ConstTerm *s)
{
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(OZCONST,s);
}

inline
TaggedRef makeTaggedTert(Tertiary *s)
{
  CHECK_POINTER_N(s);
  return makeTaggedRef2p(OZCONST,s);
}


inline
TaggedRef makeTaggedSmallInt(int32 s)
{
  return _makeTaggedSmallInt(s);
}

#else

#define makeTaggedNULL()       ((TaggedRef) 0)
#define makeTaggedFSetValue(s) makeTaggedRef2p(FSETVALUE,s)
#define makeTaggedLTuple(s)    makeTaggedRef2p(LTUPLE,s)
#define makeTaggedSRecord(s)   makeTaggedRef2p(SRECORD,s)
#define makeTaggedLiteral(s)   makeTaggedRef2p(LITERAL,s)
#define makeTaggedFloat(s)     makeTaggedRef2p(OZFLOAT,s)
#define makeTaggedConst(s)     makeTaggedRef2p(OZCONST,s)
#define makeTaggedTert(s)      makeTaggedRef2p(OZCONST,s)
#define makeTaggedSmallInt(s)  _makeTaggedSmallInt(s)

#endif


inline
TaggedRef makeTaggedMiscp(void *s)
{
  return makeTaggedRef2p((TypeOfTerm)0,s);
}

// getArg() and the like may never return variables
// NOTE: this must very efficient, because its heavily used!
#ifdef NO_MACRO
inline
TaggedRef tagged2NonVariable(TaggedRef *term)
{
  DebugRef(printf("n"));
  GCDEBUG(*term);
  TaggedRef ret = *term;
  if (_oz_isVariable(ret) && !oz_isRef(ret)) {
    ret = makeTaggedRef(term);
  }
  return ret;
}
#else
#define tagged2NonVariable(t) \
          ((_oz_isVariable(*(t)) && !oz_isRef(*(t))) ? makeTaggedRef(t) : *(t))
#endif

// ---------------------------------------------------------------------------
// --- TaggedRef: allocate on heap, an return a ref to it

inline
TaggedRef *newTaggedRef(TaggedRef *t)
{
  TaggedRef *ref = (TaggedRef *) int32Malloc(sizeof(TaggedRef));
  *ref = makeTaggedRef(t);
  return ref;
}

// ---------------------------------------------------------------------------
// --- TaggedRef: conversion: tagged2<Type>

inline
OZ_FSetValue *tagged2FSetValue(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(FSETVALUE);
  return (OZ_FSetValue *) tagValueOf2(FSETVALUE,ref);
}

inline
SRecord *tagged2SRecord(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(SRECORD);
  return (SRecord *) tagValueOf2(SRECORD,ref);
}

inline
LTuple *tagged2LTuple(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(LTUPLE);
  return (LTuple *) tagValueOf2(LTUPLE,ref);
}

inline
Literal *tagged2Literal(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(LITERAL);
  return (Literal *) tagValueOf2(LITERAL,ref);
}

inline
Float *tagged2Float(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(OZFLOAT);
  return (Float *) tagValueOf2(OZFLOAT,ref);
}

inline
ConstTerm *tagged2Const(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(OZCONST);
  return (ConstTerm *) tagValueOf2(OZCONST,ref);
}

inline
Tertiary *tagged2Tert(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(OZCONST);
  return (Tertiary*) tagValueOf2(OZCONST,ref);
}

// ---------------------------------------------------------------------------
// --- TaggedRef: DEREF

// DEREF - Philosophy:
//   Arguments are never deref'ed
// especially: builtins cannot immediately access the tag of their arguments,
//   but must first call DEREF


// DEREF MACRO:
//  - declares 'termPtr','tag' as local variables
//  - destructively changes 'term'
//  - 'tag' is the tag of the deref'ed 'term'
//  - 'termPtr' is the pointer to the last REF-Cell in a chain of REF's
//    and is NULL, if no deref step was necessary (needed for destructive
//    changes of variables)

// Usage:
// void test(TaggedRef a) {
//   DEREF(a,ptr,tag);
//   if (isLiteral(tag)) { ... }
//   if (oz_isVariable(tag) { *ptr = ... }
//   ....
// }


#define __DEREF(term, termPtr, tag)                     \
  ProfileCode(ozstat.lenDeref=0);                       \
  while(oz_isRef(term)) {                                       \
    COUNT(lenDeref);                                    \
    termPtr = tagged2Ref(term);                         \
    term = *termPtr;                                    \
  }                                                     \
  ProfileCode(ozstat.derefChain(ozstat.lenDeref));      \
  tag = tagTypeOf(term);

#define _DEREF(term, termPtr, tag)              \
  register TypeOfTerm tag;                      \
   __DEREF(term, termPtr, tag);


#define DEREF(term, termPtr, tag)               \
  register TaggedRef *termPtr = NULL;           \
  _DEREF(term,termPtr,tag);

#define DEREF0(term, termPtr, tag)              \
  register TaggedRef *termPtr;                  \
  _DEREF(term,termPtr,tag);

#define DEREFPTR(term, termPtr, tag)            \
  register TaggedRef term = *termPtr;           \
  _DEREF(term,termPtr,tag);

#define SAFE_DEREF(term)                                \
if (oz_isRef(term)) {                                   \
  DEREF(term,SAFE__PTR__,SAFE__TAG__);                  \
  if (_oz_isVariable(term)) term=makeTaggedRef(SAFE__PTR__);    \
}

inline
TaggedRef oz_deref(TaggedRef t) {
  DEREF(t,_1,_2);
  return t;
}

inline
TaggedRef oz_safeDeref(TaggedRef t) {
  SAFE_DEREF(t);
  return t;
}

inline
TaggedRef oz_derefPtr(TaggedRef t) {
  DEREF(t,tptr,_2);
  return (TaggedRef) tptr;
}

// ---------------------------------------------------------------------------
// Binding
// ---------------------------------------------------------------------------

#ifdef DEBUG_CHECK
inline
TaggedRef *_derefPtr(TaggedRef t) {
  DEREF(t,tPtr,_1);
  return tPtr;
}
#endif

inline int32  GCMARK(void *S)    { return makeTaggedRef2p(GCTAG,S); }
inline int32  GCMARK(int32 S)    { return makeTaggedRef2i(GCTAG,S); }

inline void *GCUNMARK(int32 S)   { return tagValueOf2(GCTAG,S); }
inline Bool GCISMARKED(int32 S)  { return GCTAG==tagTypeOf((TaggedRef)S); }

/*===================================================================
 * RefsArray
 *=================================================================== */

// RefsArray is an array of TaggedRef
// a[-1] = LL...LLLTTTT,
//     L = length
//     T = tag (RADirty/RAFreed) or GCTAG during GC


typedef TaggedRef *RefsArray;

/* any combination of the following must not be equal to the GCTAG */
#ifdef DEBUG_CHECK
const int RAFreed = 1; // means has been already deallocated

inline
Bool isFreedRefsArray(RefsArray a)
{
  return (a && a[-1]&RAFreed);
}

inline
void markFreedRefsArray(RefsArray a)
{
  if (a) a[-1] |= RAFreed;
}

#define RAtagSize tagSize

#else

#define RAtagSize 0

#endif



inline
void setRefsArraySize(RefsArray a, int32 n)
{
  a[-1] = (n<<RAtagSize);
}

inline
int getRefsArraySize(RefsArray a)
{
  return (a[-1]>>RAtagSize);
}


#if defined(DEBUG_CHECK) && defined(__MINGW32__)
static
#else
inline
#endif
Bool initRefsArray(RefsArray a, int size, Bool init)
{
  setRefsArraySize(a,size);
  if (init) {
    switch (size) {
    case 10: a[9] = makeTaggedNULL();
    case  9: a[8] = makeTaggedNULL();
    case  8: a[7] = makeTaggedNULL();
    case  7: a[6] = makeTaggedNULL();
    case  6: a[5] = makeTaggedNULL();
    case  5: a[4] = makeTaggedNULL();
    case  4: a[3] = makeTaggedNULL();
    case  3: a[2] = makeTaggedNULL();
    case  2: a[1] = makeTaggedNULL();
    case  1: a[0] = makeTaggedNULL();
      break;
    default:
      {
        for(int i = size-1; i >= 0; i--)
          a[i] = makeTaggedNULL();
      }
      break;
    }
  }

  return OK;  /* due to stupid CC */
}

inline
RefsArray allocateRefsArray(int n, Bool init=OK)
{
  Assert(n > 0);
  RefsArray a = ((RefsArray) freeListMalloc((n+1) * sizeof(TaggedRef)));
  a += 1;
  initRefsArray(a,n,init);
  return a;
}

inline
RefsArray allocateRefsArray(int n, TaggedRef initRef)
{
  Assert(n > 0);
  RefsArray a = ((RefsArray) freeListMalloc((n+1) * sizeof(TaggedRef)));
  a += 1;
  // Initialize with given TaggedRef:
  setRefsArraySize(a,n);
  for(int i = n-1; i >= 0; i--)
    a[i] = initRef;
  return a;
}

inline
void disposeRefsArray(RefsArray a)
{
  if (a) {
    int sz = getRefsArraySize(a);
    a -= 1;
    freeListDispose(a, (sz+1) * sizeof(TaggedRef));
  }
}

inline
RefsArray allocateStaticRefsArray(int n)
{
  RefsArray a = new TaggedRef[n + 1];
  a += 1;
  initRefsArray(a,n,OK);
  return a;
}


inline
RefsArray copyRefsArray(RefsArray a)
{
  int n = getRefsArraySize(a);
  RefsArray r = allocateRefsArray(n,NO);
  for (int i = n; i--;) {
    r[i] = tagged2NonVariable(&a[i]);
  }
  return r;
}

inline
RefsArray copyRefsArray(RefsArray a,int n,Bool init=NO)
{
  RefsArray r = allocateRefsArray(n,init);
  for (int i = n; i--;) {
    CHECK_NONVAR(a[i]);
    r[i] = a[i];
  }
  return r;
}


inline
RefsArray resize(RefsArray r, int s)
{
  int size = getRefsArraySize(r);
  if (s < size){
    setRefsArraySize(r,s);
    return r;
  }

  if (s > size){
    RefsArray aux = allocateRefsArray(s);
    for(int j = size; j--;)
      aux[j] = r[j];
    return aux;
  }
  return r;
} // resize


/*===================================================================
 *
 *=================================================================== */

//
// identity test
//
inline
Bool oz_eq(TaggedRef t1, TaggedRef t2)
{
  Assert(t1==oz_safeDeref(t1));
  Assert(t2==oz_safeDeref(t2));
  return t1==t2;
}


/*===================================================================
 * Tagged Pointer classes
 *=================================================================== */

/*
 * class Tagged2:
 *  32 bit word to store
 *    word aligned pointer + 2 tag bits or
 *    30 bit value + 2 tag bits
 */
class Tagged2
{
private:
  static const int mask=3;
  static const int bits=2;
  uint32 tagged;
  void checkTag(int tag)       { Assert(tag >=0 && tag <=mask); }
  void checkVal(uint32 val)    { Assert((val & (mask<<(32-bits))) == 0); }
  void checkPointer(void* ptr) { Assert((((uint32) ptr)&mask) == 0); }
public:
  Tagged2()                   { tagged = 0; }
  Tagged2(void* ptr,int tag)  { set(ptr,tag); }
  Tagged2(uint32 val,int tag) { set(val,tag); }

  void set(void* ptr,int tag) {
    checkPointer(ptr);
    checkTag(tag);
    tagged = ((uint32)ptr) | tag;
  }
  void set(uint32 val,int tag) {
    checkTag(tag);
    checkVal(val);
    tagged = (val<<bits) | tag;
  }
  void setPtr(void* ptr) {
    checkPointer(ptr);
    tagged = ((uint32)ptr) | getTag();
  }
  void setVal(uint32 val) {
    checkVal(val);
    tagged = (val<<bits) | getTag();
  }

  uint32* getRef()  { return &tagged; }
  int     getTag()  { return (tagged&mask); }
  uint32  getData() { return tagged>>bits; }
  void*   getPtr()  { return (void*)(tagged&~mask); }
};

/*
 * class Tagged4:
 *  32 bit word to store
 *    word aligned pointer in fixed segment + 4 tag bits or
 *    28 bit value + 4 tag bits
 */
class Tagged4
{
private:
  static const int mask=15;
  static const int bits=4;
  uint32 tagged;
  void checkTag(int tag) { Assert(tag >= 0 && tag <= mask); }
  void checkVal(int val) { Assert((val & (mask<<(32-bits))) == 0); }
  void checkPtr(void* ptr) {
    uint32 val=(uint32) ptr;
    Assert((val&(mask>>2))==0);
    Assert((val&((mask>>2)<<(32-(bits-2))))==mallocBase);
  }
public:
  Tagged4()                   { tagged = 0; }
  Tagged4(void* ptr,int tag)  { set(ptr,tag); }
  Tagged4(uint32 val,int tag) { set(val,tag); }

  void set(void* ptr,int tag) {
    checkPtr(ptr);
    checkTag(tag);
    tagged = (((uint32)ptr)<<(bits-2)) | tag;
  }
  void set(uint32 val,int tag) {
    checkTag(tag);
    checkVal(val);
    tagged = (val<<bits) | tag;
  }

  uint32* getRef() { return &tagged; }
  int     getTag() { return (tagged&mask); }
  uint32  getData(){ return tagged>>bits; }
  void*   getPtr() {
    return (void*)(mallocBase|((tagged>>(bits-2))&~(mask>>2)));
  }
};

/*===================================================================
 *
 *=================================================================== */

inline
int nextPowerOf2(int n)
{
  for(int i=2;; i*=2) {
    if (i>=n) return i;
  }
}

/*===================================================================
 * Alternate DEREF interface
 * Idea: only if the unit is a ref it can be a variable
 *=================================================================== */

#define DerefIfVarDo(v,Block)                   \
 if (oz_isRef(v)) {                             \
   v=oz_safeDeref(v);                           \
   if (oz_isRef(v)) { Block; }                  \
 }

#define DerefIfVarReturnIt(v)   DerefIfVarDo(v, return v);
#define DerefIfVarSuspend(v)    DerefIfVarDo(v, return SUSPEND);


/* Extension */

inline
int oz_isExtension(OZ_Term t) {
  return tagTypeOf(t)==EXT;
}

inline
OZ_Extension *oz_tagged2Extension(OZ_Term t) {
  return (OZ_Extension *) tagValueOf2(EXT,t);
}

inline
OZ_Term oz_makeTaggedExtension(OZ_Extension *e) {
  return makeTaggedRef2p(EXT,e);
}

#endif
