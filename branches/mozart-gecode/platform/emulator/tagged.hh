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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

#include "base.hh"
#include "mem.hh"

/*
 * There are two different classes of tags:
 *  - Short tags (stag_t). They span 3 bits and do not 
 *    distinguish between literals and small integers.
 *  - Long tags (ltag_t). They span an additional bit
 *    and thus can distinguish literals and small integers.
 *  Additionally, each short tag appears twice as long tag.
 *
 * Depending on whether one wants to distinguish literals
 * and small integers one can use either short or long tags.
 *
 */

#define RTAG_BITS 2  /* Bits for references */ 
#define STAG_BITS 3  /* Bits for short tags */
#define LTAG_BITS 4  /* Bits for long tags  */

#define __MASKIFY(i) ((1<<(i))-1)

#define RTAG_MASK __MASKIFY(RTAG_BITS)
#define STAG_MASK __MASKIFY(STAG_BITS)
#define LTAG_MASK __MASKIFY(LTAG_BITS)

/*
 * Short tags
 *
 */

// If you change this, also change OZ_isEqualVars in mozart_cpi.hh!

enum stag_t {
  STAG_REF0     = 0 /* 000 */,  // This is fixed: least two bits are zero!
  STAG_VAR      = 1 /* 001 */,  // Don't move this tag: isVarOrRef
  STAG_LTUPLE   = 2 /* 010 */,  // Don't move this tag: isLTupleOrRef
  STAG_CONST    = 3 /* 011 */,
  STAG_REF1     = 4 /* 100 */,  // This is fixed: least two bits are zero!
  STAG_SRECORD  = 5 /* 101 */,
  STAG_TOKEN    = 6 /* 110 */,
  STAG_MARK     = 7 /* 111 */,
};


/*
 * Long tags
 *
 */

enum ltag_t {
  LTAG_REF00     =  0 /* 0000 */,  // This is fixed: least two bits are zero!
  LTAG_VAR0      =  1 /* 0001 */,  // Don't move this tag: isVarOrRef
  LTAG_LTUPLE0   =  2 /* 0010 */,  // Don't move this tag: isLTupleOrRef
  LTAG_CONST0    =  3 /* 0011 */,
  LTAG_REF10     =  4 /* 0100 */,  // This is fixed: least two bits are zero!
  LTAG_SRECORD0  =  5 /* 0101 */,
  LTAG_LITERAL   =  6 /* 0110 */,  // IMPORTANT!!
  LTAG_MARK0     =  7 /* 0111 */,
  LTAG_REF01     =  8 /* 1000 */,  // This is fixed: least two bits are zero!
  LTAG_VAR1      =  9 /* 1001 */,  // Don't move this tag: isVarOrRef
  LTAG_LTUPLE1   = 10 /* 1010 */,  // Don't move this tag: isLTupleOrRef
  LTAG_CONST1    = 11 /* 1011 */,
  LTAG_REF11     = 12 /* 1100 */,  // This is fixed: least two bits are zero!
  LTAG_SRECORD1  = 13 /* 1101 */,
  LTAG_SMALLINT  = 14 /* 1110 */,  // IMPORTANT!!
  LTAG_MARK1     = 15 /* 1111 */,
};

// Zero-bits masks for is{Var,LTuple}OrRef
#define VAR_ZEROBITS	0x6
#define LTUPLE_ZEROBITS	0x5

#define LTAG_PAIR(a,b) (((a)<<LTAG_BITS)|(b))

#define LTAG_CASE_A_REF(a) \
  case LTAG_PAIR(a,LTAG_REF00): case LTAG_PAIR(a,LTAG_REF01): \
  case LTAG_PAIR(a,LTAG_REF10): case LTAG_PAIR(a,LTAG_REF11):
#define LTAG_CASE_VAL_REF \
  LTAG_CASE_A_REF(LTAG_VAR0)     LTAG_CASE_A_REF(LTAG_CONST0)   \
  LTAG_CASE_A_REF(LTAG_LTUPLE0)  LTAG_CASE_A_REF(LTAG_SRECORD0) \
  LTAG_CASE_A_REF(LTAG_LITERAL)  LTAG_CASE_A_REF(LTAG_MARK0)    \
  LTAG_CASE_A_REF(LTAG_VAR1)     LTAG_CASE_A_REF(LTAG_CONST1)   \
  LTAG_CASE_A_REF(LTAG_LTUPLE1)  LTAG_CASE_A_REF(LTAG_SRECORD1) \
  LTAG_CASE_A_REF(LTAG_SMALLINT) LTAG_CASE_A_REF(LTAG_MARK1)

#define LTAG_CASE_REF_A(b) \
  case LTAG_PAIR(LTAG_REF00,b): case LTAG_PAIR(LTAG_REF01,b): \
  case LTAG_PAIR(LTAG_REF10,b): case LTAG_PAIR(LTAG_REF11,b):
#define LTAG_CASE_REF_VAL \
  LTAG_CASE_REF_A(LTAG_VAR0)     LTAG_CASE_REF_A(LTAG_CONST0)   \
  LTAG_CASE_REF_A(LTAG_LTUPLE0)  LTAG_CASE_REF_A(LTAG_SRECORD0) \
  LTAG_CASE_REF_A(LTAG_LITERAL)  LTAG_CASE_REF_A(LTAG_MARK0)    \
  LTAG_CASE_REF_A(LTAG_VAR1)     LTAG_CASE_REF_A(LTAG_CONST1)   \
  LTAG_CASE_REF_A(LTAG_LTUPLE1)  LTAG_CASE_REF_A(LTAG_SRECORD1) \
  LTAG_CASE_REF_A(LTAG_SMALLINT) LTAG_CASE_REF_A(LTAG_MARK1)

#define LTAG_CASE_REF_REF \
  LTAG_CASE_REF_A(LTAG_REF00) LTAG_CASE_REF_A(LTAG_REF01) \
  LTAG_CASE_REF_A(LTAG_REF10) LTAG_CASE_REF_A(LTAG_REF11)


/*
 * Tagging & untagging: low level routines
 *
 */

#define __ltag_ptr(t,lt) ((TaggedRef) (((int) (t))+(lt)))
#define __stag_ptr(t,st) ((TaggedRef) (((int) (t))+(st)))

#define __ltag_int(t,lt) ((TaggedRef) ((((int) (t))<<LTAG_BITS)+(lt)))
#define __stag_int(t,st) ((TaggedRef) ((((int) (t))<<STAG_BITS)+(st)))

#define __unltag_ptr(c,t,lt)   ((c) (((TaggedRef)(t))-(lt)))
#define __unstag_ptr(c,t,st)   ((c) (((TaggedRef)(t))-(st)))

#define __tagged2stag(t) ((stag_t) (((TaggedRef)(t)) & STAG_MASK))
#define __hasStag(t,st)  !(__unstag_ptr(int,t,st) & STAG_MASK)

#define __tagged2ltag(t) ((ltag_t) (((TaggedRef)(t)) & LTAG_MASK))
#define __hasLtag(t,lt)  !(__unltag_ptr(int,t,lt) & LTAG_MASK)


// FIXME

#define ARITHMETIC_SHIFTS

#ifdef ARITHMETIC_SHIFTS

#define __unltag_int(t) (((int) (t))>>LTAG_BITS)
#define __unstag_int(t) (((int) (t))>>STAG_BITS)

#else

inline
int __unltag_int(TaggedRef t) {
  int i = (int) t;
  return (i<0) ? ~((~i)>>LTAG_BITS) : (i>>LTAG_BITS);
}
inline
int __unstag_int(TaggedRef t) {
  int i = (int) t;
  return (i<0) ? ~((~i)>>STAG_BITS) : (i>>STAG_BITS);
}

#endif





/*
 * Testing and accessing tags
 *
 */

#if defined(DEBUG_CHECK)

inline ltag_t tagged2ltag(TaggedRef t) { return __tagged2ltag(t); }
inline stag_t tagged2stag(TaggedRef t) { return __tagged2stag(t); }

inline int hasStag(TaggedRef t, stag_t st) { return __hasStag(t,st); }
inline int hasLtag(TaggedRef t, ltag_t lt) { return __hasLtag(t,lt); }

#else

#define tagged2ltag(t) __tagged2ltag(t)
#define tagged2stag(t) __tagged2stag(t)

#define hasStag(t,st)  __hasStag(t,st)
#define hasLtag(t,lt)  __hasLtag(t,lt)

#endif




/*
 * Alignment tests: "is Short/Long Tag Aligned"
 * The memory initialization routine ('initMemoryManagement()') checks
 * whether this can be really fulfilled. 
 *
 */

#define isRTAligned(ptr)  (!(ToInt32(ptr) & RTAG_MASK))
#define isSTAligned(ptr)  (!(ToInt32(ptr) & STAG_MASK))
#define isLTAligned(ptr)  (!(ToInt32(ptr) & LTAG_MASK))




/*
 * Value tests
 *
 */

#define oz_isRef(t)      (!((t) & RTAG_MASK))
#define oz_isVar(t)      hasStag(t,STAG_VAR)
#define oz_isVarOrRef(t)	(((t) & VAR_ZEROBITS) == 0)
#define oz_isLTuple(t)   hasStag(t,STAG_LTUPLE)
#define oz_isLTupleOrRef(t)	(((t) & LTUPLE_ZEROBITS) == 0)
#define oz_isLiteral(t)  hasLtag(t,LTAG_LITERAL)
#define oz_isSRecord(t)  hasStag(t,STAG_SRECORD)
#define oz_isSmallInt(t) hasLtag(t,LTAG_SMALLINT)
#define oz_isConst(t)    hasStag(t,STAG_CONST)
#define oz_isMark(t)     hasStag(t,STAG_MARK)
#define oz_isToken(t)    hasStag(t,STAG_TOKEN)



/*
 * Small integers
 *
 */

// If you change this, also change mozart_cpi.hh
#define OzMaxInt (134217727)

#if defined(FASTARITH) && defined(__GNUC__) && defined(__i386__)

#define OzMinInt (~134217727)

#else 

#define OzMinInt (-(OzMaxInt+1))

#endif

#define TaggedOzMaxInt __ltag_int(OzMaxInt,LTAG_SMALLINT)
#define TaggedOzMinInt __ltag_int(OzMinInt,LTAG_SMALLINT)




/*
 * Untagging
 *
 */

//#define SHIT_HAPPEN 1

#if defined(DEBUG_CHECK) || defined(SHIT_HAPPEN)

inline 
TaggedRef * tagged2Ref(TaggedRef t) {
  Assert(oz_isRef(t));
  return (TaggedRef *) t;
}
inline 
OzVariable * tagged2Var(TaggedRef t) {
  Assert(oz_isVar(t));
  return __unstag_ptr(OzVariable *,t,STAG_VAR);
}
inline 
SRecord * tagged2SRecord(TaggedRef t) {
  Assert(oz_isSRecord(t));
  return __unstag_ptr(SRecord *,t,STAG_SRECORD);
}
inline 
LTuple * tagged2LTuple(TaggedRef t) {
  Assert(oz_isLTuple(t));
  return __unstag_ptr(LTuple *,t,STAG_LTUPLE);
}
inline 
Literal * tagged2Literal(TaggedRef t) {
  Assert(oz_isLiteral(t));
  return __unltag_ptr(Literal *,t,LTAG_LITERAL);
}
inline 
ConstTerm * tagged2Const(TaggedRef t) {
  Assert(oz_isConst(t));
  return __unstag_ptr(ConstTerm *,t,STAG_CONST);
}
inline 
void * tagged2UnmarkedPtr(TaggedRef t) {
  Assert(oz_isMark(t));
  return __unstag_ptr(void *,t,STAG_MARK);
}
inline
int tagged2UnmarkedInt(TaggedRef t) {
  Assert(oz_isMark(t));
  return __unstag_int(t);
}
inline 
int tagged2SmallInt(TaggedRef t) {
  Assert(oz_isSmallInt(t));
  return __unltag_int(t);
}

#else

#define tagged2Ref(t)         ((TaggedRef *)  (t))
#define tagged2Var(t)         __unstag_ptr(OzVariable *,t, STAG_VAR)
#define tagged2SRecord(t)     __unstag_ptr(SRecord *,   t, STAG_SRECORD)
#define tagged2LTuple(t)      __unstag_ptr(LTuple *,    t, STAG_LTUPLE)
#define tagged2Literal(t)     __unltag_ptr(Literal *,   t, LTAG_LITERAL)
#define tagged2Const(t)       __unstag_ptr(ConstTerm *, t, STAG_CONST)
#define tagged2UnmarkedPtr(t) __unstag_ptr(void *,      t, STAG_MARK)
#define tagged2UnmarkedInt(t) __unstag_int(t)
#define tagged2SmallInt(t)    __unltag_int(t)

#endif



/*
 * Tagging
 *
 */

#if defined(DEBUG_CHECK) || defined(SHIT_HAPPEN)

inline 
TaggedRef makeTaggedRef(TaggedRef * p) {
  Assert(p != NULL && isRTAligned(p)); 
  return (TaggedRef) p;
}
inline 
TaggedRef makeTaggedVar(OzVariable * p) {
  Assert(p != NULL && isSTAligned(p));
  return __stag_ptr(p,STAG_VAR);
}
// For the GenTraverser (non-relocatable pointers are masqueraded as
// tagged variables);
inline 
TaggedRef makePseudoTaggedVar(void *p) {
  Assert(p != NULL && isSTAligned(p));
  return __stag_ptr(p, STAG_VAR);
}
inline 
TaggedRef makeTaggedLTuple(LTuple * p) {
  Assert(p != NULL && isSTAligned(p));
  return __stag_ptr(p,STAG_LTUPLE);
}
inline 
TaggedRef makeTaggedSRecord(SRecord * p) {
  Assert(p != NULL && isSTAligned(p));
  return __stag_ptr(p,STAG_SRECORD);
}
inline 
TaggedRef makeTaggedLiteral(Literal * p) {
  Assert(p != NULL && isLTAligned(p));
  return __ltag_ptr(p,LTAG_LITERAL);
}
inline 
TaggedRef makeTaggedConst(ConstTerm * p) {
  Assert(p != NULL && isSTAligned(p));
  return __stag_ptr(p,STAG_CONST);
}
inline 
TaggedRef makeTaggedMarkPtr(void * p) {
  Assert(isSTAligned(p));
  return __stag_ptr(p, STAG_MARK);
}
inline
TaggedRef makeTaggedMarkInt(int i) {
  return __stag_int(i, STAG_MARK);
}
// e.g. gentraverser needs static constants;
#define makeTaggedMarkIntNOTEST(i)	__stag_int(i, STAG_MARK)
inline 
TaggedRef makeTaggedSmallInt(int i) {
  Assert(i >= OzMinInt && i <= OzMaxInt);
  return __ltag_int(i,LTAG_SMALLINT);
}

#else

#define makeTaggedRef(p)            ((TaggedRef) ((TaggedRef*)(p)))
#define makeTaggedVar(p)            __stag_ptr(((OzVariable*)(p)),STAG_VAR)
#define makePseudoTaggedVar(p)      __stag_ptr(((void*)(p)),STAG_VAR)
#define makeTaggedLTuple(p)         __stag_ptr(((LTuple*)(p)),STAG_LTUPLE)
#define makeTaggedSRecord(p)        __stag_ptr(((SRecord*)(p)),STAG_SRECORD)
#define makeTaggedLiteral(p)        __ltag_ptr(((Literal*)(p)),LTAG_LITERAL)
#define makeTaggedConst(p)          __stag_ptr(((ConstTerm*)(p)),STAG_CONST)
#define makeTaggedMarkPtr(p)        __stag_ptr(((void*)(p)),STAG_MARK)
#define makeTaggedMarkInt(p)        __stag_int(((int)(p)),STAG_MARK)
#define makeTaggedMarkIntNOTEST(p)  __stag_int(p,STAG_MARK)
#define makeTaggedSmallInt(p)       __ltag_int(((int)(p)),LTAG_SMALLINT)

#endif




/*
 * Standard values
 *
 */

#define makeTaggedNULL()       ((TaggedRef) 0)
#define taggedVoidValue        __ltag_int(0,LTAG_SMALLINT)





/*
 * DEREF
 *
 */

#define _DEREF(term, termPtr)		               \
  MemAssert(MemChunks::isInHeap(term));		       \
  while (oz_isRef(term)) {			       \
    termPtr = tagged2Ref(term);			       \
    term    = *termPtr;				       \
  }

#define DEREF(term, termPtr)		               \
  register TaggedRef *termPtr = NULL;		       \
  _DEREF(term,termPtr);

#define DEREF0(term, termPtr)		               \
  register TaggedRef *termPtr;			       \
  _DEREF(term,termPtr);

#define DEREFPTR(term, termPtr)		               \
  register TaggedRef term = *termPtr;		       \
  _DEREF(term,termPtr);

#define DEREF_NONVAR(term)				\
  while (oz_isRef(term)) {				\
    register TaggedRef *termPtr = tagged2Ref(term);	\
    term = *termPtr;					\
  }

inline
TaggedRef oz_deref(TaggedRef t) {
  DEREF_NONVAR(t);
  return t;
}

inline
TaggedRef oz_derefOne(TaggedRef t) {
  Assert(oz_isRef(t));
  return *tagged2Ref(t);
}

inline
TaggedRef oz_safeDeref(TaggedRef t) {
  if (oz_isRef(t)) {
    TaggedRef * sp = tagged2Ref(t);
    _DEREF(t,sp);
    Assert(!oz_isRef(t));
    if (oz_isVarOrRef(t))
      t = makeTaggedRef(sp);          
  }
  return t;
}

inline
TaggedRef oz_derefPtr(TaggedRef t) {
  DEREF(t,tptr);
  return (TaggedRef) tptr;
}



/*
 * Derived abstractions for variables
 *
 */

#if defined(DEBUG_CHECK) || defined(SHIT_HAPPEN)

inline 
TaggedRef tagged2NonVariable(TaggedRef * t) {
  return oz_isVar(*t) ? makeTaggedRef(t) : *t;
}

inline 
TaggedRef tagged2NonVariableFast(TaggedRef * t) {
  return oz_isVarOrRef(*t) ? makeTaggedRef(t) : *t;
}

#else

#define tagged2NonVariable(t) ((oz_isVar(*(t))?makeTaggedRef(t):(*(t))))
#define tagged2NonVariableFast(t) ((oz_isVarOrRef(*(t))?makeTaggedRef(t):(*(t))))

#endif

inline 
TaggedRef * newTaggedRef(TaggedRef *t) {
  TaggedRef * ref = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ref = makeTaggedRef(t);
  return ref;
}

inline 
TaggedRef * newTaggedOptVar(TaggedRef proto) {
  TaggedRef * ref = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ref = proto;
  return (ref);
}

inline 
TaggedRef * newTaggedVar(OzVariable * c) {
  TaggedRef * ref = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ref = makeTaggedVar(c);
  return ref;
}


/*
 * Safe identity test 
 *
 */

#if defined(DEBUG_CHECK)

inline Bool oz_eq(TaggedRef t1, TaggedRef t2) {
  return t1==t2;
}

#else

#define oz_eq(t1,t2) ((Bool)(((TaggedRef) (t1))==((TaggedRef) (t2))))

#endif

/*
 * generic "address of" routine;
 */
void *tagged2Addr(TaggedRef t);



/*
 * Tagged Pointer class
 *
 * class Tagged2:
 *  32 bit word to store
 *    word aligned pointer + 2 tag bits or
 *    30 bit value + 2 tag bits
 *
 */

#define __tagged2Mask 3
#define __tagged2Bits 2

class Tagged2 {
private:
  uint32 tagged;
  void checkTag(int tag)       { Assert(tag >=0 && tag <=__tagged2Mask); }
  void checkVal(uint32 val)    { Assert((val & (__tagged2Mask<<(32-__tagged2Bits))) == 0); }
  void checkPointer(void* ptr) { Assert((((uint32) ptr)&__tagged2Mask) == 0); }
public:

  int     getTag()  { return (tagged&__tagged2Mask); }
  uint32  getData() { return tagged>>__tagged2Bits; }
  void*   getPtr()  { return (void*)(tagged&~__tagged2Mask); }

  void set(void* ptr,int tag) {
    checkPointer(ptr);
    checkTag(tag);
    tagged = ((uint32)ptr) | tag;
  }
  void set(uint32 val,int tag) {
    checkTag(tag);
    checkVal(val);
    tagged = (val<<__tagged2Bits) | tag;
  }
  void setPtr(void* ptr) {
    checkPointer(ptr);
    tagged = ((uint32)ptr) | getTag();
  }
  void setTag(int tag) {
    checkTag(tag);
    tagged = (tagged & ~__tagged2Mask) | tag;
  }
  void borTag(int tag) {
    checkTag(tag);
    tagged = tagged | tag;
  }
  void bandTag(int tag) {
    checkTag(tag & __tagged2Mask);
    tagged = tagged & (tag | ~__tagged2Mask);
  }
  void setVal(uint32 val) {
    checkVal(val);
    tagged = (val<<__tagged2Bits) | getTag();
  }

  Tagged2()                   { tagged = 0; }
  Tagged2(void* ptr,int tag)  { set(ptr,tag); }
  Tagged2(uint32 val,int tag) { set(val,tag); }

};



/*
 * This guy just hangs around here
 *
 */

inline 
int nextPowerOf2(int n)
{
  for(int i=2;; i*=2) {
    if (i>=n) return i;
  }
}


#endif


