/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */


#include "am.hh"
#include "indexing.hh"
#include "optostr.hh"

AbstractionEntry* AbstractionEntry::allEntries = NULL;

#ifdef RECINSTRFETCH

int CodeArea::fetchedInstr = 0;
ProgramCounter CodeArea::ops[RECINSTRFETCH];



void CodeArea::recordInstr(ProgramCounter PC){
  ops[fetchedInstr] = PC;
  fetchedInstr++;
  if(fetchedInstr >= RECINSTRFETCH)
    fetchedInstr = 0;
}

#endif

HashTable CodeArea::atomTab(HT_CHARKEY,10000);
HashTable CodeArea::nameTab(HT_CHARKEY,1000);
int CodeArea::totalSize = 0; /* in bytes */
char **CodeArea::opToString = initOpToString();
CodeArea *CodeArea::allBlocks = NULL;

#ifdef THREADED
void **CodeArea::globalInstrTable = 0;
HashTable *CodeArea::opcodeTable = 0;
#endif



Opcode CodeArea::stringToOp(const char *s)
{
  for (int i=0; i < (Opcode) OZERROR; i++) {
    if (strcmp(s,opToString[i]) == 0 ) {
      return (Opcode) i;
    }
  }

  return OZERROR;
}

inline Literal *addToLiteralTab(const char *str, HashTable *table, Bool isName)
{
  Literal *found = (Literal *) table->htFind(str);

  if (found != (Literal *) htEmpty) {
    return found;
  }

  str = ozstrdup(str);

  if (isName) {
    found = NamedName::newNamedName(str);
  } else {
    found = Atom::newAtom(str);
  }

  table->htAdd(str,found);
  return found;
}


Literal *addToAtomTab(const char *str)
{
  return addToLiteralTab(str,&CodeArea::atomTab,NO);
}

Literal *addToNameTab(const char *str)
{
  return addToLiteralTab(str,&CodeArea::nameTab,OK);
}


AbstractionTable CodeArea::abstractionTab(4000);

/*
  AbstractionEntry::defaultEntry: for bug fix. If you feed

  declare P1 P2 Px in

  proc {P1 X} true end
  proc {Px X} false end
  Px=P1
  proc {P2 X} true end

  this gives toplevel failure. Afterwards feed
   {P2 1}
  this SEGV, since FASTCALL points to an unfilled AbstractionEntry.
  Use defaultEntry for all newly created AbstractionEntry. Set by a builtin.

*/


AbstractionEntry *AbstractionTable::add(Abstraction *abstr)
{
  AbstractionEntry *ret = new AbstractionEntry(NO);
  ret->setPred(abstr);
  return ret;
}

AbstractionEntry *AbstractionTable::add(int id)
{
  if (id == 0)
    return NULL;

  AbstractionEntry *found =
    (AbstractionEntry *) CodeArea::abstractionTab.htFind(id);

  if (found != (AbstractionEntry *) htEmpty) {
    return found;
  }

  found = new AbstractionEntry(NO);
  CodeArea::abstractionTab.htAdd(id,found);
  return found;
}


/*
  we store the absolute adress of the indices in the
  instruction tables
  */

#ifdef THREADED
AdressOpcode CodeArea::opcodeToAdress(Opcode oc)
{
  return ToInt32(globalInstrTable[oc]);
}


Opcode CodeArea::adressToOpcode(AdressOpcode adr)
{
  void *ret = opcodeTable->htFind(adr);
  if (ret == htEmpty) return OZERROR;
  return (Opcode) ToInt32(ret);

  /*
    for(int i = 0; i < (int) OZERROR; i++)
      if (ToInt32(globalInstrTable[i]) == adr)
        return (Opcode)i;
    return OZERROR;
  */
}

#else /* THREADED */
AdressOpcode CodeArea::opcodeToAdress(Opcode oc)  { return  oc; }
Opcode CodeArea::adressToOpcode(AdressOpcode adr) { return adr; }
#endif /* THREADED */


void AbstractionEntry::setPred(Abstraction *ab)
{
  abstr = ab;
  pc    = abstr->getPC();
  g     = abstr->getGRegs();
  arity = abstr->getArity();

  // indexing on X[0] optimized !!!
  if (pc != NOCODE &&
      CodeArea::getOpcode(pc) == SWITCHONTERMX &&
      getRegArg(pc+1) == 0) {
    indexTable = (IHashTable *) getAdressArg(pc+2);
  } else {
    indexTable = NULL;
  }
}


#define DISPATCH() PC += sizeOf(op); break



const char *getBIName(ProgramCounter PC)
{
  BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC);
  return entry->getPrintName();
}


void CodeArea::printDef(ProgramCounter PC)
{
  ProgramCounter definitionPC = definitionStart(PC);
  if (definitionPC == NOCODE) {
    message("\tspecial task or on toplevel (PC=%s)\n",
            opToString[(int)getOpcode(PC)]);
    return;
  }

  Reg reg;
  int next;
  TaggedRef file, line, column, comment, predName;
  getDefinitionArgs(definitionPC,reg,next,file,line,column,predName);
  getNextDebugInfoArgs(PC,file,line,column,comment);

  const char *name = OZ_atomToC(predName);
  if (*name && column != makeTaggedNULL())
    message("\tprocedure '%s' in file \"%s\", line %d, column %d, PC=%ld\n",
            name,OZ_atomToC(file),OZ_intToC(line),OZ_intToC(column),(int)PC);
  else if (*name)
    message("\tprocedure '%s' in file \"%s\", line %d, PC=%ld\n",
            name,OZ_atomToC(file),OZ_intToC(line),(int)PC);
  else if (column != makeTaggedNULL())
    message("\tprocedure in file \"%s\", line %d, column %d, PC=%ld\n",
            OZ_atomToC(file),OZ_intToC(line),OZ_intToC(column),(int)PC);
  else
    message("\tprocedure in file \"%s\", line %d, PC=%ld\n",
            OZ_atomToC(file),OZ_intToC(line),(int)PC);
}

TaggedRef CodeArea::dbgGetDef(ProgramCounter PC, ProgramCounter definitionPC,
                              int frameId)
{
  Reg reg;
  int next;
  TaggedRef file, line, column, comment, predName;
  // file & line might be overwritten some lines later ...
  getDefinitionArgs(definitionPC,reg,next,file,line,column,predName);

  // if we are lucky there's some debuginfo and we can determine
  // the exact position inside the procedure application
  //--** problem: these are the coordinates of the corresponding exit
  //     instruction
  getNextDebugInfoArgs(PC,file,line,column,comment);

  TaggedRef pairlist = nil();
  if (column != makeTaggedNULL()) {
    pairlist = cons(OZ_pairA("column",column),pairlist);
  }
  int iline = smallIntValue(line);
  pairlist =
    cons(OZ_pairAI("time",findTimeStamp(PC)),
         cons(OZ_pairA("name",predName),
              cons(OZ_pairA("file",file),
                   cons(OZ_pairAI("line",iline < 0? -iline: iline),
                        cons(OZ_pairAI("PC",(int)PC),
                             cons(OZ_pairA("kind",AtomDebugCall),
                                  cons(OZ_pairA("origin",OZ_atom("dbgGetDef")),
                                       pairlist)))))));
  if (frameId != -1)
    pairlist = cons(OZ_pairAI("frameID",frameId),pairlist);

  return OZ_recordInit(OZ_atom("entry"), pairlist);
}

TaggedRef CodeArea::getFrameVariables(ProgramCounter PC,
                                      RefsArray Y, RefsArray G) {
  TaggedRef locals = nil();
  TaggedRef globals = nil();

  ProgramCounter aux = definitionEnd(PC);

  if (aux != NOCODE) {
    aux += sizeOf(getOpcode(aux));

    for (int i=0; getOpcode(aux) == LOCALVARNAME; i++) {
      if (Y) {
        TaggedRef aux1 = getLiteralArg(aux+1);
        if (!literalEq(aux1, AtomEmpty) && Y[i] != makeTaggedNULL()) {
          locals = cons(OZ_mkTupleC("#", 2, aux1, Y[i]), locals);
        }
      }
      aux += sizeOf(getOpcode(aux));
    }

    if (G) {
      for (int i=0; getOpcode(aux) == GLOBALVARNAME; i++) {
        TaggedRef aux1 = getLiteralArg(aux+1);
        if (!literalEq(aux1, AtomEmpty)) {
          globals = cons(OZ_mkTupleC("#", 2, aux1, G[i]), globals);
        }
        aux += sizeOf(getOpcode(aux));
      }
    }
  }

  TaggedRef pairlist =
    cons(OZ_pairA("Y", locals),
         cons(OZ_pairA("G", globals),
              nil()));

  TaggedRef ret = OZ_recordInit(OZ_atom("v"), pairlist);
  return ret;
}

ProgramCounter CodeArea::definitionStart(ProgramCounter from)
{
  ProgramCounter ret = definitionEnd(from);
  if (ret == NOCODE)
    return ret;
  else
    return ret+getLabelArg(ret+1);
}


Bool CodeArea::getNextDebugInfoArgs(ProgramCounter PC,
                                    TaggedRef &file, TaggedRef &line,
                                    TaggedRef &column, TaggedRef &comment)
{
  ProgramCounter end = definitionEnd(PC);
  if (end == NOCODE)
    return NO;

  while (PC < end) {
    Opcode op = getOpcode(PC);
    switch (op) {
    case DEBUGENTRY:
    case DEBUGEXIT:
      file    = getTaggedArg(PC+1);
      line    = getTaggedArg(PC+2);
      column  = getTaggedArg(PC+3);
      comment = getTaggedArg(PC+4);
      return OK;
    case DEFINITION:
    case DEFINITIONCOPY:
      PC += getLabelArg(PC+2);
      break;
    case ENDOFFILE:
    case OZERROR:
      return NO;
    default:
      DISPATCH();
    }
  }
  return NO;
}


/* find the end of the definition where from points into */
ProgramCounter CodeArea::definitionEnd(ProgramCounter PC)
{
  while (1) {
    Opcode op = getOpcode(PC);
    switch (op) {
    case DEFINITION:
    case DEFINITIONCOPY:
      PC += getLabelArg(PC+2);
      break;
    case ENDDEFINITION:
      return PC;
    case CREATENAMEDVARIABLEX:
    case CREATENAMEDVARIABLEY:
    case CREATENAMEDVARIABLEG:
    case TASKXCONT:
    case TASKCFUNCONT:
    case TASKDEBUGCONT:
    case TASKCALLCONT:
    case TASKLOCK:
    case TASKSETSELF:
    case TASKLTQ:
    case TASKCATCH:
    case TASKEMPTYSTACK:
    case TASKPROFILECALL:
    case TASKACTOR:
    case ENDOFFILE:
    case OZERROR:
    case GLOBALVARNAME:   // last instr in CodeArea::init
      return NOCODE;
    default:
      DISPATCH();
    }
  }
  return NOCODE;
}

void displayCode(ProgramCounter from, int ssize)
{
 CodeArea::display(from,ssize,stderr);
 fflush(stderr);
}

void displayDef(ProgramCounter from, int ssize)
{
  displayCode(CodeArea::definitionStart(from),ssize);
}


inline
ProgramCounter computeLabelArg(ProgramCounter pc, ProgramCounter arg)
{
  return pc+getLabelArg(arg);
}


void CodeArea::getDefinitionArgs(ProgramCounter PC,
                                 Reg &reg, int &next,
                                 TaggedRef &file, TaggedRef &line,
                                 TaggedRef &column, TaggedRef &predName)
{
  Assert(getOpcode(PC) == DEFINITION || getOpcode(PC) == DEFINITIONCOPY);
  PrTabEntry *pred = getPredArg(PC+3);

  reg      = regToInt(getRegArg(PC+1));
  next     = getLabelArg(PC+2);
  file     = pred != NULL? pred->getFileName() : OZ_atom("nofile");
  line     = OZ_int(pred != NULL? pred->getLine() : 0);
  column   = makeTaggedNULL();
  predName = OZ_atom(pred != NULL? pred->getPrintName() : "");
}


void CodeArea::display (ProgramCounter from, int sz, FILE* ofile)
{
  ProgramCounter PC = from;
  int defCount = 0; // counter for nested defintions
  for (int i = 1; i <= sz || sz <= 0 ; i++) {
    fprintf(ofile, "%p:\t", PC);
    Opcode op = getOpcode(PC);
    if (op == OZERROR || op == ENDOFFILE) {
      message("End of code block reached\n");
      return;
    }

    fprintf(ofile, "%03d\t%s", op, opToString[(int) op]);

    switch (op) {
    case FAILURE:
    case SKIP:
    case WAIT:
    case EMPTYCLAUSE:
    case WAITTOP:
    case ASK:
    case RETURN:
    case CLAUSE:
    case LASTCLAUSE:
    case DEALLOCATEL:
    case DEALLOCATEL1:
    case DEALLOCATEL2:
    case DEALLOCATEL3:
    case DEALLOCATEL4:
    case DEALLOCATEL5:
    case DEALLOCATEL6:
    case DEALLOCATEL7:
    case DEALLOCATEL8:
    case DEALLOCATEL9:
    case DEALLOCATEL10:
    case ALLOCATEL1:
    case ALLOCATEL2:
    case ALLOCATEL3:
    case ALLOCATEL4:
    case ALLOCATEL5:
    case ALLOCATEL6:
    case ALLOCATEL7:
    case ALLOCATEL8:
    case ALLOCATEL9:
    case ALLOCATEL10:
    case SHALLOWTHEN:
    case CREATEOR:
    case CREATEENUMOR:
    case CREATECHOICE:
    case PROFILEPROC:
    case POPEX:
    case TASKXCONT:
    case TASKCFUNCONT:
    case TASKDEBUGCONT:
    case TASKCALLCONT:
    case TASKLOCK:
    case TASKSETSELF:
    case TASKLTQ:
    case TASKCATCH:
    case TASKEMPTYSTACK:
    case TASKPROFILECALL:
    case TASKACTOR:
      fprintf(ofile, "\n");
      DISPATCH();

    case DEBUGENTRY:
    case DEBUGEXIT:
      {
        fprintf(ofile, "(%s ", toC(getTaggedArg(PC+1)));
        fprintf(ofile, "%s ", toC(getTaggedArg(PC+2)));
        fprintf(ofile, "%s ", toC(getTaggedArg(PC+3)));
        fprintf(ofile, "%s %d)\n", toC(getTaggedArg(PC+4)),
                getPosIntArg(PC+5));
        DISPATCH();
      }

    case PUTLISTX:
    case PUTLISTY:
    case PUTLISTG:
    case SETVALUEX:
    case SETVALUEY:
    case SETVALUEG:
    case GETLISTX:
    case GETLISTY:
    case GETLISTG:
    case UNIFYVALUEX:
    case UNIFYVALUEY:
    case UNIFYVALUEG:
    case GETVARIABLEX:
    case GETVARIABLEY:
    case GETVARIABLEG:
    case SETVARIABLEX:
    case SETVARIABLEY:
    case SETVARIABLEG:
    case UNIFYVARIABLEX:
    case UNIFYVARIABLEY:
    case UNIFYVARIABLEG:
    case CREATEVARIABLEX:
    case CREATEVARIABLEY:
    case CREATEVARIABLEG:
    case GETSELF:
    case CLEARY:
      fprintf(ofile, "(%d)\n", regToInt(getRegArg(PC+1)));
      DISPATCH();

    case CREATEVARIABLEMOVEX:
    case CREATEVARIABLEMOVEY:
    case CREATEVARIABLEMOVEG:
      fprintf (ofile, "(%d x(%d))\n",
               regToInt(getRegArg(PC+1)),
               regToInt(getRegArg(PC+2)));
      DISPATCH();

    case GETLISTVALVARX:
    case GETLISTVALVARY:
    case GETLISTVALVARG:
      fprintf (ofile, "(x(%d) %d x(%d))\n",
               regToInt(getRegArg(PC+1)),
               regToInt(getRegArg(PC+2)),
               regToInt(getRegArg(PC+3)));
      DISPATCH();

    case MOVEMOVEXYXY:
    case MOVEMOVEYXXY:
    case MOVEMOVEYXYX:
    case MOVEMOVEXYYX:
      fprintf (ofile, "(%d %d %d %d)\n",
              regToInt(getRegArg(PC+1)), regToInt(getRegArg(PC+2)),
              regToInt(getRegArg(PC+3)), regToInt(getRegArg(PC+4)));
      DISPATCH();

    case SHALLOWTEST1:
      fprintf (ofile,
               "(%s x(%d) %p %d)\n",
               getBIName(PC+1),
               regToInt(getRegArg(PC+2)),
               computeLabelArg(PC,PC+3),
               getPosIntArg(PC+4));
      DISPATCH();

    case TESTLITERALX:
    case TESTLITERALY:
    case TESTLITERALG:
    case TESTNUMBERX:
    case TESTNUMBERY:
    case TESTNUMBERG:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf (ofile,
                 "(%d %s %p %p %d)\n",
                 regToInt(getRegArg(PC+1)),
                 toC(tagged),
                 computeLabelArg(PC,PC+3),
                 computeLabelArg(PC,PC+4),
                 getPosIntArg(PC+5));
        DISPATCH();
      }

    case TESTBOOLX:
    case TESTBOOLY:
    case TESTBOOLG:
      {
        fprintf (ofile,
                 "(%d %p %p %p %d)\n",
                 regToInt(getRegArg(PC+1)),
                 computeLabelArg(PC,PC+2),
                 computeLabelArg(PC,PC+3),
                 computeLabelArg(PC,PC+4),
                 getPosIntArg(PC+5));
        DISPATCH();
      }

    case SHALLOWTEST2:
      fprintf (ofile,
               "(%s x(%d) x(%d) %p %d)\n",
               getBIName(PC+1),
               regToInt(getRegArg(PC+2)),
               regToInt(getRegArg(PC+3)),
               computeLabelArg(PC,PC+4),
               getPosIntArg(PC+5));
      DISPATCH();

    case TESTLESSEQ:
    case TESTLESS:
      fprintf (ofile,
               "(x(%d) x(%d) %p %d)\n",
               regToInt(getRegArg(PC+1)),
               regToInt(getRegArg(PC+2)),
               computeLabelArg(PC,PC+3),
               getPosIntArg(PC+4));
      DISPATCH();

    case INLINEREL1:
      fprintf (ofile,
               "(%s x(%d) %d)\n",
               getBIName(PC+1),
               regToInt(getRegArg(PC+2)),
               getPosIntArg(PC+3));
      DISPATCH();

    case INLINEFUN1:
    case INLINEREL2:
      fprintf (ofile,
               "(%s x(%d) x(%d) %d)\n",
               getBIName(PC+1),
               regToInt(getRegArg(PC+2)),
               regToInt(getRegArg(PC+3)),
               getPosIntArg(PC+4));
      DISPATCH();

    case INLINEPLUS1:
    case INLINEMINUS1:
      fprintf (ofile,
               "(x(%d) x(%d) %d)\n",
               regToInt(getRegArg(PC+1)),
               regToInt(getRegArg(PC+2)),
               getPosIntArg(PC+3));
      DISPATCH();

    case INLINEFUN2:
    case INLINEREL3:
    case INLINEEQEQ:
      fprintf (ofile,
               "(%s x(%d) x(%d) x(%d) %d)\n",
               getBIName(PC+1),
               regToInt(getRegArg(PC+2)),
               regToInt(getRegArg(PC+3)),
               regToInt(getRegArg(PC+4)),
               getPosIntArg(PC+5));
      DISPATCH();

    case INLINEPLUS:
    case INLINEMINUS:
    case INLINEUPARROW:
      {
        fprintf (ofile,
                 "(x(%d) x(%d) x(%d) %d)\n",
                 regToInt(getRegArg(PC+1)),
                 regToInt(getRegArg(PC+2)),
                 regToInt(getRegArg(PC+3)),
                 getPosIntArg(PC+4));
      }
      DISPATCH();

    case INLINEDOT:
      {
        TaggedRef literal = getLiteralArg(PC+2);
        fprintf (ofile,
                 "(x(%d) %s x(%d) %d)\n",
                 regToInt(getRegArg(PC+1)),
                 toC(literal),
                 regToInt(getRegArg(PC+3)),
                 getPosIntArg(PC+4));
      }
      DISPATCH();

    case INLINEAT:
    case INLINEASSIGN:
      {
        TaggedRef literal = getLiteralArg(PC+1);
        fprintf (ofile,
                 "(%s x(%d) %d)\n",
                 toC(literal),
                 regToInt(getRegArg(PC+2)),
                 getPosIntArg(PC+3));
      }
      DISPATCH();

    case INLINEFUN3:
      fprintf (ofile,
               "(%s x(%d) x(%d) x(%d) x(%d) %d)\n",
               getBIName(PC+1),
               regToInt(getRegArg(PC+2)),
               regToInt(getRegArg(PC+3)),
               regToInt(getRegArg(PC+4)),
               regToInt(getRegArg(PC+5)),
               getPosIntArg(PC+6));
      DISPATCH();

    case CALLBUILTIN:
      fprintf (ofile,
               "(%s %d)\n",
               getBIName(PC+1),
               getPosIntArg(PC+2));
      DISPATCH();

    case GENFASTCALL:
    case FASTCALL:
    case FASTTAILCALL:
      {
        AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
        int dummy = getPosIntArg(PC+2);
        fprintf(ofile,"(%p[pc:%p n:%d] %d)\n",entry,
                entry->getPC(),entry->getArity(),
                dummy);
        DISPATCH();
      }

    case APPLMETHX:
    case APPLMETHY:
    case APPLMETHG:
    case TAILAPPLMETHX:
    case TAILAPPLMETHY:
    case TAILAPPLMETHG:
      {
        ApplMethInfoClass *ami = (ApplMethInfoClass*) getAdressArg(PC+1);
        fprintf(ofile, "(ami(%s ", toC(ami->methName));
        SRecordArity sra = (SRecordArity) ami->arity;
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, ") %d)\n", regToInt(getRegArg(PC+2)));
        DISPATCH();
      }

    case SENDMSGX:
    case SENDMSGY:
    case SENDMSGG:
    case TAILSENDMSGX:
    case TAILSENDMSGY:
    case TAILSENDMSGG:
      {
        TaggedRef mn = getLiteralArg(PC+1);
        fprintf(ofile, "(%s %d ", toC(mn), regToInt(getRegArg(PC+2)));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " cache)\n");
        DISPATCH();
      }

    case CALLX:
    case CALLY:
    case CALLG:
    case TAILCALLX:
    case TAILCALLY:
    case TAILCALLG:
    case WEAKDETX:
    case WEAKDETY:
    case WEAKDETG:
      {
        Reg reg = regToInt(getRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, getPosIntArg(PC+2));
      }
      DISPATCH();

    case SETVOID:
    case GETVOID:
    case UNIFYVOID:
    case ALLOCATEL:
      fprintf(ofile, "(%d)\n", getPosIntArg(PC+1));
      DISPATCH();

    case SETNUMBER:
    case UNIFYNUMBER:
      {
        fprintf(ofile, "(%s)\n", toC(getNumberArg(PC+1)));
      }
      DISPATCH();

    case PUTNUMBERX:
    case PUTNUMBERY:
    case PUTNUMBERG:
    case GETNUMBERX:
    case GETNUMBERY:
    case GETNUMBERG:
      {
        fprintf(ofile, "(%s %d)\n", toC(getNumberArg(PC+1)),
                regToInt(getRegArg(PC+2)));
      }
      DISPATCH();

    case MOVEXX:
    case MOVEXY:
    case MOVEXG:
    case MOVEYX:
    case MOVEYY:
    case MOVEYG:
    case MOVEGX:
    case MOVEGY:
    case MOVEGG:

    case UNIFYXX:
    case UNIFYXY:
    case UNIFYXG:
    case UNIFYYX:
    case UNIFYYY:
    case UNIFYYG:
    case UNIFYGX:
    case UNIFYGY:
    case UNIFYGG:

    case GETVARVARXX:
    case GETVARVARXY:
    case GETVARVARXG:
    case GETVARVARYX:
    case GETVARVARYY:
    case GETVARVARYG:
    case GETVARVARGX:
    case GETVARVARGY:
    case GETVARVARGG:

    case UNIFYVALVARXX:
    case UNIFYVALVARXY:
    case UNIFYVALVARXG:
    case UNIFYVALVARYX:
    case UNIFYVALVARYY:
    case UNIFYVALVARYG:
    case UNIFYVALVARGX:
    case UNIFYVALVARGY:
    case UNIFYVALVARGG:
      {
        Reg reg = regToInt(getRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, regToInt(getRegArg(PC+2)));
      }
      DISPATCH();

    case SETCONSTANT:
    case UNIFYLITERAL:
    case SETLITERAL:
    case GLOBALVARNAME:
    case LOCALVARNAME:
      {
        TaggedRef tagged = getTaggedArg(PC+1);
        fprintf(ofile, "(%s)\n", toC(tagged));
      }
      DISPATCH();

    case DEFINITION:
    case DEFINITIONCOPY:
      {
        defCount++;

        Reg reg;
        int next;
        TaggedRef file, line, column, predName;
        getDefinitionArgs(PC,reg,next,file,line,column,predName);
        PrTabEntry *predd = getPredArg(PC+3);
        AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+4);
        AssRegArray *list = (AssRegArray*) getAdressArg(PC+5);
        fprintf(ofile,"(x(%d) %d pid(%s ",reg,next,toC(predName));
        fprintf(ofile,"_ %s ",toC(file));
        fprintf(ofile,"%s %d) %p ",toC(line),predd->copyOnce,predEntry);

        int size = list->getSize();
        if (size == 0)
          fprintf(ofile,"nil)\n");
        else {
          fprintf(ofile,"[");
          for (int k = 0; k < size; k++) {
            switch ((*list)[k].kind) {
            case XReg: fprintf(ofile,"x(%d)",(*list)[k].number); break;
            case YReg: fprintf(ofile,"y(%d)",(*list)[k].number); break;
            case GReg: fprintf(ofile,"g(%d)",(*list)[k].number); break;
            }
            if (k != size - 1)
              fprintf(ofile, " ");
          }
          fprintf(ofile, "])\n");
        }
      }
      DISPATCH();

    case PUTRECORDX:
    case PUTRECORDY:
    case PUTRECORDG:
    case GETRECORDX:
    case GETRECORDY:
    case GETRECORDG:
      {
        TaggedRef literal = getLiteralArg(PC+1);
        fprintf(ofile, "(%s ", toC(literal));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+2);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " %d)\n", regToInt(getRegArg(PC+3)));
      }
      DISPATCH();

    case PUTCONSTANTX:
    case PUTCONSTANTY:
    case PUTCONSTANTG:
    case GETLITERALX:
    case GETLITERALY:
    case GETLITERALG:
    case PUTLITERALX:
    case PUTLITERALY:
    case PUTLITERALG:
      {
        TaggedRef tagged = getTaggedArg(PC+1);

        fprintf(ofile, "(%s %d)\n", toC(tagged),
                 regToInt(getRegArg(PC+2)));
      }
      DISPATCH();

    case CREATENAMEDVARIABLEX:
    case CREATENAMEDVARIABLEY:
    case CREATENAMEDVARIABLEG:
      {
        TaggedRef literal = getLiteralArg(PC+2);
        fprintf(ofile, "(%s %d)\n", toC(literal),
                 regToInt(getRegArg(PC+1)));
      }
      DISPATCH();

    case ENDDEFINITION:
      fprintf(ofile, "(%p)\n", computeLabelArg(PC,PC+1));
      if (sz<=0 && defCount<=1) return;
      defCount--;
      DISPATCH();

    case BRANCH:
    case NEXTCLAUSE:
    case THREAD:
    case EXHANDLER:
      fprintf(ofile, "(%p)\n", computeLabelArg(PC,PC+1));
      DISPATCH();

    case THREADX:
      fprintf(ofile, "(%d %p)\n", getPosIntArg(PC+1), computeLabelArg(PC,PC+2));
      DISPATCH();

    case BRANCHONNONVARX:
    case BRANCHONNONVARY:
    case BRANCHONNONVARG:
      {
        Reg reg = regToInt(getRegArg(PC+1));
        fprintf(ofile, "(%d %p)\n", reg, computeLabelArg(PC,PC+2));
      }
      DISPATCH();

    case SWITCHONTERMX:
    case SWITCHONTERMY:
    case SWITCHONTERMG:
      {
        Reg reg = regToInt(getRegArg(PC+1));
        fprintf(ofile, "(%d ...)\n", reg);
      }
      DISPATCH();

    case SHALLOWGUARD:
    case CREATECOND:
      {
        ProgramCounter lbl = computeLabelArg(PC,PC+1);
        int n = getPosIntArg(PC+2);
        fprintf(ofile, "(%p %d)\n", lbl, n);
      }
      DISPATCH();

    case LOCKTHREAD:
      {
        ProgramCounter lbl = computeLabelArg(PC,PC+1);
        int n      = regToInt(getRegArg(PC+2));
        int toSave = getPosIntArg(PC+3);
        fprintf(ofile, "(%p x(%d) %d)\n", lbl, n, toSave);
      }
      DISPATCH();

    case GENCALL:
      {
        GenCallInfoClass *gci = (GenCallInfoClass*)getAdressArg(PC+1);
        fprintf(ofile, "(%p[ri:%d l:%s", gci,gci->regIndex, toC(gci->mn));
        fprintf(ofile, " a:%s] %d)\n", toC(sraGetArityList(gci->arity)),
                getPosIntArg(PC+2));
        DISPATCH();
      }

    case MARSHALLEDFASTCALL:
      {
        fprintf(ofile, "(%s %d)\n",toC(getTaggedArg(PC+1)),getPosIntArg(PC+2));
        DISPATCH();
      }

    default:
      fflush(ofile);
      warning("CodeArea::display: Illegal instruction");
      return;
    }
  }
}

#undef DISPATCH

ProgramCounter
  C_XCONT_Ptr,
  C_CFUNC_CONT_Ptr,
  C_DEBUG_CONT_Ptr,
  C_CALL_CONT_Ptr,
  C_LOCK_Ptr,
  C_COMMIT_Ptr,
  C_SET_SELF_Ptr,
  C_SET_ABSTR_Ptr,
  C_LTQ_Ptr,
  C_CATCH_Ptr,
  C_ACTOR_Ptr,
  C_EMPTY_STACK;



CodeArea::CodeArea(int sz)
{
  allocateBlock(sz);
}

void CodeArea::init(void **instrTable)
{
#ifdef THREADED
  globalInstrTable = instrTable;
  opcodeTable = new HashTable(HT_INTKEY,(int) (OZERROR*1.5));
  for (int i=0; i<=OZERROR; i++) {
    opcodeTable->htAdd(ToInt32(globalInstrTable[i]),ToPointer(i));
  }
#endif
  CodeArea *code = new CodeArea(20);
  C_XCONT_Ptr = code->getStart();
  C_CFUNC_CONT_Ptr   = writeOpcode(TASKXCONT,C_XCONT_Ptr);
  C_DEBUG_CONT_Ptr   = writeOpcode(TASKCFUNCONT,C_CFUNC_CONT_Ptr);
  C_CALL_CONT_Ptr    = writeOpcode(TASKDEBUGCONT,C_DEBUG_CONT_Ptr);
  C_LOCK_Ptr         = writeOpcode(TASKCALLCONT,C_CALL_CONT_Ptr);
  C_SET_SELF_Ptr     = writeOpcode(TASKLOCK,C_LOCK_Ptr);
  C_SET_ABSTR_Ptr    = writeOpcode(TASKSETSELF,C_SET_SELF_Ptr);
  C_LTQ_Ptr          = writeOpcode(TASKPROFILECALL,C_SET_ABSTR_Ptr);
  C_ACTOR_Ptr        = writeOpcode(TASKLTQ,C_LTQ_Ptr);
  C_CATCH_Ptr        = writeOpcode(TASKACTOR,C_ACTOR_Ptr);
  C_EMPTY_STACK      = writeOpcode(TASKCATCH,C_CATCH_Ptr);
  ProgramCounter aux = writeOpcode(TASKEMPTYSTACK,C_EMPTY_STACK);
  /* mark end with GLOBALVARNAME, so definitionEnd works properly */
  (void) writeOpcode(GLOBALVARNAME,aux);
}
