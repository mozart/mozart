/* -*- C++ -*-
 *  Authors:
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    Per Brand <perbrand@sics.se>
 *    Michael Mehl <mehl@dfki.de>
 *    Denys Duchier <duchier@ps.uni-sb.de>
 *    Andreas Sundstroem <andreas@sics.se>
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifdef INTERFACE
#pragma implementation "marshalerBase.hh"
#endif

#include "base.hh"
#include "marshalerBase.hh"
#if !defined(TEXT2PICKLE)
#include <math.h>
#include "mbuffer.hh"
#include "site.hh"
#include "pickle.hh"
#include "gname.hh"
#endif

//

#if !defined(TEXT2PICKLE)
//
unsigned int RobustMarshaler_Max_Shift;
unsigned int RobustMarshaler_Max_Hi_Byte;

//
// Stuff needed for to check that no overflow is done in unmarshalNumber()
void initRobustMarshaler()
{
  unsigned int intsize = sizeof(int);
  unsigned int shft = intsize*7;
  while(shft <= (intsize*8)-7) shft += 7;
  RobustMarshaler_Max_Shift = shft;
  RobustMarshaler_Max_Hi_Byte = 
    (int) pow(2, (intsize*8)-RobustMarshaler_Max_Shift);
}

//
SendRecvCounter dif_counter[DIF_LAST];
SendRecvCounter misc_counter[MISC_LAST];

//
GName *globalizeConst(ConstTerm *t, MarshalerBuffer *bs)  
{ 
  switch(t->getType()) {
  case Co_Object:      return ((Object*)t)->globalize();
  case Co_Class:       return ((ObjectClass*)t)->globalize();
  case Co_Chunk:       return ((SChunk*)t)->globalize();
  case Co_Abstraction: return ((Abstraction*)t)->globalize();
  default: Assert(0); return NULL;
  }
}

//
void skipNumber(MarshalerBuffer *bs)
{
  unsigned int c = bs->get();
  while (c >= SBit)
    c = bs->get();
}

//
// Code area;
//
void marshalBuiltin(GenTraverser *gt, Builtin *entry)
{
  gt->traverseOzValue(makeTaggedConst(entry));
}
void traverseBuiltin(GenTraverser *gt, Builtin *entry)
{
  gt->traverseOzValue(makeTaggedConst(entry));
}


//
#define MARSHALERBUFFER		MarshalerBuffer
#include "marshalerBaseShared.cc"
#undef  MARSHALERBUFFER

//
void traverseRecordArity(GenTraverser *gt, SRecordArity sra)
{
  if (!sraIsTuple(sra))
    gt->traverseOzValue(getRecordArity(sra)->getList());
}

//
void traversePredId(GenTraverser *gt, PrTabEntry *p)
{
  gt->traverseOzValue(p->getName());
  traverseRecordArity(gt, p->getMethodArity());
  gt->traverseOzValue(p->getFile());
  gt->traverseOzValue(p->getFlagsList());
}

//
void traverseCallMethodInfo(GenTraverser *gt, CallMethodInfo *cmi)
{
  gt->traverseOzValue(cmi->mn);
  traverseRecordArity(gt, cmi->arity);
}

//
void traverseHashTableRef(GenTraverser *gt, int start, IHashTable *table)
{
  int sz = table->getSize();
  int entries = table->getEntries();

  for (int i = table->getSize(); i--; ) {
    if (table->entries[i].val) {
      if (oz_isLiteral(table->entries[i].val)) {
	if (table->entries[i].sra == mkTupleWidth(0)) {
	  // That's a literal entry
	  gt->traverseOzValue(table->entries[i].val);//makeTaggedLiteral(aux->getLiteral()));
	} else {
	  // That's a record entry
	  gt->traverseOzValue(table->entries[i].val);
	  traverseRecordArity(gt, table->entries[i].sra);
	}
      } else {
	Assert(oz_isNumber(table->entries[i].val));
	// That's a number entry
	gt->traverseOzValue(table->entries[i].val);//aux->getNumber());
      }
    }
  }
}

#ifdef USE_FAST_UNMARSHALER   

//
unsigned int unmarshalNumber(MarshalerBuffer *bs)
{
  unsigned int ret = 0, shft = 0;
  unsigned int c = bs->get();
  while (c >= SBit) {
    ret += ((c-SBit) << shft);
    c = bs->get();
    shft += 7;
  }
  ret |= (c<<shft);
  return ret;
}

//
// MarshalerBuffer's 'getString' does not check overflows etc.;
static
char *getString(MarshalerBuffer *bs, unsigned int i)
{
  char *ret = new char[i+1];
  if (ret == (char *) 0)
    return ((char *) 0);
  for (unsigned int k=0; k<i; k++)
    ret[k] = bs->get();
  ret[i] = '\0';
  return (ret);
}

//
double unmarshalFloat(MarshalerBuffer *bs)
{
  static DoubleConv dc;
#if defined(ARCH_LITTLE_ENDIAN) && !defined(ARCH_BIG_WORDIAN)
    dc.u.i[0] = unmarshalNumber(bs);
    dc.u.i[1] = unmarshalNumber(bs);
#else
    dc.u.i[1] = unmarshalNumber(bs);
    dc.u.i[0] = unmarshalNumber(bs);
#endif
  return dc.u.d;
}

//
char *unmarshalString(MarshalerBuffer *bs)
{
  misc_counter[MISC_STRING].recv();
  unsigned int i = unmarshalNumber(bs);

  return getString(bs,i);
}

//
static
void unmarshalGName1(GName *gname, MarshalerBuffer *bs)
{
  gname->site=unmarshalSite(bs);
  for (int i=0; i<fatIntDigits; i++) {
    gname->id.number[i] = unmarshalNumber(bs);
  }
  gname->gnameType = (GNameType) unmarshalNumber(bs);
}

//
GName *unmarshalGName(TaggedRef *ret, MarshalerBuffer *bs)
{
  misc_counter[MISC_GNAME].recv();
  GName gname;
  unmarshalGName1(&gname,bs);
  
  TaggedRef aux = oz_findGName(&gname);
  if (aux) {
    if (ret) *ret = aux; // ATTENTION
    return 0;
  }

  //
  GName *nn = new GName(gname);
  nn->setValue((TaggedRef) 0);
  return (nn);
}

#else // USE_FAST_UNMARSHALER

//
unsigned int unmarshalNumberRobust(MarshalerBuffer *bs, int *overflow)
{
  unsigned int ret = 0, shft = 0;
  unsigned int c = bs->get();
  while (c >= SBit) {
    ret += ((c-SBit) << shft);
    c = bs->get();
    shft += 7;
  }
  if(shft > RobustMarshaler_Max_Shift) {
    *overflow = OK;
    return 0;
  }
  else if(shft == RobustMarshaler_Max_Shift) {
    if(c >= RobustMarshaler_Max_Hi_Byte) {
      *overflow = OK;
      return 0;
    }
  }
  ret |= (c<<shft);
  *overflow = NO;
  return ret;
}

//
static
char *getStringRobust(MarshalerBuffer *bs, unsigned int i, int *error)
{
  char *ret = new char[i+1];
  if (ret==NULL) {
    *error = OK;
    return NULL;
  }
  for (unsigned int k=0; k<i; k++)
    ret[k] = bs->get();
  ret[i] = '\0';
  *error = NO;
  return ret;
}

//
double unmarshalFloatRobust(MarshalerBuffer *bs, int *overflow)
{
  static DoubleConv dc;
#if defined(ARCH_LITTLE_ENDIAN) && !defined(ARCH_BIG_WORDIAN)
    dc.u.i[0] = unmarshalNumberRobust(bs, overflow);
    if(*overflow) return 0.0;
    dc.u.i[1] = unmarshalNumberRobust(bs, overflow);
    if(*overflow) return 0.0;
#else
    dc.u.i[1] = unmarshalNumberRobust(bs, overflow);
    if(*overflow) return 0.0;
    dc.u.i[0] = unmarshalNumberRobust(bs, overflow);
    if(*overflow) return 0.0;
#endif
  return dc.u.d;
}

//
char *unmarshalStringRobust(MarshalerBuffer *bs, int *error)
{
  char *string;
  misc_counter[MISC_STRING].recv();
  unsigned int i = unmarshalNumberRobust(bs,error);
  if(*error) return NULL;
  string = getStringRobust(bs,i,error);
  if(*error) return NULL;
  return string;
}

//
static
void unmarshalGName1Robust(GName *gname, MarshalerBuffer *bs, int *error)
{
  gname->site = unmarshalSiteRobust(bs, error);
  if(*error) return;
  for (int i=0; i<fatIntDigits; i++) {
    int e;
    unsigned int num;
    num = unmarshalNumberRobust(bs, &e);
    if(*error || (num > maxDigit)) return;
    gname->id.number[i] = num;
  }
  gname->gnameType = (GNameType) unmarshalNumberRobust(bs, error);
  *error = *error || (gname->gnameType > MAX_GNT);
}

//
GName *unmarshalGNameRobust(TaggedRef *ret, MarshalerBuffer *bs, int *error)
{
  misc_counter[MISC_GNAME].recv();
  GName gname;
  unmarshalGName1Robust(&gname, bs, error);
  if (*error) return ((GName *) -1);
  
  TaggedRef aux = oz_findGName(&gname);
  if (aux) {
    if (ret) *ret = aux; // ATTENTION
    return 0;
  }

  //
  GName *nn = new GName(gname);
  nn->setValue((TaggedRef) 0);
  return (nn);
}

#endif

//
// Occasionally Oz terms must be placed in the current binary area
// (which representation will follow in the stream): this is arranged
// with 'putOzValueCA' processor. Note that writing into a code area
// is that code area's business;
class CodeAreaLocation : public GTAbstractEntity,
			 public CppObjMemory {
private:
  ProgramCounter ptr;
  CodeArea *code;
  DebugCode(OzTermTypeCheck tp;);
public:
  CodeAreaLocation(ProgramCounter ptrIn, CodeArea* codeIn
		   DebugArg(OzTermTypeCheck tpIn))
    : ptr(ptrIn), code(codeIn) DebugArg(tp(tpIn)){}
  CodeAreaLocation(ProgramCounter ptrIn)
    : ptr(ptrIn) {
    DebugCode(code = (CodeArea *) -1;);
    DebugCode(tp = (OzTermTypeCheck) -1;);
  }
  virtual ~CodeAreaLocation() {}

  //
  ProgramCounter getPtr() { return (ptr); }
  CodeArea *getCodeArea() { return (code); }
  DebugCode(OzTermTypeCheck getTP() { return (tp); })

  //
  virtual int getType() { return (GT_CodeAreaLoc); }
  virtual void gc() {}
};

//
class CodeAreaOzValueLocation : public CodeAreaLocation {
public:
  CodeAreaOzValueLocation(ProgramCounter ptrIn, CodeArea*
			  codeIn DebugArg(OzTermTypeCheck tpIn))
    : CodeAreaLocation(ptrIn, codeIn DebugArg(tpIn))
  {
    // In the current GC incarnation the location must be eagerly
    // initialized because 'CodeArea::gCollectCodeAreaStart()' is called
    // before (almost) anything else during GC;
    (void) codeIn->writeTagged((OZ_Term) 0, ptrIn);
  }
  virtual ~CodeAreaOzValueLocation() {}

  //
  virtual int getType() { return (GT_CodeAreaOzValueLoc); }
  virtual void gc();
};

//
void CodeAreaOzValueLocation::gc()
{
#if defined(DEBUG_CHECK)
  OZ_Term t = getTaggedArg(getPtr());
  Assert(t == (OZ_Term) 0);
#endif
}

//
// A "CodeAreaLocation" argument ist not sufficient for more
// interesting cases, in which more than one Oz value is used to fill
// up a cell in a code area.
//
// For predicate id"s these are a predicate's name, *optional* record
// arity list, file name and flags list. Given the reversed order they
// appear in stream, the last three must be saved until the
// 'PrTabEntry' be really constructed.
class PredIdLocation : public CodeAreaLocation {
private:
  int line, column, maxX, gSize;
  // 'name' will be supplied last and does not need to be stored;
  SRecordArity sra;		// of a tuple
  OZ_Term arityList;		// of a record;
  OZ_Term file;
  OZ_Term flagsList;
  ProgramCounter defPC;
public:
  PredIdLocation(ProgramCounter ptrIn, ProgramCounter defPCIn)
    : CodeAreaLocation(ptrIn), sra((SRecordArity) 0), defPC(defPCIn)
  {
    CodeArea::writeAddressAllocated((PrTabEntry *) 0, ptrIn);
    DebugCode(line = column = maxX = -1;);
    DebugCode(file = flagsList = arityList = (OZ_Term) -1;);
  }
  virtual ~PredIdLocation() {}

  //
  ProgramCounter getDefPC() { return (defPC); }

  //
  void setSRA(SRecordArity sraIn) { sra = sraIn; }
  SRecordArity getSRA() { return (sra); }
  void setArityList(OZ_Term ra) { arityList = ra; }
  OZ_Term getArityList() { return (arityList); }
  void setFile(OZ_Term fileIn) { file = fileIn; }
  OZ_Term getFile() { return (file); }
  void setFlagsList(OZ_Term flagsListIn ) { flagsList = flagsListIn; }
  OZ_Term getFlagsList() { return (flagsList); }

  //
  void setLine(int lineIn) { line = lineIn; }
  int getLine() { return (line); }
  void setColumn(int columnIn) { column = columnIn; }
  int getColumn() { return (column); }
  void setMaxX(int maxXIn) { maxX = maxXIn; }
  int getMaxX() { return (maxX); }
  void setGSize(int gSizeIn) { gSize = gSizeIn; }
  int getGSize() { return (gSize); }

  //
  virtual int getType() { return (GT_CodeAreaPredIdLoc); }
  virtual void gc();
};

//
void PredIdLocation::gc()
{
#if defined(DEBUG_CHECK)
  void *t = getAdressArg(getPtr());
  Assert(t == (void *) 0);
#endif
}

//
// 
class CallMethodInfoLocation : public CodeAreaLocation {
private:
  int compact;
  SRecordArity sra;		// tuples;
  OZ_Term arityList;		// records;

  //
public:
  CallMethodInfoLocation(ProgramCounter ptrIn, int compactIn)
    : CodeAreaLocation(ptrIn), compact(compactIn), sra((SRecordArity) 0)
  {
    CodeArea::writeAddressAllocated((CallMethodInfo *) 0, ptrIn);
    DebugCode(arityList = (OZ_Term) -1;);
  }
  virtual ~CallMethodInfoLocation() {}

  //
  int getCompact() { return (compact); }
  SRecordArity getSRA() { return (sra); }
  void setSRA(SRecordArity sraIn) { sra = sraIn; }
  OZ_Term getArityList() { return (arityList); }
  void setArityList(OZ_Term ar) { arityList = ar; }

  //
  virtual int getType() { return (GT_CodeAreaMethInfoLoc); }
  virtual void gc();
};

//
void CallMethodInfoLocation::gc()
{
#if defined(DEBUG_CHECK)
  void *t = getAdressArg(getPtr());
  Assert(t == (void *) 0);
#endif
}

//
static void putOzValueCA(GTAbstractEntity *arg, OZ_Term value)
{
  CodeAreaOzValueLocation *loc = (CodeAreaOzValueLocation *) arg;
  //
  Assert((*loc->getTP())(value));
  (void) (loc->getCodeArea())->writeTagged(value, loc->getPtr());
  delete loc;
}

//
// A builtin in a code area is stored not as an 'OzTerm' but as an
// Builtin* (while in the stream it appears as an 'OzTerm'):
static void putBuiltinCA(GTAbstractEntity *arg, OZ_Term value)
{
  CodeAreaLocation *loc = (CodeAreaLocation *) arg;
  //
  Assert(oz_isBuiltin(value));
  CodeArea::writeAddressAllocated(tagged2Builtin(value), loc->getPtr());
  delete loc;
}

//
static inline
SRecordArity makeRealRecordArity(OZ_Term arityList)
{
  Assert(isSorted(arityList));
  Arity *ari = aritytable.find(arityList);
  Assert(!ari->isTuple());
  return (mkRecordArity(ari));
}

//
// Gets a (complete) arity list and puts 'SRecordArity';
static void putRealRecordArityCA(GTAbstractEntity *arg, OZ_Term value) {
  CodeAreaLocation *loc = (CodeAreaLocation *) arg;

  //
  SRecordArity sra = makeRealRecordArity(packlist(value));
  CodeArea::writeWordAllocated(sra, loc->getPtr());
  delete loc;
}

//
// Thus, there are four processors, first three of them save values
// and the last one does the job:
static void getPredIdNameCA(GTAbstractEntity *arg, OZ_Term value)
{
  PredIdLocation *loc = (PredIdLocation *) arg;
  SRecordArity sra = loc->getSRA();

  //
  if (!sra) {
    // must be a record:
    OZ_Term aritylist = packlist(loc->getArityList());
    sra = makeRealRecordArity(aritylist);
  }

  //
  // 'value' is the name argument;
  PrTabEntry *pred = 
    new PrTabEntry(value, 
		   sra, loc->getFile(),
		   loc->getLine(), loc->getColumn(),
		   loc->getFlagsList(), loc->getMaxX());
  CodeArea::writeAddressAllocated(pred, loc->getPtr());
  pred->setPC(loc->getDefPC());
  pred->setGSize(loc->getGSize());
  //
  delete loc;
}

//
// Note that these processors do not delete the argument;
// Note also that 'saveRecordArityPredIdCA' is used only when the
// arity is of a real record (i.e. not a tuple);
static void saveRecordArityPredIdCA(GTAbstractEntity *arg, OZ_Term value)
{
  PredIdLocation *loc = (PredIdLocation *) arg;
  // 'value' *will be* an arity list - now we have only head cell;
  loc->setArityList(value);
}
static void saveFileCA(GTAbstractEntity *arg, OZ_Term value)
{
  PredIdLocation *loc = (PredIdLocation *) arg;
  loc->setFile(value);
}
static void saveFlagsListCA(GTAbstractEntity *arg, OZ_Term value)
{
  PredIdLocation *loc = (PredIdLocation *) arg;
  loc->setFlagsList(value);
}

//
static void getCallMethodInfoNameCA(GTAbstractEntity *arg, OZ_Term value)
{
  CallMethodInfoLocation *loc = (CallMethodInfoLocation *) arg;
  int compact = loc->getCompact();
  SRecordArity sra = loc->getSRA();

  //
  if (!sra) {
    // must be a record:
    OZ_Term aritylist = packlist(loc->getArityList());
    sra = makeRealRecordArity(aritylist);
  }

  //
  int ri      = compact>>1;
  Bool ist    = (compact&1);
  CallMethodInfo *cmi = new CallMethodInfo(ri, value, ist, sra);
  CodeArea::writeAddressAllocated(cmi, loc->getPtr());

  //
  delete loc;
}

//
static void
saveCallMethodInfoRecordArityCA(GTAbstractEntity *arg, OZ_Term value)
{
  CallMethodInfoLocation *loc = (CallMethodInfoLocation *) arg;
  loc->setArityList(value);
}

//
// Processors...
void getHashTableRecordEntryLabelCA(GTAbstractEntity *arg, OZ_Term value)
{
  HashTableEntryDesc *desc = (HashTableEntryDesc *) arg;
  SRecordArity sra = desc->getSRA();

  //
  if (!sra) {
    OZ_Term aritylist = packlist(desc->getArityList());
    sra = makeRealRecordArity(aritylist);
  }
  //
  (desc->getTable())->addRecord(value, sra, desc->getLabel());

  //
  delete desc;
}

//
void saveRecordArityHashTableEntryCA(GTAbstractEntity *arg, OZ_Term value)
{
  HashTableEntryDesc *desc = (HashTableEntryDesc *) arg;
  desc->setArityList(value);
}

//
void getHashTableAtomEntryLabelCA(GTAbstractEntity *arg, OZ_Term value)
{
  HashTableEntryDesc *desc = (HashTableEntryDesc *) arg;

  //
  (desc->getTable())->addScalar(value, desc->getLabel());
  delete desc;
}

//
void getHashTableNumEntryLabelCA(GTAbstractEntity *arg, OZ_Term value)
{
  HashTableEntryDesc *desc = (HashTableEntryDesc *) arg;

  //
  (desc->getTable())->addScalar(value, desc->getLabel());
  delete desc;
}

//
void handleDEBUGENTRY(void *arg)
{
  ProgramCounter PC = (ProgramCounter) arg;
  Assert(PC);

  TaggedRef file = getTaggedArg(PC+1);
  int line = tagged2SmallInt(getNumberArg(PC+2));
  CodeArea::writeDebugInfo(PC, file, line);
}

//
ProgramCounter writeAddress(void *ptr, ProgramCounter pc)
{
  return (pc ? CodeArea::writeAddress(ptr, pc) : (ProgramCounter) 0);
}
ProgramCounter unmarshalCache(ProgramCounter pc, CodeArea *code)
{
  return (pc ? code->writeCache(pc) : 0);
}

#ifdef DEBUG_CHECK

Bool mIsAny(TaggedRef t)
{
  return (OK);
}

// Oz value type check procedures;
Bool mIsNumber(TaggedRef t)
{
  return (oz_isNumber(t));
}

Bool mIsLiteral(TaggedRef t) 
{
  return (oz_isLiteral(t));
}

Bool mIsFeature(TaggedRef t) 
{
  return (oz_isFeature(t));
}

Bool mIsConstant(TaggedRef t)
{
  return (oz_isNumber(t) ||
	  oz_isLiteral(t) ||
	  oz_isLTuple(t) ||
	  oz_isProcedure(t) ||
	  oz_isSRecord(t) ||
	  oz_isExtension(t));
}

#endif

//
ProgramCounter unmarshalOzValue(Builder *b, ProgramCounter pc,
				CodeArea *code DebugArg(OzTermTypeCheck tp))
{
  ProgramCounter retPC;
  if (pc) {
    CodeAreaOzValueLocation *loc =
      new CodeAreaOzValueLocation(pc, code DebugArg(tp));
    b->getOzValue(putOzValueCA, loc);
    retPC = CodeArea::allocateWord(pc);
  } else {
    b->discardOzValue();
    retPC = 0;
  }
  return (retPC);
}

//
ProgramCounter unmarshalBuiltin(Builder *b, ProgramCounter pc)
{
  if (pc) {
    CodeAreaLocation *loc = new CodeAreaLocation(pc);
    b->getOzValue(putBuiltinCA, loc);
    return (CodeArea::allocateWord(pc));
  } else {
    b->discardOzValue();
    return ((ProgramCounter) 0);
  }
}


#ifdef USE_FAST_UNMARSHALER   

//
ProgramCounter unmarshalGRegRef(ProgramCounter PC, MarshalerBuffer *bs)
{ 
  int nGRegs = unmarshalNumber(bs);
  AssRegArray *gregs = PC ? AssRegArray::allocate(nGRegs) : 0;

  for (int i = 0; i < nGRegs; i++) {
    unsigned int reg = unmarshalNumber(bs);
    if (PC) {
      (*gregs)[i].set(reg>>2,reg&3);
    }
  }

  return (writeAddress(gregs, PC));
}

//
ProgramCounter unmarshalLocation(ProgramCounter PC, MarshalerBuffer *bs)
{ 
  int inAr = unmarshalNumber(bs);
  int outAr = unmarshalNumber(bs);
  OZ_Location::initLocation();

  for (int i = 0; i < inAr+outAr; i++) {
    int n = unmarshalNumber(bs);
    OZ_Location::set(i,n);
  }

  return (writeAddress(OZ_Location::getLocation(inAr+outAr), PC));
}

//
// 'unmarshalRecordArity' exists in two flavors: one fills a cell in a
// code area with an 'SRecordArity' value (resp. discards it in "skip"
// mode), while another one just fills up a given cell;
ProgramCounter unmarshalRecordArity(Builder *b,
				    ProgramCounter pc, MarshalerBuffer *bs) 
{
  RecordArityType at = unmarshalRecordArityType(bs);
  if (pc) {
    if (at == RECORDARITY) {
      CodeAreaLocation *loc = new CodeAreaLocation(pc);
      b->getOzValue(putRealRecordArityCA, loc);
      return (CodeArea::allocateWord(pc));
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumber(bs);
      return (CodeArea::writeInt(mkTupleWidth(width), pc));
    }
  } else {
    if (at == RECORDARITY)
      b->discardOzValue();
    else
      skipNumber(bs);
    return ((ProgramCounter) 0);
  }
}

//
// (of course, this code must resemble 'marshalPredId()')
ProgramCounter unmarshalPredId(Builder *b, ProgramCounter pc,
			       ProgramCounter instPC, MarshalerBuffer *bs) 
{
  if (pc) {
    Assert(sizeOf(DEFINITION) == 6);
    PredIdLocation *loc = new PredIdLocation(pc, instPC + 6);

    //
    b->getOzValue(getPredIdNameCA, loc);
    //
    RecordArityType at = unmarshalRecordArityType(bs);
    if (at == RECORDARITY) {
      b->getOzValue(saveRecordArityPredIdCA, loc);
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumber(bs);
      // set 'SRecordArity' directly (and there will be no arity
      // list);
      loc->setSRA(mkTupleWidth(width));
    }
    //
    b->getOzValue(saveFileCA, loc);
    //
    loc->setLine(unmarshalNumber(bs));
    loc->setColumn(unmarshalNumber(bs));
    //
    b->getOzValue(saveFlagsListCA, loc);
    //
    loc->setMaxX(unmarshalNumber(bs));
    loc->setGSize(unmarshalNumber(bs));

    //
    return (CodeArea::allocateWord(pc));

  } else {
    //
    b->discardOzValue();	// name;
    //
    RecordArityType at = unmarshalRecordArityType(bs);
    if (at == RECORDARITY) {
      b->discardOzValue();	// arity list;
    } else {
      Assert(at == TUPLEWIDTH);
      skipNumber(bs);
    }
    //
    b->discardOzValue();	// file;
    //
    skipNumber(bs);		// line & column;
    skipNumber(bs);
    //
    b->discardOzValue();	// flags list;
    //
    skipNumber(bs);		// maxX;
    skipNumber(bs);		// GSize;

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalCallMethodInfo(Builder *b,
				       ProgramCounter pc, MarshalerBuffer *bs) 
{
  int compact = unmarshalNumber(bs);

  //
  if (pc) {
    CallMethodInfoLocation *loc = new CallMethodInfoLocation(pc, compact);

    //
    b->getOzValue(getCallMethodInfoNameCA, loc);
    //
    RecordArityType at = unmarshalRecordArityType(bs);
    if (at == RECORDARITY) {
      b->getOzValue(saveCallMethodInfoRecordArityCA, loc);
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumber(bs);
      loc->setSRA(mkTupleWidth(width));
    }

    //
    return (CodeArea::allocateWord(pc));
  } else {
    b->discardOzValue();	// name;
    //
    RecordArityType at = unmarshalRecordArityType(bs);
    if (at == RECORDARITY)
      b->discardOzValue();
    else
      skipNumber(bs);

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalHashTableRef(Builder *b, ProgramCounter pc,
				     MarshalerBuffer *bs)
{
  //
  if (pc) {
    (void) unmarshalNumber(bs); // Backward compatibility;
    int elseLabel = unmarshalNumber(bs); /* the else label */
    int listLabel = unmarshalNumber(bs);
    int nEntries = unmarshalNumber(bs);
    IHashTable *table;

    //
    table = IHashTable::allocate(nEntries, elseLabel);
    if (listLabel)
      table->addLTuple(listLabel);

    //
    for (int i = nEntries; i--; ) {    
      int termTag = unmarshalNumber(bs);
      int label   = unmarshalNumber(bs);
      HashTableEntryDesc *desc = new HashTableEntryDesc(table, label);

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->getOzValue(getHashTableRecordEntryLabelCA, desc);
	  //
	  RecordArityType at = unmarshalRecordArityType(bs);
	  if (at == RECORDARITY) {
	    b->getOzValue(saveRecordArityHashTableEntryCA, desc);
	  } else {
	    Assert(at == TUPLEWIDTH);
	    int width = unmarshalNumber(bs);
	    desc->setSRA(mkTupleWidth(width));
	  }
	  break;
	}

      case ATOMTAG:
	b->getOzValue(getHashTableAtomEntryLabelCA, desc);
	break;

      case NUMBERTAG:
	b->getOzValue(getHashTableNumEntryLabelCA, desc);
	break;

      default: Assert(0); break;
      }
    }

    // 
    // The hash table is stored already, albeit it is not yet filled
    // up;
    return (CodeArea::writeIHashTable(table, pc));
  } else {
    skipNumber(bs);		// size
    skipNumber(bs);		// elseLabel
    skipNumber(bs);		// listLabel
    int nEntries = unmarshalNumber(bs);

    //
    for (int i = nEntries; i--; ) {
      int termTag = unmarshalNumber(bs);
      skipNumber(bs);		// label

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->discardOzValue();
	  //
	  RecordArityType at = unmarshalRecordArityType(bs);
	  if (at == RECORDARITY)
	    b->discardOzValue();
	  else
	    skipNumber(bs);
	  break;
	}

      case ATOMTAG:
	b->discardOzValue();
	break;

      case NUMBERTAG:
	b->discardOzValue();
	break;

      default: Assert(0); break;
      }
    }

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalProcedureRef(Builder *b, ProgramCounter pc,
				     MarshalerBuffer *bs, CodeArea *code)
{
  AbstractionEntry *entry = 0;
  Bool copyable = unmarshalNumber(bs);
  if (copyable) {
    MarshalTag tag = (MarshalTag) bs->get();
    if (tag == DIF_REF) {
      int i = unmarshalNumber(bs);
      entry = (AbstractionEntry*) ToPointer(b->get(i));
    } else {
      Assert(tag == DIF_ABSTRENTRY);
      int refTag = unmarshalRefTag(bs);
      entry = new AbstractionEntry(OK);
      b->setNoGC(ToInt32(entry), refTag);
    }
  }
  return (pc ? code->writeAbstractionEntry(entry,pc) : (ProgramCounter) pc);
}

#else 

//
ProgramCounter unmarshalGRegRefRobust(ProgramCounter PC,
				      MarshalerBuffer *bs, int *error)
{ 
  int nGRegs = unmarshalNumberRobust(bs, error);
  if(*error) return ((ProgramCounter) 0);
  AssRegArray *gregs = PC ? AssRegArray::allocate(nGRegs) : 0;

  for (int i = 0; i < nGRegs; i++) {
    unsigned int reg = unmarshalNumberRobust(bs, error);
    if(*error) return ((ProgramCounter) 0);
    if (PC) {
      (*gregs)[i].set(reg>>2,(reg&3));
    }
  }

  return (writeAddress(gregs, PC));
}

//
ProgramCounter unmarshalLocationRobust(ProgramCounter PC, MarshalerBuffer *bs,
				     int *error) 
{
  int inAr = unmarshalNumberRobust(bs, error);
  if(*error) return ((ProgramCounter) 0);
  int outAr = unmarshalNumberRobust(bs, error);
  if(*error) return ((ProgramCounter) 0);

  OZ_Location::initLocation();

  for (int i = 0; i < inAr+outAr; i++) {
    int n = unmarshalNumberRobust(bs, error);
    if(*error) return ((ProgramCounter) 0);
    OZ_Location::set(i,n);
  }
  return (writeAddress(OZ_Location::getLocation(inAr+outAr), PC));
}

//
// 'unmarshalRecordArity' exists in two flavors: one fills a cell in a
// code area with an 'SRecordArity' value (resp. discards it in "skip"
// mode), while another one just fills up a given cell;
ProgramCounter unmarshalRecordArityRobust(Builder *b, ProgramCounter pc, 
					  MarshalerBuffer *bs, int *error) 
{
  RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
  if(*error) return ((ProgramCounter) 0);
  if (pc) {
    if (at == RECORDARITY) {
      CodeAreaLocation *loc = new CodeAreaLocation(pc);
      b->getOzValue(putRealRecordArityCA, loc);
      return (CodeArea::allocateWord(pc));
    } else {
      int width = unmarshalNumberRobust(bs, error);
      if(*error || (at != TUPLEWIDTH)) return ((ProgramCounter) 0);
      return (CodeArea::writeInt(mkTupleWidth(width), pc));
    }
  } else {
    if (at == RECORDARITY)
      b->discardOzValue();
    else
      skipNumber(bs);
    return ((ProgramCounter) 0);
  }
}

//
// (of course, this code must resemble 'marshalPredId()')
ProgramCounter unmarshalPredIdRobust(Builder *b, ProgramCounter pc,
				     ProgramCounter instPC, MarshalerBuffer *bs, 
				     int *error) 
{
  if (pc) {
    Assert(sizeOf(DEFINITION) == 6);
    PredIdLocation *loc = new PredIdLocation(pc, instPC + 6);

    //
    b->getOzValue(getPredIdNameCA, loc);
    //
    RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
    if(*error) return ((ProgramCounter) 0);
    if (at == RECORDARITY) {
      b->getOzValue(saveRecordArityPredIdCA, loc);
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumberRobust(bs, error);
      if(*error) return ((ProgramCounter) 0);
      // set 'SRecordArity' directly (and there will be no arity
      // list);
      loc->setSRA(mkTupleWidth(width));
    }
    //
    b->getOzValue(saveFileCA, loc);
    //
    loc->setLine(unmarshalNumberRobust(bs, error));
    if(*error) return ((ProgramCounter) 0);
    loc->setColumn(unmarshalNumberRobust(bs, error));
    if(*error) return ((ProgramCounter) 0);
    //
    b->getOzValue(saveFlagsListCA, loc);
    //
    loc->setMaxX(unmarshalNumberRobust(bs, error));
    if(*error) return ((ProgramCounter) 0);
    //
    loc->setGSize(unmarshalNumberRobust(bs, error));
    if(*error) return ((ProgramCounter) 0);

    //
    return (CodeArea::allocateWord(pc));

  } else {
    //
    b->discardOzValue();	// name;
    //
    RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
    if(*error) return ((ProgramCounter) 0);
    if (at == RECORDARITY) {
      b->discardOzValue();	// arity list;
    } else {
      Assert(at == TUPLEWIDTH);
      skipNumber(bs);
    }
    //
    b->discardOzValue();	// file;
    //
    skipNumber(bs);		// line & column;
    skipNumber(bs);
    //
    b->discardOzValue();	// flags list;
    //
    skipNumber(bs);		// maxX;
    skipNumber(bs);		// GSize;

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalCallMethodInfoRobust(Builder *b, ProgramCounter pc, 
					     MarshalerBuffer *bs, int *error) 
{
  int compact = unmarshalNumberRobust(bs, error);
  if(*error) return ((ProgramCounter) 0);

  //
  if (pc) {
    CallMethodInfoLocation *loc = new CallMethodInfoLocation(pc, compact);

    //
    b->getOzValue(getCallMethodInfoNameCA, loc);
    //
    RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
    if(*error) return ((ProgramCounter) 0);
    if (at == RECORDARITY) {
      b->getOzValue(saveCallMethodInfoRecordArityCA, loc);
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumberRobust(bs, error);
      if(*error) return ((ProgramCounter) 0);
      loc->setSRA(mkTupleWidth(width));
    }

    //
    return (CodeArea::allocateWord(pc));
  } else {
    b->discardOzValue();	// name;
    //
    RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
    if(*error) return ((ProgramCounter) 0);
    if (at == RECORDARITY)
      b->discardOzValue();
    else
      skipNumber(bs);

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalHashTableRefRobust(Builder *b, ProgramCounter pc,
					   MarshalerBuffer *bs, int *error)
{
  //
  if (pc) {
    (void) unmarshalNumberRobust(bs, error); // Backward compatibility;
    if (*error) return ((ProgramCounter) 0);
    int elseLabel = unmarshalNumberRobust(bs, error); /* the else label */
    if (*error) return ((ProgramCounter) 0);
    int listLabel = unmarshalNumberRobust(bs, error);
    if (*error) return ((ProgramCounter) 0);
    int nEntries = unmarshalNumberRobust(bs, error);
    if (*error) return ((ProgramCounter) 0);
    IHashTable *table;

    //
    table = IHashTable::allocate(nEntries, elseLabel);
    if (listLabel)
      table->addLTuple(listLabel);

    //
    for (int i = 0; i < nEntries; i++) {    
      int termTag = unmarshalNumberRobust(bs, error);
      if(*error) return ((ProgramCounter) 0);
      int label = unmarshalNumberRobust(bs, error);
      if(*error) return ((ProgramCounter) 0);
      HashTableEntryDesc *desc = new HashTableEntryDesc(table, label);

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->getOzValue(getHashTableRecordEntryLabelCA, desc);
	  //
	  RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
	  if(*error) return ((ProgramCounter) 0);
	  if (at == RECORDARITY) {
	    b->getOzValue(saveRecordArityHashTableEntryCA, desc);
	  } else {
	    Assert(at == TUPLEWIDTH);
	    int width = unmarshalNumberRobust(bs, error);
	    if(*error) return ((ProgramCounter) 0);
	    desc->setSRA(mkTupleWidth(width));
	  }
	  break;
	}

      case ATOMTAG:
	b->getOzValue(getHashTableAtomEntryLabelCA, desc);
	break;

      case NUMBERTAG:
	b->getOzValue(getHashTableNumEntryLabelCA, desc);
	break;

      default: *error = OK; break;
      }
    }

    // 
    // The hash table is stored already, albeit it is not yet filled
    // up;
    return (CodeArea::writeIHashTable(table, pc));
  } else {
    skipNumber(bs);		// size
    skipNumber(bs);		// elseLabel
    skipNumber(bs);		// listLabel
    int nEntries = unmarshalNumberRobust(bs, error);
    if(*error) return ((ProgramCounter) 0);

    //
    for (int i = 0; i < nEntries; i++) {
      int termTag = unmarshalNumberRobust(bs, error);
      if(*error) return ((ProgramCounter) 0);
      skipNumber(bs);		// label

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->discardOzValue();
	  //
	  RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
	  if(*error) return ((ProgramCounter) 0);
	  if (at == RECORDARITY)
	    b->discardOzValue();
	  else
	    skipNumber(bs);
	  break;
	}

      case ATOMTAG:
	b->discardOzValue();
	break;

      case NUMBERTAG:
	b->discardOzValue();
	break;

      default: *error = OK; break;
      }
    }

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalProcedureRefRobust(Builder *b, ProgramCounter pc,
					   MarshalerBuffer *bs, CodeArea *code, 
					   int *error)
{
  AbstractionEntry *entry = 0;
  Bool copyable = unmarshalNumberRobust(bs, error);
  if(*error) return ((ProgramCounter) 0);
  if (copyable) {
    MarshalTag tag = (MarshalTag) bs->get();
    if (tag == DIF_REF) {
      int i = unmarshalNumberRobust(bs, error);
      if(*error) return ((ProgramCounter) 0);
      entry = (AbstractionEntry*) ToPointer(b->get(i));
    } else {
      Assert(tag == DIF_ABSTRENTRY);
      int refTag = unmarshalRefTagRobust(bs, b, error);
      if(*error || (tag != DIF_ABSTRENTRY)) return ((ProgramCounter) 0);
      entry = new AbstractionEntry(OK);
      b->setNoGC(ToInt32(entry),refTag);
    }
  }
  return ((pc && !(*error)) ? code->writeAbstractionEntry(entry,pc)
	  : (ProgramCounter) pc);
}

#endif // defined(USE_FAST_UNMARSHALER)

#endif // !defined(TEXT2PICKLE)
