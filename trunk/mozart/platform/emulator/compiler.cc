/*
 *  Authors:
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *
 *  Copyright:
 *    Ralf Scheidhauer, 1997
 *    Leif Kornstaedt, 1997-1998
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation of Oz 3:
 *    http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *    http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */


#include "indexing.hh"
#include "builtins.hh"
#include "codearea.hh"
#include "var_base.hh"
#include "boot-manager.hh"

static const char * comp_instr[] = {
  "skip", NULL,
  "definition", NULL,
  "definitionCopy", NULL,
  "endDefinition", NULL,
  "move", "moveXX",
  "moveMove", "moveMoveXYXY",
  "createVariable", "createVariableX",
  "createVariableMove", "createVariableMoveX",
  "unify", "unifyXX",
  "putRecord", "putRecordX",
  "putList", "putListX",
  "putConstant", "putConstantX",
  "setVariable", "setVariableX",
  "setValue", "setValueX",
  "setConstant", NULL,
  "setProcedureRef", NULL,
  "setVoid", NULL,
  "getRecord", "getRecordX",
  "getList", "getListX",
  "getListValVar", "getListValVarX",
  "unifyVariable", "unifyVariableX",
  "unifyValue", "unifyValueX",
  "unifyValVar", "unifyValVarXX",
  "unifyNumber", NULL,
  "unifyLiteral", NULL,
  "unifyVoid", NULL,
  "getLiteral", "getLiteralX",
  "getNumber", "getNumberX",
  "allocateL", NULL,
  "allocateL1", NULL,
  "allocateL2", NULL,
  "allocateL3", NULL,
  "allocateL4", NULL,
  "allocateL5", NULL,
  "allocateL6", NULL,
  "allocateL7", NULL,
  "allocateL8", NULL,
  "allocateL9", NULL,
  "allocateL10", NULL,
  "deAllocateL", NULL,
  "deAllocateL1", NULL,
  "deAllocateL2", NULL,
  "deAllocateL3", NULL,
  "deAllocateL4", NULL,
  "deAllocateL5", NULL,
  "deAllocateL6", NULL,
  "deAllocateL7", NULL,
  "deAllocateL8", NULL,
  "deAllocateL9", NULL,
  "deAllocateL10", NULL,
  "callMethod", NULL,
  "callGlobal", NULL,
  "call", "callX",
  "tailCall", "tailCallX",
  "callConstant", NULL,
  "callProcedureRef", NULL,
  "sendMsg", "sendMsgX",
  "tailSendMsg", "tailSendMsgX",
  "getSelf", NULL,
  "setSelf", "setSelfG",
  "lockThread", NULL,
  "inlineAt", NULL,
  "inlineAssign", NULL,
  "branch", NULL,
  "exHandler", NULL,
  "popEx", NULL,
  "return", NULL,
  "getReturn", "getReturnX",
  "funReturn", "funReturnX",
  "testLiteral", "testLiteralX",
  "testNumber", "testNumberX",
  "testRecord", "testRecordX",
  "testList", "testListX",
  "testBool", "testBoolX",
  "match", "matchX",
  "getVariable", "getVariableX",
  "getVarVar", "getVarVarXX",
  "getVoid", NULL,
  "debugEntry", NULL,
  "debugExit", NULL,
  "globalVarname", NULL,
  "localVarname", NULL,
  "clear", "clearY",
  "profileProc", NULL,
  "callBI", NULL,
  "inlinePlus1", NULL,
  "inlineMinus1", NULL,
  "inlinePlus", NULL,
  "inlineMinus", NULL,
  "inlineDot", NULL,
  "testBI", NULL,
  "testLT", NULL,
  "testLE", NULL,
  NULL, NULL,
};

#define CI_SKIP    0
#define CI_DEFINITION    (CI_SKIP+1)
#define CI_DEFINITIONCOPY    (CI_DEFINITION+1)
#define CI_ENDDEFINITION    (CI_DEFINITIONCOPY+1)
#define CI_MOVE    (CI_ENDDEFINITION+1)
#define CI_MOVEMOVE    (CI_MOVE+1)
#define CI_CREATEVARIABLE    (CI_MOVEMOVE+1)
#define CI_CREATEVARIABLEMOVE    (CI_CREATEVARIABLE+1)
#define CI_UNIFY    (CI_CREATEVARIABLEMOVE+1)
#define CI_PUTRECORD    (CI_UNIFY+1)
#define CI_PUTLIST    (CI_PUTRECORD+1)
#define CI_PUTCONSTANT    (CI_PUTLIST+1)
#define CI_SETVARIABLE    (CI_PUTCONSTANT+1)
#define CI_SETVALUE    (CI_SETVARIABLE+1)
#define CI_SETCONSTANT    (CI_SETVALUE+1)
#define CI_SETPROCEDUREREF    (CI_SETCONSTANT+1)
#define CI_SETVOID    (CI_SETPROCEDUREREF+1)
#define CI_GETRECORD    (CI_SETVOID+1)
#define CI_GETLIST    (CI_GETRECORD+1)
#define CI_GETLISTVALVAR    (CI_GETLIST+1)
#define CI_UNIFYVARIABLE    (CI_GETLISTVALVAR+1)
#define CI_UNIFYVALUE    (CI_UNIFYVARIABLE+1)
#define CI_UNIFYVALVAR    (CI_UNIFYVALUE+1)
#define CI_UNIFYNUMBER    (CI_UNIFYVALVAR+1)
#define CI_UNIFYLITERAL    (CI_UNIFYNUMBER+1)
#define CI_UNIFYVOID    (CI_UNIFYLITERAL+1)
#define CI_GETLITERAL    (CI_UNIFYVOID+1)
#define CI_GETNUMBER    (CI_GETLITERAL+1)
#define CI_ALLOCATEL    (CI_GETNUMBER+1)
#define CI_ALLOCATEL1    (CI_ALLOCATEL+1)
#define CI_ALLOCATEL2    (CI_ALLOCATEL1+1)
#define CI_ALLOCATEL3    (CI_ALLOCATEL2+1)
#define CI_ALLOCATEL4    (CI_ALLOCATEL3+1)
#define CI_ALLOCATEL5    (CI_ALLOCATEL4+1)
#define CI_ALLOCATEL6    (CI_ALLOCATEL5+1)
#define CI_ALLOCATEL7    (CI_ALLOCATEL6+1)
#define CI_ALLOCATEL8    (CI_ALLOCATEL7+1)
#define CI_ALLOCATEL9    (CI_ALLOCATEL8+1)
#define CI_ALLOCATEL10    (CI_ALLOCATEL9+1)
#define CI_DEALLOCATEL    (CI_ALLOCATEL10+1)
#define CI_DEALLOCATEL1    (CI_DEALLOCATEL+1)
#define CI_DEALLOCATEL2    (CI_DEALLOCATEL1+1)
#define CI_DEALLOCATEL3    (CI_DEALLOCATEL2+1)
#define CI_DEALLOCATEL4    (CI_DEALLOCATEL3+1)
#define CI_DEALLOCATEL5    (CI_DEALLOCATEL4+1)
#define CI_DEALLOCATEL6    (CI_DEALLOCATEL5+1)
#define CI_DEALLOCATEL7    (CI_DEALLOCATEL6+1)
#define CI_DEALLOCATEL8    (CI_DEALLOCATEL7+1)
#define CI_DEALLOCATEL9    (CI_DEALLOCATEL8+1)
#define CI_DEALLOCATEL10    (CI_DEALLOCATEL9+1)
#define CI_CALLMETHOD    (CI_DEALLOCATEL10+1)
#define CI_CALLGLOBAL    (CI_CALLMETHOD+1)
#define CI_CALL    (CI_CALLGLOBAL+1)
#define CI_TAILCALL    (CI_CALL+1)
#define CI_CALLCONSTANT    (CI_TAILCALL+1)
#define CI_CALLPROCEDUREREF    (CI_CALLCONSTANT+1)
#define CI_SENDMSG    (CI_CALLPROCEDUREREF+1)
#define CI_TAILSENDMSG    (CI_SENDMSG+1)
#define CI_GETSELF    (CI_TAILSENDMSG+1)
#define CI_SETSELF    (CI_GETSELF+1)
#define CI_LOCKTHREAD    (CI_SETSELF+1)
#define CI_INLINEAT    (CI_LOCKTHREAD+1)
#define CI_INLINEASSIGN    (CI_INLINEAT+1)
#define CI_BRANCH    (CI_INLINEASSIGN+1)
#define CI_EXHANDLER    (CI_BRANCH+1)
#define CI_POPEX    (CI_EXHANDLER+1)
#define CI_RETURN    (CI_POPEX+1)
#define CI_GETRETURN    (CI_RETURN+1)
#define CI_FUNRETURN    (CI_GETRETURN+1)
#define CI_TESTLITERAL    (CI_FUNRETURN+1)
#define CI_TESTNUMBER    (CI_TESTLITERAL+1)
#define CI_TESTRECORD    (CI_TESTNUMBER+1)
#define CI_TESTLIST    (CI_TESTRECORD+1)
#define CI_TESTBOOL    (CI_TESTLIST+1)
#define CI_MATCH    (CI_TESTBOOL+1)
#define CI_GETVARIABLE    (CI_MATCH+1)
#define CI_GETVARVAR    (CI_GETVARIABLE+1)
#define CI_GETVOID    (CI_GETVARVAR+1)
#define CI_DEBUGENTRY    (CI_GETVOID+1)
#define CI_DEBUGEXIT    (CI_DEBUGENTRY+1)
#define CI_GLOBALVARNAME    (CI_DEBUGEXIT+1)
#define CI_LOCALVARNAME    (CI_GLOBALVARNAME+1)
#define CI_CLEAR    (CI_LOCALVARNAME+1)
#define CI_PROFILEPROC    (CI_CLEAR+1)
#define CI_CALLBI    (CI_PROFILEPROC+1)
#define CI_INLINEPLUS1    (CI_CALLBI+1)
#define CI_INLINEMINUS1    (CI_INLINEPLUS1+1)
#define CI_INLINEPLUS    (CI_INLINEMINUS1+1)
#define CI_INLINEMINUS    (CI_INLINEPLUS+1)
#define CI_INLINEDOT    (CI_INLINEMINUS+1)
#define CI_TESTBI    (CI_INLINEDOT+1)
#define CI_TESTLT    (CI_TESTBI+1)
#define CI_TESTLE    (CI_TESTLT+1)

static
SRecordArity getArity(TaggedRef arity)
{
  if (oz_isSmallInt(arity)) {
    return mkTupleWidth(tagged2SmallInt(arity));
  } else {
    Assert(oz_isSmallInt(oz_checkList(arity)));
    TaggedRef sortedarity = arity;
    if (!isSorted(arity)) {
      int len;
      TaggedRef aux = duplist(arity,len);
      sortedarity = sortlist(aux,len);
    }
    Arity *ari = aritytable.find(sortedarity);
    return (ari->isTuple())? mkTupleWidth(ari->getWidth()): mkRecordArity(ari);
  }
}


#define OZ_declareRecordArityIN(num,name)		\
  SRecordArity name;					\
  {							\
    oz_declareNonvarIN(num,__aux);			\
    name = getArity(__aux);				\
    if (name == (SRecordArity) -1) {			\
      oz_typeError(num,"RecordArity");			\
    }							\
  }


OZ_BI_define(BIgetOpcode,1,1)
{
  oz_declareAtomIN(0,opname);

  Opcode oc = stringToOpcode(opname);
  if (oc == OZERROR) {
    return oz_raise(E_ERROR,AtomAssembler,
		    "unknownInstruction",1,OZ_in(0));
  }
  OZ_RETURN_INT(oc);
} OZ_BI_end


OZ_BI_define(BIgetInstructionSizes,0,1) {

  const char ** c = comp_instr;
  TaggedRef ais = oz_nil();

  while (*c) {
    const char * f = *c++;
    const char * t = *c++;
    if (!t)
      t = f;
    ais = oz_cons(oz_pair2(oz_atomNoDup(f),
			   makeTaggedSmallInt(sizeOf(stringToOpcode(t)))),
		  ais);
  }

  OZ_RETURN(OZ_recordInit(oz_atomNoDup("sizes"),ais));
} OZ_BI_end


OZ_BI_define(BIallocateCodeBlock,2,2)
{
  oz_declareIntIN(0,size);
  oz_declareNonvarIN(1,globals);

  if (size < 0) {
    return oz_raise(E_ERROR,AtomAssembler,
		    "illegalCodeBlockSize",1,OZ_in(0));
  }
  int numGlobals = OZ_length(globals);
  if (numGlobals == -1) {
    oz_typeError(1,"List");
  }

  CodeArea *code = new CodeArea(size);
  const int maxX=1; // uses only X[0] in tailCall(x(0) 0)
  PrTabEntry *pte = new PrTabEntry(OZ_atom("toplevelAbstraction"),
				   mkTupleWidth(0), AtomEmpty, 0, -1, oz_nil(),
				   maxX);
  pte->setGSize(numGlobals);
  pte->PC = code->getStart();

  Assert(oz_onToplevel());
  Abstraction *p = Abstraction::newAbstraction(pte,oz_currentBoard());

  globals = oz_deref(globals);
  for (int i = 0; i < numGlobals; i++) {
    p->initG(i,oz_head(globals));
    globals = oz_deref(oz_tail(globals));
  }

  OZ_out(0) = OZ_makeForeignPointer(code);
  OZ_out(1) = makeTaggedConst(p);
  return PROCEED;
} OZ_BI_end


#define OZ_declareCodeBlockIN(num,name)			\
  CodeArea *name;					\
  {							\
    OZ_declareForeignPointer(num,__aux);		\
    name = (CodeArea *) __aux;				\
  }


OZ_BI_define(BIaddDebugInfo,3,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareNonvarIN(1,file);
  if (!oz_isAtom(file)) {
    oz_typeError(1,"Atom");
  }
  oz_declareIntIN(2,line);
  code->writeDebugInfo(file,line);
  return PROCEED;
} OZ_BI_end


#ifdef DEBUG_CHECK
static Opcode lastOpcode=OZERROR;
#endif

OZ_BI_define(BIstoreOpcode,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareIntIN(1,i);
  if (i < 0 || i >= (int) OZERROR) {
    return oz_raise(E_ERROR,AtomAssembler,
		    "unknownInstruction",1,OZ_in(1));
  }
  code->writeOpcode((Opcode) i);
  DebugCheckT(lastOpcode = (Opcode) i);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreNumber,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareNonvarIN(1,arg);
  if (!oz_isNumber(arg)) {
    oz_typeError(1,"Int");
  }
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreLiteral,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareNonvarIN(1,arg);
  if (!oz_isLiteral(arg)) {
    oz_typeError(1,"Literal");
  }
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreFeature,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareNonvarIN(1,arg);
  if (!oz_isFeature(arg)) {
    oz_typeError(1,"Feature");
  }
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreConstant,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareIN(1,arg);
  if (!OZ_isVariable(arg)) {
    arg = oz_deref(arg);
  }
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreBuiltinname,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  OZ_declareVirtualString(1,name);
  Builtin *bi = string2CBuiltin(name);
  if (!bi) {
    return oz_raise(E_ERROR,AtomAssembler,
		    "builtinUndefined",1,OZ_in(1));
  }
  code->writeBuiltin(bi);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreXRegisterIndex,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareIntIN(1,i);
  if (i < 0) {
    return oz_raise(E_ERROR,AtomAssembler,
		    "registerIndexOutOfRange",1,OZ_in(1));
  }
  code->writeXReg(i);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreYRegisterIndex,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareIntIN(1,i);
  if (i < 0) {
    return oz_raise(E_ERROR,AtomAssembler,
		    "registerIndexOutOfRange",1,OZ_in(1));
  }
  code->writeYReg(i);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreGRegisterIndex,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareIntIN(1,i);
  if (i < 0) {
    return oz_raise(E_ERROR,AtomAssembler,
		    "registerIndexOutOfRange",1,OZ_in(1));
  }
  code->writeGReg(i);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreInt,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareIntIN(1,i);
  code->writeInt(i);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreLabel,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareIntIN(1,label);
  code->writeLabel(label);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreProcedureRef,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareNonvarIN(1,p);
  if (OZ_isUnit(p)) {
    Assert(lastOpcode==DEFINITION || lastOpcode==DEFINITIONCOPY);
    code->writeAddress(NULL);
  } else {
    OZ_declareForeignPointer(1,predId);
    Assert(predId);
    code->writeAbstractionEntry((AbstractionEntry *) predId);
  }
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstorePredId,6,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareNonvarIN(1,name);
  if (!oz_isAtom(name)) {
    oz_typeError(1,"Atom");
  }
  OZ_declareRecordArityIN(2,arity);
  oz_declareNonvarIN(3,pos);
  if (!(OZ_isUnit(pos) || oz_isTuple(pos) && OZ_width(pos) == 3)) {
    oz_typeError(3,"Coordinates");
  }
  oz_declareNonvarIN(4,flags);
  OZ_Term ret = oz_checkList(flags);
  if (oz_isFalse(ret)) oz_typeError(4,"List");
  if (oz_isRef(ret)) oz_suspendOn(ret);

  oz_declareIntIN(5,maxX);

  PrTabEntry *pte;
  if (OZ_isUnit(pos)) {
    pte = new PrTabEntry(name,arity,AtomEmpty,0,-1,flags,maxX);
  } else {
    pte = new PrTabEntry(name,arity,pos,flags,maxX);
  }
  code->writeAddress(pte);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BInewHashTable,4,0) {
  OZ_declareCodeBlockIN(0,code);
  oz_declareIntIN(1,elbl);
  oz_declareIntIN(2,size);

  IHashTable * ht = IHashTable::allocate(size,code->computeLabel(elbl));
  
  TaggedRef tes = oz_deref(OZ_in(3));

  while (oz_isCons(tes)) {
    LTuple * es = tagged2LTuple(tes);
    SRecord * e = tagged2SRecord(oz_deref(es->getHead()));
    
    TaggedRef e1 = oz_deref(e->getArg(0));
    TaggedRef e2 = oz_deref(e->getArg(1));

    if (oz_eq(e->getLabel(),AtomRecord)) {
      TaggedRef e3 = oz_deref(e->getArg(2));
      Assert(oz_isLiteral(e1));
      SRecordArity ari = getArity(e2);
      int lbl = code->computeLabel(tagged2SmallInt(e3));
      if (oz_eq(e1,AtomCons) && sraIsTuple(ari) &&
	  getTupleWidth(ari) == 2) {
	ht->addLTuple(lbl);
      } else {
	ht->addRecord(e1,ari,lbl);
      }
    } else {
      Assert(oz_eq(e->getLabel(),OZ_atom("scalar")));
      int lbl = code->computeLabel(tagged2SmallInt(e2));
      ht->addScalar(e1,lbl);
    }
    tes = oz_deref(es->getTail());
  }

  Assert(oz_isNil(tes));
  
  code->writeAddress(ht);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreRecordArity,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  OZ_declareRecordArityIN(1,arity);
  code->writeSRecordArity(arity);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreCallMethodInfo,5,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareIntIN(1,regindex);
  oz_declareNonvarIN(2,name);
  if (!oz_isLiteral(name)) {
    oz_typeError(2,"Literal");
  }
  oz_declareBoolIN(3,isTail);
  OZ_declareRecordArityIN(4,arity);

  CallMethodInfo *cmi = new CallMethodInfo(regindex,name,isTail,arity);
  code->writeAddress(cmi);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreGRegRef,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareNonvarIN(1,globals);
  int numGlobals = OZ_length(globals);
  if (numGlobals == -1) {
    oz_typeError(1,"RegisterList");
  }

  AssRegArray *gregs = AssRegArray::allocate(numGlobals);
  globals = oz_deref(globals);
  for (int i = 0; i < numGlobals; i++) {
    OZ_Term reg = oz_deref(oz_head(globals));
    globals = oz_deref(oz_tail(globals));
    if (!oz_isTuple(reg) || OZ_width(reg) != 1) {
      oz_typeError(1,"RegisterList");
    }

    SRecord *rec = tagged2SRecord(reg);
    const char *label = rec->getLabelLiteral()->getPrintName();
    PosInt regType;
    if (!strcmp(label,"x")) {
      regType = K_XReg;
    } else if (!strcmp(label,"y")) {
      regType = K_YReg;
    } else if (!strcmp(label,"g")) {
      regType = K_GReg;
    } else {
      oz_typeError(1,"RegisterList");
    }
    OZ_Term index = oz_deref(rec->getArg(0));
    if (!oz_isSmallInt(index)) {
      oz_typeError(1,"RegisterList");
    }
    (*gregs)[i].set(tagged2SmallInt(index),regType);
  }

  code->writeAddress(gregs);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreLocation,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  oz_declareNonvarIN(1,locs);
  if (!oz_isPair2(locs)) {
    oz_typeError(1,"Location");
  }
  OZ_Term inLocs = oz_deref(oz_left(locs));
  OZ_Term outLocs = oz_deref(oz_right(locs));
  const int inArity = OZ_length(inLocs);
  const int outArity = OZ_length(outLocs);
  if (inArity == -1 || outArity == -1) {
    oz_typeError(1,"Location");
  }

  OZ_Location::initLocation();

  int i;
  for (i = 0; i < inArity; i++) {
    OZ_Term reg = oz_deref(oz_head(inLocs));
    if (!oz_isTuple(reg) || OZ_width(reg) != 1) {
      oz_typeError(1,"Location");
    }
    TaggedRef index = oz_deref(oz_arg(reg,0));
    if (!oz_isSmallInt(index)) {
      oz_typeError(1,"Location");
    }
    int j = tagged2SmallInt(index);
    if (j < 0 || j >= NumberOfXRegisters) {
      return oz_raise(E_ERROR,AtomAssembler,
		      "registerIndexOutOfRange",1,OZ_in(1));
    }
    OZ_Location::set(i,j);
    inLocs = oz_deref(oz_tail(inLocs));
  }
  for (i = 0; i < outArity; i++) {
    OZ_Term reg = oz_deref(oz_head(outLocs));
    if (!oz_isTuple(reg) || OZ_width(reg) != 1) {
      oz_typeError(1,"Location");
    }
    TaggedRef index = oz_deref(oz_arg(reg,0));
    if (!oz_isSmallInt(index)) {
      oz_typeError(1,"Location");
    }
    int j = tagged2SmallInt(index);
    if (j < 0 || j >= NumberOfXRegisters) {
      return oz_raise(E_ERROR,AtomAssembler,
		      "registerIndexOutOfRange",1,OZ_in(1));
    }
    OZ_Location::set(inArity+i,j);
    outLocs = oz_deref(oz_tail(outLocs));
  }

  code->writeAddress(OZ_Location::getLocation(inArity+outArity));
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreCache,2,0)
{
  OZ_declareCodeBlockIN(0,code);
  code->writeCache();
  return PROCEED;
} OZ_BI_end



#define CI_TYPE_OTHER 0
#define CI_TYPE_XREG  1
#define CI_TYPE_YREG  2
#define CI_TYPE_GREG  3

#define CI_MAX_ARG_LEN 16

static TaggedRef ci_ia_to_in   = taggedVoidValue;
static TaggedRef ci_type_xatom;
static TaggedRef ci_type_yatom;
static TaggedRef ci_type_gatom;

#define CIS_OPCODE(op) \
  code->writeOpcode(op);
#define CIS_XREG(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_XREG); \
  code->writeXReg(tagged2SmallInt(t_instr_args[ii]));
#define IS_CI_XREG(ii) \
 (t_instr_type[ii] == CI_TYPE_XREG)
#define CIS_YREG(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_YREG); \
  code->writeYReg(tagged2SmallInt(t_instr_args[ii]));
#define IS_CI_YREG(ii) \
 (t_instr_type[ii] == CI_TYPE_YREG)
#define CIS_GREG(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_GREG); \
  code->writeGReg(tagged2SmallInt(t_instr_args[ii]));
#define IS_CI_GREG(ii) \
 (t_instr_type[ii] == CI_TYPE_GREG)

OZ_BI_define(BIstoreInstr,3,0) {

  // Compute mapping from instruction atom to instruction number
  if (ci_ia_to_in == taggedVoidValue) {
    const char ** c = comp_instr;
    int in = 0;
    TaggedRef ais = oz_nil();
    while (*c) {
      ais = oz_cons(oz_pair2(oz_atomNoDup(*c++),makeTaggedSmallInt(in++)),
		  ais);
      c++;
    }
    ci_ia_to_in = OZ_recordInit(oz_atomNoDup("ci_ia_to_in"),ais);
    OZ_protect(&ci_ia_to_in);
    ci_type_xatom = oz_atomNoDup("x");
    OZ_protect(&ci_type_xatom);
    ci_type_yatom = oz_atomNoDup("y");
    OZ_protect(&ci_type_yatom);
    ci_type_gatom = oz_atomNoDup("g");
    OZ_protect(&ci_type_gatom);
  }
  
  // Parse instruction specification...
  oz_declareNonvarIN(0,t_instr);
  OZ_declareCodeBlockIN(1,code);

  TaggedRef t_instr_label;
  TaggedRef t_instr_num;

  TaggedRef t_instr_args[CI_MAX_ARG_LEN];
  int       t_instr_type[CI_MAX_ARG_LEN];

  if (oz_isAtom(t_instr)) {
    t_instr_label = t_instr;
  } else if (oz_isSTuple(t_instr)) {
    t_instr_label = tagged2SRecord(t_instr)->getLabel();
    for (int i = tagged2SRecord(t_instr)->getWidth(); i--; ) {
      // FIXME: CHECK FOR VAR
      t_instr_args[i] = oz_deref(tagged2SRecord(t_instr)->getArg(i));
      t_instr_type[i] = CI_TYPE_OTHER;
      if (oz_isTuple(t_instr_args[i])) {
	TaggedRef t_type_label = tagged2SRecord(t_instr_args[i])->getLabel();
	if (tagged2SRecord(t_instr_args[i])->getWidth() != 1)
	  goto bomb;
	t_instr_args[i] = tagged2SRecord(t_instr_args[i])->getArg(1);
	if (oz_eq(t_type_label,ci_type_xatom)) 
	  t_instr_type[i] = CI_TYPE_XREG;
	if (oz_eq(t_type_label,ci_type_xatom)) 
	  t_instr_type[i] = CI_TYPE_YREG;
	if (oz_eq(t_type_label,ci_type_xatom)) 
	  t_instr_type[i] = CI_TYPE_GREG;
      }
    }
  } else {
    goto bomb;
  }

  // Map instruction atom to number...
  t_instr_num = tagged2SRecord(ci_ia_to_in)->getArg(t_instr_label);

  if (!t_instr_num)
    goto bomb;

  // proc {StoreInstr Instr CodeBlock LabelDict}
  
  switch (tagged2SmallInt(t_instr_num)) {
  case CI_SKIP:
    CIS_OPCODE(SKIP);
    break;
    /*
  case CI_DEFINITION:
    CIS_OPCODE(DEFINITION);
    CIS_XREG(0);
    CIS_LABEL(1);
    CIS_PREDID(2);
    CIS_PROCREF(3);
    CIS_GREGREF(4);
    break;
      //   [] 'definition'(X1 X2 X3 X4 X5) then
      //      {StoreOpcode CodeBlock Opcodes.'definition'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreLabel CodeBlock X2 LabelDict}
      //      {StorePredId CodeBlock X3}
      //      {StoreProcedureRef CodeBlock X4}
      //      {StoreGRegRef CodeBlock X5}
  case CI_DEFINITIONCOPY:
    CIS_OPCODE(DEFINITIONCOPY);
    CIS_XREG(0);
    CIS_LABEL(1);
    CIS_PREDID(2);
    CIS_PROCREF(3);
    CIS_GREGREF(4);
    break;
    //   [] 'definitionCopy'(X1 X2 X3 X4 X5) then
    //      {StoreOpcode CodeBlock Opcodes.'definitionCopy'}
    //      {StoreXRegisterIndex CodeBlock X1}
    //      {StoreLabel CodeBlock X2 LabelDict}
    //      {StorePredId CodeBlock X3}
    //      {StoreProcedureRef CodeBlock X4}
    //      {StoreGRegRef CodeBlock X5}
  case CI_ENDDEFINITION:
    CIS_OPCODE(ENDDEFINITION);
    CIS_LABEL(0);
    //   [] 'endDefinition'(X1) then
    //      {StoreOpcode CodeBlock Opcodes.'endDefinition'}
    //      {StoreLabel CodeBlock X1 LabelDict}
    */
  case CI_MOVE:
    if (IS_CI_XREG(0) && IS_CI_XREG(1)) {
      CIS_OPCODE(MOVEXX); CIS_XREG(0); CIS_XREG(1);
      break;
      //   [] 'move'(X1=x(_) X2=x(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveXX'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreXRegisterIndex CodeBlock X2}
    }
    if (IS_CI_XREG(0) && IS_CI_YREG(1)) {
      CIS_OPCODE(MOVEXY); CIS_XREG(0); CIS_YREG(1);
      break;
      //   [] 'move'(X1=x(_) X2=y(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveXY'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreYRegisterIndex CodeBlock X2}
    }
    if (IS_CI_YREG(0) && IS_CI_XREG(1)) {
      CIS_OPCODE(MOVEYX); CIS_YREG(0); CIS_XREG(1);
      break;
      //   [] 'move'(X1=y(_) X2=x(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveYX'}
      //      {StoreYRegisterIndex CodeBlock X1}
      //      {StoreXRegisterIndex CodeBlock X2}
    }
    if (IS_CI_YREG(0) && IS_CI_YREG(1)) {
      CIS_OPCODE(MOVEYY); CIS_YREG(0); CIS_YREG(1);
      break;
      //   [] 'move'(X1=y(_) X2=y(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveYY'}
      //      {StoreYRegisterIndex CodeBlock X1}
      //      {StoreYRegisterIndex CodeBlock X2}
    }
    if (IS_CI_GREG(0) && IS_CI_XREG(1)) {
      CIS_OPCODE(MOVEGX); CIS_GREG(0); CIS_XREG(1);
      break;
      //   [] 'move'(X1=g(_) X2=x(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveGX'}
      //      {StoreGRegisterIndex CodeBlock X1}
      //      {StoreXRegisterIndex CodeBlock X2}
    }
    if (IS_CI_GREG(0) && IS_CI_YREG(1)) {
      CIS_OPCODE(MOVEGY); CIS_GREG(0); CIS_YREG(1);
      break;
      //   [] 'move'(X1=g(_) X2=y(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveGY'}
      //      {StoreGRegisterIndex CodeBlock X1}
      //      {StoreYRegisterIndex CodeBlock X2}
    }
    goto bomb_register;
  case CI_MOVEMOVE:
    if (IS_CI_XREG(0) && IS_CI_YREG(1) && IS_CI_XREG(2) && IS_CI_YREG(3)) {
      CIS_OPCODE(MOVEMOVEXYXY);
      CIS_XREG(0); CIS_YREG(1); CIS_XREG(2); CIS_YREG(3);
      break;
      //   [] 'moveMove'(X1=x(_) X2=y(_) X3=x(_) X4=y(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveMoveXYXY'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreYRegisterIndex CodeBlock X2}
      //      {StoreXRegisterIndex CodeBlock X3}
      //      {StoreYRegisterIndex CodeBlock X4}
    }
    if (IS_CI_YREG(0) && IS_CI_XREG(1) && IS_CI_YREG(2) && IS_CI_XREG(3)) {
      CIS_OPCODE(MOVEMOVEYXYX);
      CIS_YREG(0); CIS_XREG(1); CIS_YREG(2); CIS_XREG(3);
      break;
      //   [] 'moveMove'(X1=y(_) X2=x(_) X3=y(_) X4=x(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveMoveYXYX'}
      //      {StoreYRegisterIndex CodeBlock X1}
      //      {StoreXRegisterIndex CodeBlock X2}
      //      {StoreYRegisterIndex CodeBlock X3}
      //      {StoreXRegisterIndex CodeBlock X4}
    }
    if (IS_CI_XREG(0) && IS_CI_YREG(1) && IS_CI_YREG(2) && IS_CI_XREG(3)) {
      CIS_OPCODE(MOVEMOVEXYYX);
      CIS_XREG(0); CIS_YREG(1); CIS_YREG(2); CIS_XREG(3);
      break;
      //   [] 'moveMove'(X1=x(_) X2=y(_) X3=y(_) X4=x(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveMoveXYYX'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreYRegisterIndex CodeBlock X2}
      //      {StoreYRegisterIndex CodeBlock X3}
      //      {StoreXRegisterIndex CodeBlock X4}
    }
    if (IS_CI_YREG(0) && IS_CI_XREG(1) && IS_CI_XREG(2) && IS_CI_YREG(3)) {
      CIS_OPCODE(MOVEMOVEYXXY);
      CIS_YREG(0); CIS_XREG(1); CIS_XREG(2); CIS_YREG(3);
      break;
      //   [] 'moveMove'(X1=y(_) X2=x(_) X3=x(_) X4=y(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'moveMoveYXXY'}
      //      {StoreYRegisterIndex CodeBlock X1}
      //      {StoreXRegisterIndex CodeBlock X2}
      //      {StoreXRegisterIndex CodeBlock X3}
      //      {StoreYRegisterIndex CodeBlock X4}
    }
    goto bomb_register;
  case CI_CREATEVARIABLE:
    if (IS_CI_XREG(0)) {
      CIS_OPCODE(CREATEVARIABLEX); CIS_XREG(0);
      break;
      //   [] 'createVariable'(X1=x(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'createVariableX'}
      //      {StoreXRegisterIndex CodeBlock X1}
    }
    if (IS_CI_YREG(0)) {
      CIS_OPCODE(CREATEVARIABLEY); CIS_YREG(0);
      break;
      //   [] 'createVariable'(X1=y(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'createVariableY'}
      //      {StoreYRegisterIndex CodeBlock X1}
    }
    goto bomb_register;
  case CI_CREATEVARIABLEMOVE:
    if (IS_CI_XREG(0)) {
      CIS_OPCODE(CREATEVARIABLEMOVEX); CIS_XREG(0); CIS_XREG(1);
      break;
      //   [] 'createVariableMove'(X1=x(_) X2) then
      //      {StoreOpcode CodeBlock Opcodes.'createVariableMoveX'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreXRegisterIndex CodeBlock X2}
    }
    if (IS_CI_YREG(0)) {
      CIS_OPCODE(CREATEVARIABLEMOVEY); CIS_XREG(0); CIS_YREG(1);
      break;
      //   [] 'createVariableMove'(X1=y(_) X2) then
      //      {StoreOpcode CodeBlock Opcodes.'createVariableMoveY'}
      //      {StoreYRegisterIndex CodeBlock X1}
      //      {StoreXRegisterIndex CodeBlock X2}
    }
    goto bomb_register;
  case CI_UNIFY:
    if (IS_CI_XREG(1)) {
      CIS_OPCODE(UNIFYXX); CIS_XREG(0); CIS_XREG(1);
      break;
      //   [] 'unify'(X1 X2=x(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'unifyXX'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreXRegisterIndex CodeBlock X2}
    }
    if (IS_CI_YREG(1)) {
      CIS_OPCODE(UNIFYXY); CIS_XREG(0); CIS_YREG(1);
      break;
      //   [] 'unify'(X1 X2=y(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'unifyXY'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreYRegisterIndex CodeBlock X2}
    }
    if (IS_CI_GREG(1)) {
      CIS_OPCODE(UNIFYXG); CIS_XREG(0); CIS_GREG(1);
      break;
      //   [] 'unify'(X1 X2=g(_)) then
      //      {StoreOpcode CodeBlock Opcodes.'unifyXG'}
      //      {StoreXRegisterIndex CodeBlock X1}
      //      {StoreGRegisterIndex CodeBlock X2}
    }
    goto bomb_register;
    /*
   [] 'putRecord'(X1 X2 X3=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'putRecordX'}
      {StoreLiteral CodeBlock X1}
      {StoreRecordArity CodeBlock X2}
      {StoreXRegisterIndex CodeBlock X3}
   [] 'putRecord'(X1 X2 X3=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'putRecordY'}
      {StoreLiteral CodeBlock X1}
      {StoreRecordArity CodeBlock X2}
      {StoreYRegisterIndex CodeBlock X3}
   [] 'putList'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'putListX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'putList'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'putListY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'putConstant'(X1 X2=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'putConstantX'}
      {StoreConstant CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'putConstant'(X1 X2=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'putConstantY'}
      {StoreConstant CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
   [] 'setVariable'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'setVariableX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'setVariable'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'setVariableY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'setValue'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'setValueX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'setValue'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'setValueY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'setValue'(X1=g(_)) then
      {StoreOpcode CodeBlock Opcodes.'setValueG'}
      {StoreGRegisterIndex CodeBlock X1}
   [] 'setConstant'(X1) then
      {StoreOpcode CodeBlock Opcodes.'setConstant'}
      {StoreConstant CodeBlock X1}
   [] 'setProcedureRef'(X1) then
      {StoreOpcode CodeBlock Opcodes.'setProcedureRef'}
      {StoreProcedureRef CodeBlock X1}
   [] 'setVoid'(X1) then
      {StoreOpcode CodeBlock Opcodes.'setVoid'}
      {StoreInt CodeBlock X1}
   [] 'getRecord'(X1 X2 X3=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'getRecordX'}
      {StoreLiteral CodeBlock X1}
      {StoreRecordArity CodeBlock X2}
      {StoreXRegisterIndex CodeBlock X3}
   [] 'getRecord'(X1 X2 X3=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'getRecordY'}
      {StoreLiteral CodeBlock X1}
      {StoreRecordArity CodeBlock X2}
      {StoreYRegisterIndex CodeBlock X3}
   [] 'getRecord'(X1 X2 X3=g(_)) then
      {StoreOpcode CodeBlock Opcodes.'getRecordG'}
      {StoreLiteral CodeBlock X1}
      {StoreRecordArity CodeBlock X2}
      {StoreGRegisterIndex CodeBlock X3}
   [] 'getList'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'getListX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'getList'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'getListY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'getList'(X1=g(_)) then
      {StoreOpcode CodeBlock Opcodes.'getListG'}
      {StoreGRegisterIndex CodeBlock X1}
   [] 'getListValVar'(X1 X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'getListValVarX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreXRegisterIndex CodeBlock X3}
   [] 'unifyVariable'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyVariableX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'unifyVariable'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyVariableY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'unifyValue'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValueX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'unifyValue'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValueY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'unifyValue'(X1=g(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValueG'}
      {StoreGRegisterIndex CodeBlock X1}
   [] 'unifyValVar'(X1=x(_) X2=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValVarXX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'unifyValVar'(X1=x(_) X2=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValVarXY'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
   [] 'unifyValVar'(X1=y(_) X2=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValVarYX'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'unifyValVar'(X1=y(_) X2=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValVarYY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
   [] 'unifyValVar'(X1=g(_) X2=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValVarGX'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'unifyValVar'(X1=g(_) X2=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'unifyValVarGY'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
   [] 'unifyNumber'(X1) then
      {StoreOpcode CodeBlock Opcodes.'unifyNumber'}
      {StoreNumber CodeBlock X1}
   [] 'unifyLiteral'(X1) then
      {StoreOpcode CodeBlock Opcodes.'unifyLiteral'}
      {StoreLiteral CodeBlock X1}
   [] 'unifyVoid'(X1) then
      {StoreOpcode CodeBlock Opcodes.'unifyVoid'}
      {StoreInt CodeBlock X1}
   [] 'getLiteral'(X1 X2=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'getLiteralX'}
      {StoreLiteral CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'getLiteral'(X1 X2=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'getLiteralY'}
      {StoreLiteral CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
   [] 'getLiteral'(X1 X2=g(_)) then
      {StoreOpcode CodeBlock Opcodes.'getLiteralG'}
      {StoreLiteral CodeBlock X1}
      {StoreGRegisterIndex CodeBlock X2}
   [] 'getNumber'(X1 X2=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'getNumberX'}
      {StoreNumber CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'getNumber'(X1 X2=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'getNumberY'}
      {StoreNumber CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
   [] 'getNumber'(X1 X2=g(_)) then
      {StoreOpcode CodeBlock Opcodes.'getNumberG'}
      {StoreNumber CodeBlock X1}
      {StoreGRegisterIndex CodeBlock X2}
   [] 'allocateL'(X1) then
      {StoreOpcode CodeBlock Opcodes.'allocateL'}
      {StoreInt CodeBlock X1}
    */
  case ALLOCATEL1:
    CIS_OPCODE(ALLOCATEL1); break;
  case ALLOCATEL2:
    CIS_OPCODE(ALLOCATEL2); break;
  case ALLOCATEL3:
    CIS_OPCODE(ALLOCATEL3); break;
  case ALLOCATEL4:
    CIS_OPCODE(ALLOCATEL4); break;
  case ALLOCATEL5:
    CIS_OPCODE(ALLOCATEL5); break;
  case ALLOCATEL6:
    CIS_OPCODE(ALLOCATEL6); break;
  case ALLOCATEL7:
    CIS_OPCODE(ALLOCATEL7); break;
  case ALLOCATEL8:
    CIS_OPCODE(ALLOCATEL8); break;
  case ALLOCATEL9:
    CIS_OPCODE(ALLOCATEL9); break;
  case ALLOCATEL10:
    CIS_OPCODE(ALLOCATEL10); break;
  case DEALLOCATEL:
    CIS_OPCODE(DEALLOCATEL); break;
  case DEALLOCATEL1:
    CIS_OPCODE(DEALLOCATEL1); break;
  case DEALLOCATEL2:
    CIS_OPCODE(DEALLOCATEL2); break;
  case DEALLOCATEL3:
    CIS_OPCODE(DEALLOCATEL3); break;
  case DEALLOCATEL4:
    CIS_OPCODE(DEALLOCATEL4); break;
  case DEALLOCATEL5:
    CIS_OPCODE(DEALLOCATEL5); break;
  case DEALLOCATEL6:
    CIS_OPCODE(DEALLOCATEL6); break;
  case DEALLOCATEL7:
    CIS_OPCODE(DEALLOCATEL7); break;
  case DEALLOCATEL8:
    CIS_OPCODE(DEALLOCATEL8); break;
  case DEALLOCATEL9:
    CIS_OPCODE(DEALLOCATEL9); break;
  case DEALLOCATEL10:
    CIS_OPCODE(DEALLOCATEL10); break;
    /*
   [] 'callMethod'(X1 X2) then
      {StoreOpcode CodeBlock Opcodes.'callMethod'}
      {StoreCallMethodInfo CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'callGlobal'(X1 X2) then
      {StoreOpcode CodeBlock Opcodes.'callGlobal'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'call'(X1=x(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'callX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'call'(X1=y(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'callY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'call'(X1=g(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'callG'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'tailCall'(X1=x(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'tailCallX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'tailCall'(X1=g(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'tailCallG'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'callConstant'(X1 X2) then
      {StoreOpcode CodeBlock Opcodes.'callConstant'}
      {StoreConstant CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'callProcedureRef'(X1 X2) then
      {StoreOpcode CodeBlock Opcodes.'callProcedureRef'}
      {StoreProcedureRef CodeBlock X1}
      {StoreInt CodeBlock X2}
   [] 'sendMsg'(X1 X2=x(_) X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'sendMsgX'}
      {StoreLiteral CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreCache CodeBlock X4}
   [] 'sendMsg'(X1 X2=y(_) X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'sendMsgY'}
      {StoreLiteral CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreCache CodeBlock X4}
   [] 'sendMsg'(X1 X2=g(_) X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'sendMsgG'}
      {StoreLiteral CodeBlock X1}
      {StoreGRegisterIndex CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreCache CodeBlock X4}
   [] 'tailSendMsg'(X1 X2=x(_) X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'tailSendMsgX'}
      {StoreLiteral CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreCache CodeBlock X4}
   [] 'tailSendMsg'(X1 X2=y(_) X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'tailSendMsgY'}
      {StoreLiteral CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreCache CodeBlock X4}
   [] 'tailSendMsg'(X1 X2=g(_) X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'tailSendMsgG'}
      {StoreLiteral CodeBlock X1}
      {StoreGRegisterIndex CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreCache CodeBlock X4}
   [] 'getSelf'(X1) then
      {StoreOpcode CodeBlock Opcodes.'getSelf'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'setSelf'(X1) then
      {StoreOpcode CodeBlock Opcodes.'setSelfG'}
      {StoreGRegisterIndex CodeBlock X1}
   [] 'lockThread'(X1 X2) then
      {StoreOpcode CodeBlock Opcodes.'lockThread'}
      {StoreLabel CodeBlock X1 LabelDict}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'inlineAt'(X1 X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'inlineAt'}
      {StoreFeature CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreCache CodeBlock X3}
   [] 'inlineAssign'(X1 X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'inlineAssign'}
      {StoreFeature CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreCache CodeBlock X3}
   [] 'branch'(X1) then
      {StoreOpcode CodeBlock Opcodes.'branch'}
      {StoreLabel CodeBlock X1 LabelDict}
   [] 'exHandler'(X1) then
      {StoreOpcode CodeBlock Opcodes.'exHandler'}
      {StoreLabel CodeBlock X1 LabelDict}
   [] 'popEx' then
      {StoreOpcode CodeBlock Opcodes.'popEx'}
   [] 'return' then
      {StoreOpcode CodeBlock Opcodes.'return'}
   [] 'getReturn'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'getReturnX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'getReturn'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'getReturnY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'getReturn'(X1=g(_)) then
      {StoreOpcode CodeBlock Opcodes.'getReturnG'}
      {StoreGRegisterIndex CodeBlock X1}
   [] 'funReturn'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'funReturnX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'funReturn'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'funReturnY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'funReturn'(X1=g(_)) then
      {StoreOpcode CodeBlock Opcodes.'funReturnG'}
      {StoreGRegisterIndex CodeBlock X1}
   [] 'testLiteral'(X1=x(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testLiteralX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreLiteral CodeBlock X2}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testLiteral'(X1=y(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testLiteralY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreLiteral CodeBlock X2}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testLiteral'(X1=g(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testLiteralG'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreLiteral CodeBlock X2}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testNumber'(X1=x(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testNumberX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreNumber CodeBlock X2}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testNumber'(X1=y(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testNumberY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreNumber CodeBlock X2}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testNumber'(X1=g(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testNumberG'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreNumber CodeBlock X2}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testRecord'(X1=x(_) X2 X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'testRecordX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreLiteral CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreLabel CodeBlock X4 LabelDict}
   [] 'testRecord'(X1=y(_) X2 X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'testRecordY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreLiteral CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreLabel CodeBlock X4 LabelDict}
   [] 'testRecord'(X1=g(_) X2 X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'testRecordG'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreLiteral CodeBlock X2}
      {StoreRecordArity CodeBlock X3}
      {StoreLabel CodeBlock X4 LabelDict}
   [] 'testList'(X1=x(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'testListX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreLabel CodeBlock X2 LabelDict}
   [] 'testList'(X1=y(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'testListY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreLabel CodeBlock X2 LabelDict}
   [] 'testList'(X1=g(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'testListG'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreLabel CodeBlock X2 LabelDict}
   [] 'testBool'(X1=x(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testBoolX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreLabel CodeBlock X2 LabelDict}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testBool'(X1=y(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testBoolY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreLabel CodeBlock X2 LabelDict}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testBool'(X1=g(_) X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testBoolG'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreLabel CodeBlock X2 LabelDict}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'match'(X1=x(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'matchX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreHashTableRef CodeBlock X2 LabelDict}
   [] 'match'(X1=y(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'matchY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreHashTableRef CodeBlock X2 LabelDict}
   [] 'match'(X1=g(_) X2) then
      {StoreOpcode CodeBlock Opcodes.'matchG'}
      {StoreGRegisterIndex CodeBlock X1}
      {StoreHashTableRef CodeBlock X2 LabelDict}
   [] 'getVariable'(X1=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'getVariableX'}
      {StoreXRegisterIndex CodeBlock X1}
   [] 'getVariable'(X1=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'getVariableY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'getVarVar'(X1=x(_) X2=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'getVarVarXX'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'getVarVar'(X1=x(_) X2=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'getVarVarXY'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
   [] 'getVarVar'(X1=y(_) X2=x(_)) then
      {StoreOpcode CodeBlock Opcodes.'getVarVarYX'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'getVarVar'(X1=y(_) X2=y(_)) then
      {StoreOpcode CodeBlock Opcodes.'getVarVarYY'}
      {StoreYRegisterIndex CodeBlock X1}
      {StoreYRegisterIndex CodeBlock X2}
   [] 'getVoid'(X1) then
      {StoreOpcode CodeBlock Opcodes.'getVoid'}
      {StoreInt CodeBlock X1}
   [] debugEntry(X1 X2 X3 X4) then
      {AddDebugInfo CodeBlock X1 X2}
      {StoreOpcode CodeBlock Opcodes.'debugEntry'}
      {StoreLiteral CodeBlock X1}
      {StoreNumber CodeBlock X2}
      {StoreNumber CodeBlock X3}
      {StoreLiteral CodeBlock X4}
   [] 'debugExit'(X1 X2 X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'debugExit'}
      {StoreLiteral CodeBlock X1}
      {StoreNumber CodeBlock X2}
      {StoreNumber CodeBlock X3}
      {StoreLiteral CodeBlock X4}
   [] 'globalVarname'(X1) then
      {StoreOpcode CodeBlock Opcodes.'globalVarname'}
      {StoreConstant CodeBlock X1}
   [] 'localVarname'(X1) then
      {StoreOpcode CodeBlock Opcodes.'localVarname'}
      {StoreConstant CodeBlock X1}
   [] 'clear'(X1) then
      {StoreOpcode CodeBlock Opcodes.'clearY'}
      {StoreYRegisterIndex CodeBlock X1}
   [] 'profileProc' then
      {StoreOpcode CodeBlock Opcodes.'profileProc'}
   [] 'callBI'(X1 X2) then
      {StoreOpcode CodeBlock Opcodes.'callBI'}
      {StoreBuiltinname CodeBlock X1}
      {StoreLocation CodeBlock X2}
   [] 'inlinePlus1'(X1 X2) then
      {StoreOpcode CodeBlock Opcodes.'inlinePlus1'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'inlineMinus1'(X1 X2) then
      {StoreOpcode CodeBlock Opcodes.'inlineMinus1'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
   [] 'inlinePlus'(X1 X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'inlinePlus'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreXRegisterIndex CodeBlock X3}
   [] 'inlineMinus'(X1 X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'inlineMinus'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreXRegisterIndex CodeBlock X3}
   [] 'inlineDot'(X1 X2 X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'inlineDot'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreFeature CodeBlock X2}
      {StoreXRegisterIndex CodeBlock X3}
      {StoreCache CodeBlock X4}
   [] 'testBI'(X1 X2 X3) then
      {StoreOpcode CodeBlock Opcodes.'testBI'}
      {StoreBuiltinname CodeBlock X1}
      {StoreLocation CodeBlock X2}
      {StoreLabel CodeBlock X3 LabelDict}
   [] 'testLT'(X1 X2 X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'testLT'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreXRegisterIndex CodeBlock X3}
      {StoreLabel CodeBlock X4 LabelDict}
   [] 'testLE'(X1 X2 X3 X4) then
      {StoreOpcode CodeBlock Opcodes.'testLE'}
      {StoreXRegisterIndex CodeBlock X1}
      {StoreXRegisterIndex CodeBlock X2}
      {StoreXRegisterIndex CodeBlock X3}
      {StoreLabel CodeBlock X4 LabelDict}

    */
  default:
    goto bomb;
  }
  
  return PROCEED;

 bomb:
 bomb_register:
  return PROCEED;
} OZ_BI_end


/********************************************************************
 * builtins for the compiler
 ******************************************************************** */

OZ_BI_define(BIchunkArityCompiler,1,1)
{
  oz_declareNonvarIN(0,ch);
  if (!oz_isChunk(ch)) {
    oz_typeError(0,"Chunk");
  }

  switch (tagged2Const(ch)->getType()) {
  case Co_Class:
    OZ_RETURN(tagged2ObjectClass(ch)->getArityList());
  case Co_Object:
    OZ_RETURN(tagged2Object(ch)->getArityList());
  case Co_Chunk:
    OZ_RETURN(tagged2SChunk(ch)->getArityList());
  default:
    OZ_RETURN(oz_nil());
  }
} OZ_BI_end

OZ_BI_define(BIfeatureLess,2,1)
{
  oz_declareNonvarIN(0,f1);
  if (!oz_isFeature(f1)) {
    oz_typeError(0,"Feature");
  }
  oz_declareNonvarIN(1,f2);
  if (!oz_isFeature(f2)) {
    oz_typeError(1,"Feature");
  }
  OZ_RETURN(oz_bool(featureCmp(f1,f2) == -1));
} OZ_BI_end

OZ_BI_define(BIconcatenateAtomAndInt,2,1)
{
  // {ConcatenateAtomAndInts S I ?Res} computes:
  //    Res = {String.toAtom {Append {Atom.toString S} {Int.toString I}}}
  oz_declareAtomIN(0,s);
  oz_declareIntIN(1,i);
  char *news = new char[strlen(s) + 12];
  sprintf(news,"%s%d",s,i);
  OZ_Term newa = oz_atom(news);
  delete[] news;
  OZ_RETURN(newa);
} OZ_BI_end

OZ_BI_define(BIisBuiltin,1,1)
{
  oz_declareNonvarIN(0,val);

  OZ_RETURN(oz_bool(oz_isBuiltin(val) && !tagged2Builtin(val)->isSited()));
} OZ_BI_end

OZ_BI_define(BInameVariable,2,0)
{
  oz_declareIN(0,var);
  oz_declareAtomIN(1,name);
  oz_varAddName(var,name);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BInewNamedName,1,1)
{
  oz_declareAtomIN(0,printName);
  Literal *lit = NamedName::newNamedName(printName);
  OZ_RETURN(makeTaggedLiteral(lit));
} OZ_BI_end

OZ_BI_define(BInewCopyableName,1,1)
{
  oz_declareAtomIN(0,printName);
  Literal *lit = NamedName::newCopyableName(printName);
  OZ_RETURN(makeTaggedLiteral(lit));
} OZ_BI_end

OZ_BI_define(BIisCopyableName,1,1)
{
  oz_declareNonvarIN(0,val);
  OZ_RETURN(oz_bool(oz_isLiteral(val) &&
		    tagged2Literal(val)->isCopyableName()));
} OZ_BI_end

OZ_BI_define(BIisUniqueName,1,1)
{
  oz_declareNonvarIN(0,val);
  OZ_RETURN(oz_bool(oz_isLiteral(val) && tagged2Literal(val)->isUniqueName()));
} OZ_BI_end

OZ_BI_define(BInewProcedureRef,0,1)
{
  AbstractionEntry *entry = new AbstractionEntry(NO);
  OZ_RETURN(OZ_makeForeignPointer(entry));
} OZ_BI_end

OZ_BI_define(BInewCopyableProcedureRef,0,1)
{
  AbstractionEntry *entry = new AbstractionEntry(OK);
  OZ_RETURN(OZ_makeForeignPointer(entry));
} OZ_BI_end

OZ_BI_define(BIisCopyableProcedureRef,1,1)
{
  OZ_declareForeignPointer(0,p);
  AbstractionEntry *entry = (AbstractionEntry *) p;
  OZ_RETURN(oz_bool(entry->isCopyable()));
} OZ_BI_end


/* special version of isDet, that does not suspend,
 *  i.e. does not start a network request 
 */
OZ_BI_define(BIisLocalDet,1,1)
{
  oz_declareDerefIN(0,var);
  if (oz_isUVar(var))
    OZ_RETURN(oz_false());

  if (!oz_isCVar(var))
    OZ_RETURN(oz_true());

  OZ_RETURN(oz_bool(oz_check_var_status(tagged2CVar(var))==EVAR_STATUS_DET));
} OZ_BI_end


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modCompilerSupport-if.cc"

#endif

