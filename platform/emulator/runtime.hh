/* -----------------------------------------------------------------------
 *  (c) Perdio Project, DFKI & SICS
 *  Universit"at des Saarlandes
 *    Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
 *  SICS
 *    Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
 *  Author: mehl
 *
 *  internal interface to AMOZ
 * -----------------------------------------------------------------------*/

#ifndef FOREIGN_HH
#define FOREIGN_HH

#include "am.hh"

/* -----------------------------------------------------------------------
 * equality, unification
 * -----------------------------------------------------------------------*/

// pointer equality!
#define oz_eq(x,y) termEq((x),(y))

#define oz_unify(t1,t2)      (am.fastUnify((t1),(t2),OK) ? PROCEED : FAILED)

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

#define oz_newName() makeTaggedLiteral(Name::newName(am.currentBoard))

#define oz_newPort(val) \
  makeTaggedConst(new PortWithStream(am.currentBoard, (val)))

#define oz_sendPort(p,v) sendPort(p,v)

#define oz_newCell(val) makeTaggedConst(new CellLocal(am.currentBoard, (val)))
  // access, assign

#define oz_float(f)     makeTaggedFloat((f))
#define oz_int(i)       makeInt((i))

inline
OZ_Term oz_newChunk(OZ_Term val)
{
  Assert(val==deref(val));
  Assert(isRecord(val));
  return makeTaggedConst(new SChunk(am.currentBoard, val));
}

#define oz_newVariable() makeTaggedRef(newTaggedUVar(am.currentBoard))

// stop thread: {Wait v}
#define oz_currentThread        am.currentThread
#define oz_stop(th)   am.stopThread(th);
#define oz_resume(th) am.resumeThread(th);

#define return_stop   return BI_PREEMPT;

/* -----------------------------------------------------------------------
 * # - tuples
 * -----------------------------------------------------------------------*/

#define oz_cons(a,b) cons(a,b)
#define oz_nil()  nil()

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


/* -----------------------------------------------------------------------
 * suspend
 * -----------------------------------------------------------------------*/

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
#define oz_declareAtomArg(ARG,VAR) oz_declareTypeArg(ARG,VAR,char*,Atom)
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

#define oz_raise am.raise

#define oz_typeError(pos,type)			\
{						\
  (void) oz_raise(E_ERROR,E_KERNEL,		\
		  "type",5,NameUnit,NameUnit,	\
		  OZ_atom(type),		\
		  OZ_int(pos+1),		\
		  OZ_string(""));		\
  return BI_TYPE_ERROR;				\
}

/* -----------------------------------------------------------------------
 * TODO
 * -----------------------------------------------------------------------*/

/*

#define am DontUseAM
deref

*/

#endif
