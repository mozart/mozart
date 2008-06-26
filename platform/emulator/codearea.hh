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
#include "am.hh"

#define AE_COPYABLE  1
#define AE_COLLECTED 2
#define AE_MASK      3

class AbstractionEntry {
private:
  TaggedRef      abstr;
  ProgramCounter pc;
  // ProgramCounter listpc;
  Tagged2        next_flags;

  static AbstractionEntry * allEntries;

public:

  AbstractionEntry * getNext(void) {
    return (AbstractionEntry *) next_flags.getPtr();
  }
  void setNext(AbstractionEntry * n) {
    next_flags.setPtr(n);
  }
  int isCopyable(void) {
    return next_flags.getTag() & AE_COPYABLE;
  }
  int isCollected(void) {
    return next_flags.getTag() & AE_COLLECTED;
  }
  void setCollected(void) {
    next_flags.borTag(AE_COLLECTED);
  }
  void unsetCollected(void) {
    next_flags.bandTag(~AE_COLLECTED);
  }
  AbstractionEntry(Bool fc) {
    abstr      = makeTaggedNULL();
    pc         = NOCODE;
    // listpc     = NOCODE;
    next_flags.set(allEntries,fc);
    allEntries = this;
  }

  Abstraction * getAbstr(void) {
    return abstr ? (Abstraction *) tagged2Const(abstr) : (Abstraction *) NULL;
  };

  ProgramCounter getPC(void)  {
    return pc;
  };
  //    ProgramCounter getListPC(void)  {
  //      return listpc;
  //    };

  void setPred(Abstraction * ab);

  void gCollectAbstractionEntry(void);

  static void freeUnusedEntries();
};

/*****************************************************************************/

#define getWord(PC) (*(PC))


#ifdef THREADED
  typedef uint32 AdressOpcode;
#else
  typedef Opcode AdressOpcode;
#endif


class CodeArea {
  friend class AM;
  friend class TMapping;
  friend int engine(Bool init);
  friend class Statistics;
  static StringHashTable atomTab;
  static StringHashTable nameTab;
  friend TaggedRef OZ_atom(OZ_CONST char *str);
  friend TaggedRef oz_atomNoDup(OZ_CONST char *str);
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
  Bool referenced;        /* for GC */
  static CodeArea * skipInGC;
public:

#define CheckWPtr Assert(wPtr < codeBlock+size)

  static CodeArea *allBlocks;

  void allocateBlock(int sz);

  static void init(void **instrtab);

public:
  Bool isReferenced(void) {
    return referenced;
  }
  ByteCode *getStart() { return codeBlock; }
  static int getTotalSize(void);

  CodeArea(int sz);
  ~CodeArea();

  void checkPtr(void *ptr) {
      Assert(getStart()<=ptr && ptr<getStart()+size);
  }

  void gCollectInstructions(void);

  static ProgramCounter printDef(ProgramCounter PC,FILE *out=stderr);
  static TaggedRef dbgGetDef(ProgramCounter PC, ProgramCounter definitionPC,
                             int frameId, RefsArray * Y, Abstraction *G);
  static TaggedRef getFrameVariables(ProgramCounter, RefsArray*, Abstraction *);
  static void getDefinitionArgs(ProgramCounter PC, XReg &reg, int &next,
                                TaggedRef &file, int &line, int &colum,
                                TaggedRef &predName);

  /* with one argument it means that we need the code till the "query"  */
  static void display (ProgramCounter from, int size = 1, FILE* = stderr,
                       ProgramCounter to=NOCODE);

private:
  // Returns the length of live X registers (max live + 1)
  static int livenessXInternal(ProgramCounter from, int xMax, int *xUsage);
  static void livenessGYInternal(ProgramCounter from, int yMax, int *yUsage,
                                                      int gMax, int *gUsage,
                                                      int *yLiveLength, int *gLiveLength);
public:
  static int livenessX(ProgramCounter from, TaggedRef *, int);
  static int livenessX(ProgramCounter from, RefsArray * ra) {
    return livenessX(from,ra->getArgsRef(),ra->getLen());
  }

  static void livenessGY(ProgramCounter from, Frame *aFrame,
                         int yMax, RefsArray *Y,
                         int gMax, int *gUsage);

  static ProgramCounter definitionStart(ProgramCounter from);
  static ProgramCounter definitionEnd(ProgramCounter from);

#ifdef THREADED
  static void **globalInstrTable;
#ifndef INLINEOPCODEMAP
  static AddressHashTable *opcodeTable;
#endif
#endif


  static AdressOpcode opcodeToAdress(Opcode oc) {
#ifdef THREADED
    return ToInt32(globalInstrTable[oc]);
#else
    return oc;
#endif
  }
  static Opcode adressToOpcode(AdressOpcode adr) {
#ifdef THREADED
#ifdef INLINEOPCODEMAP
    return Opcode (*((int32 *) (((char *) adr) - (1<<OPCODEALIGN))));
#else
    void * ret = opcodeTable->htFind((void *) adr);
    Assert(ret != htEmpty);
    return (Opcode) ToInt32(ret);
#endif
#else
    return adr;
#endif
  }
  static AdressOpcode getOP(ProgramCounter PC) {
    return (AdressOpcode) getWord(PC);
  }
  static Opcode getOpcode(ProgramCounter PC) {
    return adressToOpcode(getOP(PC));
  }


  void gCollectCodeBlock(void);
  static void gCollectCodeAreaStart(void);
  static void gCollectCollectCodeBlocks(void);

  TaggedRef disassemble(void);

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

  // kost@ : TODO : with disappearance of the old marshaler, this must
  // become a procedure returning nothing;
  ProgramCounter writeTagged(TaggedRef t, ProgramCounter ptr) {
    Assert(getStart()<=ptr && ptr < getStart()+size);
    return writeWord(t,ptr);
  }

  static CodeArea *findBlock(ProgramCounter PC);



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

  static ProgramCounter writeXRegIndex(int index, ProgramCounter ptr)
  {
#ifdef FASTREGACCESS
#ifdef FASTERREGACCESS
    return writeWord((ByteCode) &(XREGS[index]),ptr);
#else
    index *= sizeof(TaggedRef);
    return writeWord((ByteCode)index,ptr);
#endif
#else
#ifdef CHECKREGACCESS
    return writeWord((ByteCode) ((index << 2) | 1),ptr);
#else
    return writeWord((ByteCode)index,ptr);
#endif
#endif
  }

  static ProgramCounter writeYRegIndex(int index, ProgramCounter ptr)
  {
#ifdef FASTREGACCESS
    index = index*sizeof(TaggedRef) + sizeof(int);
#else
#ifdef CHECKREGACCESS
    index = ((index << 2) | 2);
#endif
#endif
    return writeWord((ByteCode)index,ptr);
  }

  static ProgramCounter writeGRegIndex(int index, ProgramCounter ptr)
  {
#ifdef FASTREGACCESS
    index *= sizeof(TaggedRef);
#else
#ifdef CHECKREGACCESS
    index = ((index << 2) | 3);
#endif
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


  static ProgramCounter writeAddress(void *p, ProgramCounter ptr)
  {
    return writeWord(p, ptr);
  }

  ProgramCounter writeAbstractionEntry(AbstractionEntry *p, ProgramCounter ptr) {
    ProgramCounter ret = writeAddress(p,ptr);
    checkPtr(ptr);
    return ret;
  }

  void writeAbstractionEntry(AbstractionEntry *p) {
    wPtr = writeAbstractionEntry(p,wPtr);
  }


  static void writeWordAllocated(ByteCode c, ProgramCounter ptr) {
    *ptr = c;
  }
  static void writeWordAllocated(void *p, ProgramCounter ptr) {
    writeWordAllocated((ByteCode)ToInt32(p),ptr);
  }

  static void writeAddressAllocated(void *p, ProgramCounter ptr) {
    writeWordAllocated(p, ptr);
  }

  void writeCache() { wPtr = writeCache(wPtr); }
  ProgramCounter writeCache(ProgramCounter PC);

  void writeInt(int i)                   { CheckWPtr; wPtr=writeInt(i,wPtr); }
  void writeTagged(TaggedRef t)          { CheckWPtr; wPtr=writeTagged(t,wPtr); }
  void writeBuiltin(Builtin *bi)         { CheckWPtr; wPtr=writeBuiltin(bi,wPtr); }
  void writeOpcode(Opcode oc)            { CheckWPtr; curInstr=wPtr; wPtr=writeOpcode(oc,wPtr); }
  void writeSRecordArity(SRecordArity ar){ CheckWPtr; wPtr=writeSRecordArity(ar,wPtr); }
  void writeAddress(void *ptr)           { CheckWPtr; wPtr=writeWord(ptr,wPtr); }
  void writeXReg(int i)                   { CheckWPtr; wPtr=writeXRegIndex(i,wPtr); }
  void writeYReg(int i)                   { CheckWPtr; wPtr=writeYRegIndex(i,wPtr); }
  void writeGReg(int i)                   { CheckWPtr; wPtr=writeGRegIndex(i,wPtr); }
  void writeLabel(int lbl) { CheckWPtr; wPtr=writeLabel(lbl,curInstr-codeBlock,wPtr); }
  int computeLabel(int lbl) { return lbl-(curInstr-codeBlock); }
  void writeDebugInfo(TaggedRef file, int line) {
    CheckWPtr; allDbgInfos = new DbgInfo(wPtr,file,line,allDbgInfos);
  }
  static void writeDebugInfo(ProgramCounter PC, TaggedRef file, int line) {
    allDbgInfos = new DbgInfo(PC,file,line,allDbgInfos);
  }
  ProgramCounter getWritePtr(void)       { return wPtr; }

  //
  // kost@ : support for non-recursive (new) unmarshaler...
  // The builder expects the word to be word-aligned, so that it can
  // treat it as an s-pointer.
  static ProgramCounter allocateWord(ProgramCounter ptr) {
    Assert(((unsigned int) ptr) / sizeof(int) * sizeof(int) == (unsigned int) ptr);
    return (ptr+1);
  }
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

#ifdef CHECKREGACCESS
inline
XReg getXRegArg(ProgramCounter PC) {
  int w = getWord(PC);
  Assert((w & 3) == 1);
  return w >> 2;
}
inline
YReg getYRegArg(ProgramCounter PC) {
  int w = getWord(PC);
  Assert((w & 3) == 2);
  return w >> 2;
}
inline
GReg getGRegArg(ProgramCounter PC) {
  int w = getWord(PC);
  Assert((w & 3) == 3);
  return w >> 2;
}
#else
#define getXRegArg(PC)    ((XReg) getWord(PC))
#define getYRegArg(PC)    ((YReg) getWord(PC))
#define getGRegArg(PC)    ((GReg) getWord(PC))
#endif

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

  Abstraction *lookup(OzClass *c, TaggedRef meth,
                      SRecordArity arity)
  {
    if (ToInt32(c) != key) {
      Bool defaultsUsed = NO;
      Abstraction *ret = c->getMethod(meth,arity,OK,defaultsUsed);
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

class CallMethodInfo {
public:
  int regIndex;
  Bool isTailCall;
  TaggedRef mn;
  SRecordArity arity;

  CallMethodInfo(int ri, TaggedRef name, Bool ist, SRecordArity ar)
  {
    regIndex   = ri;
    isTailCall = ist;
    arity = ar;
    mn = name;
  }

  ~CallMethodInfo() {}
  void dispose()      { delete this; }
};

class OZ_LocList {
public:
  OZ_Location * loc;
  OZ_LocList  * next;
  OZ_LocList(OZ_Location * l,OZ_LocList *n) : loc(l), next(n) {}
};

class OZ_Location {
private:
  int          fingerprint;
  TaggedRef * map[1];
  static TaggedRef  * new_map[];
  static OZ_LocList * cache[];

public:
  NO_DEFAULT_CONSTRUCTORS(OZ_Location)
  static void initCache(void);
  /*
   * Construction of locations
   */
  static void initLocation(void) {
  }
  static OZ_Location * alloc(int n) {
    int sz = sizeof(OZ_Location)+sizeof(TaggedRef*)*(n-1);
    return (OZ_Location *) malloc(sz);
  }
  void deallocate(void) {
    if (fingerprint == -1) {
      free(this);
    }
  }

  static void set(int n, int i) {
    new_map[n]=&(XREGS[i]);
  }
  static int getNewIndex(int n) {
    return (new_map[n]-XREGS);
  }
  static OZ_Location * getLocation(int n);

  TaggedRef ** getMapping(void) {
    return map;
  }
  int getIndex(int n) {
    return (map[n]-XREGS);
  }
  int getInIndex(int n) {
    return getIndex(n);
  }
  int getOutIndex(Builtin * bi, int n) {
    return getIndex(bi->getInArity() + n);
  }
  TaggedRef getValue(int n) {
    return *map[n];
  }
  TaggedRef getInValue(int n) {
    return getValue(n);
  }
  TaggedRef getOutValue(Builtin * bi, int n) {
    return getValue(bi->getInArity()+n);
  }
  TaggedRef getInArgs(Builtin *);
  TaggedRef getArgs(Builtin *);
};

extern OZ_Location * OZ_ID_LOC;

#ifdef FASTREGACCESS

#ifdef FASTERREGACCESS

#define XRegToInt(N) (((TaggedRef*) (N)) - XREGS)
#define XRegToPtr(N) ((TaggedRef *) (N))

#else

#define XRegToInt(N) ((N) / sizeof(TaggedRef))
#define XRegToPtr(N) ((TaggedRef *) (((intlong) XREGS) + (N)))

#endif

#define YRegToInt(N) ((N-sizeof(int)) / sizeof(TaggedRef))
#define YRegToPtr(Y,N) Y->getFastArgRef(N)

#define GRegToInt(N) ((N) / sizeof(TaggedRef))
#define GRegToPtr(G,N) ((TaggedRef *) (((intlong) (G)) + (N)))

#else

inline
int  XRegToInt(XReg N) { return N; }
inline
int  YRegToInt(YReg N) { return N; }
inline
int  GRegToInt(GReg N) { return N; }
inline
TaggedRef * XRegToPtr(XReg N) {
  return &(XREGS[N]);
}
inline
TaggedRef * YRegToPtr(RefsArray * Y, YReg N) {
  return Y->getArgRef(N);
}
inline
TaggedRef * GRegToPtr(TaggedRef * G, GReg N) {
  return G+N;
}

#endif


#endif
