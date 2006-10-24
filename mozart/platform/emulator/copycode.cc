/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $_Date$ by $_Author$
 *    $_Revision$
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


#include "copycode.hh"
#include "indexing.hh"
#include "value.hh"
#include "codearea.hh"

#define CheckHTPtr(Type,Ptr)				\
  Type aux = (Type) ht->htFind(Ptr);			\
  if ((uint32)ToInt32(aux) != ToInt32(htEmpty))		\
    return aux;
#define CheckHTVal(Type,Val)				\
  Type aux = (Type) ht->htFind(ToPointer(Val));		\
  if ((uint32)ToInt32(aux) != ToInt32(htEmpty))		\
    return aux;

static
inline
TaggedRef checkTagged(TaggedRef t, AddressHashTable *ht) {
  CheckHTVal(TaggedRef, t);
  return t;
}

static
inline
void handleTagged(ProgramCounter PC, AddressHashTable *ht, CodeArea *code) {
  code->writeTagged(checkTagged(getTaggedArg(PC),ht),PC);
}


static
inline
CallMethodInfo *checkGCI(CallMethodInfo *cmi, AddressHashTable *ht)
{
  TaggedRef newname = checkTagged(cmi->mn,ht);

  // GCIs are deleted after use, so always make a copy
  // if (newname == gci->mn)
  //   return gci;

  CheckHTPtr(CallMethodInfo *,cmi);
  
  aux = new CallMethodInfo(cmi->regIndex,newname,cmi->isTailCall,cmi->arity);
  ht->htAdd(cmi, aux);
  return aux;
}


static
inline
void handleCallMethodInfo(ProgramCounter PC, AddressHashTable *ht)
{
  CallMethodInfo *cmi = (CallMethodInfo*)getAdressArg(PC);
  CodeArea::writeAddress(checkGCI(cmi,ht),PC);
}



static 
SRecordArity doCheckSRA(SRecordArity sra, AddressHashTable *ht) {
  Assert(!sraIsTuple(sra))
  CheckHTVal(SRecordArity,sra);

  TaggedRef list = getRecordArity(sra)->getList();
  TaggedRef newlist = oz_nil();

  while(oz_isCons(list)) {
    TaggedRef newt = checkTagged(oz_head(list),ht);
    newlist = oz_cons(newt,newlist);
    list = oz_tail(list);
  }

  newlist = reverseC(newlist);
  Assert(isSorted(newlist));
  aux = mkRecordArity(aritytable.find(newlist));
  ht->htAdd(ToPointer(sra), ToPointer(aux));
  return aux;
}

static
inline
SRecordArity checkSRA(SRecordArity sra, AddressHashTable *ht)
{
  if (sraIsTuple(sra))
    return sra;
  return doCheckSRA(sra,ht);
}

static
inline
PrTabEntry *checkPTE(PrTabEntry *pte, AddressHashTable *ht)
{
  CheckHTPtr(PrTabEntry *,pte);
  
  aux = new PrTabEntry(pte->getName(), checkSRA(pte->getMethodArity(), ht),
		       pte->getFile(), pte->getLine(), pte->getColumn(), 
		       pte->getFlagsList(),
		       pte->getMaxX());
  aux->setGSize(pte->getGSize()); // does not change;
  ht->htAdd(pte, aux);
  return aux;
}


static
inline
void handlePredId(ProgramCounter PC, ProgramCounter defPC,
		  AddressHashTable *ht)
{
  PrTabEntry *pte = checkPTE(getPredArg(PC), ht);
  CodeArea::writeAddress(pte, PC);
  pte->setPC(defPC);
}


static
inline
void handleCache(ProgramCounter PC, CodeArea *code)
{
  code->writeCache(PC);
}


/* the compiler stores AbstractionEntries as ints */

static
inline
AbstractionEntry *checkAE(AbstractionEntry *ae, AddressHashTable *ht)
{
  CheckHTPtr(AbstractionEntry *,ae);
  return ae;
}


static
inline
void handleProcedureRef(ProgramCounter PC, AddressHashTable *ht,
			CodeArea *code)
{
  AbstractionEntry *ae = checkAE((AbstractionEntry*) getAdressArg(PC),ht);
  code->writeAbstractionEntry(ae,PC);
}





static
inline
void handleRecordArity(ProgramCounter PC, AddressHashTable *ht) {
  SRecordArity ff = checkSRA((SRecordArity) getAdressArg(PC), ht);
  CodeArea::writeSRecordArity(ff, PC);
}


static
inline
void handleHashTable(ProgramCounter PC, AddressHashTable * ht) {
  IHashTable * ot = (IHashTable *) getAdressArg(PC);
  IHashTable * nt = ot->clone();
  for (int i = nt->getSize(); i--; ) 
    if (ot->entries[i].val)
      if (oz_isLiteral(ot->entries[i].val))
	nt->addRecord(checkTagged(ot->entries[i].val,ht),
		      checkSRA(ot->entries[i].sra,ht),
		      ot->entries[i].lbl);
      else
	nt->addScalar(ot->entries[i].val,ot->entries[i].lbl);
  CodeArea::writeIHashTable(nt, PC);
}


#define COPY_PREDID(PCR,DEFO)    handlePredId(PC+PCR,PC+DEFO,ht)
#define COPY_PROCEDUREREF(PCR)   handleProcedureRef(PC+PCR,ht,code)
#define COPY_TAGGED(PCR)         handleTagged(PC+PCR,ht,code)
#define COPY_RECORDARITY(PCR)    handleRecordArity(PC+PCR,ht)
#define COPY_CALLMETHODINFO(PCR) handleCallMethodInfo(PC+PCR,ht)
#define COPY_CACHE(PCR)          handleCache(PC+PCR,code)
#define COPY_IHASHTABLE(PCR)     handleHashTable(PC+PCR,ht)

ProgramCounter copyCode(PrTabEntry *ope, PrTabEntry *pe,
			ProgramCounter start, TaggedRef list)
{
  AddressHashTable *ht = new AddressHashTable(100);
  // kost@ : The entry for the whole thing is already known, so:
  ht->htAdd(ope, pe);
  
  list = oz_deref(list);

  Assert(!oz_isRef(list));
  while (oz_isLTuple(list)) {

    TaggedRef key = oz_deref(oz_head(list));
    
    if (oz_isForeignPointer(key)) {
      AbstractionEntry * oldentry =
	(AbstractionEntry *) oz_getForeignPointer(key);
      Assert(oldentry->isCopyable());
      AbstractionEntry *newentry = new AbstractionEntry(NO);
      ht->htAdd(oldentry, newentry);
    } else {
      NamedName *theCopy =
	((NamedName *) tagged2Literal(key))->generateCopy();
      ht->htAdd(ToPointer(key), ToPointer(makeTaggedLiteral(theCopy)));
    }
    list = oz_deref(oz_tail(list));
    Assert(!oz_isRef(list));
  }

  Assert(OZ_isNil(list));

  const int sizeOfDef = 6;
  Assert(sizeOf(DEFINITION) == 6);
  start -= sizeOfDef; // copy DEFINITION instructions as well (for debugging)

  XReg reg;
  int nxt,line,colum;
  TaggedRef file,predName;
  CodeArea::getDefinitionArgs(start,reg,nxt,file,line,colum,predName);

  int size = nxt;
  ProgramCounter ret;

  CodeArea *code = new CodeArea(size);  
  ret = code->getStart();
  memcpy(ret,start,size*sizeof(ByteCode));
  
  ProgramCounter PC = ret;
  ProgramCounter ende = ret+size;

  while (PC<ende) {
    switch(CodeArea::getOpcode(PC)) {
    case PROFILEPROC:
    case RETURN:
    case POPEX:
    case DEALLOCATEL10:
    case DEALLOCATEL9:
    case DEALLOCATEL8:
    case DEALLOCATEL7:
    case DEALLOCATEL6:
    case DEALLOCATEL5:
    case DEALLOCATEL4:
    case DEALLOCATEL3:
    case DEALLOCATEL2:
    case DEALLOCATEL1:
    case DEALLOCATEL:
    case ALLOCATEL10:
    case ALLOCATEL9:
    case ALLOCATEL8:
    case ALLOCATEL7:
    case ALLOCATEL6:
    case ALLOCATEL5:
    case ALLOCATEL4:
    case ALLOCATEL3:
    case ALLOCATEL2:
    case ALLOCATEL1:
    case SKIP:
    case ENDOFFILE:
      PC += 1;
      break;
    case DEFINITIONCOPY:
    case DEFINITION:
      Assert(sizeOf(DEFINITIONCOPY) == 6);
      COPY_PREDID(3, 6);
      COPY_PROCEDUREREF(4);
      PC += 6;
      break;
    case CLEARY:
    case GETVOID:
    case GETVARIABLEY:
    case GETVARIABLEX:
    case FUNRETURNG:
    case FUNRETURNY:
    case FUNRETURNX:
    case GETRETURNG:
    case GETRETURNY:
    case GETRETURNX:
    case EXHANDLER:
    case BRANCH:
    case SETSELFG:
    case GETSELF:
    case ALLOCATEL:
    case UNIFYVOID:
    case UNIFYNUMBER:
    case UNIFYVALUEG:
    case UNIFYVALUEY:
    case UNIFYVALUEX:
    case UNIFYVARIABLEY:
    case UNIFYVARIABLEX:
    case GETLISTG:
    case GETLISTY:
    case GETLISTX:
    case SETVOID:
    case SETVALUEG:
    case SETVALUEY:
    case SETVALUEX:
    case SETVARIABLEY:
    case SETVARIABLEX:
    case PUTLISTY:
    case PUTLISTX:
    case CREATEVARIABLEY:
    case CREATEVARIABLEX:
    case ENDDEFINITION:
      PC += 2;
      break;
    case INLINEMINUS1:
    case INLINEPLUS1:
    case CALLBI:
    case GETVARVARYY:
    case GETVARVARYX:
    case GETVARVARXY:
    case GETVARVARXX:
    case TESTLISTG:
    case TESTLISTY:
    case TESTLISTX:
    case LOCKTHREAD:
    case TAILCALLG:
    case TAILCALLX:
    case CALLG:
    case CALLY:
    case CALLX:
    case CALLGLOBAL:
    case GETNUMBERG:
    case GETNUMBERY:
    case GETNUMBERX:
    case UNIFYVALVARGY:
    case UNIFYVALVARGX:
    case UNIFYVALVARYY:
    case UNIFYVALVARYX:
    case UNIFYVALVARXY:
    case UNIFYVALVARXX:
    case UNIFYXG:
    case UNIFYXY:
    case UNIFYXX:
    case CREATEVARIABLEMOVEY:
    case CREATEVARIABLEMOVEX:
    case MOVEGY:
    case MOVEGX:
    case MOVEYY:
    case MOVEYX:
    case MOVEXY:
    case MOVEXX:
      PC += 3;
      break;
    case TESTLE:
    case TESTLT:
    case MOVEMOVEYXXY:
    case MOVEMOVEXYYX:
    case MOVEMOVEYXYX:
    case MOVEMOVEXYXY:
      PC += 5;
      break;
    case GETRECORDG:
    case GETRECORDY:
    case GETRECORDX:
    case PUTRECORDY:
    case PUTRECORDX:
      COPY_TAGGED(1);
      COPY_RECORDARITY(2);
      PC += 4;
      break;
    case CALLCONSTANT:
    case GETLITERALG:
    case GETLITERALY:
    case GETLITERALX:
    case PUTCONSTANTY:
    case PUTCONSTANTX:
      COPY_TAGGED(1);
      PC += 3;
      break;
    case LOCALVARNAME:
    case GLOBALVARNAME:
    case UNIFYLITERAL:
    case SETCONSTANT:
      COPY_TAGGED(1);
      PC += 2;
      break;
    case SETPROCEDUREREF:
      COPY_PROCEDUREREF(1);
      PC += 2;
      break;
    case TESTBI:
    case INLINEMINUS:
    case INLINEPLUS:
    case TESTBOOLG:
    case TESTBOOLY:
    case TESTBOOLX:
    case TESTNUMBERG:
    case TESTNUMBERY:
    case TESTNUMBERX:
    case GETLISTVALVARX:
      PC += 4;
      break;
    case CALLMETHOD:
      COPY_CALLMETHODINFO(1);
      PC += 3;
      break;
    case FASTTAILCALL:
    case FASTCALL:
    case CALLPROCEDUREREF:
      COPY_PROCEDUREREF(1);
      PC += 3;
      break;
    case TAILSENDMSGG:
    case TAILSENDMSGY:
    case TAILSENDMSGX:
    case SENDMSGG:
    case SENDMSGY:
    case SENDMSGX:
      COPY_TAGGED(1);
      COPY_RECORDARITY(3);
      COPY_CACHE(4);
      PC += 6;
      break;
    case INLINEASSIGN:
    case INLINEAT:
      COPY_TAGGED(1);
      COPY_CACHE(3);
      PC += 5;
      break;
    case TESTLITERALG:
    case TESTLITERALY:
    case TESTLITERALX:
      COPY_TAGGED(2);
      PC += 4;
      break;
    case TESTRECORDG:
    case TESTRECORDY:
    case TESTRECORDX:
      COPY_TAGGED(2);
      COPY_RECORDARITY(3);
      PC += 5;
      break;
    case MATCHG:
    case MATCHY:
    case MATCHX:
      COPY_IHASHTABLE(2);
      PC += 3;
      break;
    case DEBUGEXIT:
      COPY_TAGGED(1);
      COPY_TAGGED(4);
      PC += 5;
      break;
    case DEBUGENTRY:
      CodeArea::writeDebugInfo(PC,
			       getTaggedArg(PC+1),
			       tagged2SmallInt(getNumberArg(PC+2)));
      PC += 5;
      break;
    case INLINEDOT:
      COPY_TAGGED(2);
      COPY_CACHE(4);
      PC += 6;
      break;
    default: 
      Assert(0); 
      break;
    }
  }

  delete ht;
  return ret+sizeOfDef;
}

