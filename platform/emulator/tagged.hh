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

#include <stdio.h>

#include "base.hh"
#include "mem.hh"
#include "atoms.hh"

/*
 * TAGS
 *
 */

enum TypeOfTerm {
  TAG_REF       =  0,   // 0000
  TAG_REF2      =  4,   // 0100
  TAG_REF3      =  8,   // 1000
  TAG_REF4      = 12,   // 1100

  TAG_UNUSED_UVAR   =  1,   // 0001
  TAG_UNUSED_SVAR   =  9,   // 1001
  TAG_VAR       =  5,   // 0101
  TAG_GCMARK    = 13,   // 1101

  TAG_LTUPLE    =  2,   // 0010
  TAG_UNUSED_FSETVALUE = 14,   // 1110
  TAG_SRECORD   =  3,   // 0011

  TAG_LITERAL   = 15,   // 1111

  TAG_CONST     = 10,   // 1010

  TAG_SMALLINT  =  6,   // 0110
  TAG_UNUSED_EXT       =  7,   // 0111
  TAG_UNUSED_FLOAT     = 11    // 1011
};


#define TAG_SIZE      4
#define TAG_MASK      0xF
#define TAG_LOWERMASK 3
#define TAG_PTRBITS   2

// '!'
#define TAG_NEW_MASK    0x7
//
// double- and singe-word aligned references.
#define isDWAligned(ptr)  (((int32)ToInt32(ptr) & TAG_NEW_MASK) == TAG_REF)
#define isSWAligned(ptr)  (((int32)ToInt32(ptr) & TAG_NEW_MASK) == TAG_REF2)

/*
 * Basic macros
 *
 */

#define _tagTypeOf(ref)          ((TypeOfTerm)(ref&TAG_MASK))

#define TaggedToPointer(t)       ((void*) (mallocBase|t))

#define _tagValueOf2(tag,ref)    TaggedToPointer(((ref)>>TAG_PTRBITS) - ((tag)>>TAG_PTRBITS))
#define _tagValueOf(ref)         TaggedToPointer(((ref)>>TAG_PTRBITS)&~TAG_LOWERMASK)
#define _tagValueOfVerbatim(ref) ((void*)(((ref) >> TAG_PTRBITS)&~TAG_LOWERMASK))
#define _makeTaggedRef2(tag,i)   ((i << TAG_PTRBITS) | tag)

#define _makeTaggedRef2i(tag,ptr) _makeTaggedRef2(tag,(int32)ToInt32(ptr))


/* small ints are the only TaggedRefs that do not
 * contain a pointer in the value part */
#define _makeTaggedSmallInt(s) ((s << TAG_SIZE) | TAG_SMALLINT)
// kost@ : both generic traverser and builder exploit 'TAG_GCMARK':

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

#define OzMaxInt (INT_MAX>>TAG_SIZE)
#define OzMinInt (-(OzMaxInt+1))

#endif

#define TaggedOzMaxInt _makeTaggedSmallInt(OzMaxInt)
#define TaggedOzMinInt _makeTaggedSmallInt(OzMinInt)


inline void * tagValueOf(TaggedRef ref) {
  return _tagValueOf(ref);
}
inline void * tagValueOf2(TypeOfTerm tag, TaggedRef ref) {
  return _tagValueOf2(tag,ref);
}
inline void * tagValueOfVerbatim(TaggedRef ref) {
  return _tagValueOfVerbatim(ref);
}
inline TypeOfTerm tagTypeOf(TaggedRef ref) {
  return _tagTypeOf(ref);
}
inline TaggedRef makeTaggedRef2i(TypeOfTerm tag, int32 i) {
  Assert((i&3) == 0); return _makeTaggedRef2(tag,i);
}
inline TaggedRef makeTaggedRef2p(TypeOfTerm tag, void *ptr) {
  return _makeTaggedRef2i(tag,ptr);
}


// This is the normal tag test
#define _hasTag(term,tag)   (tagTypeOf(term)==(tag))


/*
 * VALUE TESTS
 *
 */

inline Bool oz_isRef(TaggedRef term) {
  return _isRef(term);
}
inline Bool oz_isVar(TaggedRef term) {
  return _hasTag(term,TAG_VAR);
}
inline Bool oz_isLTuple(TaggedRef term) {
  return _hasTag(term,TAG_LTUPLE);
}
inline Bool oz_isLiteral(TaggedRef term) {
  return _hasTag(term,TAG_LITERAL);
}
inline Bool oz_isSRecord(TaggedRef term) {
  return _hasTag(term,TAG_SRECORD);
}
inline Bool oz_isSmallInt(TaggedRef term) {
  return _hasTag(term,TAG_SMALLINT);
}
inline Bool oz_isConst(TaggedRef term) {
  return _hasTag(term,TAG_CONST);
}
inline Bool oz_isGcMark(TaggedRef term) {
  return _hasTag(term,TAG_GCMARK);
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
  return (OzVariable *) tagValueOf2(TAG_VAR,ref);
}
inline SRecord * tagged2SRecord(TaggedRef ref) {
  Assert(oz_isSRecord(ref));
  return (SRecord *) tagValueOf2(TAG_SRECORD,ref);
}
inline LTuple * tagged2LTuple(TaggedRef ref) {
  Assert(oz_isLTuple(ref));
  return (LTuple *) tagValueOf2(TAG_LTUPLE,ref);
}
inline Literal * tagged2Literal(TaggedRef ref) {
  Assert(oz_isLiteral(ref));
  return (Literal *) tagValueOf2(TAG_LITERAL,ref);
}
inline ConstTerm * tagged2Const(TaggedRef ref) {
  Assert(oz_isConst(ref));
  return (ConstTerm *) tagValueOf2(TAG_CONST,ref);
}
inline void * tagged2UnmarkedPtr(TaggedRef ref) {
  Assert(oz_isGcMark(ref));
  return (void *) tagValueOf2(TAG_GCMARK,ref);
}
#define tagged2UnmarkedInt(t) ((int32) ((t) >> TAG_SIZE))

inline int tagged2SmallInt(TaggedRef t) {
  Assert(oz_isSmallInt(t));
  int help = (int) t;

  if (WE_DO_ARITHMETIC_SHIFTS) {
    return (help>>TAG_SIZE);
  } else {
    return (help >= 0) ? help >> TAG_SIZE
                       : ~(~help >> TAG_SIZE);
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
  Assert(s != NULL && oz_isHeapAligned(s));
  return makeTaggedRef2p(TAG_VAR, s);
}
inline TaggedRef makeTaggedLTuple(LTuple *s) {
  Assert(s != NULL && oz_isHeapAligned(s));
  return makeTaggedRef2p(TAG_LTUPLE,s);
}
inline TaggedRef makeTaggedSRecord(SRecord *s) {
  Assert(s != NULL && oz_isHeapAligned(s));
  return makeTaggedRef2p(TAG_SRECORD,s);
}
inline TaggedRef makeTaggedLiteral(Literal *s) {
  Assert(s != NULL && oz_isDoubleHeapAligned(s));
  return makeTaggedRef2p(TAG_LITERAL,s);
}
inline TaggedRef makeTaggedConst(ConstTerm *s) {
  Assert(s != NULL && oz_isHeapAligned(s));
  return makeTaggedRef2p(TAG_CONST,s);
}
inline TaggedRef makeTaggedMarkPtr(void * s) {
  Assert(oz_isHeapAligned(s));
  return makeTaggedRef2p(TAG_GCMARK,s);
}
#define makeTaggedMarkInt(s) (((s) << TAG_SIZE) | TAG_GCMARK)

inline TaggedRef makeTaggedSmallInt(int s) {
  Assert(s >= OzMinInt && s <= OzMaxInt);
  return _makeTaggedSmallInt(s);
}
inline TaggedRef makeTaggedVerbatim(void * s) {
  return makeTaggedRef2p((TypeOfTerm)0,s);
}


#define makeTaggedNULL()       ((TaggedRef) 0)
#define taggedVoidValue        _makeTaggedSmallInt(0)


/*
 * DEREF
 *
 */

#define _DEREF(term, termPtr)                          \
  while(oz_isRef(term)) {                              \
    termPtr = tagged2Ref(term);                        \
    term    = *termPtr;                                \
  }

#define DEREF(term, termPtr)                           \
  register TaggedRef *termPtr = NULL;                  \
  _DEREF(term,termPtr);

#define DEREF0(term, termPtr)                          \
  register TaggedRef *termPtr;                         \
  _DEREF(term,termPtr);

#define DEREFPTR(term, termPtr)                        \
  register TaggedRef term = *termPtr;                  \
  _DEREF(term,termPtr);

#define SAFE_DEREF(term)                               \
if (oz_isRef(term)) {                                  \
  DEREF(term,SAFE__PTR__);                             \
  if (oz_isVar(term)) term=makeTaggedRef(SAFE__PTR__); \
}

inline
TaggedRef oz_deref(TaggedRef t) {
  DEREF(t,_1);
  return t;
}

inline
TaggedRef oz_derefOne(TaggedRef t) {
  Assert(oz_isRef(t));
  return *tagged2Ref(t);
}

inline
TaggedRef oz_safeDeref(TaggedRef t) {
  SAFE_DEREF(t);
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

inline TaggedRef * newTaggedRef(TaggedRef *t) {
  TaggedRef *ref = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ref = makeTaggedRef(t);
  return ref;
}

inline TaggedRef * newTaggedOptVar(TaggedRef proto) {
  TaggedRef *ref = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ref = proto;
  return (ref);
}

inline TaggedRef * newTaggedVar(OzVariable * c) {
  TaggedRef *ref = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ref = makeTaggedVar(c);
  return ref;
}



/*
 * RefsArray
 *
 */

typedef TaggedRef * RefsArray;

#ifdef DEBUG_CHECK

inline void setRefsArraySize(RefsArray a, int32 n) {
  a[-1] = n;
}
inline int getRefsArraySize(RefsArray a) {
  return a[-1];
}

#else

#define setRefsArraySize(a,n) ((a)[-1] = (n))
#define getRefsArraySize(a)   ((a)[-1])

#endif


#if defined(DEBUG_CHECK) && defined(WINDOWS)
static
#else
inline
#endif
Bool initRefsArray(RefsArray a, int size, Bool init) {

  setRefsArraySize(a,size);
  register TaggedRef nvr = NameVoidRegister;
  if (init) {
    switch (size) {
    case 10: a[9] = nvr;
    case  9: a[8] = nvr;
    case  8: a[7] = nvr;
    case  7: a[6] = nvr;
    case  6: a[5] = nvr;
    case  5: a[4] = nvr;
    case  4: a[3] = nvr;
    case  3: a[2] = nvr;
    case  2: a[1] = nvr;
    case  1: a[0] = nvr;
      break;
    default:
      {
        for(int i = size-1; i >= 0; i--)
          a[i] = nvr;
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
  RefsArray a = ((RefsArray) oz_freeListMalloc((n+1) * sizeof(TaggedRef)));
  a += 1;
  initRefsArray(a,n,init);
  return a;
}

inline
void disposeRefsArray(RefsArray a)
{
  if (a) {
    int sz = getRefsArraySize(a);
    a -= 1;
    oz_freeListDispose(a, (sz+1) * sizeof(TaggedRef));
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
RefsArray copyRefsArray(RefsArray a, int n)  {
  return (RefsArray) memcpy(allocateRefsArray(n), a, n * sizeof(TaggedRef));
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
 * Tagged Pointer class
 *
 * class Tagged2:
 *  32 bit word to store
 *    word aligned pointer + 2 tag bits or
 *    30 bit value + 2 tag bits
 *
 */

#define tagged2Mask 3
#define tagged2Bits 2

class Tagged2 {
private:
  uint32 tagged;
  void checkTag(int tag)       { Assert(tag >=0 && tag <=tagged2Mask); }
  void checkVal(uint32 val)    { Assert((val & (tagged2Mask<<(32-tagged2Bits))) == 0); }
  void checkPointer(void* ptr) { Assert((((uint32) ptr)&tagged2Mask) == 0); }
public:

  int     getTag()  { return (tagged&tagged2Mask); }
  uint32  getData() { return tagged>>tagged2Bits; }
  void*   getPtr()  { return (void*)(tagged&~tagged2Mask); }

  void set(void* ptr,int tag) {
    checkPointer(ptr);
    checkTag(tag);
    tagged = ((uint32)ptr) | tag;
  }
  void set(uint32 val,int tag) {
    checkTag(tag);
    checkVal(val);
    tagged = (val<<tagged2Bits) | tag;
  }
  void setPtr(void* ptr) {
    checkPointer(ptr);
    tagged = ((uint32)ptr) | getTag();
  }
  void setTag(int tag) {
    checkTag(tag);
    tagged = (tagged & ~tagged2Mask) | tag;
  }
  void borTag(int tag) {
    checkTag(tag);
    tagged = tagged | tag;
  }
  void bandTag(int tag) {
    checkTag(tag & tagged2Mask);
    tagged = tagged & (tag | ~tagged2Mask);
  }
  void setVal(uint32 val) {
    checkVal(val);
    tagged = (val<<tagged2Bits) | getTag();
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
