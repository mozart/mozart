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

  // oz_isVariable checks for last two bits.
  TAG_UNUSED_UVAR   =  1,   // 0001
  TAG_UNUSED_SVAR   =  9,   // 1001
  TAG_VAR       =  5,   // 0101
  TAG_GCMARK    = 13,   // 1101    --> !!! oz_isVariable(TAG_GCMARK) = 1 !!!
	    
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
#define makeGCTaggedInt(i) ((i << TAG_SIZE) | TAG_GCMARK)
#define getGCTaggedInt(t)  ((int32) (t >> TAG_SIZE))
#define isGCTaggedInt(t)   (_tagTypeOf(t) == TAG_GCMARK)

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


/*
 * Debugging of GC_TAG
 *
 */

#ifdef DEBUG_GC  

extern Bool isCollecting;

#define GCDEBUG(X) \
  if (!isCollecting && (_tagTypeOf(X)==TAG_GCMARK))	\
   OZ_error("GcTag unexpectedly found.");
#else

#define GCDEBUG(X)

#endif


#ifdef DEBUG_CHECK

inline void * tagValueOf(TaggedRef ref) { 
  GCDEBUG(ref); return _tagValueOf(ref);
}
inline void * tagValueOf2(TypeOfTerm tag, TaggedRef ref) {
  GCDEBUG(ref); return _tagValueOf2(tag,ref);
}
inline void * tagValueOfVerbatim(TaggedRef ref) { 
  GCDEBUG(ref); return _tagValueOfVerbatim(ref);
}
inline TypeOfTerm tagTypeOf(TaggedRef ref) { 
  GCDEBUG(ref); return _tagTypeOf(ref);
}
inline TaggedRef makeTaggedRef2i(TypeOfTerm tag, int32 i) {
  Assert((i&3) == 0); return _makeTaggedRef2(tag,i);
}
inline TaggedRef makeTaggedRef2p(TypeOfTerm tag, void *ptr) {
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


/*
 * Consistency checks
 *
 */


#define CHECK_NONVAR(term) Assert(oz_isRef(term) || !oz_isVariable(term))
#define CHECK_ISVAR(term)  Assert(oz_isVariable(term))
#define CHECK_DEREF(term)  Assert(!oz_isRef(term) && !oz_isVariable(term))
#define CHECK_POINTER(s)   Assert(!(ToInt32(s) & 3))
#define CHECK_POINTER_N(s) Assert(s != NULL && !(ToInt32(s) & 3))
#define CHECK_STRPTR(s)    Assert(s != NULL)
#define CHECK_TAG(Tag)     Assert(tagTypeOf(ref) == Tag)


/*
 * TAG_REF
 *
 */

#ifdef DEBUG_CHECK

inline TaggedRef makeTaggedRef(TaggedRef * s) {
  CHECK_POINTER_N(s); return _makeTaggedRef(s);
}
inline Bool oz_isRef(TaggedRef term) {
  GCDEBUG(term); return _isRef(term);
}
inline TaggedRef * tagged2Ref(TaggedRef ref) {
  GCDEBUG(ref); Assert(oz_isRef(ref));
  return _tagged2Ref(ref);
}

#else

#define makeTaggedRef(s) _makeTaggedRef(s)
#define oz_isRef(t)      _isRef(t)
#define tagged2Ref(ref)  _tagged2Ref(ref)

#endif




/*
 * TAG TESTS
 *
 */


// The following tests are specially optimized
#define _isLTuple(val)      (((TaggedRef) val&13)==0)      /* mask = 1101 */
#define _isVariable(val)    (((TaggedRef) val&2)==0)       /* mask = 0010 */

// This is the normal tag test
#define _hasTag(term,tag)   (tagTypeOf(term)==(tag))



#ifdef DEBUG_CHECK

inline Bool isVariableTag(TypeOfTerm tag) { 
  return _isVariable(tag);
}
inline Bool isVarTag(TypeOfTerm tag) { 
  return tag == TAG_VAR; 
}
inline Bool isLTupleTag(TypeOfTerm tag) {   
  return _isLTuple(tag);
}
inline Bool isLiteralTag(TypeOfTerm tag) {  
  return tag==TAG_LITERAL;
}
inline Bool isSRecordTag(TypeOfTerm tag) {
  return tag==TAG_SRECORD;
}
inline Bool isSmallIntTag(TypeOfTerm tag) {
  return tag==TAG_SMALLINT;
}
inline Bool isConstTag(TypeOfTerm tag) {
  return tag==TAG_CONST;
}
inline Bool isGcMarkTag(TypeOfTerm tag) {
  return tag==TAG_GCMARK;
}

#else

#define isVariableTag(tag)  _isVariable(tag)
#define isVarTag(tag)      ((tag) == TAG_VAR)
#define isLTupleTag(tag)    _isLTuple(tag)
#define isLiteralTag(tag)   ((tag)==TAG_LITERAL)
#define isSRecordTag(tag)   ((tag)==TAG_SRECORD)
#define isSmallIntTag(tag)  ((tag)==TAG_SMALLINT)
#define isConstTag(tag)     ((tag)==TAG_CONST)
#define isGcMarkTag(tag)    ((tag)==TAG_GCMARK)

#endif




/*
 * VALUE TESTS
 *
 */

#ifdef DEBUG_CHECK

inline Bool oz_isVariable(TaggedRef term) {
  GCDEBUG(term); Assert(!oz_isRef(term)); return _isVariable(term);
}
inline Bool oz_isVar(TaggedRef term) {
  GCDEBUG(term); Assert(!oz_isRef(term)); return _hasTag(term,TAG_VAR);
}
inline Bool oz_isLTuple(TaggedRef term) {
  GCDEBUG(term); return _isLTuple(term);
}
inline Bool oz_isLiteral(TaggedRef term) {
  GCDEBUG(term); return _hasTag(term,TAG_LITERAL);
}
inline Bool oz_isSRecord(TaggedRef term) {
  GCDEBUG(term); return _hasTag(term,TAG_SRECORD);
}
inline Bool oz_isSmallInt(TaggedRef term) {
  return _hasTag(term,TAG_SMALLINT);
}
inline Bool oz_isConst(TaggedRef term) {
  GCDEBUG(term); return _hasTag(term,TAG_CONST);
}
inline Bool oz_isGcMark(TaggedRef term) {
  return _hasTag(term,TAG_GCMARK);
}

#else

#define oz_isVariable(term)    _isVariable(term)
#define oz_isVar(term)         _hasTag(term,TAG_VAR)
#define oz_isLTuple(term)      _isLTuple(term)
#define oz_isLiteral(term)     _hasTag(term,TAG_LITERAL)
#define oz_isSRecord(term)     _hasTag(term,TAG_SRECORD)
#define oz_isSmallInt(term)    _hasTag(term,TAG_SMALLINT)
#define oz_isConst(term)       _hasTag(term,TAG_CONST)
#define oz_isGcMark(term)      _hasTag(term,TAG_GCMARK)

#endif

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

#ifdef DEBUG_CHECK

inline OzVariable * tagged2Var(TaggedRef ref) {
  GCDEBUG(ref); CHECK_TAG(TAG_VAR);
  return (OzVariable *) tagValueOf2(TAG_VAR,ref);
}
inline SRecord * tagged2SRecord(TaggedRef ref) {
  GCDEBUG(ref); CHECK_TAG(TAG_SRECORD);
  return (SRecord *) tagValueOf2(TAG_SRECORD,ref);
}
inline LTuple * tagged2LTuple(TaggedRef ref) {
  GCDEBUG(ref); CHECK_TAG(TAG_LTUPLE);
  return (LTuple *) tagValueOf2(TAG_LTUPLE,ref);
}
inline Literal * tagged2Literal(TaggedRef ref) {
  GCDEBUG(ref); CHECK_TAG(TAG_LITERAL);
  return (Literal *) tagValueOf2(TAG_LITERAL,ref);
}
inline ConstTerm * tagged2Const(TaggedRef ref) {
  GCDEBUG(ref); CHECK_TAG(TAG_CONST);
  return (ConstTerm *) tagValueOf2(TAG_CONST,ref);
}
inline void * tagged2GcUnmarked(TaggedRef ref) {
  CHECK_TAG(TAG_GCMARK);
  return (void *) tagValueOf2(TAG_GCMARK,ref);
}

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

inline TaggedRef tagged2NonVariable(TaggedRef *term) {
  GCDEBUG(*term);
  TaggedRef ret = *term;
  if (_isVariable(ret) && !oz_isRef(ret)) {
    ret = makeTaggedRef(term);
  }
  return ret;
}


#else


#define tagged2Var(ref) \
  ((OzVariable *) tagValueOf2(TAG_VAR,((TaggedRef) (ref))))
#define tagged2SRecord(ref) \
  ((SRecord *) tagValueOf2(TAG_SRECORD,((TaggedRef) (ref))))
#define tagged2LTuple(ref) \
  ((LTuple *) tagValueOf2(TAG_LTUPLE,((TaggedRef) (ref))))
#define tagged2Literal(ref) \
  ((Literal *) tagValueOf2(TAG_LITERAL,((TaggedRef) (ref))))
#define tagged2Const(ref) \
  ((ConstTerm *) tagValueOf2(TAG_CONST,((TaggedRef) (ref))))
#define tagged2GcUnmarked(ref) \
  ((void *) tagValueOf2(TAG_GCMARK,((TaggedRef) (ref))))

#if WE_DO_ARITHMETIC_SHIFTS

#define tagged2SmallInt(t) ((int (t))>>TAG_SIZE)

#else

#define tagged2SmallInt(t) (((int (t)) >= 0) ? ((int (t)) >> TAG_SIZE) : ~(~(int (t)) >> TAG_SIZE))

#endif

#define tagged2NonVariable(t) \
          ((_isVariable(*(t)) && !oz_isRef(*(t))) ? makeTaggedRef(t) : *(t))


#endif

/* 
 * The C++ standard does not specify whether shifting right negative values
 * means shift logical or shift arithmetical. So we test what this C++ compiler
 * does.
 *
 */


/*
 * TAGGING
 *
 */

#ifdef DEBUG_CHECK

inline TaggedRef makeTaggedVar(OzVariable *s) {
  CHECK_POINTER_N(s); return makeTaggedRef2p(TAG_VAR, s);
}
inline TaggedRef makeTaggedLTuple(LTuple *s) {
  CHECK_POINTER_N(s); return makeTaggedRef2p(TAG_LTUPLE,s);
}
inline TaggedRef makeTaggedSRecord(SRecord *s) {
  CHECK_POINTER_N(s); return makeTaggedRef2p(TAG_SRECORD,s);
}
inline TaggedRef makeTaggedLiteral(Literal *s) {
  CHECK_POINTER_N(s); return makeTaggedRef2p(TAG_LITERAL,s);
}
inline TaggedRef makeTaggedConst(ConstTerm *s) {
  CHECK_POINTER_N(s); return makeTaggedRef2p(TAG_CONST,s);
}
inline TaggedRef makeTaggedGcMark(void * s) {
  return makeTaggedRef2p(TAG_GCMARK,s);
}

inline TaggedRef makeTaggedSmallInt(int s) {
  Assert(s >= OzMinInt && s <= OzMaxInt);
  return _makeTaggedSmallInt(s);
}

inline TaggedRef makeTaggedMiscp(void * s) {
  return makeTaggedRef2p((TypeOfTerm)0,s);
}

#else

#define makeTaggedVar(s)       makeTaggedRef2p(TAG_VAR,      s)
#define makeTaggedLTuple(s)    makeTaggedRef2p(TAG_LTUPLE,    s)
#define makeTaggedSRecord(s)   makeTaggedRef2p(TAG_SRECORD,   s)
#define makeTaggedLiteral(s)   makeTaggedRef2p(TAG_LITERAL,   s)
#define makeTaggedConst(s)     makeTaggedRef2p(TAG_CONST,     s)
#define makeTaggedGcMark(s)    makeTaggedRef2p(TAG_GCMARK,    s)
#define makeTaggedSmallInt(s)  _makeTaggedSmallInt(s) 
#define makeTaggedMiscp(s)     makeTaggedRef2p((TypeOfTerm)0, s)

#endif


#define makeTaggedNULL()       ((TaggedRef) 0)
#define taggedVoidValue        _makeTaggedSmallInt(0)
#define taggedInvalidVar       makeTaggedRef2p(TAG_VAR, (TaggedRef *) -1)



/*
 * Construction of variables and refererences
 *
 */

inline TaggedRef * newTaggedRef(TaggedRef *t) {
  TaggedRef *ref = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ref = makeTaggedRef(t);
  return ref;
}

inline TaggedRef * newTaggedOptVar(TaggedRef proto) {
  Assert(proto != taggedInvalidVar);
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
 * DEREF
 *
 */

#define __DEREF(term, termPtr, tag)			\
  while(oz_isRef(term)) {				\
    termPtr = tagged2Ref(term);				\
    term = *termPtr;					\
  }							\
  tag = tagTypeOf(term);

#define _DEREF(term, termPtr, tag)		\
  register TypeOfTerm tag;			\
   __DEREF(term, termPtr, tag);


#define DEREF(term, termPtr, tag)		\
  register TaggedRef *termPtr = NULL;		\
  _DEREF(term,termPtr,tag);

#define DEREF0(term, termPtr, tag)		\
  register TaggedRef *termPtr;			\
  _DEREF(term,termPtr,tag);

#define DEREFPTR(term, termPtr, tag)		\
  register TaggedRef term = *termPtr;		\
  _DEREF(term,termPtr,tag);

#define SAFE_DEREF(term)				\
if (oz_isRef(term)) {					\
  DEREF(term,SAFE__PTR__,SAFE__TAG__);			\
  if (oz_isVariable(term)) term=makeTaggedRef(SAFE__PTR__);	\
}

inline
TaggedRef oz_deref(TaggedRef t) {
  DEREF(t,_1,_2);
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
  DEREF(t,tptr,_2);
  return (TaggedRef) tptr;
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

/*===================================================================
 * 
 *=================================================================== */

// 
// identity test
//
#ifdef DEBUG_CHECK

inline Bool oz_eq(TaggedRef t1, TaggedRef t2) {
  Assert(t1==oz_safeDeref(t1));
  Assert(t2==oz_safeDeref(t2));
  return t1==t2;
}

#else

#define oz_eq(t1,t2) (((TaggedRef) (t1))==((TaggedRef) (t2)))

#endif


/*===================================================================
 * Tagged Pointer classes
 *=================================================================== */

/*
 * class Tagged2:
 *  32 bit word to store
 *    word aligned pointer + 2 tag bits or
 *    30 bit value + 2 tag bits
 */

#define tagged2Mask 3
#define tagged2Bits 2
class Tagged2
{
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

#define DerefIfVarDo(v,Block)			\
 if (oz_isRef(v)) {				\
   v=oz_safeDeref(v);				\
   if (oz_isRef(v)) { Block; }			\
 }

#define DerefIfVarReturnIt(v)   DerefIfVarDo(v, return v);
#define DerefIfVarSuspend(v)    DerefIfVarDo(v, return SUSPEND);


#endif


