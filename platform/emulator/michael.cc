/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Copyright:
 *    Michael Mehl, 1998
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

/*
 * This file helps me debugging the system and provides my infrastructure
 * for experiments.  It's check in to the mozart system to make it simpler
 * for me to use it from multiple sites.
 *
 * Compile it with
 *
 *   make michael.o
 *   ozdynld -o libMichael.so michael.o
 *
 */

#include "builtins.hh"
#include "value.hh"
#include "genvar.hh"
#include "os.hh"
#include "codearea.hh"
#include "trace.hh"

#include <stdio.h>

/*===================================================================
 * Value.type/status
 *=================================================================== */

char *oz_valueType(OZ_Term term)
{
  Assert(!oz_isRef(term));

  switch (tagTypeOf(term)) {
  case UVAR:
  case SVAR:
  case CVAR:
    return "variable";
  case SMALLINT:
    return "int";
  case OZFLOAT:    
    return "float";
  case LITERAL:
    return tagged2Literal(term)->isAtom() ? "atom" : "name";
  case LTUPLE:
    return "cons";
  case SRECORD:
    if (tagged2SRecord(term)->isTuple()) {
      SRecord *sr=tagged2SRecord(term);
      if (oz_eq(sr->getLabel(),AtomPair) && sr->getWidth()>1) {
	return "pair";
      } else {
	return "tuple";
      }
    } else {
      return "record";
    }
  case FSETVALUE:
    return "fset";
  case OZCONST:
    switch (tagged2Const(term)->getType()) {
    case Co_BigInt:
      return "int";
    case Co_Foreign_Pointer:
      return "foreignPointer";
    case Co_Thread:
      return "thread";
    case Co_Abstraction:
    case Co_Builtin:
      return "procedure";
    case Co_Cell:
      return "cell";
    case Co_Space:
      return "space";
    case Co_Object:
      return "object";
    case Co_Port:
      return "port";
    case Co_Chunk:
      return "chunk";
    case Co_HeapChunk:
      return "heapChunk";
    case Co_BitArray:
      return "bitArray";
    case Co_Array:
      return "array";
    case Co_Dictionary:
      return "dictionary";
    case Co_Lock:
      return "lock";
    case Co_Class:
      return "class";
    default:
      break;
    }
  default:
    break;
  }
  Assert(0);
  return "other";
}


/*
 * declare M=
 * local
 *    P={Property.get 'platform'}
 * in
 *    {Foreign.load 'http://www.ps.uni-sb.de/~mehl/mozart/addon/platform/'
 *                  #P.1#'-'#P.2#'/libMichael.so'}
 * end
 *
 * {M.status X} returns
 *   var_<vt>, if X is a variable, where <vt> is one of
 *               future, free,
 *               int,fset,record,other
 *   det_<dt>, if X is determined, where <dt> is one of
 *               int, float, 
 *               atom, name, cons, pair, tuple, record,
 *               procedure, cell, object, port, heapChunk, bitArray, array,
 *                dictionary, lock, class, space, thread, fset,
 *                foreignPointer, (...maybe more built-in chunks ...),
 *                chunk
 *
 *    NOTE: pair <=> {IsTuple X} and {Label X}='#' and {Width X}>1
 *          cons <=> {IsTuple X} and {Label X}='|' and {Width X}=2
 */

OZ_BI_define(BIstatusNew,1,1)
{
  oz_declareIN(0,term);

  DEREF(term, _1, tag);

  switch (tag) {
  case UVAR: 
  case SVAR: 
    OZ_RETURN_ATOM("var_free");
  case CVAR:
    switch (tagged2CVar(term)->getType()) {
    case FDVariable:
    case BoolVariable:
      OZ_RETURN_ATOM("var_int");
    case FSetVariable:
      OZ_RETURN_ATOM("var_fset");
    case OFSVariable:
      OZ_RETURN_ATOM("var_record");
    case OZ_VAR_FUTURE:
      OZ_RETURN_ATOM("var_future");
    default:
      if (oz_isFree(term)) {
	OZ_RETURN_ATOM("var_free");
      } else {
	OZ_RETURN_ATOM("var_other");
      }
    }
  default:
    char *t=oz_valueType(term);
    static char buf[1000];
    strcpy(buf,"det_");
    strcpy(buf+4,t);
    OZ_RETURN(oz_atom(buf));
  }
  return PROCEED;
} OZ_BI_end

/*===================================================================
 * Debugging aids
 *=================================================================== */

/*
 * 'stop':
 *   stop emulator until the RETURN key is pressed
 *   sometimes this is useful for stopping a program to inspect something
 */
const int MaxLine=1000;
OZ_BI_define(BIstop, 0,0)
{
  static char command[MaxLine];
 
  printf("press the RETURN key to continue! ");
  fflush(stdout);
  if (osfgets(command,MaxLine,stdin) == (char *) NULL) {
    printf("Read no input.\n");
  } else if (feof(stdin)) {
    clearerr(stdin);
    printf("EOF.\n");
  } else {
    printf("ok.\n");
  }
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIhalt, 0,0)
{
#ifdef DEBUG_TRACE
  ozd_tracerOn();
#endif
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIlivenessX, 1,1)
{
  OZ_declareIntIN(0,pc);

  OZ_RETURN_INT(CodeArea::livenessX((ProgramCounter)ToPointer(pc),0,0));
} OZ_BI_end

OZ_BI_define(BIdisplayDef, 2,0)
{
  OZ_declareIntIN(0,pc);
  OZ_declareIntIN(1,size);
  displayDef((ProgramCounter)ToPointer(pc),size);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIdisplayCode, 2,0)
{
  OZ_declareIntIN(0,pc);
  OZ_declareIntIN(1,size);
  displayCode((ProgramCounter)ToPointer(pc),size);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIprocedureCode, 1,1)
{
  oz_declareNonvarIN(0,proc);
  if (!oz_isProcedure(proc)) {
    oz_typeError(0,"Procedure");
  }
  if (oz_isBuiltin(proc)) {
    oz_typeError(0,"Procedure (no builtin)");
  }

  Abstraction *a=tagged2Abstraction(proc);
  OZ_RETURN_INT(ToInt32(a->getPred()->getPC()));
} OZ_BI_end

OZ_BI_define(BIprocedureEnvironment,1,1)
{
  oz_declareNonvarIN(0,p);

  if (!oz_isProcedure(p)) {
    oz_typeError(0,"Procedure");
  }

  OZ_Term t;

  if (oz_isBuiltin(p)) {
    t = OZ_atom("environment");
  } else {
    Abstraction *a=tagged2Abstraction(p);

    int len=a->getPred()->getGSize();
    if (len>0) {
      t = OZ_tuple(OZ_atom("environment"),len);
      for (int i=0; i<len; i++) OZ_putArg(t,i,a->getG(i));
    } else {
      t = OZ_atom("environment");
    }
  }
  OZ_RETURN(t);
} OZ_BI_end

/********************************************************************
 * Inspecting values (EXPERIMENTAL by mm)
 ******************************************************************** */

/*
 * This function should subsume various other hacks,
 * e.g. BIprintname, BIstatus, BItermtype, ...
 * Inspect the toplevel of an Oz value
 *
 * returns:
 *  RET     = variable(printName:PN suspensions:SL kind:KIND)
 *          | ref(N RET)             N = length of reference chain
 *          | det
 *  PN      = print name
 *  SL      = length of susp list
 *  KIND    = uvar
 *          | svar
 *          | future(SUBKIND)
 *          | generic
 *  SUBKIND = byNeed(requested)
 *          | byNeed(FUN)
 *          | simple
 *  FUN     = an null-ary function
 */

OZ_Term oz_inspect(OZ_Term t)
{
  int refCount=0;
  if (oz_isRef(t)) {
    OZ_Term *tptr;
  loop:
    tptr = tagged2Ref(t);
    t = *tptr;
    if (oz_isRef(t)) {
      refCount++;
      goto loop;
    }
    if (oz_isVariable(t)) {
      OZ_Term kind;
      int sl = 0;
      if (isCVar(t)) {
	kind = tagged2CVar(t)->inspectV();
	sl = tagged2CVar(t)->getSuspListLengthV();
      } else if (isSVar(t)) {
	kind = OZ_atom("svar");
	sl = tagged2SVar(t)->getSuspList()->length();
      } else {
	Assert(isUVar(t));
	kind = OZ_atom("uvar");
      }
      const char *pn=VariableNamer::getName(makeTaggedRef(tptr));
      OZ_Term pl = oz_list(OZ_pairAA("printName",pn),
			   OZ_pairAI("suspensions",sl),
			   OZ_pairA("kind",kind),
			   0);
      OZ_Term ret = OZ_recordInitC("variable",pl);
      if (refCount!=0) {
	ret=OZ_mkTupleC("ref",2,OZ_int(refCount),ret);
      }
      return ret;
    }
  }
  OZ_Term ret=oz_atom("det");
  if (refCount) {
    ret = OZ_mkTupleC("ref",2,OZ_int(refCount),ret);
  }
  return ret;
}

OZ_BI_define(BIinspect, 1, 1)
{
  OZ_Term t = OZ_in(0);
  OZ_RETURN(oz_inspect(t));
} OZ_BI_end


OZ_C_proc_interface oz_interface[] = {
  {"stop",0,0,BIstop},
  {"status",1,1,BIstatusNew},
  {"displayDef",2,0,BIdisplayDef},
  {"displayCode",2,0,BIdisplayCode},
  {"procedureCode",1,1,BIprocedureCode},
  //  {"print",2,0,BIdebugPrint},
  //  {"printLong",2,0,BIdebugPrintLong},
  {"inspect",1,1,BIinspect},
  {"livenessX",1,1,BIlivenessX},
  {"procedureEnvironment",1,1,BIprocedureEnvironment},
  {0,0,0,0}
};
