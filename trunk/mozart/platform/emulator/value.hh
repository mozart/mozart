/*
 * FBPS Saarbr"ucken
 * Author: mehl
 * Last modified: $Date$ from $Author$
 * Version: $Revision$
 * State: $State$
 *
 * Values: literal, list, records
 */

#ifndef __VALUEHH
#define __VALUEHH

#ifdef INTERFACE
#pragma interface
#endif

/*===================================================================
 * global names and atoms
 *=================================================================== */
extern TaggedRef AtomNil, AtomCons, AtomPair, AtomVoid,
       AtomLess, AtomGreater, AtomSame, AtomUncomparable,
       AtomInt, AtomFloat, AtomTuple, AtomProcedure, AtomCell,
       AtomChunk,
       AtomRecord, AtomAtom, AtomName, AtomUnknown,
       AtomClosed, AtomVariable,
       NameTrue, NameFalse, AtomBool, AtomSup, AtomCompl,
       NameGroupVoid,
       NameTclName,
       AtomTclOption, AtomTclList, AtomTclPosition,
       AtomTclQuote, AtomTclString, AtomTclVS,
       AtomTclBatch,
       AtomError,
       AtomDot, AtomTagPrefix, AtomVarPrefix, AtomImagePrefix;

/*===================================================================
 * Literal
 *=================================================================== */

class Literal {
private:
  static int LiteralCurrentNumber;
  // 
  char *printName;
  Board *home;
  // home can be: NULL: an atom;
  //              ALLBITS: optimized (static) top-level name;
  //              <board-addr>: non-top-level name;
  int seqNumber; 
public:
  Literal (char *str = "", Bool flag = NO);
  Literal (Board *hb); 

  isAtom()     { return home == (Board *) NULL; }
  isOptName()  { return home == (Board *) ToPointer(ALLBITS); }
  isDynName()  { return !isAtom() && !isOptName(); }

  Board *getBoardFast(); // see am.icc

  Literal *gc ();       // for non-top-level names only;
  void gcRecurse (); // too;

// atoms are the only terms which are not allocated on heap and that's why, 
// its new-operator has to be overloaded to allocate global memory for atoms
  void *operator new(size_t size) {return ::new char[size]; }
  void operator  delete(void *p, size_t) { ::delete(p); }

// term functions

  OZPRINT;
  OZPRINTLONG;
  
  /* It's an assumption: assembler produces only one entry.   */
  /* OK means that this is equal constants.       */

  // name access
  char* getPrintName() { return printName; }
  int getSize() { return strlen(printName); }

  int getSeqNumber() { return (seqNumber); } 

  int hash() { return getSeqNumber(); }
};

class Name : public Literal {
public:
  USEHEAPMEMORY;

  Name(Board *hb) : Literal (hb)
  {
    Assert(hb != NULL);
  }
};

inline
Bool isAtom(TaggedRef term) {
  return isLiteral(tagTypeOf(term)) && tagged2Literal(term)->isAtom();
}


//mm2: one argument is guarantueed to be a literal ???
inline
Bool sameLiteral(TaggedRef a, TaggedRef b)
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

  // res == 0

  return (((a->getSeqNumber ()) < (b->getSeqNumber ())) ? -1 : 1);
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
#include "gmp.h"
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
Bool sameSmallInt(TaggedRef a, TaggedRef b)
{
  return (a == b);
}

inline
Bool smallIntCmp(TaggedRef a, TaggedRef b)
{
  return smallIntLess(a,b) ? -1 : (sameSmallInt(a,b) ? 0 : 1);
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
Bool sameFloat(TaggedRef a, TaggedRef b)
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
      OZ_warning("BigInt(%s): cannot convert",s);
      mpz_init_set_si(&value,0);
    }
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
  int BigInt2Int()
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

#define MKOP(op,mpop)							      \
  TaggedRef op(BigInt *b) {						      \
    BigInt *n = new BigInt();						      \
    mpop(&n->value,&value,&b->value);					      \
    return n->shrink();							      \
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
Bool sameBigInt(TaggedRef a, TaggedRef b)
{
  return tagged2BigInt(a)->equal(tagged2BigInt(b));
}

inline
Bool numberEq(TaggedRef a, TaggedRef b)
{
  Assert(tagTypeOf(a)==tagTypeOf(b));
  
  TypeOfTerm tag = tagTypeOf(a);
  switch(tag) {
  case OZFLOAT:  return sameFloat(a,b);
  case SMALLINT: return sameSmallInt(a,b);
  case BIGINT:   return sameBigInt(a,b);
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

  OZPRINT;
  OZPRINTLONG;
  
  void gcRecurse();
  LTuple *gc();
};


// functions for builtins and features
//   with explicit deref


inline
Bool isCons(TaggedRef term) {
  DEREF(term,_1,_2);
  return isLTuple(term);
}

inline
Bool isNil(TaggedRef term) {
  DEREF(term,_1,_2);
  return sameLiteral(term,AtomNil);
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
  DEREF(list,_1,_2);
  Assert(isLTuple(list));
  return tagged2LTuple(list)->getHead();
}

inline
TaggedRef tail(TaggedRef list)
{
  DEREF(list,_1,_2);
  Assert(isLTuple(list));
  return tagged2LTuple(list)->getTail();
}

// -------------------------------------------------------------------------
// --- Access functions: do not only deref input, but also output;


inline
TaggedRef headDeref(TaggedRef list)
{
  DEREF(list,zzz,tag);
  TaggedRef ret = tagged2LTuple(list)->getHead();
  DEREF(ret,_1,tag2);
  CHECK_NONVAR(ret);
  return ret;
}

inline
TaggedRef tailDeref(TaggedRef list)
{
  DEREF(list,zzz,tag)
  TaggedRef ret = tagged2LTuple(list)->getTail();
  DEREF(ret,_1,_2);
  CHECK_NONVAR(ret);
  return ret;
}

/*===================================================================
 * ConstTerm
 *=================================================================== */

/* must not match GCTAG (ie <> 13 (1101) !!!! */
enum TypeOfConst {
  Co_Board,
  Co_Actor,
  Co_Class,
  Co_HeapChunk,

  Co_Abstraction,
  Co_Object,   
  Co_Builtin,  
  Co_Cell,
  Co_Chunk
};


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
  void gcConstRecurse(void);

  void setTagged(TypeOfConst t, void *p) { ctu.tagged = makeTaggedRef((TypeOfTerm)t,p); }
  ConstTerm(TypeOfConst t)     { setTagged(t,NULL); }
  TypeOfConst getType() { return (TypeOfConst) tagTypeOf(ctu.tagged); }
  TypeOfConst typeOf()  { return getType(); }
  char *getPrintName();
  void *getPtr()        { return tagValueOf(ctu.tagged); }
  void setPtr(void *p)  { setTagged(getType(),p); }

  OZPRINT;
  OZPRINTLONG;

  Bool unify(TaggedRef, Bool) { return NO; }
  Bool install(TaggedRef t) { return unify(t,OK); };  
  Bool deinstall(TaggedRef) { return OK; };
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
Bool featureEq(TaggedRef a,TaggedRef b)
{
  CHECK_FEATURE(a);
  CHECK_FEATURE(b);
  if (isLiteral(a)) {
    // Note: if b is no literal this also returns NO
    return a==b ? OK : NO;
  }
  TypeOfTerm tagA = tagTypeOf(a);
  TypeOfTerm tagB = tagTypeOf(b);
  if (tagA != tagB) return NO;
  switch(tagA) {
  case SMALLINT: return sameSmallInt(a,b);
  case BIGINT:   return sameBigInt(a,b);
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
int featureHash(TaggedRef a)
{
  CHECK_FEATURE(a);
  TypeOfTerm tag = tagTypeOf(a);
  switch (tag) {
  case LITERAL:
    return tagged2Literal(a)->hash();
  case SMALLINT:
    return (int) a;
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

  int size;		// size is always a power of 2
  int hashmask;		// size-1, used as mask for hashing
  int width;	        // next unused index in RefsArray (for add())
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

typedef int32 SRecordArity; /* do not want to use a pointer on the Alpha! */

inline Bool sraIsTuple(SRecordArity a)      { return a&1; }
inline SRecordArity mkTupleWidth(int w)     { return (SRecordArity) ((w<<1)|1);}
inline int getTupleWidth(SRecordArity a)    { return a>>1; }
inline SRecordArity mkRecordArity(Arity *a) { return ToInt32(a); }
inline Arity *getRecordArity(SRecordArity a){ return (Arity*) ToPointer(a); }
inline Bool sameSRecordArity(SRecordArity a, SRecordArity b) { return a==b; }


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
  
  int getWidth() {
    return isTuple() ? getTupleWidth() : getRecordArity()->getWidth();
  }
  void downSize(unsigned int s) { // TMUELLER
    Assert(isTuple());
    setTupleWidth(s);
  } // FD
  
  static SRecord *newSRecord(TaggedRef lab, Arity *f)
  {
    if (f->isTuple()) return newSRecord(lab,f->getWidth());

    CHECK_LITERAL(lab);
    Assert(f != NULL);
    int sz = f->getWidth();
    int memSize = sizeof(SRecord) + sizeof(TaggedRef) * (sz - 1);
    SRecord *ret = (SRecord *) heapMalloc(memSize);
    ret->label = lab;
    ret->setRecordArity(f);
    return ret;
  }

  static SRecord *newSRecord(TaggedRef lab, int w)
  {
    CHECK_LITERAL(lab);
    Assert(w > 0);
    int memSize = sizeof(SRecord) + sizeof(TaggedRef) * (w - 1);
    SRecord *ret = (SRecord *) heapMalloc(memSize);
    ret->label = lab;
    ret->setTupleWidth(w);
    return ret;
  }

  // returns copy of tuple st (args are appropriately copied as well)
  static SRecord * newSRecord(SRecord * st)
  {
    SRecord *ret;
    if (st->isTuple()) {
      ret = newSRecord(st->label, st->getWidth());
    } else {
      ret = newSRecord(st->label, st->getArity());
    }
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
    return isTuple() ? makeTupleArityList(getWidth()) : getArity()->getList();
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

  Bool isPair() { return isTuple() && sameLiteral(label,AtomPair); }

  OZPRINT;
  OZPRINTLONG;

  Bool compareSortAndArity(TaggedRef lbl, Arity *ff) {
    return !isTuple() &&
      sameLiteral(getLabel(),lbl) &&
      getArity() == ff;
  }
  Bool compareSortAndSize(TaggedRef lbl, int ww) {
    return isTuple() &&
      sameLiteral(getLabel(),lbl) &&
      getWidth() == ww;
  }
  Bool compareFunctor(SRecord* str) {
    return sameLiteral(getLabel(),str->getLabel()) &&
      sameSRecordArity(recordArity,str->recordArity);
  }
};

TaggedRef sortlist(TaggedRef list,int len);
  
inline
Bool isRecord(TaggedRef term) {
  GCDEBUG(term);
  TypeOfTerm tag = tagTypeOf(term);
  return isSRecord(tag) || isLiteral(tag);
}


State adjoinPropList(TaggedRef t0, TaggedRef list, TaggedRef &out,
		     Bool recordFlag);

SRecord *makeRecord(TaggedRef t);


inline
int isPair(TaggedRef term) {
  DEREF(term,zzz,tag);
  return isSRecord(tag) && 
    tagged2SRecord(term)->isPair();
}

inline
int isSTuple(TaggedRef term) {
  return isSRecord(term) && tagged2SRecord(term)->isTuple();
}

inline
int isTuple(TaggedRef term) {
  return isLTuple(term) || isSTuple(term);
}

inline
TaggedRef left(TaggedRef pair)
{
  DEREF(pair,zzz,tag);
  Assert(isPair(pair));
  return tagged2SRecord(pair)->getArg(0);
}

inline
TaggedRef right(TaggedRef pair)
{
  DEREF(pair,zzz,tag);
  Assert(isPair(pair));
  return tagged2SRecord(pair)->getArg(1);
}

inline
TaggedRef arg(TaggedRef tuple, int i)
{
  DEREF(tuple,zzz,tag);
  Assert(isSTuple(tuple));
  return tagged2SRecord(tuple)->getArg(i);
}


inline
TaggedRef leftDeref(TaggedRef pair)
{
  TaggedRef ret = tagged2SRecord(pair)->getArg(0);
  DEREF(ret,_1,_2);
  CHECK_NONVAR(ret);
  return ret;
}

inline
TaggedRef rightDeref(TaggedRef pair)
{
  TaggedRef ret = tagged2SRecord(pair)->getArg(1);
  DEREF(ret,_1,_2);
  CHECK_NONVAR(ret);
  return ret;
}

inline
TaggedRef argDeref(TaggedRef tuple, int i)
{
  DEREF(tuple,zzz,tag)
  TaggedRef ret = tagged2SRecord(tuple)->getArg(i);
  DEREF(ret,_1,_2);
  CHECK_NONVAR(ret);
  return ret;
}

/*===================================================================
 * ObjectOrClass incl. ObjectClass, DeepObjectOrClass
 *=================================================================== */

/* Internal representation of Oz classes */

class ObjectClass: public ConstTerm {
private:
  SRecord *fastMethods;
  Literal *printName;
  TaggedRef slowMethods;
  Abstraction *send;
  SRecord *unfreeFeatures;
  TaggedRef ozclass;    /* the class as seen by the Oz user */

public:
  Bool hasFastBatch;    /* for optimized batches */

  ObjectClass(SRecord *fm, Literal *pn, TaggedRef sm, 
	      Abstraction *snd, Bool hfb, SRecord *uf): 
    ConstTerm(Co_Class) 
  {
    fastMethods    = fm;
    printName      = pn;
    slowMethods    = sm;
    send           = snd;
    hasFastBatch   = hfb;
    unfreeFeatures = uf;
    ozclass        = AtomNil;

  }
  TaggedRef getslowMethods()    { return slowMethods; }
  SRecord *getfastMethods()     { return fastMethods; }
  Abstraction *getAbstraction() { return send; }
  char *getPrintName()          { return printName->getPrintName(); }
  TaggedRef getOzClass()        { return ozclass; }
  void setOzClass(TaggedRef cl) { ozclass = cl; }

  TaggedRef getFeature(TaggedRef lit) 
  {
    return unfreeFeatures
      ? unfreeFeatures->getFeature(lit)
      : makeTaggedNULL();
  }

  SRecord *getUnfreeRecord() { return unfreeFeatures; }

  ObjectClass *gcClass();
  OZPRINT;
  OZPRINTLONG;
};


/*
 * Object
 */

typedef enum {
  OFlagClosed = 0x1,
  OFlagDeep   = 0x2,
  OFlagClass  = 0x4
} OFlag;

#define DeepnessShift 3

class Object: public ConstTerm {
  friend void ConstTerm::gcConstRecurse(void);
protected:
  int32 state;  // was: SRecord *state, but saves memory on the Alpha
  int32 aclass; // was: ObjectClass *aclass
  TaggedRef threads;  /* list of variables with threads attached to them */
  int32 deepness;     /* deepnes plus OFlag */
public:

  Object(SRecord *s,ObjectClass *ac,SRecord *feat,Bool iscl):
    ConstTerm(Co_Object)
  {
    setFreeRecord(feat);
    deepness = 0;
    threads = AtomNil;
    setClass(ac);
    setState(s);
    if (iscl) setClass();
  };

  int getDeepness()     { return (deepness >> DeepnessShift);}
  int incDeepness()     { deepness += (1<<DeepnessShift); return getDeepness();}
  int decDeepness()     { deepness -= (1<<DeepnessShift); return getDeepness();}

  Bool isClosedOrClassOrDeepOrLocked() { return (deepness!=0); }

  void setFlag(OFlag f) { deepness |= (int) f; } 

  Bool isClass()        { return (deepness)&OFlagClass; }
  void setClass()       { setFlag(OFlagClass); }
  Bool isDeep()         { return (deepness)&OFlagDeep; }
  void setIsDeep()      { setFlag(OFlagDeep); }
  Bool isClosed()       { return (deepness)&OFlagClosed; }
  void close()          { setFlag(OFlagClosed); }

  void setClass(ObjectClass *c) { aclass = ToInt32(c); }

  TaggedRef attachThread();
  inline void release();

  ObjectClass *getClass() { return (ObjectClass*) ToPointer(aclass); }

  Bool getFastBatch()     { return getClass()->hasFastBatch; }

  char *getPrintName()          { return getClass()->getPrintName(); }
  SRecord *getMethods()         { return getClass()->getfastMethods(); }
  Abstraction *getMethod(TaggedRef label, int arity);
  SRecord *getState()           { return (SRecord*) ToPointer(state); }
  void setState(SRecord *s)     { state = ToInt32(s); }
  Abstraction *getAbstraction() { return getClass()->getAbstraction(); }
  TaggedRef getSlowMethods()    { return getClass()->getslowMethods(); }
  TaggedRef getOzClass()        { return getClass()->getOzClass(); }
  Board *getBoardFast();
  SRecord *getFreeRecord()          { return (SRecord *) getPtr(); }
  SRecord *getUnfreeRecord() { 
    return isClass() ? NULL : getClass()->getUnfreeRecord(); 
  }
  void setFreeRecord(SRecord *aRec) { setPtr(aRec); }

  /* same functionality is also in instruction inlineDot */
  TaggedRef getFeature(TaggedRef lit) 
  {
    TaggedRef ret = getFreeRecord()->getFeature(lit);
    if (ret!=makeTaggedNULL())
      return ret;
    SRecord *fr = getUnfreeRecord();
    return fr ?  fr->getFeature(lit) : makeTaggedNULL();
  }

  TaggedRef getArityList();

  Object *gcObject();

  OZPRINT;
  OZPRINTLONG;
};


inline
void Object::release()
{
  if (getDeepness()==0) return; // !!!!!!!!
  if (decDeepness()==0) {
    /* wake threads */
    Assert(!isRef(threads));
    if (!sameLiteral(threads,AtomNil)) {
      incDeepness();
      TaggedRef var = head(threads);
      if (OZ_unify(var, isClosed() ? NameTrue : NameFalse)==FAILED) {
	warning("Object::wakeThreads: unify failed");
      }
      threads = tail(threads);
    }
  }
}

/* objects not created on toplevel need a home pointer */

class DeepObject: public Object {
  friend void ConstTerm::gcConstRecurse(void);
  friend class Object;
private:
  Board *home;
public:
  DeepObject(SRecord *s,ObjectClass *aclass,
	     SRecord *feat,Bool iscl, Board *bb):
    Object(s,aclass,feat,iscl)
  {
    setIsDeep();
    home=bb;
  };
};



inline
Bool isObject(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType()==Co_Object;
}

inline
Object *tagged2Object(TaggedRef term)
{
  Assert(isObject(term));
  return (Object *)tagged2Const(term);
}

/*===================================================================
 * SChunk
 *=================================================================== */

class SChunk: public ConstTerm {
friend void ConstTerm::gcConstRecurse(void);
private:
  Board *home;
public:
  SChunk(Board *b,SRecord *v) : ConstTerm(Co_Chunk), home(b) {
    setPtr(v);
  };

  OZPRINT;
  OZPRINTLONG;

  SRecord *getRecord() { return (SRecord *) getPtr(); }
  TaggedRef getFeature(TaggedRef fea) { return getRecord()->getFeature(fea); }
  Board *getBoardFast();
};


inline
Bool isSChunk(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType() == Co_Chunk;
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
  return (isSChunk(t) || isObject(t)) ? OK : NO;
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
  AssRegArray () { numbOfGRegs = 0; first = (AssReg *)NULL; }

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

class PrTabEntry {
private:
  TaggedRef printname; // must be atom
  unsigned short arity;
  unsigned short spyFlag;

public:
  AssRegArray gRegs;
  ProgramCounter PC;

  PrTabEntry (TaggedRef name, int arityInit, int numbOfGRegs)
  : printname(name),
    arity ((unsigned short) arityInit), gRegs (numbOfGRegs), spyFlag(NO)
  {
      Assert(arityInit==(unsigned short) arityInit);
      Assert(isLiteral(name));
  }

  OZPRINTLONG;
  
  int getArity () { return (int) arity; }
  int getGSize () { return gRegs.getSize(); }
  char *getPrintName () { return tagged2Literal(printname)->getPrintName(); }
  TaggedRef getName () { return printname; }
  ProgramCounter getPC() { return PC; }
  Bool getSpyFlag()   { return (Bool) spyFlag; }
  void setSpyFlag()   { spyFlag = OK; }
  void unsetSpyFlag() { spyFlag = NO; }
};

class Abstraction: public ConstTerm {
  friend void ConstTerm::gcConstRecurse(void);
private:
// DATA
  RefsArray gRegs;
  PrTabEntry *pred;
  Board *home;
public:
  Abstraction(PrTabEntry *prd, RefsArray gregs, Board *b)
  : ConstTerm(Co_Abstraction), gRegs(gregs), pred(prd)
  {
    home = b;
  }

  OZPRINT;
  OZPRINTLONG;

  Board *getBoardFast();
  RefsArray &getGRegs()  { return gRegs; }
  ProgramCounter getPC() { return pred->getPC(); }
  int getArity()         { return pred->getArity(); }
  int getGSize()         { return pred->getGSize(); }
  char *getPrintName()   { return pred->getPrintName(); }
  PrTabEntry *getPred()  { return pred; }
  TaggedRef getName()    { return pred->getName(); }

  TaggedRef DBGgetGlobals();
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
  return (s->getType() == Co_Abstraction) ? OK : NO;
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

// special builtins known in emulate
enum BIType {
  BIDefault,
  BIsolve,
  BIsolveEatWait,
  BIsolveDebug,
  BIsolveDebugEatWait,
  BIsolveCont,
  BIsolved,
  BIraise
};

class BuiltinTabEntry {
  friend class Debugger;
public:
  BuiltinTabEntry (Literal *name,int arty,OZ_CFun fn,
		   IFOR infun=NULL)
  : printname(makeTaggedLiteral(name)), arity(arty),fun(fn),
    inlineFun(infun), type(BIDefault)
  {
    Assert(isAtom(printname));
  }
  BuiltinTabEntry (char *s,int arty,CFun fn,
		   IFOR infun=NULL)
  : arity(arty),fun(fn), inlineFun(infun), type(BIDefault)
  {
    printname = makeTaggedAtom(s);
    Assert(isAtom(printname));
  }
  BuiltinTabEntry (char *s,int arty,CFun fn,BIType t,
		   IFOR infun=NULL)
    : arity(arty),fun(fn), inlineFun(infun), type(t) {
      printname = makeTaggedAtom(s);
    }
  BuiltinTabEntry (char *s,int arty,BIType t, IFOR infun=(IFOR)NULL)
    : arity(arty),fun((CFun)NULL), inlineFun(infun), type(t)
  {
    printname = makeTaggedAtom(s);
    Assert(isAtom(printname));
  }

  ~BuiltinTabEntry () {}

  OZPRINT;
  CFun getFun() { return fun; }
  int getArity() { return arity; }
  char *getPrintName() { return tagged2Literal(printname)->getPrintName(); }
  TaggedRef getName() { return printname; }
  IFOR getInlineFun() { return inlineFun; } 
  BIType getType() { return type; } 

private:

  TaggedRef printname; //must be atom
  int arity;
  CFun fun;
  IFOR inlineFun;
  BIType type;
};



class Builtin: public ConstTerm {
friend void ConstTerm::gcConstRecurse(void);
private:
  BuiltinTabEntry *fun;
  TaggedRef suspHandler; // this one is called, when it must suspend
protected:
  RefsArray gRegs;       // context;
public:
  Builtin(BuiltinTabEntry *fn, TaggedRef handler, RefsArray gregs = NULL)
    : suspHandler(handler), fun(fn), ConstTerm(Co_Builtin),
    gRegs (gregs) {}

  OZPRINT;
  OZPRINTLONG;

  int getArity()                    { return fun->getArity(); }
  CFun getFun()                     { return fun->getFun(); }
  char *getPrintName()              { return fun->getPrintName(); }
  TaggedRef getName()               { return fun->getName(); }
  BIType getType()                  { return fun->getType(); } 
  TaggedRef getSuspHandler()        { return suspHandler; }
  BuiltinTabEntry *getBITabEntry()  { return fun; }
};

/*
 * Essential Note:
 *  If gregs == NULL, *that* builtin was already applied,
 *  and 'isSeen' says 'OK'!
 *  'hasSeen' removes simply the gregs;
 */
class OneCallBuiltin: public Builtin {
public:
  USEHEAPMEMORY;

  OneCallBuiltin (BuiltinTabEntry *fn, RefsArray gregs)
    : Builtin (fn, (TaggedRef) 0, gregs) {}

  Bool isSeen ()        { return (gRegs == NULL); }
  RefsArray &getGRegs() { return(gRegs); }
  void hasSeen ()       { gRegs = (RefsArray) NULL; }
};

class SolvedBuiltin: public Builtin {
public:
  USEHEAPMEMORY;

  SolvedBuiltin(BuiltinTabEntry *fn, RefsArray gregs)
    : Builtin (fn, (TaggedRef) 0, gregs) {}

  RefsArray &getGRegs() { return(gRegs); }
};

inline
Bool isBuiltin(ConstTerm *s)
{
  return (s->getType() == Co_Builtin) ? OK : NO;
}

inline
Bool isBuiltin(TaggedRef term)
{
  return isConst(term) && isBuiltin(tagged2Const(term));
}

inline
Builtin *tagged2Builtin(TaggedRef term)
{
  Assert(isBuiltin(term));
  return (Builtin *)tagged2Const(term);
}

/*===================================================================
 * Cell
 *=================================================================== */

class Cell: public ConstTerm {
friend void ConstTerm::gcConstRecurse(void);
private:
  TaggedRef val;
  Board *home;
public:
  Cell(Board *b,TaggedRef v) : ConstTerm(Co_Cell), home(b), val(v) {};

  OZPRINT;
  OZPRINTLONG;

  TaggedRef getValue() { return val; }
  TaggedRef exchangeValue(TaggedRef v) {
    TaggedRef ret = val;
    val = v;
    return ret;
  }
  Board *getBoardFast();
};


inline Bool isCell(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType() == Co_Cell;
}

inline
Cell *tagged2Cell(TaggedRef term)
{
  Assert(isCell(term));
  return (Cell *) tagged2Const(term);
}

/*===================================================================
 * 
 *=================================================================== */

#endif
