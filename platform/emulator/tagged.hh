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
#include "atoms.hh"

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

enum stag_t {
  STAG_REF0     = 0 /* 000 */,  // This is fixed: least two bits are zero!
  STAG_VAR      = 1 /* 001 */,
  STAG_CONST    = 2 /* 010 */,
  STAG_LTUPLE   = 3 /* 011 */,
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
  LTAG_VAR0      =  1 /* 0001 */,
  LTAG_CONST0    =  2 /* 0010 */,
  LTAG_LTUPLE0   =  3 /* 0011 */,
  LTAG_REF10     =  4 /* 0100 */,  // This is fixed: least two bits are zero!
  LTAG_SRECORD0  =  5 /* 0101 */,
  LTAG_LITERAL   =  6 /* 0110 */,  // IMPORTANT!!
  LTAG_MARK0     =  7 /* 0111 */,
  LTAG_REF01     =  8 /* 1000 */,  // This is fixed: least two bits are zero!
  LTAG_VAR1      =  9 /* 1001 */,
  LTAG_CONST1    = 10 /* 1010 */,
  LTAG_LTUPLE1   = 11 /* 1011 */,
  LTAG_REF11     = 12 /* 1100 */,  // This is fixed: least two bits are zero!
  LTAG_SRECORD1  = 13 /* 1101 */,
  LTAG_SMALLINT  = 14 /* 1110 */,  // IMPORTANT!!
  LTAG_MARK1     = 15 /* 1111 */,
};



#define __tagged2stag(t) ((stag_t) ((t) & STAG_MASK))
#define __hasStag(t,st)  !((((unsigned int) (t)) - ((unsigned int) (st))) \
			   & STAG_MASK)

#define __tagged2ltag(t) ((ltag_t) ((t) & LTAG_MASK))
#define __hasLtag(t,lt)  !((((unsigned int) (t)) - ((unsigned int) (lt))) \
			   & LTAG_MASK)


#ifdef DEBUG_CHECK

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
 * Alignment tests
 *
 */

#define isSTAligned(ptr)  (!(ToInt32(ptr) & STAG_MASK))
#define isLTAligned(ptr)  (!(ToInt32(ptr) & LTAG_MASK))




/*
 * Value tests
 *
 */

#define oz_isRef(t)      (!((t) & RTAG_MASK))
#define oz_isVar(t)      hasStag(t,STAG_VAR)
#define oz_isLTuple(t)   hasStag(t,STAG_LTUPLE)
#define oz_isLiteral(t)  hasLtag(t,LTAG_LITERAL)
#define oz_isSRecord(t)  hasStag(t,STAG_SRECORD)
#define oz_isSmallInt(t) hasLtag(t,LTAG_SMALLINT)
#define oz_isConst(t)    hasStag(t,STAG_CONST)
#define oz_isMark(t)     hasStag(t,STAG_MARK)


#define TAG_LOWERMASK 3
#define TAG_PTRBITS   2




/*
 * Basic macros
 *
 */

#define TaggedToPointer(t)       ((void*) (mallocBase|t))

#define _tagValueOf2(tag,ref)    TaggedToPointer(((ref)>>TAG_PTRBITS) - ((tag)>>TAG_PTRBITS))
#define _tagValueOf(ref)         TaggedToPointer(((ref)>>TAG_PTRBITS)&~TAG_LOWERMASK)
#define _makeTaggedRef2(tag,i)   ((i << TAG_PTRBITS) | tag)

#define _makeTaggedRef2i(tag,ptr) _makeTaggedRef2(tag,(int32)ToInt32(ptr))


/* small ints are the only TaggedRefs that do not
 * contain a pointer in the value part */
#define _makeTaggedSmallInt(s) ((s << LTAG_BITS) | LTAG_SMALLINT)
#define _makeTaggedMarkInt(s) ((s << LTAG_BITS) | LTAG_MARK0)

#define _makeTaggedRef(s) ((TaggedRef) ToInt32(s))
#define _isRef(term)      ((term & TAG_LOWERMASK) == 0)
#define _tagged2Ref(ref)  ((TaggedRef *) ToPointer(ref))


/*
 * SMALL INTEGERS
 *
 */

#if defined(FASTARITH) && defined(__GNUC__) && defined(__i386__)

#define OzMaxInt (134217727)
#define OzMinInt (~134217727)

#else 

#define OzMaxInt (INT_MAX>>LTAG_BITS)
#define OzMinInt (-(OzMaxInt+1))

#endif

#define TaggedOzMaxInt _makeTaggedSmallInt(OzMaxInt)
#define TaggedOzMinInt _makeTaggedSmallInt(OzMinInt)


inline void * tagValueOf(TaggedRef ref) { 
  return _tagValueOf(ref);
}
inline void * tagValueOf2(ltag_t tag, TaggedRef ref) {
  return _tagValueOf2(tag,ref);
}
inline TaggedRef makeTaggedRef2i(ltag_t tag, int32 i) {
  Assert((i&3) == 0); return _makeTaggedRef2(tag,i);
}
inline TaggedRef makeTaggedRef2p(ltag_t tag, void *ptr) {
  return _makeTaggedRef2i(tag,ptr);
}



/*
 * UNTAGGING
 *
 */


/* 
 * The C++ standard does not specify whether shifting right negative values
 * means shift logical or shift arithmetical. So we test what this C++ compiler
 * does.
 *
 * CS: I guess this has always been broken (it's always true!)
 *
 */

#define WE_DO_ARITHMETIC_SHIFTS (1||(-1>>1) == -1)

inline TaggedRef * tagged2Ref(TaggedRef ref) {
  Assert(oz_isRef(ref));
  return _tagged2Ref(ref);
}
inline OzVariable * tagged2Var(TaggedRef ref) {
  Assert(oz_isVar(ref));
  return (OzVariable *) tagValueOf2(LTAG_VAR0,ref);
}
inline SRecord * tagged2SRecord(TaggedRef ref) {
  Assert(oz_isSRecord(ref));
  return (SRecord *) tagValueOf2(LTAG_SRECORD0,ref);
}
inline LTuple * tagged2LTuple(TaggedRef ref) {
  Assert(oz_isLTuple(ref));
  return (LTuple *) tagValueOf2(LTAG_LTUPLE0,ref);
}
inline Literal * tagged2Literal(TaggedRef ref) {
  Assert(oz_isLiteral(ref));
  return (Literal *) tagValueOf2(LTAG_LITERAL,ref);
}
inline ConstTerm * tagged2Const(TaggedRef ref) {
  Assert(oz_isConst(ref));
  return (ConstTerm *) tagValueOf2(LTAG_CONST0,ref);
}
inline void * tagged2UnmarkedPtr(TaggedRef ref) {
  Assert(oz_isMark(ref));
  return (void *) tagValueOf2(LTAG_MARK0,ref);
}

inline int tagged2UnmarkedInt(TaggedRef t) {
  Assert(oz_isMark(t));
  int help = (int) t;

  if (WE_DO_ARITHMETIC_SHIFTS) {
    return (help>>LTAG_BITS);
  } else {
    return (help >= 0) ? help >> LTAG_BITS
                       : ~(~help >> LTAG_BITS);
  }
}

inline int tagged2SmallInt(TaggedRef t) {
  Assert(oz_isSmallInt(t));
  int help = (int) t;

  if (WE_DO_ARITHMETIC_SHIFTS) {
    return (help>>LTAG_BITS);
  } else {
    return (help >= 0) ? help >> LTAG_BITS
                       : ~(~help >> LTAG_BITS);
  }
}



/*
 * TAGGING
 *
 */

inline TaggedRef makeTaggedRef(TaggedRef * s) {
  Assert(s != NULL); 
  return _makeTaggedRef(s);
}
inline TaggedRef makeTaggedVar(OzVariable *s) {
  Assert(s != NULL && isSTAligned(s));
  return makeTaggedRef2p(LTAG_VAR0, s);
}
// For the GenTraverser (non-relocatable pointers are masqueraded as
// tagged variables);
inline TaggedRef makePseudoTaggedVar(void *p) {
  Assert(p != NULL && isSTAligned(p));
  return makeTaggedRef2p(LTAG_VAR0, p);
}
inline TaggedRef makeTaggedLTuple(LTuple *s) {
  Assert(s != NULL && isSTAligned(s));
  return makeTaggedRef2p(LTAG_LTUPLE0,s);
}
inline TaggedRef makeTaggedSRecord(SRecord *s) {
  Assert(s != NULL && isSTAligned(s));
  return makeTaggedRef2p(LTAG_SRECORD0,s);
}
inline TaggedRef makeTaggedLiteral(Literal *s) {
  Assert(s != NULL && isLTAligned(s));
  return makeTaggedRef2p(LTAG_LITERAL,s);
}
inline TaggedRef makeTaggedConst(ConstTerm *s) {
  Assert(s != NULL && isSTAligned(s));
  return makeTaggedRef2p(LTAG_CONST0,s);
}
inline TaggedRef makeTaggedMarkPtr(void * s) {
  Assert(isSTAligned(s));
  return makeTaggedRef2p(LTAG_MARK0,s);
}
inline TaggedRef makeTaggedMarkInt(int si) {
  Assert(tagged2UnmarkedInt(_makeTaggedMarkInt(si)) == si);
  return (_makeTaggedMarkInt(si));
}
// e.g. gentraverser needs static constants;
#define makeTaggedMarkIntNOTEST(si)	_makeTaggedMarkInt(si)

inline TaggedRef makeTaggedSmallInt(int s) {
  Assert(s >= OzMinInt && s <= OzMaxInt);
  return _makeTaggedSmallInt(s);
}


#define makeTaggedNULL()       ((TaggedRef) 0)
#define taggedVoidValue        _makeTaggedSmallInt(0)


/*
 * DEREF
 *
 */

#define _DEREF(term, termPtr)		               \
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

inline
TaggedRef oz_deref(TaggedRef t) {
  DEREF0(t,_1);
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
    if (oz_isVar(t)) t=makeTaggedRef(sp);          
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

inline 
TaggedRef tagged2NonVariable(TaggedRef * t) {
  return oz_isVar(*t) ? makeTaggedRef(t) : *t;
}

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

#ifdef DEBUG_CHECK

inline Bool oz_eq(TaggedRef t1, TaggedRef t2) {
  Assert(t1==oz_safeDeref(t1));
  Assert(t2==oz_safeDeref(t2));
  return t1==t2;
}

#else

#define oz_eq(t1,t2) (((TaggedRef) (t1))==((TaggedRef) (t2)))

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


