/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997)
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

// internal interface to AMOZ

#ifndef RUNTIME_HH
#define RUNTIME_HH

#include "am.hh"

/* -----------------------------------------------------------------------
 * equality, unification
 * -----------------------------------------------------------------------*/

// pointer equality!
#define oz_eq(x,y) termEq((x),(y))

#define oz_unify(t1,t2)      (am.fastUnify((t1),(t2),0) ? PROCEED : FAILED)

#define oz_unifyFloat(t1,f)  oz_unify((t1), oz_float(f))
#define oz_unifyInt(t1,i)    oz_unify((t1), oz_int(i))
#define oz_unifyAtom(t1,s)   oz_unify((t1), oz_atom(s))

/* -----------------------------------------------------------------------
 * values
 * -----------------------------------------------------------------------*/

#define oz_isVariable(t) isAnyVar(t)
#define oz_isInt(t) isInt(t)
#define oz_isFloat(t) isFloat(t)
#define oz_isAtom(t) isAtom(t)
#define oz_isDictionary(t) isDictionary(t)
#define oz_isThread(t) isThread(t)
#define oz_isProcedure(t) isProcedure(t)

#define oz_atom(s) makeTaggedAtom(s)

#define oz_newName() makeTaggedLiteral(Name::newName(am.currentBoard()))

#define oz_newPort(val) \
  makeTaggedConst(new PortWithStream(am.currentBoard(), (val)))

#define oz_sendPort(p,v) sendPort(p,v)

#define oz_newCell(val) makeTaggedConst(new CellLocal(am.currentBoard(), (val)))
  // access, assign

#define oz_float(f)       newTaggedFloat((f))
#define oz_int(i)         makeInt((i))
#define oz_unsignedint(i) makeUnsignedInt((i))

inline
OZ_Term oz_newChunk(OZ_Term val)
{
  Assert(val==deref(val));
  Assert(isRecord(val));
  return makeTaggedConst(new SChunk(am.currentBoard(), val));
}

#define oz_newVariable() makeTaggedRef(newTaggedUVar(am.currentBoard()))

// stop thread: {Wait v}
void oz_suspendOnNet(Thread *th);
void oz_resumeFromNet(Thread *th);

/* -----------------------------------------------------------------------
 * # - tuples
 * -----------------------------------------------------------------------*/

#define oz_cons(a,b) cons(a,b)
#define oz_nil()  nil()

/*
 * list checking
 *   checkChar:
 *     0 = any list
 *     1 = list of char
 *     2 = list of char != 0
 * return
 *     OZ_true
 *     OZ_false
 *     var
 */

inline
OZ_Term oz_isList(OZ_Term l, int checkChar=0)
{
  DerefReturnVar(l);
  OZ_Term old = l;
  Bool updateF = 0;
  int len = 0;
  while (isCons(l)) {
    len++;
    if (checkChar) {
      OZ_Term h = head(l);
      DerefReturnVar(h);
      if (!isSmallInt(h)) return NameFalse;
      int i=smallIntValue(h);
      if (i<0 || i>255) return NameFalse;
      if (checkChar>1 && i==0) return NameFalse;
    }
    l = tail(l);
    DerefReturnVar(l);
    if (l==old) return NameFalse; // cyclic
    if (updateF) {
      old=deref(tail(old));
    }
    updateF=1-updateF;
  }
  if (isNil(l)) {
    return oz_int(len);
  } else {
    return NameFalse;
  }
}

inline
int oz_isPair(OZ_Term term)
{
  if (isLiteral(term)) return literalEq(term,AtomPair);
  if (!isSRecord(term)) return 0;
  SRecord *sr = tagged2SRecord(term);
  if (!sr->isTuple()) return 0;
  return literalEq(sr->getLabel(),AtomPair);
}

inline
int oz_isPair2(OZ_Term term)
{
  if (!isSRecord(term)) return 0;
  SRecord *sr = tagged2SRecord(term);
  if (!sr->isTuple()) return 0;
  if (!literalEq(sr->getLabel(),AtomPair)) return 0;
  return sr->getWidth()==2;
}

inline
TaggedRef oz_left(TaggedRef pair)
{
  Assert(oz_isPair2(pair));
  return tagged2SRecord(pair)->getArg(0);
}

inline
TaggedRef oz_right(TaggedRef pair)
{
  Assert(oz_isPair2(pair));
  return tagged2SRecord(pair)->getArg(1);
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

inline
Arity *oz_makeArity(OZ_Term list)
{
  list=packsort(list);
  if (!list) return 0;
  return aritytable.find(list);
}

/* -----------------------------------------------------------------------
 * suspend
 * -----------------------------------------------------------------------*/

#define oz_suspendOnVar(vin) {			\
  am.addSuspendVarList(vin);			\
  return SUSPEND;				\
}

#define oz_suspendOn(vin) {			\
  OZ_Term v=vin;				\
  DEREF(v,vPtr,___1);				\
  Assert(isAnyVar(v));				\
  am.addSuspendVarList(vPtr);			\
  return SUSPEND;				\
}

#define oz_suspendOnPtr(vPtr) {			\
  am.addSuspendVarList(vPtr);			\
  return SUSPEND;				\
}

#define oz_suspendOn2(v1in,v2in) {		\
  OZ_Term v1=v1in;				\
  DEREF(v1,v1Ptr,___1); 			\
  if (isAnyVar(v1)) {				\
    am.addSuspendVarList(v1Ptr);		\
  }						\
  OZ_Term v2=v2in;				\
  DEREF(v2,v2Ptr,___2); 			\
  if (isAnyVar(v2)) {				\
    am.addSuspendVarList(v2Ptr);		\
  }						\
  return SUSPEND;				\
}

#define oz_suspendOn3(v1in,v2in,v3in) {		\
  OZ_Term v1=v1in;				\
  DEREF(v1,v1Ptr,___1);				\
  if (isAnyVar(v1)) {				\
    am.addSuspendVarList(v1Ptr);		\
  }						\
  OZ_Term v2=v2in;				\
  DEREF(v2,v2Ptr,___2);				\
  if (isAnyVar(v2)) {				\
    am.addSuspendVarList(v2Ptr);		\
  }						\
  OZ_Term v3=v3in;				\
  DEREF(v3,v3Ptr,___3);				\
  if (isAnyVar(v3)) {				\
    am.addSuspendVarList(v3Ptr);		\
  }						\
  return SUSPEND;				\
}

/* -----------------------------------------------------------------------
 * argument declaration for builtins
 * -----------------------------------------------------------------------*/

#define oz_declareArg(ARG,VAR)			\
register OZ_Term VAR = OZ_args[ARG];		\


#define oz_declareDerefArg(ARG,VAR)		\
oz_declareArg(ARG,VAR);				\
DEREF(VAR,VAR ## Ptr,VAR ## Tag);		\


#define oz_declareNonvarArg(ARG,VAR)		\
oz_declareDerefArg(ARG,VAR);			\
{						\
  if (oz_isVariable(VAR)) {			\
    oz_suspendOnPtr(VAR ## Ptr);		\
  }						\
}

#define oz_declareTypeArg(ARG,VAR,TT,TYPE)	\
TT VAR;						\
{						\
  oz_declareNonvarArg(ARG,_VAR);		\
  if (!oz_is ## TYPE(_VAR)) {			\
    oz_typeError(ARG, #TYPE);			\
  } else {					\
    VAR = oz_ ## TYPE ## ToC(_VAR);		\
  }						\
}

#define oz_IntToC(v) OZ_intToC(v)
#define oz_AtomToC(v) OZ_atomToC(v)
#define oz_ThreadToC(v) tagged2Thread(v)
#define oz_DictionaryToC(v) tagged2Dictionary(v)

#define oz_declareIntArg(ARG,VAR) oz_declareTypeArg(ARG,VAR,int,Int)
#define oz_declareFloatArg(ARG,VAR) oz_declareTypeArg(ARG,VAR,double,Float)
#define oz_declareAtomArg(ARG,VAR) oz_declareTypeArg(ARG,VAR,const char*,Atom)
#define oz_declareThreadArg(ARG,VAR) \
 oz_declareTypeArg(ARG,VAR,Thread*,Thread)
#define oz_declareDictionaryArg(ARG,VAR) \
 oz_declareTypeArg(ARG,VAR,OzDictionary*,Dictionary)


#define oz_declareProperStringArg(ARG,VAR)			\
char *VAR;							\
{								\
  oz_declareArg(ARG,_VAR1);					\
  OZ_Term _VAR2;						\
  if (!OZ_isProperString(_VAR1,&_VAR2)) {			\
    if (!_VAR2) {						\
      oz_typeError(ARG,"ProperString");				\
    } else {							\
      oz_suspendOn(_VAR2);					\
    }								\
  }								\
  VAR = OZ_stringToC(_VAR1);					\
}

#define oz_declareVirtualStringArg(ARG,VAR)	\
char *VAR;					\
{						\
  oz_declareArg(ARG,_VAR1);			\
  OZ_Term _VAR2;				\
  if (!OZ_isVirtualString(_VAR1,&_VAR2)) {	\
    if (!_VAR2) {				\
      oz_typeError(ARG,"VirtualString");	\
    } else {					\
      oz_suspendOn(_VAR2);			\
    }						\
  }						\
  VAR = OZ_virtualStringToC(_VAR1);		\
}

/* -----------------------------------------------------------------------
 * exceptions
 * -----------------------------------------------------------------------*/

int oz_raise(OZ_Term cat, OZ_Term key, char *label, int arity, ...);

#define oz_typeError(pos,type)			\
{						\
  (void) oz_raise(E_ERROR,E_KERNEL,		\
		  "type",5,NameUnit,NameUnit,	\
		  OZ_atom(type),		\
		  OZ_int(pos+1),		\
		  OZ_string(""));		\
  return BI_TYPE_ERROR;				\
}

OZ_Return typeError(int pos, char *comment, char *typeString);

#define ExpectedTypes(S) char * __typeString = S;
#define TypeError(Pos, Comment) return typeError(Pos,Comment,__typeString);

/* -----------------------------------------------------------------------
 * MISC
 * -----------------------------------------------------------------------*/

OZ_Term oz_getLocation(Board *bb);

/* -----------------------------------------------------------------------
 * BuiltinTab
 * -----------------------------------------------------------------------*/

// specification for builtins
struct BIspec {
  char *name;
  int arity;
  OZ_CFun fun;
  IFOR ifun;
};


class BuiltinTab : public HashTable {
public:
  BuiltinTab(int sz) : HashTable(HT_CHARKEY,sz) {};
  unsigned memRequired(void) {
    return HashTable::memRequired(sizeof(BuiltinTabEntry));
  }
  const char * getName(void * fp) {
    HashNode * hn = getFirst();
    for (; hn != NULL; hn = getNext(hn)) {
      BuiltinTabEntry * abit = (BuiltinTabEntry *) hn->value;
      if (abit->getInlineFun() == (IFOR) fp ||
	  abit->getFun() == (OZ_CFun) fp)
	return hn->key.fstr;
    }
    return "???";
  }
  BuiltinTabEntry * getEntry(void * fp) {
    HashNode * hn = getFirst();
    for (; hn != NULL; hn = getNext(hn)) {
      BuiltinTabEntry * abit = (BuiltinTabEntry *) hn->value;
      if (abit->getInlineFun() == (IFOR) fp ||
	  abit->getFun() == (OZ_CFun) fp)
	return abit;
    }
    return (BuiltinTabEntry *) NULL;
  }

  BuiltinTabEntry *find(const char *name) {
    return (BuiltinTabEntry*) htFind(name);
  }
};

extern BuiltinTab builtinTab;

// (see builtins.hh)
BuiltinTabEntry *BIadd(const char *name,int arity,OZ_CFun fun,
		       IFOR infun=(IFOR) NULL);
void BIaddSpec(BIspec *spec); // add specification to builtin table


/* -----------------------------------------------------------------------
 * TODO
 * -----------------------------------------------------------------------*/

/*

#define am DontUseAM
deref

*/

#endif
