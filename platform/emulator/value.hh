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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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
#include "hashtbl.hh"

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
       AtomDebugCall, AtomDebugCond, AtomDebugHandler, AtomDebugLock,

       NameOoFreeFlag,NameOoAttr,NameOoFreeFeatR,NameOoUnFreeFeat,
       NameOoFastMeth,NameOoDefaults,NameOoRequiredArg,NameOoDefaultVar,
       NameOoPrintName,NameOoLocking,NameOoFallback,NameOoId,
       AtomNew, AtomSend, AtomApply,

       NameUnit,
       AtomKinded, AtomDet, AtomRecord, AtomFSet,
       // Atoms for System.get and System.set
       AtomActive, AtomAtoms, AtomBuiltins, AtomCommitted,
       AtomMoreInfo, AtomTotal, AtomCache,
       AtomCloned, AtomCode, AtomCopy, AtomCreated, AtomDebug, AtomDepth,
       AtomFeed, AtomForeign, AtomFree, AtomFreelist, AtomGC, AtomHigh,
       AtomHints, AtomIdle, AtomInt, AtomInvoked, AtomLimits, AtomLoad,
       AtomLocation, AtomMedium, AtomNames, AtomOn, AtomPropagate,
       AtomPropagators, AtomRun, AtomRunnable, AtomShowSuspension,
       AtomStopOnToplevelFailure, AtomSystem, AtomThread,
       AtomThreshold, AtomTolerance, AtomUser, AtomVariables, AtomWidth,
  AtomDetailed,
  AtomHeap, AtomDebugIP, AtomDebugPerdio,
  // Atoms for NetError Handlers
  AtomTempBlocked, AtomPermBlocked,
  AtomPermMe, AtomTempMe,
  AtomPermAllOthers, AtomTempAllOthers,
  AtomPermSomeOther, AtomTempSomeOther,

RecordFailure,
  E_ERROR, E_KERNEL, E_OBJECT, E_TK, E_OS, E_SYSTEM,
  BI_Unify,BI_Show,BI_send,BI_restop;


extern Board *ozx_rootBoard();

/*===================================================================
 *  Handlers
 *=================================================================== */

enum WatcherKind{
  KIND_HANDLER = 0,
    KIND_WATCHER = 1};

enum EntityCondFlags{
  ENTITY_NORMAL=0,
  PERM_BLOCKED = 2,
    TEMP_BLOCKED = 1,
    PERM_ALL = 4,
    TEMP_ALL = 8,
    PERM_SOME = 16,
    TEMP_SOME = 32,
    PERM_ME =64,
    TEMP_ME=128};

typedef unsigned int EntityCond;

class EntityInfo{
  friend class Tertiary;
protected:
  Watcher *watchers;
  short entityCond;
  short managerEntityCond;

public:
  USEHEAPMEMORY;
  NO_DEFAULT_CONSTRUCTORS(EntityInfo);
  EntityInfo(Watcher *w){
    entityCond=ENTITY_NORMAL;
    managerEntityCond=ENTITY_NORMAL;
    watchers=w;}
  EntityInfo(EntityCond c){
    entityCond=c;
    managerEntityCond=ENTITY_NORMAL;
    watchers=NULL;}
  EntityInfo(EntityCond c,EntityCond d){
    entityCond=c;
    managerEntityCond=d;
    watchers=NULL;}
  void gcWatchers();
};

class Watcher{
friend class Tertiary;
friend class EntityInfo;
protected:
  TaggedRef proc;
  Watcher *next;
  Thread *thread;
  short kind;
  short watchcond;
public:
  USEHEAPMEMORY;

  NO_DEFAULT_CONSTRUCTORS(Watcher);
  // handler
  Watcher(TaggedRef p,Thread* t,EntityCond wc){
    proc=p;
    next=NULL;
    thread=t;
    kind=KIND_HANDLER;
    Assert((wc==PERM_BLOCKED) || (wc==TEMP_BLOCKED|PERM_BLOCKED));
    watchcond=wc;}

// watcher
  Watcher(TaggedRef p,EntityCond wc){
    proc=p;
    next=NULL;
    thread=NULL;
    kind=KIND_WATCHER;
    watchcond=wc;}

  Bool isHandler(){ return kind==KIND_HANDLER;}
  void setNext(Watcher* w){next=w;}
  Watcher* getNext(){return next;}
  void invokeHandler(EntityCond ec,Tertiary* t);
  void invokeWatcher(EntityCond ec,Tertiary* t);
  Thread* getThread(){Assert(thread!=NULL);return thread;}
  Bool isTriggered(EntityCond ec){
    if(ec & watchcond) return OK;
    return NO;}
  EntityCond getWatchCond(){return watchcond;}
};

/*===================================================================
 * Literal
 *=================================================================== */


/* any combination of the following must be different from GCTAG,
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
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Literal);

  void init() { flagsAndOthers=0; }
  void setFlag(int flag) { flagsAndOthers |= flag; }
  int getFlags() { return (flagsAndOthers&litFlagsMask); }
  int getOthers() { return flagsAndOthers>>sizeOfLitFlags; }
  void setOthers(int value) { flagsAndOthers = getFlags()|(value<<sizeOfLitFlags); }

  Bool isName()       { return (getFlags()&Lit_isName); }
  Bool isNamedName()  { return (getFlags()&Lit_isNamedName); }
  Bool isUniqueName() { return (getFlags()&Lit_isUniqueName); }
  Bool isAtom()       { return !isName(); }

  const char *getPrintName();

  Literal *gc();

  TaggedRef *getRef() { return (TaggedRef*)&flagsAndOthers; }

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
      ? ozx_rootBoard() : (Board*)ToPointer(homeOrGName);
  }

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
  NO_DEFAULT_CONSTRUCTORS(NamedName);
  const char *printName;
  static NamedName *newNamedName(const char *str);
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
  return ((int)n)>>tagSize;
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
TaggedRef newTaggedFloat(double i)
{
  return makeTaggedFloat(Float::newFloat(i));
}

/*===================================================================
 * BigInt
 *=================================================================== */

class BigInt {
private:
  MP_INT value;

public:
  USEFREELISTMEMORY;
  OZPRINT;

  NO_DEFAULT_CONSTRUCTORS2(BigInt);

  BigInt() {
    mpz_init(&value);
  }

  BigInt(int i)          { mpz_init_set_si(&value,i); }
  BigInt(unsigned int i) { mpz_init_set_ui(&value,i); }

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

  unsigned int hash()              { return 75; } // all BigInt hash to same value
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
TaggedRef makeUnsignedInt(unsigned int i)
{
  if (i > (unsigned int) OzMaxInt)
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
private:
  TaggedRef args[2];

public:
  USEHEAPMEMORY32;
  OZPRINTLONG;

  NO_DEFAULT_CONSTRUCTORS2(LTuple);
  LTuple(void) {
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
TaggedRef tail(TaggedRef list)
{
  Assert(isLTuple(list));
  return tagged2LTuple(list)->getTail();
}

inline
int fastlength(OZ_Term l)
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
#ifdef FOREIGN_POINTER
  Co_Foreign_Pointer,
#else
  Co_UNUSED1,
#endif
  Co_UNUSED2,
  Co_Thread,
  Co_Abstraction,

  Co_Builtin,       /* 4 */
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
  Co_HeapChunk,
  Co_Array,
  Co_Dictionary,    /* 12 */
  Dummy = GCTAG,
  Co_Lock,
  Co_Class
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
  USEHEAPMEMORY;
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(ConstTerm);
  ConstTerm(TypeOfConst t)  { setTagged(t,NULL); }

  Bool gcIsMarked(void) {
    return GCISMARKED(ctu.tagged);
  }
  void gcMark(ConstTerm * fwd) {
    ctu.tagged = GCMARK(fwd);
  }
  void ** gcGetMarkField(void) {
    return (void **) &ctu.tagged;
  }
  ConstTerm * gcGetFwd(void) {
    Assert(gcIsMarked());
    return (ConstTerm *) GCUNMARK((int) ctu.tagged);
  }
  ConstTerm *gcConstTerm(void);
  ConstTerm *gcConstTermSpec(void);
  void gcConstRecurse(void);

  void setTagged(TypeOfConst t, void *p) {
    ctu.tagged = makeTaggedRef2p((TypeOfTerm)t,p);
  }
  TypeOfConst getType()     { return (TypeOfConst) tagTypeOf(ctu.tagged); }

  const char *getPrintName();
  int getArity();
  void *getPtr() {
    return isNullPtr(ctu.tagged) ? NULL : tagValueOf(ctu.tagged);
  }
  void setPtr(void *p)  { setTagged(getType(),p); }
  TaggedRef *getRef()   { return &ctu.tagged; }

  /* optimized isChunk test */
  Bool isChunk() { return (int) getType() >= (int) Co_Object; }
};


#define CWH_Board 0
#define CWH_GName 1


class ConstTermWithHome: public ConstTerm {
private:
  TaggedPtr boardOrGName;
  void setBoard(Board *b)
  {
    boardOrGName.setPtr(b);
    boardOrGName.setType(CWH_Board);
  }
public:
  NO_DEFAULT_CONSTRUCTORS(ConstTermWithHome);
  ConstTermWithHome(Board *b, TypeOfConst t) : ConstTerm(t) { setBoard(b);  }

  Board *getBoardInternal() {
    return hasGName() ? ozx_rootBoard() : (Board*)boardOrGName.getPtr();
  }

  void gcConstTermWithHome();
  void setGName(GName *gn) {
    Assert(gn);
    boardOrGName.setPtr(gn);
    boardOrGName.setType(CWH_GName);
  }
  Bool hasGName() { return (boardOrGName.getType()&CWH_GName); }
  GName *getGName1() {
    return hasGName()?(GName *)boardOrGName.getPtr():(GName *)NULL;
  }
};

class Tertiary: public ConstTerm {
private:
  TaggedPtr tagged;
  EntityInfo* info;
public:

  TertType getTertType()       { return (TertType) tagged.getType(); }
  void setTertType(TertType t) { tagged.setType((int) t); }

  NO_DEFAULT_CONSTRUCTORS(Tertiary);
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

  Bool errorIgnore(){
    if(getEntityCond()==ENTITY_NORMAL) return OK;
    if(info->watchers==NULL) return OK;
    return NO;}

  Bool hasWatchers(){
    if(info==NULL) return NO;
    if(info->watchers==NULL) return NO;
    return OK;}

  EntityCond getEntityCond(){
    if(info==NULL) return ENTITY_NORMAL;
    return info->entityCond | info->managerEntityCond;}

  Bool setEntityCondOwn(EntityCond c){
    if(info==NULL){
      info= new EntityInfo(c);
      return OK;}
    EntityCond old_ec=getEntityCond();
    info->entityCond = (info->entityCond | c);
    if(getEntityCond()==old_ec) return NO;
    return OK;}

  Bool setEntityCondManager(EntityCond c){
    if(info==NULL){
      info= new EntityInfo(ENTITY_NORMAL,c);
      return OK;}
    EntityCond old_ec=getEntityCond();
    info->managerEntityCond=info->managerEntityCond | c;
    if(getEntityCond()==old_ec) return NO;
    return OK;}

  Bool resetEntityCondProxy(EntityCond c){
    Assert(info!=NULL);
    EntityCond old_ec=getEntityCond();
    info->entityCond &= ~c;
    if(getEntityCond()==old_ec) return NO;
    return OK;}

  Bool resetEntityCondManager(EntityCond c){
    Assert(!(c & PERM_BLOCKED|PERM_ME|PERM_SOME|PERM_ALL));
    Assert(info!=NULL);
    EntityCond old_ec=getEntityCond();
    info->managerEntityCond &= ~c;
    if(getEntityCond()==old_ec) return NO;
    return OK;}

  void gcEntityInfo();

  Watcher *getWatchers(){
    return info->watchers;}

  Watcher *getWatchersIfExist(){
    if(info==NULL){return NULL;}
    return info->watchers;}

  Watcher** getWatcherBase(){
    if(info==NULL) return NULL;
    if(info->watchers==NULL) return NULL;
    return &(info->watchers);}

  Watcher** findWatcherBase(Thread*,EntityCond);

  void setWatchers(Watcher* e){
    info->watchers=e;}

  void insertWatcher(Watcher* w);
  void releaseWatcher(Watcher*);
  Bool handlerExists(Thread *);

  void setIndex(int i) { tagged.setIndex(i); }
  int getIndex() { return tagged.getIndex(); }
  void setPointer (void *p) { tagged.setPtr(p); }
  void *getPointer() { return tagged.getPtr(); }

  Bool checkTertiary(TypeOfConst s,TertType t){
    return (s==getType() && t==getTertType());}

  Board *getBoardInternal() {
    return isLocal() ? (Board*)getPointer() : ozx_rootBoard();}

  Bool isLocal()   { return (getTertType() == Te_Local); }
  Bool isManager() { return (getTertType() == Te_Manager); }
  Bool isProxy()   { return (getTertType() == Te_Proxy); }

  void setBoard(Board *b);
  void globalizeTert();
  void localize();

  void gcProxy();
  void gcManager();
  void gcTertiary();
  void gcTertiaryInfo();
  void gcBorrowMark();

  Bool installHandler(EntityCond,TaggedRef,Thread*);
  Bool deinstallHandler(Thread*);
  void installWatcher(EntityCond,TaggedRef);
  Bool deinstallWatcher(EntityCond,TaggedRef);

  void entityProblem();
  void managerProbeFault(Site*,int);
  void proxyProbeFault(int);
  void restop();

  Bool maybeHasInform(){
    if(info==NULL) return NO;
    return OK;}
  Bool threadIsPending(Thread* th);
};



/*===================================================================
 * HeapChunk
 *=================================================================== */

class HeapChunk: public ConstTerm {
private:
  size_t chunk_size;
  void * chunk_data;
  void * copyChunkData(void) {
    char * data = (char *) allocate(chunk_size);
    for (int i = chunk_size; i--; )
      data[i] = ((char *) chunk_data)[i];
    return (void *) data;
  }
  void * allocate(int size) {
    COUNT1(sizeHeapChunks,size);
    return (void *) alignedMalloc(size, sizeof(double));
  }
public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS(HeapChunk);
  HeapChunk(int size)
  : ConstTerm(Co_HeapChunk), chunk_size(size), chunk_data(allocate(size))
  {
    COUNT1(sizeHeapChunks,sizeof(HeapChunk));
  }

  size_t getChunkSize(void) { return chunk_size; }

  void * getChunkData(void) { return chunk_data; }

  HeapChunk * gc(void);
};

#ifdef FOREIGN_POINTER
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
#endif

/*===================================================================
 * SRecord: incl. Arity, ArityTable
 *=================================================================== */

inline
Bool isFeature(TaggedRef lab) { return isLiteral(lab) || isInt(lab); }

#define CHECK_FEATURE(lab) \
Assert(!isRef(lab) && !isAnyVar(lab) && isFeature(lab));


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

  int hashmask;         // size-1, used as mask for hashing and opt marker
  int width;            // next unused index in RefsArray (for add())
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

  void downSize(unsigned int s) { // TMUELLER
    Assert(isTuple());
    setTupleWidth(s);
  } // FD

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
      if (!isSmallInt(feature)) return -1;
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
    return literalEq(getLabel(),lbl) &&
           sameSRecordArity(getSRecordArity(),arity);
  }

  Bool compareFunctor(SRecord* str) {
    return compareSortAndArity(str->getLabel(),str->getSRecordArity());
  }

  TaggedRef *getCycleAddr() { return &label; }
};

TaggedRef oz_adjoinAt(SRecord *, TaggedRef feature, TaggedRef value);
TaggedRef oz_adjoin(SRecord *, SRecord *);
TaggedRef oz_adjoinList(SRecord *, TaggedRef arity, TaggedRef proplist);

Bool isSorted(TaggedRef list);

TaggedRef duplist(TaggedRef list, int &len);
TaggedRef sortlist(TaggedRef list,int len);
TaggedRef packsort(TaggedRef list);

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
  return isLTuple(term) || isSTuple(term) || isLiteral(term);
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
 * ObjectClass
 *=================================================================== */

/* Internal representation of Oz classes */

class ObjectClass: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
private:
  SRecord *features;
  SRecord *unfreeFeatures;
  OzDictionary *fastMethods;
  OzDictionary *defaultMethods;
  Bool locking;
public:
  USEHEAPMEMORY;
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(ObjectClass);

  ObjectClass(SRecord *feat,OzDictionary *fm,SRecord *uf,OzDictionary *dm,
              Bool lck, Board *b)
    : ConstTermWithHome(b,Co_Class)
  {
    features       = feat;
    fastMethods    = fm;
    unfreeFeatures = uf;
    defaultMethods = dm;
    locking        = lck;
  }

  Bool supportsLocking() { return locking; }

  OzDictionary *getDefMethods()  { return defaultMethods; }
  OzDictionary *getfastMethods() { return fastMethods; }

  Abstraction *getMethod(TaggedRef label, SRecordArity arity, RefsArray X,
                         Bool &defaultsUsed);

  TaggedRef getFallbackNew();
  TaggedRef getFallbackSend();
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
              OzDictionary *dm, Bool l)
  {
    features       = feat;
    fastMethods    = fm;
    unfreeFeatures = uf;
    defaultMethods = dm;
    locking        = l;
  }

  TaggedRef getArityList();
  int getWidth();

  GName *getGName() {
    GName *gn = getGName1();
    Assert(gn);
    return gn;
  }
  void globalize();
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
protected:
  // ObjectClass *cl is in ptr field of ConstTerm
  RecOrCell state;
  OzLock *lock;
  SRecord *freeFeatures;
public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Object);

  Object(Board *bb,SRecord *s,ObjectClass *ac,SRecord *feat, OzLock *lck):
    Tertiary(bb, Co_Object,Te_Local)
  {
    setFreeRecord(feat);
    setClass(ac);
    setState(s);
    lock = lck;
  }

  Object(int i): Tertiary(i,Co_Object,Te_Proxy)
  {
    setFreeRecord(NULL);
    setClass(NULL);
    setState((Tertiary*)NULL);
    lock = 0;
  }

  void setClass(ObjectClass *c) { setPtr(c); }

  OzLock *getLock() { return lock; }
  void setLock(OzLock *l) { lock=l; }

  ObjectClass *getClass()       { return (ObjectClass*) getPtr(); }
  OzDictionary *getMethods()    { return getClass()->getfastMethods(); }
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
Bool isObjectClass(ConstTerm *t)
{
  return (t->getType()==Co_Class);
}

inline
Bool isObjectClass(TaggedRef term)
{
  return isConst(term) && isObjectClass(tagged2Const(term));
}

inline
ObjectClass *tagged2ObjectClass(TaggedRef term)
{
  Assert(isObjectClass(term));
  return (ObjectClass *)tagged2Const(term);
}

inline
Bool isClass(TaggedRef term)
{
  return isObjectClass(term);
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
    Assert(v==0||isRecord(v));
    Assert(b);
  };

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
  NO_DEFAULT_CONSTRUCTORS(OzArray);
  OZPRINT;

  OzArray(Board *b, int low, int high, TaggedRef initvalue)
    : ConstTermWithHome(b,Co_Array)
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


class PrTabEntry {
private:
  TaggedRef printname;
  unsigned short arity;
  SRecordArity methodArity;
  TaggedRef fileName;
  int lineno;
  TaggedRef info;
  TaggedRef names; // list of names for components: when loading
                   // theses names are replaced
                   // default: unit --> no replacements

public:
  Bool copyOnce; // for functors
  PrTabEntry *next;
  unsigned int numClosures, numCalled, heapUsed, samples, lastHeap;
  static PrTabEntry *allPrTabEntries;
  static void printPrTabEntries();
  static TaggedRef getProfileStats();
  static void profileReset();

  ProgramCounter PC;

public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS(PrTabEntry);
  PrTabEntry (TaggedRef name, SRecordArity arityInit,
              TaggedRef file, int line, Bool co)
  : printname(name), fileName(file), lineno(line)
  {
    Assert(isLiteral(name));
    methodArity = arityInit;
    arity =  (unsigned short) getWidth(arityInit);
    Assert((int)arity == getWidth(arityInit)); /* check for overflow */
    PC = NOCODE;
    info = nil();
    names = NameUnit;
    numClosures = numCalled = heapUsed = samples = lastHeap = 0;
    copyOnce = co;
    next = allPrTabEntries;
    allPrTabEntries = this;
  }

  int getArity () { return (int) arity; }
  TaggedRef getFileName() { return fileName; }
  int getLine() { return lineno; }
  SRecordArity getMethodArity() { return methodArity; }
  const char *getPrintName () { return tagged2Literal(printname)->getPrintName(); }
  TaggedRef getName () { return printname; }
  ProgramCounter getPC() { return PC; }
  void setPC(ProgramCounter pc) { PC = pc; }

  void setInfo(TaggedRef t) { info = t; }
  TaggedRef getInfo()       { return info; }

  void setNames(TaggedRef n) { names = n; }
  TaggedRef getNames()       { return names; }

  void patchFileAndLine();

  void gcPrTabEntry();
};



class Abstraction: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
protected:
  // PrTabEntry *pred is in ptr field of ConstTerm
  RefsArray gRegs;
public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(Abstraction);
  Abstraction(PrTabEntry *prd, RefsArray gregs, Board *b)
    : ConstTermWithHome(b,Co_Abstraction), gRegs(gregs)
  {
    setPtr(prd);
  }

  PrTabEntry *getPred()  { return (PrTabEntry *) getPtr(); }
  RefsArray &getGRegs()  { return gRegs; }
  ProgramCounter getPC() { return getPred()->getPC(); }
  int getArity()         { return getPred()->getArity(); }
  SRecordArity getMethodArity()   { return getPred()->getMethodArity(); }
  int getGSize()         { return getRefsArraySize(gRegs); }
  const char *getPrintName()   { return getPred()->getPrintName(); }
  TaggedRef getName()    { return getPred()->getName(); }

  TaggedRef DBGgetGlobals();

  GName *globalize();
  GName *getGName() {
    GName *gn = getGName1();
    return gn ? gn : globalize();
  }
  void import(RefsArray g, ProgramCounter pc) {
    gRegs = g;
    if (pc!=NOCODE) {
      getPred()->setPC(pc);
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
private:
  TaggedRef printname; //must be atom
  int arity;
  OZ_CFun fun;
  IFOR inlineFun;
#ifdef PROFILE_BI
  unsigned long counter;
#endif

public:
  OZPRINTLONG;
  NO_DEFAULT_CONSTRUCTORS(BuiltinTabEntry);

  /* use malloc to allocate memory */
  static void *operator new(size_t chunk_size)
  { return ::new char[chunk_size]; }

  BuiltinTabEntry(const char *s,int arty,OZ_CFun fn,IFOR infun)
  : arity(arty),fun(fn), inlineFun(infun), ConstTerm(Co_Builtin)
  {
    printname = makeTaggedAtom(s);
#ifdef PROFILE_BI
    counter = 0;
#endif
  }

  OZ_CFun getFun() { return fun; }
  int getArity() { return arity; }
  const char *getPrintName() {
    return tagged2Literal(printname)->getPrintName();
  }
  TaggedRef getName() { return printname; }
  IFOR getInlineFun() { return inlineFun; }

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
 * Unused third field from tertiary.
 *
 *=================================================================== */


class CellLocal:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  TaggedRef val;
public:
  OZPRINTLONG;

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

class CellSec{
friend class CellFrame;
friend class CellManager;
friend class Chain;
private:
  unsigned int state;
  TaggedRef head;
  PendThread* pending;
  Site* next;
  TaggedRef contents;
  PendBinding* pendBinding;

public:
  USEHEAPMEMORY;
  NO_DEFAULT_CONSTRUCTORS2(CellSec);
  CellSec(TaggedRef val){ // on globalize
    state=Cell_Lock_Valid;
    pending=NULL;
    next=NULL;
    contents=val;
    pendBinding=NULL;}

  CellSec(){ // on Proxy becoming Frame
    state=Cell_Lock_Invalid;
    pending=NULL;
    pendBinding=NULL;
    next=NULL;}

  unsigned int stateWithoutAccessBit(){return state & (~Cell_Lock_Access_Bit);}

  void addPendBinding(Thread*,TaggedRef);

  TaggedRef getHead(){return head;}

  Site* getNext(){return next;}

  unsigned int getState(){return state;}

  TaggedRef getContents(){
    Assert(state & (Cell_Lock_Valid|Cell_Lock_Requested));
    return contents;}

  PendThread** getPendBase(){return &pending;}

  void gcCellSec();
  void exchange(Tertiary*,TaggedRef,TaggedRef,Thread*);
  void access(Tertiary*,TaggedRef);
  Bool cellRecovery(TaggedRef);
  Bool secReceiveRemoteRead(TaggedRef&);
  void secReceiveReadAns(TaggedRef);
  Bool secReceiveContents(TaggedRef,Site* &,TaggedRef &);
  Bool secForward(Site*,TaggedRef&);
  Bool threadIsPending(Thread* th);
};

class CellManager:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  CellSec *sec;
public:
  OZPRINT;

  NO_DEFAULT_CONSTRUCTORS2(CellManager);
  CellManager() : Tertiary(0,Co_Cell,Te_Manager) {} // hack

  CellSec* getSec(){return sec;}

  Chain *getChain() {return (Chain*) getPtr();}

  unsigned int getState(){return sec->state;}

  void initOnGlobalize(int index,Chain* ch,CellSec *secX);

  void setOwnCurrent();
  Bool isOwnCurrent();
  void init();
  Site* getCurrent();
  void localize();
  void gcCellManager();
  void tokenLost();
  PendThread* getPending(){return sec->pending;}
  PendBinding *getPendBinding(){return sec->pendBinding;}
};

class CellProxy:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  int holder;
public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS(CellProxy);

  CellProxy(int manager):Tertiary(manager,Co_Cell,Te_Proxy){  // on import
    holder = 0;}
};

class CellFrame:public Tertiary{
friend void ConstTerm::gcConstRecurse(void);
private:
  CellSec *sec;
public:
  OZPRINT;

  NO_DEFAULT_CONSTRUCTORS2(CellFrame);
  CellFrame() : Tertiary(0,Co_Cell,Te_Frame) {} // hack

  void setDumpBit(){sec->state |= Cell_Lock_Dump_Asked;}

  void resetDumpBit(){sec->state &= ~Cell_Lock_Dump_Asked;}

  Bool dumpCandidate(){
    if((sec->state & Cell_Lock_Valid)
       && (!(sec->state & Cell_Lock_Dump_Asked))){
      setDumpBit();
      return OK;}
    return NO;}

  Bool isAccessBit(){return sec->state & Cell_Lock_Access_Bit;}

  void setAccessBit(){sec->state |= Cell_Lock_Access_Bit;}

  void resetAccessBit(){sec->state &= (~Cell_Lock_Access_Bit);}

  unsigned int getState(){return sec->state;}



  void myStoreForward(void* forward){setPtr(forward);}

  void* getForward(){return getPtr();}

  CellSec* getSec(){return sec;}

  void convertToProxy(){
    setTertType(Te_Proxy);
    sec=NULL;}

  void convertFromProxy(){
    setTertType(Te_Frame);
    sec=new CellSec();}

  void gcCellFrame();

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
  NO_DEFAULT_CONSTRUCTORS(Port);
  Port(Board *b, TertType tt) : Tertiary(b,Co_Port,tt){}
};

class PortWithStream: public Port {
friend void ConstTerm::gcConstRecurse(void);
protected:
  TaggedRef strm;
public:
  NO_DEFAULT_CONSTRUCTORS(PortWithStream);
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
  NO_DEFAULT_CONSTRUCTORS2(PortManager);
  PortManager() : PortWithStream(0,0) {} // hack
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
  NO_DEFAULT_CONSTRUCTORS(PortLocal);
  PortLocal(Board *b, TaggedRef s) : PortWithStream(b,s) {};
};

class PortProxy: public Port {
friend void ConstTerm::gcConstRecurse(void);
public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS(PortProxy);
  PortProxy(int i): Port(0,Te_Proxy) { setIndex(i); }
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

class OzLock:public Tertiary{
public:
  NO_DEFAULT_CONSTRUCTORS(OzLock);
  OzLock(Board *b,TertType tt):Tertiary(b,Co_Lock,tt){}
  OzLock(int i,TertType tt):Tertiary(i,Co_Lock,tt){}
};

class LockLocal:public OzLock{
friend void ConstTerm::gcConstRecurse(void);
private:
  PendThread *pending;
public:
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS(LockLocal);
  LockLocal(Board *b) : OzLock(b,Te_Local){
    pending=NULL;
    setPtr(NULL);
    pending= NULL;}

  PendThread* getPending(){return pending;}
  void setPending(PendThread *pt){pending=pt;}
  PendThread** getPendBase(){return &pending;}

  Thread * getLocker(){return (Thread*) getPtr();}
  void setLocker(Thread *t){setPtr(t);}
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

class LockSec{
friend class LockFrame;
friend class LockManager;
friend class Chain;
private:
  unsigned int state;
  PendThread* pending;
  Site* next;
  Thread *locker;

public:
  NO_DEFAULT_CONSTRUCTORS2(LockSec);
  LockSec(Thread *t,PendThread *pt){ // on globalize
    state=Cell_Lock_Valid;
    pending=pt;
    locker=t;
    next=NULL; }

  LockSec(){ // on Proxy becoming Frame
    state=Cell_Lock_Invalid;
    locker=NULL;
    pending=NULL;
    next=NULL;}

  void setAccessBit(){state |= Cell_Lock_Access_Bit;}

  void resetAccessBit(){state &= ~Cell_Lock_Access_Bit;}

  Bool isPending(Thread *th);

  Thread* getLocker(){return locker;}

  Site* getNext(){return next;}

  unsigned int getState(){return state;}

  Bool secLockB(Thread*t){
    if(t==locker) return OK;
    if((locker==NULL) && (state==Cell_Lock_Valid)){
      Assert(pending==NULL);
      locker=t;
      return OK;}
    return NO;}

  PendThread** getPendBase(){return &pending;}

  void lockComplex(Thread* th,Tertiary*);
  void unlockComplex(Tertiary* );
  void unlockComplexB(Thread *);
  void unlockPending(Thread*);
  void gcLockSec();
  Bool secReceiveToken(Tertiary*,Site* &);
  Bool secForward(Site*);
  Bool lockRecovery();
  void makeRequested(){
    Assert(state==Cell_Lock_Invalid);
    state=Cell_Lock_Requested;}
  Bool threadIsPending(Thread* th);
};

class LockFrame:public OzLock{
friend void ConstTerm::gcConstRecurse(void);
private:
  LockSec *sec;
public:
  OZPRINT;

  NO_DEFAULT_CONSTRUCTORS2(LockFrame);
  LockFrame() : OzLock(0,(TertType)0) {} // hack

  Bool hasLock(Thread *t){if(t==sec->getLocker()) return TRUE;return FALSE;}

  Bool isAccessBit(){
    if(sec->state & Cell_Lock_Access_Bit) return TRUE;
    return FALSE;}

  void setAccessBit(){sec->setAccessBit();}

  void resetAccessBit(){sec->resetAccessBit();}

  void setDumpBit(){sec->state |= Cell_Lock_Dump_Asked;}

  void resetDumpBit(){sec->state &= ~Cell_Lock_Dump_Asked;}

  Bool dumpCandidate(){
    if((sec->state & Cell_Lock_Valid)
       && (!(sec->state & Cell_Lock_Dump_Asked))){
      setDumpBit();
      return OK;}
    return NO;}

  unsigned int getState(){return sec->state;}

  void myStoreForward(void* forward){setPtr(forward);}

  void* getForward(){return getPtr();}

  void convertToProxy(){
    setTertType(Te_Proxy);
    sec=NULL;}

  void convertFromProxy(){
    setTertType(Te_Frame);
    sec=new LockSec();}

  void lock(Thread *t){
    if(sec->secLockB(t)) return;
    sec->lockComplex(t,(Tertiary*) this);}

  Bool lockB(Thread *t){
    if(sec->secLockB(t)) return TRUE;
    sec->lockComplex(t,(Tertiary*) this);
    return FALSE;}

  void unlock(Thread *t){
    if (sec->locker!=t){
      sec->unlockPending(t);
      return;}
    sec->locker=NULL;
    if((sec->state==Cell_Lock_Valid) && (sec->pending==NULL)){
      return;}
    sec->unlockComplex((Tertiary*) this);}

  LockSec* getSec(){return sec;}

  void gcLockFrame();
};

class LockManager:public OzLock{
friend void ConstTerm::gcConstRecurse(void);
private:
  LockSec *sec;
public:
  OZPRINT;

  NO_DEFAULT_CONSTRUCTORS2(LockManager);
  LockManager() : OzLock(0,(TertType)0) {} // hack

  void initOnGlobalize(int index,Chain* ch,LockSec *secX);

  Bool hasLock(Thread *t){
    if(sec->locker==t) return TRUE;
    return FALSE;}

  void lock(Thread *t){
    if(sec->secLockB(t)) return;
    sec->lockComplex(t,(Tertiary*)this);}

  LockSec *getSec(){return sec;}

  Bool lockB(Thread *t){
    if(sec->secLockB(t)) return TRUE;
    sec->lockComplex(t,(Tertiary*)this);
    return FALSE;}

  void unlock(Thread *t){
    if (sec->getLocker()!=t){
      sec->unlockPending(t);
      return;}
    Assert(sec->state & Cell_Lock_Valid);
    sec->locker=NULL;
    if((sec->state==Cell_Lock_Valid) && sec->pending==NULL) return;
    sec->unlockComplex((Tertiary*) this);}

  Chain *getChain() {return (Chain*) getPtr();}

  PendThread* getPending(){return sec->pending;}

  void localize();
  void gcLockManager();
  void tokenLost();
  void setOwnCurrent();
  Bool isOwnCurrent();
  void init();
  Site* getCurrent();
  void probeFault(Site*, int);
};

class LockProxy:public OzLock{
friend void ConstTerm::gcConstRecurse(void);
private:
  int holder;
public:
  OZPRINT;

  NO_DEFAULT_CONSTRUCTORS(LockProxy);
  LockProxy(int manager):OzLock(manager,Te_Proxy){  // on import
    holder = 0;}

  void lock(Thread *);
  void unlock();
  void probeFault(Site*, int);
};



inline
Bool isLock(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType() == Co_Lock;
}

/*===================================================================
 *
 *=================================================================== */

char *toC(OZ_Term);
TaggedRef reverseC(TaggedRef l);
TaggedRef appendI(TaggedRef x,TaggedRef y);
Bool member(TaggedRef elem,TaggedRef list);
TaggedRef getUniqueName(const char *s);

/*===================================================================
 * Service Registry
 *=================================================================== */

extern OZ_Term service_registry;

extern OZ_Term service_get(OZ_Term);
inline OZ_Term service_get(char*s)
{
  return service_get(makeTaggedAtom(s));
}
extern void service_put(OZ_Term,OZ_Term);
inline void service_put(char*s,OZ_Term v)
{
  service_put(makeTaggedAtom(s),v);
}

#endif
