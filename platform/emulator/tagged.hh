/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

/* classes:
     TaggedRef
     RefsArray
     Equation
     */


#ifndef __TAGGEDH
#define __TAGGEDH

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>


#include "error.h"
#include "types.hh"
#include "mem.hh"
#include "gc.hh"

// ---------------------------------------------------------------------------
// ---------- TAGGED REF -----------------------------------------------------
// ---------------------------------------------------------------------------

// --- TaggedRef: Type Declaration

enum TypeOfTerm {
//  REF              =  0, // XX00
  
  UVAR             =  1,   // 0001
  SVAR             =  9,   // 1001
  CVAR             =  5,   // 0101
  
  UNUSEDVARIABLE   =  13,  // 1101
	    
  LTUPLE           =  2,   // 0010
  STUPLE           =  6,   // 0110
  SRECORD          = 14,   // 1110
		  
  ATOM             = 15,   // 1111

  CONST            = 10,   // 1010

  SMALLINT         =  3,   // 0011
  BIGINT           =  7,   // 0111
  FLOAT            = 11    // 1011
// 12 = 1100 unusable \
//  8 = 1000 unusable  >  recognized as reference
//  4 = 0100 unusable /
//  3 = 0011 free
};  

// ---------------------------------------------------------------------------
// --- TaggedRef: CLASS / BASIC ACCESS FUNCTIONS


typedef unsigned int TaggedRef;

const int tagSize = 4;
const int tagMask   = 0xF;


#if defined(ULTRIX_MIPS) || defined(IRIX5_MIPS)
const int mallocBase = 0x10000000;
#else
#ifdef HPUX_700
const int mallocBase = 0x40000000;
#else
const int mallocBase = 0;
#endif
#endif


// we loose 5 bits, 1 for GC, 4 for tags
const int maxPointer = ((unsigned int)~0 >> (tagSize+1))|mallocBase;


// ------------------------------------------------------
// Debug macros for debugging outside of gc

extern int gcing;

inline
void GCDEBUG(TaggedRef X)
{
#ifdef DEBUG_GC
  if ( gcing && ((int)X & GCTAG) )   // ugly, but I've found no other way  
    error("GcTag unexpectedly found.");
#endif
}


inline
TaggedRef makeTaggedRef(TypeOfTerm tag, void *ptr)
{
  return ((int)ptr << tagSize) | tag;
}

inline
TypeOfTerm tagTypeOf(TaggedRef ref) 
{ 
  GCDEBUG(ref);
  return (TypeOfTerm)(ref&tagMask); 
}

inline
void *tagValueOf(TaggedRef ref) 
{ 
  GCDEBUG(ref);
  return (void*)((ref >> tagSize) | mallocBase);
}



// ---------------------------------------------------------------------------
// --- TaggedRef: useful functions --> print.C

char *tagged2String(TaggedRef ref, int depth = 10, int offset = 0);
void taggedPrint(TaggedRef ref,int depth = 10, int offset = 0);
void taggedPrintLong(TaggedRef ref, int depth = 10, int offset = 0);


// ---------------------------------------------------------------------------
// --- TaggedRef: CHECK_xx

// Philosophy:
//   Arguments which are passed around are never variables, but only
//     REF or bound data
#define CHECK_NONVAR(term) Assert(isRef(term) || !isAnyVar(term))
#define CHECK_ISVAR(term)  Assert(isAnyVar(term))
#define CHECK_DEREF(term)  Assert(!isRef(term) && !isAnyVar(term))
#define CHECK_POINTER(s)   Assert(s != NULL && !((int) s & 3) )
#define CHECKTAG(Tag)      Assert(tagTypeOf(ref) == Tag)


// ---------------------------------------------------------------------------
// --- TaggedRef: BASIC TYPE TESTS

// if you want to test a term:
//   1. if you have the tag: is<Type>(tag)
//   2. if you have the term: is<Type>(term)
//   3. if you need many tests:
//        switch(typeOf(term)) { ... }
//    or  switch(tag) { ... }


#define IsRef(term) (((term & 3) == 0) ? OK : NO)
inline
Bool isRef(TaggedRef term) {
  GCDEBUG(term);
  return IsRef(term);
}


// ---------------------------------------------------------------------------
// Tests for variables and their semantics:
//                           tag
// unconstrained var         0001 (UVAR:1)  isUVar   isNotCVar  isAnyVar
// unconstr. suspending var  1001 (SVAR:9)  isSVar   isNotCVar  isAnyVar
// constrained-susp. var     0101 (CVAR:5)           isCVar    isAnyVar
// ---------------------------------------------------------------------------


inline
Bool isUVar(TypeOfTerm tag) {
  return (tag == UVAR) ? OK : NO ;
}

inline
Bool isUVar(TaggedRef term) {
  GCDEBUG(term);
  DebugCheck(isRef(term),
	     error("isUVar: illegal Arg: REF");
	     return NO;);
  return ((term & 14) == 0) ? OK : NO;   // mask 1110
}

inline
Bool isSVar(TypeOfTerm tag) {
  return (tag == SVAR) ? OK : NO ;
}

inline
Bool isSVar(TaggedRef term) {
  GCDEBUG(term);
  DebugCheck(isRef(term),
	     error("isSVar: illegal Arg: REF");
	     return NO;);
  return isSVar(tagTypeOf(term));
}


inline
Bool isCVar(TypeOfTerm tag) {
  return (tag == CVAR) ? OK : NO;
}

inline
Bool isCVar(TaggedRef term) {
  GCDEBUG(term);
  DebugCheck(isRef(term),
	     error("isCVar: illegal Arg: REF");
	     return NO;);
  return isCVar(tagTypeOf(term));
}


inline
Bool isNotCVar(TypeOfTerm tag) {
  return ((tag & 6) == 0) ? OK : NO;
}

inline
Bool isNotCVar(TaggedRef term) {
  GCDEBUG(term);
  DebugCheck(isRef(term),
	     error("isNotCVar: illegal Arg: REF");
	     return NO;);
  return ((term & 6) == 0) ? OK : NO;  // mask = 0110
}


inline
Bool isAnyVar(TypeOfTerm tag) {
  return ((tag & 2) == 0) ? OK : NO;   // mask = 0010
}

inline
Bool isAnyVar(TaggedRef term) {
  GCDEBUG(term);
  DebugCheck(isRef(term),
	     error("isAnyVar: illegal Arg: REF");
	     return NO;);
  return ((term & 2) == 0) ? OK : NO;
}



inline
Bool isLiteral(TypeOfTerm tag) {
  return tag == ATOM ? OK : NO ;
}

inline
Bool isLiteral(TaggedRef term) {
  GCDEBUG(term);
  return isLiteral(tagTypeOf(term));
}

inline
Bool isSRecord(TypeOfTerm tag) {
  return tag == SRECORD ? OK : NO ;
}

inline
Bool isSRecord(TaggedRef term) {
  GCDEBUG(term);
  return isSRecord(tagTypeOf(term));
}

inline
Bool isRecord(TypeOfTerm tag) {
  return isSRecord(tag) || isLiteral(tag) ? OK : NO ;
}

inline
Bool isRecord(TaggedRef term) {
  GCDEBUG(term);
  return isRecord(tagTypeOf(term));
}

inline
Bool isLTuple(TypeOfTerm tag) {
  return (tag == LTUPLE) ? OK : NO ;
}

inline
Bool isLTuple(TaggedRef term) {
  GCDEBUG(term);
  DebugCheck(isRef(term),
	     error("isLTuple: illegal Arg: REF");
	     return NO;);
  return ((term & 13) == 0) ? OK : NO; // mask = 1101
}

inline
Bool isSTuple(TypeOfTerm tag) {
  return (tag == STUPLE) ? OK : NO ;
}

inline
Bool isSTuple(TaggedRef term) {
  GCDEBUG(term);
  return isSTuple(tagTypeOf(term));
}

inline
Bool isTuple(TypeOfTerm tag) {
  return isSTuple(tag) || isLTuple(tag) || isLiteral(tag) ? OK : NO ;
}

inline
Bool isTuple(TaggedRef term) {
  GCDEBUG(term);
  return isTuple(tagTypeOf(term));
}

inline
Bool isNoNumber(TypeOfTerm tag) {
  return isRecord(tag) || isTuple(tag) ? OK : NO ;
}

inline
Bool isNoNumber(TaggedRef term) {
  GCDEBUG(term);
  return isNoNumber(tagTypeOf(term));
}

inline
Bool isFloat(TypeOfTerm tag) {
  return (tag == FLOAT) ? OK : NO ;
}

inline
Bool isFloat(TaggedRef term) {
  GCDEBUG(term);
  return isFloat(tagTypeOf(term));
}

inline
Bool isSmallInt(TypeOfTerm tag) {
  return (tag == SMALLINT) ? OK : NO ;
}

inline
Bool isSmallInt(TaggedRef term) {
  return isSmallInt(tagTypeOf(term));
}

inline
Bool isBigInt(TypeOfTerm tag) {
  return (tag == BIGINT) ? OK : NO ;
}

inline
Bool isBigInt(TaggedRef term) {
  GCDEBUG(term);
  return isBigInt(tagTypeOf(term));
}

inline
Bool isInt(TypeOfTerm tag) {
  return (isSmallInt(tag) || isBigInt(tag)) ? OK : NO ;
}

inline
Bool isInt(TaggedRef term) {
  GCDEBUG(term);
  return isInt(tagTypeOf(term));
}

inline
Bool isNumber(TypeOfTerm tag) {
  return (isFloat(tag) || isInt(tag)) ? OK : NO ;
}

inline
Bool isNumber(TaggedRef term) {
  GCDEBUG(term);
  return isNumber(tagTypeOf(term));
}

inline
Bool isConst(TypeOfTerm tag) {
  return (tag == CONST) ? OK : NO ;
}

inline
Bool isConst(TaggedRef term) {
  GCDEBUG(term);
  return isConst(tagTypeOf(term));
}

// ---------------------------------------------------------------------------
// --- TaggedRef: create: makeTagged<Type>

// this function should be used, if tagged references are to be initialized
#ifdef DEBUG_CHECK
inline
TaggedRef makeTaggedNULL()
{
  return makeTaggedRef((TypeOfTerm)0,NULL);
}
#else
#define makeTaggedNULL() ((TaggedRef) 0)
#endif

inline
TaggedRef makeTaggedMisc(void *s)
{
  return makeTaggedRef((TypeOfTerm)0,s);
}

inline
TaggedRef makeTaggedMisc(int s)
{
  return makeTaggedMisc((void *) s);
}

inline
TaggedRef makeTaggedRef(TaggedRef *s)
{
  CHECK_POINTER(s);
  DebugGC(gcing == 0 && !inChunkChain (heapGetStart (), (void *)s),
	  error ("making TaggedRef pointing to 'from' space"));
  return (TaggedRef)s;
}

inline
TaggedRef makeTaggedUVar(Board *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(UVAR,s);
}

inline
TaggedRef makeTaggedSVar(SVariable *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(SVAR,s);
}

inline
TaggedRef makeTaggedCVar(GenCVariable *s) {
  CHECK_POINTER(s);
  return makeTaggedRef(CVAR, s);
}

inline
TaggedRef makeTaggedSTuple(STuple *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(STUPLE,s);
}

inline
TaggedRef makeTaggedLTuple(LTuple *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(LTUPLE,s);
}

inline
TaggedRef makeTaggedSRecord(SRecord *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(SRECORD,s);
}


inline
TaggedRef makeTaggedAtom(char *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(ATOM,addToAtomTab(s));
}

inline
TaggedRef makeTaggedAtom(Atom *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(ATOM,s);
}

inline
TaggedRef makeTaggedSmallInt(unsigned int s)
{
  return makeTaggedRef(SMALLINT,(void*)s);
}

inline
TaggedRef makeTaggedBigInt(BigInt *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(BIGINT,s);
}

inline
TaggedRef makeTaggedFloat(Float *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(FLOAT,s);
}


inline
TaggedRef makeTaggedConst(ConstTerm *s)
{
  CHECK_POINTER(s);
  return makeTaggedRef(CONST,s);
}

// getArg() and the like may never return variables
inline
TaggedRef tagged2NonVariable(TaggedRef *term)
{
  GCDEBUG(*term);
  TaggedRef ret = *term;
  if (!IsRef(ret) && isAnyVar(ret)) {
    ret = makeTaggedRef(term);
  }
  return ret;
}


// ---------------------------------------------------------------------------
// --- TaggedRef: allocate on heap, an return a ref to it

inline
TaggedRef *newTaggedSVar(SVariable *c)
{
  TaggedRef *ref = (TaggedRef *) heapMalloc(sizeof(TaggedRef));
  *ref = makeTaggedSVar(c);
  return ref;
}

inline
TaggedRef *newTaggedUVar(Board *c)
{
  TaggedRef *ref = (TaggedRef *) heapMalloc(sizeof(TaggedRef));
  *ref = makeTaggedUVar(c);
  return ref;
}

inline
TaggedRef *newTaggedCVar(GenCVariable *c) {
  TaggedRef *ref = (TaggedRef *) heapMalloc(sizeof(TaggedRef));
  *ref = makeTaggedCVar(c);
  return ref;
}



// ---------------------------------------------------------------------------
// --- TaggedRef: conversion: tagged2<Type>


// this function is now obsolete, the call of tagValueOf cannot be optimized
//   if the tag is known
inline
void *tagValueOf(TypeOfTerm /* tag */, TaggedRef ref) 
{
  GCDEBUG(ref);
  return tagValueOf(ref);
}

inline
TaggedRef *tagged2Ref(TaggedRef ref)
{
  GCDEBUG(ref);
// cannot use CHECKTAG(REF); because only last two bits must be zero
  DebugCheck((ref & 3) != 0,
	     error("tagged2Ref: no REF");
	     return NULL;);

  return (TaggedRef *) ref;
}

/* does not deref home pointer! */
inline
Board *tagged2VarHome(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(UVAR);
  return (Board *) tagValueOf(UVAR,ref);
}

inline
STuple *tagged2STuple(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(STUPLE);
  return (STuple *) tagValueOf(STUPLE,ref);
}

inline
SRecord *tagged2SRecord(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(SRECORD);
  return (SRecord *) tagValueOf(SRECORD,ref);
}

inline
LTuple *tagged2LTuple(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(LTUPLE);
  return (LTuple *) tagValueOf(LTUPLE,ref);
}

inline
Atom *tagged2Atom(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(ATOM);
  return (Atom *) tagValueOf(ATOM,ref);
}

inline
Float *tagged2Float(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(FLOAT);
  return (Float *) tagValueOf(FLOAT,ref);
}

inline
unsigned int tagged2SmallInt(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(SMALLINT);
  return (unsigned int) tagValueOf(SMALLINT,ref);
}

inline
BigInt *tagged2BigInt(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(BIGINT);
  return (BigInt *) tagValueOf(BIGINT,ref);
}


inline
ConstTerm *tagged2Const(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(CONST);
  return (ConstTerm *) tagValueOf(CONST,ref);
}

inline
SVariable *tagged2SVar(TaggedRef ref)
{
  GCDEBUG(ref);
  CHECKTAG(SVAR);
  return (SVariable *) tagValueOf(SVAR,ref);
}

inline
GenCVariable *tagged2CVar(TaggedRef ref) {
  GCDEBUG(ref);
  CHECKTAG(CVAR);
  return (GenCVariable *) tagValueOf(CVAR,ref);
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
//   if (isAnyVar(tag) { *ptr = ... }
//   ....
// }

#define DEREF(term, termPtr, tag)                                             \
  TaggedRef *termPtr = NULL;         					      \
  while(IsRef(term)) {							      \
    termPtr = tagged2Ref(term);						      \
    term = *termPtr;							      \
  }									      \
  TypeOfTerm tag = tagTypeOf(term);					      \


#define DEREFPTR(termPtr, term, tag)					      \
  register TaggedRef term = *termPtr;					      \
  while(IsRef(term)) {							      \
    termPtr = tagged2Ref(term);						      \
    term = *termPtr;							      \
  }									      \
  TypeOfTerm tag = tagTypeOf(term);					      \

// ---------------------------------------------------------------------------
// --- Equation Class --------------------------------------------------------
// ---------------------------------------------------------------------------

struct Equation {
private:
  TaggedRef left;
  TaggedRef right;
public:
  void setLeft(TaggedRef *l) { left = makeTaggedRef(l); }
  void setRight(TaggedRef r) { right = r; }
  TaggedRef getLeft() { return left; }
  TaggedRef getRight() { return right; }
  TaggedRef *getLeftRef() { return &left; }
  TaggedRef *getRightRef() { return &right; }
};

// ---------------------------------------------------------------------------
// ------- RefsArray ----------------------------------------------------------
// ---------------------------------------------------------------------------


// RefsArray is an array of TaggedRef
// a[-1] = LL...LLLTT,
//     L = length
//     T = tag
// if gc bit set contains forward address
// a[0] .. a[length-1] == contents

typedef TaggedRef *RefsArray;

const int RADirty = 1; // means something has suspendeed on it
#ifdef DEBUG_CHECK
const int RAFreed = 2; // means has been already deallocated
#endif

inline
Bool isDirtyRefsArray(RefsArray a)
{
  return (a[-1]&RADirty) ? OK : NO;
}

inline
void markDirtyRefsArray(RefsArray a)
{
  if (a) a[-1] |= RADirty;
}

#ifdef DEBUG_CHECK
inline
Bool isFreedRefsArray(RefsArray a)
{
  return (a && a[-1]&RAFreed) ? OK : NO;
}

inline
void markFreedRefsArray(RefsArray a)
{
  if (a) a[-1] |= RAFreed;
}

#endif

inline
void setRefsArraySize(RefsArray a, int n)
{
  a[-1] = (TaggedRef)n<<2;
}

inline
int getRefsArraySize(RefsArray a) {
  return (int) a[-1] >> 2;
}

inline
Bool initRefsArray(RefsArray a, int size, Bool init)
{
  setRefsArraySize(a,size);
  TaggedRef help = makeTaggedNULL();    /* due to stupid gcc */
  if (init) {
    for(int i = size-1; i >= 0; i--) 
      a[i] = help;
  }

  return OK;  /* due to stupid CC */
}

inline
RefsArray allocateRefsArray(int n, Bool init=OK)
{
  Assert(n > 0);
  RefsArray a = ((RefsArray) heapMalloc((n+1) * sizeof(TaggedRef)));
  a += 1;
  initRefsArray(a,n,init);
  return a;  
}

inline
RefsArray allocateY(int n)
{
  RefsArray a = ((RefsArray) freeListMalloc((n+1) * sizeof(TaggedRef)));
  a += 1;
  initRefsArray(a,n,OK);
  return a;  
}

inline
void deallocateY(RefsArray a)
{
#ifdef DEBUG_CHECK
  markFreedRefsArray(a);
#else
  freeListDispose(a-1,(getRefsArraySize(a)+1) * sizeof(TaggedRef));
#endif
}

inline
RefsArray allocateStaticRefsArray(int n) {
// RefsArray a = (RefsArray) new char[(n+1) * sizeof(TaggedRef)];
  RefsArray a = new TaggedRef[n + 1];
  a += 1;
  initRefsArray(a,n,OK);
  return a;
}


inline
RefsArray copyRefsArray(RefsArray a) {
  int n = getRefsArraySize(a);
  RefsArray r = allocateRefsArray(n,NO);
  for (int i = n-1; i >= 0; i--) {
    r[i] = tagged2NonVariable(&a[i]);
  }
  return r;
}

inline
RefsArray copyRefsArray(RefsArray a,int n,Bool init=NO) {
  RefsArray r = allocateRefsArray(n,init);
  for (int i = n-1; i >= 0; i--) {
    CHECK_NONVAR(a[i]);
    r[i] = a[i];
  }
  return r;
}


inline
RefsArray resize(RefsArray r, int s){
  int size = getRefsArraySize(r);
  if (s < size){
    setRefsArraySize(r,s);
    return r;
  }
  
  if (s > size){
    RefsArray aux = allocateRefsArray(s);
    for(int j = size-1; j >= 0; j--)
      aux[j] = r[j];
    return aux;
  }
  return r;
} // resize


#endif
