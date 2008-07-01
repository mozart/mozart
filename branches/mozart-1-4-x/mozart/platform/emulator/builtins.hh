/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#ifndef __BUILTINSH
#define __BUILTINSH

#include "value.hh"
#include "am.hh"

/********************************************************************
 * Macros
 ******************************************************************** */

#define NONVAR(X,term)				\
TaggedRef term = X;		        	\
{ DEREF_NONVAR(term);		        	\
  Assert(!oz_isRef(term));			\
  if (oz_isVarOrRef(term)) return SUSPEND;	\
}

// Suspend on free variables
#define SUSPEND_ON_NONKINDED_VAR(X,term)	\
TaggedRef term = X;				\
{ DEREF(term,_myTermPtr);			\
  if (oz_isNonKinded(term)) return SUSPEND;	\
}


#define OZ_DECLAREBI_USEINLINEREL1(Name,InlineName)	\
OZ_BI_define(Name,1,0)					\
{							\
  oz_declareIN(0,arg1);					\
  OZ_Return state = InlineName(arg1);			\
  if (state == SUSPEND)	{				\
    oz_suspendOnInArgs1;				\
  } else {						\
    return state;					\
  }							\
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEREL2(Name,InlineName)	\
OZ_BI_define(Name,2,0)					\
{							\
  oz_declareIN(0,arg0);					\
  oz_declareIN(1,arg1);					\
  OZ_Return state = InlineName(arg0,arg1);		\
  if (state == SUSPEND) {				\
    oz_suspendOnInArgs2;			        \
  } else {						\
    return state;					\
  }							\
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEREL3(Name,InlineName)	\
OZ_BI_define(Name,3,0)					\
{							\
  oz_declareIN(0,arg0);					\
  oz_declareIN(1,arg1);					\
  oz_declareIN(2,arg2);					\
  OZ_Return state = InlineName(arg0,arg1,arg2);		\
  if (state == SUSPEND) {				\
    oz_suspendOnInArgs3;			        \
  } else {						\
    return state;					\
  }							\
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEFUN1(Name,InlineName)	\
OZ_BI_define(Name,1,1)					\
{							\
  OZ_Term aux=0;					\
  oz_declareIN(0,arg1);					\
  OZ_Return state = InlineName(arg1,aux);		\
  OZ_result(aux);					\
  if (state==SUSPEND) {					\
    oz_suspendOnInArgs1;				\
  } else {						\
    return state;					\
  }							\
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEFUN3(Name,InlineName)	\
OZ_BI_define(Name,3,1)					\
{							\
  OZ_Term aux=0;					\
  oz_declareIN(0,arg0);					\
  oz_declareIN(1,arg1);					\
  oz_declareIN(2,arg2);					\
  OZ_Return state=InlineName(arg0,arg1,arg2,aux);	\
  OZ_result(aux);					\
  if (state==SUSPEND) {					\
    oz_suspendOnInArgs3;		                \
  } else {						\
    return state;					\
  }							\
} OZ_BI_end


#define OZ_DECLAREBI_USEINLINEFUN4(Name,InlineName)	\
OZ_BI_define(Name,4,1)					\
{							\
  OZ_Term aux=0;					\
  oz_declareIN(0,arg0);					\
  oz_declareIN(1,arg1);					\
  oz_declareIN(2,arg2);					\
  oz_declareIN(3,arg3);					\
  OZ_Return state=InlineName(arg0,arg1,arg2,arg3,aux);	\
  OZ_result(aux);					\
  if (state==SUSPEND) {					\
    oz_suspendOnInArgs4;		                \
  } else {						\
    return state;					\
  }							\
} OZ_BI_end


#define OZ_DECLAREBOOLFUN1(BIfun,irel)		\
OZ_BI_define(BIfun,1,1)				\
{						\
  OZ_Return r = irel(OZ_in(0));			\
  switch (r) {					\
  case PROCEED: OZ_RETURN(oz_true());		\
  case FAILED : OZ_RETURN(oz_false());		\
  case SUSPEND: oz_suspendOnInArgs1;		\
  default     : return r;			\
  }						\
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEFUN2(Name,InlineName)	\
OZ_BI_define(Name,2,1)					\
{							\
  OZ_Term aux=0;					\
  oz_declareIN(0,arg0);					\
  oz_declareIN(1,arg1);					\
  OZ_Return state=InlineName(arg0,arg1,aux);		\
  OZ_result(aux);					\
  if (state==SUSPEND) {					\
    oz_suspendOnInArgs2;				\
  } else {						\
    return state;					\
  }							\
} OZ_BI_end



/* -----------------------------------------------------------------------
 * argument declaration for builtins
 * -----------------------------------------------------------------------*/

#define oz_declareIN(ARG,VAR)			\
register OZ_Term VAR = OZ_in(ARG);		\

#define oz_declareDerefIN(ARG,VAR)		\
oz_declareIN(ARG,VAR);				\
DEREF(VAR,VAR ## Ptr);

#define oz_declareSafeDerefIN(ARG,VAR)		\
oz_declareIN(ARG,VAR);				\
VAR=oz_safeDeref(VAR);

#define oz_declareNonvarIN(ARG,VAR)		\
oz_declareDerefIN(ARG,VAR);			\
{						\
  Assert(!oz_isRef(VAR));			\
  if (oz_isVarOrRef(VAR)) {		        \
    oz_suspendOnPtr(VAR ## Ptr);		\
  }						\
}

#define oz_declareNonKindedIN(ARG,VAR)		\
oz_declareDerefIN(ARG,VAR);			\
{						\
  if (oz_isNonKinded(VAR)) {			\
    oz_suspendOnPtr(VAR ## Ptr);		\
  }						\
}

#define oz_declareTypeIN(ARG,VAR,TT,TYPE)	\
TT VAR;						\
{						\
  register OZ_Term _VAR = OZ_in(ARG);           \
  while (OK) {                                  \
    if (oz_is ## TYPE(_VAR)) {                  \
      VAR = oz_ ## TYPE ## ToC(_VAR);		\
      break;                                    \
    }                                           \
    if (oz_isRef(_VAR)) {                       \
      _VAR = * tagged2Ref(_VAR);                \
      continue;                                 \
    }                                           \
    Assert(!oz_isRef(_VAR));			\
    if (oz_isVarOrRef(_VAR)) {                  \
      oz_suspendOn(OZ_in(ARG));			\
    }                                           \
    oz_typeError(ARG, #TYPE);                   \
  }                                             \
}


#define oz_declareFloatIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,double,Float)
#define oz_declareAtomIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,const char*,Atom)
#define oz_declareDictionaryIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,OzDictionary*,Dictionary)
#define oz_declareSRecordIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,SRecord*,SRecord)
#define oz_declareSTupleIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,SRecord*,STuple)
#define oz_declareThreadIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,Thread*,Thread);

#define oz_declareThread(ARG,VAR)				\
 oz_declareTypeIN(ARG,VAR,Thread*,Thread);			\
 if ((VAR)->isDead())				\
   return oz_raise(E_ERROR,E_KERNEL,"deadThread",1,OZ_in(ARG));

#define oz_declareIntIN(ARG,VAR) \
int VAR;					\
{						\
  register OZ_Term _VAR = OZ_in(ARG);           \
  while (OK) {                                  \
    if (oz_isSmallInt(_VAR)) {                  \
      VAR = tagged2SmallInt(_VAR);              \
      break;                                    \
    }                                           \
    if (oz_isRef(_VAR)) {                       \
      _VAR = * tagged2Ref(_VAR);                \
      continue;                                 \
    }                                           \
    if (oz_isBigInt(_VAR)) {                    \
      VAR = tagged2BigInt(_VAR)->getInt();      \
      break;                                    \
    }                                           \
    Assert(!oz_isRef(_VAR));			\
    if (oz_isVarOrRef(_VAR)) {                  \
      oz_suspendOn(OZ_in(ARG));			\
    }                                           \
    oz_typeError(ARG, "Int");                   \
  }                                             \
}

#define oz_declareSmallIntIN(ARG,VAR) \
int VAR;					\
{						\
  register OZ_Term _VAR = OZ_in(ARG);           \
  while (OK) {                                  \
    if (oz_isSmallInt(_VAR)) {                  \
      VAR = tagged2SmallInt(_VAR);              \
      break;                                    \
    }                                           \
    if (oz_isRef(_VAR)) {                       \
      _VAR = * tagged2Ref(_VAR);                \
      continue;                                 \
    }                                           \
    Assert(!oz_isRef(_VAR));			\
    if (oz_isVarOrRef(_VAR)) {                  \
      oz_suspendOn(OZ_in(ARG));			\
    }                                           \
    oz_typeError(ARG, "Small Int");             \
  }                                             \
}

#define oz_declareBoolIN(ARG,VAR) \
Bool VAR;					\
{						\
  register OZ_Term _VAR = OZ_in(ARG);           \
  while (OK) {                                  \
    if (oz_isTrue(_VAR)) {                      \
      VAR = OK;                                 \
      break;                                    \
    }                                           \
    if (oz_isFalse(_VAR)) {                     \
      VAR = NO;                                 \
      break;                                    \
    }                                           \
    if (oz_isRef(_VAR)) {                       \
      _VAR = * tagged2Ref(_VAR);                \
      continue;                                 \
    }                                           \
    Assert(!oz_isRef(_VAR));			\
    if (oz_isVarOrRef(_VAR)) {                  \
      oz_suspendOn(OZ_in(ARG));			\
    }                                           \
    oz_typeError(ARG, "Bool");                  \
  }                                             \
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
  VAR = OZ_stringToC(_VAR1,0);					\
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
  VAR = OZ_virtualStringToC(_VAR1,0);		\
}


#define CheckLocalBoard(Object,Where);					\
  if (!oz_onToplevel() && !oz_isCurrentBoard(GETBOARD(Object))) {	\
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom(Where));	\
  }


#define OZ_getINDeref(N, V, VPTR)		\
  OZ_Term V = OZ_in(N);				\
  DEREF(V, VPTR);

/* -----------------------------------------------------------------------
 * suspend
 * -----------------------------------------------------------------------*/

#define oz_suspendOn(v)         return oz_addSuspendVarList(v)
#define oz_suspendOn2(v1,v2)    return oz_addSuspendVarList2(v1,v2)
#define oz_suspendOn3(v1,v2,v3) return oz_addSuspendVarList3(v1,v2,v3)
#define oz_suspendOn4(v1,v2,v3,v4) return oz_addSuspendVarList4(v1,v2,v3,v4)

#define oz_suspendOnPtr(vPtr)   return oz_addSuspendVarList(vPtr)

#define oz_suspendOnInArgs1     return oz_addSuspendInArgs1(_OZ_LOC)
#define oz_suspendOnInArgs2     return oz_addSuspendInArgs2(_OZ_LOC)
#define oz_suspendOnInArgs3     return oz_addSuspendInArgs3(_OZ_LOC)
#define oz_suspendOnInArgs4     return oz_addSuspendInArgs4(_OZ_LOC)

/* -----------------------------------------------------------------------
 * C <-> Oz conversions
 * -----------------------------------------------------------------------*/

#define oz_IntToC(v)        OZ_intToC(v)
#define oz_AtomToC(v)       OZ_atomToC(v)
#define oz_DictionaryToC(v) tagged2Dictionary(v)
#define oz_SRecordToC(v)    tagged2SRecord(v)
#define oz_STupleToC(v)     tagged2SRecord(v)
#define oz_BoolToC(t)       oz_isTrue(t)

/* -----------------------------------------------------------------------
 * exceptions
 * -----------------------------------------------------------------------*/

int  oz_raise(OZ_Term cat, OZ_Term key, const char * label, int arity, ...);
OZ_Return oz_typeErrorInternal(const int pos, const char * type);

#define oz_typeError(pos,type) return oz_typeErrorInternal(pos,type)

OZ_Return typeError(int pos, char *comment, char *typeString);

#define ExpectedTypes(S) char * __typeString = S;
#define TypeError(Pos, Comment) return typeError(Pos,Comment,__typeString);

#endif

