/*
 *  Authors:
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *
 *  Contributor:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Ralf Scheidhauer, 1997
 *    Leif Kornstaedt, 1997-1998
 *    Christian Schulte, 2000
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
#include "dictionary.hh"

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
  "deconsCall", "deconsCallX",
  "tailDeconsCall", "tailDeconsCallX",
  "consCall", "consCallX",
  "tailConsCall", "tailConsCallX",
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
#define CI_DECONSCALL    (CI_TESTLE+1)
#define CI_TAILDECONSCALL    (CI_DECONSCALL+1)
#define CI_CONSCALL    (CI_TAILDECONSCALL+1)
#define CI_TAILCONSCALL   (CI_CONSCALL+1)


#define CI_TYPE_OTHER 0
#define CI_TYPE_XREG  1
#define CI_TYPE_YREG  2
#define CI_TYPE_GREG  3

#define CI_MAX_ARG_LEN 16

static TaggedRef ci_ia_to_in;
static TaggedRef ci_type_xatom;
static TaggedRef ci_type_yatom;
static TaggedRef ci_type_gatom;

void compiler_init(void) {
  // Compute mapping from instruction atom to instruction number
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


static
SRecordArity getArity(TaggedRef arity)
{
  if (oz_isSmallInt(arity)) {
    return mkTupleWidth(tagged2SmallInt(arity));
  } else {
    Assert(oz_isSmallInt(oz_checkList(arity)));
    arity = packlist(arity);
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


#define CI_BOMB() goto bomb;

#define CIS_OPCODE(op) code->writeOpcode(op);


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


#define CIS_CONST(ii) \
  code->writeTagged(oz_safeDeref(tagged2SRecord(t_instr)->getArg(ii)));
#define CIS_LIT(ii)  CIS_CONST(ii)
#define CIS_FEAT(ii) CIS_CONST(ii)
#define CIS_NUM(ii)  CIS_CONST(ii)


#define CIS_INT(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_OTHER); \
  code->writeInt(tagged2SmallInt(t_instr_args[ii]));

static int ci_getlbl(OzDictionary * lbldict, TaggedRef t_lbl) {
  TaggedRef t_ilbl;
  if (lbldict->getArg(t_lbl,t_ilbl) == FAILED)
    return -1;
  t_ilbl = oz_deref(t_ilbl);
  if (!oz_isInt(t_ilbl))
    return -1;
  return oz_intToC(t_ilbl);
}

#define CIS_LBL(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);         \
  { int _lbl = ci_getlbl(lbldict,t_instr_args[ii]);  \
    if (_lbl == -1) CI_BOMB();                       \
    code->writeLabel(_lbl); }


#define CIS_CACHE(ii) \
  code->writeCache();


static OZ_Location * ci_store_location(TaggedRef locs) {
  if (!oz_isPair2(locs))
    return (OZ_Location *) NULL;

  OZ_Term inLocs  = oz_deref(oz_left(locs));
  OZ_Term outLocs = oz_deref(oz_right(locs));
  const int inArity  = OZ_length(inLocs);
  const int outArity = OZ_length(outLocs);

  if (inArity == -1 || outArity == -1)
    return (OZ_Location *) NULL;
  
  OZ_Location::initLocation();

  int i;
  for (i = 0; i < inArity; i++) {
    OZ_Term reg = oz_deref(oz_head(inLocs));

    if (!oz_isTuple(reg) || OZ_width(reg) != 1)
      return (OZ_Location *) NULL;

    TaggedRef index = oz_deref(oz_arg(reg,0));

    if (!oz_isSmallInt(index))
      return (OZ_Location *) NULL;

    int j = tagged2SmallInt(index);

    if (j < 0 || j >= NumberOfXRegisters)
      return (OZ_Location *) NULL;

    OZ_Location::set(i,j);
    inLocs = oz_deref(oz_tail(inLocs));
  }

  for (i = 0; i < outArity; i++) {
    OZ_Term reg = oz_deref(oz_head(outLocs));

    if (!oz_isTuple(reg) || OZ_width(reg) != 1)
      return (OZ_Location *) NULL;

    TaggedRef index = oz_deref(oz_arg(reg,0));

    if (!oz_isSmallInt(index))
      return (OZ_Location *) NULL;

    int j = tagged2SmallInt(index);

    if (j < 0 || j >= NumberOfXRegisters)
      return (OZ_Location *) NULL;

    OZ_Location::set(inArity+i,j);
    outLocs = oz_deref(oz_tail(outLocs));
  }

  return OZ_Location::getLocation(inArity+outArity);
}

#define CIS_LOC(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);                  \
  { OZ_Location * _loc = ci_store_location(t_instr_args[ii]); \
    if (!_loc) CI_BOMB();                                     \
    code->writeAddress(_loc); }


static PrTabEntry * ci_store_predid(TaggedRef t_predid) {
  if (!oz_isSTuple(t_predid) ||
      (tagged2SRecord(t_predid)->getWidth() != 5))
    return (PrTabEntry *) NULL;

  SRecord * predid = tagged2SRecord(t_predid);

  TaggedRef t_name  = oz_getPrintName(oz_deref(predid->getArg(0)));

  TaggedRef t_arity = oz_deref(predid->getArg(1));

  SRecordArity arity = getArity(t_arity);
  if (arity == (SRecordArity) -1)
    return (PrTabEntry *) NULL;


  TaggedRef t_pos   = oz_deref(predid->getArg(2));
  if (!(OZ_isUnit(t_pos) || oz_isTuple(t_pos) && OZ_width(t_pos) == 3))
    return (PrTabEntry *) NULL;

  TaggedRef t_flags = oz_deref(predid->getArg(3));
  OZ_Term ret = oz_checkList(t_flags);
  if (oz_isFalse(ret) || oz_isRef(ret))
    return (PrTabEntry *) NULL;

  TaggedRef t_maxx  = oz_deref(predid->getArg(4));
  if (!oz_isInt(t_maxx))
    return (PrTabEntry *) NULL;

  int maxx = tagged2SmallInt(t_maxx);
  
  return (OZ_isUnit(t_pos) ?
	  new PrTabEntry(t_name,arity,AtomEmpty,0,-1,t_flags,maxx) :
	  new PrTabEntry(t_name,arity,t_pos,t_flags,maxx));
}

#define CIS_PREDID(ii,pte,defCode)			\
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);		\
  pte = ci_store_predid(t_instr_args[ii]);		\
  if (!pte) CI_BOMB();					\
  pte->setPC(defCode);					\
  code->writeAddress(pte);

static AbstractionEntry * ci_store_procref(TaggedRef p) {
  if (OZ_isUnit(p)) {
    return (AbstractionEntry *) NULL;
  } else {
    return (AbstractionEntry *) OZ_getForeignPointer(p);
  }
}


#define CIS_PROCREF(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);                       \
  code->writeAbstractionEntry(ci_store_procref(t_instr_args[ii]));


// kost@ : 'pe' is the PrTabEntry for the 'DEFINITION' we are
//         currently filling up;
static
AssRegArray * ci_store_gregref(TaggedRef globals, PrTabEntry *pe)
{
  int numGlobals = OZ_length(globals);
  if (numGlobals == -1)
    return (AssRegArray *) NULL;

  pe->setGSize(numGlobals);

  AssRegArray * gregs = AssRegArray::allocate(numGlobals);

  for (int i = 0; i < numGlobals; i++) {
    OZ_Term reg = oz_deref(oz_head(globals));
    globals = oz_deref(oz_tail(globals));
    if (!oz_isTuple(reg) || OZ_width(reg) != 1)
      return (AssRegArray *) NULL;

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
      return (AssRegArray *) NULL;
    }

    OZ_Term index = oz_deref(rec->getArg(0));

    if (!oz_isSmallInt(index))
      return (AssRegArray *) NULL;

    (*gregs)[i].set(tagged2SmallInt(index),regType);
  }
  
  return gregs;
}


#define CIS_GREGREF(ii,pe)						\
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);				\
  { AssRegArray * _grgs = ci_store_gregref(t_instr_args[ii], pe);	\
    if (!_grgs) CI_BOMB();						\
    code->writeAddress(_grgs); }


#define CIS_RECAR(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);       \
  { SRecordArity _ar = getArity(t_instr_args[ii]); \
    if (_ar == (SRecordArity) -1) CI_BOMB();       \
    code->writeSRecordArity(_ar); }



static CallMethodInfo * ci_store_cmi(TaggedRef t_cmi) {
  //	 proc {StoreCallMethodInfo CodeBlock
  //	       cmi(g(Index) Name IsTail RecordArity)}
  //	    {BIStoreCallMethodInfo CodeBlock Index Name IsTail RecordArity}
  //	 end
  if (!oz_isSTuple(t_cmi) || (tagged2SRecord(t_cmi)->getWidth() != 4))
    return (CallMethodInfo *) NULL;

  SRecord * cmi = tagged2SRecord(t_cmi);

  TaggedRef t_gri     = oz_deref(cmi->getArg(0));
  TaggedRef t_name    = oz_deref(cmi->getArg(1));
  TaggedRef t_is_tail = oz_deref(cmi->getArg(2));
  TaggedRef t_arity   = oz_deref(cmi->getArg(3));

  if (!oz_isSTuple(t_gri) || 
      (tagged2SRecord(t_gri)->getWidth() != 1) ||
      !oz_eq(tagged2SRecord(t_gri)->getLabel(),ci_type_gatom))
    return (CallMethodInfo *) NULL;
  
  TaggedRef t_ri = oz_deref(tagged2SRecord(t_gri)->getArg(0));
  
  if (!oz_isSmallInt(t_ri))
    return (CallMethodInfo *) NULL;
    
  if (!oz_isLiteral(t_name))
    return (CallMethodInfo *) NULL;

  if (!oz_isBool(t_is_tail))
    return (CallMethodInfo *) NULL;

  SRecordArity arity = getArity(t_arity);

  if (arity == (SRecordArity) -1)
    return (CallMethodInfo *) NULL;

  return new CallMethodInfo(tagged2SmallInt(t_ri),t_name,
			    oz_isTrue(t_is_tail),
			    arity);
}



#define CIS_CMI(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);                \
  { CallMethodInfo * _cmi = ci_store_cmi(t_instr_args[ii]); \
    if (!_cmi) CI_BOMB();                                   \
    code->writeAddress(_cmi); }


#define CIS_BINAME(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);                   \
  if (!OZ_isVirtualString(t_instr_args[ii],NULL)) CI_BOMB();   \
  { Builtin * bi = string2CBuiltin(oz_vs2c(t_instr_args[ii])); \
    if (!bi) CI_BOMB();                                        \
    code->writeBuiltin(bi); }

  
static IHashTable * ci_store_hsh(TaggedRef t_ht, 
				 OzDictionary * lbldict,
				 CodeArea * code) {

  if (!oz_isSTuple(t_ht) || (tagged2SRecord(t_ht)->getWidth() != 2))
    return (IHashTable *) NULL;

  TaggedRef t_elbl = oz_deref(tagged2SRecord(t_ht)->getArg(0));
  TaggedRef t_tes  = oz_deref(tagged2SRecord(t_ht)->getArg(1));

  int size = OZ_length(t_tes);
  if (size == -1)
    return (IHashTable *) NULL;

  int elbl = ci_getlbl(lbldict,t_elbl);
  if (elbl == -1)
    return (IHashTable *) NULL;

  IHashTable * ht = IHashTable::allocate(size, code->computeLabel(elbl));
  
  Assert(!oz_isRef(t_tes));
  while (oz_isLTuple(t_tes)) {
    LTuple * es = tagged2LTuple(t_tes);
    SRecord * e = tagged2SRecord(oz_deref(es->getHead()));
    
    TaggedRef e1 = oz_deref(e->getArg(0));
    TaggedRef e2 = oz_deref(e->getArg(1));

    if (oz_eq(e->getLabel(),AtomOnRecord)) {
      TaggedRef e3 = oz_deref(e->getArg(2));
      Assert(oz_isLiteral(e1));
      SRecordArity ari = getArity(e2);
      int u_lbl = ci_getlbl(lbldict,e3);
      if (u_lbl == -1)
	return (IHashTable *) NULL;
      int r_lbl = code->computeLabel(u_lbl);
      if (oz_eq(e1,AtomCons) && sraIsTuple(ari) &&
	  getTupleWidth(ari) == 2) {
	ht->addLTuple(r_lbl);
      } else {
	ht->addRecord(e1,ari,r_lbl);
      }
    } else {
      Assert(oz_eq(e->getLabel(),OZ_atom("onScalar")));
      int u_lbl = ci_getlbl(lbldict,e2);
      if (u_lbl == -1)
	return (IHashTable *) NULL;
      int r_lbl = code->computeLabel(u_lbl);
      ht->addScalar(e1,r_lbl);
    }
    t_tes = oz_deref(es->getTail());
    Assert(!oz_isRef(t_tes));
  }

  Assert(oz_isNil(t_tes));
  
  return ht;
}


#define CIS_HSH(ii) \
  Assert(t_instr_type[ii] == CI_TYPE_OTHER);                          \
  { IHashTable * _ht = ci_store_hsh(t_instr_args[ii], lbldict, code); \
    if (!_ht) CI_BOMB();                                              \
    code->writeAddress(_ht); }


#define CIS_DBGI(ii1,ii2) \
  Assert(t_instr_type[ii1] == CI_TYPE_OTHER);                \
  Assert(t_instr_type[ii2] == CI_TYPE_OTHER);                \
  if (!oz_isAtom(t_instr_args[ii1])) CI_BOMB();              \
  code->writeDebugInfo(t_instr_args[ii1],t_instr_args[ii2]);




OZ_BI_define(BIstoreInstructions,4,1) {
  oz_declareIntIN(0,size);
  oz_declareNonvarIN(1,globals);
  oz_declareNonvarIN(2,t_instrs);
  oz_declareNonvarIN(3,t_lbldict);

  if (size < 0)
    CI_BOMB();

  int numGlobals;
  numGlobals = OZ_length(globals);

  if (numGlobals == -1)
    CI_BOMB();

  if (!oz_isDictionary(t_lbldict))
    CI_BOMB();
  
  OzDictionary * lbldict;
  lbldict = tagged2Dictionary(t_lbldict);

  CodeArea * code;
  code = new CodeArea(size);

  Abstraction * topl;
  {
    PrTabEntry * pte = new PrTabEntry(oz_atomNoDup("toplevelAbstraction"),
				      mkTupleWidth(0), AtomEmpty, 0, -1, 
				      oz_nil(), 1);
    pte->setGSize(numGlobals);
    pte->setPC(code->getStart());

    Assert(oz_onToplevel());
    topl = Abstraction::newAbstraction(pte,oz_currentBoard());

    globals = oz_deref(globals);
    for (int i = 0; i < numGlobals; i++) {
      topl->initG(i,oz_head(globals));
      globals = oz_deref(oz_tail(globals));
    }
  }

  TaggedRef t_instr;

  Assert(!oz_isRef(t_instrs));
  while (oz_isLTuple(t_instrs)) {
    t_instr = oz_deref(oz_head(t_instrs));
    TaggedRef t_instr_label;
    TaggedRef t_instr_num;

    TaggedRef t_instr_args[CI_MAX_ARG_LEN];
    int       t_instr_type[CI_MAX_ARG_LEN];

    if (oz_isAtom(t_instr)) {
      t_instr_label = t_instr;
    } else if (oz_isSTuple(t_instr)) {
      t_instr_label = tagged2SRecord(t_instr)->getLabel();
      for (int i = tagged2SRecord(t_instr)->getWidth(); i--; ) {
	t_instr_args[i] = oz_deref(tagged2SRecord(t_instr)->getArg(i));
	t_instr_type[i] = CI_TYPE_OTHER;
	if (oz_isSTuple(t_instr_args[i]) && 
	    (tagged2SRecord(t_instr_args[i])->getWidth() == 1)) {
	  TaggedRef t_type_label = tagged2SRecord(t_instr_args[i])->getLabel();
	  if (oz_eq(t_type_label,ci_type_xatom)) 
	    t_instr_type[i] = CI_TYPE_XREG;
	  if (oz_eq(t_type_label,ci_type_yatom)) 
	    t_instr_type[i] = CI_TYPE_YREG;
	  if (oz_eq(t_type_label,ci_type_gatom)) 
	    t_instr_type[i] = CI_TYPE_GREG;
	  if (t_instr_type[i] != CI_TYPE_OTHER)
	    t_instr_args[i] = oz_deref(tagged2SRecord(t_instr_args[i])->getArg(0));
	}
      }
    } else {
      CI_BOMB();
    }

    // Map instruction atom to number...
    t_instr_num = tagged2SRecord(ci_ia_to_in)->getFeature(t_instr_label);
    if (!t_instr_num)
      CI_BOMB();
  
    switch (tagged2SmallInt(t_instr_num)) {
    case CI_SKIP:
      CIS_OPCODE(SKIP);
      break;

    case CI_DEFINITION:
      {
	// kost@ : since we load the code straight into the runtime
	//         system, we can write PC"s into it;
	PrTabEntry *pte;
	ProgramCounter defCode =
	  code->getWritePtr() + sizeOf(DEFINITION);
	CIS_OPCODE(DEFINITION);
	CIS_XREG(0);
	CIS_LBL(1);
	CIS_PREDID(2, pte, defCode);
	CIS_PROCREF(3);
	CIS_GREGREF(4, pte);
      }
      break;

    case CI_DEFINITIONCOPY:
      {
	PrTabEntry *pte;
	ProgramCounter defCode =
	  code->getWritePtr() + sizeOf(DEFINITIONCOPY);
	CIS_OPCODE(DEFINITIONCOPY);
	CIS_XREG(0);
	CIS_LBL(1);
	CIS_PREDID(2, pte, defCode);
	CIS_PROCREF(3);
	CIS_GREGREF(4, pte);
      }
      break;

    case CI_ENDDEFINITION:
      CIS_OPCODE(ENDDEFINITION); CIS_LBL(0);
      break;
    case CI_MOVE:
      if (IS_CI_XREG(0) && IS_CI_XREG(1)) {
	CIS_OPCODE(MOVEXX); CIS_XREG(0); CIS_XREG(1);
	break;
      }
      if (IS_CI_XREG(0) && IS_CI_YREG(1)) {
	CIS_OPCODE(MOVEXY); CIS_XREG(0); CIS_YREG(1);
	break;
      }
      if (IS_CI_YREG(0) && IS_CI_XREG(1)) {
	CIS_OPCODE(MOVEYX); CIS_YREG(0); CIS_XREG(1);
	break;
      }
      if (IS_CI_YREG(0) && IS_CI_YREG(1)) {
	CIS_OPCODE(MOVEYY); CIS_YREG(0); CIS_YREG(1);
	break;
      }
      if (IS_CI_GREG(0) && IS_CI_XREG(1)) {
	CIS_OPCODE(MOVEGX); CIS_GREG(0); CIS_XREG(1);
	break;
      }
      if (IS_CI_GREG(0) && IS_CI_YREG(1)) {
	CIS_OPCODE(MOVEGY); CIS_GREG(0); CIS_YREG(1);
	break;
      }
      CI_BOMB();
    case CI_MOVEMOVE:
      if (IS_CI_XREG(0) && IS_CI_YREG(1) && IS_CI_XREG(2) && IS_CI_YREG(3)) {
	CIS_OPCODE(MOVEMOVEXYXY);
	CIS_XREG(0); CIS_YREG(1); CIS_XREG(2); CIS_YREG(3);
	break;
      }
      if (IS_CI_YREG(0) && IS_CI_XREG(1) && IS_CI_YREG(2) && IS_CI_XREG(3)) {
	CIS_OPCODE(MOVEMOVEYXYX);
	CIS_YREG(0); CIS_XREG(1); CIS_YREG(2); CIS_XREG(3);
	break;
      }
      if (IS_CI_XREG(0) && IS_CI_YREG(1) && IS_CI_YREG(2) && IS_CI_XREG(3)) {
	CIS_OPCODE(MOVEMOVEXYYX);
	CIS_XREG(0); CIS_YREG(1); CIS_YREG(2); CIS_XREG(3);
	break;
      }
      if (IS_CI_YREG(0) && IS_CI_XREG(1) && IS_CI_XREG(2) && IS_CI_YREG(3)) {
	CIS_OPCODE(MOVEMOVEYXXY);
	CIS_YREG(0); CIS_XREG(1); CIS_XREG(2); CIS_YREG(3);
	break;
      }
      CI_BOMB();
    case CI_CREATEVARIABLE:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(CREATEVARIABLEX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(CREATEVARIABLEY); CIS_YREG(0);
	break;
      }
      CI_BOMB();
    case CI_CREATEVARIABLEMOVE:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(CREATEVARIABLEMOVEX); CIS_XREG(0); CIS_XREG(1);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(CREATEVARIABLEMOVEY); CIS_YREG(0); CIS_XREG(1);
	break;
      }
      CI_BOMB();
    case CI_UNIFY:
      if (IS_CI_XREG(1)) {
	CIS_OPCODE(UNIFYXX); CIS_XREG(0); CIS_XREG(1);
	break;
      }
      if (IS_CI_YREG(1)) {
	CIS_OPCODE(UNIFYXY); CIS_XREG(0); CIS_YREG(1);
	break;
      }
      if (IS_CI_GREG(1)) {
	CIS_OPCODE(UNIFYXG); CIS_XREG(0); CIS_GREG(1);
	break;
      }
      CI_BOMB();
    case CI_PUTRECORD:
      if (IS_CI_XREG(2)) {
	CIS_OPCODE(PUTRECORDX); CIS_LIT(0); CIS_RECAR(1); CIS_XREG(2);
	break;
      }
      if (IS_CI_YREG(2)) {
	CIS_OPCODE(PUTRECORDY); CIS_LIT(0); CIS_RECAR(1); CIS_YREG(2);
	break;
      }
      CI_BOMB();
    case CI_PUTLIST:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(PUTLISTX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(PUTLISTY); CIS_YREG(0);
	break;
      }
      CI_BOMB();
    case CI_PUTCONSTANT:
      if (IS_CI_XREG(1)) {
	CIS_OPCODE(PUTCONSTANTX); CIS_CONST(0); CIS_XREG(1);
	break;
      }
      if (IS_CI_YREG(1)) {
	CIS_OPCODE(PUTCONSTANTY); CIS_CONST(0); CIS_YREG(1);
	break;
      }
      CI_BOMB();
    case CI_SETVARIABLE:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(SETVARIABLEX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(SETVARIABLEY); CIS_YREG(0);
	break;
      }
      CI_BOMB();
    case CI_SETVALUE:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(SETVALUEX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(SETVALUEY); CIS_YREG(0);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(SETVALUEG); CIS_GREG(0);
	break;
      }
      CI_BOMB();
    case CI_SETCONSTANT:
      CIS_OPCODE(SETCONSTANT); CIS_CONST(0);
      break;
    case CI_SETPROCEDUREREF:
      CIS_OPCODE(SETPROCEDUREREF); CIS_PROCREF(0);
      break;
    case CI_SETVOID:
      CIS_OPCODE(SETVOID); CIS_INT(0);
      break;
    case CI_GETRECORD:
      if (IS_CI_XREG(2)) {
	CIS_OPCODE(GETRECORDX); CIS_LIT(0); CIS_RECAR(1); CIS_XREG(2);
	break;
      }
      if (IS_CI_YREG(2)) {
	CIS_OPCODE(GETRECORDY); CIS_LIT(0); CIS_RECAR(1); CIS_YREG(2);
	break;
      }
      if (IS_CI_GREG(2)) {
	CIS_OPCODE(GETRECORDG); CIS_LIT(0); CIS_RECAR(1); CIS_GREG(2);
	break;
      }
      CI_BOMB();
    case CI_GETLIST:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(GETLISTX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(GETLISTY); CIS_YREG(0);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(GETLISTG); CIS_GREG(0);
	break;
      }
      CI_BOMB();
    case CI_GETLISTVALVAR:
      CIS_OPCODE(GETLISTVALVARX); CIS_XREG(0); CIS_XREG(1); CIS_XREG(2);
      break;
    case CI_UNIFYVARIABLE:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(UNIFYVARIABLEX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(UNIFYVARIABLEY); CIS_YREG(0);
	break;
      }
      CI_BOMB();
    case CI_UNIFYVALUE:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(UNIFYVALUEX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(UNIFYVALUEY); CIS_YREG(0);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(UNIFYVALUEG); CIS_GREG(0);
	break;
      }
      CI_BOMB();
    case CI_UNIFYVALVAR:
      if (IS_CI_XREG(0)) {
	if (IS_CI_XREG(1)) {
	  CIS_OPCODE(UNIFYVALVARXX); CIS_XREG(0); CIS_XREG(1);
	  break;
	}
	if (IS_CI_YREG(1)) {
	  CIS_OPCODE(UNIFYVALVARXY); CIS_XREG(0); CIS_YREG(1);
	  break;
	}
      }
      if (IS_CI_YREG(0)) {
	if (IS_CI_XREG(1)) {
	  CIS_OPCODE(UNIFYVALVARYX); CIS_YREG(0); CIS_XREG(1);
	  break;
	}
	if (IS_CI_YREG(1)) {
	  CIS_OPCODE(UNIFYVALVARYY); CIS_YREG(0); CIS_YREG(1);
	  break;
	}
      }
      if (IS_CI_GREG(0)) {
	if (IS_CI_XREG(1)) {
	  CIS_OPCODE(UNIFYVALVARGX); CIS_GREG(0); CIS_XREG(1);
	  break;
	}
	if (IS_CI_YREG(1)) {
	  CIS_OPCODE(UNIFYVALVARGY); CIS_GREG(0); CIS_YREG(1);
	  break;
	}
      }
      CI_BOMB();
    case CI_UNIFYNUMBER:
      CIS_OPCODE(UNIFYNUMBER); CIS_NUM(0);
      break;
    case CI_UNIFYLITERAL:
      CIS_OPCODE(UNIFYLITERAL); CIS_LIT(0);
      break;
    case CI_UNIFYVOID:
      CIS_OPCODE(UNIFYVOID); CIS_INT(0);
      break;
    case CI_GETLITERAL:
      if (IS_CI_XREG(1)) {
	CIS_OPCODE(GETLITERALX); CIS_LIT(0); CIS_XREG(1);
	break;
      }
      if (IS_CI_YREG(1)) {
	CIS_OPCODE(GETLITERALY); CIS_LIT(0); CIS_YREG(1);
	break;
      }
      if (IS_CI_GREG(1)) {
	CIS_OPCODE(GETLITERALG); CIS_LIT(0); CIS_GREG(1);
	break;
      }
      CI_BOMB();
    case CI_GETNUMBER:
      if (IS_CI_XREG(1)) {
	CIS_OPCODE(GETNUMBERX); CIS_NUM(0); CIS_XREG(1);
	break;
      }
      if (IS_CI_YREG(1)) {
	CIS_OPCODE(GETNUMBERY); CIS_NUM(0); CIS_YREG(1);
	break;
      }
      if (IS_CI_GREG(1)) {
	CIS_OPCODE(GETNUMBERG); CIS_NUM(0); CIS_GREG(1);
	break;
      }
      CI_BOMB();
    case CI_ALLOCATEL:
      CIS_OPCODE(ALLOCATEL); CIS_INT(0);
      break;
    case CI_ALLOCATEL1:
      CIS_OPCODE(ALLOCATEL1);
      break;
    case CI_ALLOCATEL2:
      CIS_OPCODE(ALLOCATEL2);
      break;
    case CI_ALLOCATEL3:
      CIS_OPCODE(ALLOCATEL3);
      break;
    case CI_ALLOCATEL4:
      CIS_OPCODE(ALLOCATEL4);
      break;
    case CI_ALLOCATEL5:
      CIS_OPCODE(ALLOCATEL5);
      break;
    case CI_ALLOCATEL6:
      CIS_OPCODE(ALLOCATEL6);
      break;
    case CI_ALLOCATEL7:
      CIS_OPCODE(ALLOCATEL7);
      break;
    case CI_ALLOCATEL8:
      CIS_OPCODE(ALLOCATEL8);
      break;
    case CI_ALLOCATEL9:
      CIS_OPCODE(ALLOCATEL9);
      break;
    case CI_ALLOCATEL10:
      CIS_OPCODE(ALLOCATEL10);
      break;
    case CI_DEALLOCATEL:
      CIS_OPCODE(DEALLOCATEL);
      break;
    case CI_DEALLOCATEL1:
      CIS_OPCODE(DEALLOCATEL1);
      break;
    case CI_DEALLOCATEL2:
      CIS_OPCODE(DEALLOCATEL2);
      break;
    case CI_DEALLOCATEL3:
      CIS_OPCODE(DEALLOCATEL3);
      break;
    case CI_DEALLOCATEL4:
      CIS_OPCODE(DEALLOCATEL4);
      break;
    case CI_DEALLOCATEL5:
      CIS_OPCODE(DEALLOCATEL5);
      break;
    case CI_DEALLOCATEL6:
      CIS_OPCODE(DEALLOCATEL6);
      break;
    case CI_DEALLOCATEL7:
      CIS_OPCODE(DEALLOCATEL7);
      break;
    case CI_DEALLOCATEL8:
      CIS_OPCODE(DEALLOCATEL8);
      break;
    case CI_DEALLOCATEL9:
      CIS_OPCODE(DEALLOCATEL9);
      break;
    case CI_DEALLOCATEL10:
      CIS_OPCODE(DEALLOCATEL10);
      break;
    case CI_CALLMETHOD:
      CIS_OPCODE(CALLMETHOD); CIS_CMI(0); CIS_INT(1); 
      break;
    case CI_CALLGLOBAL:
      CIS_OPCODE(CALLGLOBAL); CIS_GREG(0); CIS_INT(1); 
      break;
    case CI_CALL:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(CALLX); CIS_XREG(0); CIS_INT(1); 
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(CALLY); CIS_YREG(0); CIS_INT(1); 
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(CALLG); CIS_GREG(0); CIS_INT(1); 
	break;
      }
      CI_BOMB();
    case CI_TAILCALL:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(TAILCALLX); CIS_XREG(0); CIS_INT(1); 
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(TAILCALLG); CIS_GREG(0); CIS_INT(1); 
	break;
      }
      CI_BOMB();
    case CI_CALLCONSTANT:
      CIS_OPCODE(CALLCONSTANT); CIS_CONST(0); CIS_INT(1); 
      break;
    case CI_CALLPROCEDUREREF:
      CIS_OPCODE(CALLPROCEDUREREF); CIS_PROCREF(0); CIS_INT(1); 
      break;
    case CI_SENDMSG:
      if (IS_CI_XREG(1)) {
	CIS_OPCODE(SENDMSGX);
	CIS_LIT(0); CIS_XREG(1); CIS_RECAR(2); CIS_CACHE(3);
	break;
      }
      if (IS_CI_YREG(1)) {
	CIS_OPCODE(SENDMSGY);
	CIS_LIT(0); CIS_YREG(1); CIS_RECAR(2); CIS_CACHE(3);
	break;
      }
      if (IS_CI_GREG(1)) {
	CIS_OPCODE(SENDMSGG);
	CIS_LIT(0); CIS_GREG(1); CIS_RECAR(2); CIS_CACHE(3);
	break;
      }
      CI_BOMB();
    case CI_TAILSENDMSG:
      if (IS_CI_XREG(1)) {
	CIS_OPCODE(TAILSENDMSGX);
	CIS_LIT(0); CIS_XREG(1); CIS_RECAR(2); CIS_CACHE(3);
	break;
      }
      if (IS_CI_YREG(1)) {
	CIS_OPCODE(TAILSENDMSGY);
	CIS_LIT(0); CIS_YREG(1); CIS_RECAR(2); CIS_CACHE(3);
	break;
      }
      if (IS_CI_GREG(1)) {
	CIS_OPCODE(TAILSENDMSGG);
	CIS_LIT(0); CIS_GREG(1); CIS_RECAR(2); CIS_CACHE(3);
	break;
      }
    case CI_GETSELF:
      CIS_OPCODE(GETSELF); CIS_XREG(0);
      break;
    case CI_SETSELF:
      CIS_OPCODE(SETSELFG); CIS_GREG(0);
      break;
    case CI_LOCKTHREAD:
      CIS_OPCODE(LOCKTHREAD); CIS_LBL(0); CIS_XREG(1);
      break;
    case CI_INLINEAT:
      CIS_OPCODE(INLINEAT); CIS_FEAT(0); CIS_XREG(1); CIS_CACHE(2);
      break;
    case CI_INLINEASSIGN:
      CIS_OPCODE(INLINEASSIGN); CIS_FEAT(0); CIS_XREG(1); CIS_CACHE(2);
      break;
    case CI_BRANCH:
      CIS_OPCODE(BRANCH); CIS_LBL(0);
      break;
    case CI_EXHANDLER:
      CIS_OPCODE(EXHANDLER); CIS_LBL(0);
      break;
    case CI_POPEX:
      CIS_OPCODE(POPEX);
      break;
    case CI_RETURN:
      CIS_OPCODE(RETURN);
      break;
    case CI_TESTLITERAL:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(TESTLITERALX); CIS_XREG(0); CIS_LIT(1); CIS_LBL(2);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(TESTLITERALY); CIS_YREG(0); CIS_LIT(1); CIS_LBL(2);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(TESTLITERALG); CIS_GREG(0); CIS_LIT(1); CIS_LBL(2);
	break;
      }
      CI_BOMB();
    case CI_TESTNUMBER:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(TESTNUMBERX); CIS_XREG(0); CIS_NUM(1); CIS_LBL(2);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(TESTNUMBERY); CIS_YREG(0); CIS_NUM(1); CIS_LBL(2);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(TESTNUMBERG); CIS_GREG(0); CIS_NUM(1); CIS_LBL(2);
	break;
      }
      CI_BOMB();
    case CI_TESTRECORD:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(TESTRECORDX); 
	CIS_XREG(0); CIS_LIT(1); CIS_RECAR(2); CIS_LBL(3);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(TESTRECORDY); 
	CIS_YREG(0); CIS_LIT(1); CIS_RECAR(2); CIS_LBL(3);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(TESTRECORDG); 
	CIS_GREG(0); CIS_LIT(1); CIS_RECAR(2); CIS_LBL(3);
	break;
      }
      CI_BOMB();
    case CI_TESTLIST:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(TESTLISTX); CIS_XREG(0); CIS_LBL(1);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(TESTLISTY); CIS_YREG(0); CIS_LBL(1);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(TESTLISTG); CIS_GREG(0); CIS_LBL(1);
	break;
      }
      CI_BOMB();
    case CI_TESTBOOL:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(TESTBOOLX); CIS_XREG(0); CIS_LBL(1); CIS_LBL(2);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(TESTBOOLY); CIS_YREG(0); CIS_LBL(1); CIS_LBL(2);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(TESTBOOLG); CIS_GREG(0); CIS_LBL(1); CIS_LBL(2);
	break;
      }
      CI_BOMB();
    case CI_MATCH:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(MATCHX); CIS_XREG(0); CIS_HSH(1);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(MATCHY); CIS_YREG(0); CIS_HSH(1);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(MATCHG); CIS_GREG(0); CIS_HSH(1);
	break;
      }
      CI_BOMB();
    case CI_GETVARIABLE:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(GETVARIABLEX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(GETVARIABLEY); CIS_YREG(0);
	break;
      }
      CI_BOMB();
    case CI_GETVARVAR:
      if (IS_CI_XREG(0)) {
	if (IS_CI_XREG(1)) {
	  CIS_OPCODE(GETVARVARXX); CIS_XREG(0); CIS_XREG(1);
	  break;
	}
	if (IS_CI_YREG(1)) {
	  CIS_OPCODE(GETVARVARXY); CIS_XREG(0); CIS_YREG(1);
	  break;
	}
      }
      if (IS_CI_YREG(0)) {
	if (IS_CI_XREG(1)) {
	  CIS_OPCODE(GETVARVARYX); CIS_YREG(0); CIS_XREG(1);
	  break;
	}
	if (IS_CI_YREG(1)) {
	  CIS_OPCODE(GETVARVARYY); CIS_YREG(0); CIS_YREG(1);
	  break;
	}
      }
      CI_BOMB();
    case CI_GETVOID:
      CIS_OPCODE(GETVOID); CIS_INT(0);
      break;
    case CI_DEBUGENTRY:
      CIS_DBGI(0,1); 
      CIS_OPCODE(DEBUGENTRY);
      CIS_LIT(0); CIS_NUM(1); CIS_NUM(2); CIS_LIT(3);
      break;
    case CI_DEBUGEXIT:
      CIS_OPCODE(DEBUGEXIT);
      CIS_LIT(0); CIS_NUM(1); CIS_NUM(2); CIS_LIT(3);
      break;
    case CI_GLOBALVARNAME:
      CIS_OPCODE(GLOBALVARNAME); CIS_CONST(0);
      break;
    case CI_LOCALVARNAME:
      CIS_OPCODE(LOCALVARNAME); CIS_CONST(0);
      break;
    case CI_CLEAR:
      CIS_OPCODE(CLEARY); CIS_YREG(0);
      break;
    case CI_PROFILEPROC:
      CIS_OPCODE(PROFILEPROC);
      break;
    case CI_CALLBI:
      CIS_OPCODE(CALLBI); CIS_BINAME(0); CIS_LOC(1);
      break;
    case CI_INLINEPLUS1:
      CIS_OPCODE(INLINEPLUS1); CIS_XREG(0); CIS_XREG(1);
      break;
    case CI_INLINEMINUS1:
      CIS_OPCODE(INLINEMINUS1); CIS_XREG(0); CIS_XREG(1);
      break;
    case CI_INLINEPLUS:
      CIS_OPCODE(INLINEPLUS); CIS_XREG(0); CIS_XREG(1); CIS_XREG(2);
      break;
    case CI_INLINEMINUS:
      CIS_OPCODE(INLINEMINUS); CIS_XREG(0); CIS_XREG(1); CIS_XREG(2);
      break;
    case CI_INLINEDOT:
      CIS_OPCODE(INLINEDOT); 
      CIS_XREG(0); CIS_FEAT(1); CIS_XREG(2); CIS_CACHE(4);
      break;
    case CI_TESTBI:
      CIS_OPCODE(TESTBI); CIS_BINAME(0); CIS_LOC(1); CIS_LBL(2);
      break;
    case CI_TESTLT:
      CIS_OPCODE(TESTLT); CIS_XREG(0); CIS_XREG(1); CIS_XREG(2); CIS_LBL(3);
      break;
    case CI_TESTLE:
      CIS_OPCODE(TESTLE); CIS_XREG(0); CIS_XREG(1); CIS_XREG(2); CIS_LBL(3);
      break;
    case CI_DECONSCALL:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(DECONSCALLX); CIS_XREG(0);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(DECONSCALLY); CIS_YREG(0);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(DECONSCALLG); CIS_GREG(0);
	break;
      }
      CI_BOMB();
    case CI_TAILDECONSCALL:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(TAILDECONSCALLX); CIS_XREG(0);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(TAILDECONSCALLG); CIS_GREG(0);
	break;
      }
      CI_BOMB();
    case CI_CONSCALL:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(CONSCALLX); CIS_XREG(0); CIS_INT(1);
	break;
      }
      if (IS_CI_YREG(0)) {
	CIS_OPCODE(CONSCALLY); CIS_YREG(0); CIS_INT(1);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(CONSCALLG); CIS_GREG(0); CIS_INT(1);
	break;
      }
      CI_BOMB();
    case CI_TAILCONSCALL:
      if (IS_CI_XREG(0)) {
	CIS_OPCODE(TAILCONSCALLX); CIS_XREG(0); CIS_INT(1);
	break;
      }
      if (IS_CI_GREG(0)) {
	CIS_OPCODE(TAILCONSCALLG); CIS_GREG(0); CIS_INT(1);
	break;
      }
      CI_BOMB();
    default:
      CI_BOMB();
    }

    t_instrs = oz_deref(oz_tail(t_instrs));
    Assert(!oz_isRef(t_instrs));
  }
  
  OZ_RETURN(makeTaggedConst(topl));

 bomb:
  OZ_error("Failed to assemble: %s\n", toC(t_instr));
  return FAILED;
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

  Assert(!oz_isRef(var));
  if (oz_isVarOrRef(var)) {
    if (oz_isOptVar(var)) {
      OZ_RETURN(oz_false());
    } else {
      OZ_RETURN(oz_bool(oz_check_var_status(tagged2Var(var))==EVAR_STATUS_DET));
    }
  } else {
    OZ_RETURN(oz_true());
  }
} OZ_BI_end


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modCompilerSupport-if.cc"

#endif

