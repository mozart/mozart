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
 * Spaces
 * -----------------------------------------------------------------------*/

inline
int oz_isRootBoard(Board *bb) { return bb==oz_rootBoard(); }

inline
Board *oz_rootBoard() { return am._rootBoard; }

inline
Board *oz_currentBoard() { return am._currentBoard; }

#define CheckLocalBoard(Object,Where);					\
  if (!am.onToplevel() && !am.isCurrentBoard(GETBOARD(Object))) {	\
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom(Where));	\
  }

/* -----------------------------------------------------------------------
 * equality, unification
 * -----------------------------------------------------------------------*/

// pointer equality!
// see tagged.hh: oz_eq(x,y)

OZ_Return oz_unify(OZ_Term t1, OZ_Term t2, ByteCode *scp=0);
void oz_bind(OZ_Term *varPtr, OZ_Term var, OZ_Term term);
void oz_bind_global(OZ_Term var, OZ_Term term);

inline
void oz_bindToNonvar(OZ_Term *varPtr, OZ_Term var,
		     OZ_Term a, ByteCode *scp=0)
{
  // most probable case first: local UVar
  // if (isUVar(var) && isCurrentBoard(tagged2VarHome(var))) {
  // more efficient:
  if (am.currentUVarPrototypeEq(var) && scp==0) {
    doBind(varPtr,a);
  } else {
    oz_bind(varPtr,var,a);
  }
}

/* -----------------------------------------------------------------------
 * values
 * -----------------------------------------------------------------------*/

#define oz_true() NameTrue
#define oz_false() NameFalse
#define oz_unit() NameUnit

#define oz_atom(s) makeTaggedAtom(s)

#define oz_newName() makeTaggedLiteral(Name::newName(oz_currentBoard()))

#define oz_newPort(val) \
  makeTaggedConst(new PortWithStream(oz_currentBoard(), (val)))

#define oz_sendPort(p,v) sendPort(p,v)

#define oz_newCell(val) makeTaggedConst(new CellLocal(oz_currentBoard(), (val)))
  // access, assign

#define oz_float(f)       newTaggedFloat((f))
#define oz_int(i)         makeInt((i))

// see value.{hh,cc}
// OZ_Term oz_unsignedInt(unsigned int i);

OZ_Term oz_long(long i);
OZ_Term oz_unsignedLong(unsigned long i);

#define oz_string(s)      OZ_string(s)

inline
OZ_Term oz_newChunk(OZ_Term val)
{
  Assert(val==oz_deref(val));
  Assert(oz_isRecord(val));
  return makeTaggedConst(new SChunk(oz_currentBoard(), val));
}

#define oz_newVar(bb)            makeTaggedRef(newTaggedUVar(bb))
#define oz_newVariable()         oz_newVar(oz_currentBoard())
#define oz_newToplevelVariable() oz_newVar(oz_rootBoard())

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
  while (oz_isCons(l)) {
    len++;
    if (checkChar) {
      OZ_Term h = head(l);
      DerefReturnVar(h);
      if (!oz_isSmallInt(h)) return NameFalse;
      int i=smallIntValue(h);
      if (i<0 || i>255) return NameFalse;
      if (checkChar>1 && i==0) return NameFalse;
    }
    l = tail(l);
    DerefReturnVar(l);
    if (l==old) return NameFalse; // cyclic
    if (updateF) {
      old=oz_deref(tail(old));
    }
    updateF=1-updateF;
  }
  if (oz_isNil(l)) {
    return oz_int(len);
  } else {
    return NameFalse;
  }
}

inline
int oz_isPair(OZ_Term term)
{
  if (oz_isLiteral(term)) return literalEq(term,AtomPair);
  if (!oz_isSRecord(term)) return 0;
  SRecord *sr = tagged2SRecord(term);
  if (!sr->isTuple()) return 0;
  return literalEq(sr->getLabel(),AtomPair);
}

inline
int oz_isPair2(OZ_Term term)
{
  if (!oz_isSRecord(term)) return 0;
  SRecord *sr = tagged2SRecord(term);
  if (!sr->isTuple()) return 0;
  if (!literalEq(sr->getLabel(),AtomPair)) return 0;
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
  Assert(oz_isVariable(v));				\
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
  if (oz_isVariable(v1)) {				\
    am.addSuspendVarList(v1Ptr);		\
  }						\
  OZ_Term v2=v2in;				\
  DEREF(v2,v2Ptr,___2); 			\
  if (oz_isVariable(v2)) {				\
    am.addSuspendVarList(v2Ptr);		\
  }						\
  return SUSPEND;				\
}

#define oz_suspendOn3(v1in,v2in,v3in) {		\
  OZ_Term v1=v1in;				\
  DEREF(v1,v1Ptr,___1);				\
  if (oz_isVariable(v1)) {				\
    am.addSuspendVarList(v1Ptr);		\
  }						\
  OZ_Term v2=v2in;				\
  DEREF(v2,v2Ptr,___2);				\
  if (oz_isVariable(v2)) {				\
    am.addSuspendVarList(v2Ptr);		\
  }						\
  OZ_Term v3=v3in;				\
  DEREF(v3,v3Ptr,___3);				\
  if (oz_isVariable(v3)) {				\
    am.addSuspendVarList(v3Ptr);		\
  }						\
  return SUSPEND;				\
}

/* -----------------------------------------------------------------------
 * control variables
 * -----------------------------------------------------------------------*/

#define ControlVarNew(var,home)			\
OZ_Term var = oz_newVar(home);			\
am.addSuspendVarList(var);

#define _controlVarUnify(var,val) oz_bind_global(var,val)

#define ControlVarResume(var)			\
_controlVarUnify(var,NameUnit)


#define ControlVarRaise(var,exc) 			\
_controlVarUnify(var,OZ_mkTuple(AtomException,1,exc))

#define ControlVarUnify(var,A,B) 			\
_controlVarUnify(var,OZ_mkTuple(AtomUnify,2,A,B))

#define ControlVarApply(var,P,Args)			\
_controlVarUnify(var,OZ_mkTuple(AtomApply,2,P,Args))

#define ControlVarApplyList(var,PairList)			\
_controlVarUnify(var,OZ_mkTuple(AtomApplyList,1,PairList))


OZ_Return suspendOnControlVar();

#define SuspendOnControlVar			\
  return suspendOnControlVar();




/* -----------------------------------------------------------------------
 * argument declaration for builtins
 * -----------------------------------------------------------------------*/

#define oz_declareArg(ARG,VAR)			\
register OZ_Term VAR = OZ_args[ARG];		\


#define oz_declareIN(ARG,VAR)			\
register OZ_Term VAR = OZ_in(ARG);		\


#define oz_declareDerefArg(ARG,VAR)		\
oz_declareArg(ARG,VAR);				\
DEREF(VAR,VAR ## Ptr,VAR ## Tag);		\


#define oz_declareDerefIN(ARG,VAR)		\
oz_declareIN(ARG,VAR);				\
DEREF(VAR,VAR ## Ptr,VAR ## Tag);		\


#define oz_declareNonvarArg(ARG,VAR)		\
oz_declareDerefArg(ARG,VAR);			\
{						\
  if (oz_isVariable(VAR)) {			\
    oz_suspendOnPtr(VAR ## Ptr);		\
  }						\
}

#define oz_declareNonvarIN(ARG,VAR)		\
oz_declareDerefIN(ARG,VAR);			\
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

#define oz_declareTypeIN(ARG,VAR,TT,TYPE)	\
TT VAR;						\
{						\
  oz_declareNonvarIN(ARG,_VAR);		\
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

#define oz_declareIntIN(ARG,VAR) oz_declareTypeIN(ARG,VAR,int,Int)
#define oz_declareFloatIN(ARG,VAR) oz_declareTypeIN(ARG,VAR,double,Float)
#define oz_declareAtomIN(ARG,VAR) oz_declareTypeIN(ARG,VAR,const char*,Atom)
#define oz_declareThreadIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,Thread*,Thread)
#define oz_declareDictionaryIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,OzDictionary*,Dictionary)


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

#define oz_declareProperStringIN(ARG,VAR)			\
char *VAR;							\
{								\
  oz_declareIN(ARG,_VAR1);					\
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

#define oz_declareVirtualStringIN(ARG,VAR)	\
char *VAR;					\
{						\
  oz_declareIN(ARG,_VAR1);			\
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

OZ_Return oz_bi_wrapper(Builtin *bi,OZ_Term *X);

#ifdef DEBUG_CHECK
extern int checkBIArity(OZ_CFun f);
#endif


/* -----------------------------------------------------------------------
 * BuiltinTab
 * -----------------------------------------------------------------------*/

// specification for builtins
struct BIspec {
  char *name;
  int inArity;
  int outArity;
  OZ_CFun fun;
  Bool native;
};


class BuiltinTab : public HashTable {
public:
  BuiltinTab(int sz) : HashTable(HT_CHARKEY,sz) {};
  unsigned memRequired(void) {
    return HashTable::memRequired(sizeof(Builtin));
  }
  const char * getName(void * fp) {
    HashNode * hn = getFirst();
    for (; hn != NULL; hn = getNext(hn)) {
      Builtin * abit = (Builtin *) hn->value;
      if (abit->getFun() == (OZ_CFun) fp)
	return hn->key.fstr;
    }
    return "???";
  }
  Builtin * getEntry(void * fp) {
    HashNode * hn = getFirst();
    for (; hn != NULL; hn = getNext(hn)) {
      Builtin * abit = (Builtin *) hn->value;
      if (abit->getFun() == (OZ_CFun) fp)
	return abit;
    }
    return (Builtin *) NULL;
  }

  Builtin *find(const char *name) {
    return (Builtin*) htFind(name);
  }
};

extern BuiltinTab builtinTab;

// (see builtins.hh)
Builtin *BIadd(const char *name,int inArity,int outArity, OZ_CFun fun,
	       Bool native);
void BIaddSpec(BIspec *spec); // add specification to builtin table


/* -----------------------------------------------------------------------
 * Threads
 * -----------------------------------------------------------------------*/

inline
Thread *oz_newSuspendedThread()
{
  return am.mkSuspendedThread(oz_currentBoard(), DEFAULT_PRIORITY);
}

inline
Thread *oz_newRunnableThread(int prio=DEFAULT_PRIORITY)
{
  Thread *tt = am.mkRunnableThreadOPT(prio, oz_currentBoard());
  am.scheduleThread(tt);
  return tt;
}

/* -----------------------------------------------------------------------
 * TODO
 * -----------------------------------------------------------------------*/

/*

#define am DontUseAM
deref

*/

#endif
