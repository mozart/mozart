/*
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

//
// kost@: The whole thing is moved around now, so this file contains
// declarations of primitives for "standard" marshaling format of
// basic data types.  etc.  These are primitives from the point of
// view that they do not care whether the data structure should be
// marshaled at all, whether there is enough space in the buffer etc.

#ifndef __MARSHALERBASE_H
#define __MARSHALERBASE_H

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#if !defined(TEXT2PICKLE)
#include "mbuffer.hh"
#include "gentraverser.hh"
#endif

//
#if defined(TEXT2PICKLE)
#define EmulatorOnly(Code)
#else 
#define EmulatorOnly(Code) Code
#endif

//
// the DIFs
// the protocol layer needs to know about some of these 
typedef enum {
  DIF_UNUSED0,
  DIF_SMALLINT,         
  DIF_BIGINT,           
  DIF_FLOAT, 		
  DIF_ATOM,		
  DIF_NAME,
  DIF_UNIQUENAME,	
  DIF_RECORD,		
  DIF_TUPLE,
  DIF_LIST,
  DIF_REF, 
  DIF_REF_DEBUG, 
  DIF_OWNER, 
  DIF_OWNER_SEC,
  DIF_PORT,		
  DIF_CELL,             
  DIF_LOCK,             
  DIF_VAR,
  DIF_BUILTIN,
  DIF_DICT,
  DIF_OBJECT,	        // full object;
  DIF_THREAD_UNUSED,		
  DIF_SPACE,		
  DIF_CHUNK,            // SITE INDEX NAME value
  DIF_PROC,		// SITE INDEX NAME ARITY globals code
  DIF_CLASS,            // SITE INDEX NAME obj class
  DIF_ARRAY,
  DIF_FSETVALUE,	// finite set constant
  DIF_ABSTRENTRY,	// AbstractionEntry (code instantiation)
  DIF_PRIMARY,
  DIF_SECONDARY,
  DIF_SITE,
  DIF_SITE_VI,          // no longer in use (referred to VirtualInfo)
  DIF_SITE_PERM,
  DIF_PASSIVE,
  DIF_COPYABLENAME,
  DIF_EXTENSION,
  DIF_RESOURCE_T,
  DIF_RESOURCE_N,
  DIF_FUTURE,
  DIF_VAR_AUTO,
  DIF_FUTURE_AUTO,
  DIF_EOF,
  DIF_CODEAREA,
  DIF_VAR_OBJECT,		// var object exported;
  DIF_SYNC,
  DIF_CLONEDCELL,
  DIF_STUB_OBJECT,		// object exported (lazy objects);
  DIF_SUSPEND,
  DIF_LIT_CONT,
  DIF_EXT_CONT,
  DIF_LAST
} MarshalTag;

//
const struct {
  MarshalTag tag;
  char *name;
} dif_names[] = {
  { DIF_UNUSED0,      "UNUSED0"},
  { DIF_SMALLINT,     "SMALLINT"},
  { DIF_BIGINT,       "BIGINT"},
  { DIF_FLOAT,        "FLOAT"},
  { DIF_ATOM,         "ATOM"},
  { DIF_NAME,  	      "NAME"},
  { DIF_UNIQUENAME,   "UNIQUENAME"},
  { DIF_RECORD,       "RECORD"},
  { DIF_TUPLE,        "TUPLE"},
  { DIF_LIST,         "LIST"},
  { DIF_REF,          "REF"},
  { DIF_REF_DEBUG,    "REF_DEBUG"},
  { DIF_OWNER,        "OWNER"},
  { DIF_OWNER_SEC,    "OWNER_SEC"},
  { DIF_PORT,	      "PORT"},
  { DIF_CELL,         "CELL"},
  { DIF_LOCK,         "LOCK"},
  { DIF_VAR,          "VAR"},
  { DIF_BUILTIN,      "BUILTIN"},
  { DIF_DICT,         "DICT"},
  { DIF_OBJECT,       "OBJECT"},
  { DIF_THREAD_UNUSED,"THREAD"},
  { DIF_SPACE,	      "SPACE"},
  { DIF_CHUNK,        "CHUNK"},
  { DIF_PROC,	      "PROC"},
  { DIF_CLASS,        "CLASS"},
  { DIF_ARRAY,        "ARRAY"},
  { DIF_FSETVALUE,    "FSETVALUE"},
  { DIF_ABSTRENTRY,   "ABSTRENTRY"},
  { DIF_PRIMARY,      "PRIMARY"},
  { DIF_SECONDARY,    "SECONDARY"},
  { DIF_SITE,         "SITE"},
  { DIF_SITE_VI,      "SITE_VI"},
  { DIF_SITE_PERM,    "SITE_PERM"},
  { DIF_PASSIVE,      "PASSIVE"},
  { DIF_COPYABLENAME, "COPYABLENAME"},
  { DIF_EXTENSION,    "EXTENSION"},
  { DIF_RESOURCE_T,   "RESOURCE_T"},
  { DIF_RESOURCE_N,   "RESOURCE_N"},
  { DIF_FUTURE,       "FUTURE"},
  { DIF_VAR_AUTO,     "AUTOMATICALLY_REGISTERED_VAR"},
  { DIF_FUTURE_AUTO,  "AUTOMATICALLY_REGISTERED_FUTURE"},
  { DIF_EOF,          "EOF"},
  { DIF_CODEAREA,     "CODE_AREA_SEGMENT"},
  { DIF_VAR_OBJECT,   "VAR_OBJECT_EXPORTED"},
  { DIF_SYNC,         "SYNC"},
  { DIF_CLONEDCELL,   "CLONEDCELL"},
  { DIF_STUB_OBJECT,  "OBJECT_EXPORTED"},
  { DIF_SUSPEND,      "MARSHALING_SUSPENDED"},
  { DIF_LIT_CONT,     "DIF_LITERAL_CONTINUATION"},
  { DIF_EXT_CONT,     "DIF_EXTENSION_CONTINUATION"},
  { DIF_LAST,         "LAST"}
};

// the names of the difs for statistics 
enum {
  MISC_STRING,
  MISC_GNAME,
  MISC_SITE,
  //
  MISC_LAST
};

//
#define GT_MCodeAreaDesc	(GT_AE_GenericBase + 0)
#define GT_BCodeAreaDesc	(GT_AE_GenericBase + 1)
#define GT_CodeAreaLoc		(GT_AE_GenericBase + 2)
#define GT_CodeAreaOzValueLoc	(GT_AE_GenericBase + 3)
#define GT_CodeAreaPredIdLoc	(GT_AE_GenericBase + 4)
#define GT_CodeAreaMethInfoLoc	(GT_AE_GenericBase + 5)
#define GT_CodeAreaHTEntryDesc	(GT_AE_GenericBase + 6)

#if !defined(TEXT2PICKLE)

//
// statistics (e.g. for the "pane module");
class SendRecvCounter {
private:
  long c[2];
public:
  SendRecvCounter() { c[0]=0; c[1]=0; }
  void send() { c[0]++; }
  long getSend() { return c[0]; }
  void recv() { c[1]++; }
  long getRecv() { return c[1]; }
};

//
extern SendRecvCounter dif_counter[];
extern SendRecvCounter misc_counter[];

#endif

//
// common with the 'text2pickle' functions;
const unsigned int SBit = 1<<7;
const int shortSize = 2;

//
inline void marshalDIF(MarshalerBuffer *bs, MarshalTag tag) {
  EmulatorOnly(dif_counter[tag].send());
  bs->put(tag);
}

inline void marshalByte(MarshalerBuffer *bs, unsigned char c) {
  bs->put(c);
}

inline void marshalShort(MarshalerBuffer *bs, unsigned short i) {
  for (int k=0; k<shortSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;
  }
}

inline void marshalNumber(MarshalerBuffer *bs, unsigned int i) {
  while(i >= SBit) {
    bs->put((i%SBit)|SBit);
    i /= SBit;
  }
  bs->put(i);
}

inline void marshalString(MarshalerBuffer *bs, const char *s) {
  EmulatorOnly(misc_counter[MISC_STRING].send());
  marshalNumber(bs, strlen(s));
  while(*s) {
    bs->put(*s);
    s++;  
  }
}

inline void marshalLabel(MarshalerBuffer *bs, int start, int lbl) {
  // fprintf(stderr,"Label: %d\n",lbl);
  marshalNumber(bs, lbl);
}

inline void marshalOpCode(MarshalerBuffer *bs, int lbl, Opcode op,
			  Bool showLabel = 1) {	
  bs->put(op);
}

inline void marshalCodeStart(MarshalerBuffer *bs) {}

inline void marshalCodeEnd(MarshalerBuffer *bs) {}

inline void marshalTermDef(MarshalerBuffer *bs, int lbl) {
  marshalNumber(bs, lbl);
}

inline void marshalTermRef(MarshalerBuffer *bs, int lbl) {
  marshalNumber(bs, lbl);
}

#if !defined(TEXT2PICKLE)
//
// Globalization - needed for both pickling & distribution;
GName *globalizeConst(ConstTerm *t, MarshalerBuffer *bs);

//
void initRobustMarshaler();

//
// Constants needed for checking that no overflow occurs in unmarshalNumber()

// Biggest number dividable with 7 and less then sizeof(int)
extern unsigned int RobustMarshaler_Max_Shift;
// (sizeof(int)-RobustMarshaler_Max_Shift)^2
extern unsigned int RobustMarshaler_Max_Hi_Byte;

//
// '*MaxSize' constants are for the suspendable marshaler;
#define DIFMaxSize 1
#define OpcodeMaxSize 1
#define MNumberMaxSize 5
#define MShortMaxSize 2
#define MFloatMaxSize (MNumberMaxSize*2)
#define MBaseSiteForGNameMaxSize (3*MNumberMaxSize + MShortMaxSize)
#define MGNameMaxSize (MBaseSiteForGNameMaxSize + MNumberMaxSize*(fatIntDigits + 1))

//
void marshalFloat(MarshalerBuffer *bs, double d);
void marshalGName(MarshalerBuffer *bs, GName *gname);

//
//
// Dump 'num' characters; 'num' should not be larger than the real
// string length. It returns the remaining portion of the string if
// any, an zero otherwise. Observe that the marshaled string fragment
// looks exactly as a 'marshalString's string;
inline
void marshalStringNum(MarshalerBuffer *bs, const char *s, int num) {
  marshalNumber(bs, num);
  while (num--) {
    Assert(*s);
    bs->put(*s);
    s++;
  }
}

//
// The following procedures must be either macros, or be redefined for
// different MarshalerBuffer"s. Otherwise only the predefined
// 'marshalTermDef' etc. will be used;

#define rememberNode(gt, bs, node)				\
{								\
  int ind = gt->rememberTerm(node);				\
  marshalTermDef(bs, ind);					\
}
#define rememberVarNode(gt, bs, p)				\
{								\
  int ind = gt->rememberVarLocation(p);				\
  marshalTermDef(bs, ind);					\
}
#define rememberLocation(gt, bs, p)				\
{								\
  int ind = gt->rememberLocation(p);				\
  marshalTermDef(bs, ind);					\
}

//
void skipNumber(MarshalerBuffer *bs);

//
class DoubleConv {
public:
  union {
    unsigned char c[sizeof(double)];
    int i[sizeof(double)/sizeof(int)];
    double d;
  } u;
};

//
inline Bool isLowEndian() {
  DoubleConv dc;
  dc.u.i[0] = 1;
  return dc.u.c[0] == 1;
}
//
static const Bool lowendian = isLowEndian();

//
// Basic data structures;
// Primitive:
void marshalSmallInt(MarshalerBuffer *bs, OZ_Term siTerm);
void marshalFloat(MarshalerBuffer *bs, OZ_Term floatTerm);
void marshalLiteral(MarshalerBuffer *bs, OZ_Term litTerm, int litTermInd);
void marshalBigInt(MarshalerBuffer *bs, OZ_Term biTerm, ConstTerm *biConst);

//
// Marshaling/unmarshaling of record arity also requires some special
// machinery... The problem is that the surrounding context must know
// sometimes (aka when unmarshaling hash tables from code area) how
// many subtrees are there, and what to do with them;
enum RecordArityType {
  RECORDARITY,
  TUPLEWIDTH
};

//
// Code area;
//
void marshalBuiltin(GenTraverser *gt, Builtin *entry);
void marshalProcedureRef(GenTraverser *gt,
			 AbstractionEntry *entry, MarshalerBuffer *bs);
void marshalRecordArity(GenTraverser *gt,
			SRecordArity sra, MarshalerBuffer *bs);
void marshalPredId(GenTraverser *gt, PrTabEntry *p, MarshalerBuffer *bs);
void marshalCallMethodInfo(GenTraverser *gt,
			   CallMethodInfo *cmi, MarshalerBuffer *bs);
void marshalGRegRef(AssRegArray *gregs, MarshalerBuffer *bs);
void marshalLocation(Builtin *bi, OZ_Location *loc, MarshalerBuffer *bs);
void marshalHashTableRef(GenTraverser *gt,
			 int start, IHashTable *table, MarshalerBuffer *bs);
//
enum { ATOMTAG, NUMBERTAG, RECORDTAG};


//
// Scanning for certain data structures (ResourceExcavator etc.)
// needs just to traverse nodes without writing anything into the
// stream:
void traverseBuiltin(GenTraverser *gt, Builtin *entry);
void traversePredId(GenTraverser *gt, PrTabEntry *p);
void traverseRecordArity(GenTraverser *gt, SRecordArity sra);
void traverseHashTableRef(GenTraverser *gt, int start, IHashTable *table);
void traverseCallMethodInfo(GenTraverser *gt, CallMethodInfo *cmi);

//
// Unmarshaling;
//
inline
unsigned short unmarshalShort(MarshalerBuffer *bs) {
  unsigned short sh;
  unsigned int i1 = bs->get();
  unsigned int i2 = bs->get();
  sh= (i1 + (i2<<8));
  return (sh);
}

//
#ifdef USE_FAST_UNMARSHALER

//
unsigned int unmarshalNumber(MarshalerBuffer *bs);
double unmarshalFloat(MarshalerBuffer *bs);
char *unmarshalString(MarshalerBuffer *);

//
inline
int unmarshalRefTag(MarshalerBuffer *bs) {
  return unmarshalNumber(bs);
}

//
GName* unmarshalGName(TaggedRef*,MarshalerBuffer*);

#else

unsigned int unmarshalNumberRobust(MarshalerBuffer *bs, int *overflow);
double unmarshalFloatRobust(MarshalerBuffer *bs, int *overflow);
char *unmarshalStringRobust(MarshalerBuffer *, int *error);

//
inline
int unmarshalRefTagRobust(MarshalerBuffer *bs, Builder *builder, int *error) {
  int e;
  int rt = unmarshalNumberRobust(bs, &e);
  *error = e || !builder->checkNewIndex(rt); // RefTags are found in order?
  return rt;
}

//
GName* unmarshalGNameRobust(TaggedRef*,MarshalerBuffer*,int*);

#endif

//
// 'CodeAreaProcessor' argument: keeps the location of a code area
// being processed;
class MarshalerCodeAreaDescriptor : public GTAbstractEntity,
				    public CppObjMemory {
protected:
  ProgramCounter start, end, current;
public:
  MarshalerCodeAreaDescriptor(ProgramCounter startIn, ProgramCounter endIn)
    : start(startIn), end(endIn), current(startIn) {}
  virtual ~MarshalerCodeAreaDescriptor() {
    DebugCode(start = end = current = (ProgramCounter) -1;);
  }

  //
  virtual int getType() { return (GT_MCodeAreaDesc); }
  virtual void gc() {}

  //
  ProgramCounter getStart() { return (start); }
  ProgramCounter getEnd() { return (end); }
  ProgramCounter getCurrent() { return (current); }
  void setCurrent(ProgramCounter pc) { current = pc; }
};

//
class BuilderCodeAreaDescriptor : public GTAbstractEntity,
				  public CppObjMemory {
protected:
  ProgramCounter start, end, current;
  CodeArea *code;
public:
  BuilderCodeAreaDescriptor(ProgramCounter startIn, ProgramCounter endIn,
			    CodeArea *codeIn)
    : start(startIn), end(endIn), current(startIn), code(codeIn) {}
  virtual ~BuilderCodeAreaDescriptor() {
    DebugCode(start = end = current = (ProgramCounter) -1;);
    DebugCode(code = (CodeArea *) -1;);
  }

  //
  ProgramCounter getStart() { return (start); }
  ProgramCounter getEnd() { return (end); }
  ProgramCounter getCurrent() { return (current); }
  void setCurrent(ProgramCounter pc) { current = pc; }
  //
  CodeArea* getCodeArea() { return (code); }

  //
  virtual int getType() { return (GT_BCodeAreaDesc); }
  virtual void gc() {}
};

//
ProgramCounter writeAddress(void *ptr, ProgramCounter pc);
ProgramCounter unmarshalCache(ProgramCounter PC, CodeArea *code);

//
// Hash table entries are constructed using the table itself, label,
// and either an Oz value or an Oz value and SRecordArity. Thus, a
// descriptor of an entry used for the 'Builder::getOzValue()' task
// keeps table, label and may be a record arity list.
class HashTableEntryDesc : public GTAbstractEntity,
			   public CppObjMemory {
private:
  IHashTable *table;
  int label;
  SRecordArity sra;		// for "tuple" record entries only;
  OZ_Term arityList;		// for "proper" record entries only;
public:
  HashTableEntryDesc(IHashTable *tableIn, int labelIn)
    : table(tableIn), label(labelIn), sra((SRecordArity) 0)
  {
    DebugCode(arityList = (OZ_Term) -1;);
  }
  virtual ~HashTableEntryDesc() {}

  //
  IHashTable* getTable() { return (table); }
  int getLabel() { return (label); }
  SRecordArity getSRA() { return (sra); }
  void setSRA(SRecordArity sraIn) { sra = sraIn; }
  void setArityList(OZ_Term ra) { arityList = ra; }
  OZ_Term getArityList() { return (arityList); }

  //
  virtual int getType() { return (GT_CodeAreaHTEntryDesc); }
  virtual void gc() {}
};

//
void getHashTableRecordEntryLabelCA(GTAbstractEntity *arg, OZ_Term value);
void saveRecordArityHashTableEntryCA(GTAbstractEntity *arg, OZ_Term value);
void getHashTableAtomEntryLabelCA(GTAbstractEntity *arg, OZ_Term value);
void getHashTableNumEntryLabelCA(GTAbstractEntity *arg, OZ_Term value);

//
#ifdef USE_FAST_UNMARSHALER
static inline
RecordArityType unmarshalRecordArityType(MarshalerBuffer *bs)
{
  return ((RecordArityType) unmarshalNumber(bs));
}
#else
static inline
RecordArityType unmarshalRecordArityTypeRobust(MarshalerBuffer *bs, int *error)
{
  return ((RecordArityType) unmarshalNumberRobust(bs, error));
}
#endif

//
#ifdef DEBUG_CHECK
// Oz value type check procedures;

typedef Bool (*OzTermTypeCheck)(TaggedRef);

Bool mIsAny(TaggedRef t);
Bool mIsNumber(TaggedRef t);
Bool mIsLiteral(TaggedRef t);
Bool mIsFeature(TaggedRef t);
Bool mIsConstant(TaggedRef t);

#endif

//
ProgramCounter unmarshalOzValue(Builder *b, ProgramCounter pc,
				CodeArea *code DebugArg(OzTermTypeCheck tp));
ProgramCounter unmarshalBuiltin(Builder *b, ProgramCounter pc);

//
void handleDEBUGENTRY(void *arg);

#ifdef USE_FAST_UNMARSHALER

//
inline ProgramCounter unmarshalNum(ProgramCounter pc, MarshalerBuffer *bs) {
  int num = unmarshalNumber(bs);
  return (pc ? CodeArea::writeInt(num,pc) : (ProgramCounter) 0);
}
inline ProgramCounter unmarshalXReg(ProgramCounter pc, MarshalerBuffer *bs) {
  int idx = unmarshalNumber(bs);
  return (pc ? CodeArea::writeXRegIndex(idx, pc) : (ProgramCounter) 0);
}
inline ProgramCounter unmarshalYReg(ProgramCounter pc, MarshalerBuffer *bs) {
  int idx = unmarshalNumber(bs);
  return (pc ? CodeArea::writeYRegIndex(idx, pc) : (ProgramCounter) 0);
}
inline ProgramCounter unmarshalGReg(ProgramCounter pc, MarshalerBuffer *bs) {
  int idx = unmarshalNumber(bs);
  return (pc ? CodeArea::writeGRegIndex(idx, pc) : (ProgramCounter) 0);
}

//
inline ProgramCounter unmarshalLabel(ProgramCounter PC, MarshalerBuffer *bs) {
  int offset = unmarshalNumber(bs);
  return (PC ? CodeArea::writeLabel(offset,0,PC) : (ProgramCounter) 0);
}

//
ProgramCounter unmarshalGRegRef(ProgramCounter PC, MarshalerBuffer *bs);
ProgramCounter unmarshalLocation(ProgramCounter PC, MarshalerBuffer *bs);
ProgramCounter unmarshalRecordArity(Builder *b,
				    ProgramCounter pc, MarshalerBuffer *bs);
ProgramCounter unmarshalPredId(Builder *b, ProgramCounter pc,
			       ProgramCounter lastPC, MarshalerBuffer *bs);
ProgramCounter unmarshalCallMethodInfo(Builder *b,
				       ProgramCounter pc, MarshalerBuffer *bs);
ProgramCounter unmarshalHashTableRef(Builder *b, ProgramCounter pc,
				     MarshalerBuffer *bs);
ProgramCounter unmarshalProcedureRef(Builder *b, ProgramCounter pc,
				     MarshalerBuffer *bs, CodeArea *code);

#else

//
inline ProgramCounter unmarshalNumRobust(ProgramCounter pc,
					 MarshalerBuffer *bs, int *error) {
  int num = unmarshalNumberRobust(bs, error);
  return ((pc && !(*error)) ?
	  CodeArea::writeInt(num,pc) : (ProgramCounter) 0);
}

//
inline ProgramCounter unmarshalXRegRobust(ProgramCounter pc,
					  MarshalerBuffer *bs, int *error) {
  int idx = unmarshalNumberRobust(bs, error);
  return ((pc && !(*error)) ?
	  CodeArea::writeXRegIndex(idx, pc) : (ProgramCounter) 0);
}
inline ProgramCounter unmarshalYRegRobust(ProgramCounter pc,
					  MarshalerBuffer *bs, int *error) {
  int idx = unmarshalNumberRobust(bs, error);
  return ((pc && !(*error)) ?
	  CodeArea::writeYRegIndex(idx, pc) : (ProgramCounter) 0);
}
inline ProgramCounter unmarshalGRegRobust(ProgramCounter pc,
					  MarshalerBuffer *bs, int *error) {
  int idx = unmarshalNumberRobust(bs, error);
  return ((pc && !(*error)) ?
	  CodeArea::writeGRegIndex(idx, pc) : (ProgramCounter) 0);
}

//
inline ProgramCounter unmarshalLabelRobust(ProgramCounter pc,
					   MarshalerBuffer *bs, int *error) {
  int offset = unmarshalNumberRobust(bs, error);
  return ((pc && !(*error)) ?
	  CodeArea::writeLabel(offset,0,pc) : (ProgramCounter) 0);
}

//
ProgramCounter unmarshalGRegRefRobust(ProgramCounter PC,
				      MarshalerBuffer *bs, int *error);
ProgramCounter unmarshalLocationRobust(ProgramCounter PC, MarshalerBuffer *bs,
				       int *error);
ProgramCounter unmarshalPredIdRobust(Builder *b, ProgramCounter pc,
				     ProgramCounter lastPC, MarshalerBuffer *bs, 
				     int *error);
ProgramCounter unmarshalRecordArityRobust(Builder *b, ProgramCounter pc, 
					  MarshalerBuffer *bs, int *error);
ProgramCounter unmarshalCallMethodInfoRobust(Builder *b, ProgramCounter pc, 
					     MarshalerBuffer *bs, int *error);
ProgramCounter unmarshalHashTableRefRobust(Builder *b, ProgramCounter pc,
					   MarshalerBuffer *bs, int *error);
ProgramCounter unmarshalProcedureRefRobust(Builder *b, ProgramCounter pc,
					   MarshalerBuffer *bs, CodeArea *code, 
					   int *error);

// Return values from unmarshalCodeRobust etc.:
typedef enum {
  UCR_DONE = 0,
  UCR_SUSPEND,
  UCR_ERROR
} UCRReturn;
// 'U'nmarshal 'C'ode 'R'obust 'Return';

#endif // defined(USE_FAST_UNMARSHALER)

#endif // !defined(TEXT2PICKLE)

#endif
