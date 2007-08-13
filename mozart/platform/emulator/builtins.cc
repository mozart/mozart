/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *    Fred Spiessens (fsp@info.ucl.ac.be)
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Michael Mehl, 1997,1998
 *    Kostja Popow, 1997
 *    Ralf Scheidhauer, 1997
 *    Christian Schulte, 1997
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

#include "wsock.hh"

#include "builtins.hh"
#include "site.hh"
#include "gname.hh"

#include "codearea.hh"
#include "thr_int.hh"
#include "debug.hh"
#include "iso-ctype.hh"
#include "var_base.hh"
#include "var_ext.hh"
#include "var_of.hh"
#include "var_readonly.hh"
#include "mozart_cpi.hh"
#include "dictionary.hh"
#include "dpInterface.hh"
#include "bytedata.hh"
#include "atoms.hh"

#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>


#include "GeVar.hh"


/********************************************************************
 * Type tests
 ******************************************************************** */

OZ_BI_define(BIwait,1,0)
{
  oz_declareNonvarIN(0, val);
  return PROCEED;
} OZ_BI_end

// suspends quietly until the argument is determined or failed
OZ_BI_define(BIwaitQuiet,1,0)
{
  oz_declareDerefIN(0,v);
  Assert(!oz_isRef(v));
  if (oz_isVarOrRef(v)) {
    if (oz_isFailed(v)) return PROCEED;
    return oz_var_addQuietSusp(vPtr, oz_currentThread());
  }
  return PROCEED;
} OZ_BI_end

// suspends until the argument becomes needed
OZ_BI_define(BIwaitNeeded,1,0)
{
  oz_declareDerefIN(0,v);
  Assert(!oz_isRef(v));

  if (oz_isNeeded(v)) {
    return PROCEED;
  } else {
    return oz_var_addQuietSusp(vPtr, oz_currentThread());
  }
} OZ_BI_end

// makes the argument needed (if necessary)
OZ_BI_define(BImakeNeeded,1,0)
{
  oz_declareDerefIN(0,v);
  Assert(!oz_isRef(v));

  if (oz_isVar(v)) {
    return oz_var_makeNeeded(vPtr);
  } else {
    return PROCEED;
  }
} OZ_BI_end

OZ_BI_define(BIwaitOr,2,0)
{
  oz_declareDerefIN(0,a);

  Assert(!oz_isRef(a));  
  if (!oz_isVarOrRef(a))
    return PROCEED;

  oz_declareDerefIN(1,b);

  Assert(!oz_isRef(b));
  if (!oz_isVarOrRef(b))
    return PROCEED;
  
  Assert(oz_isVar(a) && oz_isVar(b));

  if (!tagged2Var(a)->isInSuspList(oz_currentThread()))
    (void) am.addSuspendVarListInline(aPtr);
  if (!tagged2Var(b)->isInSuspList(oz_currentThread()))
    (void) am.addSuspendVarListInline(bPtr);

  return SUSPEND;
} OZ_BI_end

OZ_BI_define(BIwaitOrF,1,1)
{
  oz_declareNonvarIN(0,a);

  if (!oz_isRecord(a)) 
    oz_typeError(0,"Record");

  if (oz_isLiteral(a)) 
    oz_typeError(0,"ProperRecord");

  TaggedRef arity=OZ_arityList(a);
  
  while (!OZ_isNil(arity)) {
    TaggedRef v=OZ_subtree(a,OZ_head(arity));
    DEREF(v,vPtr);
    Assert(!oz_isRef(v));
    if (!oz_isVarOrRef(v)) {
      am.emptySuspendVarList();
      OZ_RETURN(OZ_head(arity));
    }
    if (!tagged2Var(v)->isInSuspList(oz_currentThread())) 
      (void) am.addSuspendVarListInline(vPtr);
    arity=OZ_tail(arity);
  }
  
  return SUSPEND;
} OZ_BI_end


OZ_BI_define(BIwaitStatus,2,1)
{
  oz_declareNonvarIN(0,status);
  oz_declareNonvarIN(1,what);
 
  if (oz_isSRecord(status)) {
    status = tagged2SRecord(status)->getLabel();
  }
  OZ_RETURN(oz_bool(oz_isLiteral(status)
		    && oz_isLiteral(what)
		    && oz_eq(status,what)));
} OZ_BI_end


OZ_BI_define(BIwaitGetChoice,1,0) {
  TaggedRef answer = OZ_in(0);
  DEREF(answer,answer_ptr);
 
  if(oz_isVar(answer)) {
    // This thread must wait until answer is bound.
    oz_suspendOn(makeTaggedRef(answer_ptr));
  }
  
  Board *bb = oz_currentBoard();
  // Reset branching to not repeat the same branch further.
  bb->setBranching(AtomNil);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIbindCSync,1,0) {
  //printf("hola desde bindCSync\n");fflush(stdout);
  oz_currentBoard()->commitB(OZ_in(0));
  return PROCEED;
} OZ_BI_end


#define CheckStatus(var,varstatus,statusAtom)						\
  OzVariable *cv   = tagged2Var(var);							\
  VarStatus status = oz_check_var_status(cv);						\
  switch (status) {									\
  case varstatus:									\
    OZ_RETURN(oz_true());								\
  case EVAR_STATUS_UNKNOWN:								\
    {											\
      OZ_Term status = _var_status(cv);							\
      OZ_Term out = oz_newVariable();							\
      OZ_out(0) = out;									\
      am.prepareCall(BI_waitStatus,RefsArray::make(status,statusAtom,out));					\
      return BI_REPLACEBICALL;								\
    }											\
  default:										\
    OZ_RETURN(oz_false());								\
  }

OZ_BI_define(BIisFree, 1,1)
{
  oz_declareDerefIN(0,var);

  Assert(!oz_isRef(var));
  if (oz_isVarOrRef(var)) {
    if (oz_isOptVar(var)) {
      OZ_RETURN(oz_true());
    } else {
      CheckStatus(var,EVAR_STATUS_FREE,AtomFree);
    }
  } else {
    OZ_RETURN(oz_false());
  }
} OZ_BI_end

OZ_BI_define(BIisKinded, 1,1)
{
  oz_declareDerefIN(0,var);
  Assert(!oz_isRef(var));
  if (!oz_isVarOrRef(var))
    OZ_RETURN(oz_false());

  CheckStatus(var,EVAR_STATUS_KINDED,AtomKinded);
} OZ_BI_end

OZ_BI_define(BIisFuture, 1,1)
{
  oz_declareDerefIN(0,var);
  Assert(!oz_isRef(var));
  if (!oz_isVarOrRef(var))
    OZ_RETURN(oz_false());

  CheckStatus(var,EVAR_STATUS_READONLY,AtomFuture);
} OZ_BI_end

OZ_BI_define(BIisDet,1,1)
{
  oz_declareDerefIN(0,var);

  Assert(!oz_isRef(var));
  if (oz_isVarOrRef(var)) {
    if (oz_isOptVar(var)) {
      OZ_RETURN(oz_false());
    } else {
      CheckStatus(var,EVAR_STATUS_DET,AtomDet);
    }
  } else {
    OZ_RETURN(oz_true());
  }
} OZ_BI_end

// returns true iff the argument is a failed value
OZ_BI_define(BIisFailed,1,1)
{
  oz_declareDerefIN(0,var);
  Assert(!oz_isRef(var));
  if (!oz_isVarOrRef(var))
    OZ_RETURN(oz_false());

  CheckStatus(var,EVAR_STATUS_FAILED,AtomFailed);
} OZ_BI_end

// returns true iff the argument is needed
OZ_BI_define(BIisNeeded,1,1)
{
  oz_declareDerefIN(0,var);
  Assert(!oz_isRef(var));
  OZ_RETURN(oz_isNeeded(var) ? oz_true() : oz_false());
} OZ_BI_end

#undef CheckStatus

#define BI_TESTDEF(BINAME,TEST)                        \
OZ_BI_define(BINAME, 1,1)                              \
{ TaggedRef t = OZ_in(0);                              \
 redo:                                                 \
  if (TEST(t))     { OZ_RETURN(oz_true());           } \
  if (oz_isRef(t)) { t = * tagged2Ref(t); goto redo; } \
  Assert(!oz_isRef(t));					\
  if (oz_isVarOrRef(t)) { oz_suspendOnInArgs1; }	\
  OZ_RETURN(oz_false());                               \
} OZ_BI_end

BI_TESTDEF(BIisLiteral,        oz_isLiteral)
BI_TESTDEF(BIisAtom,           oz_isAtom)
BI_TESTDEF(BIisName,           oz_isName)
BI_TESTDEF(BIisTuple,          oz_isTuple)
BI_TESTDEF(BIisRecord,         oz_isRecord)
BI_TESTDEF(BIisProcedure,      oz_isProcedure)
BI_TESTDEF(BIisChunk,          oz_isChunk)
BI_TESTDEF(BIisExtension,      oz_isExtension)
BI_TESTDEF(BIisCell,           oz_isCell)
BI_TESTDEF(BIisUnit,           oz_isUnit)
BI_TESTDEF(BIisBool,           oz_isBool)
BI_TESTDEF(BIisFloat,          oz_isFloat)
BI_TESTDEF(BIisInt,            oz_isInt)
BI_TESTDEF(BIisNumber,         oz_isNumber)
BI_TESTDEF(BIisPort,           oz_isPort)
BI_TESTDEF(BIisLock,           oz_isLock)
BI_TESTDEF(BIisArray,          oz_isArray)
BI_TESTDEF(BIisForeignPointer, OZ_isForeignPointer)
BI_TESTDEF(BIisObject,         oz_isObject)

#undef BI_TESTDEF

OZ_BI_define(BIprocedureArity, 1,1)
{
  oz_declareNonvarIN(0, pterm);

  if (oz_isProcedure(pterm)) {
    OZ_RETURN(makeTaggedSmallInt(oz_procedureArity(pterm)));
  } else {
    oz_typeError(0,"Procedure");
  }
} OZ_BI_end


// ---------------------------------------------------------------------
// Tuple
// ---------------------------------------------------------------------

OZ_BI_define(BItuple, 2, 1)
{
  oz_declareNonvarIN(0, label);
  oz_declareIntIN(1, i);

  if (!oz_isLiteral(label)) oz_typeError(0, "Literal");
  if (i < 0) oz_typeError(1, "(non-negative small) Int");
  if (i == 0) OZ_RETURN(label);

  SRecord *sr = SRecord::newSRecord(label, i);
  
  TaggedRef ov = am.getCurrentOptVar();
  while (i--)
    sr->setArg(i, ov);

  OZ_RETURN(sr->normalize());
} OZ_BI_end

/********************************************************************
 * Arrays
 ******************************************************************** */

OZ_BI_define(BIarrayNew,3,1)
{
  oz_declareIntIN(0,ilow);
  oz_declareIntIN(1,ihigh);
  oz_declareIN(2,initValue);

  if (!oz_isSmallInt(OZ_deref(OZ_in(0)))) { oz_typeError(0,"smallInteger"); }
  if (!oz_isSmallInt(OZ_deref(OZ_in(1)))) { oz_typeError(1,"smallInteger"); }

  OzArray *array = new OzArray(oz_currentBoard(),ilow,ihigh,initValue);
  if (array==NULL || array->getWidth()==-1) {
    return oz_raise(E_SYSTEM,E_SYSTEM,"limitExternal",1,OZ_atom("not enough memory"));
  }
    
  OZ_RETURN(makeTaggedConst(array));
} OZ_BI_end


inline
OZ_Return arrayLowInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term );
  if (!oz_isArray(term)) {
    oz_typeError(0,"Array");
  }
  out = makeTaggedSmallInt(tagged2Array(term)->getLow());
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN1(BIarrayLow,arrayLowInline)

inline
OZ_Return arrayHighInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term );
  if (!oz_isArray(term)) {
    oz_typeError(0,"Array");
  }
  out = makeTaggedSmallInt(tagged2Array(term)->getHigh());
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN1(BIarrayHigh,arrayHighInline)

inline
OZ_Return arrayGetInline(TaggedRef t, TaggedRef i, TaggedRef &out)
{
  NONVAR( t, array );
  NONVAR( i, index );

  if (!oz_isArray(array)) {
    oz_typeError(0,"Array");
  }

  if (!oz_isSmallInt(index)) {
    oz_typeError(1,"smallInteger");
  }

  OzArray *ar = tagged2Array(array);
  out = ar->getArg(tagged2SmallInt(index));
  if (out) return PROCEED;
  return oz_raise(E_ERROR,E_KERNEL,"array",2,array,index);
}
OZ_DECLAREBI_USEINLINEFUN2(BIarrayGet,arrayGetInline)

inline
OZ_Return arrayPutInline(TaggedRef t, TaggedRef i, TaggedRef value)
{
  NONVAR( t, array );
  NONVAR( i, index );

  if (!oz_isArray(array)) {
    oz_typeError(0,"Array");
  }

  if (!oz_isSmallInt(index)) {
    oz_typeError(1,"smallInteger");
  }

  OzArray *ar = tagged2Array(array);
  CheckLocalBoard(ar,"array");
  if (ar->setArg(tagged2SmallInt(index),value)) return PROCEED;

  return oz_raise(E_ERROR,E_KERNEL,"array",2,array,index);
}

OZ_DECLAREBI_USEINLINEREL3(BIarrayPut,arrayPutInline);

inline
OZ_Return arrayExchangeInline(TaggedRef t, TaggedRef i, TaggedRef value, TaggedRef& old)
{
  NONVAR( t, array );
  NONVAR( i, index );

  if (!oz_isArray(array)) {
    oz_typeError(0,"Array");
  }

  if (!oz_isSmallInt(index)) {
    oz_typeError(1,"smallInteger");
  }

  OzArray *ar = tagged2Array(array);
  CheckLocalBoard(ar,"array");
  old = ar->exchange(tagged2SmallInt(index),value);
  if (old) return PROCEED;

  return oz_raise(E_ERROR,E_KERNEL,"array",2,array,index);
}

OZ_DECLAREBI_USEINLINEFUN3(BIarrayExchange,arrayExchangeInline);

// ---------------------------------------------------------------------
// Tuple & Record
// ---------------------------------------------------------------------

OZ_BI_define(BIlabel, 1, 1)
{
 oz_declareNonKindedIN(0,rec);
 Assert(!oz_isRef(rec));
 if (oz_isLTupleOrRef(rec)) OZ_RETURN(AtomCons);
 if (oz_isLiteral(rec)) OZ_RETURN(rec);
 if (oz_isSRecord(rec)) OZ_RETURN(tagged2SRecord(rec)->getLabel());
 if (isGenOFSVar(rec)) {
   TaggedRef thelabel=tagged2GenOFSVar(rec)->getLabel(); 
   DEREF(thelabel,lPtr);
   Assert(!oz_isRef(thelabel));
   if (oz_isVarOrRef(thelabel)) oz_suspendOnPtr(lPtr);
   OZ_RETURN(thelabel);
 }
 oz_typeError(0,"Record");
} OZ_BI_end


/*
 * NOTE: similar functions are dot, genericSet, uparrow
 */

#define GDOT_CASE_TYPEFEAT(a) \
  case LTAG_PAIR(a,LTAG_MARK0):    case LTAG_PAIR(a,LTAG_MARK1):   \
  case LTAG_PAIR(a,LTAG_LTUPLE0):  case LTAG_PAIR(a,LTAG_LTUPLE1): \
  case LTAG_PAIR(a,LTAG_SRECORD0): case LTAG_PAIR(a,LTAG_SRECORD1):

inline
OZ_Return genericDot(TaggedRef t, TaggedRef f, TaggedRef &tf, Bool isdot) {
  ltag_t t_t = tagged2ltag(t);
  ltag_t f_t = tagged2ltag(f);
 redo:
  switch (LTAG_PAIR(t_t,f_t)) {
    // Dereferencing
  LTAG_CASE_REF_REF
    t = * tagged2Ref(t);
    t_t = tagged2ltag(t);
    f   = * tagged2Ref(f);
    f_t = tagged2ltag(f);
    goto redo;
  LTAG_CASE_REF_VAL
    t   = * tagged2Ref(t);    
    t_t = tagged2ltag(t);
    goto redo;
  LTAG_CASE_VAL_REF
    f   = * tagged2Ref(f);
    f_t = tagged2ltag(f);
    goto redo;

    // Type errors for feature
  GDOT_CASE_TYPEFEAT(LTAG_LTUPLE0)
  GDOT_CASE_TYPEFEAT(LTAG_LTUPLE1)
  GDOT_CASE_TYPEFEAT(LTAG_SRECORD0)
  GDOT_CASE_TYPEFEAT(LTAG_SRECORD1)
  GDOT_CASE_TYPEFEAT(LTAG_CONST0)
  GDOT_CASE_TYPEFEAT(LTAG_CONST1)
    goto type_error_f;

    // Variable as feature
  case LTAG_PAIR(LTAG_CONST0,LTAG_VAR0):
  case LTAG_PAIR(LTAG_CONST1,LTAG_VAR0):
  case LTAG_PAIR(LTAG_CONST0,LTAG_VAR1):
  case LTAG_PAIR(LTAG_CONST1,LTAG_VAR1):
    if (!oz_isChunk(t))
      goto type_error_t;
    else
      goto feature_var;
  case LTAG_PAIR(LTAG_VAR0,LTAG_VAR0):
  case LTAG_PAIR(LTAG_VAR0,LTAG_VAR1):
  case LTAG_PAIR(LTAG_VAR1,LTAG_VAR0):
  case LTAG_PAIR(LTAG_VAR1,LTAG_VAR1):
    switch (tagged2Var(t)->getType()) {
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
    case OZ_VAR_FS:
      goto type_error_t;
    default:
      goto feature_var;
    }
  case LTAG_PAIR(LTAG_SRECORD0,LTAG_VAR0):
  case LTAG_PAIR(LTAG_SRECORD0,LTAG_VAR1):
  case LTAG_PAIR(LTAG_SRECORD1,LTAG_VAR0):
  case LTAG_PAIR(LTAG_SRECORD1,LTAG_VAR1):
  case LTAG_PAIR(LTAG_LITERAL,LTAG_VAR0):
  case LTAG_PAIR(LTAG_LITERAL,LTAG_VAR1):
  case LTAG_PAIR(LTAG_LTUPLE0,LTAG_VAR0):
  case LTAG_PAIR(LTAG_LTUPLE0,LTAG_VAR1):
  case LTAG_PAIR(LTAG_LTUPLE1,LTAG_VAR0):
  case LTAG_PAIR(LTAG_LTUPLE1,LTAG_VAR1):
  feature_var:
    switch (tagged2Var(f)->getType()) {
    case OZ_VAR_FS:
      goto type_error_f;
    default:
      return SUSPEND;
    }

    // List
  case LTAG_PAIR(LTAG_LTUPLE0,LTAG_SMALLINT):
  case LTAG_PAIR(LTAG_LTUPLE1,LTAG_SMALLINT):
    if (f == makeTaggedSmallInt(1)) {
      tf = tagged2LTuple(t)->getHead();
      return PROCEED;
    }
    if (f == makeTaggedSmallInt(2)) {
      tf = tagged2LTuple(t)->getTail();
      return PROCEED;
    }
    goto no_feature;
  case LTAG_PAIR(LTAG_LTUPLE0,LTAG_LITERAL):
  case LTAG_PAIR(LTAG_LTUPLE1,LTAG_LITERAL):
    goto no_feature;
  case LTAG_PAIR(LTAG_LTUPLE0,LTAG_CONST0):
  case LTAG_PAIR(LTAG_LTUPLE1,LTAG_CONST0):
  case LTAG_PAIR(LTAG_LTUPLE0,LTAG_CONST1):
  case LTAG_PAIR(LTAG_LTUPLE1,LTAG_CONST1):
    if (tagged2Const(f)->getType() != Co_BigInt)
      goto type_error_f;
    else
      goto no_feature;

    // SRecord
  case LTAG_PAIR(LTAG_SRECORD0,LTAG_CONST0):
  case LTAG_PAIR(LTAG_SRECORD1,LTAG_CONST0):
  case LTAG_PAIR(LTAG_SRECORD0,LTAG_CONST1):
  case LTAG_PAIR(LTAG_SRECORD1,LTAG_CONST1):
    if (tagged2Const(f)->getType() != Co_BigInt)
      goto type_error_f;
    tf = tagged2SRecord(t)->getBigIntFeatureInline(f);
    if (tf == makeTaggedNULL())
      goto no_feature;
    return PROCEED;
  case LTAG_PAIR(LTAG_SRECORD0,LTAG_SMALLINT):
  case LTAG_PAIR(LTAG_SRECORD1,LTAG_SMALLINT):
    tf = tagged2SRecord(t)->getSmallIntFeatureInline(f);
    if (tf == makeTaggedNULL())
      goto no_feature;
    return PROCEED;
  case LTAG_PAIR(LTAG_SRECORD0,LTAG_LITERAL):
  case LTAG_PAIR(LTAG_SRECORD1,LTAG_LITERAL):
    tf = tagged2SRecord(t)->getLiteralFeatureInline(f);
    if (tf == makeTaggedNULL())
      goto no_feature;
    return PROCEED;

    // Literal
  case LTAG_PAIR(LTAG_LITERAL,LTAG_CONST0):
  case LTAG_PAIR(LTAG_LITERAL,LTAG_CONST1):
    if (tagged2Const(f)->getType() != Co_BigInt)
      goto type_error_f;
    else
      goto no_feature;
  case LTAG_PAIR(LTAG_LITERAL,LTAG_SMALLINT):
  case LTAG_PAIR(LTAG_LITERAL,LTAG_LITERAL):
    goto no_feature;

    // Variable
  case LTAG_PAIR(LTAG_VAR0,LTAG_CONST0):
  case LTAG_PAIR(LTAG_VAR1,LTAG_CONST0):
  case LTAG_PAIR(LTAG_VAR0,LTAG_CONST1):
  case LTAG_PAIR(LTAG_VAR1,LTAG_CONST1):
    if (tagged2Const(f)->getType() != Co_BigInt)
      goto type_error_f;
    /* fall through */
  case LTAG_PAIR(LTAG_VAR0,LTAG_SMALLINT):
  case LTAG_PAIR(LTAG_VAR1,LTAG_SMALLINT):
  case LTAG_PAIR(LTAG_VAR0,LTAG_LITERAL):
  case LTAG_PAIR(LTAG_VAR1,LTAG_LITERAL):
    switch (tagged2Var(t)->getType()) {
    case OZ_VAR_OF:
      {
	int ret = tagged2GenOFSVar(t)->hasFeature(f,&tf);
	if (ret == FAILED) 
	  goto no_feature;
	return ret;
      }
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
    case OZ_VAR_FS:
      goto type_error_t;
    default:
      return SUSPEND;
    }

    // Const
  case LTAG_PAIR(LTAG_CONST0,LTAG_CONST0):
  case LTAG_PAIR(LTAG_CONST0,LTAG_CONST1):
  case LTAG_PAIR(LTAG_CONST1,LTAG_CONST0):
  case LTAG_PAIR(LTAG_CONST1,LTAG_CONST1):
    if (tagged2Const(f)->getType() != Co_BigInt)
      goto type_error_f;
  case LTAG_PAIR(LTAG_CONST0,LTAG_LITERAL):
  case LTAG_PAIR(LTAG_CONST1,LTAG_LITERAL):
  case LTAG_PAIR(LTAG_CONST0,LTAG_SMALLINT):
  case LTAG_PAIR(LTAG_CONST1,LTAG_SMALLINT):
    switch (tagged2Const(t)->getType()) {
    case Co_Extension:
      if (!oz_isChunkExtension(t))
	goto type_error_t;
      tf = tagged2Extension(t)->getFeatureV(f);
      if (tf == makeTaggedNULL())
	goto no_feature;
      return PROCEED;
    case Co_Object:
      tf = tagged2Object(t)->getFeature(f);
      if (tf == makeTaggedNULL())
	goto no_feature;
      return PROCEED;
    case Co_Port:
    case Co_Lock:
      goto no_feature;
    case Co_Chunk:
      tf = tagged2SChunk(t)->getFeature(f);
      if (tf == makeTaggedNULL())
	goto no_feature;
      return PROCEED;
    case Co_Class:
      tf = tagged2ObjectClass(t)->classGetFeature(f);
      if (tf == makeTaggedNULL()) {
	TaggedRef cfs = oz_deref(tagged2ObjectClass(t)->classGetFeature(NameOoFeat));
	if (oz_isSRecord(cfs)) {
	  tf = tagged2SRecord(cfs)->getFeature(f);
	  if (tf) {
	    TaggedRef dt = oz_deref(tf);
	    if (oz_isName(dt) && oz_eq(dt,NameOoFreeFlag))
	      goto no_feature;
	    return PROCEED;
	  }
	}
	goto no_feature;
      }
      return PROCEED;
    case Co_Array:
      {
	if (!oz_isSmallInt(f))
	  goto no_feature;
	tf = tagged2Array(t)->getArg(tagged2SmallInt(f));
	if (tf == makeTaggedNULL()) 
	  goto no_feature;
	return PROCEED;
      }
    case Co_Dictionary:
      tf = tagged2Dictionary(t)->getArg(f);
      if (!tf)
	goto no_feature;
      return PROCEED;
    default:
      goto type_error_t;
    }
 
  default:
   goto type_error_t;
  }

 type_error_t:
  oz_typeError(0,"Record or Chunk");

 type_error_f:
  oz_typeError(1,"Feature");

 no_feature:
  if (isdot)
    return oz_raise(E_ERROR,E_KERNEL,".",2,t,f);
  else
    return FAILED;
}

// extern
OZ_Return dotInline(TaggedRef term, TaggedRef fea, TaggedRef &out) {
  return genericDot(term,fea,out,TRUE);
}

OZ_BI_define(BIdot,2,1) {
  OZ_Return r = genericDot(OZ_in(0), OZ_in(1), OZ_out(0), TRUE);
  if (r == SUSPEND) {
    oz_suspendOnInArgs2;
  } else {
    return r;
  }
} OZ_BI_end
  

OZ_BI_define(BIhasFeature,2,1)
{
  TaggedRef dummy;
  OZ_Return r = genericDot(OZ_in(0),OZ_in(1),dummy,FALSE);
  switch (r) {
  case PROCEED: OZ_RETURN(oz_true());
  case FAILED : OZ_RETURN(oz_false());
  case SUSPEND: oz_suspendOnInArgs2;
  default     : return r;
  }
} OZ_BI_end

/*
 * fun {matchDefault Term Attr Defau}
 *    if X in Term.Attr = X then X else Defau fi
 * end
 */

OZ_BI_define(BImatchDefault,3,1) {
  OZ_Return ret = genericDot(OZ_in(0),OZ_in(1),OZ_out(0),FALSE);
  switch (ret) {
  case PROCEED:
    return PROCEED;
  case FAILED:
    OZ_out(0)=OZ_in(2);
    return PROCEED;
  case SUSPEND:
    oz_suspendOnInArgs2;
  default:
    return ret;
  }
} OZ_BI_end


OZ_BI_define(BIwidth,1,1) {
  TaggedRef t = OZ_in(0);
 redo:
  switch (tagged2ltag(t)) {
  case LTAG_REF00:
  case LTAG_REF01:
  case LTAG_REF10:
  case LTAG_REF11:
    t = * tagged2Ref(t);
    goto redo;
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    OZ_RETURN(makeTaggedSmallInt(2));
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    OZ_RETURN(makeTaggedSmallInt(tagged2SRecord(t)->getWidth()));
  case LTAG_LITERAL:
    OZ_RETURN(makeTaggedSmallInt(0));
  case LTAG_VAR0:
  case LTAG_VAR1:
    switch (tagged2Var(t)->getType()) {
    case OZ_VAR_FD:
    case OZ_VAR_FS:
    case OZ_VAR_BOOL:
      break;
    default:
      oz_suspendOn(OZ_in(0));
    }
  default:
    break;
  }
  oz_typeError(0,"Record");
} OZ_BI_end

// ---------------------------------------------------------------------
// Dict/Array Update
// ---------------------------------------------------------------------

static inline
OZ_Return genericSet(TaggedRef term, TaggedRef fea, TaggedRef val) {
  DEREF(fea,  _1);
  DEREF(term, _2);

  Assert(!oz_isRef(fea));
  if (oz_isVarOrRef(fea)) {
    switch (tagged2ltag(term)) {
    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
      return SUSPEND;
    case LTAG_VAR0:
    case LTAG_VAR1:
      switch (tagged2Var(term)->getType()) {
      case OZ_VAR_FD:
      case OZ_VAR_BOOL:
      case OZ_VAR_FS:
          goto typeError0;
      default:
          return SUSPEND;
      }
    case LTAG_LITERAL:
      goto typeError0;
    default:
      if (oz_isChunk(term)) return SUSPEND;
      goto typeError0;
    }
  }

  if (!oz_isFeature(fea)) goto typeError1;

  switch (tagged2ltag(term)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    goto typeError0;
    
  case LTAG_VAR0:
  case LTAG_VAR1:
    switch (tagged2Var(term)->getType()) {
    case OZ_VAR_OF:
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
    case OZ_VAR_FS:
      goto typeError0;
    default:
      return SUSPEND;
    }

  case LTAG_LITERAL: 
    goto typeError0;

  default:
    if (oz_isChunk(term)) {
      switch (tagged2Const(term)->getType()) {
      case Co_Extension:
	return tagged2Extension(term)->putFeatureV(fea,val);
      case Co_Chunk:
      case Co_Object:
      case Co_Class:
	goto typeError0;
      case Co_Array:
	{
	  OZ_Return arrayPutInline(TaggedRef,TaggedRef,TaggedRef);
	  return arrayPutInline(term,fea,val);
	}
      case Co_Dictionary:
	{
	  extern OZ_Return dictionaryPutInline(TaggedRef,TaggedRef,TaggedRef);
	  return dictionaryPutInline(term,fea,val);
	}
      default:
	goto typeError0;
      }
    }

    goto typeError0;
  }
typeError0:
  oz_typeError(0,"Dictionary or Array");
typeError1:
  oz_typeError(1,"Feature");
}

OZ_DECLAREBI_USEINLINEREL3(BIdotAssign,genericSet)

static inline
OZ_Return genericExchange(TaggedRef term, TaggedRef fea, TaggedRef val, TaggedRef& old) {
  DEREF(fea,  _1);
  DEREF(term, _2);

  Assert(!oz_isRef(fea));
  if (oz_isVarOrRef(fea)) {
    switch (tagged2ltag(term)) {
    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
      return SUSPEND;
    case LTAG_VAR0:
    case LTAG_VAR1:
      switch (tagged2Var(term)->getType()) {
      case OZ_VAR_FD:
      case OZ_VAR_BOOL:
      case OZ_VAR_FS:
          goto typeError0;
      default:
          return SUSPEND;
      }
    case LTAG_LITERAL:
      goto typeError0;
    default:
      if (oz_isChunk(term)) return SUSPEND;
      goto typeError0;
    }
  }

  if (!oz_isFeature(fea)) goto typeError1;

  switch (tagged2ltag(term)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    goto typeError0;
    
  case LTAG_VAR0:
  case LTAG_VAR1:
    switch (tagged2Var(term)->getType()) {
    case OZ_VAR_OF:
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
    case OZ_VAR_FS:
      goto typeError0;
    default:
      return SUSPEND;
    }

  case LTAG_LITERAL: 
    goto typeError0;

  default:
    if (oz_isChunk(term)) {
      switch (tagged2Const(term)->getType()) {
      case Co_Extension:
	{
	  OZ_Extension *ext = tagged2Extension(term);
	  OZ_Return r = ext->getFeatureV(fea,old);
	  if (r != PROCEED) return r;
	  return ext->putFeatureV(fea,val);
	}
      case Co_Chunk:
      case Co_Object:
      case Co_Class:
	goto typeError0;
      case Co_Array:
	{
	  OZ_Return arrayExchangeInline(TaggedRef,TaggedRef,TaggedRef,TaggedRef&);
	  return arrayExchangeInline(term,fea,val,old);
	}
      case Co_Dictionary:
	{
      	  extern OZ_Return dictionaryExchangeInline(TaggedRef,TaggedRef,TaggedRef,TaggedRef&);
	  return dictionaryExchangeInline(term,fea,val,old);
	}
      default:
	goto typeError0;
      }
    }

    goto typeError0;
  }
typeError0:
  oz_typeError(0,"Dictionary or Array");
typeError1:
  oz_typeError(1,"Feature");
}

OZ_DECLAREBI_USEINLINEFUN3(BIdotExchange,genericExchange)


// ---------------------------------------------------------------------
// Bool things
// ---------------------------------------------------------------------

OZ_BI_define(BInot, 1, 1)
{
  oz_declareBoolIN(0,b);
  OZ_RETURN(oz_bool(!b));
} OZ_BI_end

OZ_BI_define(BIand, 2, 1)
{
  oz_declareBoolIN(0,A);
  oz_declareBoolIN(1,B);

  if (A) {
    OZ_RETURN(oz_bool(B));
  } else {
    OZ_RETURN(oz_false());
  }
} OZ_BI_end

OZ_BI_define(BIor, 2, 1)
{
  oz_declareBoolIN(0,A);
  oz_declareBoolIN(1,B);

  if (A) {
    OZ_RETURN(oz_true());
  } else {
    OZ_RETURN(oz_bool(B));
  }
} OZ_BI_end



// ---------------------------------------------------------------------
// Atom
// ---------------------------------------------------------------------

OZ_BI_define(BIatomToString, 1, 1)
{
  oz_declareNonvarIN(0,t);

  if (!oz_isAtom(t)) oz_typeError(0,"atom");
  
  OZ_RETURN(OZ_string((OZ_CONST char*)tagged2Literal(t)->getPrintName()));
} OZ_BI_end

OZ_BI_define(BIstringToAtom,1,1)
{
  oz_declareProperStringIN(0,str);

  OZ_RETURN(oz_atom(str));
} OZ_BI_end

// ---------------------------------------------------------------------
// Virtual Strings
// ---------------------------------------------------------------------

inline
TaggedRef vs_suspend(SRecord *vs, int i, TaggedRef arg_rest) {
  if (i == vs->getWidth()-1) {
    return arg_rest;
  } else {
    SRecord *stuple = SRecord::newSRecord(AtomPair, vs->getWidth() - i);
    stuple->setArg(0, arg_rest);
    i++;
    for (int j=1 ; i < vs->getWidth() ; (j++, i++))
      stuple->setArg(j, vs->getArg(i));
    return makeTaggedSRecord(stuple);
  }
}

static OZ_Return vs_check(OZ_Term vs, OZ_Term *rest) {
  DEREF(vs, vs_ptr);

  Assert(!oz_isRef(vs));
  if (oz_isVarOrRef(vs)) {
    *rest = makeTaggedRef(vs_ptr);
    oz_suspendOn(*rest);
  } else if (oz_isSmallInt(vs)) {
    return PROCEED;
  } else if (oz_isBigInt(vs)) {
    return PROCEED;
  } else if (oz_isFloat(vs)) {
    return PROCEED;
  } else if (oz_isAtom(vs)) {
    return PROCEED;
  } else if (oz_isLTupleOrRef(vs)) {
    TaggedRef cdr  = vs;
    TaggedRef prev = vs;

    while (1) {
      DEREF(cdr, cdr_ptr);

      if (oz_isNil(cdr))
	return PROCEED;

      Assert(!oz_isRef(cdr));
      if (oz_isVarOrRef(cdr)) {
	*rest = prev;
	oz_suspendOn(makeTaggedRef(cdr_ptr));
      }

      Assert(!oz_isRef(cdr));
      if (!oz_isLTupleOrRef(cdr))
	return FAILED;

      TaggedRef car = tagged2LTuple(cdr)->getHead();
      DEREF(car, car_ptr);

      Assert(!oz_isRef(car));
      if (oz_isVarOrRef(car)) {
	*rest = cdr;
	oz_suspendOn(makeTaggedRef(car_ptr));
      } else if (!oz_isSmallInt(car) ||
		 (tagged2SmallInt(car) <= 0) ||
		 (tagged2SmallInt(car) > 255)) {
	return FAILED;
      } else {
	prev = cdr;
	cdr  = tagged2LTuple(cdr)->getTail();
      }
    };
    
    return FAILED;

  } else if (oz_isSTuple(vs) && 
	     oz_eq(tagged2SRecord(vs)->getLabel(),AtomPair)) {
    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++) {
      TaggedRef arg_rest;
      OZ_Return status = vs_check(tagged2SRecord(vs)->getArg(i), &arg_rest);

      if (status == SUSPEND) {
	*rest = vs_suspend(tagged2SRecord(vs), i, arg_rest);

	return SUSPEND;
      } else if (status==FAILED) {
	return FAILED;
      }
    }
    return PROCEED;
  } else if (oz_isByteString(vs)) {
    return PROCEED;
  } else {
    return FAILED;
  }
}


// mm2: cycle check needed
static OZ_Return vs_length(OZ_Term vs, OZ_Term *rest, int *len) {
  DEREF(vs, vs_ptr);

  Assert(!oz_isRef(vs));
  if (oz_isVarOrRef(vs)) {
    *rest = makeTaggedRef(vs_ptr);
    oz_suspendOn(*rest);
  } else if (oz_isSmallInt(vs) || oz_isBigInt(vs)) {
    *len = *len + strlen(toC(vs));
    return PROCEED;
  } else if (oz_isFloat(vs)) {
    *len = *len + strlen(toC(vs));
    return PROCEED;
  } else if (oz_isAtom(vs)) {
    if (oz_eq(vs,AtomPair) || 
	oz_eq(vs,AtomNil)) 
      return PROCEED;
    *len = *len + ((Atom*)tagged2Literal(vs))->getSize();
    return PROCEED;
  } else if (oz_isLTupleOrRef(vs)) {
    TaggedRef cdr  = vs;
    TaggedRef prev = vs;

    while (1) {
      DEREF(cdr, cdr_ptr);

      if (oz_isNil(cdr))
	return PROCEED;

      Assert(!oz_isRef(cdr));
      if (oz_isVarOrRef(cdr)) {
	*rest = prev;
	Assert((*len)>0);
	*len = *len - 1;
	oz_suspendOn(makeTaggedRef(cdr_ptr));
      }

      Assert(!oz_isRef(cdr));
      if (!oz_isLTupleOrRef(cdr))
	return FAILED;

      TaggedRef car = tagged2LTuple(cdr)->getHead();
      DEREF(car, car_ptr);

      Assert(!oz_isRef(car));
      if (oz_isVarOrRef(car)) {
	*rest = cdr;
	oz_suspendOn(makeTaggedRef(car_ptr));
      } else if (!oz_isSmallInt(car) ||
		 (tagged2SmallInt(car) < 0) ||
		 (tagged2SmallInt(car) > 255)) {
	return FAILED;
      } else {
	prev = cdr;
	cdr  = tagged2LTuple(cdr)->getTail();
	*len = *len + 1;
      }
    };
    
    return FAILED;

  } else if (oz_isSTuple(vs) && 
	     oz_eq(tagged2SRecord(vs)->getLabel(),AtomPair)) {
    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++) {
      TaggedRef arg_rest;
      OZ_Return status = 
	vs_length(tagged2SRecord(vs)->getArg(i), &arg_rest, len);

      if (status == SUSPEND) {
	*rest = vs_suspend(tagged2SRecord(vs), i, arg_rest);
	return SUSPEND;
      } else if (status==FAILED) {
	return FAILED;
      }
    }
    return PROCEED;
  } else if (oz_isByteString(vs)) {
    *len = *len + tagged2ByteString(vs)->getWidth();
    return PROCEED;
  } else {
    return FAILED;
  }
}


OZ_BI_define(BIvsLength,2,1) {
  TaggedRef rest = taggedVoidValue;
  int len = tagged2SmallInt(oz_deref(OZ_in(1)));
  OZ_Return status = vs_length(OZ_in(0), &rest, &len);
  if (status == SUSPEND) {
    OZ_in(0) = rest;
    OZ_in(1) = makeTaggedSmallInt(len);
    return SUSPEND;
  } else if (status == FAILED) {
    oz_typeError(0, "Virtual String");
  } else {
    OZ_RETURN(makeTaggedSmallInt(len));
  }
} OZ_BI_end

OZ_BI_define(BIvsIs,1,1) {
  TaggedRef rest = taggedVoidValue;
  OZ_Return status = vs_check(OZ_in(0), &rest);
  if (status == SUSPEND) {
    OZ_in(0) = rest;
    return SUSPEND;
  }
  OZ_RETURN(oz_bool(status == PROCEED));
} OZ_BI_end

OZ_BI_define(BIvsToBs,3,1)
{
  // OZ_in(0) is initially the argument vs
  // OZ_in(1) is initially 0
  // OZ_in(2) is always the initial argument vs
  //
  // OZ_in(0) is side effected to hold what remains so far undetermined
  // packed up as a new virtual string
  TaggedRef rest = taggedVoidValue;
  int len = tagged2SmallInt(oz_deref(OZ_in(1)));
  OZ_Return status = vs_length(OZ_in(0), &rest, &len);
  if (status == SUSPEND) {
    OZ_in(0) = rest;
    OZ_in(1) = makeTaggedSmallInt(len);
    return SUSPEND;
  } else if (status == FAILED) {
    oz_typeError(0, "Virtual String");
  } else {
    // the initial argument vs is in OZ_in(2)
    // it is known to be fully determined and its
    // size is len
    ByteString* bs = new ByteString(len);
    ostrstream *out = new ostrstream;
    extern void virtualString2buffer(ostream &,OZ_Term,int);
    virtualString2buffer(*out,OZ_in(2),1);
    bs->copy(out->str(),len);
    delete out;
    OZ_RETURN(makeTaggedExtension(bs));
  }
} OZ_BI_end

// ---------------------------------------------------------------------
// Chunk
// ---------------------------------------------------------------------

OZ_BI_define(BInewChunk,1,1)
{
  oz_declareNonvarIN(0,val);

  if (!oz_isRecord(val)) oz_typeError(0,"Record");

  OZ_RETURN(oz_newChunk(oz_currentBoard(),val));
} OZ_BI_end

/* ---------------------------------------------------------------------
 * Threads
 * --------------------------------------------------------------------- */

OZ_BI_define(BIthreadThis,0,1)
{
  OZ_RETURN(oz_thread(oz_currentThread()));
} OZ_BI_end

/*
 * change priority of a thread
 *  if my priority is lowered, then preempt me
 *  if priority of other thread become higher than mine, then preempt me
 */
OZ_BI_define(BIthreadSetPriority,2,0)
{
  oz_declareThread(0,th);
  oz_declareNonvarIN(1,atom_prio);

  int prio;

  if (!oz_isAtom(atom_prio)) 
    goto type_goof;
    
  if (oz_eq(atom_prio, AtomLow)) {
    prio = LOW_PRIORITY;
  } else if (oz_eq(atom_prio, AtomMedium)) {
    prio = MID_PRIORITY;
  } else if (oz_eq(atom_prio, AtomHigh)) {
    prio = HI_PRIORITY;
  } else {
  type_goof:
    oz_typeError(1,"Atom [low medium high]");
  }

  int oldPrio = th->getPriority();
  th->setPriority(prio);

  if (oz_currentThread() == th) {
    if (prio <= oldPrio)
      return BI_PREEMPT;
  } else {
    if (th->isRunnable()) {
      am.threadsPool.rescheduleThread(th);
    }
    if (prio > oz_currentThread()->getPriority()) {
      return BI_PREEMPT;
    }
  }

  return PROCEED;
} OZ_BI_end

OZ_Term threadGetPriority(Thread *th) {
  switch (th->getPriority()) {
  case LOW_PRIORITY: return AtomLow;
  case MID_PRIORITY: return AtomMedium;
  case HI_PRIORITY:  return AtomHigh;
  default: Assert(0); return AtomHigh; 
  }
}

OZ_BI_define(BIthreadGetPriority,1,1)
{
  oz_declareThread(0,th);

  OZ_RETURN(threadGetPriority(th));
} OZ_BI_end

OZ_BI_define(BIthreadIs,1,1)
{
  oz_declareNonvarIN(0,th);

  OZ_RETURN(oz_bool(oz_isThread(th)));
} OZ_BI_end

/*
 * raise exception on thread
 */

static void threadRaise(Thread *th,OZ_Term E) {
  Assert(oz_currentThread() != th);

  th->pushCall(BI_raise, RefsArray::make(E));

  th->unsetStop();

  if (th->isSuspended()) {
    oz_wakeupThread(th);
    return;
  }

  if (!am.threadsPool.isScheduledSlow(th))
    am.threadsPool.scheduleThread(th);
}

OZ_BI_define(BIthreadRaise,2,0)
{
  oz_declareThread(0,th);
  oz_declareNonvarIN(1,E);

  if (oz_currentThread() == th) {
    return OZ_raiseDebug(E);
  }

  threadRaise(th,E);
  return PROCEED;
} OZ_BI_end

/*
 * suspend a thread
 *   is done lazy: when the thread becomes running the stop flag is tested
 */
OZ_BI_define(BIthreadSuspend,1,0)
{
  oz_declareThread(0,th);
  
  th->setStop();
  if (th == oz_currentThread()) {
    return BI_PREEMPT;
  }
  return PROCEED;
} OZ_BI_end

void threadResume(Thread *th) {
  th->unsetStop();

  /* mm2: I don't understand this, but let's try to give some explanation.
   *  1. resuming the current thread should be a NOOP.
   *  2. only runnable threads need to be rescheduled.
   */
  if (th!=oz_currentThread() &&
      th->isRunnable() &&
      !am.threadsPool.isScheduledSlow(th)) {
    am.threadsPool.scheduleThread(th);
  }
}

OZ_BI_define(BIthreadResume,1,0)
{
  oz_declareThread(0,th);

  threadResume(th);

  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIthreadIsSuspended,1,1)
{
  oz_declareThread(0,th);

  OZ_RETURN(oz_bool(th->isStop()));
} OZ_BI_end

OZ_Term threadState(Thread *th) {
  if (th->isDead()) {
    return oz_atom("terminated");
  }
  if (th->isRunnable()) {
    return oz_atom("runnable");
  }
  return oz_atom("blocked");
}

OZ_BI_define(BIthreadState,1,1)
{
  oz_declareThreadIN(0,th);

  OZ_RETURN(threadState(th));
} OZ_BI_end

OZ_BI_define(BIthreadPreempt,1,0)
{
  oz_declareThread(0,th);
  
  if (th == oz_currentThread()) {
    return BI_PREEMPT;
  }
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIthreadCreate,1,0)
{
  oz_declareNonvarIN(0,p);

  if (!oz_isAbstraction(p)) {
    oz_typeError(0,"Abstraction");
  }

  Abstraction *a = tagged2Abstraction(p);
  if (a->getArity() != 0) {
    oz_typeError(0,"Nullary Abstraction");
  }
  
  int prio   = min(oz_currentThread()->getPriority(),DEFAULT_PRIORITY);
  Thread *tt = oz_newThread(prio);
  
  tt->getTaskStackRef()->pushCont(a->getPC(),NULL,a);
  tt->setAbstr(a->getPred());
  
  if (am.debugmode() && oz_onToplevel() && oz_currentThread()->isTrace()) {
    tt->setTrace();
    tt->setStep();
  }

  return PROCEED;
} OZ_BI_end

// ---------------------------------------------------------------------
// NAMES
// ---------------------------------------------------------------------

OZ_BI_define(BInewName,0,1)
{
  OZ_RETURN(oz_newName());
} OZ_BI_end

OZ_BI_define(BInewUniqueName,1,1)
{
  oz_declareAtomIN(0,name);
  OZ_RETURN(oz_uniqueName(name));
} OZ_BI_end

OZ_BI_define(BInewNamedName,1,1)
{
  oz_declareAtomIN(0,printName);
  Literal *lit = NamedName::newNamedName(printName);
  OZ_RETURN(makeTaggedLiteral(lit));
} OZ_BI_end

OZ_BI_define(BInameLess,2,1)
{
  oz_declareNonvarIN(0,name1);
  oz_declareNonvarIN(1,name2);
  if (!oz_isName(name1)) {
    oz_typeError(0,"Name");
  } else if (!oz_isName(name2)) {
    oz_typeError(1,"Name");
  } else {
    OZ_RETURN_BOOL(atomcmp(tagged2Literal(name1),tagged2Literal(name2)) < 0);
  }
} OZ_BI_end

OZ_BI_define(BInameHash,1,1)
{
  oz_declareNonvarIN(0,name);
  if (!oz_isName(name)) {
    oz_typeError(0,"Name");
  } else {
    OZ_RETURN_INT(tagged2Literal(name)->hash());
  }
} OZ_BI_end

OZ_BI_define(BInameToString,1,1)
{
  oz_declareNonvarIN(0,name);
  if (!oz_isName(name)) {
    oz_typeError(0,"Name");
  } else {
    Literal *literal = tagged2Literal(name);
    if (literal->isUniqueName()) {
      OZ_RETURN(oz_atom(literal->getPrintName()));
    } else {
      Name *name = (Name *) literal;
      GName *gname = name->globalize();
      TimeStamp *ts = gname->site->getTimeStamp();
      static char s[256];
      sprintf(s, "%u:%u:%u:%ld", ts->pid,
	      gname->id.getNumber(1), gname->id.getNumber(0),
	      (unsigned long) ts->start);
      OZ_RETURN(oz_atom(s));
    }
  }
} OZ_BI_end

// ---------------------------------------------------------------------
// term type
// ---------------------------------------------------------------------

OZ_BI_define(BItermType,1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(OZ_termType(term));
} OZ_BI_end


OZ_Term oz_status(OZ_Term term)
{
  DEREF(term, _1);

  Assert(!oz_isRef(term));
  if (oz_isVarOrRef(term)) {
    OzVariable *cv = tagged2Var(term);
    VarStatus status = oz_check_var_status(cv);
      
    switch (status) {
    case EVAR_STATUS_FREE: 
      return AtomFree;
    case EVAR_STATUS_READONLY:
      return AtomFuture;
    case EVAR_STATUS_FAILED:
      return AtomFailed;
    case EVAR_STATUS_DET:
    case EVAR_STATUS_UNKNOWN:
      return _var_status(cv);
    case EVAR_STATUS_KINDED:
      break;
    default:
      Assert(0);
    }

    SRecord *t = SRecord::newSRecord(AtomKinded, 1);

    switch (cv->getType()) {
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
      t->setArg(0, AtomInt); break;
    case OZ_VAR_FS:
      t->setArg(0, AtomFSet); break;
    case OZ_VAR_OF:
      t->setArg(0, AtomRecord); break;
    case OZ_VAR_EXT:
      // Check if the variable is a gecode variable
      if (oz_isGeVar(term)) {
	t->setArg(0,oz_atom("gevar"));
	break;
      }
    default:
      t->setArg(0, AtomOther); break;
    }
    return makeTaggedSRecord(t);
  }

  SRecord *t = SRecord::newSRecord(AtomDet, 1);
  t->setArg(0, OZ_termType(term));
  return makeTaggedSRecord(t);
}

OZ_BI_define(BIstatus,1,1)
{
  oz_declareIN(0,term);
  OZ_RETURN(oz_status(term));
} OZ_BI_end

// ---------------------------------------------------------------------
// Builtins ==, \=, ==B and \=B
// ---------------------------------------------------------------------

inline
OZ_Return oz_eqeq(TaggedRef Ain,TaggedRef Bin)
{
  // simulate a shallow guard
  trail.pushMark();
  am.setEqEqMode();
  OZ_Return ret = oz_unify(Ain,Bin);
  am.unsetEqEqMode();

  if (ret == PROCEED) {
    if (trail.isEmptyChunk()) {
      trail.popMark();
      return PROCEED;
    }

    trail.unwindEqEq();
    return SUSPEND;
  }

  trail.unwindFailed();
  return ret;
}

inline
OZ_Return eqeqWrapper(TaggedRef Ain, TaggedRef Bin)
{
  TaggedRef A = Ain, B = Bin;
  DEREF(A,aPtr); DEREF(B,bPtr);

  if (oz_isToken(A) && oz_isToken(B))
    return oz_eq(A,B) ? PROCEED : FAILED;

  Assert(!oz_isRef(A));
  if (A == B && !oz_isVarOrRef(A))
    return PROCEED;

  return oz_eqeq(Ain,Bin);
}


OZ_Return neqInline(TaggedRef A, TaggedRef B, TaggedRef &out);
OZ_Return eqeqInline(TaggedRef A, TaggedRef B, TaggedRef &out);


OZ_BI_define(BIneqB,2,1)
{
  return neqInline(OZ_in(0),OZ_in(1),OZ_out(0));
} OZ_BI_end

OZ_BI_define(BIeqB,2,1)
{
  return eqeqInline(OZ_in(0),OZ_in(1),OZ_out(0));
} OZ_BI_end


OZ_Return eqeqInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch(eqeqWrapper(A,B)) {
  case PROCEED:
    out = oz_true();
    return PROCEED;
  case FAILED:
    out = oz_false();
    return PROCEED;
  case BI_REPLACEBICALL:
    return (BI_REPLACEBICALL);
  case RAISE:
    return RAISE;
  default:
    return SUSPEND;
  }
}


OZ_Return neqInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch(eqeqWrapper(A,B)) {
  case PROCEED:
    out = oz_false();
    return PROCEED;
  case FAILED:
    out = oz_true();
    return PROCEED;
  case BI_REPLACEBICALL:
    return (BI_REPLACEBICALL);
  case RAISE:
    return RAISE;
    // case SUSPEND:
  default:
    return SUSPEND;
  }
}

// ---------------------------------------------------------------------
// String
// ---------------------------------------------------------------------

OZ_BI_define(BIisString,1,1)
{
  OZ_Term in=OZ_in(0);

  OZ_Term var;
  if (!OZ_isString(in,&var)) {
    if (var == 0) OZ_RETURN(oz_false());
    oz_suspendOn(var);
  }
  OZ_RETURN(oz_true());
} OZ_BI_end



/********************************************************************
 * Records
 ******************************************************************** */


OZ_BI_define(BIadjoin,2,1)
{
  oz_declareNonvarIN(0,t0);
  oz_declareNonvarIN(1,t1);

  if (oz_isLiteral(t0)) {
    if (oz_isRecord(t1)) OZ_RETURN(t1);
    oz_typeError(1,"Record");
  }
  if (oz_isRecord(t0)) {
    SRecord *rec= makeRecord(t0);
    if (oz_isLiteral(t1)) {
      SRecord *newrec = SRecord::newSRecord(rec);
      newrec->setLabelForAdjoinOpt(t1);
      OZ_RETURN(newrec->normalize());
    }
    if (oz_isRecord(t1)) {
      OZ_RETURN(oz_adjoin(rec,makeRecord(t1)));
    }
    oz_typeError(1,"Record");
  }
  oz_typeError(0,"Record");
} OZ_BI_end

OZ_BI_define(BIadjoinAt,3,1)
{
  oz_declareNonvarIN(0,rec);
  oz_declareNonvarIN(1,fea);
  oz_declareIN(2,value);

  if (!oz_isFeature(fea)) oz_typeError(1,"Feature");

  if (oz_isLiteral(rec)) {
    SRecord *newrec
      = SRecord::newSRecord(rec,aritytable.find(oz_cons(fea,oz_nil())));
    newrec->setArg(0,value);
    OZ_RETURN(makeTaggedSRecord(newrec));
  }
  if (oz_isRecord(rec)) {
    OZ_RETURN(oz_adjoinAt(makeRecord(rec),fea,value));
  }
  oz_typeError(0,"Record");
} OZ_BI_end

inline
TaggedRef getArityFromList(TaggedRef list, Bool isPair) {
  TaggedRef arity;
  TaggedRef *next=&arity;
  Bool updateFlag=NO;
  list = oz_safeDeref(list);
  TaggedRef old = list;

  if (oz_isRef(list))
    return list;

loop:
  Assert(!oz_isRef(list));
  if (oz_isLTupleOrRef(list)) {
    TaggedRef fea;

    if (isPair) {
      TaggedRef pair = oz_safeDeref(oz_head(list));

      if (oz_isRef(pair))
	return pair;

      if (!oz_isPair2(pair))
	return 0;

      fea = tagged2SRecord(pair)->getArg(0);
    } else {
      fea = oz_head(list);
    }

    fea = oz_safeDeref(fea);
    if (oz_isRef(fea))
      return fea;

    if (!oz_isFeature(fea))
      return 0;

    LTuple *lt=new LTuple();
    *next=makeTaggedLTuple(lt);
    lt->setHead(fea);
    next=lt->getRefTail();

    list = oz_safeDeref(oz_tail(list));
    if (oz_isRef(list))
      return list;

    // cycle check:
    if (list == old) return 0;
    if (updateFlag)
      old = oz_deref(oz_tail(old));
    updateFlag=1-updateFlag;

    goto loop;
  }

  if (oz_isNil(list)) {
    *next=oz_nil();
    return arity;
  }

  return 0;
}

/* common subroutine for builtins adjoinList and record:
   recordFlag=OK: 'adjoinList' allows records as first arg
              N0: 'makeRecord' only allows literals */
static
OZ_Return adjoinPropListInline(TaggedRef t0, TaggedRef list, TaggedRef &out,
			   Bool recordFlag)
{
  TaggedRef arity=getArityFromList(list,OK);
  if (!arity) {
    oz_typeError(1,"finite list(Feature#Value)");
  }
  DEREF(t0,t0Ptr);
  if (oz_isRef(arity)) { // must suspend
    out=arity;
    switch (tagged2ltag(t0)) {
    case LTAG_LITERAL:
      return SUSPEND;
    case LTAG_SRECORD0: case LTAG_SRECORD1:
    case LTAG_LTUPLE0:  case LTAG_LTUPLE1:
      if (recordFlag) {
	return SUSPEND;
      }
      goto typeError0;
    case LTAG_VAR0: case LTAG_VAR1:
      if (oz_isKindedVar(t0) && tagged2Var(t0)->getType()!=OZ_VAR_OF)
	goto typeError0;
      if (recordFlag) {
	return SUSPEND;
      }
      goto typeError0;
    default:
      goto typeError0;
    }
  }

  if (oz_isNil(arity)) { // adjoin nothing
    switch (tagged2ltag(t0)) {
    case LTAG_SRECORD0: case LTAG_SRECORD1:
    case LTAG_LTUPLE0:  case LTAG_LTUPLE1:
      if (recordFlag) {
	out = t0;
	return PROCEED;
      }
      goto typeError0;
    case LTAG_LITERAL:
      out = t0;
      return PROCEED;
    case LTAG_VAR0: case LTAG_VAR1:
      if (oz_isKindedVar(t0) && tagged2Var(t0)->getType()!=OZ_VAR_OF)
	goto typeError0;
      out=makeTaggedRef(t0Ptr);
      return SUSPEND;
    default:
      goto typeError0;
    }
  }

  switch (tagged2ltag(t0)) {
  case LTAG_LITERAL:
    {
      int len1 = oz_fastlength(arity);
      arity = sortlist(arity,len1);
      int len = oz_fastlength(arity); // NOTE: duplicates may be removed
      if (!recordFlag && len!=len1) {  // handles case f(a:_ a:_)
	return oz_raise(E_ERROR,E_KERNEL,"recordConstruction",2,
			t0,list
			);
      }
      SRecord *newrec = SRecord::newSRecord(t0,aritytable.find(arity));
      newrec->setFeatures(list);
      out = newrec->normalize();
      return PROCEED;
    }
  case LTAG_SRECORD0: case LTAG_SRECORD1:
  case LTAG_LTUPLE0:  case LTAG_LTUPLE1:
    if (recordFlag) {
      out = oz_adjoinList(makeRecord(t0),arity,list);
      return PROCEED;
    }
    goto typeError0;
  case LTAG_VAR0: case LTAG_VAR1:
    if (oz_isKindedVar(t0) && tagged2Var(t0)->getType()!=OZ_VAR_OF)
        goto typeError0;
    out=makeTaggedRef(t0Ptr);
    return SUSPEND;
  default:
    goto typeError0;
  }

 typeError0:
  if (recordFlag) {
    oz_typeError(0,"Record");
  } else {
    oz_typeError(0,"Literal");
  }
}

// extern
OZ_Return adjoinPropList(TaggedRef t0, TaggedRef list, TaggedRef &out,
		     Bool recordFlag) 
{
  return adjoinPropListInline(t0,list,out,recordFlag);
}


OZ_BI_define(BIadjoinList,2,1)
{
  OZ_Return state = adjoinPropListInline(OZ_in(0),OZ_in(1),OZ_out(0),OK);
  switch (state) {
  case SUSPEND:
    oz_suspendOn(OZ_out(0));
  default:
    return state;
  }
} OZ_BI_end


OZ_BI_define(BImakeRecord,2,1) {

  OZ_Return state = adjoinPropListInline(OZ_in(0),OZ_in(1),OZ_out(0),NO);
  switch (state) {
  case SUSPEND:
    oz_suspendOn(OZ_out(0));
    return PROCEED;
  default:
    return state;
  }
} OZ_BI_end


OZ_BI_define(BIrealMakeRecord,2,1) {
  TaggedRef t0   = OZ_in(0);
  TaggedRef list = OZ_in(1);
  
  TaggedRef arity = getArityFromList(list,NO);

  if (!arity) {
    oz_typeError(1,"finite list(Feature)");
  }

  DEREF(t0,t0Ptr);

  if (oz_isNil(arity)) { // adjoin nothing
    if (oz_isLiteral(t0)) {
      OZ_RETURN(t0);
    }
    if (!oz_isKinded(t0)) {
      oz_suspendOnPtr(t0Ptr);
    }
    goto typeError0;
  }

  if (oz_isRef(arity)) { // must suspend
    Assert(!oz_isRef(t0));
    if (oz_isLiteral(t0) || 
	(oz_isVarOrRef(t0) && !oz_isKindedVar(t0))) {
      oz_suspendOn(arity);
    } else {
      goto typeError0;
    }
  }
  
  if (oz_isLiteral(t0)) {
    int len1 = oz_fastlength(arity);
    arity = sortlist(arity,len1);
    int len = oz_fastlength(arity); // NOTE: duplicates may be removed
    if (len!=len1) {  // handles case f(a:_ a:_)
      return oz_raise(E_ERROR,E_KERNEL,"recordConstruction",2,
		      t0,list
		      );
    }
    SRecord *newrec = SRecord::newSRecord(t0,aritytable.find(arity));
    newrec->initArgs();
    OZ_RETURN(newrec->normalize());
  }

  Assert(!oz_isRef(t0));
  if (oz_isVarOrRef(t0) && !oz_isKindedVar(t0)) {
    oz_suspendOnPtr(t0Ptr);
  }
  
 typeError0:
  oz_typeError(0,"Literal");
  
} OZ_BI_end


OZ_BI_define(BIcloneRecord,1,1) {
  oz_declareNonvarIN(0,rec);

  if (oz_isLiteral(rec)) {
    OZ_RETURN(rec);
  }

  TaggedRef ov = am.getCurrentOptVar();
  
  if (oz_isSRecord(rec)) {
    SRecord *in  = tagged2SRecord(rec);
    SRecord *out = SRecord::newSRecord(in->getLabel(),in->getArity());
    
    for (int i=in->getWidth(); i--; )
      out->setArg(i,ov);
    
    OZ_RETURN(makeTaggedSRecord(out));
  }

  Assert(!oz_isRef(rec));
  if (oz_isLTupleOrRef(rec)) {
    OZ_RETURN(oz_cons(ov,ov));
  }

  oz_typeError(0,"Record");
  
} OZ_BI_end


OZ_BI_define(BIrecordToDictionary,1,1) {
  oz_declareNonvarIN(0,rec);

  OzDictionary * dict;
  Board * bh = oz_currentBoard();

  Assert(!oz_isRef(rec));
  if (oz_isLiteral(rec)) {
    dict = new OzDictionary(bh);
  } else if (oz_isLTupleOrRef(rec)) {
    dict = new OzDictionary(bh);
    dict->setArg(makeTaggedSmallInt(1), oz_head(rec));
    dict->setArg(makeTaggedSmallInt(2), oz_tail(rec));
  } else if (oz_isSRecord(rec)) {
    SRecord * r = tagged2SRecord(rec);
    int size = r->getWidth();
    dict = new OzDictionary(bh,size);

    if (r->isTuple()) {
      for (int i=size; i--; )
	dict->setArg(makeTaggedSmallInt(i+1),r->getArg(i));
    } else {
      TaggedRef as = r->getArityList();

      while (!oz_isNil(as)) {
	TaggedRef a = oz_head(as);
	dict->setArg(a,r->getFeature(a));
	as = oz_tail(as);
      }
    }
  } else {
    oz_typeError(0,"Record");
  }

  OZ_RETURN(makeTaggedConst(dict));
  
} OZ_BI_end

inline
OZ_Return BIarityInlineInline(TaggedRef term, TaggedRef &out) {
  DEREF(term,termPtr);

  Assert(!oz_isRef(term));
  if (oz_isVarOrRef(term)) {
    if (oz_isKindedVar(term) && !isGenOFSVar(term)) {
      goto type_error;
    }
    return SUSPEND;
  }
  out = getArityList(term);
  if (out) return PROCEED;
 type_error:
  oz_typeError(0,"Record");
}

OZ_Return BIarityInline(TaggedRef term, TaggedRef &out) {
  return BIarityInlineInline(term,out);
}

OZ_DECLAREBI_USEINLINEFUN1(BIarity,BIarityInlineInline)


// Builtins for Record Pattern-Matching

OZ_BI_define(BItestRecord,3,1)
{
  // type-check the inputs:
  oz_declareNonKindedIN(0,val);
  oz_declareNonvarIN(1,patLabel);
  oz_declareNonvarIN(2,patArityList);

  if (!oz_isLiteral(patLabel)) oz_typeError(1,"Literal");

  OZ_Term ret=oz_checkList(patArityList,OZ_CHECK_FEATURE);
  if (oz_isRef(ret))   oz_suspendOn(ret);
  if (oz_isFalse(ret)) oz_typeError(2,"finite list(Feature)");
  int len = tagged2SmallInt(ret);
  if (len==0)
    OZ_RETURN_BOOL(oz_eq(val,patLabel));

  // compute the pattern's arity:
  TaggedRef sortedPatArityList =
    sortlist(duplist(packlist(patArityList),len),len);
  if (oz_fastlength(sortedPatArityList) != len) {
    // duplicate features are not allowed
    return oz_raise(E_ERROR,E_KERNEL,"recordPattern",2,patLabel,patArityList);
  }
  Arity *patArity = aritytable.find(sortedPatArityList);

  // is the input a proper record (or can it still become one)?
  Assert(!oz_isRef(val));
  if (oz_isVarOrRef(val) && oz_isKindedVar(val) && isGenOFSVar(val)) {
    OzOFVariable *ofsvar = tagged2GenOFSVar(val);
    if (patArity->isTuple()) {
      if (ofsvar->disentailed(tagged2Literal(patLabel),patArity->getWidth())) {
	OZ_RETURN(oz_false());
      }
    } else {
      if (ofsvar->disentailed(tagged2Literal(patLabel),patArity)) {
	OZ_RETURN(oz_false());
      }
    }
    oz_suspendOnPtr(valPtr);
  }
  if (oz_isLiteral(val) || !oz_isRecord(val)) {
    // literals never match since the arity is always a non-empty list
    OZ_RETURN(oz_false());
  }
  // from here on we deal with a determined proper record

  // get the value's label and SRecordArity:
  TaggedRef valLabel;
  SRecordArity valSRA;
  if (oz_isSRecord(val)) {
    SRecord *rec = tagged2SRecord(val);
    valLabel = rec->getLabel();
    valSRA = rec->getSRecordArity();
  } else {
    Assert(oz_isLTupleOrRef(val));
    valLabel = AtomCons;
    valSRA = mkTupleWidth(2);
  }

  // do the records match?
  SRecordArity patSRA = (patArity->isTuple())?
    mkTupleWidth(patArity->getWidth()): mkRecordArity(patArity);
  if (oz_eq(valLabel,patLabel) && sameSRecordArity(valSRA,patSRA)) {
    OZ_RETURN(oz_true());
  } else {
    OZ_RETURN(oz_false());
  }
} OZ_BI_end

OZ_BI_define(BItestRecordLabel,2,1)
{
  oz_declareNonKindedIN(0,val);
  oz_declareNonvarIN(1,patLabel);
  if (!oz_isLiteral(patLabel)) {
    oz_typeError(1,"Literal");
  }

  // get value's label:
  TaggedRef valLabel;
  if (isGenOFSVar(val)) {
    valLabel = oz_safeDeref(tagged2GenOFSVar(val)->getLabel());
    if (oz_isRef(valLabel)) {
      oz_suspendOnPtr(valPtr);
    }
  } else if (oz_isLiteral(val)) {
    valLabel = val;
  } else if (!oz_isRecord(val)) {
    OZ_RETURN(oz_false());
  } else if (oz_isSRecord(val)) {
    valLabel = tagged2SRecord(val)->getLabel();
  } else {
    Assert(oz_isCons(val));
    valLabel = AtomCons;
  }

  // do the labels match?
  OZ_RETURN(oz_bool(oz_eq(patLabel,valLabel)));
} OZ_BI_end

OZ_BI_define(BItestRecordFeature,2,2)
{
  oz_declareIN(0,val);
  oz_declareIN(1,patFeature);
  TaggedRef out;
  OZ_Return ret = genericDot(val,patFeature,out,FALSE);
  switch (ret) {
  case SUSPEND:
    oz_suspendOnInArgs2;
  case FAILED:
    OZ_out(1) = oz_unit();
    OZ_RETURN(oz_false());
  case PROCEED:
    OZ_out(1) = out;
    OZ_RETURN(oz_true());
  default:
    return ret;
  }
} OZ_BI_end

OZ_BI_define(BIaritySublist,2,1)
{
  oz_declareNonvarIN(0,a);
  oz_declareNonvarIN(1,b);

  OZ_Term ar1, ar2;
  if (oz_isRecord(a)) {
    ar1 = OZ_arityList(a);
  } else if (oz_isLiteral(a)) {
    ar1 = AtomNil;
  } else {
    oz_typeError(0,"Record");
  }
  if (oz_isRecord(b)) {
    ar2 = OZ_arityList(b);
  } else if (oz_isLiteral(b)) {
    ar2 = AtomNil;
  } else {
    oz_typeError(1,"Record");
  }

  while (!OZ_isNil(ar1)) {
    while (!OZ_isNil(ar2) && !OZ_eq(OZ_head(ar1),OZ_head(ar2))) {
      ar2 = OZ_tail(ar2);
    }
    if (OZ_isNil(ar2)) {
      OZ_RETURN(oz_false());
    }
    ar1 = OZ_tail(ar1);
  }
  OZ_RETURN(oz_true());
} OZ_BI_end

/* -----------------------------------------------------------------------
   Numbers
   ----------------------------------------------------------------------- */

static OZ_Return bombArith(char *type)
{
  oz_typeError(-1,type);
}

#define suspendTest(A,B,test,type)			\
  Assert(!oz_isRef(A));					\
  Assert(!oz_isRef(B));					\
  if (oz_isVarOrRef(A)) {			       	\
    if (oz_isVarOrRef(B) || test(B)) { return SUSPEND; }\
    return bombArith(type);				\
  } 							\
  if (oz_isVarOrRef(B)) {				\
    if (oz_isNumber(A)) { return SUSPEND; }		\
  }							\
  return bombArith(type);


static OZ_Return suspendOnNumbers(TaggedRef A, TaggedRef B) 
{
  suspendTest(A,B,oz_isNumber,"int or float\nuniformly for all arguments");
}

inline Bool isNumOrAtom(TaggedRef t)
{
  return oz_isNumber(t) || oz_isAtom(t);
}

static OZ_Return suspendOnNumbersAndAtoms(TaggedRef A, TaggedRef B) 
{
  suspendTest(A,B,isNumOrAtom,"int, float or atom\nuniformly for all arguments");
}

static OZ_Return suspendOnFloats(TaggedRef A, TaggedRef B) 
{
  suspendTest(A,B,oz_isFloat,"Float");
}


static OZ_Return suspendOnInts(TaggedRef A, TaggedRef B) 
{
  suspendTest(A,B,oz_isInt,"Int");
}

#undef suspendTest





/* -----------------------------------
   Z = X op Y
   ----------------------------------- */

// Float x Float -> Float
inline
OZ_Return BIfdivInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);
  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(floatValue(A) / floatValue(B));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}


// Int x Int -> Int
#define BIGOP(op)					\
  if (oz_isBigInt(A)) {					\
    if (oz_isBigInt(B)) {				\
      out = tagged2BigInt(A)->op(tagged2BigInt(B));	\
      return PROCEED;					\
    }							\
    if (oz_isSmallInt(B)) {				\
      BigInt *b = new BigInt(tagged2SmallInt(B));	\
      out = tagged2BigInt(A)->op(b);			\
      b->dispose();					\
      return PROCEED;					\
    }							\
  }							\
  if (oz_isBigInt(B)) {					\
    if (oz_isSmallInt(A)) {				\
      BigInt *a = new BigInt(tagged2SmallInt(A));	\
      out = a->op(tagged2BigInt(B));			\
      a->dispose();					\
      return PROCEED;					\
    }							\
  }

// Integer x Integer -> Integer
inline
OZ_Return BIdivInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isSmallInt(B) && tagged2SmallInt(B) == 0) {
    if (oz_isSmallInt(A) || oz_isBigInt(A)) {
      return oz_raise(E_ERROR,E_KERNEL,"div0",1,A);
    } else {
      return bombArith("Int");
    }
  }

  if (oz_isSmallInt(A) && oz_isSmallInt(B)) {
    out = makeTaggedSmallInt(tagged2SmallInt(A) / tagged2SmallInt(B));
    return PROCEED;
  }
  BIGOP(div);
  return suspendOnInts(A,B);
}

// Integer x Integer -> Integer
inline
OZ_Return BImodInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isSmallInt(B) && (tagged2SmallInt(B) == 0)) {
    if (oz_isSmallInt(A) || oz_isBigInt(A)) {
      return oz_raise(E_ERROR,E_KERNEL,"mod0",1,A);
    } else {
      return bombArith("Int");
    }
  }
  
  if (oz_isSmallInt(A) && oz_isSmallInt(B)) {
    out = makeTaggedSmallInt(tagged2SmallInt(A) % tagged2SmallInt(B));
    return PROCEED;
  }

  BIGOP(mod);
  return suspendOnInts(A,B);
}


/* Division is slow on RISC (at least SPARC)
 *  --> first make a simpler test for no overflow
 */

inline
int multOverflow(int a, int b)
{
  int absa = ozabs(a);
  int absb = ozabs(b);
  const int bits = (sizeof(TaggedRef)*8-LTAG_BITS)/2 - 1;

  if (!((absa|absb)>>bits)) /* if none of the 13 MSB in neither a nor b are set */
    return NO;
  return ((b!=0) && (absa >= OzMaxInt / absb));
}

inline
OZ_Return BImultInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isSmallInt(A) && oz_isSmallInt(B)) {
    int valA = tagged2SmallInt(A);
    int valB = tagged2SmallInt(B);
    if ( multOverflow(valA,valB) ) {
      BigInt *a = new BigInt(valA);
      BigInt *b = new BigInt(valB);
      out = a->mul(b);
      a->dispose();
      b->dispose();
      return PROCEED;
    } else {
      out = makeTaggedSmallInt(valA*valB);
      return PROCEED;
    }
  }
  
  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(floatValue(A) * floatValue(B));
    return PROCEED;
  }
  
  BIGOP(mul);
  return suspendOnNumbers(A,B);
}

OZ_Return BIminusInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isSmallInt(A) && oz_isSmallInt(B)) {
    out = oz_int(tagged2SmallInt(A) - tagged2SmallInt(B));
    return PROCEED;
  } 

  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(floatValue(A) - floatValue(B));
    return PROCEED;
  }

  
  BIGOP(sub);
  return suspendOnNumbers(A,B);
}

OZ_Return BIplusInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isSmallInt(A) && oz_isSmallInt(B)) {
    out = oz_int(tagged2SmallInt(A) + tagged2SmallInt(B));
    return PROCEED;
  } 

  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(floatValue(A) + floatValue(B));
    return PROCEED;
  }

  BIGOP(add);
  return suspendOnNumbers(A,B);
}

#undef BIGOP

/* -----------------------------------
   Z = op X
   ----------------------------------- */

// unary minus: Number -> Number
inline
OZ_Return BIuminusInline(TaggedRef A, TaggedRef &out)
{
  A = oz_deref(A);

  if (oz_isSmallInt(A)) {
    out = makeTaggedSmallInt(-tagged2SmallInt(A));
    return PROCEED;
  }

  if (oz_isFloat(A)) {
    out = oz_float(-floatValue(A));
    return PROCEED;
  }

  if (oz_isBigInt(A)) {
    out = tagged2BigInt(A)->neg();
    return PROCEED;
  }

  Assert(!oz_isRef(A));
  if (oz_isVarOrRef(A)){
    return SUSPEND;
  }

  oz_typeError(0,"Number");

}

inline
OZ_Return BIabsInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1);

  if (oz_isSmallInt(A)) {
    int i = tagged2SmallInt(A);
    out = (i >= 0) ? A : makeTaggedSmallInt(-i);
    return PROCEED;
  }

  if (oz_isFloat(A)) {
    double f = floatValue(A);
    out = (f >= 0.0) ? A : oz_float(fabs(f));
    return PROCEED;
  }

  if (oz_isBigInt(A)) {
    BigInt *b = tagged2BigInt(A);
    out = (b->cmp(0l) >= 0) ? A : b->neg();
    return PROCEED;
  }

  Assert(!oz_isRef(A));
  if (oz_isVarOrRef(A)){
    return SUSPEND;
  }

  oz_typeError(0,"Number");
}

// add1(X) --> X+1
inline
OZ_Return BIadd1Inline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1);

  if (oz_isSmallInt(A)) {
    /* INTDEP */
    int res = (int)A + (1<<LTAG_BITS);
    if ((int)A < res) {
      out = res;
      return PROCEED;
    }
  }

  return BIplusInline(A,makeTaggedSmallInt(1),out);
}

// sub1(X) --> X-1
inline
OZ_Return BIsub1Inline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1);

  if (oz_isSmallInt(A)) {
    /* INTDEP */
    int res = (int)A - (1<<LTAG_BITS);
    if ((int)A > res) {
      out = res;
      return PROCEED;
    }
  }
  
  return BIminusInline(A,makeTaggedSmallInt(1),out);
}


/* -----------------------------------
   X test Y
   ----------------------------------- */

inline
OZ_Return bigintLess(BigInt *A, BigInt *B)
{
  return (A->cmp(B) < 0 ? PROCEED : FAILED);
}


inline
OZ_Return bigintLe(BigInt *A, BigInt *B)
{
  return (A->cmp(B) <= 0 ? PROCEED : FAILED);
}

inline
OZ_Return bigtest(TaggedRef A, TaggedRef B,
		  OZ_Return (*test)(BigInt*, BigInt*))
{
  if (oz_isBigInt(A)) {
    if (oz_isBigInt(B)) {
      return test(tagged2BigInt(A),tagged2BigInt(B));
    }
    if (oz_isSmallInt(B)) {
      BigInt *b = new BigInt(tagged2SmallInt(B));
      OZ_Return res = test(tagged2BigInt(A),b);
      b->dispose();
      return res;
    }
  }
  if (oz_isBigInt(B)) {
    if (oz_isSmallInt(A)) {
      BigInt *a = new BigInt(tagged2SmallInt(A));
      OZ_Return res = test(a,tagged2BigInt(B));
      a->dispose();
      return res;
    }
  }

  Assert(!oz_isRef(A));
  Assert(!oz_isRef(B));
  if (oz_isVarOrRef(A) || oz_isVarOrRef(B)) 
    return SUSPEND;

  oz_typeError(-1,"int, float or atom\nuniformly for all arguments");
}



inline
OZ_Return BIminInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1); 
  DEREF(B,_2);

  if (oz_isSmallInt(A) && oz_isSmallInt(B)) {
    out = (smallIntLess(A,B) ? A : B);
    return PROCEED;
  }
  if (oz_isAtom(A) && oz_isAtom(B)) {
    out = (strcmp(tagged2Literal(A)->getPrintName(),
		  tagged2Literal(B)->getPrintName()) < 0)
      ? A : B;
    return PROCEED;
  }
  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = (floatValue(A) < floatValue(B)) ? A : B; 
    return PROCEED;
  }
  if (oz_isInt(A) && oz_isInt(B)) {
    OZ_Return ret = bigtest(A,B,bigintLess);
    switch (ret) {
    case PROCEED: out = A; return PROCEED;
    case FAILED:  out = B; return PROCEED;
    case RAISE:   return RAISE;
    default:      break;
    }
  }

  Assert(!oz_isRef(A));
  Assert(!oz_isRef(B));
  if (oz_isVarOrRef(A) || oz_isVarOrRef(B))
    return suspendOnNumbersAndAtoms(A,B);

  oz_typeError(-1,"Comparable");
}


/* code adapted from min */
inline
OZ_Return BImaxInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1); 
  DEREF(B,_2);

  if (oz_isSmallInt(A) && oz_isSmallInt(B)) {
    out = (smallIntLess(A,B) ? B : A);
    return PROCEED;
  }
  if (oz_isAtom(A) && oz_isAtom(B)) {
    out = (strcmp(tagged2Literal(A)->getPrintName(),
		  tagged2Literal(B)->getPrintName()) < 0)
      ? B : A;
    return PROCEED;
  }
  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = (floatValue(A) < floatValue(B)) ? B : A; 
    return PROCEED;
  }
  if (oz_isInt(A) && oz_isInt(B)) {
    OZ_Return ret = bigtest(A,B,bigintLess);
    switch (ret) {
    case PROCEED: out = B; return PROCEED;
    case FAILED:  out = A; return PROCEED;
    case RAISE:   return RAISE;
    default:      break;
    }
  }

  Assert(!oz_isRef(A));
  Assert(!oz_isRef(B));
  if (oz_isVarOrRef(A) || oz_isVarOrRef(B))
    return suspendOnNumbersAndAtoms(A,B);

  oz_typeError(-1,"Comparable");
}

inline
OZ_Return BIlessInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isSmallInt(A) && oz_isSmallInt(B))
    return (smallIntLess(A,B) ? PROCEED : FAILED);
  if (oz_isAtom(A) && oz_isAtom(B))
    return (strcmp(tagged2Literal(A)->getPrintName(),
		   tagged2Literal(B)->getPrintName()) < 0)
      ? PROCEED : FAILED;
  if (oz_isFloat(A) && oz_isFloat(B))
    return (floatValue(A) < floatValue(B)) ? PROCEED : FAILED;
  if (oz_isInt(A) && oz_isInt(B)) {
    OZ_Return ret = bigtest(A,B,bigintLess); 
    if (ret!=SUSPEND) 
      return ret;
  }

  Assert(!oz_isRef(A));
  Assert(!oz_isRef(B));
  if (oz_isVarOrRef(A) || oz_isVarOrRef(B))
    return suspendOnNumbersAndAtoms(A,B);
  oz_typeError(-1,"Comparable");
}

inline
OZ_Return BIlessInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  OZ_Return ret = BIlessInline(A,B);
  switch (ret) {
  case PROCEED: out = oz_true();  return PROCEED;
  case FAILED:  out = oz_false(); return PROCEED;
  default:      return ret;
  }
}

inline
OZ_Return BIgreatInline(TaggedRef A, TaggedRef B)
{
  return BIlessInline(B,A);
}

inline
OZ_Return BIgreatInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return BIlessInlineFun(B,A,out);
}

inline
OZ_Return BIleInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isSmallInt(A) && oz_isSmallInt(B))
    return (smallIntLE(A,B) ? PROCEED : FAILED);
  if (oz_isAtom(A) && oz_isAtom(B))
    return (strcmp(tagged2Literal(A)->getPrintName(),
		   tagged2Literal(B)->getPrintName()) <= 0)
      ? PROCEED : FAILED;
  if (oz_isFloat(A) && oz_isFloat(B))
    return (floatValue(A) <= floatValue(B)) ? PROCEED : FAILED;
  if (oz_isInt(A) && oz_isInt(B)) {
    OZ_Return ret = bigtest(A,B,bigintLe); 
    if (ret!=SUSPEND) 
      return ret;
  }

  Assert(!oz_isRef(A));
  Assert(!oz_isRef(B));
  if (oz_isVarOrRef(A) || oz_isVarOrRef(B))
    return suspendOnNumbersAndAtoms(A,B);
  oz_typeError(-1,"Comparable");
}

inline
OZ_Return BIleInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  OZ_Return ret = BIleInline(A,B);
  switch (ret) {
  case PROCEED: out = oz_true();  return PROCEED;
  case FAILED:  out = oz_false(); return PROCEED;
  default:      return ret;
  }
}


inline
OZ_Return BIgeInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return BIleInlineFun(B,A,out);
}

inline
OZ_Return BIgeInline(TaggedRef A, TaggedRef B)
{
  return BIleInline(B,A);
}

OZ_Return BILessOrLessEq(Bool callLess, TaggedRef A, TaggedRef B)
{
  return callLess ? BIlessInline(A,B) : BIleInline(A,B);
}


/* -----------------------------------
   X = conv(Y)
   ----------------------------------- */

inline
OZ_Return BIintToFloatInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1);
  if (oz_isSmallInt(A)) {
    out = oz_float((double)tagged2SmallInt(A));
    return PROCEED;
  }
  if (oz_isBigInt(A)) {
    char *s = toC(A);
    out = OZ_CStringToFloat(s);
    return PROCEED;
  }

  Assert(!oz_isRef(A));
  if (oz_isVarOrRef(A)) {
    return SUSPEND;
  }

  oz_typeError(0,"Int");
}

/* mm2: I don't know if this is efficient, but it's only called,
   when rounding "xx.5" */
inline
Bool ozisodd(double ff)
{
  double m = ff/2;
  return m != floor(m);
}

/* ozround -- round float to int as required by IEEE Standard */
inline
double ozround(double in) {
  double ff = floor(in);
  double diff = in-ff;
  if (diff > 0.5 || (diff == 0.5 && ozisodd(ff))) {
    ff += 1;
  }
  return ff;
}

inline
OZ_Return BIfloatToIntInline(TaggedRef A, TaggedRef &out) {
  A=oz_deref(A);

  Assert(!oz_isRef(A));
  if (oz_isVarOrRef(A))
    return SUSPEND;

  if (oz_isFloat(A)) {
    double ff = ozround(floatValue(A));
    if (ff > INT_MAX || ff < INT_MIN) {
      OZ_warning("float to int: truncated to signed 32 Bit\n");
    }
    out = oz_int((int) ff);
    return PROCEED;
  }

  oz_typeError(-1,"Float");
}

OZ_BI_define(BIfloatToString, 1,1)
{
  oz_declareNonvarIN(0,in);

  if (oz_isFloat(in)) {
    char *s = OZ_toC(in,100,100); // mm2
    OZ_RETURN(OZ_string(s));
  }
  oz_typeError(0,"Float");
} OZ_BI_end

OZ_BI_define(BIstringToFloat, 1,1)
{
  oz_declareProperStringIN(0,str);

  char *end = OZ_parseFloat(str);
  if (!end || *end != 0) {
    return oz_raise(E_ERROR,E_KERNEL,"stringNoFloat",1,OZ_in(0));
  }
  OZ_RETURN(OZ_CStringToFloat(str));
} OZ_BI_end

OZ_BI_define(BIstringToInt, 1,1)
{
  oz_declareProperStringIN(0,str);

  if (!str) return oz_raise(E_ERROR,E_KERNEL,"stringNoInt",1,OZ_in(0));


  OZ_Term res = OZ_CStringToInt(str);
  if (res == 0)
    return oz_raise(E_ERROR,E_KERNEL,"stringNoInt",1,OZ_in(0));
  else
    OZ_RETURN(res);
} OZ_BI_end

OZ_BI_define(BIintToString, 1,1)
{
  oz_declareNonvarIN(0,in);

  if (oz_isInt(in)) {
    OZ_RETURN(OZ_string(OZ_toC(in,100,100))); //mm2
  }
  oz_typeError(0,"Int");
} OZ_BI_end

/* -----------------------------------
   type X
   ----------------------------------- */



/* -----------------------------------------------------------------------
   misc. floating point functions
   ----------------------------------------------------------------------- */


#define FLOATFUN(Fun,BIName,InlineName)			\
inline                                                  \
OZ_Return InlineName(TaggedRef AA, TaggedRef &out)	\
{							\
  DEREF(AA,_);					        \
							\
  Assert(!oz_isRef(AA));				\
  if (oz_isVarOrRef(AA)) {			        \
    return SUSPEND;					\
  }							\
							\
  if (oz_isFloat(AA)) {				        \
    out = oz_float(Fun(floatValue(AA)));		\
    return PROCEED;					\
  }							\
  oz_typeError(0,"Float");				\
}							\
OZ_DECLAREBI_USEINLINEFUN1(BIName,InlineName)

#ifdef WINDOWS
// These we had to hack ourselves because they are not provided
// by Windows libraries:

inline
double asinh(double x) {
  return log(x + sqrt(x * x + 1.0));
}

inline
double acosh(double x) {
  return log(x + sqrt(x * x - 1.0));
}

inline
double atanh(double x) {
  if (fabs(x) > 1.0) {
    errno = EDOM;
    return asin(2.0);
  } else {
    return log((1.0 + x) / (1.0 - x)) / 2.0;
  }
}
#endif

FLOATFUN(exp, BIexp, BIinlineExp)
FLOATFUN(log, BIlog, BIinlineLog)
FLOATFUN(sqrt,BIsqrt,BIinlineSqrt) 
FLOATFUN(sin, BIsin, BIinlineSin) 
FLOATFUN(asin,BIasin,BIinlineAsin) 
FLOATFUN(cos, BIcos, BIinlineCos) 
FLOATFUN(acos,BIacos,BIinlineAcos) 
FLOATFUN(tan, BItan, BIinlineTan) 
FLOATFUN(atan,BIatan,BIinlineAtan) 
FLOATFUN(ceil,BIceil,BIinlineCeil)
FLOATFUN(floor,BIfloor,BIinlineFloor)
FLOATFUN(ozround, BIround, BIinlineRound)
FLOATFUN(sinh, BIsinh, BIinlineSinh)
FLOATFUN(cosh, BIcosh, BIinlineCosh)
FLOATFUN(tanh, BItanh, BIinlineTanh)
FLOATFUN(asinh, BIasinh, BIinlineAsinh)
FLOATFUN(acosh, BIacosh, BIinlineAcosh)
FLOATFUN(atanh, BIatanh, BIinlineAtanh)
#undef FLOATFUN


inline
OZ_Return BIfPowInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(pow(floatValue(A),floatValue(B)));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}

inline
OZ_Return BIfModInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(fmod(floatValue(A),floatValue(B)));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}

inline
OZ_Return BIatan2Inline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1);
  DEREF(B,_2);

  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(atan2(floatValue(A),floatValue(B)));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}


/* -----------------------------------
   make non inline versions
   ----------------------------------- */

OZ_DECLAREBI_USEINLINEFUN2(BIplus,BIplusInline)
OZ_DECLAREBI_USEINLINEFUN2(BIminus,BIminusInline)

OZ_DECLAREBI_USEINLINEFUN2(BImult,BImultInline)
OZ_DECLAREBI_USEINLINEFUN2(BIdiv,BIdivInline)
OZ_DECLAREBI_USEINLINEFUN2(BIfdiv,BIfdivInline)
OZ_DECLAREBI_USEINLINEFUN2(BImod,BImodInline)

OZ_DECLAREBI_USEINLINEFUN2(BIfPow,BIfPowInline)
OZ_DECLAREBI_USEINLINEFUN2(BIfMod,BIfModInline)
OZ_DECLAREBI_USEINLINEFUN2(BIatan2,BIatan2Inline)

OZ_DECLAREBI_USEINLINEFUN2(BImax,BImaxInline)
OZ_DECLAREBI_USEINLINEFUN2(BImin,BIminInline)

OZ_DECLAREBI_USEINLINEFUN2(BIlessFun,BIlessInlineFun)
OZ_DECLAREBI_USEINLINEFUN2(BIleFun,BIleInlineFun)
OZ_DECLAREBI_USEINLINEFUN2(BIgreatFun,BIgreatInlineFun)
OZ_DECLAREBI_USEINLINEFUN2(BIgeFun,BIgeInlineFun)

OZ_DECLAREBI_USEINLINEFUN1(BIintToFloat,BIintToFloatInline)
OZ_DECLAREBI_USEINLINEFUN1(BIfloatToInt,BIfloatToIntInline)
OZ_DECLAREBI_USEINLINEFUN1(BIuminus,BIuminusInline)
OZ_DECLAREBI_USEINLINEFUN1(BIabs,BIabsInline)
OZ_DECLAREBI_USEINLINEFUN1(BIadd1,BIadd1Inline)
OZ_DECLAREBI_USEINLINEFUN1(BIsub1,BIsub1Inline)

// ---------------------------------------------------------------------
// Ports
// ---------------------------------------------------------------------

// use VARS or READONLYS for ports
// #define VAR_PORT

#ifdef VAR_PORT

OZ_BI_define(BInewPort,1,1)
{
  oz_declareIN(0,val);

  OZ_RETURN(oz_newPort(val));
} OZ_BI_end


void doPortSend(PortWithStream *port,TaggedRef val)
{
  LTuple *lt = new LTuple(am.getCurrentOptVar(), am.getCurrentOptVar());
    
  OZ_Term old = ((PortWithStream*)port)->exchangeStream(lt->getTail());

  OZ_unifyInThread(old,makeTaggedLTuple(lt));
  OZ_unifyInThread(val,lt->getHead()); // might raise exception if val is non exportable
}

#else

OZ_BI_define(BInewPort,1,1)
{
  OZ_Term strm = oz_newReadOnly(oz_currentBoard());
  OZ_Term port = oz_newPort(strm);

  OZ_out(0) = port;
  return oz_unify(OZ_in(0), strm);     // beware: unification may suspend!
} OZ_BI_end

#define FAST_DOPORTSEND

#ifdef FAST_DOPORTSEND
void doPortSend(PortWithStream *port,TaggedRef val,Board * home) {
  if (home==(Board*)NULL || home==oz_currentBoard()) {
    OZ_Term newFut = oz_newReadOnly(oz_currentBoard());
    OZ_Term lt     = oz_cons(val,newFut);
    OZ_Term oldFut = port->exchangeStream(newFut);
    DEREF(oldFut,ptr);
    oz_bindReadOnly(ptr,lt);
  } else {
    // I believe this branch is only for sending to a port
    // in a super-ordinated space.  oz_sendPort has already
    // performed the check of situatedness.  We cannot perform
    // oz_bindReadOnly right here: it must be done in the port's
    // home space (this is required for properly waking up
    // suspensions, etc...).  In order to preserve order on
    // the stream, we must perform the exchange immediately.
    // *** HOWEVER I SEE NO REASON FOR INTRODUCING A VARIABLE
    // *** AND PERFORMING AN ADDITIONAL UNIFY
    OZ_Term newFut = oz_newReadOnly(home);

    // kost@ --> Denys: that's how it looked:
    //   OZ_Term newVar = oz_newVariable(home);
    //   OZ_Term lt     = oz_cons(newVar,newFut);
    //
    // My *theory* is that a variable was introduced at the point the
    // so-called "SB distribution model" was brought in (which is gone
    // by now, God bless). We are used to have "oz_cons(val, newFut)"
    // in the past (though not for this case, of course). Even more:
    // what was written is illegal 'cause it introduces potentially
    // observable variables that do not exists from the computation
    // model's point of view. Please correct me if i'm wrong.
    OZ_Term lt     = oz_cons(val,newFut);

    OZ_Term oldFut = port->exchangeStream(newFut);
    Thread * t = oz_newThreadInject(home);
    // kost@ : so, this one goes away as well:
    // t->pushCall(BI_Unify,RefsArray::make(val,newVar));
    t->pushCall(BI_bindReadOnly,RefsArray::make(oldFut,lt));
  }
}
#else
void doPortSend(PortWithStream *port,TaggedRef val,Board * home) {
  if (home != (Board *) NULL) {
    OZ_Term newFut = oz_newReadOnly(home);
    OZ_Term newVar = oz_newVariable(home);
    OZ_Term lt     = oz_cons(newVar,newFut);
    OZ_Term oldFut = port->exchangeStream(newFut);

    Thread * t = oz_newThreadInject(home);
    t->pushCall(BI_Unify,RefsArray::make(val,oz_head(lt)));
    t->pushCall(BI_bindReadOnly,RefsArray::make(oldFut,lt));
  } else {
    OZ_Term newFut = oz_newReadOnly(oz_currentBoard());
    OZ_Term lt     = oz_cons(am.getCurrentOptVar(), newFut);
    OZ_Term oldFut = port->exchangeStream(newFut);

    DEREF(oldFut,ptr);
    oz_bindReadOnly(ptr,lt);

    // might raise exception if val is non exportable
    OZ_unifyInThread(val,oz_head(lt)); 
  }
}
#endif

#endif


extern 
OZ_Return (*OZ_checkSituatedness)(Board *,TaggedRef *);


OZ_Return oz_sendPort(OZ_Term prt, OZ_Term val)
{
  Assert(oz_isPort(prt));

  Port *port  = tagged2Port(prt);

  Board * prt_home = port->getBoardInternal()->derefBoard();
  
  Bool sc_required = (prt_home != oz_currentBoard());
  
  if (sc_required) {
    // Perform situatedness check
    OZ_Return ret = (*OZ_checkSituatedness)(prt_home,&val);
    if (ret!=PROCEED)
      return ret;
  }
    
  if (port->isProxy()) {
    if (sc_required) {
      // Fork a thread to redo the send
      oz_newThreadInject(prt_home)->pushCall(BI_send,RefsArray::make(prt,val));
      return PROCEED;
    } else {
      return (*portSend)(port,val);
    }
  } 
  doPortSend((PortWithStream*)port,val,
	     sc_required ? prt_home : (Board *) NULL);
  return PROCEED;
}

OZ_BI_define(BIsendPort,2,0)
{
  oz_declareNonvarIN(0,prt);
  oz_declareIN(1,val);

  if (!oz_isPort(prt)) {
    oz_typeError(0,"Port");
  }

  return oz_sendPort(prt,val);
} OZ_BI_end

OZ_BI_define(BIsendRecvPort,2,1)
{
  oz_declareNonvarIN(0,prt);
  oz_declareIN(1,val);

  if (!oz_isPort(prt)) {
    oz_typeError(0,"Port");
  }

  TaggedRef rv = 
    oz_newVariable(tagged2Port(prt)->getBoardInternal()->derefBoard());

  TaggedRef ms = oz_pair2(val,rv);

  OZ_Return s = oz_sendPort(prt,ms);

  if (s != PROCEED)
    return s;

  OZ_RETURN(rv);
} OZ_BI_end


// ---------------------------------------------------------------------
// Locks
// ---------------------------------------------------------------------

OZ_BI_define(BInewLock,0,1)
{
  OZ_RETURN(makeTaggedConst(new LockLocal(oz_currentBoard())));
} OZ_BI_end

// ---------------------------------------------------------------------
// Cell
// ---------------------------------------------------------------------

OZ_BI_define(BInewCell,1,1)
{
  OZ_Term val = OZ_in(0);

  OZ_RETURN(oz_newCell(val));
} OZ_BI_end


OZ_Return accessCell(OZ_Term cell,OZ_Term &out)
{
  Tertiary *tert= (Tertiary *) tagged2Const(cell);
  if(!tert->isLocal()){
    out = oz_newVariable(); /* ATTENTION - clumsy */
    return (*cellDoAccess)(tert,out);
  }
  out = ((CellLocal*)tert)->getValue();
  return PROCEED;
}

OZ_Return exchangeCell(OZ_Term cell, OZ_Term newVal, OZ_Term &oldVal)
{
  Assert(!oz_isVar(newVal));
  Tertiary *tert = (Tertiary *) tagged2Const(cell);
  if(tert->isLocal()){
    CellLocal *cellLocal=(CellLocal*)tert;
    CheckLocalBoard(cellLocal,"cell");
    oldVal = cellLocal->exchangeValue(newVal);
    return PROCEED;
  } else {
    if(!tert->isProxy() && (tert->getInfo()==NULL)){
      CellSecEmul* sec;
      if(tert->getTertType()==Te_Frame){
	sec=((CellFrameEmul*)tert)->getSec();}
      else{
	sec=((CellManagerEmul*)tert)->getSec();}    
      if(sec->getState()==Cell_Lock_Valid){
	TaggedRef old=sec->getContents();
	sec->setContents(newVal);
	oldVal = old;
	return PROCEED;}}
    oldVal = oz_newVariable();
    return (*cellDoExchange)(tert,oldVal,newVal);
  }
}

OZ_BI_define(BIaccessCell,1,1)
{			
  oz_declareNonvarIN(0,cell);
  if (!oz_isCell(cell)) { oz_typeError(0,"Cell"); }

  OZ_Term out;
  int ret = accessCell(cell,out);
  OZ_result(out);
  return ret;
} OZ_BI_end

OZ_BI_define(BIassignCell,2,0)
{			
  oz_declareNonvarIN(0,cell);
  if (!oz_isCell(cell)) { oz_typeError(0,"Cell"); }
  oz_declareIN(1,newVal);
  // SaveDeref(newVal);
  OZ_Term oldIgnored;
  return exchangeCell(cell,newVal,oldIgnored);
} OZ_BI_end


OZ_BI_define(BIexchangeCellFun,2,1)
{			
  oz_declareNonvarIN(0,cell);
  if (!oz_isCell(cell)) { oz_typeError(0,"Cell"); }
  oz_declareIN(1,newVal);
  OZ_Term old;
  int ret = exchangeCell(cell,newVal,old);
  OZ_result(old);
  return ret;
} OZ_BI_end

/*
 * Control Vars
 */

OZ_Return applyProc(TaggedRef proc, TaggedRef args)
{
  OZ_Term var;
  if (!OZ_isList(args,&var)) {
    if (var == 0) oz_typeError(1,"finite List");
    oz_suspendOn(var);
  }

  int len = OZ_length(args);
  RefsArray * argsArray = RefsArray::allocate(len,NO);
  for (int i=0; i < len; i++) {
    argsArray->setArg(i,OZ_head(args));
    args=OZ_tail(args);
  }
  Assert(OZ_isNil(args));

  if (!oz_isProcedure(proc) && !oz_isObject(proc)) {
    oz_typeError(0,"Procedure or Object");
  }

  am.prepareCall(proc,argsArray);
  return BI_REPLACEBICALL;
}


OZ_BI_define(BIcontrolVarHandler,1,0)
{
  OZ_Term varlist = oz_deref(OZ_in(0));

  {
    TaggedRef aux = varlist;
    while (oz_isCons(aux)) {
      TaggedRef car = oz_head(aux);
      if (oz_isVarOrRef(oz_deref(car))) {
	(void) oz_addSuspendVarList(car);
	aux = oz_tail(aux);
      } else {
	am.emptySuspendVarList();
	goto no_suspend;
      }
    }
    /* only unbound variables found */
    return SUSPEND;
  }

no_suspend:
  for ( ; oz_isCons(varlist); varlist = oz_deref(oz_tail(varlist))) {
    TaggedRef car = oz_deref(oz_head(varlist));
    Assert(!oz_isRef(car));
    if (oz_isVarOrRef(car))
      continue;
    if (oz_isLiteral(car) && oz_eq(car,NameUnit))
      return PROCEED;

    if (!oz_isSTuple(car))
      goto bomb;

    SRecord *tpl = tagged2SRecord(car);
    TaggedRef label = tpl->getLabel();

    if (oz_eq(label,AtomUnify)) {
      Assert(OZ_width(car)==2);
      return oz_unify(tpl->getArg(0),tpl->getArg(1));
    }

    if (oz_eq(label,AtomException)) {
      Assert(OZ_width(car)==1);
      return OZ_raise(tpl->getArg(0));
    }

    if (oz_eq(label,AtomApply)) {
      Assert(OZ_width(car)==2);
      return applyProc(tpl->getArg(0),tpl->getArg(1));
    }

    if (oz_eq(label,AtomApplyList)) {
      Assert(OZ_width(car)==1);
      TaggedRef list = reverseC(oz_deref(tpl->getArg(0)));
      while(oz_isCons(list)) {
	TaggedRef car = oz_head(list);
	if (!OZ_isPair(car))
	  return oz_raise(E_ERROR,E_SYSTEM,"applyList: pair expected",1,car);
	OZ_Return aux = applyProc(OZ_getArg(car,0),OZ_getArg(car,1));
	if (aux != BI_REPLACEBICALL)
	  return aux;
	list = oz_deref(oz_tail(list));
      }
      return BI_REPLACEBICALL;
    }

    goto bomb;
  }
  
bomb:
  return oz_raise(E_ERROR,E_SYSTEM,"controlVarHandler: no action found",1,OZ_in(0));
} OZ_BI_end



/* -----------------------------------------------------------------
   Statistics
   ----------------------------------------------------------------- */


#ifdef MISC_BUILTINS

#ifdef PROFILE_INSTR
OZ_BI_define(BIinstructionsPrint, 0,0)
{
  ozstat.printInstr();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIinstructionsPrintCollapsable, 0,0)
{
  ozstat.printInstrCollapsable();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIinstructionsPrintReset, 0,0)
{
  ozstat.printInstrReset();
  return PROCEED;
} OZ_BI_end
#endif

#ifdef PROFILE_BI
OZ_BI_define(BIbiPrint, 0,0)
{
  unsigned long sum = 0;

  extern TaggedRef dictionary_of_builtins;
  OzDictionary * d = tagged2Dictionary(dictionary_of_builtins);
  for (int i=d->getFirst(); i>=0; i=d->getNext(i)) {
    TaggedRef v = d->getValue(i);
    if (v && oz_isBuiltin(v)) {
      Builtin * abit = tagged2Builtin(v);
      sum += abit->getCounter();
      if (abit->getCounter()!=0) {
	printf("%010lu x %s\n",abit->getCounter(),abit->getPrintName());
      }
    }
  }
  printf("----------\n%010lu\n",sum);
  return PROCEED;
} OZ_BI_end
#endif


#endif

/* -----------------------------------------------------------------
   dynamic link objects files
   ----------------------------------------------------------------- */

// currently the ozm format has lines mentionning
// foreign pointers with hexadecimal addresses.  these
// are use solely for identification purposes, but unfortunately
// vary from one computation to the next which means that
// the ozm file is always different even if abstractly it truly
// hasn't changed.  To fix this problem, foreign pointers
// should be assigned integers to serve as these identification
// labels, in the order in which they are encountered (which will
// be the same from one execution to the next - unless something
// major has changed).  To support this we need to keep a mapping
// from foreign pointers to integers: we must use a dictionary.
// however foreign pointers cannot be keys.  so we provide the
// function below to use the integer value of its address as the
// key instead.

OZ_BI_define(BIForeignPointerToInt,1,1)
{
  OZ_declareForeignPointer(0,handle);
  OZ_RETURN_INT((long)handle);
} OZ_BI_end


/* ------------------------------------------------------------
 * Shutdown
 * ------------------------------------------------------------ */

OZ_BI_define(BIshutdown,1,0)
{
  oz_declareIntIN(0,status);
  am.exitOz(status);
  Assert(0);
  return(PROCEED); /* not reached but anyway */
} OZ_BI_end

/* ------------------------------------------------------------
 * Alarm und Delay
 * ------------------------------------------------------------ */

OZ_BI_define(BIalarm,2,0) {
  oz_declareIntIN(0,t);
  oz_declareIN(1,out);

  if (!oz_onToplevel()) {
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("io"));
  }

  if (t <= 0) 
    return oz_unify(NameUnit,out);

#ifdef DENYS_EVENTS
  TaggedRef var = oz_newVariable();
  OZ_eventPush(OZ_mkTupleC("delay",2,OZ_in(0),var));
  OZ_in(0)=makeTaggedSmallInt(-1);
  return oz_unify(out,var);
#else
  am.insertUser(t,oz_cons(NameUnit,out));
  return PROCEED;
#endif
} OZ_BI_end

OZ_BI_define(BItimeTime,0,1) {
  time_t ttt;

  time(&ttt);
  
  struct tm * t = gmtime(&ttt);

  int tt = (t->tm_yday * 86400 +
	    t->tm_hour * 3600  +
	    t->tm_min  * 60 +
	    t->tm_sec);

  OZ_RETURN(makeTaggedSmallInt(tt));
} OZ_BI_end

/* ------------------------------------------------------------
 * System specials
 * ------------------------------------------------------------ */

OZ_BI_define(BIunify,2,0)
{
  oz_declareIN(0,a);
  oz_declareIN(1,b);

  return oz_unify(a,b);
} OZ_BI_end

OZ_BI_define(BIfail,0,0)
{
  return FAILED;
} OZ_BI_end

OZ_BI_define(BIskip,0,0) {
  return PROCEED;
} OZ_BI_end


// ------------------------------------------------------------------------
// --- Apply
// ------------------------------------------------------------------------

OZ_BI_define(BIapply,2,0)
{
  oz_declareNonvarIN(0,proc);
  oz_declareIN(1,args);

  return applyProc(proc, args);
} OZ_BI_end


OZ_BI_define(BItermToVS,3,1)
{
  oz_declareIN(0,t);
  oz_declareIntIN(1,depth);
  oz_declareIntIN(2,width);
  OZ_RETURN(OZ_string(OZ_toC(t,depth,width)));
} OZ_BI_end

OZ_BI_define(BIvalueNameVariable,2,0)
{
  oz_declareIN(0, var);
  oz_declareAtomIN(1, name);
  oz_varAddName(var, name);
  return PROCEED;
} OZ_BI_end

// ---------------------------------------------------------------------------
// ???
// ---------------------------------------------------------------------------

TaggedRef Abstraction::DBGgetGlobals() {
  int n = getPred()->getGSize();
  OZ_Term t = OZ_tuple(oz_atom("globals"),n);
  for (int i = 0; i < n; i++) {
    OZ_putArg(t,i,getG(i));
  }
  return t;
}


// ---------------------------------------------------------------------
// OO Stuff
// ---------------------------------------------------------------------

/*
 *	Construct a new SRecord to be a copy of old.
 *	This is the functionality of adjoin(old,newlabel).
 */
OZ_BI_define(BIcopyRecord,1,1)
{
  oz_declareNonvarIN(0,rec);
  
  if (oz_isSRecord(rec)) {
    SRecord *rec0 = tagged2SRecord(rec);
    SRecord *rec1 = SRecord::newSRecord(rec0);
    OZ_RETURN(makeTaggedSRecord(rec1));
  }
  
  if (oz_isLiteral(rec)) {
    OZ_RETURN(rec);
  }

  oz_typeError(0,"Determined Record");
} OZ_BI_end


// perdio
inline
SRecord *getRecordFromState(RecOrCell state)
{
  if (!stateIsCell(state))
    return getRecord(state);

  Tertiary *t=getCell(state);          // shortcut
  if(t->isLocal()) { // can happen if globalized object becomes localized again
    return tagged2SRecord(oz_deref(((CellLocal*)t)->getValue()));
  }

  if(!t->isProxy()) {
    CellSecEmul* sec;
    if(t->getTertType()==Te_Frame) {
      sec=((CellFrameEmul*)t)->getSec();
    } else {
      sec=((CellManagerEmul*)t)->getSec();
    }
    if(sec->getState()==Cell_Lock_Valid) {
      TaggedRef old = oz_deref(sec->getContents());
      Assert(!oz_isRef(old));
      if (!oz_isVarOrRef(old))
	return tagged2SRecord(old);
    }
  }
  return NULL;
}
  

// perdio
// returns SUSPEND (suspend on variable feature)
// returns BI_REPLACEBICALL (suspend on perdio)
// returns PROCEED

inline
OZ_Return stateAt(RecOrCell state, OZ_Term fea, OZ_Term &old){
  Assert(!oz_isVar(fea));
  SRecord *aux = getRecordFromState(state);
  if (aux){
    TaggedRef t;
    t=aux->getFeature(fea);
    if(t){
      old=t;
      return PROCEED;}
    oz_typeError(0,"(valid) Feature");
  }
  
  old=oz_newVariable();  
  Tertiary *t=getCell(state);          // shortcut
  if (oz_onToplevel()) {
    return (*cellAtExchange)(t,fea,old);}
  return (*cellAtAccess)(t,fea,old);
}

OZ_Return stateLevelError(TaggedRef fea,TaggedRef newVal){
  return oz_raise(E_ERROR,E_OBJECT,
		  "deep assignment attempted",3,
		  makeTaggedConst(am.getSelf()),fea,newVal);}

inline
OZ_Return stateAssign(RecOrCell state, OZ_Term fea, OZ_Term neW){
  Assert(!oz_isVar(fea));
  SRecord *aux = getRecordFromState(state);
  if (aux){
    TaggedRef t;
    t=aux->replaceFeature(fea,neW); 
    if(t){
      return PROCEED;}
    oz_typeError(0,"(valid) Feature");
  }

  if (!oz_onToplevel()) {return stateLevelError(fea,neW);}
  Tertiary *t=getCell(state);          // shortcut
  return (*cellAssignExchange)(t,fea,neW);
}

inline
OZ_Return stateExch(RecOrCell state, OZ_Term fea, OZ_Term &old, OZ_Term neW){
  Assert(!oz_isVar(fea));
  SRecord *aux = getRecordFromState(state);
  if (aux){
    TaggedRef t=aux->getFeature(fea); 
    if(t){
      old=t;
      t=aux->replaceFeature(fea,neW);
      Assert(t);
      return PROCEED;}
    oz_typeError(0,"(valid) Feature");}
  
  old=oz_newVariable();
  Tertiary *t=getCell(state);          // shortcut
  if (!oz_onToplevel()) {return stateLevelError(fea,neW);} 
  return (*objectExchange)(t,fea,old,neW);
}

//perdio interface to engine inline object functions
SRecord *getState(RecOrCell state, Bool isAssign, OZ_Term fea,OZ_Term &val)
{
  Assert(!oz_isVar(fea));  
  SRecord *aux=getRecordFromState(state);
  if(aux){
    return aux;}
  
  int EmCode;
  Tertiary *t=getCell(state);          // shortcut

  if (oz_onToplevel()) {
    if(isAssign) {
      EmCode = (*cellAssignExchange)(t,fea,val);
    } else {
      val= oz_newVariable();
      EmCode = (*cellAtExchange)(t,fea,val);
    }
  } else {
    if(!isAssign) val = oz_newVariable();
    EmCode = (*cellAtAccess)(t,fea,val);
  }

  Assert(EmCode==BI_REPLACEBICALL);
  return NULL;
}


//perdio
OZ_Return atInlineRedo(TaggedRef fea, TaggedRef out)
{
  DEREF(fea, feaPtr);
  if (!oz_isFeature(fea)) {
    Assert(!oz_isRef(fea));
    if (oz_isVarOrRef(fea)) {
      oz_suspendOnPtr(feaPtr);
    }
    oz_typeError(0,"Feature");
  }

  RecOrCell state = am.getSelf()->getState();
  return stateAt(state,fea,out);
}

OZ_DECLAREBI_USEINLINEREL2(BIatRedo,atInlineRedo)

OZ_BI_define(BIat,1,1)
{
  oz_declareIN(0,fea);

  DEREF(fea, feaPtr);
  if (!oz_isFeature(fea)) {
    Assert(!oz_isRef(fea));
    if (oz_isVarOrRef(fea)) {
      oz_suspendOnPtr(feaPtr);
    }
    oz_typeError(0,"Feature");
  }

  RecOrCell state = am.getSelf()->getState();
  OZ_Term old;
  OZ_Return ret=stateAt(state,fea,old);
  if(ret==PROCEED) {
    OZ_RETURN(old);}
  else{
    OZ_result(old);
    return ret;}
} OZ_BI_end

// ---------------------------------------------------------------------
// catXxxxxx routines work on mutable references 
// (cell/attribute/dict#key/array#index)
// ---------------------------------------------------------------------


OZ_BI_define(BIcatAccessOO,1,1)
{
  OZ_Return ret;
  oz_declareNonvarIN(0,cat);

  // cat can be either a cell, feature, D#I tuple, or an error
  if (oz_isCell(cat)) {
    // Cell
    OZ_Term out;
    ret = accessCell(cat,out);
    OZ_result(out);
  }
  else {
    if (oz_isPair2(cat)) {
      OZ_Term left = oz_left(cat);
      DEREF(left, leftptr);
      if (oz_isDictionary(left) || oz_isArray(left)) {
	OZ_Term out;
	ret = genericDot(left, oz_right(cat), out, TRUE);
	if (ret == SUSPEND) {
	  // Must explicitly suspend on key
	  oz_suspendOn(oz_right(cat));
	}         
	OZ_result(out);
      }
      else {
	// Type Error
	oz_typeError(0,"Dict#Key, Array#Index");
      }
    }
    else {
      // Check for feature
      Object *self = am.getSelf();
      if (self && oz_isFeature(cat)) {
	// Object attribute
	RecOrCell state = self->getState();
	OZ_Term old;
	ret = stateAt(state,cat,old);
	if(ret==PROCEED) {
	  OZ_RETURN(old);}
	else {
	  OZ_result(old);}
      }
      else {
        // Type Error
        oz_typeError(0,"Feature, Cell, Dict#Key, Array#Index");}
    }
  }
  return ret;
} OZ_BI_end

OZ_BI_define(BIcatAssignOO,2,0)
{
  oz_declareNonvarIN(0,cat);
  oz_declareIN(1,value);

  // cat can be either a cell, feature, D#I tuple, or an error
  if (oz_isCell(cat)) {
    // Cell
    OZ_Term oldIgnored;
    return exchangeCell(cat,value,oldIgnored);
  }
  if (oz_isPair2(cat)) {
    OZ_Term left = oz_left(cat);
    DEREF(left, leftptr);
    if (oz_isDictionary(left) || oz_isArray(left)) {
      // T#K pair
      OZ_Return ret = genericSet(left, oz_right(cat), value);
      if (ret == SUSPEND) {
        // Must explicitly suspend on components of arg 
        oz_suspendOn(oz_right(cat));
      }
      return ret;
    }
    else {
      // Type Error
      oz_typeError(0,"Dict#Key, Array#Index");
    }
  }         
  // Check for feature
  Object *self = am.getSelf();
  if (oz_isFeature(cat) && self) {
    // Object attribute
    CheckLocalBoard(self,"object");
  
    RecOrCell state = self->getState();
    return stateAssign(state,cat,value);
  }
  else {
    // Type Error
    oz_typeError(0,"Feature, Cell, Dict#Key, Array#Index");
  }
} OZ_BI_end

OZ_BI_define(BIcatExchangeOO,2,1)
{
  OZ_Term old;
  OZ_Return ret;

  oz_declareNonvarIN(0,cat);
  oz_declareIN(1,newVal);

  // ca can be either a cell or feature or an error
  if (oz_isCell(cat)) {
    // Cell
    ret = exchangeCell(cat,newVal,old);
    OZ_result(old);
  }
  else {
    if (oz_isPair2(cat)) {
      OZ_Term left = oz_left(cat);
      DEREF(left, leftptr);
      // T#K pair
      if (oz_isDictionary(left) || oz_isArray(left)) {
	ret = genericExchange(left, oz_right(cat), newVal, old);
	if (ret == SUSPEND) {
	  // Must explicitly suspend on key
	  oz_suspendOn(oz_right(cat));
	}         
	OZ_result(old);
      }
      else {
	// Type Error
	oz_typeError(0,"Dict#Key, Array#Index");
      }
    }
    else {
      // Check for feature
      Object *self = am.getSelf();
      if (oz_isFeature(cat) && self) {
	// Object attribute
	CheckLocalBoard(self,"object");
	RecOrCell state = self->getState();

	OZ_Return ret=stateExch(state,cat,old,newVal);
	if(ret==PROCEED) {
	  OZ_RETURN(old);}
	else{
	  OZ_result(old);
	  return ret;}
      } 
      else {
        // Type Error
        oz_typeError(0,"Feature, Cell, Dict#Key, Array#Index");
      }
    }
  }
  return ret;

} OZ_BI_end

OZ_BI_define(BIcatAccess,1,1)
{
  OZ_Return ret;
  oz_declareNonvarIN(0,cat);

  // cat can be either a cell, D#I tuple, or an error
  if (oz_isCell(cat)) {
    // Cell
    OZ_Term out;
    ret = accessCell(cat,out);
    OZ_result(out);
  }
  else {
    if (oz_isPair2(cat)) {
      OZ_Term left = oz_left(cat);
      DEREF(left, leftptr);
      if (oz_isDictionary(left) || oz_isArray(left)) {
	OZ_Term out;
	ret = genericDot(left, oz_right(cat), out, TRUE);
	if (ret == SUSPEND) {
	  // Must explicitly suspend on key
	  oz_suspendOn(oz_right(cat));
	}         
	OZ_result(out);
      }
      else {
	// Type Error
	oz_typeError(0,"Dict#Key, Array#Index");
      }
    }
    else {
      // Type Error
      oz_typeError(0,"Cell, Dict#Key, Array#Index");
    }
  }
  return ret;
} OZ_BI_end

OZ_BI_define(BIcatAssign,2,0)
{
  oz_declareNonvarIN(0,cat);
  oz_declareIN(1,value);

  // cat can be either a cell, feature, D#I tuple, or an error
  if (oz_isCell(cat)) {
    // Cell
    OZ_Term oldIgnored;
    return exchangeCell(cat,value,oldIgnored);
  }
  if (oz_isPair2(cat)) {
    OZ_Term left = oz_left(cat);
    DEREF(left, leftptr);
    if (oz_isDictionary(left) || oz_isArray(left)) {
      // T#K pair
      OZ_Return ret = genericSet(left, oz_right(cat), value);
      if (ret == SUSPEND) {
        // Must explicitly suspend on components of arg 
        oz_suspendOn(oz_right(cat));
      }
      return ret;
    }
    else {
      // Type Error
      oz_typeError(0,"Dict#Key, Array#Index");
    }
  }         
  // Type Error
  oz_typeError(0,"Cell, Dict#Key, Array#Index");
} OZ_BI_end

OZ_BI_define(BIcatExchange,2,1)
{
  OZ_Term old;
  OZ_Return ret;

  oz_declareNonvarIN(0,cat);
  oz_declareIN(1,newVal);

  // ca can be either a cell or feature or an error
  if (oz_isCell(cat)) {
    // Cell
    ret = exchangeCell(cat,newVal,old);
    OZ_result(old);
  }
  else {
    if (oz_isPair2(cat)) {
      OZ_Term left = oz_left(cat);
      DEREF(left, leftptr);
      // T#K pair
      if (oz_isDictionary(left) || oz_isArray(left)) {
	ret = genericExchange(left, oz_right(cat), newVal, old);
	if (ret == SUSPEND) {
	  // Must explicitly suspend on key
	  oz_suspendOn(oz_right(cat));
	}         
	OZ_result(old);
      }
      else {
	// Type Error
	oz_typeError(0,"Dict#Key, Array#Index");
      }
    }
    else {
      // Type Error
      oz_typeError(0,"Cell, Dict#Key, Array#Index");
    }
  }
  return ret;

} OZ_BI_end

OZ_BI_define(BIassign,2,0)
{
  oz_declareIN(0,fea);
  oz_declareIN(1,value);

  DEREF(fea, feaPtr);
  if (!oz_isFeature(fea)) {
    Assert(!oz_isRef(fea));
    if (oz_isVarOrRef(fea)) {
      oz_suspendOnPtr(feaPtr);
    }
    oz_typeError(0,"Feature");
  }

  Object *self = am.getSelf();
  CheckLocalBoard(self,"object");
  
  RecOrCell state = self->getState();
  return stateAssign(state,fea,value);
} OZ_BI_end


OZ_BI_define(BIexchange,2,1)
{
  oz_declareIN(0,fea);
  oz_declareIN(1,newVal);

  DEREF(fea, feaPtr);

  if (!oz_isFeature(fea)) {
    Assert(!oz_isRef(fea));
    if (oz_isVarOrRef(fea)) {
      oz_suspendOnPtr(feaPtr);
      return SUSPEND;
    }
    oz_typeError(1,"Feature");
  }

  Object *self=am.getSelf();
  CheckLocalBoard(self,"object");
  RecOrCell state = self->getState();
  TaggedRef old;

  OZ_Return ret=stateExch(state,fea,old,newVal);
  if(ret==PROCEED) {
    OZ_RETURN(old);}
  else{
    OZ_result(old);
    return ret;}

} OZ_BI_end




inline int sizeOf(SRecord *sr)
{
  return sr ? sr->sizeOf() : 0;
}

inline
Object *newObject(SRecord *feat, SRecord *st, ObjectClass *cla, Board *b)
{
  OzLock *lck=NULL;
  if (cla->supportsLocking()) {
    lck = new LockLocal(oz_currentBoard());
  }
  return new Object(b,st,cla,feat,lck);
}


OZ_BI_define(BInewClass,3,1) {
  OZ_Term features   = OZ_in(0); { DEREF(features,_1); }
  OZ_Term locking    = OZ_in(1); { DEREF(locking,_1); }
  OZ_Term sited      = OZ_in(2); { DEREF(sited,_1); }

  SRecord * fr = tagged2SRecord(features);

  OZ_Term fastmeth   = fr->getFeature(NameOoFastMeth); 
  { DEREF(fastmeth,_1); }
  OZ_Term ufeatures  = fr->getFeature(NameOoFeat); 
  { DEREF(ufeatures,_1); }
  OZ_Term defmethods = fr->getFeature(NameOoDefaults); 
  { DEREF(defmethods,_1); }

  TaggedRef uf = oz_isSRecord(ufeatures) ? ufeatures : makeTaggedNULL();

  ObjectClass *cl = new ObjectClass(features,
				    fastmeth,
				    uf,
				    defmethods,
				    oz_isTrue(locking),
				    oz_isTrue(sited),
				    oz_currentBoard());

  OZ_RETURN(makeTaggedConst(cl));
} OZ_BI_end


OZ_BI_define(BIcomma,2,0) 
{
  oz_declareNonvarIN(0,cl);
  cl = oz_deref(cl);

  if (!oz_isClass(cl)) {
    oz_typeError(0,"Class");
  }

  TaggedRef fb = tagged2ObjectClass(cl)->getFallbackApply();
  Assert(fb);

  am.prepareCall(fb,RefsArray::make(OZ_in(0),OZ_in(1)));
  am.emptySuspendVarList();  
  return BI_REPLACEBICALL;
} OZ_BI_end

OZ_BI_define(BIsend,3,0) 
{
  oz_declareNonvarIN(1,cl);
  oz_declareNonvarIN(2,obj);

  cl = oz_deref(cl);
  if (!oz_isClass(cl)) {
    oz_typeError(1,"Class");
  }

  obj = oz_deref(obj);
  if (!oz_isObject(obj)) {
    oz_typeError(2,"Object");
  }

  TaggedRef fb = tagged2ObjectClass(cl)->getFallbackApply();
  Assert(fb);

  am.changeSelf(tagged2Object(obj));

  am.prepareCall(fb,RefsArray::make(OZ_in(1),OZ_in(0)));
  am.emptySuspendVarList();  
  return BI_REPLACEBICALL;
} OZ_BI_end


inline
OZ_Return getClassInline(TaggedRef t, TaggedRef &out)
{ 
  DEREF(t,_);
  Assert(!oz_isRef(t));
  if (oz_isVarOrRef(t)) return SUSPEND;
  if (!oz_isObject(t)) {
    oz_typeError(0,"Object");
  }
  out = makeTaggedConst(tagged2Object(t)->getClass());
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN1(BIgetClass,getClassInline)




inline
TaggedRef cloneObjectRecord(TaggedRef record, Bool cloneAll)
{
  if (oz_isLiteral(record))
    return record;

  Assert(oz_isSRecord(record));

  SRecord *in  = tagged2SRecord(record);
  SRecord *rec = SRecord::newSRecord(in);

  for(int i=0; i < in->getWidth(); i++) {
    OZ_Term arg = in->getArg(i);
    if (cloneAll || oz_eq(NameOoFreeFlag,oz_deref(arg))) {
      arg = oz_newVariable();
    }
    rec->setArg(i,arg);
  }

  return makeTaggedSRecord(rec);
}

static TaggedRef dummyRecord = 0;

inline
OZ_Term makeObject(OZ_Term initState, OZ_Term ffeatures, ObjectClass *clas)
{
  Assert(oz_isRecord(initState) && oz_isRecord(ffeatures));

  /* state is _allways_ a record, this makes life somewhat easier */
  if (!oz_isSRecord(initState)) {
    if (dummyRecord==0) {
      dummyRecord = OZ_recordInitC("noattributes",
				   oz_list(OZ_pair2(OZ_newName(),taggedVoidValue),0));
      OZ_protect(&dummyRecord);
    }
    initState = dummyRecord;
  }

  Object *out = 
    newObject(oz_isSRecord(ffeatures) ? tagged2SRecord(ffeatures) : (SRecord*) NULL,
	      tagged2SRecord(initState),
	      clas,
	      oz_currentBoard());
  
  return makeTaggedConst(out);
}


inline
OZ_Return newObjectInline(TaggedRef cla, TaggedRef &out)
{ 
  { DEREF(cla,_1); }
  Assert(!oz_isRef(cla));
  if (oz_isVarOrRef(cla)) return SUSPEND;
  if (!oz_isClass(cla)) {
    oz_typeError(0,"Class");
  }

  ObjectClass *realclass = tagged2ObjectClass(cla);
  TaggedRef attr = realclass->classGetFeature(NameOoAttr);
  { DEREF(attr,_1); }
  Assert(!oz_isRef(attr));
  if (oz_isVarOrRef(attr)) return SUSPEND;
  
  TaggedRef attrclone = cloneObjectRecord(attr,NO);

  TaggedRef freefeat = realclass->classGetFeature(NameOoFreeFeat);
  { DEREF(freefeat,_1); }
  Assert(!oz_isVar(freefeat));
  TaggedRef freefeatclone = cloneObjectRecord(freefeat,OK);

  out = makeObject(attrclone, freefeatclone, realclass);

  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN1(BInewObject,newObjectInline)


OZ_BI_define(BINew,3,0)
{
  oz_declareNonvarIN(0,cl);
  cl = oz_deref(cl);

  if (!oz_isClass(cl)) {
    oz_typeError(0,"Class");
  }

  ObjectClass * oc = tagged2ObjectClass(cl);

  TaggedRef fb = oc->getFallbackNew();

  Assert(fb);

  am.prepareCall(fb,RefsArray::make(OZ_in(0),OZ_in(1),OZ_in(2)));
  am.emptySuspendVarList();  
  return BI_REPLACEBICALL;
} OZ_BI_end


inline
OZ_Return ooGetLockInline(TaggedRef val)
{ 
  OzLock *lock = am.getSelf()->getLock();
  if (lock==NULL)
    return oz_raise(E_ERROR,E_OBJECT,"locking",1,
		    makeTaggedConst(am.getSelf()));

  return oz_unify(val,makeTaggedConst(lock));
}
OZ_DECLAREBI_USEINLINEREL1(BIooGetLock,ooGetLockInline)


OZ_BI_define(BIclassIs,1,1)  {
  oz_declareNonvarIN(0,cl);
  cl = oz_deref(cl);

  OZ_RETURN(oz_isClass(cl) ? oz_true() : oz_false());
} OZ_BI_end

OZ_BI_define(BIclassIsSited,1,1)  {
  oz_declareNonvarIN(0,cl);
  cl = oz_deref(cl);

  if (!oz_isClass(cl)) {
    oz_typeError(0,"Class");
  }

  OZ_RETURN(tagged2ObjectClass(cl)->isSited() ? oz_true() : oz_false());
} OZ_BI_end

OZ_BI_define(BIclassIsLocking,1,1)  {
  oz_declareNonvarIN(0,cl);
  cl = oz_deref(cl);

  if (!oz_isClass(cl)) {
    oz_typeError(0,"Class");
  }

  OZ_RETURN(tagged2ObjectClass(cl)->supportsLocking() ? oz_true():oz_false());
} OZ_BI_end

#ifdef MISC_BUILTINS

/********************************************************************
 * Functions
 ******************************************************************** */

OZ_BI_define(BIfunReturn,1,0) 
{
  OZ_warning("funReturn should never be called");
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIgetReturn,0,1) 
{
  OZ_warning("getReturn should never be called");
  return PROCEED;
} OZ_BI_end

#endif


/********************************************************************
 * Exceptions
 ******************************************************************** */

OZ_BI_define(BIraise,1,0)
{
  oz_declareIN(0,exc);
  //  message("Raise: %s\n",toC(exc));
  return OZ_raiseDebug(exc);
} OZ_BI_end

OZ_BI_define(BIraiseError,1,0)
{
  oz_declareIN(0,exc);

  return OZ_raiseError(exc);
} OZ_BI_end

int oz_raise(OZ_Term cat, OZ_Term key, const char *label, int arity, ...)
{
  Assert(!oz_isRef(cat));
  OZ_Term exc=OZ_tuple(key,arity+1);
  OZ_putArg(exc,0,OZ_atom(label));

  va_list ap;
  va_start(ap,arity);

  for (int i = 0; i < arity; i++) {
    OZ_putArg(exc,i+1,va_arg(ap,OZ_Term));
  }

  va_end(ap);


  OZ_Term ret = OZ_record(cat,
			  oz_cons(makeTaggedSmallInt(1),
				  oz_cons(AtomDebug,oz_nil())));
  OZ_putSubtree(ret,makeTaggedSmallInt(1),exc);
  OZ_putSubtree(ret,AtomDebug,NameUnit);

  am.setException(ret, oz_eq(cat,E_ERROR) ? TRUE : ozconf.errorDebug);
  return RAISE;
}


/*===================================================================
 * type errors
 *=================================================================== */

static
char *getTypeOfPos(char * t, int p)
{
  static char buffer[100];
  int i, bi, comma;

  for (i = 0, comma = 0; t[i] != '\0' && comma < p; i += 1) {
    if (t[i] == ',') comma += 1;
    if (t[i] == '\\' && t[i+1] == ',') i += 1;
  } 

  for (bi = 0; t[i] != '\0' && t[i] != ','; i += 1, bi += 1) {
    if (t[i] == '\\' && t[i+1] == ',') i += 1;
    buffer[bi] = t[i];
  }

  buffer[bi] = '\0';
  
  return buffer;
}

OZ_Return typeError(int Pos, char *Comment, char *TypeString)
{
  (void) oz_raise(E_ERROR,E_KERNEL,
		  "type",5,NameUnit,NameUnit,
		  OZ_atom(getTypeOfPos(TypeString, Pos)),
		  makeTaggedSmallInt(Pos+1),
		  OZ_string(Comment));
  return BI_TYPE_ERROR;
}

OZ_Return oz_typeErrorInternal(const int pos, const char * type) {
  (void) oz_raise(E_ERROR,E_KERNEL,
		  "type",5,NameUnit,NameUnit,
		  OZ_atom(type),
		  makeTaggedSmallInt(pos+1),
		  OZ_string(""));
  return BI_TYPE_ERROR;
}

