/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

  ------------------------------------------------------------------------
*/

#ifndef __CODE_AREAH
#define __CODE_AREAH

#include "opcodes.hh"
#include "term.hh"
#include "indexing.hh"
#include "hashtbl.hh"


class CodeArea;


/**********************************************************************
 *                                                                    *
 *    class AbstractionTable: represented as hash table               *
 *                                                                    *
 **********************************************************************/


class AbstractionEntry {
private:
  Abstraction *abstr;
  ProgramCounter pc;
  RefsArray g;
public:
  IHashTable *indexTable;

  AbstractionEntry() : abstr(NULL), indexTable(NULL), g(NULL), pc(NULL) {}

  Abstraction *getAbstr() { return abstr; };
  RefsArray getGRegs()    { return g; };
  ProgramCounter getPC()  { return pc; };
  void setPred(Abstraction *abs);

  void gc();
};


class AbstractionTable: public HashTable {
public:
  AbstractionEntry *add(int id);
  AbstractionTable(int s) : HashTable(INTTYPE,s) {};

  void gc();
};



/*****************************************************************************/


class CodeAreaList {
  CodeArea *elem;
  CodeAreaList *next;
 public:
  CodeAreaList(CodeArea *e,CodeAreaList *n) : elem(e), next(n) {};
};

#if defined THREADED && THREADED > 0
  typedef void* AdressOpcode;
#else // THREADED == 0
  typedef Opcode AdressOpcode;
#endif //THREADED


class CodeArea {
  friend class AM;
  friend class TMapping;
  friend void engine();
  friend class Statistics;
  static HashTable atomTab;
  static HashTable nameTab;
  static AbstractionTable abstractionTab;
  friend Atom *addToAtomTab(char *str);
  friend Atom *addToNameTab(char *str);
  friend AbstractionEntry *addAbstractionTab(int id);
  friend void printAtomTab();
  friend void printNameTab();

 protected:
  ByteCode *codeBlock;    /* a block of abstract machine code */
  int size;               /* size of thie block */
  ProgramCounter wPtr;       /* write pointer for the code block */

  static CodeAreaList *allBlocks;

public:
  static int totalSize; /* total size of code allocated in bytes */

  /* read from file and return start in "pc" */
  CodeArea(FILE *fd, int size, ProgramCounter &pc);

  /* with one argument it means that we need the code till the "query"  */
  static void display (ProgramCounter from, int size = 1, FILE* = stderr);

  static ProgramCounter definitionStart(ProgramCounter from);

  void showAtomNames();


  /* load statements from "codeFile" until "ENDOFFILE", acknowledge if ok*/
  static Bool load(FILE *fd, ProgramCounter &newPC);

  static unsigned int getShort(ProgramCounter PC) { return (*PC); }
  static unsigned int getWord(ProgramCounter PC)  { return getShort(PC);}

#if defined THREADED && THREADED == 2
  static void **globalInstrTable;
#endif

  static AdressOpcode getOP(ProgramCounter PC)
  { return (AdressOpcode) getShort(PC); }
  static Opcode adressToOpcode(AdressOpcode);
  static AdressOpcode opcodeToAdress(Opcode);

  static char **opToString;
  static Opcode stringToOp(char *s);

  static void gc();

#ifdef RECINSTRFETCH
  static void writeInstr(void);
#endif

private:
// data
#ifdef RECINSTRFETCH
  static int fetchedInstr;
  static ProgramCounter ops[RECINSTRFETCH];
  static void recordInstr(ProgramCounter PC);
#endif

// functions
  static int scanChar (FILE *fd);
  static char *scanString (FILE *fd);
  static int scanUInt (FILE *fd);
  void scanVariablename (FILE *fd);
  void scanLiteral(FILE *fd);
  TaggedRef scanAtom(FILE *fd);
  void scanRegister (FILE *fd, int &regAdd);
  void scanRegisterIndex (FILE *fd);
  void scanArity (FILE *fd);
  void scanNumber (FILE *fd);
  void scanPosint (FILE *fd);
  void scanPredicateRef(FILE *fd);
  void scanLabel (FILE *fd, ProgramCounter start);
  void scanRecordArity (FILE *fd);
  TaggedRef scanRecordArity1 (FILE *fd, int length);
  void scanBuiltinname(FILE *fd);
  BuiltinTabEntry *scanFun(FILE *fd);

  static ProgramCounter writeWord(ByteCode c, ProgramCounter ptr)
  {
    *ptr = c;
    return ptr+1;
  }

  static ProgramCounter writeShort(ByteCode c, ProgramCounter ptr)
  {
    return writeWord(c,ptr);
  }

public:
  static ProgramCounter writeIHashTable(IHashTable *ht, ProgramCounter ptr)
  {

    *(IHashTable*) ptr = *ht;

    Assert(SizeofIHashTable == sizeof(IHashTable) / sizeof(ptr));

    return ptr + SizeofIHashTable;
  }

  static ProgramCounter writeAtom(TaggedRef atom, ProgramCounter ptr)
  {
    Assert(isLiteral(atom));
    return writeWord(atom,ptr);
  }

  static ProgramCounter writeInt(TaggedRef i, ProgramCounter ptr)
  {
    Assert(isNumber(i));
    return writeWord(i,ptr);
  }

  static ProgramCounter writeLabel(int label, ProgramCounter start, ProgramCounter ptr)
  {
    //  label==0 means fail in switchOnTerm and createCond
    //  in this case do not add start
    return writeWord((ByteCode)(label == 0 ? NOCODE : start+label),ptr);
  }

  static ProgramCounter writeBuiltin(BuiltinTabEntry *bi, ProgramCounter ptr)
  {
    //  label==0 means fail in switchOnTerm and createCond
    //  in this case do not add start
    return writeWord((ByteCode)bi,ptr);
  }

  static ProgramCounter writeOpcode(Opcode oc, ProgramCounter ptr)
  {
    return writeWord((ByteCode)opcodeToAdress(oc),ptr);
  }

  static ProgramCounter writeRegIndex(int index, ProgramCounter ptr)
  {
#ifdef FASTREGACCESS
    index *= sizeof(TaggedRef);
#endif
    return writeShort((ByteCode)index,ptr);
  }

  static ProgramCounter writeArity(int ar, ProgramCounter ptr)
  {
    return writeShort((ByteCode)ar,ptr);
  }

  static ProgramCounter writeRecordArity(Arity *ar, ProgramCounter ptr)
  {
    return writeWord((ByteCode)ar,ptr);
  }

  static ProgramCounter writeInt(int i, ProgramCounter ptr)
  {
    return writeShort((ByteCode)i,ptr);
  }

  static ProgramCounter writePredicateRef(int i, ProgramCounter ptr)
  {
    AbstractionEntry *entry = addAbstractionTab(i);
    return writeWord((ByteCode) entry, ptr);
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

inline Reg getRegArg(ProgramCounter PC)
{
  return (Reg) CodeArea::getShort(PC);
}

inline int getPosIntArg(ProgramCounter PC)
{
  return (int) CodeArea::getShort(PC);
}

inline TaggedRef getNumberArg(ProgramCounter PC)
{
  return (TaggedRef) CodeArea::getWord(PC);
}

inline  TaggedRef getAtomArg(ProgramCounter PC)
{
  return (TaggedRef) CodeArea::getWord(PC);
}

inline PrTabEntry *getPredArg(ProgramCounter PC)
{
  return(PrTabEntry *) CodeArea::getWord(PC);
}

inline void *getAdressArg(ProgramCounter PC)
{
  return (void *) CodeArea::getWord(PC);
}

inline ProgramCounter getLabelArg(ProgramCounter PC)
{
  return (ProgramCounter) CodeArea::getWord(PC);
}


#endif
