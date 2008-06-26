/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Boriss Mejias (bmc@info.ucl.ac.be)
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

// Values: literal, list, records, ...

#ifndef __VALUEHH
#define __VALUEHH

#ifdef INTERFACE
#pragma interface
#endif

#include "tagged.hh"
#include "dpInterface.hh"
#include "atoms.hh"
#include "pointer-marks.hh"

/*===================================================================
 * global names and atoms
 *=================================================================== */

extern TaggedRef
  RecordFailure,

  BI_Unify,BI_send,BI_wait,
  BI_load,BI_fail,BI_skip,BI_url_load,
  BI_get_internal, BI_get_native,
  BI_dot,
  BI_exchangeCell,BI_assign,BI_atRedo,
  BI_controlVarHandler,
  BI_PROP_LPQ,
  BI_waitStatus,
  BI_unknown,
  BI_bindReadOnly,
  BI_varToReadOnly,
  BI_raise,

  __UNUSED_DUMMY_END_MARKER;

extern Builtin *bi_raise, *bi_raiseError;

// hack to avoid including am.hh
extern Board *oz_rootBoardOutline();

/*===================================================================
 * Literal
 *=================================================================== */


#define Lit_isName          2
// named names are names;
#define Lit_isNamedName     4
#define Lit_hasGName        8
// unique names are named names;
#define Lit_isUniqueName   16
// copyable names are named names;
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
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(Literal)

  int getOthers() { return flagsAndOthers>>sizeOfLitFlags; }
  void setOthers(int value) { flagsAndOthers = getFlags()|(value<<sizeOfLitFlags); }

  Bool isName()         { return (getFlags()&Lit_isName); }
  int isNameAsInt()     { return (getFlags()&Lit_isName); }
  Bool isNamedName()    { return (getFlags()&Lit_isNamedName); }
  Bool isUniqueName()   { return (getFlags()&Lit_isUniqueName); }
  Bool isCopyableName() { return (getFlags()&Lit_isCopyableName); }
  Bool isAtom()         { return !isName(); }

  const char *getPrintName();

  Literal *gCollect(void);
  Literal *sClone(void);

  inline unsigned int hash();
  int checkSituatedness(void);
};

class Atom: public Literal {
private:
  const char *printName;
public:
  NO_DEFAULT_CONSTRUCTORS(Atom)
  static Atom *newAtom(const char *str);
  const char* getPrintName() { return printName; }
  int getSize() { return getOthers(); }
  unsigned int hash() { return (ToInt32(this) >> LTAG_BITS); }
};

/* This one goes onto the heap */
class Name: public Literal {
protected:
  static int NameCurrentNumber;
  int32 homeOrGName;
public:
  NO_DEFAULT_CONSTRUCTORS(Name)
  static Name *newName(Board *b);

  int getSeqNumber() { return getOthers(); }
  unsigned int hash() { return getSeqNumber(); }

  Bool isOnHeap() { return (getFlags()&Lit_isNamedName)==0; }
  Bool hasGName() { return (getFlags()&Lit_hasGName); }

  Board *getBoardInternal() {
    return (hasGName() || isNamedName())
      ? oz_rootBoardOutline() : (Board*)ToPointer(homeOrGName);
  }

  Bool cacIsMarked(void) {
    return oz_isMark(homeOrGName);
  }
  void cacMark(Name * fwd) {
    homeOrGName = makeTaggedMarkPtr(fwd);
  }
  int32 ** cacGetMarkField(void) {
    return (int32**) &homeOrGName;
  }
  Name * cacGetFwd(void) {
    Assert(cacIsMarked());
    return (Name *) tagged2UnmarkedPtr(homeOrGName);
  }

  Name * gCollectName(void);
  void gCollectRecurse(void);

  Name * sCloneName(void);
  void sCloneRecurse(void);

  GName *getGName1() { return (GName*) ToPointer(homeOrGName); }
  GName *globalize();
  void import(GName *);
  int checkSituatedness(void);
};

inline
Bool needsNoCollection(TaggedRef t) {
  Assert(t != makeTaggedNULL());

  if (oz_isSmallInt(t))
    return OK;

  if (!oz_isLiteral(t))
    return NO;

  if (tagged2Literal(t)->isAtom())
    return OK;

  return !((Name *) tagged2Literal(t))->isOnHeap();
}


/* This one managed via malloc */

class NamedName: public Name {
public:
  NO_DEFAULT_CONSTRUCTORS(NamedName)
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

#define oz_true() NameTrue
#define oz_false() NameFalse
#define oz_unit() NameUnit

#define oz_bool(i) ((i) ? NameTrue : NameFalse)

#define oz_isTrue(t)  oz_eq((t),NameTrue)
#define oz_isFalse(t) oz_eq((t),NameFalse)
#define oz_isUnit(t)  oz_eq((t),NameUnit)

inline
Bool oz_isBool(TaggedRef term) {
  return oz_isTrue(term) || oz_isFalse(term);
}

/*
 * atomcmp(a,b) is used to construct the arity lists of records.
 *
 * It returns: 0        if a == b
 *             negative if a < b
 *             positive if a > b
 *
 * So that
 *
 *   NAMES < ATOMS
 *
 * and both names and atoms are compared lexigraphically, and names
 * with equal print names are compared according to their "sequential
 * numbers".
 */

inline
int atomcmp(Literal *a, Literal *b)
{
  int res;

  if (a == b) return 0;
  res = b->isNameAsInt() - a->isNameAsInt();
  if (res != 0) return (res);
  res = strcmp(a->getPrintName(), b->getPrintName());
  if (res != 0) return (res);
  Assert(a->isName() && b->isName());
  res = ((Name *) a)->getSeqNumber() - ((Name *) b)->getSeqNumber();
  return (res);
}

// see codearea.cc
extern TaggedRef OZ_atom(OZ_CONST char *s);
inline
TaggedRef oz_atom(OZ_CONST char *s) {
  return OZ_atom(s);
}
extern TaggedRef oz_atomNoDup(const char *s);
extern TaggedRef oz_uniqueName(const char *s);


/*===================================================================
 * Numbers
 *=================================================================== */

// #include <math.h>
#include <limits.h>

#ifndef GMP_NEEDS_CPLUSPLUS
extern "C" {
#endif
#include <gmp.h>
#ifndef GMP_NEEDS_CPLUSPLUS
}
#endif


/*===================================================================
 * SmallInt
 *=================================================================== */

#ifdef DEBUG_CHECK

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
  return (((unsigned int) n) >> LTAG_BITS);
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
  Assert(oz_isSmallInt(a) || oz_isSmallInt(b));
  return ((int) a - (int) b);
}

#else

#define smallIntLess(A,B) (((int) (A)) < ((int) (B)))
#define smallIntLE(A,B)   (((int) (A)) <= ((int) (B)))
#define smallIntEq(A,B)   ((A) == (B))
#define smallIntHash(A)   (((unsigned int) (A)) >> LTAG_BITS)
#define smallIntCmp(A,B)  (((int) (A)) - ((int) (B)))

#endif


/*===================================================================
 * LTuple
 *=================================================================== */

class LTuple {
private:
  TaggedRef args[2];

public:
  USEHEAPMEMORY;
  OZPRINTLONG

  NO_DEFAULT_CONSTRUCTORS2(LTuple)
  LTuple(void) {
    DebugCheckT(args[0]=0;args[1]=0);
  }
  LTuple(TaggedRef head, TaggedRef tail) {
    args[0] = head; args[1] = tail;
  }

  int cacIsMarked(void) {
    return oz_isMark(args[0]);
  }
  void cacMark(LTuple * fwd) {
    args[0] = makeTaggedMarkPtr(fwd);
  }
  LTuple * cacGetFwd(void) {
    return (LTuple *) tagged2UnmarkedPtr(args[0]);
  }
  int32 ** cacGetMarkField(void) {
    return (int32**) &args[0];
  }
  void gCollectRecurse(void);
  LTuple * gCollect(void);

  void sCloneRecurse(void);
  LTuple * sClone(void);

  // kost@ : don't do 'tagged2NonVariableFast' here: things become slow!
  TaggedRef getHead()          { return tagged2NonVariable(args); }
  TaggedRef getTail()          { return tagged2NonVariable(args+1); }
  void setHead(TaggedRef term) { args[0] = term;}
  void setTail(TaggedRef term) { args[1] = term;}
  void setBoth(TaggedRef h, TaggedRef t) {
    args[0] = h; args[1] = t;
  }
  TaggedRef *getRef()          { return args; }
  TaggedRef *getRefTail()      { return args+1; }
  TaggedRef *getRefHead()      { return args; }
};


#define oz_nil() AtomNil

#define oz_isCons(term) oz_isLTuple(term)

#define oz_isNil(term) oz_eq(term,oz_nil())

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
TaggedRef oz_mklist(TaggedRef l1) {
  return oz_cons(l1,AtomNil);
}

inline
TaggedRef oz_mklist(TaggedRef l1,TaggedRef l2) {
  LTuple * l = (LTuple *) oz_heapMalloc(2 * sizeof(LTuple));
  l[0].setBoth(l1,makeTaggedLTuple(l+1));
  l[1].setBoth(l2,AtomNil);
  return makeTaggedLTuple(l);
}

inline
TaggedRef oz_mklist(TaggedRef l1,TaggedRef l2,TaggedRef l3) {
  LTuple * l = (LTuple *) oz_heapMalloc(3 * sizeof(LTuple));
  l[0].setBoth(l1,makeTaggedLTuple(l+1));
  l[1].setBoth(l2,makeTaggedLTuple(l+2));
  l[2].setBoth(l3,AtomNil);
  return makeTaggedLTuple(l);
}

inline
TaggedRef oz_mklist(TaggedRef l1,TaggedRef l2,TaggedRef l3,TaggedRef l4) {
  LTuple * l = (LTuple *) oz_heapMalloc(4 * sizeof(LTuple));
  l[0].setBoth(l1,makeTaggedLTuple(l+1));
  l[1].setBoth(l2,makeTaggedLTuple(l+2));
  l[2].setBoth(l3,makeTaggedLTuple(l+3));
  l[3].setBoth(l4,AtomNil);
  return makeTaggedLTuple(l);
}

inline
TaggedRef oz_mklist(TaggedRef l1,TaggedRef l2,TaggedRef l3,TaggedRef l4,TaggedRef l5) {
  LTuple * l = (LTuple *) oz_heapMalloc(5 * sizeof(LTuple));
  l[0].setBoth(l1,makeTaggedLTuple(l+1));
  l[1].setBoth(l2,makeTaggedLTuple(l+2));
  l[2].setBoth(l3,makeTaggedLTuple(l+3));
  l[3].setBoth(l4,makeTaggedLTuple(l+4));
  l[4].setBoth(l5,AtomNil);
  return makeTaggedLTuple(l);
}

OZ_Term oz_list(OZ_Term t, ...);

#ifdef DEBUG_CHECK
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

#else

#define oz_head(list) (tagged2LTuple(list)->getHead())
#define oz_tail(list) (tagged2LTuple(list)->getTail())

#endif

int oz_fastlength(OZ_Term l);



/*===================================================================
 * ConstTerm
 *=================================================================== */

/* when adding a new type update
 *   print (print.cc and foreign.cc)
 *   OZ_toVirtualString
 *   marshaling/unmarshaling
 */
const int Co_Bits = 16;
const int Co_Mask = (1<<Co_Bits)-1;

enum TypeOfConst {
  Co_Extension = OZ_CONTAINER_TAG,
  Co_Float,
  Co_BigInt,
  Co_Foreign_Pointer,
  Co_Abstraction,
  Co_Builtin,
  Co_Cell,
  Co_Space,
  Co_Resource,
  Co_FSetValue,

  //
  // chunks must stay together and the first one must be Co_Object
  // otherwise you'll have to change the "isChunk" test;
  //
  // NOTE: update the builtins: subtree and chunkArity, when adding
  //       new chunks
  Co_Object,
  Co_ObjectState,
  Co_Port,
  Co_Chunk,
  Co_Array,
  Co_Dictionary,
  Co_Lock,
  Co_Class
};

#define Co_ChunkStart Co_Object

#define DebugIndexCheck(IND) {Assert(IND< (1<<27));Assert(IND>=0);}

class ConstTerm : public OZ_Container {
public:
  USEHEAPMEMORY;
  OZPRINTLONG
  ConstTerm() { Assert(0); }
  //  void init(TypeOfConst t) { tag = t<<1; }
  ConstTerm(TypeOfConst t) {
    init(t);
  }

  ConstTerm *gCollectConstTermInline(void);
  ConstTerm *gCollectConstTerm(void);
  ConstTerm *sCloneConstTermInline(void);
  void gCollectConstRecurse(void);
  void sCloneConstRecurse(void);

  TypeOfConst getType() { return (TypeOfConst) ((tag & Co_Mask)>>1); }

  void setVal(int val) {
    tag = (tag & Co_Mask) | (val << Co_Bits);
  }
  int getVal() {
    return tag >> Co_Bits;
  }
  const char *getPrintName();
  int getArity();
  int checkSituatedness(void);
};


#define CWH_BOARD    0
#define CWH_GNAME    1
#define CWH_MEDIATOR 2
class Mediator;

class ConstTermWithHome: public ConstTerm {
private:
  Tagged2 boardOrGName;

public:
  void setBoard(Board *b)
  {
    boardOrGName.set(b,CWH_BOARD);
  }
  void setMediator(Mediator *med)
  {
    boardOrGName.set(med, CWH_MEDIATOR);
  }
  Mediator *getMediator()
  {
    return (Mediator*) boardOrGName.getPtr();
  }
  ConstTermWithHome() { Assert(0); }
  ConstTermWithHome(Board *bb, TypeOfConst tt) : ConstTerm(tt) { setBoard(bb);}

  Bool hasGName() { return (boardOrGName.getTag()&CWH_GNAME); }
  Bool isDistributed(){ return (boardOrGName.getTag()&CWH_MEDIATOR); }
  void init(Board *bb, TypeOfConst tt) { ConstTerm::init(tt); setBoard(bb); }

  Board *getBoardInternal() {
    return isDistributed() ? oz_rootBoardOutline() : (Board*)boardOrGName.getPtr();
  }

  Board *getSubBoardInternal() {
    Assert(!isDistributed());
    return (Board*)boardOrGName.getPtr();
  }

  void setGName(GName *gn) {
    Assert(gn);
    boardOrGName.set(gn,CWH_GNAME);
  }
  GName *getGName1() {
    return hasGName()?(GName *)boardOrGName.getPtr():(GName *)NULL;
  }
  GName *getGName() {
    Assert(hasGName());
    return ((GName *) boardOrGName.getPtr());
  }
};

/*
 * Extensions
 *
 */

// raph: Extensions are "wrapped" in ConstTerm's.  In memory, an
// OZ_Extension object always sits next to a ConstTermWithHome object.
// The latter is of type Co_Extension, an provides a board, mediator,
// or gname to the extension.

inline
Bool oz_isExtension(TaggedRef t) {
  return oz_isConst(t) && tagged2Const(t)->getType() == Co_Extension;
}

inline
OZ_Extension* const2Extension(ConstTerm* p) {
  ConstTermWithHome* c = static_cast<ConstTermWithHome*>(p);
  return reinterpret_cast<OZ_Extension*>(c + 1);
}

inline
ConstTermWithHome* extension2Const(OZ_Extension* p) {
  return reinterpret_cast<ConstTermWithHome*>(p) - 1;
}

inline
OZ_Extension * tagged2Extension(TaggedRef t) {
  Assert(oz_isExtension(t));
  return const2Extension(tagged2Const(t));
}

inline
TaggedRef makeTaggedExtension(OZ_Extension * s) {
  return makeTaggedConst(extension2Const(s));
}

/*===================================================================
 * Float
 *=================================================================== */

class Float : public ConstTerm {
protected:
  double value;

public:
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS2(Float)
  Float(double f) : ConstTerm(Co_Float) {
    value = f;
  }
  double getValue(void) {
    return value;
  }
  unsigned int hash(void) {
    return (unsigned int) value;
  }
};

inline
Bool oz_isFloat(TaggedRef t) {
  return oz_isConst(t) && tagged2Const(t)->getType() == Co_Float;
}

inline
Float * tagged2Float(TaggedRef t) {
  return (Float *) tagged2Const(t);
}

inline
TaggedRef makeTaggedFloat(Float * s) {
  return makeTaggedConst(s);
}

inline
double floatValue(TaggedRef t) {
  return tagged2Float(t)->getValue();
}

inline
Bool floatEq(TaggedRef a, TaggedRef b) {
  return (tagged2Float(a)->getValue() == tagged2Float(b)->getValue());
}

inline
TaggedRef oz_float(double f) {
  return makeTaggedFloat(new Float(f));
}



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
  OZPRINT

  NO_DEFAULT_CONSTRUCTORS2(BigInt)

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
    oz_freeListDispose(this,sizeof(BigInt));
  }

  // Make a small int if <Big> fits into it, else return big int.
  // That is, no BigInt should be representable as a SmallInt!
  // Or, differently put, BigInt != SmallInt for any BigInt, SmallInt;
  TaggedRef shrink(void) {
    TaggedRef ret;
    // kost@ : having 'omi' is the GMP kludge: 'mpz_cmp_si' is a macro
    // that references 'mpz_cmp_ui' in the right case, which causes a
    // compiler warning for OzMinInt when the last one is defined as
    // -(OzMaxInt+1);
    const long int omi = OzMinInt;
    if (mpz_cmp_si(&value, OzMaxInt) > 0 ||
        mpz_cmp_si(&value, omi) < 0)
      ret = makeTaggedConst(this);
    else {
      ret =  makeTaggedSmallInt((int) mpz_get_si(&value));
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

  long getLong(void) {
    return mpz_get_si(&value);
  }

  /* make an 'unsigned long' if <Big> fits into it, else return 0,~0 */
  unsigned long getUnsignedLong(void) {

    if (mpz_cmp_ui(&value,ULONG_MAX) > 0) {
      return ULONG_MAX;
    } else if (mpz_cmp_si(&value,0) < 0) {
      return 0;
    } else {
      return mpz_get_ui(&value);
    }
  }


  Bool equal(BigInt *b) {
    return (mpz_cmp(&value, &b->value) == 0);
  }

#define MKOP(op,mpop)                                                         \
  TaggedRef op(BigInt *b) {                                                   \
    BigInt *n = newBigInt();                                                  \
    mpop(&n->value,&value,&b->value);                                         \
    return n->shrink();                                                       \
  }
  MKOP(div,mpz_tdiv_q);
  MKOP(mod,mpz_tdiv_r);
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

  unsigned int hash()     { return 75; } // all BigInt hash to same value
  BigInt * gCollect(void);
};


inline
Bool oz_isBigInt(TaggedRef term) {
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_BigInt;
}

inline
Bool oz_isInt(TaggedRef term) {
  return oz_isSmallInt(term) || oz_isBigInt(term);
}

inline
Bool oz_isNumber(TaggedRef term) {
  return oz_isInt(term) || oz_isFloat(term);
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
    return makeTaggedSmallInt(i);
}

inline
TaggedRef oz_ulong(unsigned long l) {
  if (l > (unsigned long) OzMaxInt)
    return makeTaggedConst(newBigInt(l));
  else
    return makeTaggedSmallInt((int)l);
}

inline
int oz_intToC(TaggedRef term)
{
  if (oz_isSmallInt(term)) {
    return tagged2SmallInt(term);
  }

  return tagged2BigInt(term)->getInt();
}

inline
long oz_intToCL(TaggedRef term)
{
  if (oz_isSmallInt(term)) {
    return (long) tagged2SmallInt(term);
  }

  return tagged2BigInt(term)->getLong();
}

OZ_Term oz_long(long i);
OZ_Term oz_unsignedLong(unsigned long i);

inline
TaggedRef oz_unsignedInt(unsigned int i)
{
  if (i > (unsigned int) OzMaxInt)
    return makeTaggedConst(newBigInt(i));
  else
    return makeTaggedSmallInt(i);
}

// Both 'a' and 'b' must be BigInt"s;
Bool bigIntEq(TaggedRef a, TaggedRef b);

inline
Bool oz_numberEq(TaggedRef a, TaggedRef b) {
  if (oz_isSmallInt(a) && oz_isSmallInt(b))
    return smallIntEq(a,b);
  if (oz_isFloat(a) && oz_isFloat(b))
    return (((Float *) tagged2Const(a))->getValue() ==
            ((Float *) tagged2Const(b))->getValue());
  if (oz_isBigInt(a) && oz_isBigInt(b))
    return ((BigInt *) tagged2Const(a))->equal((BigInt *) tagged2Const(b));
  return NO;
}

extern TaggedRef TaggedOzOverMaxInt;
extern TaggedRef TaggedOzOverMinInt;

/*
 * FSetValue: FIX TOBIAS
 *
 */

class ConstFSetValue : public ConstTerm {
private:
  OZ_FSetValue * _fsv;

public:
  USEFREELISTMEMORY;
  NO_DEFAULT_CONSTRUCTORS2(ConstFSetValue)

  ConstFSetValue(OZ_FSetValue * fsv) : ConstTerm(Co_FSetValue) {
    _fsv = fsv;
  }
  void dispose(void) {
    oz_freeListDispose(this,sizeof(Co_FSetValue));
  }
  OZ_FSetValue * getValue(void) {
    return _fsv;
  }
  ConstFSetValue * gCollect(void);
};

inline
int oz_isFSetValue(TaggedRef t) {
  return oz_isConst(t) && tagged2Const(t)->getType() == Co_FSetValue;
}

inline
OZ_FSetValue * tagged2FSetValue(TaggedRef t) {
  Assert(oz_isFSetValue(t));
  return ((ConstFSetValue *) tagged2Const(t))->getValue();
}

inline
TaggedRef makeTaggedFSetValue(OZ_FSetValue * fsv) {
  // FIX TOBIAS: THIS ALLOCATES MEMORY ON THE HEAP!
  return makeTaggedConst(new ConstFSetValue(fsv));
}


/*===================================================================
 * ForeignPointer
 *=================================================================== */

class ForeignPointer: public ConstTerm {
private:
  void* ptr;
public:
  OZPRINT
  NO_DEFAULT_CONSTRUCTORS2(ForeignPointer)

  ForeignPointer():ConstTerm(Co_Foreign_Pointer),ptr(0){}
  ForeignPointer(void*p):ConstTerm(Co_Foreign_Pointer),ptr(p){}
  void*getPointer(){ return ptr; }
  ForeignPointer * cac(void);
};

inline
Bool oz_isForeignPointer(TaggedRef term) {
  return oz_isConst(term)
    && tagged2Const(term)->getType() == Co_Foreign_Pointer;
}

inline
void * oz_getForeignPointer(TaggedRef t) {
  return ((ForeignPointer*) tagged2Const(t))->getPointer();
}


/*===================================================================
 * SRecord: incl. Arity, ArityTable
 *=================================================================== */

inline
Bool oz_isFeature(TaggedRef f) {
  return oz_isToken(f) || oz_isBigInt(f);
;
}

inline
int featureEq(TaggedRef a,TaggedRef b) {
  Assert(oz_isFeature(a) && oz_isFeature(b));
  return (a == b ||
          (oz_isBigInt(a) && oz_isBigInt(b) && bigIntEq(a, b)));
}

/*
 * For sorting the arity one needs to have a total order
 *
 *   (SMALLINT, BIGINT) < LITERAL
 *
 * returns 0:        if equal
 *         negative: if a<b
 *         positive: if a>b
 *
 * NOTE: it should not be e.g.
 *
 *   SMALLINT < BIGINT < LITERAL
 *
 * because SmallInt"s can be different in size on different platforms;
 *
 */
inline
int featureCmp(TaggedRef a, TaggedRef b) {
  Assert(oz_isFeature(a) && oz_isFeature(b));

  switch (tagged2ltag(a)) {
  case LTAG_LITERAL:
    if (oz_isLiteral(b)) {
      return (atomcmp(tagged2Literal(a), tagged2Literal(b)));
    } else {
      return (1);
    }

  case LTAG_SMALLINT:
    switch (tagged2ltag(b)) {
    case LTAG_LITERAL:
      return (-1);
    case LTAG_SMALLINT:
      return (smallIntCmp(a, b));
    default:
      Assert(oz_isBigInt(b));
      return (-tagged2BigInt(b)->cmp(tagged2SmallInt(a)));
    }

  default:
    Assert(oz_isBigInt(a));
    switch (tagged2ltag(b)) {
    case LTAG_LITERAL:
      return (-1);
    case LTAG_SMALLINT:
      return (tagged2BigInt(a)->cmp(tagged2SmallInt(b)));
    default:
      Assert(oz_isBigInt(b));
      return (tagged2BigInt(a)->cmp(tagged2BigInt(b)));
    }
  }

  Assert(0);
}


/*
 * Hash function for Features:
 * NOTE: all bigints are hashed to the same value (mm2)
 */
inline
unsigned int featureHash(TaggedRef a) {
  Assert(oz_isFeature(a));
  switch (tagged2ltag(a)) {
  case LTAG_LITERAL:
    return tagged2Literal(a)->hash();
  case LTAG_SMALLINT:
    return smallIntHash(a);
  default:
    Assert(oz_isBigInt(a));
    return tagged2BigInt(a)->hash();
  }
}

class KeyAndIndex {
public:
  TaggedRef key;
  int index;
  NO_DEFAULT_CONSTRUCTORS(KeyAndIndex)
};

class Arity {
friend class ArityTable;
private:
  static Arity *newArity(TaggedRef, Bool);

  void gCollect(void);

  TaggedRef list;
  Arity *next;

  int hashmask;         // size-1, used as mask for hashing and opt marker
  int width;            // next unused index in RefsArray (for add())
  DebugCheckT(int numberOfCollisions;)

  KeyAndIndex table[1];

  int scndhash(int i) {
    return ((i&7)<<1)|1;
  }
  int hashfold(int i) {
    return i&hashmask;
  }

public:
  OZPRINT
  NO_DEFAULT_CONSTRUCTORS(Arity)

  Bool isTuple() { return hashmask == 0; }

  int getCollisions() {
    DebugCode(return numberOfCollisions);
    return -1;
  }

  int lookupLiteralInternal(TaggedRef entry) {
    Assert(!isTuple());
    const int hsh  = tagged2Literal(entry)->hash();
    const int step = scndhash(hsh);
    int i = hashfold(hsh);
    while (1) {
      const TaggedRef key = table[i].key;
      if (oz_eq(key,entry))
        return table[i].index;
      if (!key)
        return -1;
      i = hashfold(i+step);
    }
  }

  int lookupSmallIntInternal(TaggedRef entry) {
    Assert(!isTuple());
    const int hsh  = smallIntHash(entry);
    const int step = scndhash(hsh);
    int i = hashfold(hsh);
    while (1) {
      const TaggedRef key = table[i].key;
      if (oz_eq(key,entry))
        return table[i].index;
      if (!key)
        return -1;
      i = hashfold(i+step);
    }
  }

  int lookupBigIntInternal(TaggedRef entry);
  int lookupInternal(TaggedRef entry);

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
  void gCollect(void);
  OZPRINT
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

typedef int32 SRecordArity; /* do not want to use a pointer on the Alpha! */

inline
int sraIsTuple(SRecordArity a) {
  return a&1;
}

inline
int sraCacIsMarked(SRecordArity a) {
  return a&2;
}

inline
SRecordArity mkTupleWidth(int w) {
  return (SRecordArity) ((w<<2)|1);
}

inline
int getTupleWidth(SRecordArity a) {
  return a>>2;
}

inline
SRecordArity mkRecordArity(Arity *a) {
  Assert(!a->isTuple());
  return ToInt32(a);
}

inline
Arity *getRecordArity(SRecordArity a) {
  return (Arity*) ToPointer(a);
}

inline
int sameSRecordArity(SRecordArity a, SRecordArity b) {
  return a==b;
}

inline
int getWidth(SRecordArity a) {
  return sraIsTuple(a) ? getTupleWidth(a) : getRecordArity(a)->getWidth();
}


inline
OZ_Term sraGetArityList(SRecordArity arity)
{
  return (sraIsTuple(arity))
    ? makeTupleArityList(getTupleWidth(arity))
    : getRecordArity(arity)->getList();
}

class SRecord {
private:
  // The order matters, never change it: garbage collection!
  SRecordArity recordArity;
  TaggedRef label, args[1];   // really maybe more

public:
  USEHEAPMEMORY;
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(SRecord)

  int cacIsMarked(void) {
    return sraCacIsMarked(recordArity);
  }
  SRecord * cacGetFwd(void) {
    Assert(cacIsMarked());
    return (SRecord *) ToPointer(ToInt32(recordArity) & ~2);
  }
  int32 ** cacGetMarkField(void) {
    return (int32 **) &recordArity;
  }
  void cacMark(SRecord * fwd) {
    recordArity = (SRecordArity) (ToInt32(fwd) | 2);
  }

  SRecord *gCollectSRecord(void);
  SRecord *sCloneSRecord(void);

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

  TaggedRef normalize(void);

  void initArgs();

  int getWidth() { return ::getWidth(getSRecordArity()); }

  void downSize(unsigned int s) {
    Assert(isTuple());
    setTupleWidth(s);
  }

  static SRecord *newSRecord(TaggedRef lab, SRecordArity arity, int width)
  {
    Assert(oz_isLiteral(lab));
    Assert(width > 0);
    int memSize = sizeof(SRecord) + sizeof(TaggedRef) * (width - 1);
    SRecord *ret = (SRecord *) oz_heapMalloc(memSize);
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
  void setArg(int i, TaggedRef t) {   Assert(i >= 0); args[i] = t; }
  TaggedRef *getRef() { return args; }
  TaggedRef *getRef(int i) { return args+i; }
  TaggedRef &operator [] (int i) { return args[i]; }

  void setFeatures(TaggedRef proplist);

  TaggedRef getLabel() { return label; }
  void setLabelInternal(TaggedRef l) {
    Assert(oz_isLiteral(l));
    label=l;
  }
  Literal *getLabelLiteral() { return tagged2Literal(label); }
  void setLabelForAdjoinOpt(TaggedRef newLabel) {
    Assert(oz_isLiteral(newLabel));
    label = newLabel;
  }

  TaggedRef getArityList() {
    return sraGetArityList(getSRecordArity());
  }

  Arity* getArity () {
    return isTuple() ? aritytable.find(getArityList()) : getRecordArity();
  }

  int getLiteralIndex(TaggedRef f) {
    Assert(oz_isLiteral(f));
    if (isTuple()) {
      return -1;
    } else {
      return getRecordArity()->lookupLiteralInternal(f);
    }
  }
  int getSmallIntIndex(TaggedRef f) {
    Assert(oz_isSmallInt(f));
    if (isTuple()) {
      int i = tagged2SmallInt(f)-1;
      if ((0<=i) && (i<getWidth()))
        return i;
      else
        return -1;
    } else {
      return getRecordArity()->lookupSmallIntInternal(f);
    }
  }
  int getBigIntIndex(TaggedRef f) {
    Assert(oz_isBigInt(f));
    if (isTuple()) {
      return -1; // FIXME
    } else {
      return getRecordArity()->lookupBigIntInternal(f);
    }
  }
  int getIndex(TaggedRef f) {
    if (oz_isSmallInt(f)) {
      return getSmallIntIndex(f);
    } else if (oz_isLiteral(f)) {
      return getLiteralIndex(f);
    }
    Assert(oz_isBigInt(f));
    return getBigIntIndex(f);
  }


  TaggedRef getSmallIntFeatureInline(TaggedRef f) {
    int i = getSmallIntIndex(f);
    return i < 0 ? makeTaggedNULL() : getArg(i);
  }
  TaggedRef getLiteralFeatureInline(TaggedRef f) {
    int i = getLiteralIndex(f);
    return i < 0 ? makeTaggedNULL() : getArg(i);
  }
  TaggedRef getBigIntFeatureInline(TaggedRef f) {
    int i = getBigIntIndex(f);
    return i < 0 ? makeTaggedNULL() : getArg(i);
  }
  TaggedRef getFeature(TaggedRef);

  Bool setFeature(TaggedRef feature,TaggedRef value);
  TaggedRef replaceFeature(TaggedRef feature,TaggedRef value);

  void gCollectRecurse(void);
  void sCloneRecurse(void);

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
TaggedRef packlist(TaggedRef list);
TaggedRef packsortlist(TaggedRef list);

inline
Bool oz_isRecord(TaggedRef t) {
  return oz_isSRecord(t) || oz_isLTuple(t) || oz_isLiteral(t);
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
  return (0);                   // ???
}

inline
Arity *oz_makeArity(OZ_Term list)
{
  list = packsortlist(list);
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
#define oz_pairAS(s1,s2)    oz_pair2(oz_atom(s1),OZ_string(s2))
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
  OZ_CHECK_ANY,                 // any list
  OZ_CHECK_CHAR,                // list of char
  OZ_CHECK_CHAR_NONZERO,        // list of char != 0
  OZ_CHECK_FEATURE              // list of features
};

inline
OZ_Term oz_checkList(OZ_Term l, OzCheckList check=OZ_CHECK_ANY) {
  l = oz_safeDeref(l);
  if (oz_isRef(l))
    return l;
  OZ_Term old = l;
  Bool updateF = 0;
  int len = 0;

  Assert(!oz_isRef(l));
  while (oz_isLTupleOrRef(l)) {
    len++;
    if (check != OZ_CHECK_ANY) {
      OZ_Term h = oz_safeDeref(oz_head(l));
      if (oz_isRef(h))
        return h;
      if (check == OZ_CHECK_FEATURE) {
        if (!oz_isFeature(h)) return oz_false();
      } else {
        Assert(check==OZ_CHECK_CHAR || check==OZ_CHECK_CHAR_NONZERO);
        if (!oz_isSmallInt(h)) return oz_false();
        int i=tagged2SmallInt(h);
        if (i<0 || i>255) return oz_false();
        if (check == OZ_CHECK_CHAR_NONZERO && i==0) return oz_false();
      }
    }
    l = oz_safeDeref(oz_tail(l));
    if (oz_isRef(l))
      return l;
    if (l==old) return oz_false(); // cyclic
    if (updateF) {
      old=oz_deref(oz_tail(old));
    }
    updateF=1-updateF;
    Assert(!oz_isRef(l));
  }
  if (oz_isNil(l)) {
    return oz_int(len);
  } else {
    return oz_false();
  }
}

/*===================================================================
 * OzClass
 *=================================================================== */

/* Internal representation of Oz classes */

#define CLASS_LOCKING 0x1
#define CLASS_SITED   0x2
#define CLASS_FLAGS_MAX 0x3

class OzClass: public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  // Never change order, the garbage collector relies on it!
  TaggedRef features, unfreeFeatures, fastMethods, defaultMethods;
  int flags;

  Bool lookupDefault(TaggedRef label, SRecordArity arity, Bool reorder);

public:
  USEHEAPMEMORY;
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(OzClass)

  OzClass(TaggedRef feat, TaggedRef fm, TaggedRef uf, TaggedRef dm,
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

  int supportsLocking(void) {
    return flags&CLASS_LOCKING;
  }
  int isSited(void)         {
    return flags&CLASS_SITED;
  }

  int getFlags(void) {
    return flags;
  }
  void setFlags(int f) {
    flags = f;
  }

  OzDictionary * getDefMethods(void)  {
    return (OzDictionary *) tagged2Const(defaultMethods);
  }
  OzDictionary * getfastMethods(void) {
    return (OzDictionary *) tagged2Const(fastMethods);
  }
  SRecord * getUnfreeRecord(void) {
    return unfreeFeatures ? tagged2SRecord(unfreeFeatures) : (SRecord *) NULL;
  }
  SRecord * getFeatures(void) {
    return tagged2SRecord(features);
  }

  Abstraction *getMethod(TaggedRef label, SRecordArity arity,
                         Bool reorder,
                         Bool &defaultsUsed);

  TaggedRef getFallbackNew(void);
  TaggedRef getFallbackApply(void);

  TaggedRef classGetFeature(TaggedRef lit) {
    return getFeatures()->getFeature(lit);
  }


  const char * getPrintName(void);

  void import(TaggedRef feat, TaggedRef fm, TaggedRef uf,
              TaggedRef dm, int f) {
    features       = feat;
    fastMethods    = fm;
    unfreeFeatures = uf;
    defaultMethods = dm;
    flags          = f;
  }

  TaggedRef getArityList(void) {
    return getFeatures()->getArityList();
  }
  int getWidth(void) {
    return getFeatures()->getWidth();
  }

  // distribution support: the state is present iff feat!=0
  Bool isComplete() { return features != makeTaggedNULL(); }

  GName *globalize(void);
};

inline
Bool isOzClass(ConstTerm *t)
{
  return (t->getType()==Co_Class);
}

inline
Bool oz_isClass(TaggedRef term)
{
  return oz_isConst(term) && isOzClass(tagged2Const(term));
}

inline
OzClass *tagged2OzClass(TaggedRef term)
{
  Assert(oz_isClass(term));
  return (OzClass *)tagged2Const(term);
}


/*
 * Object
 */

// raph: Objects are distributed with two mediators: one for the
// object itself, and one for its state.  The object is considered an
// immutable, and its state only is mutable.  The state of an object
// looks like a chunk with mutable fields.

class ObjectState : public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  TaggedRef value;     // must be a record (possibly with no field)
public:
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(ObjectState)
  ObjectState(Board *b, TaggedRef v)
    : ConstTermWithHome(b, Co_ObjectState), value(v)
  {
    Assert(v==0||oz_isRecord(v));
    Assert(b);
  };

  TaggedRef getValueTerm() { return value; }
  SRecord* getValue() { return value ? tagged2SRecord(value) : NULL; }
  void setValue(TaggedRef v) {
    Assert(v == makeTaggedNULL() || oz_isRecord(v));
    value = v;
  }

  TaggedRef getFeature(TaggedRef fea) { return OZ_subtree(value,fea); }
  Bool setFeature(TaggedRef fea, TaggedRef val) {
    return tagged2SRecord(value)->setFeature(fea, val);
  }
  TaggedRef getArityList() { return value ? ::getArityList(value) : oz_nil(); }
  int getWidth () { return ::getWidth(value); }
};

inline
Bool oz_isObjectState(TaggedRef t) {
  return oz_isConst(t) && tagged2Const(t)->getType() == Co_ObjectState;
}

inline
ObjectState* tagged2ObjectState(TaggedRef t) {
  Assert(oz_isObjectState(t));
  return (ObjectState*) tagged2Const(t);
}

// object state operations, with two arrays for inputs and outputs
OZ_Return ostateOperation(OperationTag, ObjectState*, TaggedRef*, TaggedRef*);

// Objects no longer use a GName.  The object is serialized by its
// mediator.

class OzObject: public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  // do not change order, the garbage collector relies on it!
  TaggedRef cls, lock, freeFeatures, state;

public:
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(OzObject)

  // handle class, lock, free features, and state
  OzClass* getClass(void) {
    return cls == makeTaggedNULL() ? NULL : tagged2OzClass(cls);
  }
  void setClass(OzClass *c) {
    cls = c ? makeTaggedConst(c) : makeTaggedNULL();
  }
  void setClass(OZ_Term cl) {
    Assert(oz_isClass(cl));
    cls = cl;
  }

  OzLock *getLock(void) {
    return lock == makeTaggedNULL() ? NULL : (OzLock *) tagged2Const(lock);
  }
  void setLock(OzLock *l) {
    lock = l ? makeTaggedConst((ConstTerm*) l) : makeTaggedNULL();
  }
  void setLock(OZ_Term l) {
    Assert(oz_isConst(l) && tagged2Const(l)->getType() == Co_Lock);
    lock = l;
  }

  SRecord *getFreeRecord(void) {
    return freeFeatures ? tagged2SRecord(freeFeatures) : (SRecord *) NULL;
  }
  void setFreeRecord(SRecord *sr) {
    freeFeatures = sr ? makeTaggedSRecord(sr) : makeTaggedNULL();
  }
  void setFreeRecord(OZ_Term r) {
    freeFeatures = r;
  }

  TaggedRef getStateTerm(void) { return state; }
  ObjectState* getState(void) {
    return state == makeTaggedNULL() ? NULL : tagged2ObjectState(state);
  }
  void setState(ObjectState *s) {
    state = s ? makeTaggedConst(s) : makeTaggedNULL();
  }
  void setState(OZ_Term s) {
    Assert(s == makeTaggedNULL() || oz_isObjectState(s));
    state = s;
  }

  // support for the glue layer
  Bool isComplete() {
    return cls != makeTaggedNULL();
  }
  OZ_Term getRepresentation(void) {
    Assert(isComplete());
    OZ_Term fea = freeFeatures ? freeFeatures : oz_nil();
    OZ_Term lck = lock ? lock : oz_nil();
    return OZ_mkTupleC("#", 4, cls, fea, lck, state);
  }
  void setRepresentation(OZ_Term t) {
    Assert(!isComplete());
    SRecord* rec = tagged2SRecord(t);
    Assert(rec->getTupleWidth() == 4);
    setClass(rec->getArg(0));
    if (!oz_isNil(rec->getArg(1))) setFreeRecord(rec->getArg(1));
    if (!oz_isNil(rec->getArg(2))) setLock(rec->getArg(2));
    setState(rec->getArg(3));
  }

  // properties coming directly from the class
  const char *getPrintName(void);

  OzDictionary *getDefMethods() {   // unsafe
    return getClass()->getDefMethods();
  }
  SRecord *getUnfreeRecord() {   // unsafe
    return getClass()->getUnfreeRecord();
  }

  /* same functionality is also in instruction inlineDot */
  TaggedRef getFeature(TaggedRef lit) {   // unsafe
    SRecord *freefeat = getFreeRecord();
    if (freefeat) {
      TaggedRef ret = freefeat->getFeature(lit);
      if (ret!=makeTaggedNULL())
        return ret;
    }
    SRecord *fr = getUnfreeRecord();
    return fr ?  fr->getFeature(lit) : makeTaggedNULL();
  }

  TaggedRef replaceFeature(TaggedRef lit, TaggedRef value) {   // unsafe
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
    if (fr) {
      int ind = fr->getIndex(lit);
      if (ind != -1) {
        TaggedRef ret = fr->getArg(ind);
        fr->setArg(ind, value);
        return ret;
      }
    }
    return makeTaggedNULL();
  }

  TaggedRef getArityList();
  int getWidth ();

  // constructor
  OzObject(Board *bb, SRecord *s, OzClass *ac, SRecord *feat, OzLock *lck)
    : ConstTermWithHome(bb, Co_Object)
  {
    setClass(ac);
    setFreeRecord(feat);
    setLock(lck);
    setState(new ObjectState(bb, makeTaggedSRecord(s)));
  }

  // raph: this is the constructor to be used by the builder
  OzObject(Board *bb)
    : ConstTermWithHome(bb, Co_Object),
      cls(makeTaggedNULL()), lock(makeTaggedNULL()),
      freeFeatures(makeTaggedNULL()), state(makeTaggedNULL())
  {}
};

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
OzObject *tagged2Object(TaggedRef term)
{
  Assert(oz_isObject(term));
  return (OzObject *)tagged2Const(term);
}

// object operations (not state), with two arrays for inputs and outputs
OZ_Return objectOperation(OperationTag, OzObject*, TaggedRef*, TaggedRef*);

/*===================================================================
 * SChunk
 *=================================================================== */

class SChunk: public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  TaggedRef value;
public:
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(SChunk)
  SChunk(Board *b,TaggedRef v)
    : ConstTermWithHome(b,Co_Chunk), value(v)
  {
    Assert(v==0||oz_isRecord(v));
    Assert(b);
  };

  TaggedRef getValue() { return value; }
  TaggedRef getFeature(TaggedRef fea) { return OZ_subtree(value,fea); }
  TaggedRef getArityList() { return value ? ::getArityList(value) : oz_nil(); }
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

// chunk operations, with two arrays for inputs and outputs
OZ_Return chunkOperation(OperationTag, SChunk*, TaggedRef*, TaggedRef*);

/*===================================================================
 * Arrays
 *=================================================================== */

class OzArray: public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  TaggedRef *args;
  int offset, width; // mm2: put one into ConstTerm tag?

  TaggedRef *getArgs() { return args; }

public:
  NO_DEFAULT_CONSTRUCTORS(OzArray)
  OZPRINT

  int getLow()      { return offset; }
  int getWidth()    { return width; }
  int getHigh()     { return getWidth() + offset - 1; }
  bool checkIndex(int n) {
    n -= offset;
    return !(n>=width || n<0);
  }

  OzArray(Board *b, int low, int high, TaggedRef initvalue)
    : ConstTermWithHome(b,Co_Array)
  {
    Assert(oz_isRef(initvalue) || !oz_isVar(initvalue));

    offset = low;
    width = high-low+1;
    if (width <= 0) {
      width = 0;
      args = NULL; // mm2: attention if globalize gname!
    } else {
      args = (TaggedRef*) oz_heapMalloc(sizeof(TaggedRef)*width);
      if (args==NULL) { width = -1; return; }
      for(int i=0; i<width; i++) {
        args[i] = initvalue;
      }
    }
  }


  OZ_Term getArg(int n)
  {
    n -= offset;
    if (n>=getWidth() || n<0)
      return 0;

    OZ_Term out = getArgs()[n];
    Assert(oz_isRef(out) || !oz_isVar(out));

    return out;
  }

  int setArg(int n,TaggedRef val)
  {
    Assert(oz_isRef(val) || !oz_isVar(val));

    n -= offset;
    if (n>=getWidth() || n<0) return FALSE;

    getArgs()[n] = val;
    return TRUE;
  }

  OZ_Term exchange(int n,TaggedRef val)
  {
    Assert(oz_isRef(val) || !oz_isVar(val));

    n -= offset;
    if (n>=getWidth() || n<0) return FALSE;

    OZ_Term out = getArgs()[n];
    Assert(oz_isRef(out) || !oz_isVar(out));

    getArgs()[n] = val;

    return out;
  }

  TaggedRef *getRef() { return args; }
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

// array operations, with two arrays for inputs and outputs
OZ_Return arrayOperation(OperationTag, OzArray*, TaggedRef*, TaggedRef*);


/*===================================================================
 * Abstraction (incl. PrTabEntry, AssRegArray, AssReg)
 *=================================================================== */

#define K_XReg 0
#define K_YReg 1
#define K_GReg 2

#define K_RegMask  3
#define K_RegShift 2

class AssReg {
private:
  PosInt info;
public:
  OZPRINT
  NO_DEFAULT_CONSTRUCTORS2(AssReg)
  AssReg() {}

  PosInt getKind(void) {
    return info & K_RegMask;
  }
  PosInt getIndex(void) {
    return (info & ~K_RegMask) >> K_RegShift;
  }
  void set(PosInt index, PosInt kind) {
    Assert(kind == K_XReg || kind == K_YReg || kind == K_GReg);
    info = index << K_RegShift | kind;
  }
};

class AssRegArray  {
private:
  int    numbOfGRegs;
  AssReg ar[1]; // maybe less or more

  static AssRegArray * nullArray;

  static AssRegArray * _allocate(int n) {
    AssRegArray * a = (AssRegArray *) malloc(sizeof(AssRegArray) +
                                             (n-1) * sizeof(AssReg));
    a->numbOfGRegs = n;
    return a;
  }
public:

  static void init(void) {
    nullArray = _allocate(0);
  }

  NO_DEFAULT_CONSTRUCTORS(AssRegArray)

  static AssRegArray * allocate(int n) {
    if (n == 0)
      return nullArray;
    else
      return _allocate(n);
  }

  int getSize () { return (numbOfGRegs); }
  AssReg &operator[] (int elem) { return ar[elem]; }
  /* no bounds checking;    */
};


// Debugger ---------------------------------------------

class DbgInfo {
public:
  ProgramCounter PC;
  TaggedRef file;
  int line;
  DbgInfo *next;
  NO_DEFAULT_CONSTRUCTORS(DbgInfo)
  DbgInfo(ProgramCounter pc, TaggedRef f, int l, DbgInfo *nxt)
    : PC(pc), file(f), line(l), next(nxt) {};
};

extern DbgInfo *allDbgInfos;

// ---------------------------------------------


#define PR_SITED   0x1

class PrTabEntryProfile {
public:
  unsigned int numClosures, numCalled, heapUsed, samples, lastHeap;
  PrTabEntryProfile(void) {
    numClosures = numCalled = heapUsed = samples = lastHeap = 0;
  }
};

class PrTabEntry {
private:
  SRecordArity methodArity;
  TaggedRef printname, file, info; // Never change: garbage collection
  int line, colum;

  struct {
    unsigned int arity   : 16;
    unsigned int maxX    : 15;
    unsigned int isSited : 1;
  } p;

  ProgramCounter PC;
  int gSize;

public:
  PrTabEntry *next;
  static PrTabEntry *allPrTabEntries;

  static TaggedRef getProfileStats();
  static void profileReset();

  PrTabEntryProfile * pprof;

  CodeArea *codeBlock; // cache surrounding code block for code GC

public:
  OZPRINT
  NO_DEFAULT_CONSTRUCTORS(PrTabEntry)
  void init(TaggedRef name, SRecordArity arityInit,
             TaggedRef fil, int lin, int colu, TaggedRef fl, int max)
  {
    printname = name;
    p.maxX = max;
    file  = fil;
    line  = lin;
    colum = colu;

    codeBlock = NULL;
    p.isSited = 0;
    fl = oz_deref(fl);
    Assert(!oz_isRef(fl));
    while (oz_isLTuple(fl)) {
      OZ_Term ff=oz_deref(oz_head(fl));
      if (oz_eq(ff,AtomSited)) {
        p.isSited = 1;
      }
      fl = oz_deref(oz_tail(fl));
      Assert(!oz_isRef(fl));
    }
    Assert(oz_isNil(fl));

    Assert(oz_isLiteral(name));
    methodArity = arityInit;
    p.arity =  getWidth(arityInit);
    Assert((int)p.arity == getWidth(arityInit)); /* check for overflow */
    PC = NOCODE;
    info = oz_nil();
    pprof = NULL;
    next = allPrTabEntries;
    allPrTabEntries = this;
    DebugCheckT(gSize=-1);
  }

  PrTabEntryProfile * getProfile(void) {
    if (!pprof)
      pprof = new PrTabEntryProfile();
    return pprof;
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

  ~PrTabEntry(void) {
    if (pprof)
      delete pprof;
  }

  PrTabEntry(TaggedRef name, SRecordArity arityInit,
             TaggedRef fil, int lin, int colu, TaggedRef fl, int max)
  {
    init(name, arityInit, fil, lin, colu, fl, max);
  }


  void setGSize(int n) { gSize = n; }
  int getGSize() { return gSize; }
  int getArity () { return p.arity; }

  SRecordArity getMethodArity() { return methodArity; }
  void setMethodArity(SRecordArity sra) { methodArity = sra; }
  const char *getPrintName () { return tagged2Literal(printname)->getPrintName(); }
  TaggedRef getName () { return printname; }
  ProgramCounter getPC() { return PC; }
  void setPC(ProgramCounter pc) { PC = pc; }

  void setInfo(TaggedRef t) { info = t; }
  TaggedRef getInfo()       { return info; }

  int isSited()    { return p.isSited; }
  int getFlags()   { return p.isSited; }
  OZ_Term getFlagsList() {
    OZ_Term ret = oz_nil();
    if (isSited()) ret = oz_cons(AtomSited,ret);
    return ret;
  }

  int getMaxX()    { return p.maxX; }
  int getLine()   { return line; }
  int getColumn() { return colum; }
  TaggedRef getFile() { return file; }

  CodeArea *getCodeBlock();

  // retain the particular entry;
  void gCollect(void);
  void sClone(void);            // empty;
  // the whole list;
  static void gCollectPrTabEntries(void);
  static void gCollectDispose(void);
};



class Abstraction: public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
protected:
  PrTabEntry *pred;       // predicate
  TaggedRef *globals;     // array of global registers
  int arity : 16;         // arity of abstraction
  bool complete : 1;      // whether predicate and globals are complete

public:
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(Abstraction)

  // regular constructor, where predicate is known
  Abstraction(Board *bb, PrTabEntry *p) :
    ConstTermWithHome(bb, Co_Abstraction),
    pred(p), globals(NULL), arity(p->getArity()), complete(TRUE)
  {
    Assert(p && p->getGSize() >= 0);
    if (p->getGSize() > 0) {
      globals = (TaggedRef*) oz_heapMalloc(sizeof(TaggedRef) * p->getGSize());
      DebugCheckT(for (int i = p->getGSize(); i--; ) globals[i] = 0);
    }
  }

  static Abstraction *newAbstraction(PrTabEntry *prd,Board *bb) {
    return new Abstraction(bb, prd);
  }

  // special constructor, where predicate is not known yet
  Abstraction(Board *bb, int n) :
    ConstTermWithHome(bb, Co_Abstraction),
    pred(NULL), globals(NULL), arity(n), complete(FALSE)
  {}

  bool isComplete(void) { return complete; }
  void setComplete() {
    Assert(pred);
    Assert(pred->getGSize() == 0 || globals);
    complete = TRUE;
  }

  void initG(int i, TaggedRef val) {
    Assert(globals && i>=0 && i<getPred()->getGSize());
    globals[i]=val;
  }
  TaggedRef getG(int i) {
    Assert(globals && i>=0 && i<cacGetPred()->getGSize());
    return globals[i];
  }
  TaggedRef *getGRef() { return globals; }

  // WARNING: During garbage collection we overwrite the pred pointer
  // with a to pointer.  During GC use cacGetPred() instead which Does
  // The Right Thing (and don't forget these other methods that
  // indirectly call getPred() too).  Making cacGetPred() the default
  // caused performance problems in one of my test programs.
  PrTabEntry *getPred() {
    Assert (!cacIsCopied());
    return pred;
  }

  // set predicate (use only with second constructor!)
  void setPred(PrTabEntry *p) {
    Assert(!pred && p && p->getArity() == arity);
    pred = p;
    if (p->getGSize() > 0) {
      globals = (TaggedRef*) oz_heapMalloc(sizeof(TaggedRef) * p->getGSize());
      // nullifying globals is necessary for garbage collection
      for (int i = p->getGSize(); i--; ) globals[i] = 0;
    } else {
      setComplete();
    }
  }

  // always safe (even when the predicate is unknown)
  int getArity() { return arity; }

  // those are not safe: check isComplete() before!
  ProgramCounter getPC() { return getPred()->getPC(); }
  SRecordArity getMethodArity() { return getPred()->getMethodArity(); }
  TaggedRef getName() { return getPred()->getName(); }
  const char *getPrintName();

  TaggedRef DBGgetGlobals();

  GName *globalize();

  //
  // Garbage collection and copying
  //

  // pred field is overwritten when abstraction first
  // copied to to-space.  If we need to access getPred()
  // during gc this ensures result is always correct
  PrTabEntry *cacGetPred() {
    if (cacIsCopied())
      return cacGetCopy()->getPred();
    else
      return pred;
  }

  Bool cacIsCopied(void) {
    return IsMarkedPointer(pred,1);
  }
  Abstraction * cacGetCopy(void) {
    Assert(cacIsCopied());
    return (Abstraction *) UnMarkPointer(pred,1);
  }
  int32 ** cacGetCopyField(void) {
    return (int32 **) &pred;
  }

  void cacCopy(Abstraction * fwd) {
    Assert(!cacIsCopied());
    pred = (PrTabEntry *) MarkPointer(fwd,1);
  }

  Abstraction* gCollect(int gUsageLen, int * gUsage);
  Abstraction* gCollect(void) {
    return gCollect(0, (int *) NULL);
  }
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
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  const char * mod_name;
  const char * bi_name;
  OZ_CFun fun;
  short inArity;
  short outArity;
  Bool sited;
#ifdef PROFILE_BI
  unsigned long counter;
#endif

  void initname(void);

public:
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(Builtin)

  /* use malloc to allocate memory */
  static void *operator new(size_t chunk_size) { return ::new char[chunk_size]; }

  Builtin(const char * mn, const char * bn,
          int inArity, int outArity,
          OZ_CFun fn, Bool nat);

  OZ_CFun getFun(void) {
    return fun;
  }
  int getArity(void) {
    return inArity+outArity;
  }
  int getInArity(void) {
    return inArity;
  }
  int getOutArity(void) {
    return outArity;
  }
  TaggedRef getName() {
    if (mod_name) {
      initname();
      Assert(!mod_name);
    }
    return (TaggedRef) ToInt32(bi_name);
  }
  const char * getPrintName(void) {
    return tagged2Literal(getName())->getPrintName();
  }
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
 *=================================================================== */

class OzCell:public ConstTermWithHome{
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  // When the cell is distributed, val == 0 means that the state is not present.
  TaggedRef val;

public:
  NO_DEFAULT_CONSTRUCTORS(OzCell);

  OzCell(Board *b, TaggedRef v) : ConstTermWithHome(b, Co_Cell), val(v) {}
  TaggedRef getValue() { return val; }

  void setValue(TaggedRef v) { val=v; }

  TaggedRef exchangeValue(TaggedRef v) {
    TaggedRef ret = val;
    val = v;
    return ret;}

  TaggedRef *getRef() { return &val; }
};

inline
Bool oz_isCell(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Cell;
}

inline
OzCell *tagged2Cell(TaggedRef term)
{
  Assert(oz_isCell(term));
  return (OzCell *) tagged2Const(term);
}

// cell operations, with two arrays for inputs and outputs
// (note: input arity of 'op' is OperationIn[op]-1)
OZ_Return cellOperation(OperationTag, OzCell*, TaggedRef*, TaggedRef*);

/*===================================================================
 * Port
 *=================================================================== */

class OzPort : public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
public:
  TaggedRef strm;
  NO_DEFAULT_CONSTRUCTORS(OzPort)
  TaggedRef exchangeStream(TaggedRef newStream) {
    TaggedRef ret = strm;
    strm = newStream;
    return ret;
  }

  OzPort(Board *b, TaggedRef s):ConstTermWithHome(b, Co_Port), strm(s) {}
};

inline
Bool oz_isPort(TaggedRef term)
{ return oz_isConst(term) && tagged2Const(term)->getType() == Co_Port;}

inline OzPort *tagged2Port(TaggedRef term)
{ return (OzPort *) tagged2Const(term);}

// implemented in builtins.cc
void doPortSend(OzPort *port, TaggedRef val, Board*);

/*===================================================================
 * Space
 *=================================================================== */

class Space: public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  Board *solve;
  // The solve pointer can be:
  // - 0 (the board is failed and has been discarded by the garbage
  //      collector)
  // - 1 (the space has been merged)
  // or a valid pointer
public:
  OZPRINTLONG
  NO_DEFAULT_CONSTRUCTORS(Space)

  Space(Board *h, Board *s) : ConstTermWithHome(h, Co_Space), solve(s) {};
// bmc: I do not think that we need this constructor.
// Space(int i, TertType t) : ConstTermWithHome(i,Co_Space,t) {}

  Bool isMarkedFailed(void) {
    return !solve;
  }

  Bool isMarkedMerged(void) {
    return (solve == (Board *) 1) ? OK : NO;
  }

  Board * getSpace(void) {
    return solve;
  }

  void  markMerged(void) {
    solve = (Board *) 1;
  }
};

inline
Bool oz_isSpace(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Space;
}

/*===================================================================
 * Unusables
 *=================================================================== */

class UnusableResource: public ConstTermWithHome {
public:
  NO_DEFAULT_CONSTRUCTORS(UnusableResource);
  UnusableResource():ConstTermWithHome(NULL, Co_Resource) {}
};

/*===================================================================
 * PendingThreadList  (only used for locks now)
 *=================================================================== */

// a linked list of pairs (thread, control var)
class PendingThreadList {
public:
  TaggedRef thread;
  TaggedRef controlvar;
  PendingThreadList *next;

  PendingThreadList(PendingThreadList *pt):
    thread(0), controlvar(0), next(pt) {}
  PendingThreadList(TaggedRef t, TaggedRef cv, PendingThreadList *pt):
    thread(t), controlvar(cv), next(pt) {}
  USEFREELISTMEMORY;
  void dispose(){oz_freeListDispose(this,sizeof(PendingThreadList));}
};

// convert from/to an Oz list of pairs
TaggedRef pendingThreadList2List(PendingThreadList*);
PendingThreadList* list2PendingThreadList(TaggedRef);

// add/remove elements
void pendingThreadAdd(PendingThreadList** pt, TaggedRef t, TaggedRef cv);
void pendingThreadDrop(PendingThreadList** pt, TaggedRef t);
TaggedRef pendingThreadResumeFirst(PendingThreadList** pt);
void disposePendingThreadList(PendingThreadList*);

/*===================================================================
 * Locks
 *=================================================================== */

class OzLock:public ConstTermWithHome{
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  TaggedRef locker;               // the thread that has the lock
  int depth;                      // locking depth (reentrant locks)
  PendingThreadList *pending;     // threads waiting for the lock

public:
  NO_DEFAULT_CONSTRUCTORS(OzLock)
  OzLock() { Assert(0); }
  OzLock(Board *b):
    ConstTermWithHome(b, Co_Lock), locker(0), depth(0), pending(NULL) {}

  // get/set the locker thread, locking depth, and queue of pending
  // threads.  The latter is a list of pairs ThreadId#ControlVar.
  TaggedRef getLocker() { return locker; }
  void setLocker(TaggedRef t) { locker = t; }

  int getLockingDepth() { return depth; }
  void setLockingDepth(int d) { depth = d; }

  PendingThreadList* getPending() { return pending; }
  void setPending(PendingThreadList* pt) {
    disposePendingThreadList(pending);
    pending = pt;
  }

  // simple test
  Bool hasLock(TaggedRef t) { return oz_eq(t, locker); }

  // try to take the lock, and return TRUE if done.  Note that the
  // thread must explicitly subscribe if the lock was not granted!
  Bool take(TaggedRef t) {
    if (hasLock(t)) {
      Assert(depth > 0);
      depth++;
      return TRUE;
    }
    if (locker == 0) {
      setLocker(t);
      Assert(depth == 0);
      depth = 1;
      return TRUE;
    }
    return FALSE;
  }

  // subscribe to the pending list of the lock
  void subscribe(TaggedRef t, TaggedRef cv) {
    Assert(!oz_eq(t, locker) && locker != 0);
    pendingThreadAdd(&pending, t, cv);
  }

  // release the lock for thread t (possibly earlier than expected)
  void release(TaggedRef t) {
    Assert(locker != 0 && depth > 0);
    if (oz_eq(t, locker)) {
      depth--;
      if (depth == 0) {
        if (pending != NULL) {
          locker = pendingThreadResumeFirst(&pending);
          depth = 1;
        } else {
          locker = 0;
        }
      }
    } else { // this is an early release, drop t from the queue
      pendingThreadDrop(&pending, t);
    }
  }
};

inline
Bool oz_isLock(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Lock;
}

// those operations support both the centralized and distributed cases
OZ_Return lockTake(OzLock*);
void lockRelease(OzLock*);

/*===================================================================
 *
 *=================================================================== */

char *toC(OZ_Term);
TaggedRef oz_getPrintName(TaggedRef);
TaggedRef reverseC(TaggedRef l);
TaggedRef appendI(TaggedRef x,TaggedRef y);
Bool member(TaggedRef elem,TaggedRef list);


inline
Bool oz_isExtensionPlus(TaggedRef t) {
  if (oz_isExtension(t)) return TRUE;
  if (oz_isConst(t)) {
    TypeOfConst tc = tagged2Const(t)->getType();
    if (tc==Co_Resource || tc==Co_Foreign_Pointer) return TRUE;
  }
  if (oz_isFSetValue(t)) return TRUE;
  return FALSE;
}

OZ_Term oz_string(const char *s, const int len, const OZ_Term tail);


/*===================================================================
 * Service Registry
 *=================================================================== */

extern OZ_Term registry_get(OZ_Term);
inline OZ_Term registry_get(char*s) {
  return registry_get(oz_atom(s));
}
extern void registry_put(OZ_Term,OZ_Term);
inline void registry_put(char*s,OZ_Term v) {
  registry_put(oz_atom(s),v);
}

/*
 * Macros for easy record construction
 *
 */


/*
 * Needed because literal commas (,) are interpreted as argument
 * separators
 */

#define OZ_COMMA ,

#define OZ_MAKE_RECORD_S(LABEL,WIDTH,ARITY_S,FIELDS,REC) \
  OZ_Term REC;                                                            \
  {                                                                       \
    extern Arity * __OMR_static(const int,const char**,int*);             \
    extern OZ_Term __OMR_dynamic(const int,OZ_Term,Arity*,int*,OZ_Term*); \
    static Bool      __once  = OK;                                        \
    static TaggedRef __label = makeTaggedNULL();                          \
    static Arity *   __arity = (Arity *) NULL;                            \
    static const char * __c_feat[WIDTH] = ARITY_S;                        \
    static int       __i_feat[WIDTH];                                     \
    if (__once) {                                                         \
      __once = NO;                                                        \
      __label = oz_atomNoDup(LABEL);                                      \
      __arity = __OMR_static(WIDTH,__c_feat,__i_feat);                    \
    }                                                                     \
    TaggedRef __fields[WIDTH] = FIELDS;                                   \
    REC = __OMR_dynamic(WIDTH,__label,__arity,__i_feat,__fields);         \
  }

#endif
