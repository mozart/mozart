/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __CODE_AREAH
#define __CODE_AREAH

#include <time.h>
#include "base.hh"
#include "hashtbl.hh"
#include "opcodes.hh"
#include "value.hh"

class AbstractionEntry {
private:
  Abstraction *abstr;
  ProgramCounter pc;
  int arity;
  AbstractionEntry *next;
  Bool collected;

  static AbstractionEntry *allEntries;

public:

  Bool copyable;  // true iff may be copied with definitionCopy
  IHashTable *indexTable;

  AbstractionEntry(Bool fc) {
    abstr      = 0;
    pc         = NOCODE;
    copyable   = fc;
    collected  = NO;
    indexTable = 0;
    next       = allEntries;
    allEntries = this;
  }
  Abstraction *getAbstr() { return abstr; };
  ProgramCounter getPC()  { return pc; };
  int getArity()          { return arity; };
  void setPred(Abstraction *abs);

  void gcAbstractionEntry();
  static void freeUnusedEntries();
};


/*****************************************************************************/

#define getWord(PC) (*(PC))


#ifdef THREADED
  typedef uint32 AdressOpcode;
#else
  typedef Opcode AdressOpcode;
#endif


class TaggedList {
public:
  TaggedRef *t;
  TaggedList *next;

  TaggedList(TaggedRef *tptr, TaggedList *nxt): t(tptr), next(nxt) {}
  TaggedList *dispose() {
    TaggedList *ret = next;
    delete this;
    return ret;
  }
};

/**************************************************
 *  Invalidating of inline caches
 **************************************************/

typedef enum {C_TAGGED, C_INLINECACHE, C_ABSTRENTRY, C_FREE} GCListTag;

const int codeGCListBlockSize = 10;

class GCListEntry {
public:
  GCListTag tag;
  ProgramCounter pc;
};

class CodeGCList {
  CodeGCList *next;
  int nextFree;
  GCListEntry block[codeGCListBlockSize];

public:
  CodeGCList(CodeGCList *nxt) { nextFree=0; next=nxt; }

  void dispose() {
    CodeGCList *aux = this;
    while(aux) {
      CodeGCList *aux1 = aux->next;
      delete aux;;
      aux = aux1;
    }
  }


  CodeGCList *add(ProgramCounter ptr, GCListTag tag)
  {
    if (this==NULL || nextFree >= codeGCListBlockSize) {
      CodeGCList *aux = new CodeGCList(this);
      return aux->add(ptr,tag);
    }

    block[nextFree].tag = tag;
    block[nextFree].pc  = ptr;
    nextFree++;
    return this;
  }

  CodeGCList *addInlineCache(ProgramCounter ptr) { return add(ptr,C_INLINECACHE); }
  CodeGCList *addTagged(ProgramCounter ptr) { return add(ptr,C_TAGGED); }
  CodeGCList *addAbstractionEntry(ProgramCounter ptr) { return add(ptr,C_ABSTRENTRY); }

  void remove(TaggedRef *t);

  void collectGClist();
};


class CodeArea {
  friend class AM;
  friend class TMapping;
  friend int engine(Bool init);
  friend class Statistics;
  static HashTable atomTab;
  static HashTable nameTab;
  friend TaggedRef OZ_atom(const char *str);
  friend TaggedRef oz_uniqueName(const char *str);
  friend inline void printAtomTab();
  friend inline void printNameTab();

  static Bool getNextDebugInfoArgs(ProgramCounter from,
                                   TaggedRef &file, int &line, int &colum,
                                   TaggedRef &comment);

protected:
  ByteCode *codeBlock;    /* a block of abstract machine code */
  int size;               /* size of this block */
  CodeArea *nextBlock;
  ProgramCounter wPtr;    /* write pointer for the code block */
  ProgramCounter curInstr;/* start of current instruction */
  time_t timeStamp;       /* feed time */
  Bool referenced;        /* for GC */
  CodeGCList *gclist;

#define CheckWPtr Assert(wPtr < codeBlock+size)

  static CodeArea *allBlocks;

  void allocateBlock(int sz);
  static void init(void **instrtab);

public:
  static time_t findTimeStamp(ProgramCounter PC)
  {

    CodeArea *aux = allBlocks;
    while (aux) {
      if (aux->codeBlock<=PC && PC<aux->codeBlock+aux->size)
        return aux->timeStamp;
      aux = aux->nextBlock;
    }
    return (time_t) 0;
  }

  ByteCode *getStart() { return codeBlock; }
  static int totalSize; /* total size of code allocated in bytes */

  CodeArea(int sz);
  ~CodeArea();

  void checkPtr(void *ptr) {
      Assert(getStart()<=ptr && ptr<getStart()+size);
  }

  void protectInlineCache(InlineCache *cache) {
    gclist = gclist->addInlineCache((ProgramCounter)cache);
  }

  static ProgramCounter printDef(ProgramCounter PC,FILE *out=stderr);
  static TaggedRef dbgGetDef(ProgramCounter PC, ProgramCounter definitionPC,
                             int frameId, RefsArray Y, Abstraction *G);
  static TaggedRef getFrameVariables(ProgramCounter, RefsArray, Abstraction *);
  static void getDefinitionArgs(ProgramCounter PC, Reg &reg, int &next,
                                TaggedRef &file, int &line, int &colum,
                                TaggedRef &predName);

  /* with one argument it means that we need the code till the "query"  */
  static void display (ProgramCounter from, int size = 1, FILE* = stderr,
                       ProgramCounter to=NOCODE);

private:
  static int livenessXInternal(ProgramCounter from, TaggedRef *X,int n);
public:
  static int livenessX(ProgramCounter from, TaggedRef *X=0,int n=0);

  static ProgramCounter definitionStart(ProgramCounter from);
  static ProgramCounter definitionEnd(ProgramCounter from);

#ifdef THREADED
  static void **globalInstrTable;
  static HashTable *opcodeTable;
#endif

  static AdressOpcode getOP(ProgramCounter PC) {
    return (AdressOpcode) getWord(PC); }
  static Opcode adressToOpcode(AdressOpcode);
  static AdressOpcode opcodeToAdress(Opcode);
  static Opcode getOpcode(ProgramCounter PC) {
    return adressToOpcode(getOP(PC)); }

  void gcCodeBlock();
  static void gcCodeAreaStart();
  static void gcCollectCodeBlocks();

#ifdef RECINSTRFETCH
  static void writeInstr(void);
#else
  static void writeInstr(void) {};
#endif

private:
// data
#ifdef RECINSTRFETCH
  static int fetchedInstr;
  static ProgramCounter ops[RECINSTRFETCH];
  static void recordInstr(ProgramCounter PC);
#endif


  static ProgramCounter writeWord(ByteCode c, ProgramCounter ptr)
  {
    *ptr = c;
    return ptr+1;
  }

  static ProgramCounter writeWord(void *p, ProgramCounter ptr)
  {
    return writeWord((ByteCode)ToInt32(p),ptr);
  }

public:
  static ProgramCounter writeIHashTable(IHashTable *ht, ProgramCounter ptr)
  {
    return writeWord(ht,ptr);
  }

  static ProgramCounter writeLiteral(TaggedRef literal, ProgramCounter ptr)
  {
    Assert(oz_isLiteral(literal));
    return writeWord(literal,ptr);
  }

  ProgramCounter writeTagged(TaggedRef t, ProgramCounter ptr);

  static CodeArea *findBlock(ProgramCounter PC);

  void unprotect(TaggedRef* t);

  static ProgramCounter writeInt(TaggedRef i, ProgramCounter ptr)
  {
    Assert(oz_isNumber(i));
    return writeWord(i,ptr);
  }

  static ProgramCounter writeLabel(int label, int offset, ProgramCounter ptr)
  {
    return writeWord(ToPointer(label-offset), ptr);
  }

  static ProgramCounter writeBuiltin(Builtin *bi, ProgramCounter ptr)
  {
    return writeWord(bi,ptr);
  }

  static ProgramCounter writeOpcode(Opcode oc, ProgramCounter ptr)
  {
    return writeWord(opcodeToAdress(oc),ptr);
  }

  static ProgramCounter writeRegIndex(int index, ProgramCounter ptr)
  {
#ifdef FASTREGACCESS
    index *= sizeof(TaggedRef);
#endif
    return writeWord((ByteCode)index,ptr);
  }

  static ProgramCounter writeSRecordArity(SRecordArity ar, ProgramCounter ptr)
  {
    return writeWord((ByteCode)ar,ptr);
  }
  static ProgramCounter writeArity(int ar, ProgramCounter ptr)
  {
    return writeWord((ByteCode)ar,ptr);
  }

  static ProgramCounter writeRecordArity(Arity *ar, ProgramCounter ptr)
  {
    return writeWord(ar,ptr);
  }

  static ProgramCounter writeInt(int i, ProgramCounter ptr)
  {
    return writeWord((ByteCode)i,ptr);
  }


  ProgramCounter writeAbstractionEntry(AbstractionEntry *p, ProgramCounter ptr);
  void writeAbstractionEntry(AbstractionEntry *p)
  {
    wPtr = writeAbstractionEntry(p,wPtr);
  }


  static ProgramCounter writeAddress(void *p, ProgramCounter ptr)
  {
    return writeWord(p, ptr);
  }

  void writeCache() { wPtr = writeCache(wPtr); }
  ProgramCounter writeCache(ProgramCounter PC);

  void writeInt(int i)                   { CheckWPtr; wPtr=writeInt(i,wPtr); }
  void writeTagged(TaggedRef t)          { CheckWPtr; wPtr=writeTagged(t,wPtr); }
  void writeBuiltin(Builtin *bi)         { CheckWPtr; wPtr=writeBuiltin(bi,wPtr); }
  void writeOpcode(Opcode oc)            { CheckWPtr; curInstr=wPtr; wPtr=writeOpcode(oc,wPtr); }
  void writeSRecordArity(SRecordArity ar){ CheckWPtr; wPtr=writeSRecordArity(ar,wPtr); }
  void writeAddress(void *ptr)           { CheckWPtr; wPtr=writeWord(ptr,wPtr); }
  void writeReg(int i)                   { CheckWPtr; wPtr=writeRegIndex(i,wPtr); }
  void writeLabel(int lbl) { CheckWPtr; wPtr=writeLabel(lbl,curInstr-codeBlock,wPtr); }
  int computeLabel(int lbl) { return lbl-(curInstr-codeBlock); }
  void writeDebugInfo(TaggedRef file, int line) {
    CheckWPtr; allDbgInfos = new DbgInfo(wPtr,file,line,allDbgInfos);
  }
  static void writeDebugInfo(ProgramCounter PC, TaggedRef file, int line) {
    allDbgInfos = new DbgInfo(PC,file,line,allDbgInfos);
  }
  ProgramCounter getWritePtr(void)       { return wPtr; }
};



inline void printAtomTab()
{
  CodeArea::atomTab.print();
}


inline void printNameTab()
{
  CodeArea::nameTab.print();
}


/*
 * the following are not members of CodeArea: they are used
 * within the emulator, so we save prefixing them every time
 * with "CodeArea::"
 */

#define getRegArg(PC)    ((Reg) getWord(PC))
#define getPosIntArg(PC) ((int) getWord(PC))
#define getTaggedArg(PC) ((TaggedRef) getWord(PC))
#define getNumberArg(PC)  getTaggedArg(PC)
#define getLiteralArg(PC) getTaggedArg(PC)
#define getAdressArg(PC)  (ToPointer(getWord(PC)))
#define getPredArg(PC)   ((PrTabEntry *) getAdressArg(PC))
#define getLabelArg(PC)  ((int) getWord(PC))
#define GetLoc(PC)       ((OZ_Location*) getAdressArg(PC))
#define GetBI(PC)        ((Builtin*) getAdressArg(PC))

/*
 * Inline caching
 */

class InlineCache {
  uint32 key;
  int32 value;

public:
  InlineCache() { key = value = 0; }

  int lookup(SRecord *rec, TaggedRef feature)
  {
    if (key!=(uint32)rec->getSRecordArity()) {
      int32 aux = rec->getIndex(feature);
      if (aux==-1)
        return aux;
      value = aux;
      key   = rec->getSRecordArity();
    }
    return value;
  }

  Abstraction *lookup(ObjectClass *c, TaggedRef meth,
                      SRecordArity arity, RefsArray X)
  {
    if (ToInt32(c) != key) {
      Bool defaultsUsed = NO;
      Abstraction *ret = c->getMethod(meth,arity,X,defaultsUsed);
      if (!defaultsUsed && ret) {
        value = ToInt32(ret);
        key   = ToInt32(c);
      }
      return ret;
    }
    return (Abstraction*) ToPointer(value);
  }

  void invalidate() { key = value = 0; }
};

class GenCallInfoClass {
public:
  int regIndex;
  Bool isMethAppl, isTailCall;
  TaggedRef mn;
  SRecordArity arity;

  GenCallInfoClass(int ri, Bool ism, TaggedRef name, Bool ist, SRecordArity ar)
  {
    regIndex   = ri;
    isMethAppl = ism;
    isTailCall = ist;
    arity = ar;
    mn = name;
    OZ_protect(&mn);
  }

  ~GenCallInfoClass() { OZ_unprotect(&mn); }
  void dispose()      { delete this; }
};

class ApplMethInfoClass {
public:
  TaggedRef methName;
  SRecordArity arity;
  InlineCache methCache;

  ApplMethInfoClass(TaggedRef mn, SRecordArity i, CodeArea *code)
  {
    arity = i;
    methName = mn;
    oz_staticProtect(&methName);
    code->protectInlineCache(&methCache);
  }
};

class OZ_Location {
private:
  int inAr,outAr;
  int map[1];
public:
  NO_DEFAULT_CONSTRUCTORS(OZ_Location)
  static OZ_Location *newLocation(int inArity,int outArity)
  {
    int sz = sizeof(OZ_Location)+sizeof(int)*(inArity+outArity-1);
    OZ_Location *loc = (OZ_Location *)new char[sz];
    loc->inAr=inArity;
    loc->outAr=outArity;
    return loc;
  }
  int *mapping() { return map; }
  int get(int n) {
    Assert(n>=0 && n<inAr+outAr);
    return map[n];
  }
  void set(int n,int i) {
    Assert(n>=0 && n<inAr+outAr);
    map[n]=i;
  }
  int &out(int n) {
    Assert(n>=0 && n<outAr);
    return map[inAr+n];
  }
  int &in(int n) {
    Assert(n>=0 && n<inAr);
    return map[n];
  }
  int getArity() { return inAr+outAr; }
  int getInArity() { return inAr; }
  int getOutArity() { return outAr; }
  int max(int n) {
    for (int i = inAr+outAr-1; i >= inAr; i--) {
      if (get(i)>=n) {
        n=get(i)+1;
      }
    }
    return n;
  }
};


#ifdef FASTREGACCESS
inline Reg regToInt(Reg N) { return (N / sizeof(TaggedRef)); }
inline Reg intToReg(Reg N) { return N * sizeof(TaggedRef); }
#else
inline Reg regToInt(Reg N) { return N; }
inline Reg intToReg(Reg N) { return N; }
#endif


#endif
