/*
 * FBPS Saarbr"ucken
 * Author: mehl
 *
 * Values: literal, list, records
 */

#ifndef __VALUEHH
#define __VALUEHH

#ifdef INTERFACE
#pragma interface
#endif

#if defined(OUTLINE)
#define INLINE
#else
#define INLINE inline
#endif

/*===================================================================
 * global names and atoms
 *=================================================================== */
extern TaggedRef AtomNil, AtomCons, AtomPair, AtomVoid,
       AtomSucceeded, AtomAlt, AtomMerged, AtomFailed,
       AtomEntailed, AtomSuspended, AtomBlocked,
       AtomEmpty, AtomUpper, AtomLower, AtomDigit,
       AtomCharSpace, AtomPunct, AtomOther,
       NameTrue, NameFalse, AtomBool, AtomSup, AtomCompl,
       AtomMin, AtomMax, AtomMid, AtomLow,
       AtomNaive, AtomSize, AtomNbSusps,

       NameOoFreeFlag,NameOoAttr,NameOoFreeFeatR,NameOoUnFreeFeat,
       NameOoFastMeth,NameOoDefaults,NameOoRequiredArg,NameOoDefaultVar,
       NameOoPrintName,NameOoLocking,

       NameUnit,
       AtomKinded, AtomDet, AtomRecord, AtomFSet,
       // Atoms for System.get and System.set
       AtomActive, AtomAtoms, AtomBuiltins, AtomCommitted,
       AtomMoreInfo, AtomTotal,
       AtomCloned, AtomCode, AtomCopy, AtomCreated, AtomDebug, AtomDepth,
       AtomFeed, AtomForeign, AtomFree, AtomFreelist, AtomGC, AtomHigh,
       AtomHints, AtomIdle, AtomInt, AtomInvoked, AtomLimits, AtomLoad,
       AtomLocation, AtomMedium, AtomNames, AtomOn, AtomPropagate,
       AtomPropagators, AtomRun, AtomRunnable, AtomShowSuspension,
       AtomStopOnToplevelFailure, AtomSystem, AtomThread,
       AtomThreshold, AtomTolerance, AtomUser, AtomVariables, AtomWidth,
       AtomHeap, AtomDebugIP, AtomDebugPerdio,
  RecordFailure,
  E_ERROR, E_KERNEL, E_OBJECT, E_TK, E_OS, E_SYSTEM,
BI_send;

/*===================================================================
 * Literal
 *=================================================================== */


/* any combination iof the following must be different from GCTAG,
 * otherwise getRef() will not work
 */
#define Lit_isName        2
#define Lit_isNamedName   4
#define Lit_hasGName      8
#define Lit_isUniqueName 16

const int sizeOfLitFlags = 5;
const int litFlagsMask   = (1<<sizeOfLitFlags)-1;

class Literal {
  int32 flagsAndOthers;
public:
  void init() { flagsAndOthers=0; }
  void setFlag(int flag) { flagsAndOthers |= flag; }
  int getFlags() { return (flagsAndOthers&litFlagsMask); }
  int getOthers() { return flagsAndOthers>>sizeOfLitFlags; }
  void setOthers(int value) { flagsAndOthers = getFlags()|(value<<sizeOfLitFlags); }

  Bool isName()      { return (getFlags()&Lit_isName); }
  Bool isNamedName() { return (getFlags()&Lit_isNamedName); }
  Bool isAtom()      { return !isName(); }

  Literal() { Assert(0); }

  char *getPrintName();

  Literal *gc();

  TaggedRef *getRef() { return (TaggedRef*)&flagsAndOthers; }
  OZPRINT;
  OZPRINTLONG;

  inline unsigned int hash();
};

class Atom: public Literal {
private:
  char *printName;
public:
  static Atom *newAtom(char *str);
  char* getPrintName() { return printName; }
  int getSize() { return getOthers(); }
  unsigned int hash() { return ToInt32(getPrintName()); }
};


/* This one goes onto the heap */
class Name: public Literal {
protected:
  static int NameCurrentNumber;
  int32 homeOrGName;
public:
  Name() { Assert(0); }
  static Name *newName(Board *b);

  Board *getBoard(); // see am.icc

  int getSeqNumber() { return getOthers(); }
  unsigned int hash() { return getSeqNumber(); }

  Bool isOnHeap() { return (getFlags()&Lit_isNamedName)==0; }
  Bool hasGName() { return (getFlags()&Lit_hasGName); }

  Name *gcName();
  void gcRecurse();

  GName *getGName() {
    return hasGName() ? (GName*) ToPointer(homeOrGName) : globalize();
  }
  GName *globalize();
  void import(GName *);
};


/* This one managed via malloc */

class NamedName: public Name {
public:
  char *printName;
  static NamedName *newNamedName(char *str);
};


unsigned int Literal::hash()
{
  if (isAtom()) return ((Atom*)this)->hash();
  return ((Name*)this)->hash();
}

inline
Bool isAtom(TaggedRef term) {
  return isLiteral(tagTypeOf(term)) && tagged2Literal(term)->isAtom();
}


//mm2: one argument is guarantueed to be a literal ???
inline
Bool literalEq(TaggedRef a, TaggedRef b)
{
  Assert(isLiteral(a) || isLiteral(b));
  return (a==b);
}

/*
 * mm2: how should this function handle names ?
 *
 * atomcmp(a,b) is used to construct the arity lists of records.
 * It returns: 0   if   a == b
 *            -1   if   a < b
 *             1   if   a > b
 *
 * Note: if a and b are atoms (not names) than they MUST be compared via
 * strcmp, otherwise the compiler must be changed too!!!!!!
 *
 */

inline
int atomcmp(Literal *a, Literal *b)
{
  if (a==b) return 0;

  int res = strcmp(a->getPrintName(), b->getPrintName());
  if (res < 0) return -1;
  if (res > 0) return  1;

  Assert(a->isName() && b->isName());
  return (((Name*)a)->getSeqNumber() < ((Name*)b)->getSeqNumber()) ? -1 : 1;
}


inline
int atomcmp(TaggedRef a, TaggedRef b)
{
  if (a==b) return 0;

  return atomcmp(tagged2Literal(a), tagged2Literal(b));
}



/*===================================================================
 * Numbers
 *=================================================================== */

#include <math.h>
#include <limits.h>

extern "C" {
#include "../include/gmp.h"
}


/*===================================================================
 * SmallInt
 *=================================================================== */

/* ----------------------------------------------------------------------
   SmallInts represented as follows
   Gxx...xxTTTT

   G = GC bit, T = tag bits, x = value bits (left shifted value)
   ----------------------------------------------------------------------

   Search for string "INTDEP" to see procedures that depend on it !!

*/


#ifdef XGNUWIN32
#define INT_MAX    2147483647
#define INT_MIN    (-2147483647-1)
#endif

const int OzMaxInt = INT_MAX>>tagSize;
const int OzMinInt = -OzMaxInt;


inline
TaggedRef newSmallInt(int val)
{
  Assert(val >= OzMinInt && val <= OzMaxInt);

  return makeTaggedSmallInt((int32)val);
}


/* The C++ standard does not specify whether shifting right negative values
 * means shift logical or shift arithmetical. So we test what this C++ compiler
 * does.
 */
inline
int smallIntValue(TaggedRef t)
{
  int help = (int) t;

  if (1||(-1>>1) == -1) {  /* -1>>1  means SRA? */
    return (help>>tagSize);
  } else {
    return (help >= 0) ? help >> tagSize
                       : ~(~help >> tagSize);
  }
}

inline
Bool smallIntLess(TaggedRef A, TaggedRef B)
{
  Assert(isSmallInt(A) && isSmallInt(B));
  int ahelp = A;
  int bhelp = B;
  return ahelp < bhelp;
}

inline
Bool smallIntLE(TaggedRef A, TaggedRef B)
{
  Assert(isSmallInt(A) && isSmallInt(B));
  int ahelp = A;
  int bhelp = B;
  return ahelp <= bhelp;
}

inline
unsigned int smallIntHash(TaggedRef n)
{
  return (unsigned int) ToInt32(tagValueOf(n));
}


inline
Bool smallIntEq(TaggedRef a, TaggedRef b)
{
  Assert(isSmallInt(a) || isSmallInt(b));
  return (a == b);
}

inline
Bool smallIntCmp(TaggedRef a, TaggedRef b)
{
  Assert(isSmallInt(a) && isSmallInt(b));
  return smallIntLess(a,b) ? -1 : (smallIntEq(a,b) ? 0 : 1);
}

/*===================================================================
 * Float
 *=================================================================== */

class Float {
protected:
  double value;

public:
  Float() { error("use newFloat");; };
  static Float *newFloat(double val);
  double getValue() { return value; }
  OZPRINT;
  OZPRINTLONG;
  unsigned int hash() { return (unsigned int) value; }

  Float *gc();
};

inline
Float *Float::newFloat(double val)
{
  Float *ret = (Float *) alignedMalloc(sizeof(Float),sizeof(double));
  ret->value = val;
  return ret;
}

inline
double floatValue(TaggedRef t)
{
  return tagged2Float(t)->getValue();
}

inline
Bool floatEq(TaggedRef a, TaggedRef b)
{
  return (tagged2Float(a)->getValue() == tagged2Float(b)->getValue());
}

inline
TaggedRef makeTaggedFloat(double i)
{
  return makeTaggedFloat(Float::newFloat(i));
}

/*===================================================================
 * BigInt
 *=================================================================== */

class BigInt {
public:
  USEFREELISTMEMORY;

private:
  MP_INT value;

public:
  BigInt() {
    mpz_init(&value);
  }
  BigInt(int i) {
    mpz_init_set_si(&value,i);
  }
  BigInt(char *s) {
    if(mpz_init_set_str(&value, s, 10)) {
      Assert(0);
    }
  }
  BigInt(MP_INT *i) {
    mpz_init_set(&value, i);
  }
  void dispose()
  {
    mpz_clear(&value);
    freeListDispose(this,sizeof(BigInt));
  }
/* make a small int if <Big> fits into it, else return big int */
  TaggedRef shrink() {
    TaggedRef ret;
    if (mpz_cmp_si(&value,OzMaxInt) > 0 ||
        mpz_cmp_si(&value,OzMinInt) < 0)
      ret = makeTaggedBigInt(this);
    else {
      ret =  newSmallInt((int) mpz_get_si(&value));
      dispose();
    }
    return ret;
  }

  /* make an 'int' if <Big> fits into it, else return INT_MAX,INT_MIN */
  int getInt()
  {
    if (mpz_cmp_si(&value,INT_MAX) > 0) {
      return INT_MAX;
    } else if (mpz_cmp_si(&value,INT_MIN) < 0) {
      return INT_MIN;
    } else {
      return mpz_get_si(&value);
    }
  }


  Bool equal(BigInt *b) {
    return (mpz_cmp(&value, &b->value) == 0);
  }

#define MKOP(op,mpop)                                                         \
  TaggedRef op(BigInt *b) {                                                   \
    BigInt *n = new BigInt();                                                 \
    mpop(&n->value,&value,&b->value);                                         \
    return n->shrink();                                                       \
  }
  MKOP(div,mpz_div);
  MKOP(mod,mpz_mod);
  MKOP(mul,mpz_mul);
  MKOP(sub,mpz_sub);
  MKOP(add,mpz_add);
#undef MKOP

  TaggedRef neg() {
    BigInt *n = new BigInt();
    mpz_neg(&n->value,&value);
    return n->shrink();
  }
  int cmp(BigInt *b)      { return mpz_cmp(&value,&b->value); }
  int cmp(long int i)     { return mpz_cmp_si(&value,i); }
  Bool less(BigInt *b)    { return cmp(b) < 0; }
  Bool leq(BigInt *b)     { return cmp(b) <= 0; }
  int stringLength()      { return mpz_sizeinbase(&value,10)+2; }
  void getString(char *s) { mpz_get_str(s,10,&value); }
  OZPRINTLONG;
  unsigned int hash() { return 75; } // all BigInt hash to same value
  BigInt *gc();
};

inline
TaggedRef makeInt(int i)
{
  if (i > OzMaxInt || i < OzMinInt)
    return makeTaggedBigInt(new BigInt(i));
  else
    return newSmallInt(i);
}

inline
Bool bigIntEq(TaggedRef a, TaggedRef b)
{
  return tagged2BigInt(a)->equal(tagged2BigInt(b));
}

inline
Bool numberEq(TaggedRef a, TaggedRef b)
{
  Assert(tagTypeOf(a)==tagTypeOf(b));

  TypeOfTerm tag = tagTypeOf(a);
  switch(tag) {
  case OZFLOAT:  return floatEq(a,b);
  case SMALLINT: return smallIntEq(a,b);
  case BIGINT:   return bigIntEq(a,b);
  default:       return NO;
  }
}

#define CHECK_LITERAL(lab) \
Assert(!isRef(lab) && !isAnyVar(lab) && isLiteral(lab));


/*===================================================================
 * LTuple
 *=================================================================== */

class LTuple {
protected:
// DATA
  TaggedRef args[2];   // head, tail
 public:
  USEHEAPMEMORY;

  LTuple(void) {;} // called by putlist and the like
  LTuple(TaggedRef head, TaggedRef tail)
  { args[0] = head; args[1] = tail; }

  TaggedRef getHead() { return tagged2NonVariable(args); }
  TaggedRef getTail() { return tagged2NonVariable(args+1); }
  void setHead(TaggedRef term) { args[0] = term;}
  void setTail(TaggedRef term) { args[1] = term;}
  TaggedRef getLabel() { return AtomCons; }
  Literal *getLabelLiteral() { return tagged2Literal(AtomCons); }
  TaggedRef *getRef() { return &args[0]; }
  TaggedRef *getRefHead() { return &args[0]; }
  TaggedRef *getRefTail() { return &args[1]; }
  void initArgs(TaggedRef val) { args[0]=val; args[1]=val;}

  OZPRINT;
  OZPRINTLONG;

  void gcRecurse();
  LTuple *gc();
};


// functions for builtins and features
//   with explicit deref


inline
Bool isCons(TaggedRef term) {
  return isLTuple(term);
}

inline
Bool isNil(TaggedRef term) {
  return literalEq(term,AtomNil);
}

inline
TaggedRef nil() { return AtomNil; }


inline
TaggedRef cons(TaggedRef head, TaggedRef tail)
{
  return makeTaggedLTuple(new LTuple(head,tail));
}

inline
TaggedRef cons(Literal *head, TaggedRef tail)
{
  return cons(makeTaggedLiteral(head),tail);
}

inline
TaggedRef cons(char *head, TaggedRef tail)
{
  return cons(makeTaggedAtom(head),tail);
}

inline
TaggedRef head(TaggedRef list)
{
  Assert(isLTuple(list));
  return tagged2LTuple(list)->getHead();
}

inline
TaggedRef * headRef(TaggedRef list)
{
  Assert(isLTuple(list));
  return tagged2LTuple(list)->getRefHead();
}

inline
TaggedRef tail(TaggedRef list)
{
  Assert(isLTuple(list));
  return tagged2LTuple(list)->getTail();
}

inline
TaggedRef * tailRef(TaggedRef list)
{
  Assert(isLTuple(list));
  return tagged2LTuple(list)->getRefTail();
}

inline
int length(OZ_Term l)
{
  int len = 0;
  l = deref(l);
  while (isCons(l)) {
    len++;
    l = deref(tail(l));
  }
  return len;
}


/*===================================================================
 * ConstTerm
 *=================================================================== */

/* must not match GCTAG (ie <> 13 (1101) !!!! */
enum TypeOfConst {
  Co_Board,
  Co_Actor,
  Co_HeapChunk,
  Co_Thread,

  Co_Abstraction,  /* 4 */
  Co_Builtin,
  Co_Cell,
  Co_Space,

  /* chunks must stay together and the first one
   * must be Co_Object
   * otherwise you'll have to change the "isChunk" test
   * NOTE: update the builtins: subtree and chunkArity !
   */
  Co_Object,
  Co_Port,
  Co_Chunk,
  Co_Array,
  Co_Dictionary,    /* 12 */
  Dummy,           // GCTAG
  Co_Lock
};

enum TertType {
  Te_Local   = 0, // 0000
  Te_Manager = 1, // 0001
  Te_Proxy   = 2, // 0010
  Te_Frame   = 3  // 0011
};

#define DebugIndexCheck(IND) {Assert(IND< (1<<27));Assert(IND>=0);}

class ConstTerm {
protected:
  union {
    TaggedRef tagged;
    void *padForAlpha;   // otherwise we get lots of warnings
  } ctu;
public:
  /* where do we store forward reference */
  int32 *getGCField() { return (int32*) &ctu.tagged; }
public:
  USEHEAPMEMORY;

  ConstTerm *gcConstTerm(void);
  ConstTerm *gcConstTermSpec(void);
  void gcConstRecurse(void);

  void setTagged(TypeOfConst t, void *p) {
    ctu.tagged = makeTaggedRef((TypeOfTerm)t,p);
  }
  ConstTerm(TypeOfConst t)  { setTagged(t,NULL); }
  TypeOfConst getType()     { return (TypeOfConst) tagTypeOf(ctu.tagged); }
  //  TypeOfConst typeOf()  { return getType(); }
  char *getPrintName();
  int getArity();
  void *getPtr() {
    return isNullPtr(ctu.tagged) ? NULL : tagValueOf(ctu.tagged);
  }
  void setPtr(void *p)  { setTagged(getType(),p); }
  TaggedRef *getRef()   { return &ctu.tagged; }

  void setGName(GName *gn) { setPtr(gn); }
  GName *getGName1() { return (GName *) getPtr(); }

  void setPrimIndex(int i){
    int j=i+1;
    DebugIndexCheck(j);
    unsigned int k= (unsigned int)j;
    k= (k<<2);
    ctu.tagged=makeTaggedRef((TypeOfTerm)getType(),(void *)k);}

  int getPrimIndex(){
    unsigned int tmp= (unsigned int)tagValueOf(ctu.tagged);
    tmp=tmp>>2;
    int tmp2= (int)tmp;
    return (tmp2-1);}

  OZPRINT;
  OZPRINTLONG;

  Bool unify(TaggedRef, Bool) { return NO; }
  Bool install(TaggedRef t) { return unify(t,OK); };
  Bool deinstall(TaggedRef) { return OK; };

  /* optimized isChunk test */
  Bool isChunk() { return (int) getType() >= (int) Co_Object; }
};


class ConstTermWithHome: public ConstTerm {
private:
  Board *board;
public:
  ConstTermWithHome(Board *b, TypeOfConst t) : ConstTerm(t), board(b) {}
  Board *getBoardInternal() { return board; }
  Board *getBoard();
  setBoard(Board *bb) { board = bb; }
};


class Tertiary: public ConstTerm {
  TaggedPtr tagged;
public:

  TertType getTertType()       { return (TertType) tagged.getType(); }
  void setTertType(TertType t) { tagged.setType((int) t); }

  Tertiary(Board *b, TypeOfConst s,TertType t) : ConstTerm(s) {
    setTertType(t);
    setBoard(b);}
  Tertiary(int i, TypeOfConst s,TertType t) : ConstTerm(s)
  {
    setTertType(t);
    setIndex(i);
  }

  void setIndex(int i) { tagged.setIndex(i); }
  int getIndex() { return tagged.getIndex(); }
  void setPointer (void *p) { tagged.setPtr(p); }
  void *getPointer()        { return tagged.getPtr(); }

  Bool checkTertiary(TypeOfConst s,TertType t){
    return (s==getType() && t==getTertType());}

  INLINE Board *getBoard();
  Board *getBoardInternal();
  void setBoard(Board *b);

  Bool isLocal()   { return (getTertType() == Te_Local); }
  Bool isManager() { return (getTertType() == Te_Manager); }
  Bool isProxy()   { return (getTertType() == Te_Proxy); }

  void globalizeTert();
  void localize();

  void gcProxy();
  void gcManager();
  void gcBorrowSec(int);
  void gcTertiary();
};



/*===================================================================
 * HeapChunk
 *=================================================================== */

class HeapChunk: public ConstTerm {
private:
  size_t chunk_size;
  char * chunk_data;
  char * copyChunkData(void) {
    char * data = allocate(chunk_size);
    for (int i = chunk_size; i--; )
      data[i] = chunk_data[i];
    return data;
  }
  char * allocate(int size) {
    return (char *) alignedMalloc(size, sizeof(double));
  }
public:
  HeapChunk(HeapChunk&);
  HeapChunk(int size)
  : ConstTerm(Co_HeapChunk), chunk_size(size), chunk_data(allocate(size)) {
    }

  size_t getChunkSize(void) { return chunk_size; }

  char * getChunkData(void) { return chunk_data; }

  OZPRINT;
  OZPRINTLONG;

  HeapChunk * gc(void);
};

/*===================================================================
 * SRecord: incl. Arity, ArityTable
 *=================================================================== */


inline
Bool isFeature(TaggedRef lab) { return isLiteral(lab) || isInt(lab); }

#define CHECK_FEATURE(lab) \
Assert(!isRef(lab) && !isAnyVar(lab) && isFeature(lab));

inline
int featureEq(TaggedRef a,TaggedRef b)
{
  CHECK_FEATURE(a);
  CHECK_FEATURE(b);
  if (isLiteral(a)) {
    // Note: if b is no literal this also returns NO
    return literalEq(a,b);
  }
  TypeOfTerm tagA = tagTypeOf(a);
  TypeOfTerm tagB = tagTypeOf(b);
  if (tagA != tagB) return NO;
  switch(tagA) {
  case SMALLINT: return smallIntEq(a,b);
  case BIGINT:   return bigIntEq(a,b);
  default:       return NO;
  }
}

/*
 * for sorting the arity one needs to have a total order
 *
 * SMALLINT < BIGINT < LITERAL
 * return 0: if equal
 *       -1: a<b
 *        1: a>b
 */
inline
int featureCmp(TaggedRef a,TaggedRef b)
{
  CHECK_FEATURE(a);
  CHECK_FEATURE(b);
  TypeOfTerm tagA = tagTypeOf(a);
  TypeOfTerm tagB = tagTypeOf(b);
  if (tagA != tagB) {
    if (tagA==SMALLINT) return -1;
    if (tagA==BIGINT) {
      if (tagB==SMALLINT) return 1;
      Assert(tagB==LITERAL);
      return -1;
    }
    Assert(tagA==LITERAL);
    return 1;
  }
  switch (tagA) {
  case LITERAL:
    return atomcmp(tagged2Literal(a),tagged2Literal(b));
  case SMALLINT:
    return smallIntCmp(a,b);
  case BIGINT:
    return tagged2BigInt(a)->cmp(tagged2BigInt(b));
  default:
    error("featureCmp");
    return 0;
  }
}


/*
 * Hash function for Features:
 * NOTE: all bigints are hashed to the same value
 */
inline
unsigned int featureHash(TaggedRef a)
{
  CHECK_FEATURE(a);
  TypeOfTerm tag = tagTypeOf(a);
  switch (tag) {
  case LITERAL:
    return tagged2Literal(a)->hash();
  case SMALLINT:
    return (unsigned int) a;
  case BIGINT:
    return 75;
  default:
    error("featureHash");
    return 0;
  }
}

class Arity {
friend class ArityTable;
private:
  Arity ( TaggedRef, Bool );

  void gc();

  TaggedRef list;
  Arity *next;

  int size;             // size is always a power of 2
  int hashmask;         // size-1, used as mask for hashing
  int width;            // next unused index in RefsArray (for add())
  DebugCheckT(int numberofentries);
  DebugCheckT(int numberofcollisions);
  Bool isTupleFlag;

  TaggedRef *keytable;
  int *indextable;
  int scndhash(TaggedRef a) { return ((featureHash(a)&7)<<1)|1; }
  int hashfold(int i) { return(i&hashmask); }
  void add (TaggedRef);

public:
  Bool isTuple() {
    return isTupleFlag;
  }
  int find(TaggedRef entry)   // return -1, if argument not found.
  {
    if (isTuple()) return isSmallInt(entry) ? smallIntValue(entry)-1 : -1;

    int i=hashfold(featureHash(entry));
    int step=scndhash(entry);
    while (OK) {
      if ( keytable[i] == makeTaggedNULL()) return -1;
      if ( featureEq(keytable[i],entry) ) return indextable[i];
      i = hashfold(i+step);
    }
  }
public:
  TaggedRef getList() { return list; }
  int getWidth() { return width; }

  OZPRINT;
};

Arity *mkArity(TaggedRef list);

#define ARITYTABLESIZE 8000

class ArityTable {
friend class Arity;
public:
  ArityTable ( unsigned int );
  Arity *find ( TaggedRef);
  void gc();
private:
  OZPRINT;

  Bool hashvalue( TaggedRef, unsigned int & );
  Arity* *table;

  int size;
  int hashmask;
  int hashfold(int i) { return(i&hashmask); }
};

extern ArityTable aritytable;

TaggedRef makeTupleArityList(int i);


/*
 * Abstract data type SRecordArity for records and tuples:
 * either an Arity* or an int
 */

// #define TYPECHECK_SRecordArity
#ifdef TYPECHECK_SRecordArity

class SRA;
typedef SRA* SRecordArity;
#define Body(X) ;
#define inline

#else

typedef int32 SRecordArity; /* do not want to use a pointer on the Alpha! */
#define Body(X) X

#endif

inline Bool sraIsTuple(SRecordArity a)
     Body({ return a&1; })

inline SRecordArity mkTupleWidth(int w)
     Body({ return (SRecordArity) ((w<<1)|1);})

inline int getTupleWidth(SRecordArity a)
     Body({ return a>>1; })

inline SRecordArity mkRecordArity(Arity *a)
     Body({ Assert(!a->isTuple()); return ToInt32(a); })

inline Arity *getRecordArity(SRecordArity a)
     Body({ return (Arity*) ToPointer(a); })

inline Bool sameSRecordArity(SRecordArity a, SRecordArity b)
     Body({ return a==b; })
inline int getWidth(SRecordArity a)
     Body({
       return sraIsTuple(a) ? getTupleWidth(a) : getRecordArity(a)->getWidth();
     })

#undef Body

inline
OZ_Term sraGetArityList(SRecordArity arity)
{
  return (sraIsTuple(arity))
    ? makeTupleArityList(getTupleWidth(arity))
    : getRecordArity(arity)->getList();
}

class SRecord {
private:
  TaggedRef label;
  SRecordArity recordArity;
  TaggedRef args[1];   // really maybe more

public:
  USEHEAPMEMORY;

  SRecord *gcSRecord();

  SRecord() { error("do not use SRecord"); }

  Bool isTuple() { return sraIsTuple(recordArity); }

  void setTupleWidth(int w) { recordArity = mkTupleWidth(w); }
  int getTupleWidth() {
    Assert(isTuple());
    return ::getTupleWidth(recordArity);
  }

  SRecordArity getSRecordArity() { return recordArity; }

  void setRecordArity(Arity *a) { recordArity = mkRecordArity(a);}
  Arity *getRecordArity() {
    Assert(!isTuple());
    return ::getRecordArity(recordArity);
  }

  TaggedRef normalize()
  {
    if (isTuple() && label == AtomCons && getWidth()==2) {
      return makeTaggedLTuple(new LTuple(getArg(0),getArg(1)));
    }
    return makeTaggedSRecord(this);
  }

  void initArgs(TaggedRef val)
  {
    for (int i = getWidth(); i--; )
      args[i] = val;
  }

  int getWidth() { return ::getWidth(getSRecordArity()); }

  void downSize(unsigned int s) { // TMUELLER
    Assert(isTuple());
    setTupleWidth(s);
  } // FD

  static SRecord *newSRecord(TaggedRef lab, SRecordArity arity, int width)
  {
    CHECK_LITERAL(lab);
    Assert(width > 0);
    int memSize = sizeof(SRecord) + sizeof(TaggedRef) * (width - 1);
    SRecord *ret = (SRecord *) int32Malloc(memSize);
    ret->label = lab;
    ret->recordArity = arity;
    return ret;
  }

  static SRecord *newSRecord(TaggedRef lab, int i)
  {
    return newSRecord(lab,mkTupleWidth(i),i);
  }

  static SRecord *newSRecord(TaggedRef lab, Arity *arity)
  {
    if (arity->isTuple())
      return newSRecord(lab,arity->getWidth());

    return newSRecord(lab,mkRecordArity(arity),arity->getWidth());
  }

  // returns copy of tuple st (args are appropriately copied as well)
  static SRecord * newSRecord(SRecord * st)
  {
    SRecord *ret = newSRecord(st->label, st->getSRecordArity(),st->getWidth());
    for (int i = st->getWidth(); i--; ) {
      ret->args[i] = tagged2NonVariable((st->args)+i);
    }
    return ret;
  }

  TaggedRef getArg(int i) { return tagged2NonVariable(args+i); }
  void setArg(int i, TaggedRef t) { args[i] = t; }
  TaggedRef *getRef() { return args; }
  TaggedRef *getRef(int i) { return args+i; }
  TaggedRef &operator [] (int i) {return args[i];}

  TaggedRef adjoinAt(TaggedRef feature, TaggedRef value);

  SRecord *replaceLabel(TaggedRef newlabel)
  {
    SRecord *copy = newSRecord(this);
    copy->label = newlabel;
    return copy;
  }

  TaggedRef adjoin(SRecord* highstr);
  TaggedRef adjoinList(TaggedRef arity, TaggedRef proplist);
  void setFeatures(TaggedRef proplist);

  TaggedRef getLabel() { return label; }
  Literal *getLabelLiteral() { return tagged2Literal(label); }
  void setLabel(TaggedRef newLabel) { label = newLabel; }

  TaggedRef getArityList() {
    return sraGetArityList(getSRecordArity());
  }

  Arity* getArity () {
    return isTuple() ? aritytable.find(getArityList()) : getRecordArity();
  }

  int getIndex(TaggedRef feature)
  {
    CHECK_FEATURE(feature);
    if (isTuple()) {
      if (!isSmallInt(feature)) return -1;
      int f=smallIntValue(feature);
      return (1 <= f && f <= getWidth()) ? f-1 : -1;
    }
    return getArity()->find(feature);
  }

  Bool hasFeature(TaggedRef feature) { return getIndex(feature) >= 0; }
  TaggedRef getFeature(TaggedRef feature)
  {
    int i = getIndex(feature);
    return i < 0 ? makeTaggedNULL() : getArg(i);
  }

  Bool setFeature(TaggedRef feature,TaggedRef value);
  TaggedRef replaceFeature(TaggedRef feature,TaggedRef value);

  void gcRecurse();

  OZPRINT;
  OZPRINTLONG;

  Bool compareSortAndArity(TaggedRef lbl, SRecordArity arity) {
    return literalEq(getLabel(),lbl) &&
           sameSRecordArity(getSRecordArity(),arity);
  }

  Bool compareFunctor(SRecord* str) {
    return compareSortAndArity(str->getLabel(),str->getSRecordArity());
  }

  TaggedRef *getCycleAddr() { return &label; }

};

TaggedRef sortlist(TaggedRef list,int len);
TaggedRef mkRecord(TaggedRef label,SRecordArity ff);

inline
Bool isRecord(TaggedRef term) {
  GCDEBUG(term);
  TypeOfTerm tag = tagTypeOf(term);
  return isSRecord(tag) || isLTuple(tag) || isLiteral(tag);
}


SRecord *makeRecord(TaggedRef t);

inline
int isSTuple(TaggedRef term) {
  return isSRecord(term) && tagged2SRecord(term)->isTuple();
}

inline
int isTuple(TaggedRef term) {
  return isLTuple(term) || isSTuple(term);
}

inline
OZ_Term getArityList(OZ_Term term)
{
  if (isSRecord(term)) {
    return tagged2SRecord(term)->getArityList();
  }
  if (isLTuple(term)) {
    return makeTupleArityList(2);
  }
  if (isLiteral(term)) {
    return nil();
  }
  return 0;
}

inline
int getWidth(OZ_Term term)
{
  if (isSRecord(term)) {
    return tagged2SRecord(term)->getWidth();
  }
  if (isLTuple(term)) {
    return (2);
  }
  if (isLiteral(term)) {
    return (0);
  }
  return (0);                   // ???
}


/*===================================================================
 * ObjectOrClass incl. ObjectClass, DeepObjectOrClass
 *=================================================================== */

/* Internal representation of Oz classes */

class ObjectClass {
private:
  OzDictionary *fastMethods;
  OzDictionary *defaultMethods;
  SRecord *unfreeFeatures;
  Object *ozclass;    /* the class as seen by the Oz user */
  Bool locking;
public:
  USEHEAPMEMORY;

  ObjectClass(OzDictionary *fm, SRecord *uf, OzDictionary *dm, Bool lck)
  {
    fastMethods    = fm;
    unfreeFeatures = uf;
    defaultMethods = dm;
    ozclass        = NULL;
    locking        = lck;
  }

  Bool supportsLocking() { return locking; }

  OzDictionary *getDefMethods()  { return defaultMethods; }
  OzDictionary *getfastMethods() { return fastMethods; }
  Object *getOzClass()           { return ozclass; }
  void setOzClass(Object *cl)    { ozclass = cl; }

  TaggedRef getFeature(TaggedRef lit)
  {
    return unfreeFeatures
      ? unfreeFeatures->getFeature(lit)
      : makeTaggedNULL();
  }

  SRecord *getUnfreeRecord() { return unfreeFeatures; }

  char *getPrintName();

  ObjectClass *gcClass();

  void import(OzDictionary *fm, SRecord *uf, OzDictionary *dm, Bool l)
  {
    fastMethods    = fm;
    unfreeFeatures = uf;
    defaultMethods = dm;
    locking        = l;
  }

  OZPRINT;
  OZPRINTLONG;
};


/*
 * Object
 */

typedef enum {
  OFlagClass  = 1
} OFlag;


#define ObjFlagMask ~3


typedef int32 RecOrCell;

inline
Bool stateIsCell(RecOrCell rc)     { return rc&1; }

inline
Tertiary *getCell(RecOrCell rc)   {
  Assert(stateIsCell(rc)); return (Tertiary*) ToPointer(rc-1);
}

inline
SRecord *getRecord(RecOrCell rc) {
  Assert(!stateIsCell(rc)); return (SRecord*) ToPointer(rc);
}

inline
RecOrCell makeRecCell(Tertiary *c)    { return ToInt32(c)|1; }

inline
RecOrCell makeRecCell(SRecord *r) { return ToInt32(r); }


class Object: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
protected:
  RecOrCell state;  // was: SRecord *state, but saves memory on the Alpha
  int32 aclass;     // was: ObjectClass *aclass
  int32 flagsAndLock;
  SRecord *freeFeatures;
public:
  Object();
  ~Object();
  Object(Object&);

  void setFlag(OFlag f)   { flagsAndLock |= (int) f; }
  void unsetFlag(OFlag f) { flagsAndLock &= ~((int) f); }
  int  getFlag(OFlag f)   { return (flagsAndLock & ((int) f)); }

  Bool isClass()        { return getFlag(OFlagClass); }
  void setClass()       { setFlag(OFlagClass); }

  Object(Board *bb,SRecord *s,ObjectClass *ac,SRecord *feat,Bool iscl,
         OzLock *lck):
    ConstTermWithHome(bb,Co_Object)
  {
    setFreeRecord(feat);
    setClass(ac);
    if (iscl) setClass();
    setState(s);
    flagsAndLock = ToInt32(lck);
    setGName(0);
  }

  Object(Board *bb, GName *gn):
    ConstTermWithHome(bb,Co_Object)
  {
    setFreeRecord(NULL);
    setClass(NULL);
    setState((Tertiary*)NULL);
    flagsAndLock = 0;
    setGName(gn);
  }

  void setClass(ObjectClass *c) { aclass = ToInt32(c); }

  OzLock *getLock() { return (OzLock*)ToPointer(flagsAndLock&ObjFlagMask); }
  void setLock(OzLock *l) { flagsAndLock |= ToInt32(l); }

  ObjectClass *getClass() { return (ObjectClass*) ToPointer(aclass); }

  OzDictionary *getMethods()    { return getClass()->getfastMethods(); }
  Abstraction *getMethod(TaggedRef label, SRecordArity arity, RefsArray X,
                         Bool &defaultsUsed);

  char *getPrintName()          { return getClass()->getPrintName(); }
  Bool lookupDefault(TaggedRef label, SRecordArity arity, RefsArray X);
  RecOrCell getState()          { return state; }
  void setState(SRecord *s) {
    Assert(s!=0 || isClass()); state=makeRecCell(s);
  }
  void setState(Tertiary *c)    { state = makeRecCell(c); }
  OzDictionary *getDefMethods() { return getClass()->getDefMethods(); }
  Object *getOzClass()          { return getClass()->getOzClass(); }

  SRecord *getFreeRecord()          { return freeFeatures; }
  SRecord *getUnfreeRecord() {
    return isClass() ? (SRecord*) NULL : getClass()->getUnfreeRecord();
  }
  void setFreeRecord(SRecord *aRec) { freeFeatures = aRec; }

  /* same functionality is also in instruction inlineDot */
  TaggedRef getFeature(TaggedRef lit)
  {
    SRecord *freefeat = getFreeRecord();
    if (freefeat) {
      TaggedRef ret = freefeat->getFeature(lit);
      if (ret!=makeTaggedNULL())
        return ret;
    }
    SRecord *fr = getUnfreeRecord();
    return fr ?  fr->getFeature(lit) : makeTaggedNULL();
  }

  TaggedRef replaceFeature(TaggedRef lit, TaggedRef value) {
    SRecord *freefeat = getFreeRecord();
    if (freefeat) {
      int ind = freefeat->getIndex(lit);
      if (ind != -1) {
        TaggedRef ret = freefeat->getArg(ind);
        freefeat->setArg(ind, value);
        return ret;
      }
    }
    SRecord *fr = getUnfreeRecord();

    if (!fr)
      return makeTaggedNULL();

    int ind = fr->getIndex(lit);

    if (ind == -1)
      return makeTaggedNULL();

    TaggedRef ret = fr->getArg(ind);
    fr->setArg(ind, value);

    return ret;
  }

  TaggedRef getArityList();
  int getWidth ();

  Object *gcObject();

  GName *getGName() {
    GName *gn = getGName1();
    return gn ? gn : globalize();
  }
  GName *globalize();

  OZPRINT;
  OZPRINTLONG;
};

SRecord *getState(RecOrCell state, Bool isAssign, OZ_Term fea, OZ_Term &val);

inline
Bool isObject(ConstTerm *t)
{
  return (t->getType()==Co_Object);
}

inline
Bool isObject(TaggedRef term)
{
  return isConst(term) && isObject(tagged2Const(term));
}

inline
Object *tagged2Object(TaggedRef term)
{
  Assert(isObject(term));
  return (Object *)tagged2Const(term);
}

inline
Bool isClass(TaggedRef term)
{
  return isObject(term) && tagged2Object(term)->isClass();
}

inline
Bool isObjectTrue(TaggedRef term)
{
  return isObject(term) && !tagged2Object(term)->isClass();
}

/*===================================================================
 * SChunk
 *=================================================================== */

class SChunk: public ConstTermWithHome {
friend void ConstTerm::gcConstRecurse(void);
private:
  TaggedRef value;
public:
  SChunk(Board *b,TaggedRef v)
    : ConstTermWithHome(b,Co_Chunk), value(v)
  {
    Assert(v==0||isRecord(v));
    Assert(b);
    setGName(0);
  };

  OZPRINT;
  OZPRINTLONG;

  TaggedRef getValue() { return value; }
  TaggedRef getFeature(TaggedRef fea) { return OZ_subtree(value,fea); }
  TaggedRef getArityList() { return ::getArityList(value); }
  int getWidth () { return ::getWidth(value); }

  void import(TaggedRef val) {
    Assert(!value);
    Assert(isRecord(val));
    Assert(getGName1());
    value=val;
  }

  GName *globalize();
  GName *getGName() {
    GName *gn = getGName1();
    return gn ? gn : globalize();
  }
};


inline
Bool isSChunk(ConstTerm *t)
{
  return t->getType() == Co_Chunk;
}

inline
Bool isSChunk(TaggedRef term)
{
  return isConst(term) && isSChunk(tagged2Const(term));
}

inline
SChunk *tagged2SChunk(TaggedRef term)
{
  Assert(isSChunk(term));
  return (SChunk *) tagged2Const(term);
}


inline
Bool isChunk(TaggedRef t)
{
  return isConst(t) && tagged2Const(t)->isChunk();
}

/*===================================================================
 * Arrays
 *=================================================================== */

class OzArray: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
private:
  int offset, width;

  TaggedRef *getArgs() { return (TaggedRef*) getPtr(); }

public:

  OzArray(Board *b, int low, int high, TaggedRef initvalue) : ConstTermWithHome(b,Co_Array)
  {
    Assert(isRef(initvalue) || !isAnyVar(initvalue));

    offset = low;
    width = high-low+1;
    if (width <= 0) {
      width = 0;
      setPtr(NULL); // mm2: attention if globalize gname!
    } else {
      TaggedRef *args = (TaggedRef*) int32Malloc(sizeof(TaggedRef)*width);
      for(int i=0; i<width; i++) {
        args[i] = initvalue;
      }
      setPtr(args);
    }
  }

  int getLow()      { return offset; }
  int getHigh()     { return getWidth() + offset - 1; }
  int getWidth()    { return width; }

  OZ_Term getArg(int n)
  {
    n -= offset;
    if (n>=getWidth() || n<0)
      return 0;

    OZ_Term out = getArgs()[n];
    Assert(isRef(out) || !isAnyVar(out));

    return out;
  }

  int setArg(int n,TaggedRef val)
  {
    Assert(isRef(val) || !isAnyVar(val));

    n -= offset;
    if (n>=getWidth() || n<0) return FALSE;

    getArgs()[n] = val;
    return TRUE;
  }

  OZPRINT;
  OZPRINTLONG;

};


inline
Bool isArray(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType() == Co_Array;
}

inline
OzArray *tagged2Array(TaggedRef term)
{
  Assert(isArray(term));
  return (OzArray *) tagged2Const(term);
}


/*===================================================================
 * Abstraction (incl. PrTabEntry, AssRegArray, AssReg)
 *=================================================================== */

enum KindOfReg {
  XReg = XREG,
  YReg = YREG,
  GReg = GREG
};

class AssReg {
public:
  void print();

  PosInt number;
  KindOfReg kind;
};

inline
AssReg* allocAssRegBlock (int numb)
{
  return new AssReg[numb];
}

class AssRegArray  {
  friend void restore (int fileDesc, AssRegArray *array);
  /* we restore at the address ptr number of AssRegs;           */
public:
  AssRegArray (int sizeInit) : numbOfGRegs (sizeInit)
  {
    first = (sizeInit==0 ? (AssReg*) NULL : allocAssRegBlock(sizeInit));
  }
  ~AssRegArray () { Assert(0) };

  int getSize () { return (numbOfGRegs); }
  AssReg &operator[] (int elem) { return ( *(first + elem) ); }
  /* no bounds checking;    */

private:
  int numbOfGRegs;
  AssReg* first;
};


// Debugger ---------------------------------------------

class DbgInfo {
public:
  ProgramCounter PC;
  TaggedRef file;
  int line;
  DbgInfo *next;
  DbgInfo(ProgramCounter pc, TaggedRef f, int l, DbgInfo *nxt)
    : PC(pc), file(f), line(l), next(nxt) {};
};

extern DbgInfo *allDbgInfos;

// ---------------------------------------------


class PrTabEntry {
private:
  TaggedRef printname;
  unsigned short arity;
  unsigned short spyFlag;
  SRecordArity methodArity;
  TaggedRef fileName;
  int lineno;

public:
  ProgramCounter PC;

  PrTabEntry (TaggedRef name, SRecordArity arityInit,TaggedRef file, int line)
  : printname(name), spyFlag(NO), fileName(file), lineno(line)
  {
    Assert(isLiteral(name));
    methodArity = arityInit;
    arity =  (unsigned short) getWidth(arityInit);
    Assert((int)arity == getWidth(arityInit)); /* check for overflow */
    PC = NOCODE;
  }

  OZPRINTLONG;

  int getArity () { return (int) arity; }
  TaggedRef getFileName() { return fileName; }
  int getLine() { return lineno; }
  SRecordArity getMethodArity() { return methodArity; }
  char *getPrintName () { return tagged2Literal(printname)->getPrintName(); }
  TaggedRef getName () { return printname; }
  ProgramCounter getPC() { return PC; }
  Bool getSpyFlag()   { return (Bool) spyFlag; }
  void setSpyFlag()   { spyFlag = OK; }
  void unsetSpyFlag() { spyFlag = NO; }
};



class Abstraction: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
protected:
// DATA
  RefsArray gRegs;
  PrTabEntry *pred;
public:
  Abstraction(Abstraction&);
  Abstraction(PrTabEntry *prd, RefsArray gregs, Board *b)
    : ConstTermWithHome(b,Co_Abstraction), gRegs(gregs), pred(prd)
  { }

  OZPRINT;
  OZPRINTLONG;

  RefsArray &getGRegs()  { return gRegs; }
  ProgramCounter getPC() { return pred->getPC(); }
  int getArity()         { return pred->getArity(); }
  SRecordArity getMethodArity()   { return pred->getMethodArity(); }
  int getGSize()         { return getRefsArraySize(gRegs); }
  char *getPrintName()   { return pred->getPrintName(); }
  PrTabEntry *getPred()  { return pred; }
  TaggedRef getName()    { return pred->getName(); }

  TaggedRef DBGgetGlobals();

  GName *globalize();
  GName *getGName() {
    GName *gn = getGName1();
    return gn ? gn : globalize();
  }
  void import(RefsArray g, ProgramCounter pc) {
    gRegs = g;
    if (pc!=NOCODE) {
      pred->PC = pc;
    }
  }
};

inline
Bool isProcedure(TaggedRef term)
{
  if (!isConst(term)) {
    return NO;
  }
  switch (tagged2Const(term)->getType()) {
  case Co_Abstraction:
  case Co_Builtin:
    return OK;
  default:
    return NO;
  }
}


inline
Bool isAbstraction(ConstTerm *s)
{
  return s->getType() == Co_Abstraction;
}

inline
Bool isAbstraction(TaggedRef term)
{
  return isConst(term) && isAbstraction(tagged2Const(term));
}

inline
Abstraction *tagged2Abstraction(TaggedRef term)
{
  Assert(isAbstraction(term));
  return (Abstraction *)tagged2Const(term);
}


/*===================================================================
 * Builtin (incl. BuiltinTabEntry)
 *=================================================================== */

class BuiltinTabEntry: public ConstTerm {
friend void ConstTerm::gcConstRecurse(void);
  friend class Debugger;
private:
  TaggedRef printname; //must be atom
  int arity;
  OZ_CFun fun;
  IFOR inlineFun;

public:

  /* use malloc to allocate memory */
  static void *operator new(size_t chunk_size)
  { return ::new char[chunk_size]; }

  BuiltinTabEntry(char *s,int arty,OZ_CFun fn,IFOR infun)
  : arity(arty),fun(fn), inlineFun(infun), ConstTerm(Co_Builtin)
  {
    printname = makeTaggedAtom(s);
  }

  ~BuiltinTabEntry () {}

  OZPRINT;
  OZPRINTLONG;

  OZ_CFun getFun() { return fun; }
  int getArity() { return arity; }
  char *getPrintName() { return tagged2Literal(printname)->getPrintName(); }
  TaggedRef getName() { return printname; }
  IFOR getInlineFun() { return inlineFun; }
};



inline
Bool isBuiltin(ConstTerm *s)
{
  return s->getType() == Co_Builtin;
}

inline
Bool isBuiltin(TaggedRef term)
{
  return isConst(term) && isBuiltin(tagged2Const(term));
}

inline
BuiltinTabEntry *tagged2Builtin(TaggedRef term)
{
  Assert(isBuiltin(term));
  return (BuiltinTabEntry *)tagged2Const(term);
}

/*===================================================================
 * Cell
 *=================================================================== */

#define NO_INDEX (0-1)

class CellLocal:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  TaggedRef val;
public:

  CellLocal(Board *b,TaggedRef v) : Tertiary(b, Co_Cell,Te_Local), val(v) {}
  OZPRINT;
  OZPRINTLONG;

  TaggedRef getValue() { return val; }
  void setValue(TaggedRef v) { val=v; }
  TaggedRef exchangeValue(TaggedRef v) {
    TaggedRef ret = val;
    val = v;
    return ret;}

  void globalize(int);
};


#define Cell_Invalid     0
#define Cell_Requested   1
#define Cell_Next        2
#define Cell_Valid       4
#define Cell_Dump_Asked  8
#define Cell_Access_Bit 16

class CellSec{
friend class CellFrame;
friend class CellManager;
private:
  short state;
  short int readCtr;
  TaggedRef head;
  PendThread* pending;
  int nextIndex;
  TaggedRef contents;

public:
  CellSec(TaggedRef val){ // on globalize
    state=Cell_Valid;
    readCtr=0;
    pending=NULL;
    nextIndex=NO_INDEX; // debug
    contents=val;}

  CellSec(){ // on Proxy becoming Frame
    state=Cell_Invalid;
    readCtr=0;

    DebugCode(head=makeTaggedNULL());
    DebugCode(contents=makeTaggedNULL());
    DebugCode(pending=NULL);
    DebugCode(nextIndex=NO_INDEX);}
};

class CellManager:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  CellSec *sec;
public:
  OZPRINT;
  OZPRINTLONG;

  CellManager() : Tertiary(0,Co_Cell,Te_Manager){Assert(0);}

  void incCtr(){sec->readCtr++;}
  int getAndInitCtr(){int i=sec->readCtr;sec->readCtr=0;return i;}

  void setOwnCurrent(){
    setPrimIndex(NO_INDEX);}
  Bool isOwnCurrent(){
    if(getPrimIndex()==NO_INDEX) return TRUE;
    return FALSE;}
  void setCurrent(int i){
    DebugIndexCheck(i);
    setPrimIndex(i);}
  int getCurrent(){
    return getPrimIndex();}

  int getOwnerIndex(){return getIndex();}
  void setOwnerIndex(int i){
    DebugIndexCheck(i);
    setIndex(i);}

  void localize();
  void gcCellManager();
};

class CellProxy:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  int * dummy;
public:
  OZPRINT;
  OZPRINTLONG;

 CellProxy(int manager):Tertiary(manager,Co_Cell,Te_Proxy){  // on import
   DebugIndexCheck(manager);
   DebugCode(setPrimIndex(NO_INDEX));
   setIndex(manager);}

  void convertToFrame(int myIndex);

  int getManagerIndex(){return getIndex();}
};

class CellFrame:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  CellSec *sec;
public:
  OZPRINT;
  OZPRINTLONG;

  CellFrame():Tertiary((Board*)NULL,Co_Cell,Te_Frame){Assert(0);}
  short getState(){
    Assert(sec!=NULL);
    return sec->state;}
  void setState(short s){
    Assert(sec!=NULL);
    sec->state=s;}

  Bool isAccessBit(){
    if(getState() & Cell_Access_Bit) return TRUE;
    return FALSE;}
  void setAccessBit(){
    setState(getState()| Cell_Access_Bit);}
  void resetAccessBit(){setState(getState() & (~Cell_Access_Bit));}

  void myStoreForward(void* forward){setPtr(forward);}
  void* getForward(){return getPtr();}

  int getManagerIndex(){return getIndex();}
  void setManagerIndex(int i){
    DebugIndexCheck(i);
    setIndex(i);}

  int getOwnerIndex(){return getPrimIndex();}
  void setOwnerIndex(int i){
    DebugIndexCheck(i);
    setPrimIndex(i);}

  void setPending(PendThread *pt){
    sec->pending=pt;}
  PendThread* getPending(){return sec->pending;}

  void incCtr(){sec->readCtr++;}
  void decCtr(int i){sec->readCtr -= i;}
  int getCtr(){return sec->readCtr;}

  TaggedRef getHead(){return sec->head;}
  void setHead(TaggedRef val){sec->head=val;}

  void setNext(int n){
    DebugIndexCheck(n);
    sec->nextIndex=n;}
  int getNext(){return sec->nextIndex;}

  TaggedRef getContents(){
    Assert(getState()&(Cell_Valid|Cell_Requested));return sec->contents;}
  void setContents(TaggedRef tr){sec->contents=tr;}

  void initFromProxy(int myIndex){
    DebugIndexCheck(myIndex);
    setOwnerIndex(myIndex);
    sec=new CellSec();}

  void initFromGlobalize(TaggedRef val){
    sec=new CellSec(val);}

  void convertToProxy(){
    DebugCode(setOwnerIndex(NO_INDEX));
    setTertType(Te_Proxy);
    DebugCode(sec=NULL);}

  void gcCellFrame();
  void gcCellFrameSec();
};


inline Bool isCell(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType() == Co_Cell;
}

/*===================================================================
 * Ports
 *=================================================================== */

class Port: public Tertiary {
friend void ConstTerm::gcConstRecurse(void);
public:
  Port(Board *b, TertType tt) : Tertiary(b,Co_Port,tt){}
};

class PortWithStream: public Port {
friend void ConstTerm::gcConstRecurse(void);
protected:
  TaggedRef strm;
public:
  TaggedRef exchangeStream(TaggedRef newStream)
  {
    TaggedRef ret = strm;
    strm = newStream;
    return ret;   }
  PortWithStream(Board *b, TaggedRef s) : Port(b,Te_Local)  {
    strm = s;}
};

class PortManager: public PortWithStream {
friend void ConstTerm::gcConstRecurse(void);
public:
  OZPRINTLONG;
  OZPRINT;
  PortManager() : PortWithStream(0,0) { Assert(0); };
};

/* ----------------------------------------------------
   PORTS    local               manager           proxy
lst word:   Co_Port:board       Co_Port:_         Co_Port:_
2nd word:   Te_Local:NO_ENTRY   Te_Manager:owner  Te_Proxy:borrow
3rd word    <<stream>>          <<stream>>        _
---------------------------------------------------- */

class PortLocal: public PortWithStream {
friend void ConstTerm::gcConstRecurse(void);
public:
  OZPRINTLONG;
  OZPRINT;
  PortLocal(Board *b, TaggedRef s) : PortWithStream(b,s) {};
};

class PortProxy: public Port {
friend void ConstTerm::gcConstRecurse(void);
public:
  PortProxy(int i): Port(0,Te_Proxy) { setIndex(i); }
  OZPRINTLONG;
  OZPRINT;
};

inline Bool isPort(TaggedRef term)
{ return isConst(term) && tagged2Const(term)->getType() == Co_Port;}

inline PortWithStream *tagged2PortWithStream(TaggedRef term)
{ return (PortWithStream *) tagged2Const(term);}

inline Port *tagged2Port(TaggedRef term)
{ return (Port*) tagged2Const(term);}

/*===================================================================
 * Space
 *=================================================================== */

class Space: public Tertiary {
friend void ConstTerm::gcConstRecurse(void);
private:
  Board *solve;
  // The solve pointer can be:
  // - 0 (the board is failed and has been discarded by the garbage
  //      collector)
  // - 1 (the space has been merged)
  // or a valid pointer
public:
  Space(Board *h, Board *s) : Tertiary(h,Co_Space,Te_Local), solve(s) {};
  Space(int i, TertType t) : Tertiary(i,Co_Space,t) {}

  OZPRINT;
  OZPRINTLONG;

  SolveActor *getSolveActor();
  Board *getSolveBoard() { return solve; }
  void  merge() { solve = (Board *) 1; }
  Bool isFailed();
  Bool isMerged();
};


inline
Bool isSpace(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType() == Co_Space;
}

inline
Space *tagged2Space(TaggedRef term)
{
  Assert(isSpace(term));
  return (Space *) tagged2Const(term);
}


/*===================================================================
 * Locks
 *=================================================================== */

class OzLock: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
private:
  Thread *locker;
  TaggedRef threads;
public:
  OzLock(Board *b) : ConstTermWithHome(b,Co_Lock)
  {
    locker = NULL;
    threads = nil();
  }

  void unlock()
  {
    Assert(!isRef(threads));
    Assert(locker);
    locker = NULL;

    if (!isNil(threads)) {
      /* wake first thread */
      TaggedRef var = head(threads);
      if (OZ_unify(var, NameUnit)==FAILED) {
        warning("OzLock::wakeThreads: unify failed");
      }
      threads = tail(threads);
    }
  }

  Bool isLocked(Thread *t) { return (locker==t); }
  TaggedRef *lock(Thread *t);

  OZPRINT;
  OZPRINTLONG;
};


inline
Bool isLock(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType() == Co_Lock;
}

inline
OzLock *tagged2Lock(TaggedRef term)
{
  Assert(isLock(term));
  return (OzLock *) tagged2Const(term);
}

/*===================================================================
 *
 *=================================================================== */

char *toC(OZ_Term);
TaggedRef reverseC(TaggedRef l);
TaggedRef appendI(TaggedRef x,TaggedRef y);
TaggedRef getUniqueName(char *s);

#endif
