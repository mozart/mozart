/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  */


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "assemble.hh"
#endif

#include "assemble.hh"
#include "am.hh"
#include "runtime.hh"
#include "indexing.hh"



OZ_C_proc_begin(BIopInfo,2)
{
  OZ_declareAtomArg(0,opname);
  OZ_declareArg(1,ret);

  Opcode oc = CodeArea::stringToOp(opname);
  if (oc == OZERROR) {
    return oz_raise(E_ERROR,OZ_atom("assembler"),"unknownInstruction",1,opname);
  }
  return OZ_unify(ret, OZ_pair2(OZ_int(oc), OZ_int(sizeOf(oc))));
}
OZ_C_proc_end


OZ_C_proc_begin(BInewCodeBlock,2)
{
  OZ_declareIntArg(0,size);
  OZ_declareArg(1,ret);

  CodeArea *code = new CodeArea(size);
  return OZ_unifyInt(ret, ToInt32(code));
}
OZ_C_proc_end


#define declareCodeBlock(num,name)			\
   OZ_declareIntArg(num,__aux);				\
   CodeArea *name = (CodeArea*) ToPointer(__aux);


OZ_C_proc_begin(BImakeProc,3)
{
  declareCodeBlock(0,code);
  OZ_declareNonvarArg(1,globals);
  OZ_declareArg(2,ret);
  globals = deref(globals);
  
  int numglobals = length(globals);
  RefsArray gRegs = (numglobals == 0) ? (RefsArray) NULL : allocateRefsArray(numglobals);

  for (int i = 0; i < numglobals; i++) {
    gRegs[i] = head(globals);
    globals = deref(tail(globals));
  }
  Assert(isNil(globals));

  PrTabEntry *pte = new PrTabEntry(OZ_atom("toplevelAbstraction"),
				   mkTupleWidth(0),nil(),0);
  pte->PC = code->getStart();

  Abstraction *p = new Abstraction (pte, gRegs, am.rootBoard);

  return OZ_unify(ret,makeTaggedConst(p));
}
OZ_C_proc_end


   
OZ_C_proc_begin(BIstoreInt,2)
{
  declareCodeBlock(0,code);
  OZ_declareIntArg(1,i);
  code->writeInt(i);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreReg,2)
{
  declareCodeBlock(0,code);
  OZ_declareIntArg(1,i);
  code->writeReg(i);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreOp,2)
{
  declareCodeBlock(0,code);
  OZ_declareIntArg(1,i);
  Assert(i>=0 && i<(int)OZERROR);
  code->writeOpcode((Opcode)i);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreTagged,2)
{
  declareCodeBlock(0,code);
  OZ_declareNonvarArg(1,atomOrInt);
  atomOrInt = deref(atomOrInt);
  Assert(OZ_isLiteral(atomOrInt) || OZ_isInt(atomOrInt));
  code->writeTagged(atomOrInt);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreBuiltinname,2)
{
  declareCodeBlock(0,code);
  OZ_declareNonvarArg(1,builtin);
  builtin = deref(builtin);
  Assert(isBuiltin(builtin));
  code->writeBuiltin(tagged2Builtin(builtin));
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreLabel,2)
{
  declareCodeBlock(0,code);
  OZ_declareIntArg(1,label);
  code->writeLabel(label);
  return PROCEED;
}
OZ_C_proc_end


static 
SRecordArity getArity(TaggedRef arity)
{
  arity = deref(arity);
  if (isSmallInt(arity)) {
    return mkTupleWidth(smallIntValue(arity));
  } 

  int len=length(arity);
  arity = sortlist(arity,len);
  Arity *ari = aritytable.find(arity);
  return (ari->isTuple()) ? mkTupleWidth(ari->getWidth()) :  mkRecordArity(ari);
}


OZ_C_proc_begin(BIstoreRecordArity,2)
{
  declareCodeBlock(0,code);
  OZ_declareNonvarArg(1,arity);
  code->writeSRecordArity(getArity(arity));
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreCache,2)
{
  declareCodeBlock(0,code);
  OZ_declareNonvarArg(1,ignored);
  code->writeCache();  
  return PROCEED;
}
OZ_C_proc_end



OZ_C_proc_begin(BIstorePredicateRef,2)
{
  declareCodeBlock(0,code);
  OZ_declareIntArg(1,id);
  AbstractionEntry *predId = AbstractionTable::add(id);
  code->writeAddress(predId);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstorePredId,5)
{
  declareCodeBlock(0,code);
  OZ_declareNonvarArg(1,name); name = deref(name);
  OZ_declareNonvarArg(2,arity);
  OZ_declareNonvarArg(3,file); file = deref(file);
  OZ_declareIntArg(4,line);
  PrTabEntry *pte  = new PrTabEntry(name,getArity(arity),file,line);
  code->writeAddress(pte);
  return PROCEED;
}
OZ_C_proc_end


inline 
Bool getBool(TaggedRef t)
{
  return literalEq(deref(t),NameTrue);
}

OZ_C_proc_begin(BIstoreGenCallInfo,6)
{
  declareCodeBlock(0,code);
  OZ_declareIntArg(1,regindex);
  OZ_declareNonvarArg(2,isMethod);
  OZ_declareNonvarArg(3,name); name = deref(name);
  OZ_declareNonvarArg(4,isTail);
  OZ_declareNonvarArg(5,arity);

  GenCallInfoClass *gci = new GenCallInfoClass(regindex,getBool(isMethod),name, 
					       getBool(isTail),getArity(arity));
  code->writeAddress(gci);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreApplMethInfo,3)
{
  declareCodeBlock(0,code);
  OZ_declareNonvarArg(1,name); name = deref(name);
  OZ_declareNonvarArg(2,arity);

  ApplMethInfoClass *ami = new ApplMethInfoClass(name,getArity(arity));
  code->writeAddress(ami);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreGRegRef,2)
{
  declareCodeBlock(0,code);
  OZ_declareNonvarArg(1,globals);
  globals = deref(globals);
  int numglobals = length(globals);

  AssRegArray *gregs = new AssRegArray(numglobals);

  for (int i = 0; i < numglobals; i++) {
    SRecord *rec = tagged2SRecord(deref(head(globals)));
    globals = deref(tail(globals));

    char *label = rec->getLabelLiteral()->getPrintName();
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
}
OZ_C_proc_end


OZ_C_proc_begin(BInewHashTable,4)
{
  declareCodeBlock(0,code);
  OZ_declareIntArg(1,size);
  OZ_declareIntArg(2,elseLabel);
  OZ_declareArg(3,ret);

  IHashTable *ht = new IHashTable(size,code->getStart()+elseLabel);
  
  code->writeAddress(ht);
  return OZ_unifyInt(ret, ToInt32(ht));
}
OZ_C_proc_end


#define declareHTable(num,name)				\
   OZ_declareIntArg(num,__aux1);			\
   IHashTable *name = (IHashTable*) ToPointer(__aux1);


OZ_C_proc_begin(BIstoreHTVarLabel,3)
{
  declareCodeBlock(0,code);
  declareHTable(1,ht);
  OZ_declareIntArg(2,label);
  ByteCode *absaddr = code->getStart() + label;

  ht->addVar(absaddr);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreHTScalar,4)
{
  declareCodeBlock(0,code);
  declareHTable(1,ht);
  OZ_declareNonvarArg(2,value);
  OZ_declareIntArg(3,label);
  ByteCode *absaddr = code->getStart() + label;

  value = deref(value);
  if (isLiteral(value)) {
    ht->add(tagged2Literal(value),absaddr);
  } else {
    Assert(isNumber(value));
    ht->add(value,absaddr);
  }

  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstoreHTRecord,5)
{
  declareCodeBlock(0,code);
  declareHTable(1,ht);
  OZ_declareNonvarArg(2,reclabel);
  OZ_declareNonvarArg(3,arity);
  OZ_declareIntArg(4,label);

  SRecordArity ar   = getArity(arity);
  reclabel          = deref(reclabel);
  ByteCode *absaddr = code->getStart() + label;
  
  if (literalEq(reclabel,AtomCons) && sraIsTuple(ar) && getTupleWidth(ar)==2) {
    ht->addList(absaddr);
  } else {
    ht->add(tagged2Literal(reclabel),ar,absaddr);
  }

  return PROCEED;
}
OZ_C_proc_end


static
BIspec biSpec[] = {
  {"opInfo",           2, BIopInfo,           0},
  {"newCodeBlock",     2, BInewCodeBlock,     0},
  {"makeProc",         3, BImakeProc,         0},
  {"storeOp",          2, BIstoreOp,          0},
  {"storeInt",         2, BIstoreInt,         0},
  {"storeReg",         2, BIstoreReg,         0},
  {"storeTagged",      2, BIstoreTagged,      0},
  {"storeBuiltinname", 2, BIstoreBuiltinname, 0},
  {"storeLabel",       2, BIstoreLabel,       0},
  {"storeRecordArity", 2, BIstoreRecordArity, 0},
  {"storeCache",       2, BIstoreCache,       0},
  {"storePredicateRef",2, BIstorePredicateRef,0},
  {"storePredId",      5, BIstorePredId,      0},
  {"storeGenCallInfo", 6, BIstoreGenCallInfo, 0},
  {"storeApplMethInfo",3, BIstoreApplMethInfo, 0},
  {"storeGRegRef",     2, BIstoreGRegRef, 0},

  {"newHashTable",     4, BInewHashTable, 0},
  {"storeHTVarLabel",  3, BIstoreHTVarLabel, 0},
  {"storeHTScalar",    4, BIstoreHTScalar, 0},
  {"storeHTRecord",    5, BIstoreHTRecord, 0},

  {0,0,0,0}
};

void BIinitAssembler()
{
  BIaddSpec(biSpec);
}







