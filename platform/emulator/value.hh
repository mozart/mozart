/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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

// Values: literal, list, records, ...

#ifndef __VALUEHH
#define __VALUEHH

#ifdef INTERFACE
#pragma interface
#endif

#include "tagged.hh"
#include "dpInterface.hh"
#include "atoms.hh"

/*===================================================================
 * global names and atoms
 *=================================================================== */

extern TaggedRef 
  RecordFailure,

  _NameTrue, _NameFalse,

  BI_Unify,BI_send,BI_Delay,
  BI_load,BI_fail,BI_url_load, BI_obtain_native,
  BI_dot,
  BI_exchangeCell,BI_assign,BI_atRedo,
  BI_controlVarHandler,
  BI_unknown,

  __UNUSED_DUMMY_END_MARKER;

extern Builtin *bi_raise, *bi_raiseError, *bi_raiseDebug;

// hack to avoid including am.hh
extern Board *oz_rootBoardOutline();

/*===================================================================
 * Literal
 *=================================================================== */


#define Lit_isName          2
#define Lit_isNamedName     4
#define Lit_hasGName        8
#define Lit_isUniqueName   16
#define Lit_isCopyableName 32

const int sizeOfLitFlags  = 6;
const int sizeOfCopyCount = 10;
const int litFlagsMask    = (1<<sizeOfLitFlags)-1;
const int copyCountMask   = (1<<sizeOfCopyCount)-1;

class Literal {
friend TaggedRef oz_uniqueName(const char *s);

private:
  int32 flagsAndOthers;

protected:
  void init() { flagsAndOthers=0; }
  void setFlag(int flag) { flagsAndOthers |= flag; }
  void clearFlag(int flag) { flagsAndOthers &= ~flag; }
  int getFlags() { return (flagsAndOthers&litFlagsMask); }

public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Literal);

  int getOthers() { return flagsAndOthers>>sizeOfLitFlags; }
  void setOthers(int value) { flagsAndOthers = getFlags()|(value<<sizeOfLitFlags); }

  Bool isName()         { return (getFlags()&Lit_isName); }
  Bool isNamedName()    { return (getFlags()&Lit_isNamedName); }
  Bool isUniqueName()   { return (getFlags()&Lit_isUniqueName); }
  Bool isCopyableName() { return (getFlags()&Lit_isCopyableName); }
  Bool isAtom()         { return !isName(); }

  const char *getPrintName();

  Literal *gc();

  inline unsigned int hash();  
};

class Atom: public Literal {
private:
  const char *printName;
public:
  NO_DEFAULT_CONSTRUCTORS(Atom);
  static Atom *newAtom(const char *str);
  const char* getPrintName() { return printName; }
  int getSize() { return getOthers(); }
  unsigned int hash() { return ToInt32(this)>>3; }
};

/* This one goes onto the heap */
class Name: public Literal {
protected:
  static int NameCurrentNumber;
  int32 homeOrGName;
public:
  NO_DEFAULT_CONSTRUCTORS(Name);
  static Name *newName(Board *b);

  Board *getBoardInternal() {
    return (hasGName() || isNamedName())
      ? oz_rootBoardOutline() : (Board*)ToPointer(homeOrGName);
  }

  int getSeqNumber() { return getOthers(); }
  unsigned int hash() { return getSeqNumber(); }

  Bool isOnHeap() { return (getFlags()&Lit_isNamedName)==0; }
  Bool hasGName() { return (getFlags()&Lit_hasGName); }

  Name *gcName();
  void gcRecurse();

  GName *getGName1() { return (GName*) ToPointer(homeOrGName); }
  GName *globalize();
  void import(GName *);
};


/* This one managed via malloc */

class NamedName: public Name {
public:
  NO_DEFAULT_CONSTRUCTORS(NamedName);
  const char *printName;
  static NamedName *newNamedName(const char *str);
  static NamedName *newCopyableName(const char *str);
  NamedName *generateCopy();
};

unsigned int Literal::hash()
{
  if (isAtom()) return ((Atom*)this)->hash();
  return ((Name*)this)->hash();
}

inline
Bool oz_isAtom(TaggedRef term) {
  return oz_isLiteral(term) && tagged2Literal(term)->isAtom();
}

inline
Bool oz_isName(TaggedRef term) {
  return oz_isLiteral(term) && tagged2Literal(term)->isName();
}

inline OZ_Term oz_true()  { return _NameTrue; }
inline OZ_Term oz_false() { return _NameFalse; }
inline OZ_Term oz_unit()  { return NameUnit; }

inline
OZ_Term oz_bool(int i) {
  return i ? oz_true() : oz_false();
}

inline
Bool oz_isTrue(OZ_Term t) {
  return oz_eq(t,oz_true());
}

inline
Bool oz_isFalse(OZ_Term t) {
  return oz_eq(t,oz_false());
}

inline
Bool oz_isBool(TaggedRef term) {
  return oz_isTrue(term) || oz_isFalse(term);
}

/*
 * atomcmp(a,b) is used to construct the arity lists of records.
 * It returns: 0   if   a == b
 *            -1   if   a < b
 *             1   if   a > b
 */

inline
int atomcmp(Literal *a, Literal *b)
{
  if (a==b) return 0;

  if (a->isName() != b->isName()) return a->isName() ? -1 : 1;

  int res = strcmp(a->getPrintName(), b->getPrintName());
  if (res < 0) return -1;
  if (res > 0) return  1;

  Assert(a->isName() && b->isName());
  return (((Name*)a)->getSeqNumber() < ((Name*)b)->getSeqNumber()) ? -1 : 1;
}

// see codearea.cc
extern TaggedRef oz_atom(const char *s);
extern TaggedRef oz_uniqueName(const char *s);


/*===================================================================
 * Numbers
 *=================================================================== */

// #include <math.h>
#include <limits.h>

extern "C" {
#include <gmp.h>
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
const unsigned long OzMaxUnsignedLong = ~0;


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
  Assert(oz_isSmallInt(t));
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
  Assert(oz_isSmallInt(A) && oz_isSmallInt(B));
  int ahelp = A;
  int bhelp = B;
  return ahelp < bhelp;
}

inline
Bool smallIntLE(TaggedRef A, TaggedRef B)
{
  Assert(oz_isSmallInt(A) && oz_isSmallInt(B));
  int ahelp = A;
  int bhelp = B;
  return ahelp <= bhelp;
}

inline
unsigned int smallIntHash(TaggedRef n)
{
  return ((int)n)>>tagSize;
}


inline
Bool smallIntEq(TaggedRef a, TaggedRef b)
{
  Assert(oz_isSmallInt(a) || oz_isSmallInt(b));
  return (a == b);
}

inline
Bool smallIntCmp(TaggedRef a, TaggedRef b)
{
  Assert(oz_isSmallInt(a) && oz_isSmallInt(b));
  return smallIntLess(a,b) ? -1 : (smallIntEq(a,b) ? 0 : 1);
}

/*===================================================================
 * Float
 *=================================================================== */

class Float {
protected:
  double value;

public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Float);
  static Float *newFloat(double val);
  double getValue() { return value; }
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
TaggedRef oz_float(double i)
{
  return makeTaggedFloat(Float::newFloat(i));
}

#define CHECK_LITERAL(lab) \
Assert(!oz_isRef(lab) && !oz_isVariable(lab) && oz_isLiteral(lab));


/*===================================================================
 * LTuple
 *=================================================================== */

class LTuple {
private:
  TaggedRef args[2];

public:
  USEHEAPMEMORY32;
  OZPRINTLONG;

  NO_DEFAULT_CONSTRUCTORS2(LTuple);
  LTuple(void) {
    DebugCheckT(args[0]=0;args[1]=0);
    COUNT1(sizeLists,sizeof(LTuple));
  }
  LTuple(TaggedRef head, TaggedRef tail) {
    COUNT1(sizeLists,sizeof(LTuple));
    args[0] = head; args[1] = tail;
  }

  void gcRecurse();
  LTuple *gc();

  TaggedRef getHead()          { return tagged2NonVariable(args); }
  TaggedRef getTail()          { return tagged2NonVariable(args+1); }
  void setHead(TaggedRef term) { args[0] = term;}
  void setTail(TaggedRef term) { args[1] = term;}
  TaggedRef *getRef()          { return args; }
  TaggedRef *getRefTail()      { return args+1; }
  TaggedRef *getRefHead()      { return args; }
};


inline
TaggedRef oz_nil() { return AtomNil; }

inline
Bool oz_isCons(TaggedRef term) {
  return oz_isLTuple(term);
}

inline
Bool oz_isNil(TaggedRef term) {
  return oz_eq(term,oz_nil());
}

inline
TaggedRef oz_cons(TaggedRef head, TaggedRef tail)
{
  return makeTaggedLTuple(new LTuple(head,tail));
}

inline
TaggedRef oz_cons(Literal *head, TaggedRef tail)
{
  return oz_cons(makeTaggedLiteral(head),tail);
}

inline
TaggedRef oz_cons(char *head, TaggedRef tail)
{
  return oz_cons(oz_atom(head),tail);
}

inline
TaggedRef oz_mklist(TaggedRef l1,TaggedRef l2) {
  return oz_cons(l1,oz_cons(l2,oz_nil()));
}

inline
TaggedRef oz_mklist(TaggedRef l1,TaggedRef l2,TaggedRef l3) {
  return oz_cons(l1,oz_mklist(l2,l3));
}

inline
TaggedRef oz_mklist(TaggedRef l1,TaggedRef l2,TaggedRef l3,TaggedRef l4) {
  return oz_cons(l1,oz_mklist(l2,l3,l4));
}

OZ_Term oz_list(OZ_Term t, ...);

inline
TaggedRef oz_head(TaggedRef list)
{
  Assert(oz_isLTuple(list));
  return tagged2LTuple(list)->getHead();
}

inline
TaggedRef oz_tail(TaggedRef list)
{
  Assert(oz_isLTuple(list));
  return tagged2LTuple(list)->getTail();
}

inline
int oz_fastlength(OZ_Term l)
{
  int len = 0;
  l = oz_deref(l);
  while (oz_isCons(l)) {
    len++;
    l = oz_deref(oz_tail(l));
  }
  return len;
}



/*===================================================================
 * ConstTerm
 *=================================================================== */

/* when adding a new type update
 *   gc and gcRecurse
 *   print (print.cc and foreign.cc)
 *   OZ_toVirtualString
 *   finalizable
 *   marshaling/unmarshaling
 */
const int Co_Bits = 16;
const int Co_Mask = (1<<Co_Bits)-1;

enum TypeOfConst {
  Co_BigInt,
  Co_Foreign_Pointer,
  Co_Abstraction,
  Co_Builtin,
  Co_Cell,
  Co_Space,
  Co_Resource,

  /* chunks must stay together and the first one must be Co_Object
   * otherwise you'll have to change the "isChunk" test
   * NOTE: update the builtins: subtree and chunkArity, when adding new chunks
   */
  Co_Object,
  Co_Port,
  Co_Chunk,
  Co_Array,
  Co_Dictionary,
  Co_Lock,
  Co_Class
};

#define Co_ChunkStart Co_Object

enum TertType {
  Te_Local   = 0, // 0000
  Te_Manager = 1, // 0001
  Te_Proxy   = 2, // 0010
  Te_Frame   = 3  // 0011
};

#define DebugIndexCheck(IND) {Assert(IND< (1<<27));Assert(IND>=0);}

class ConstTerm {
private:
  int32 tag;
public:
  USEHEAPMEMORY;
  OZPRINTLONG;
  ConstTerm() { Assert(0); }
  ConstTerm(TypeOfConst t) { init(t); }
  void init(TypeOfConst t) { tag = t<<1; }
  Bool gcIsMarked(void)        { return tag&1; }
  void gcMark(ConstTerm * fwd) { tag |= 1; }
  void ** gcGetMarkField(void) { return (void **) &tag; }
  ConstTerm * gcGetFwd(void) {
    Assert(gcIsMarked());
    return (ConstTerm *) (tag&~1);
  }
  ConstTerm *gcConstTerm(void);
  void gcConstRecurse(void);

  TypeOfConst getType() { return (TypeOfConst) ((tag & Co_Mask)>>1); }

  void setVal(int val) {
    tag = (tag & Co_Mask) | (val << Co_Bits);
  }
  int getVal() {
    return tag >> Co_Bits;
  }
  const char *getPrintName();
  int getArity();
};


#define CWH_Board 0
#define CWH_GName 1


class ConstTermWithHome: public ConstTerm {
private:
  Tagged2 boardOrGName;

protected:
  void setBoard(Board *b)
  {
    boardOrGName.set(b,CWH_Board);
  }
public:
  ConstTermWithHome() { Assert(0); }
  ConstTermWithHome(Board *bb, TypeOfConst tt) : ConstTerm(tt) { setBoard(bb);}

  void init(Board *bb, TypeOfConst tt) { ConstTerm::init(tt); setBoard(bb); }

  Board *getBoardInternal() {
    return hasGName() ? oz_rootBoardOutline() : (Board*)boardOrGName.getPtr();
  }

  void gcConstTermWithHome();
  void setGName(GName *gn) { 
    Assert(gn);
    boardOrGName.set(gn,CWH_GName);
  }
  Bool hasGName() { return (boardOrGName.getTag()&CWH_GName); }
  GName *getGName1() {
    return hasGName()?(GName *)boardOrGName.getPtr():(GName *)NULL;
  }
};

/*===================================================================
 * BigInt
 *=================================================================== */

BigInt *newBigInt();
BigInt *newBigInt(long i);
BigInt *newBigInt(unsigned long i);
BigInt *newBigInt(int i);
BigInt *newBigInt(unsigned int i);
BigInt *newBigInt(char *s);


class BigInt : public ConstTerm {
private:
  MP_INT value;

public:
  USEFREELISTMEMORY;
  OZPRINT;

  NO_DEFAULT_CONSTRUCTORS2(BigInt);

  BigInt() : ConstTerm(Co_BigInt)
  {
    mpz_init(&value);
  }
  BigInt(long i) : ConstTerm(Co_BigInt)
  {
    mpz_init_set_si(&value,i);
  }
  BigInt(unsigned long i) : ConstTerm(Co_BigInt)
  {
    mpz_init_set_ui(&value,i);
  }
  BigInt(int i) : ConstTerm(Co_BigInt)
  {
    mpz_init_set_si(&value,i);
  }
  BigInt(unsigned int i) : ConstTerm(Co_BigInt)
  {
    mpz_init_set_ui(&value,i);
  }

  BigInt(char *s) : ConstTerm(Co_BigInt)
  {
    if(mpz_init_set_str(&value, s, 10)) {
      Assert(0);
    }
  }
  BigInt(MP_INT *i) : ConstTerm(Co_BigInt)
  {
    mpz_init_set(&value, i);
  }
  void dispose() 
  {
    mpz_clear(&value);
    freeListDispose(this,sizeof(BigInt));
  }
/* make a small int if <Big> fits into it, else return big int */
  inline TaggedRef shrink(); // see below

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

  /* make an 'unsigned long' if <Big> fits into it, else return 0,~0 */
  unsigned long getUnsignedLong()
  {
    if (mpz_cmp_ui(&value,OzMaxUnsignedLong) > 0) {
      return OzMaxUnsignedLong;
    } else if (mpz_cmp_si(&value,0) < 0) {
      return 0;
    } else {
      return mpz_get_ui(&value);
    }
  }


  Bool equal(BigInt *b) {
    return (mpz_cmp(&value, &b->value) == 0);
  }

#define MKOP(op,mpop)							      \
  TaggedRef op(BigInt *b) {						      \
    BigInt *n = newBigInt();						      \
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
    BigInt *n = newBigInt();
    mpz_neg(&n->value,&value);
    return n->shrink();
  }
  int cmp(BigInt *b)      { return mpz_cmp(&value,&b->value); }
  int cmp(long int i)     { return mpz_cmp_si(&value,i); }
  Bool less(BigInt *b)    { return cmp(b) < 0; }
  Bool leq(BigInt *b)     { return cmp(b) <= 0; }
  int stringLength()      { return mpz_sizeinbase(&value,10)+2; }
  void getString(char *s) { mpz_get_str(s,10,&value); }

  unsigned int hash()              { return 75; } // all BigInt hash to same value
  BigInt *gc();
};


inline
Bool oz_isBigInt(TaggedRef term) {
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_BigInt;
}

inline
Bool oz_isInt(TaggedRef term) {
  GCDEBUG(term);
  return oz_isSmallInt(term) || oz_isBigInt(term);
}

inline
Bool oz_isNumber(TaggedRef term) {
  return oz_isInt(term) || oz_isFloat(term);
}

inline
TaggedRef BigInt::shrink()
{
  TaggedRef ret;
  if (mpz_cmp_si(&value,OzMaxInt) > 0 ||
      mpz_cmp_si(&value,OzMinInt) < 0)
    ret = makeTaggedConst(this);
  else {
    ret =  newSmallInt((int) mpz_get_si(&value));
    dispose();
  }
  return ret;
}

inline
BigInt *tagged2BigInt(TaggedRef term)
{
  Assert(oz_isBigInt(term));
  return (BigInt *)tagged2Const(term);
}

inline
TaggedRef oz_int(int i)
{
  if (i > OzMaxInt || i < OzMinInt) 
    return makeTaggedConst(newBigInt(i));
  else
    return newSmallInt(i);
}

inline
TaggedRef oz_ulong(unsigned long l) {
  if (l > (unsigned long) OzMaxInt) 
    return makeTaggedConst(newBigInt(l));
  else
    return newSmallInt(l);
}

inline
int oz_intToC(TaggedRef term)
{
  if (oz_isSmallInt(term)) {
    return smallIntValue(term);
  }

  return tagged2BigInt(term)->getInt();
}

OZ_Term oz_long(long i);
OZ_Term oz_unsignedLong(unsigned long i);

inline
TaggedRef oz_unsignedInt(unsigned int i)
{
  if (i > (unsigned int) OzMaxInt) 
    return makeTaggedConst(newBigInt(i));
  else
    return newSmallInt(i);
}

inline
Bool bigIntEq(TaggedRef a, TaggedRef b)
{
  return oz_isBigInt(a) && oz_isBigInt(b)
    && tagged2BigInt(a)->equal(tagged2BigInt(b));
}

inline
Bool oz_numberEq(TaggedRef a, TaggedRef b)
{
  TypeOfTerm tag = tagTypeOf(a);
  if (tag != tagTypeOf(b)) return NO;
  switch(tag) {
  case OZFLOAT:  return floatEq(a,b);
  case SMALLINT: return smallIntEq(a,b);
  case OZCONST:  return bigIntEq(a,b);
  default:       return NO;
  }
}

/*===================================================================
 * Tertiary
 *=================================================================== */

class Tertiary: public ConstTerm {
private:
  Tagged2 tagged; // TertType + Board || TertType + OTI
  EntityInfo* info;
public:

  TertType getTertType()       { return (TertType) tagged.getTag(); }
  void setTertType(TertType t) { tagged.set(tagged.getData(),(int) t); }

  NO_DEFAULT_CONSTRUCTORS(Tertiary);
  Tertiary() { Assert(0); }	// keep gcc happy;
  Tertiary(Board *b, TypeOfConst s,TertType t) : ConstTerm(s) {
    setTertType(t);
    info=NULL;
    setBoard(b);}
  Tertiary(int i, TypeOfConst s,TertType t) : ConstTerm(s) 
  {
    setTertType(t);
    info=NULL;
    setIndex(i);
  }

  //
  EntityInfo *getInfo() { return (info); }
  void setInfo(EntityInfo *infoIn) { info = infoIn; }

  void setIndex(int i) { tagged.setVal(i); }
  int getIndex() { return tagged.getData(); }
  void setPointer (void *p) { tagged.setPtr(p); }
  void *getPointer() { return tagged.getPtr(); }

  Bool checkTertiary(TypeOfConst s,TertType t){
    return (s==getType() && t==getTertType());}

  Board *getBoardInternal() {
    return isLocal() ? (Board*)getPointer() : oz_rootBoardOutline();}

  Bool isLocal()   { return (getTertType() == Te_Local); }
  Bool isManager() { return (getTertType() == Te_Manager); }
  Bool isProxy()   { return (getTertType() == Te_Proxy); }
  Bool isFrame()   { return (getTertType() == Te_Frame); }

  void setBoard(Board *b);
};

/*===================================================================
 * ForeignPointer
 *=================================================================== */

class ForeignPointer: public ConstTerm {
private:
  void* ptr;
public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS2(ForeignPointer);

  ForeignPointer():ConstTerm(Co_Foreign_Pointer),ptr(0){}
  ForeignPointer(void*p):ConstTerm(Co_Foreign_Pointer),ptr(p){}
  void*getPointer(){ return ptr; }
  ForeignPointer* gc(void);
};

/*===================================================================
 * SRecord: incl. Arity, ArityTable
 *=================================================================== */

inline
Bool oz_isFeature(TaggedRef lab) { return oz_isLiteral(lab) || oz_isInt(lab); }

#define CHECK_FEATURE(lab) \
Assert(!oz_isRef(lab) && !oz_isVariable(lab) && oz_isFeature(lab));


int featureEqOutline(TaggedRef a, TaggedRef b);

inline
int featureEq(TaggedRef a,TaggedRef b)
{
  CHECK_FEATURE(a);
  CHECK_FEATURE(b);
  return a == b || featureEqOutline(a,b);
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
    if (tagA==OZCONST) {
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
  case OZCONST:
    return tagged2BigInt(a)->cmp(tagged2BigInt(b));
  default:
    OZ_error("featureCmp");
    return 0;
  }
}


/*
 * Hash function for Features:
 * NOTE: all bigints are hashed to the same value (mm2)
 */
inline
unsigned int featureHash(TaggedRef a)
{
  CHECK_FEATURE(a);
  const TypeOfTerm tag = tagTypeOf(a);
  if (tag == LITERAL) {
    return tagged2Literal(a)->hash();
  } else if (tag == SMALLINT) {
    return smallIntHash(a);
  } else {
    return tagged2BigInt(a)->hash();
  }
}

class KeyAndIndex {
public:
  TaggedRef key;
  int index;
  NO_DEFAULT_CONSTRUCTORS(KeyAndIndex);
};

class Arity {
friend class ArityTable;
private:
  static Arity *newArity(TaggedRef, Bool);

  void gc();

  TaggedRef list;
  Arity *next;

  int hashmask;		// size-1, used as mask for hashing and opt marker
  int width;	        // next unused index in RefsArray (for add())
  DebugCheckT(int numberOfCollisions;)

  KeyAndIndex table[1];

  int scndhash(int i) { return ((i&7)<<1)|1; }
  int hashfold(int i) { return i&hashmask; }

public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS(Arity);

  Bool isTuple() { return hashmask == 0; }

  int getCollisions() {
    DebugCode(return numberOfCollisions);
    return -1;
  }

  // use SRecord::getIndex instead of this!
  int lookupInternal(TaggedRef entry);   // return -1, if argument not found.

  TaggedRef getList() { return list; }
  int getWidth()      { return width; }
  int getSize()       { return hashmask+1; }
};

#define ARITYTABLESIZE 8000

class ArityTable {
friend class Arity;
public:
  ArityTable ( int );
  Arity *find ( TaggedRef);
  void gc();
  OZPRINT;
  void printStat();

private:

  Bool hashvalue( TaggedRef, int & );
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
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(SRecord);

  SRecord *gcSRecord();
  
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

  void initArgs();
  
  int getWidth() { return ::getWidth(getSRecordArity()); }

  void downSize(unsigned int s) {
    Assert(isTuple());
    setTupleWidth(s);
  }
  
  static SRecord *newSRecord(TaggedRef lab, SRecordArity arity, int width)
  {
    CHECK_LITERAL(lab);
    Assert(width > 0);
    int memSize = sizeof(SRecord) + sizeof(TaggedRef) * (width - 1);
    COUNT1(sizeRecords,memSize);
    SRecord *ret = (SRecord *) int32Malloc(memSize);
    ret->label = lab;
    ret->recordArity = arity;
    return ret;
  }

  int sizeOf()
  {
    return sizeof(SRecord) + sizeof(TaggedRef) * (getWidth() - 1);
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
  
  void setFeatures(TaggedRef proplist);
  
  TaggedRef getLabel() { return label; }
  void setLabelInternal(TaggedRef l) { label=l; }
  Literal *getLabelLiteral() { return tagged2Literal(label); }
  void setLabelForAdjoinOpt(TaggedRef newLabel) { label = newLabel; }
  
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
      if (!oz_isSmallInt(feature)) return -1;
      int f=smallIntValue(feature);
      return (1 <= f && f <= getWidth()) ? f-1 : -1;
    }
    return getRecordArity()->lookupInternal(feature);
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

  Bool compareSortAndArity(TaggedRef lbl, SRecordArity arity) {
    return oz_eq(getLabel(),lbl) &&
           sameSRecordArity(getSRecordArity(),arity);
  }

  Bool compareFunctor(SRecord* str) {
    return compareSortAndArity(str->getLabel(),str->getSRecordArity());
  }
};

TaggedRef oz_adjoinAt(SRecord *, TaggedRef feature, TaggedRef value);
TaggedRef oz_adjoin(SRecord *, SRecord *);
TaggedRef oz_adjoinList(SRecord *, TaggedRef arity, TaggedRef proplist);

Bool isSorted(TaggedRef list);

TaggedRef duplist(TaggedRef list, int &len);
TaggedRef sortlist(TaggedRef list,int len);
TaggedRef packsort(TaggedRef list);
  
inline
Bool oz_isRecord(TaggedRef term) {
  GCDEBUG(term);
  TypeOfTerm tag = tagTypeOf(term);
  return isSRecordTag(tag) || isLTupleTag(tag) || isLiteralTag(tag);
}


SRecord *makeRecord(TaggedRef t);

inline
int oz_isSTuple(TaggedRef term) {
  return oz_isSRecord(term) && tagged2SRecord(term)->isTuple();
}

inline
int oz_isTuple(TaggedRef term) {
  return oz_isLTuple(term) || oz_isSTuple(term) || oz_isLiteral(term);
}

inline
OZ_Term getArityList(OZ_Term term)
{
  if (oz_isSRecord(term)) {
    return tagged2SRecord(term)->getArityList();
  }
  if (oz_isLTuple(term)) {
    return makeTupleArityList(2);
  }
  if (oz_isLiteral(term)) {
    return oz_nil();
  }
  return 0;
}

inline
int getWidth(OZ_Term term)
{
  if (oz_isSRecord(term)) {
    return tagged2SRecord(term)->getWidth();
  }
  if (oz_isLTuple(term)) {
    return (2);
  }
  if (oz_isLiteral(term)) {
    return (0);
  }
  return (0);			// ???
}

inline
Arity *oz_makeArity(OZ_Term list)
{
  list=packsort(list);
  if (!list) return 0;
  return aritytable.find(list);
}

inline
int oz_isPair(OZ_Term term)
{
  if (oz_isLiteral(term)) return oz_eq(term,AtomPair);
  if (!oz_isSRecord(term)) return 0;
  SRecord *sr = tagged2SRecord(term);
  if (!sr->isTuple()) return 0;
  return oz_eq(sr->getLabel(),AtomPair);
}

inline
int oz_isPair2(OZ_Term term)
{
  if (!oz_isSRecord(term)) return 0;
  SRecord *sr = tagged2SRecord(term);
  if (!sr->isTuple()) return 0;
  if (!oz_eq(sr->getLabel(),AtomPair)) return 0;
  return sr->getWidth()==2;
}


inline
OZ_Term oz_arg(OZ_Term tuple, int i)
{
  Assert(oz_isTuple(tuple));
  return tagged2SRecord(tuple)->getArg(i);
}

inline
OZ_Term oz_left(OZ_Term pair)
{
  Assert(oz_isPair2(pair));
  return oz_arg(pair,0);
}

inline
OZ_Term oz_right(OZ_Term pair)
{
  Assert(oz_isPair2(pair));
  return oz_arg(pair,1);
}

inline 
OZ_Term oz_pair2(OZ_Term t1,OZ_Term t2) {
  SRecord *sr = SRecord::newSRecord(AtomPair,2);
  sr->setArg(0,t1);
  sr->setArg(1,t2);
  return makeTaggedSRecord(sr);
}

#define oz_pairA(s1,t)      oz_pair2(oz_atom(s1),t)
#define oz_pairAI(s1,i)     oz_pair2(oz_atom(s1),oz_int(i))
#define oz_pairAA(s1,s2)    oz_pair2(oz_atom(s1),oz_atom(s2))
#define oz_pairAS(s1,s2)    oz_pair2(oz_atom(s1),oz_string(s2))
#define oz_pairII(i1,i2)    oz_pair2(oz_int(i1),oz_int(i2))

/* -----------------------------------------------------------------------
 * lists
 * -----------------------------------------------------------------------*/

/*
 * list checking returns
 *     name false, if no list or has cycle
 *     int length, if finite list, and conditions
 *     ref var, if var found
 */

enum OzCheckList {
  OZ_CHECK_ANY,			// any list
  OZ_CHECK_CHAR,		// list of char
  OZ_CHECK_CHAR_NONZERO,	// list of char != 0
  OZ_CHECK_FEATURE		// list of features
};

inline
OZ_Term oz_checkList(OZ_Term l, OzCheckList check=OZ_CHECK_ANY)
{
  DerefIfVarReturnIt(l);
  OZ_Term old = l;
  Bool updateF = 0;
  int len = 0;
  while (oz_isCons(l)) {
    len++;
    if (check != OZ_CHECK_ANY) {
      OZ_Term h = oz_head(l);
      DerefIfVarReturnIt(h);
      if (check == OZ_CHECK_FEATURE) {
	if (!oz_isFeature(h)) return oz_false();
      } else {
	Assert(check==OZ_CHECK_CHAR || check==OZ_CHECK_CHAR_NONZERO);
	if (!oz_isSmallInt(h)) return oz_false();
	int i=smallIntValue(h);
	if (i<0 || i>255) return oz_false();
	if (check == OZ_CHECK_CHAR_NONZERO && i==0) return oz_false();
      }
    }
    l = oz_tail(l);
    DerefIfVarReturnIt(l);
    if (l==old) return oz_false(); // cyclic
    if (updateF) {
      old=oz_deref(oz_tail(old));
    }
    updateF=1-updateF;
  }
  if (oz_isNil(l)) {
    return oz_int(len);
  } else {
    return oz_false();
  }
}

/*===================================================================
 * ObjectClass
 *=================================================================== */

/* Internal representation of Oz classes */

#define CLASS_LOCKING 0x1
#define CLASS_SITED   0x2

class ObjectClass: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
private:
  SRecord *features;
  SRecord *unfreeFeatures;
  OzDictionary *fastMethods;
  OzDictionary *defaultMethods;
  int flags;
public:
  USEHEAPMEMORY;
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(ObjectClass);

  ObjectClass(SRecord *feat,OzDictionary *fm,SRecord *uf,OzDictionary *dm,
	      Bool lck, Bool sited, Board *b)
    : ConstTermWithHome(b,Co_Class)
  {
    features       = feat;
    fastMethods    = fm;
    unfreeFeatures = uf;
    defaultMethods = dm;
    flags          = 0;
    if (lck)   flags |= CLASS_LOCKING;
    if (sited) flags |= CLASS_SITED;
  }

  int supportsLocking() { return flags&CLASS_LOCKING; }
  int isSited()         { return flags&CLASS_SITED; }

  int getFlags() { return flags; }
  void setFlags(int f) { flags = f; }

  OzDictionary *getDefMethods()  { return defaultMethods; }
  OzDictionary *getfastMethods() { return fastMethods; }

  Abstraction *getMethod(TaggedRef label, SRecordArity arity, RefsArray X,
			 Bool &defaultsUsed);

  TaggedRef getFallbackNew();
  TaggedRef getFallbackApply();

  Bool lookupDefault(TaggedRef label, SRecordArity arity, RefsArray X);

  TaggedRef classGetFeature(TaggedRef lit)
  {
    return features->getFeature(lit);
  }

  SRecord *getUnfreeRecord() { return unfreeFeatures; }
  SRecord *getFeatures()     { return features; }

  const char *getPrintName();

  ObjectClass *gcClass() { return (ObjectClass *) gcConstTerm(); }

  void import(SRecord *feat,OzDictionary *fm, SRecord *uf,
	      OzDictionary *dm, int f)
  {
    features       = feat;
    fastMethods    = fm;
    unfreeFeatures = uf;
    defaultMethods = dm;
    flags          = f;
  }

  TaggedRef getArityList();
  int getWidth();

  GName *globalize();
};


/*
 * Object
 */


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


class Object: public Tertiary {
  friend void ConstTerm::gcConstRecurse(void);
private:
  ObjectClass *cl1;
  RecOrCell state;
  OzLock *lock;
  SRecord *freeFeatures;
  GName  *objectID; 
public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Object);

  Object(Board *bb,SRecord *s,ObjectClass *ac,SRecord *feat, OzLock *lck):
    Tertiary(bb, Co_Object,Te_Local)
  {
    setFreeRecord(feat);
    setClass(ac);
    setState(s);
    setGName(NULL);
    lock = lck;
  }

  Object(int i): Tertiary(i,Co_Object,Te_Proxy)
  {
    setFreeRecord(NULL);
    setClass(NULL);
    setState((Tertiary*)NULL);
    setGName(NULL);
    lock = 0;
  }

  ObjectClass *getClass()       { return cl1; }
  void setClass(ObjectClass *c) {
    Assert(!c||c->supportsLocking()>=0);
    cl1=c;
  }
  
  GName *getGName1()       { return objectID; }
  void setGName(GName *gn) { objectID = gn;}
    
  OzLock *getLock() { return lock; }
  void setLock(OzLock *l) { lock=l; }

  const char *getPrintName()    { return getClass()->getPrintName(); }
  RecOrCell getState()          { return state; }
  void setState(SRecord *s)     { Assert(s!=0); state=makeRecCell(s); }
  void setState(Tertiary *c)    { state = makeRecCell(c); }
  OzDictionary *getDefMethods() { return getClass()->getDefMethods(); }

  SRecord *getFreeRecord()          { return freeFeatures; }
  SRecord *getUnfreeRecord() { return getClass()->getUnfreeRecord(); }
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

  GName *globalize();
  void localize();
};

SRecord *getState(RecOrCell state, Bool isAssign, OZ_Term fea, OZ_Term &val);

inline
Bool isObject(ConstTerm *t)
{
  return (t->getType()==Co_Object);
}

inline
Bool oz_isObject(TaggedRef term)
{
  return oz_isConst(term) && isObject(tagged2Const(term));
}

inline
Object *tagged2Object(TaggedRef term)
{
  Assert(oz_isObject(term));
  return (Object *)tagged2Const(term);
}

inline
Bool isObjectClass(ConstTerm *t)
{
  return (t->getType()==Co_Class);
}

inline
Bool oz_isClass(TaggedRef term)
{
  return oz_isConst(term) && isObjectClass(tagged2Const(term));
}

inline
ObjectClass *tagged2ObjectClass(TaggedRef term)
{
  Assert(oz_isClass(term));
  return (ObjectClass *)tagged2Const(term);
}

/*===================================================================
 * SChunk
 *=================================================================== */

class SChunk: public ConstTermWithHome {
friend void ConstTerm::gcConstRecurse(void);
private:
  TaggedRef value;
public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(SChunk);
  SChunk(Board *b,TaggedRef v)
    : ConstTermWithHome(b,Co_Chunk), value(v)
  {
    Assert(v==0||oz_isRecord(v));
    Assert(b);
  };

  TaggedRef getValue() { return value; }
  TaggedRef getFeature(TaggedRef fea) { return OZ_subtree(value,fea); }
  TaggedRef getArityList() { return ::getArityList(value); }
  int getWidth () { return ::getWidth(value); }

  void import(TaggedRef val) {
    Assert(!value);
    Assert(oz_isRecord(val));
    Assert(getGName1());
    value=val;
  }

  GName *globalize();
};


inline
Bool isSChunk(ConstTerm *t)
{
  return t->getType() == Co_Chunk;
}

inline
Bool oz_isSChunk(TaggedRef term)
{
  return oz_isConst(term) && isSChunk(tagged2Const(term));
}

inline
SChunk *tagged2SChunk(TaggedRef term)
{
  Assert(oz_isSChunk(term));
  return (SChunk *) tagged2Const(term);
}


Bool oz_isChunkExtension(TaggedRef term);
/* optimized isChunk test */
inline
Bool oz_isChunk(TaggedRef t)
{
  return
    (oz_isConst(t) && tagged2Const(t)->getType()>=Co_ChunkStart)
    || (oz_isExtension(t) && oz_isChunkExtension(t));
}

inline
OZ_Term oz_newChunk(Board *bb, OZ_Term val)
{
  Assert(val==oz_deref(val));
  Assert(oz_isRecord(val));
  return makeTaggedConst(new SChunk(bb, val));
}

/*===================================================================
 * Arrays
 *=================================================================== */

class OzArray: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
private:
  TaggedRef *args;
  int offset, width; // mm2: put one into ConstTerm tag?

  TaggedRef *getArgs() { return args; }

public:
  NO_DEFAULT_CONSTRUCTORS(OzArray);
  OZPRINT;

  OzArray(Board *b, int low, int high, TaggedRef initvalue)
    : ConstTermWithHome(b,Co_Array) 
  {
    Assert(oz_isRef(initvalue) || !oz_isVariable(initvalue));

    offset = low;
    width = high-low+1;
    if (width <= 0) {
      width = 0;
      args = NULL; // mm2: attention if globalize gname!
    } else {
      args = (TaggedRef*) int32Malloc(sizeof(TaggedRef)*width);
      if (args==NULL) { width = -1; return; }
      for(int i=0; i<width; i++) {
	args[i] = initvalue;
      }
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
    Assert(oz_isRef(out) || !oz_isVariable(out));

    return out;
  }

  int setArg(int n,TaggedRef val) 
  { 
    Assert(oz_isRef(val) || !oz_isVariable(val));

    n -= offset;
    if (n>=getWidth() || n<0) return FALSE;

    getArgs()[n] = val;
    return TRUE;
  }
};


inline
Bool oz_isArray(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Array;
}

inline
OzArray *tagged2Array(TaggedRef term)
{
  Assert(oz_isArray(term));
  return (OzArray *) tagged2Const(term);
}


/*===================================================================
 * Abstraction (incl. PrTabEntry, AssRegArray, AssReg)
 *=================================================================== */

enum KindOfReg {
  XReg,
  YReg,
  GReg
};

class AssReg {
public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS2(AssReg);
  AssReg() {}
  
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
  NO_DEFAULT_CONSTRUCTORS(AssRegArray);
  AssRegArray (int sizeInit) : numbOfGRegs (sizeInit) 
  {
    first = (sizeInit==0 ? (AssReg*) NULL : allocAssRegBlock(sizeInit));
  }

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
  NO_DEFAULT_CONSTRUCTORS(DbgInfo);
  DbgInfo(ProgramCounter pc, TaggedRef f, int l, DbgInfo *nxt) 
    : PC(pc), file(f), line(l), next(nxt) {};
};

extern DbgInfo *allDbgInfos;

// ---------------------------------------------


#define PR_SITED   0x1

class PrTabEntry {
private:
  TaggedRef printname;
  unsigned short arity;
  SRecordArity methodArity;
  TaggedRef file;
  int line, colum;
  TaggedRef info;
  int flags;
  int gSize;
  int maxX;
public:
  PrTabEntry *next;
  unsigned int numClosures, numCalled, heapUsed, samples, lastHeap, szVars;
  static PrTabEntry *allPrTabEntries;
  static void printPrTabEntries();
  static TaggedRef getProfileStats();
  static void profileReset();

  ProgramCounter PC;

  CodeArea *codeBlock; // cache surrounding code block for code GC

public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS(PrTabEntry);
  void init(TaggedRef name, SRecordArity arityInit,
	     TaggedRef fil, int lin, int colu, TaggedRef fl, int max)
  {
    printname = name;
    maxX = max;
    file  = fil;
    line  = lin;
    colum = colu;

    codeBlock = NULL;
    flags = 0;
    fl = oz_deref(fl);
    while (oz_isCons(fl)) {
      OZ_Term ff=oz_deref(oz_head(fl));
      if (oz_eq(ff,OZ_atom("sited"))) { flags |= PR_SITED; }
      fl = oz_deref(oz_tail(fl));
    }
    Assert(oz_isNil(fl));

    Assert(oz_isLiteral(name));
    methodArity = arityInit;
    arity =  (unsigned short) getWidth(arityInit);
    Assert((int)arity == getWidth(arityInit)); /* check for overflow */
    PC = NOCODE;
    info = oz_nil();
    numClosures = numCalled = heapUsed = samples = lastHeap = szVars = 0;
    next = allPrTabEntries;     
    allPrTabEntries = this;
    DebugCheckT(gSize=-1);
  }

  PrTabEntry(TaggedRef name, SRecordArity arityInit,
	     TaggedRef pos, TaggedRef fl, int maxX)
  {
    Assert(OZ_isTuple(pos) && OZ_width(pos)==3 &&
	   OZ_isAtom(OZ_getArg(pos,0)) &&
	   OZ_isInt(OZ_getArg(pos,1)) &&
	   OZ_isInt(OZ_getArg(pos,2)));
    
    OZ_Term fil = OZ_getArg(pos,0);
    int lin     = OZ_intToC(OZ_getArg(pos,1));
    int colu    = OZ_intToC(OZ_getArg(pos,2));
    
    init(name, arityInit, fil, lin, colu, fl, maxX);
  }

  PrTabEntry(TaggedRef name, SRecordArity arityInit,
	     TaggedRef fil, int lin, int colu, TaggedRef fl, int max)
  {
    init(name, arityInit, fil, lin, colu, fl, max);
  }


  void setGSize(int n) { gSize = n; }
  int getGSize() { return gSize; }
  int getArity () { return (int) arity; }

  SRecordArity getMethodArity() { return methodArity; }
  void setMethodArity(SRecordArity sra) { methodArity = sra; }
  const char *getPrintName () { return tagged2Literal(printname)->getPrintName(); }
  TaggedRef getName () { return printname; }
  ProgramCounter getPC() { return PC; }
  void setPC(ProgramCounter pc) { PC = pc; }

  void setInfo(TaggedRef t) { info = t; }
  TaggedRef getInfo()       { return info; }

  int isSited()    { return flags&PR_SITED; }
  int getFlags()   { return flags; }
  OZ_Term getFlagsList() {
    OZ_Term ret = oz_nil();
    if (isSited()) ret = oz_cons(OZ_atom("sited"),ret);
    return ret;
  }

  int getMaxX()    { return maxX; }
  int getLine()   { return line; }
  int getColumn() { return colum; }
  TaggedRef getFile() { return file; }

  CodeArea *getCodeBlock();

  void patchFileAndLine();

  static void gcPrTabEntries();
};



class Abstraction: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
protected:
  PrTabEntry *pred;
  TaggedRef globals[1];
public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Abstraction);
  static Abstraction *Abstraction::newAbstraction(PrTabEntry *prd,
						  Board *bb)
  {
    Assert(prd->getGSize()>=0);
    int sz=sizeof(Abstraction)+sizeof(TaggedRef)*(prd->getGSize()-1);
    COUNT1(sizeClosures,sz);
    Abstraction *ab = (Abstraction *) int32Malloc(sz);
    ab->ConstTermWithHome::init(bb,Co_Abstraction);
    ab->pred=prd;
    DebugCheckT(for (int i=prd->getGSize(); i--; ) ab->globals[i]=0);
    return ab;
  }

  void initG(int i, TaggedRef val) {
    Assert(i>=0 && i<getPred()->getGSize());
    globals[i]=val;
  }
  TaggedRef getG(int i) {
    Assert(i>=0 && i<getPred()->getGSize());
    return globals[i];
  }
  TaggedRef *getGRef() { return globals; }

  PrTabEntry *getPred()  { return pred; }
  ProgramCounter getPC() { return getPred()->getPC(); }
  int getArity()         { return getPred()->getArity(); }
  SRecordArity getMethodArity()   { return getPred()->getMethodArity(); }
  const char *getPrintName()   { return getPred()->getPrintName(); }
  TaggedRef getName()    { return getPred()->getName(); }

  TaggedRef DBGgetGlobals();

  GName *globalize();
};

inline
Bool isAbstraction(ConstTerm *s)
{
  return s->getType() == Co_Abstraction;
}

inline
Bool oz_isAbstraction(TaggedRef term)
{
  return oz_isConst(term) && isAbstraction(tagged2Const(term));
}

inline
Abstraction *tagged2Abstraction(TaggedRef term)
{
  Assert(oz_isAbstraction(term));
  return (Abstraction *)tagged2Const(term);
}


/*===================================================================
 * Builtin (incl. Builtin)
 *=================================================================== */

class Builtin: public ConstTerm {
friend void ConstTerm::gcConstRecurse(void);
private:
  TaggedRef printname; //must be atom
  short inArity;
  short outArity;
  OZ_CFun fun;
  Bool sited;
#ifdef PROFILE_BI
  unsigned long counter;
#endif

public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Builtin);

  /* use malloc to allocate memory */
  static void *operator new(size_t chunk_size)
  { return ::new char[chunk_size]; }
  
  Builtin(const char *s,int inArity,int outArity, OZ_CFun fn, Bool nat)
  : inArity(inArity),outArity(outArity),fun(fn), sited(nat),
    ConstTerm(Co_Builtin)
  {
    printname = oz_atom(s);
#ifdef PROFILE_BI
    counter = 0;
#endif
  }

  OZ_CFun getFun() { return fun; }
  int getArity() { return inArity+outArity; }
  int getInArity() { return inArity; }
  int getOutArity() { return outArity; }
  const char *getPrintName() {
    return tagged2Literal(printname)->getPrintName();
  }
  TaggedRef getName() { return printname; }
  Bool isSited()      { return sited; }

#ifdef PROFILE_BI
  void incCounter() { counter++; }
  long getCounter() { return counter; }
#endif
};

inline
Bool isBuiltin(ConstTerm *s)
{
  return s->getType() == Co_Builtin;
}

inline
Bool oz_isBuiltin(TaggedRef term)
{
  return oz_isConst(term) && isBuiltin(tagged2Const(term));
}

inline
Builtin *tagged2Builtin(TaggedRef term)
{
  Assert(oz_isBuiltin(term));
  return (Builtin *)tagged2Const(term);
}

/* -----------------------------------------------------------------------
 * BuiltinTab
 * -----------------------------------------------------------------------*/

extern TaggedRef dictionary_of_builtins;

extern
Builtin * atom2Builtin(TaggedRef);

inline
Builtin * string2Builtin(const char * s) {
  return atom2Builtin(oz_atom(s));
}

Builtin * cfunc2Builtin(void * f);

/*===================================================================
 * Procedure = Abstraction or Builtin
 *=================================================================== */

inline
Bool oz_isProcedure(TaggedRef term)
{
  if (!oz_isConst(term)) {
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
int oz_procedureArity(OZ_Term pterm)
{
  ConstTerm *rec = tagged2Const(pterm);

  switch (rec->getType()) {
  case Co_Abstraction:
    return ((Abstraction *) rec)->getArity();
  case Co_Builtin:
    return ((Builtin *) rec)->getArity();
  default:
    Assert(0);
    return -1;
  }
}

/*===================================================================
 * Cell
 * Unused third field from tertiary.
 *=================================================================== */


class CellLocal:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  TaggedRef val;
  void *dummy; // mm2
public:                
  NO_DEFAULT_CONSTRUCTORS(CellLocal);
  CellLocal(Board *b,TaggedRef v) : Tertiary(b, Co_Cell,Te_Local), val(v) {}  
  TaggedRef getValue() { return val; }

  void setValue(TaggedRef v) { val=v; }

  TaggedRef exchangeValue(TaggedRef v) {
    TaggedRef ret = val;
    val = v;
    return ret;}

  void globalize(int);
};

#define Cell_Lock_Invalid     0
#define Cell_Lock_Requested   1
#define Cell_Lock_Next        2
#define Cell_Lock_Valid       4
#define Cell_Lock_Dump_Asked  8
#define Cell_Lock_Access_Bit 16

class CellSecEmul {
friend class CellManagerEmul;
friend class CellFrameEmul;
friend class CellFrame;
friend class CellManager;
protected:
  unsigned int state;
  PendThread* pending;
  DSite* next;
  TaggedRef contents;
  PendThread* pendBinding;

public:
  CellSecEmul(TaggedRef val) { Assert(0); }
  CellSecEmul() {}

  unsigned int getState(){return state;}

  TaggedRef getContents(){
    Assert(state & (Cell_Lock_Valid|Cell_Lock_Requested));
    return contents;}

  void setContents(TaggedRef t){
    Assert(state & (Cell_Lock_Valid|Cell_Lock_Requested));
    contents = t;}
};

//
// 'CellManagerEmul' (like other "*Emul" classes) provide only for
// methods that emulator must be able to use; everithing else (like
// obtaining the state from a remote site) is PERDIO business
// ('CellManager')
class CellManagerEmul : public Tertiary {
friend void ConstTerm::gcConstRecurse(void);
protected:
  CellSecEmul *sec;
  Chain *chain;
public:
  NO_DEFAULT_CONSTRUCTORS(CellManagerEmul);
  CellManagerEmul() { Assert(0); }

  CellSecEmul* getSec(){return sec;}
  unsigned int getState(){return sec->state;}
};

class CellFrameEmul : public Tertiary {
friend void ConstTerm::gcConstRecurse(void);
protected:
  CellSecEmul *sec;
  void *forward;
public:
  NO_DEFAULT_CONSTRUCTORS2(CellFrameEmul);
  CellFrameEmul() { Assert(0); }

  unsigned int getState(){return sec->state;}

  CellSecEmul* getSec(){return sec;}
};


inline
Bool oz_isCell(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Cell;
}
                  
/*===================================================================
 * Ports          
 *=================================================================== */

class Port : public Tertiary {
friend void ConstTerm::gcConstRecurse(void);
public:
  NO_DEFAULT_CONSTRUCTORS(Port);
  Port(Board *b, TertType tt) : Tertiary(b,Co_Port,tt){}
};

class PortWithStream : public Port {
friend void ConstTerm::gcConstRecurse(void);
public:
  TaggedRef strm;
  NO_DEFAULT_CONSTRUCTORS(PortWithStream);
  TaggedRef exchangeStream(TaggedRef newStream)
  { 
    TaggedRef ret = strm;
    strm = newStream;
    return ret;   }
  PortWithStream(Board *b, TaggedRef s) : Port(b,Te_Local)  {
    strm = s;}
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
  NO_DEFAULT_CONSTRUCTORS(PortLocal);
  PortLocal(Board *b, TaggedRef s) : PortWithStream(b,s) {};
};

inline
Bool oz_isPort(TaggedRef term)
{ return oz_isConst(term) && tagged2Const(term)->getType() == Co_Port;}
  
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
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Space);

  Space(Board *h, Board *s) : Tertiary(h,Co_Space,Te_Local), solve(s) {};
  Space(int i, TertType t) : Tertiary(i,Co_Space,t) {}

  SolveActor *getSolveActor();
  Board *getSolveBoard() { return solve; }
  void  merge() { solve = (Board *) 1; } 
  Bool isFailed();
  Bool isMerged();
};


inline 
Bool oz_isSpace(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Space;
}

inline
Space *tagged2Space(TaggedRef term)
{
  Assert(oz_isSpace(term));
  return (Space *) tagged2Const(term);
}

/*===================================================================
 * PendThread  (only used for locks in centralized mozart
 *=================================================================== */

enum ExKind{
  EXCHANGE    = 0,
  ASSIGN      = 1,
  AT          = 2,
  NOEX        = 3, // lock
  ACCESS      = 4,
  DEEPAT      = 5,
  REMOTEACCESS= 6,
  O_EXCHANGE  = 7,
  MOVEEX      = 8,
  DUMMY       = 9
};

//
// 
class PendThread{
public:
  Thread *thread;
  PendThread *next;
  TaggedRef controlvar;
  TaggedRef nw;
  TaggedRef old;
  ExKind    exKind;
  PendThread(Thread *th,PendThread *pt):
    next(pt), thread(th),old(0),nw(0), controlvar(0), exKind(NOEX) {}
  PendThread(Thread *th,PendThread *pt,ExKind e):
    next(pt), thread(th),old(0),nw(0), controlvar(0), exKind(e) {}
  PendThread(Thread *th,PendThread *pt,TaggedRef o, TaggedRef n, TaggedRef cv,
	     ExKind e)
    :next(pt), thread(th),old(o),nw(n), exKind(e), controlvar(cv) {}
  PendThread(Thread *th,PendThread *pt,TaggedRef cv,ExKind e)
    :next(pt), thread(th),old(0),nw(0), exKind(e), controlvar(cv) {}
  USEFREELISTMEMORY;
  void dispose(){freeListDispose(this,sizeof(PendThread));}
};

Thread* pendThreadResumeFirst(PendThread **pt);
OZ_Return pendThreadAddToEndEmul(PendThread **pt,Thread *t, Board *home);
void gcPendThreadEmul(PendThread**);

/*===================================================================
 * Locks
 *=================================================================== */

class OzLock:public Tertiary{
public:
  NO_DEFAULT_CONSTRUCTORS(OzLock);
  OzLock() { Assert(0); }
  OzLock(Board *b,TertType tt):Tertiary(b,Co_Lock,tt){}
  OzLock(int i,TertType tt):Tertiary(i,Co_Lock,tt){}
};

class LockLocal:public OzLock{
friend void ConstTerm::gcConstRecurse(void);
private:
  PendThread *pending;
  Thread *locker;
public:                
  NO_DEFAULT_CONSTRUCTORS(LockLocal);
  LockLocal(Board *b) : OzLock(b,Te_Local){
    pending=NULL;
    locker = NULL;
    pending= NULL;}

  PendThread* getPending(){return pending;}
  void setPending(PendThread *pt){pending=pt;}
  PendThread** getPendBase(){return &pending;}
  
  Thread * getLocker() { return locker; }
  void setLocker(Thread *t) { locker=t; }
  Bool hasLock(Thread *t){return (t==getLocker()) ? TRUE : FALSE;}

  void unlockComplex();
  void unlock(){
    Assert(getLocker()!=NULL);
    if(pending==NULL){
      setLocker(NULL);
      return;}
    unlockComplex();}

  Bool isLocked(Thread *t) { return (getLocker()==t); }

  void lockComplex(Thread *);
  void lock(Thread *t){
    if(t==getLocker()) {return;}
    if(getLocker()==NULL) {setLocker(t);return;}
    lockComplex(t);}

  Bool lockB(Thread *t){
    if(t==getLocker()) {return TRUE;}
    if(getLocker()==NULL) {setLocker(t);return TRUE;}
    lockComplex(t);
    return FALSE;}

  void globalize(int);

  void convertToLocal(Thread *t,PendThread *pt){
    setLocker(t);
    pending=pt;}
};

class LockSecEmul {
friend class LockFrameEmul;
friend class LockManagerEmul;
friend class LockFrame;
friend class LockManager;
friend class Chain;
protected:
  unsigned int state;
  PendThread* pending;
  DSite* next;
  Thread *locker;
  
public:
  NO_DEFAULT_CONSTRUCTORS2(LockSecEmul);
  LockSecEmul(Thread *t,PendThread *pt) { Assert(0); }
  LockSecEmul() {}		// 'LockSec()'

  Thread* getLocker(){return locker;}

  unsigned int getState() { return state; }

  Bool secLockB(Thread*t){
    if(t==locker) return OK;
    if((locker==NULL) && (state==Cell_Lock_Valid)){
      Assert(pending==NULL);
      locker=t;
      return OK;}
    return NO;}

  PendThread** getPendBase() { return &pending; }
  void unlockPending(Thread* th);
};

class LockManagerEmul : public OzLock {
protected:
  LockSecEmul *sec;
  Chain *chain;
public:
  NO_DEFAULT_CONSTRUCTORS2(LockManagerEmul);
  LockManagerEmul() { Assert(0); }

  Bool hasLock(Thread *t) { return (sec->locker==t) ? TRUE : FALSE;}

  void lock(Thread *t){
    if(sec->secLockB(t)) return;
    (*lockLockManagerOutline)(this, t);
  }

  LockSecEmul *getSec(){return sec;}

  Bool lockB(Thread *t){
    if(sec->secLockB(t)) return TRUE;
    (*lockLockManagerOutline)(this, t);
    return FALSE;}

  void unlock(Thread *t){
    if (sec->getLocker()!=t){
      sec->unlockPending(t); 
      return;}
    Assert(sec->state & Cell_Lock_Valid);
    sec->locker=NULL;
    if((sec->state==Cell_Lock_Valid) && sec->pending==NULL) return;
    (*unlockLockManagerOutline)(this, t);
  }

  PendThread* getPending() { return sec->pending; }
};

class LockFrameEmul : public OzLock {
protected:
  LockSecEmul *sec;
  void *forward;

public:
  NO_DEFAULT_CONSTRUCTORS2(LockFrameEmul);
  LockFrameEmul() { Assert(0); }

  Bool hasLock(Thread *t){ return (t==sec->getLocker()) ? TRUE : FALSE;}

  unsigned int getState(){return sec->state;}

  void lock(Thread *t){
    if(sec->secLockB(t)) return;
    (*lockLockFrameOutline)(this, t);
  }

  Bool lockB(Thread *t){
    if(sec->secLockB(t)) return TRUE;
    (*lockLockFrameOutline)(this, t);
    return FALSE;}

  void unlock(Thread *t){
    if (sec->getLocker() != t){
      sec->unlockPending(t);
      return;}
    sec->locker=NULL;
    if((sec->state==Cell_Lock_Valid) && (sec->pending==NULL)){
      return;}
    (*unlockLockFrameOutline)(this, t);
  }

  LockSecEmul* getSec() { return sec; }
};    

inline
Bool oz_isLock(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Lock;
}
/*===================================================================
 * 
 *=================================================================== */

char *toC(OZ_Term);
TaggedRef reverseC(TaggedRef l);
TaggedRef appendI(TaggedRef x,TaggedRef y);
Bool member(TaggedRef elem,TaggedRef list);


inline
Bool oz_isExtensionPlus(TaggedRef t) {
  if (oz_isExtension(t)) return TRUE;
  if (oz_isConst(t)) {
    TypeOfConst t = tagged2Const(t)->getType();
    if (t==Co_Resource || t==Co_Foreign_Pointer) return TRUE;
  }
  if (oz_isFSetValue(t)) return TRUE;
  return FALSE;
}

inline
OZ_Term oz_string(const char *s)
{
  if (!s) { return oz_nil(); }
  const char *p=s;
  while (*p!='\0') {
    p++;
  }
  OZ_Term ret = oz_nil();
  while (p!=s) {
    ret = oz_cons(oz_int((unsigned char)*(--p)), ret);
  }
  return ret;
}

/*===================================================================
 * Service Registry
 *=================================================================== */

extern OZ_Term system_registry;

extern OZ_Term registry_get(OZ_Term);
inline OZ_Term registry_get(char*s)
{
  return registry_get(oz_atom(s));
}
extern void registry_put(OZ_Term,OZ_Term);
inline void registry_put(char*s,OZ_Term v)
{
  registry_put(oz_atom(s),v);
}

#endif
