/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *
 *  Copyright:
 *    Ralf Scheidhauer, 1997
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


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "assemble.hh"
#endif

#include "assemble.hh"
#include "am.hh"
#include "runtime.hh"
#include "indexing.hh"


static
SRecordArity getArity(TaggedRef arity)
{
  arity = deref(arity);
  if (isSmallInt(arity)) {
    return mkTupleWidth(smallIntValue(arity));
  }

  Arity *ari = oz_makeArity(arity);
  return (ari->isTuple())? mkTupleWidth(ari->getWidth()): mkRecordArity(ari);
}


inline
Bool getBool(TaggedRef t)
{
  return literalEq(deref(t),NameTrue);
}


OZ_BI_define(BIgetOpcode,1,1)
{
  OZ_declareAtomIN(0,opname);

  Opcode oc = CodeArea::stringToOp(opname);
  if (oc == OZERROR) {
    return oz_raise(E_ERROR,OZ_atom("assembler"),
                    "unknownInstruction",1,OZ_in(0));
  }
  OZ_RETURN_INT(oc);
} OZ_BI_end


OZ_BI_define(BIgetInstructionSize,1,1)
{
  OZ_declareAtomIN(0,opname);

  Opcode oc = CodeArea::stringToOp(opname);
  if (oc == OZERROR) {
    return oz_raise(E_ERROR,OZ_atom("assembler"),
                    "unknownInstruction",1,OZ_in(0));
  }
  OZ_RETURN_INT(sizeOf(oc));
} OZ_BI_end


OZ_BI_define(BInewCodeBlock,1,1)
{
  OZ_declareIntIN(0,size);

  CodeArea *code = new CodeArea(size);
  OZ_RETURN_INT(ToInt32(code));
} OZ_BI_end


#define declareCodeBlock(num,name)                      \
   OZ_declareIntArg(num,__aux);                         \
   CodeArea *name = (CodeArea*) ToPointer(__aux);

#define NEW_declareCodeBlock(num,name)                  \
   OZ_declareIntIN(num,__aux);                          \
   CodeArea *name = (CodeArea*) ToPointer(__aux);


OZ_BI_define(BImakeProc,2,1)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,globals);
  globals = deref(globals);

  int numglobals = OZ_length(globals);
  RefsArray gRegs =
    numglobals? allocateRefsArray(numglobals): (RefsArray) NULL;

  for (int i = 0; i < numglobals; i++) {
    gRegs[i] = head(globals);
    globals = deref(tail(globals));
  }
  Assert(isNil(globals));

  PrTabEntry *pte = new PrTabEntry(OZ_atom("toplevelAbstraction"),
                                   mkTupleWidth(0),nil(),0,NO);
  pte->PC = code->getStart();

  Assert(am.onToplevel());
  Abstraction *p = new Abstraction (pte, gRegs, am.currentBoard());

  OZ_RETURN(makeTaggedConst(p));
} OZ_BI_end


OZ_BI_define(BIaddDebugInfo,3,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,file); file = deref(file);
  OZ_declareIntIN(2,line);
  code->writeDebugInfo(file,line);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreOpcode,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareIntIN(1,i);
  Assert(i>=0 && i<(int)OZERROR);
  code->writeOpcode((Opcode)i);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreNumber,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,arg);
  arg = deref(arg);
  Assert(OZ_isNumber(arg));
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreLiteral,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,arg);
  arg = deref(arg);
  Assert(OZ_isLiteral(arg));
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreFeature,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,arg);
  arg = deref(arg);
  Assert(OZ_isFeature(arg));
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreConstant,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,arg);
  arg = deref(arg);
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreBuiltinname,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,builtin);
  builtin = deref(builtin);
  Assert(isBuiltin(builtin));
  code->writeBuiltin(tagged2Builtin(builtin));
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreVariablename,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,arg);
  arg = deref(arg);
  Assert(OZ_isAtom(arg));
  code->writeTagged(arg);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreRegisterIndex,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareIntIN(1,i);
  Assert(i >= 0);
  code->writeReg(i);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreInt,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareIntIN(1,i);
  code->writeInt(i);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreLabel,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareIntIN(1,label);
  code->writeLabel(label);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstorePredicateRef,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,p);
  if (OZ_isUnit(p))
    code->writeAddress(NULL);
  else {
    OZ_declareForeignPointerIN(1,predId);
    code->writeAddress((AbstractionEntry *) predId);
  }
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstorePredId,6,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,name); name = deref(name);
  OZ_declareNonvarIN(2,arity);
  OZ_declareNonvarIN(3,file); file = deref(file);
  OZ_declareIntIN(4,line);
  OZ_declareNonvarIN(5,copyOnce);
  PrTabEntry *pte = new PrTabEntry(name,getArity(arity),file,line,OZ_isTrue(copyOnce));
  code->writeAddress(pte);
  return PROCEED;
} OZ_BI_end



OZ_BI_define(BInewHashTable,3,1)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareIntIN(1,size);
  OZ_declareIntIN(2,elseLabel);

  IHashTable *ht = new IHashTable(size,code->computeLabel(elseLabel));

  code->writeAddress(ht);
  OZ_RETURN_INT(ToInt32(ht));
} OZ_BI_end


#define declareHTable(num,name)                         \
   OZ_declareIntArg(num,__aux1);                        \
   IHashTable *name = (IHashTable*) ToPointer(__aux1);

#define NEW_declareHTable(num,name)                             \
   OZ_declareIntIN(num,__aux1);                 \
   IHashTable *name = (IHashTable*) ToPointer(__aux1);


OZ_BI_define(BIstoreHTScalar,4,0)
{
  NEW_declareCodeBlock(0,code);
  NEW_declareHTable(1,ht);
  OZ_declareNonvarIN(2,value);
  OZ_declareIntIN(3,label);

  value = deref(value);
  if (isLiteral(value)) {
    ht->add(tagged2Literal(value),code->computeLabel(label));
  } else {
    Assert(isNumber(value));
    ht->add(value,code->computeLabel(label));
  }

  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreHTRecord,5,0)
{
  NEW_declareCodeBlock(0,code);
  NEW_declareHTable(1,ht);
  OZ_declareNonvarIN(2,reclabel);
  OZ_declareNonvarIN(3,arity);
  OZ_declareIntIN(4,label);

  SRecordArity ar   = getArity(arity);
  reclabel          = deref(reclabel);

  if (literalEq(reclabel,AtomCons) && sraIsTuple(ar) && getTupleWidth(ar)==2) {
    ht->addList(code->computeLabel(label));
  } else {
    ht->add(tagged2Literal(reclabel),ar,code->computeLabel(label));
  }

  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreRecordArity,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,arity);
  code->writeSRecordArity(getArity(arity));
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreGenCallInfo,6,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareIntIN(1,regindex);
  OZ_declareNonvarIN(2,isMethod);
  OZ_declareNonvarIN(3,name); name = deref(name);
  OZ_declareNonvarIN(4,isTail);
  OZ_declareNonvarIN(5,arity);

  GenCallInfoClass *gci =
    new GenCallInfoClass(regindex,getBool(isMethod),name,
                         getBool(isTail),getArity(arity));
  code->writeAddress(gci);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreApplMethInfo,3,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,name); name = deref(name);
  OZ_declareNonvarIN(2,arity);

  ApplMethInfoClass *ami = new ApplMethInfoClass(name,getArity(arity));
  code->writeAddress(ami);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreGRegRef,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,globals);
  globals = deref(globals);
  int numglobals = OZ_length(globals);

  AssRegArray *gregs = new AssRegArray(numglobals);

  for (int i = 0; i < numglobals; i++) {
    SRecord *rec = tagged2SRecord(deref(head(globals)));
    globals = deref(tail(globals));

    const char *label = rec->getLabelLiteral()->getPrintName();
    KindOfReg regType;
    if (strcmp(label,"x")==0) {
      regType = XReg;
    } else if (strcmp(label,"y")==0) {
      regType = YReg;
    } else {
      Assert(strcmp(label,"g")==0);
      regType = GReg;
    }
    (*gregs)[i].kind = regType;
    (*gregs)[i].number = smallIntValue(deref(rec->getArg(0)));
  }

  Assert(isNil(globals));

  code->writeAddress(gregs);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreLocation,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,locs);
  locs=deref(locs);
  OZ_Term inLocs = deref(oz_left(locs));
  OZ_Term outLocs = deref(oz_right(locs));
  const int inArity = OZ_length(inLocs);
  const int outArity = OZ_length(outLocs);

  OZ_Location *loc = OZ_Location::newLocation(inArity,outArity);

  for (int i = 0; i < inArity; i++) {
    OZ_Term reg = deref(head(inLocs));
    loc->in(i) = smallIntValue(deref(oz_arg(reg,0)));
    inLocs = deref(tail(inLocs));
  }

  Assert(isNil(inLocs));

  for (int i = 0; i < outArity; i++) {
    OZ_Term reg = deref(head(outLocs));
    loc->out(i) = smallIntValue(deref(oz_arg(reg,0)));
    outLocs = deref(tail(outLocs));
  }

  Assert(isNil(outLocs));

  code->writeAddress(loc);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIstoreCache,2,0)
{
  NEW_declareCodeBlock(0,code);
  OZ_declareNonvarIN(1,ignored);
  code->writeCache();
  return PROCEED;
} OZ_BI_end
