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


#define OZ_declareRecordArityIN(num,name)               \
  SRecordArity name;                                    \
  {                                                     \
    oz_declareNonvarIN(num,__aux);                      \
    name = getArity(__aux);                             \
    if (name == (SRecordArity) -1) {                    \
      oz_typeError(num,"RecordArity");                  \
    }                                                   \
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


OZ_BI_define(BIgetInstructionSize,1,1)
{
  oz_declareAtomIN(0,opname);

  Opcode oc = stringToOpcode(opname);
  if (oc == OZERROR) {
    return oz_raise(E_ERROR,AtomAssembler,
                    "unknownInstruction",1,OZ_in(0));
  }
  OZ_RETURN_INT(sizeOf(oc));
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


#define OZ_declareCodeBlockIN(num,name)                 \
  CodeArea *name;                                       \
  {                                                     \
    OZ_declareForeignPointer(num,__aux);                \
    name = (CodeArea *) __aux;                          \
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
