/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
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

#ifndef __BUILTINSH
#define __BUILTINSH


void initObjectBuiltins(void);

/********************************************************************
 * Macros
 ******************************************************************** */

#define NONVAR(X,term)                          \
TaggedRef term = X;                             \
{ DEREF(term,_myTermPtr,_myTag);                \
  if (isVariableTag(_myTag)) return SUSPEND;    \
}

// Suspend on free variables
#define SUSPEND_ON_FREE_VAR(X,term,tag)         \
TaggedRef term = X;                             \
TypeOfTerm tag;                                 \
{ DEREF(term,_myTermPtr,myTag);                 \
  tag = myTag;                                  \
  if (oz_isFree(term)) return SUSPEND;          \
}


#define OZ_DECLAREBI_USEINLINEREL1(Name,InlineName)     \
OZ_BI_define(Name,1,0)                                  \
{                                                       \
  oz_declareIN(0,arg1);                                 \
  OZ_Return state = InlineName(arg1);                   \
  if (state == SUSPEND) {                               \
    oz_suspendOn(arg1);                                 \
  } else {                                              \
    return state;                                       \
  }                                                     \
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEREL2(Name,InlineName)     \
OZ_BI_define(Name,2,0)                                  \
{                                                       \
  oz_declareIN(0,arg0);                                 \
  oz_declareIN(1,arg1);                                 \
  OZ_Return state = InlineName(arg0,arg1);              \
  if (state == SUSPEND) {                               \
    oz_suspendOn2(arg0,arg1);                           \
  } else {                                              \
    return state;                                       \
  }                                                     \
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEREL3(Name,InlineName)     \
OZ_BI_define(Name,3,0)                                  \
{                                                       \
  oz_declareIN(0,arg0);                                 \
  oz_declareIN(1,arg1);                                 \
  oz_declareIN(2,arg2);                                 \
  OZ_Return state = InlineName(arg0,arg1,arg2);         \
  if (state == SUSPEND) {                               \
    oz_suspendOn3(arg0,arg1,arg2);                      \
  } else {                                              \
    return state;                                       \
  }                                                     \
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEFUN1(Name,InlineName)     \
OZ_BI_define(Name,1,1)                                  \
{                                                       \
  OZ_Term aux=0;                                        \
  oz_declareIN(0,arg1);                                 \
  OZ_Return state = InlineName(arg1,aux);               \
  OZ_result(aux);                                       \
  if (state==SUSPEND) {                                 \
    oz_suspendOn(arg1);                                 \
  } else {                                              \
    return state;                                       \
  }                                                     \
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEFUN3(Name,InlineName)     \
OZ_BI_define(Name,3,1)                                  \
{                                                       \
  OZ_Term aux=0;                                        \
  oz_declareIN(0,arg0);                                 \
  oz_declareIN(1,arg1);                                 \
  oz_declareIN(2,arg2);                                 \
  OZ_Return state=InlineName(arg0,arg1,arg2,aux);       \
  OZ_result(aux);                                       \
  if (state==SUSPEND) {                                 \
    oz_suspendOn3(arg0,arg1,arg2);                      \
  } else {                                              \
    return state;                                       \
  }                                                     \
} OZ_BI_end


#define OZ_DECLAREBOOLFUN1(BIfun,irel)          \
OZ_BI_define(BIfun,1,1)                         \
{                                               \
  OZ_Return r = irel(OZ_in(0));                 \
  switch (r) {                                  \
  case PROCEED: OZ_RETURN(NameTrue);            \
  case FAILED : OZ_RETURN(NameFalse);           \
  case SUSPEND: oz_suspendOn(OZ_in(0));         \
  default     : return r;                       \
  }                                             \
} OZ_BI_end

#define OZ_DECLAREBI_USEINLINEFUN2(Name,InlineName)     \
OZ_BI_define(Name,2,1)                                  \
{                                                       \
  OZ_Term aux=0;                                        \
  oz_declareIN(0,arg0);                                 \
  oz_declareIN(1,arg1);                                 \
  OZ_Return state=InlineName(arg0,arg1,aux);            \
  OZ_result(aux);                                       \
  if (state==SUSPEND) {                                 \
    oz_suspendOn2(arg0,arg1);                           \
  } else {                                              \
    return state;                                       \
  }                                                     \
} OZ_BI_end



/* -----------------------------------------------------------------------
 * argument declaration for builtins
 * -----------------------------------------------------------------------*/

#define oz_declareArg(ARG,VAR)                  \
register OZ_Term VAR = OZ_args[ARG];            \


#define oz_declareIN(ARG,VAR)                   \
register OZ_Term VAR = OZ_in(ARG);              \


#define oz_declareDerefArg(ARG,VAR)             \
oz_declareArg(ARG,VAR);                         \
DEREF(VAR,VAR ## Ptr,VAR ## Tag);               \


#define oz_declareDerefIN(ARG,VAR)              \
oz_declareIN(ARG,VAR);                          \
DEREF(VAR,VAR ## Ptr,VAR ## Tag);               \


#define oz_declareNonvarArg(ARG,VAR)            \
oz_declareDerefArg(ARG,VAR);                    \
{                                               \
  if (oz_isVariable(VAR)) {                     \
    oz_suspendOnPtr(VAR ## Ptr);                \
  }                                             \
}

#define oz_declareNonvarIN(ARG,VAR)             \
oz_declareDerefIN(ARG,VAR);                     \
{                                               \
  if (oz_isVariable(VAR)) {                     \
    oz_suspendOnPtr(VAR ## Ptr);                \
  }                                             \
}

#define oz_declareTypeArg(ARG,VAR,TT,TYPE)      \
TT VAR;                                         \
{                                               \
  oz_declareNonvarArg(ARG,_VAR);                \
  if (!oz_is ## TYPE(_VAR)) {                   \
    oz_typeError(ARG, #TYPE);                   \
  } else {                                      \
    VAR = oz_ ## TYPE ## ToC(_VAR);             \
  }                                             \
}

#define oz_declareTypeIN(ARG,VAR,TT,TYPE)       \
TT VAR;                                         \
{                                               \
  oz_declareNonvarIN(ARG,_VAR);         \
  if (!oz_is ## TYPE(_VAR)) {                   \
    oz_typeError(ARG, #TYPE);                   \
  } else {                                      \
    VAR = oz_ ## TYPE ## ToC(_VAR);             \
  }                                             \
}

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
#define oz_declareSRecordIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,SRecord*,SRecord)
#define oz_declareSTupleIN(ARG,VAR) \
 oz_declareTypeIN(ARG,VAR,SRecord*,STuple)


#define oz_declareProperStringArg(ARG,VAR)                      \
char *VAR;                                                      \
{                                                               \
  oz_declareArg(ARG,_VAR1);                                     \
  OZ_Term _VAR2;                                                \
  if (!OZ_isProperString(_VAR1,&_VAR2)) {                       \
    if (!_VAR2) {                                               \
      oz_typeError(ARG,"ProperString");                         \
    } else {                                                    \
      oz_suspendOn(_VAR2);                                      \
    }                                                           \
  }                                                             \
  VAR = OZ_stringToC(_VAR1);                                    \
}

#define oz_declareProperStringIN(ARG,VAR)                       \
char *VAR;                                                      \
{                                                               \
  oz_declareIN(ARG,_VAR1);                                      \
  OZ_Term _VAR2;                                                \
  if (!OZ_isProperString(_VAR1,&_VAR2)) {                       \
    if (!_VAR2) {                                               \
      oz_typeError(ARG,"ProperString");                         \
    } else {                                                    \
      oz_suspendOn(_VAR2);                                      \
    }                                                           \
  }                                                             \
  VAR = OZ_stringToC(_VAR1);                                    \
}

#define oz_declareVirtualStringArg(ARG,VAR)     \
char *VAR;                                      \
{                                               \
  oz_declareArg(ARG,_VAR1);                     \
  OZ_Term _VAR2;                                \
  if (!OZ_isVirtualString(_VAR1,&_VAR2)) {      \
    if (!_VAR2) {                               \
      oz_typeError(ARG,"VirtualString");        \
    } else {                                    \
      oz_suspendOn(_VAR2);                      \
    }                                           \
  }                                             \
  VAR = OZ_virtualStringToC(_VAR1);             \
}

#define oz_declareVirtualStringIN(ARG,VAR)      \
char *VAR;                                      \
{                                               \
  oz_declareIN(ARG,_VAR1);                      \
  OZ_Term _VAR2;                                \
  if (!OZ_isVirtualString(_VAR1,&_VAR2)) {      \
    if (!_VAR2) {                               \
      oz_typeError(ARG,"VirtualString");        \
    } else {                                    \
      oz_suspendOn(_VAR2);                      \
    }                                           \
  }                                             \
  VAR = OZ_virtualStringToC(_VAR1);             \
}

#endif
