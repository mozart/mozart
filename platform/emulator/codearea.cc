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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "codearea.hh"
#include "indexing.hh"

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
CodeArea *CodeArea::allBlocks = NULL;

#ifdef THREADED
void **CodeArea::globalInstrTable = 0;
HashTable *CodeArea::opcodeTable = 0;
#endif




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


OZ_Term oz_atom(const char *str)
{
  CHECK_STRPTR(str);
  Literal *lit=addToLiteralTab(str,&CodeArea::atomTab,NO);
  return makeTaggedLiteral(lit);
}

TaggedRef oz_uniqueName(const char *str)
{
  CHECK_STRPTR(str);
  Literal *lit = addToLiteralTab(str,&CodeArea::nameTab,OK);
  lit->setFlag(Lit_isUniqueName);
  return makeTaggedLiteral(lit);
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


AbstractionEntry *AbstractionEntry::allEntries = NULL;

void AbstractionEntry::setPred(Abstraction *ab)
{
  Assert(!copyable && !abstr);

  abstr = ab;
  pc    = abstr->getPC();
  arity = abstr->getArity();

  // indexing on X[0] optimized !!!
  if (pc != NOCODE &&
      CodeArea::getOpcode(pc) == MATCHX &&
      getRegArg(pc+1) == 0) {
    indexTable = (IHashTable *) getAdressArg(pc+2);
  } else {
    indexTable = NULL;
  }
}


#define DISPATCH() PC += sizeOf(op); break



const char *getBIName(ProgramCounter PC)
{
  Builtin* entry = (Builtin*) getAdressArg(PC);
  return entry->getPrintName();
}


ProgramCounter CodeArea::printDef(ProgramCounter PC,FILE *out)
{
  ProgramCounter definitionPC = definitionStart(PC);
  if (definitionPC == NOCODE) {
    fprintf(out,"***\tspecial task or on toplevel (PC=%s)\n",
            opcodeToString(getOpcode(PC)));
    fflush(out);
    return definitionPC;
  }

  Reg reg;
  int next, line,colum;
  TaggedRef comment, predName, file;
  getDefinitionArgs(definitionPC,reg,next,file,line,colum,predName);
  getNextDebugInfoArgs(PC,file,line,colum,comment);

  const char *name = OZ_atomToC(predName);
  fprintf(out,"***\tprocedure");
  if (*name) fprintf(out," '%s'",name);
  fprintf(out," f: \"%s\" l: %d c: %d",
          OZ_atomToC(file),
          line,colum);
  fprintf(out," PC: %p\n",definitionPC);
  fflush(out);
  return definitionPC;
}

TaggedRef CodeArea::dbgGetDef(ProgramCounter PC, ProgramCounter definitionPC,
                              int frameId, RefsArray Y, Abstraction *CAP)
{
  Reg reg;
  int next, line, colum;
  TaggedRef comment, predName, file;
  // file & line might be overwritten some lines later ...
  getDefinitionArgs(definitionPC,reg,next,file,line,colum,predName);

  // if we are lucky there's some debuginfo and we can determine
  // the exact position inside the procedure application
  //--** problem: these are the coordinates of the corresponding exit
  //     instruction
  getNextDebugInfoArgs(PC,file,line,colum,comment);

  TaggedRef pairlist = oz_nil();
  pairlist =
    oz_cons(OZ_pairAI("time",findTimeStamp(PC)),
         oz_cons(OZ_pairA("data",makeTaggedConst(CAP)),
              oz_cons(OZ_pairA("file",file),
                   oz_cons(OZ_pairAI("line",line < 0? -line: line),
                        oz_cons(OZ_pairA("column",OZ_int(colum)),
                             oz_cons(OZ_pairAI("PC",(int)PC),
                                  oz_cons(OZ_pairA("kind",OZ_atom("call")),
                                       oz_cons(OZ_pairA("origin",
                                                  OZ_atom("procedureFrame")),
                                            pairlist))))))));
  if (frameId != -1)
    pairlist = oz_cons(OZ_pairAI("frameID",frameId),pairlist);
  else
    pairlist = oz_cons(OZ_pairA("vars",getFrameVariables(PC,Y,CAP)),pairlist);

  return OZ_recordInit(OZ_atom("entry"), pairlist);
}

TaggedRef CodeArea::getFrameVariables(ProgramCounter PC,
                                      RefsArray Y, Abstraction *CAP) {
  TaggedRef locals = oz_nil();
  TaggedRef globals = oz_nil();

  ProgramCounter aux = definitionEnd(PC);

  if (aux != NOCODE) {
    aux += sizeOf(getOpcode(aux));

    for (int i=0; getOpcode(aux) == LOCALVARNAME; i++) {
      if (Y) {
        TaggedRef aux1 = getLiteralArg(aux+1);
        if (!oz_eq(aux1, AtomEmpty) && Y[i] != makeTaggedNULL()) {
          locals = oz_cons(OZ_mkTupleC("#", 2, aux1, Y[i]), locals);
        }
      }
      aux += sizeOf(getOpcode(aux));
    }
    locals = reverseC(locals);

    int gsize=CAP->getPred()->getGSize();
    if (gsize>0) {
      for (int i=0; getOpcode(aux) == GLOBALVARNAME; i++) {
        TaggedRef aux1 = getLiteralArg(aux+1);
        if (!oz_eq(aux1, AtomEmpty)) {
          globals = oz_cons(OZ_mkTupleC("#", 2, aux1, CAP->getG(i)), globals);
        }
        aux += sizeOf(getOpcode(aux));
      }
      globals = reverseC(globals);
    }
  }

  TaggedRef pairlist =
    oz_cons(OZ_pairA("Y", locals),
         oz_cons(OZ_pairA("G", globals),
              oz_nil()));

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
                                    TaggedRef &file, int &line, int &colum,
                                    TaggedRef &comment)
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
      line    = OZ_intToC(getTaggedArg(PC+2));
      colum   = OZ_intToC(getTaggedArg(PC+3));
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
    case TASKXCONT:
    case TASKCFUNCONT:
    case TASKDEBUGCONT:
    case TASKCALLCONT:
    case TASKLOCK:
    case TASKSETSELF:
    case TASKLPQ:
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
  ProgramCounter start=CodeArea::printDef(from,stderr);
  if (start != NOCODE) CodeArea::display(start,ssize,stderr,from);
}


inline
ProgramCounter computeLabelArg(ProgramCounter pc, ProgramCounter arg)
{
  return pc+getLabelArg(arg);
}


void CodeArea::getDefinitionArgs(ProgramCounter PC,
                                 Reg &reg, int &next,
                                 TaggedRef &file, int &line, int &colum,
                                 TaggedRef &predName)
{
  Assert(getOpcode(PC) == DEFINITION || getOpcode(PC) == DEFINITIONCOPY);
  PrTabEntry *pred = getPredArg(PC+3);

  reg      = regToInt(getRegArg(PC+1));
  next     = getLabelArg(PC+2);
  if (pred!=NULL) {
    file     = pred->getFile();
    line     = pred->getLine();
    colum    = pred->getColumn();
    predName = OZ_atom(pred->getPrintName());
  } else {
    file     = OZ_atom("");
    line     = colum = 0;
    predName = OZ_atom("");
  }
}

static
void printLoc(FILE *ofile,OZ_Location *loc) {
  if (loc->getInArity()) {
    fprintf(ofile,"[");
    for (int i=0; i<loc->getInArity(); i++) {
      fprintf(ofile,"%sx(%d)",i?" ":"",loc->in(i));
    }
    fprintf(ofile,"]");
  } else
    fprintf(ofile,"nil");
  fprintf(ofile,"#");
  if (loc->getOutArity()) {
    fprintf(ofile,"[");
    for (int i=0; i<loc->getOutArity(); i++) {
      fprintf(ofile,"%sx(%d)",i?" ":"",loc->out(i));
    }
    fprintf(ofile,"]");
  } else
    fprintf(ofile,"nil");
}

void CodeArea::display(ProgramCounter from, int sz, FILE* ofile,
                       ProgramCounter to)
{
  ProgramCounter PC = from;
  int defCount = 0; // counter for nested definitions
  for (int i = 1; i <= sz || sz <= 0 ; i++) {

    if (sz <=0 && to != NOCODE && PC > to) {
      fflush(ofile);
      return;
    }

    fprintf(ofile, "%p:\t", PC);
    Opcode op = getOpcode(PC);
    if (op == OZERROR || op == ENDOFFILE) {
      fprintf(ofile,"End of code block reached\n");
      fflush(ofile);
      return;
    }

    fprintf(ofile, "%03d\t%s", op, opcodeToString(op));

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
    case TASKLPQ:
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
        fprintf(ofile, "%s)\n", toC(getTaggedArg(PC+4)));
        DISPATCH();
      }

    case PUTLISTX:
    case PUTLISTY:
    case PUTLISTG:
    case FUNRETURNX:
    case FUNRETURNY:
    case FUNRETURNG:
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
    case GETRETURNX:
    case GETRETURNY:
    case GETRETURNG:
    case CREATEVARIABLEX:
    case CREATEVARIABLEY:
    case CREATEVARIABLEG:
    case GETSELF:
    case SETSELF:
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

    case TESTLITERALX:
    case TESTLITERALY:
    case TESTLITERALG:
    case TESTNUMBERX:
    case TESTNUMBERY:
    case TESTNUMBERG:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf (ofile,
                 "(%d %s %p)\n",
                 regToInt(getRegArg(PC+1)),
                 toC(tagged),
                 computeLabelArg(PC,PC+3));
        DISPATCH();
      }

    case TESTRECORDX:
    case TESTRECORDY:
    case TESTRECORDG:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf(ofile, "(%d %s ", regToInt(getRegArg(PC+1)), toC(tagged));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " %p)\n", computeLabelArg(PC,PC+4));
        DISPATCH();
      }

    case TESTLISTX:
    case TESTLISTY:
    case TESTLISTG:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf(ofile,
                "(%d %p)\n",
                regToInt(getRegArg(PC+1)),
                computeLabelArg(PC,PC+2));
        DISPATCH();
      }

    case TESTBOOLX:
    case TESTBOOLY:
    case TESTBOOLG:
      {
        fprintf (ofile,
                 "(%d %p %p)\n",
                 regToInt(getRegArg(PC+1)),
                 computeLabelArg(PC,PC+2),
                 computeLabelArg(PC,PC+3));
        DISPATCH();
      }

    case TESTLE:
    case TESTLT:
      fprintf (ofile,
               "(x(%d) x(%d) x(%d) %p)\n",
               regToInt(getRegArg(PC+1)),
               regToInt(getRegArg(PC+2)),
               regToInt(getRegArg(PC+3)),
               computeLabelArg(PC,PC+4));
      DISPATCH();

    case INLINEPLUS1:
    case INLINEMINUS1:
      fprintf (ofile,
               "(x(%d) x(%d))\n",
               regToInt(getRegArg(PC+1)),
               regToInt(getRegArg(PC+2)));
      DISPATCH();

    case INLINEPLUS:
    case INLINEMINUS:
    case INLINEUPARROW:
      {
        fprintf (ofile,
                 "(x(%d) x(%d) x(%d))\n",
                 regToInt(getRegArg(PC+1)),
                 regToInt(getRegArg(PC+2)),
                 regToInt(getRegArg(PC+3)));
      }
      DISPATCH();

    case INLINEDOT:
      {
        TaggedRef literal = getLiteralArg(PC+2);
        fprintf (ofile,
                 "(x(%d) %s x(%d))\n",
                 regToInt(getRegArg(PC+1)),
                 toC(literal),
                 regToInt(getRegArg(PC+3)));
      }
      DISPATCH();

    case INLINEAT:
    case INLINEASSIGN:
      {
        TaggedRef literal = getLiteralArg(PC+1);
        fprintf (ofile,
                 "(%s x(%d))\n",
                 toC(literal),
                 regToInt(getRegArg(PC+2)));
      }
      DISPATCH();

    case CALLBI: // mm2
      fprintf(ofile, "(%s ", getBIName(PC+1));
      printLoc(ofile,GetLoc(PC+2));
      fprintf(ofile, ")\n");
      DISPATCH();

    case TESTBI: // mm2
      fprintf(ofile, "(%s ", getBIName(PC+1));
      printLoc(ofile,GetLoc(PC+2));
      fprintf(ofile, " %p)\n",computeLabelArg(PC,PC+3));
      DISPATCH();

    case GENFASTCALL:
    case FASTCALL:
    case FASTTAILCALL:
      {
        AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
        fprintf(ofile,"(%p[pc:%p n:%d] %d)\n",entry,
                entry->getPC(),entry->getArity(),getPosIntArg(PC+2));
        DISPATCH();
      }

    case SETPREDICATEREF:
      {
        AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
        fprintf(ofile,"(%p[pc:%p n:%d])\n",entry,
                entry->getPC(),entry->getArity());
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

    case UNIFYNUMBER:
      {
        fprintf(ofile, "(%s)\n", toC(getNumberArg(PC+1)));
      }
      DISPATCH();

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
        int next,line,colum;
        TaggedRef file, predName;
        getDefinitionArgs(PC,reg,next,file,line,colum,predName);
        PrTabEntry *predd = getPredArg(PC+3);
        AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+4);
        AssRegArray *list = (AssRegArray*) getAdressArg(PC+5);
        fprintf(ofile,"(x(%d) %d pid(%s",reg,next,toC(predName));
        fprintf(ofile," %d",predd->getArity());
        fprintf(ofile," pos(%s %d %d)",
                OZ_atomToC(file),line,colum);
        fprintf(ofile," [");
        fprintf(ofile,"%s",predd->isSited()?" sited":"");
        fprintf(ofile," ]");
        fprintf(ofile," %d",predd->getMaxX());
        fprintf(ofile,") ");
        if (predEntry)
          fprintf(ofile,"%p ",predEntry);
        else
          fprintf(ofile,"unit ");

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
      {
        TaggedRef tagged = getTaggedArg(PC+1);

        fprintf(ofile, "(%s %d)\n", toC(tagged),
                 regToInt(getRegArg(PC+2)));
      }
      DISPATCH();

    case ENDDEFINITION:
      fprintf(ofile, "(%p)\n", computeLabelArg(PC,PC+1));
      if (sz<=0 && defCount<=1) {
        fflush(ofile);
        return;
      }
      defCount--;
      DISPATCH();

    case BRANCH:
    case NEXTCLAUSE:
    case THREAD:
    case EXHANDLER:
      fprintf(ofile, "(%p)\n", computeLabelArg(PC,PC+1));
      DISPATCH();

    case MATCHX:
    case MATCHY:
    case MATCHG:
      {
        Reg reg = regToInt(getRegArg(PC+1));
        fprintf(ofile, "(%d ...)\n", reg);
      }
      DISPATCH();

    case SHALLOWGUARD:
    case CREATECOND:
      {
        ProgramCounter lbl = computeLabelArg(PC,PC+1);
        fprintf(ofile, "(%p)\n", lbl);
      }
      DISPATCH();

    case LOCKTHREAD:
      {
        ProgramCounter lbl = computeLabelArg(PC,PC+1);
        int n      = regToInt(getRegArg(PC+2));
        fprintf(ofile, "(%p x(%d))\n", lbl, n);
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
      fprintf(ofile,"Illegal instruction");
      fflush(ofile);
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
  C_LPQ_Ptr,
  C_CATCH_Ptr,
  C_ACTOR_Ptr,
  C_EMPTY_STACK;



CodeArea::~CodeArea()
{
#ifdef DEBUG_CHECK
  memset(getStart(),-1,size*sizeof(ByteCode));
#else
  delete [] getStart();
#endif
  gclist->dispose();
}


CodeArea::CodeArea(int sz)
{
  allocateBlock(sz);
  referenced = NO;
  gclist     = NULL;
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
  C_LPQ_Ptr          = writeOpcode(TASKPROFILECALL,C_SET_ABSTR_Ptr);
  C_ACTOR_Ptr        = writeOpcode(TASKLPQ,C_LPQ_Ptr);
  C_CATCH_Ptr        = writeOpcode(TASKACTOR,C_ACTOR_Ptr);
  C_EMPTY_STACK      = writeOpcode(TASKCATCH,C_CATCH_Ptr);
  ProgramCounter aux = writeOpcode(TASKEMPTYSTACK,C_EMPTY_STACK);
  /* mark end with GLOBALVARNAME, so definitionEnd works properly */
  (void) writeOpcode(GLOBALVARNAME,aux);
}

#ifdef RECINSTRFETCH

#define InstrDumpFile "fetchedInstr.dump"

void CodeArea::writeInstr(void){
  FILE* ofile;
  if((ofile = fopen(InstrDumpFile, "w"))){
    int i = fetchedInstr;
//    ofile=stdout;
    do {
      if (ops[i]) {
        display(ops[i], 1, ofile);
      }
      i++;
      if(i >= RECINSTRFETCH)
        i = 0;
    } while (i != fetchedInstr);
    fclose(ofile);
    fprintf(stderr,
            "Wrote the %d most recently fetched instructions in file '%s'\n",
            RECINSTRFETCH, InstrDumpFile);
    fflush(stderr);
  } else
    OZ_error("Cannot open file '%s'.", InstrDumpFile);
} // CodeArea::writeInstr
#endif

#ifdef DEBUG_CHECK
// for debugging
void printWhere(ostream &stream,ProgramCounter PC)
{
  PC = CodeArea::definitionStart(PC);

  if (PC == NOCODE) {
    stream << "in toplevel code";
  } else {
    TaggedRef file      = getLiteralArg(PC+3);
    TaggedRef line      = getNumberArg(PC+4);
    PrTabEntry *pred    = getPredArg(PC+5);

    stream << "procedure "
           << (pred ? pred->getPrintName() : "(NULL)")
           << " in file \""
           << toC(file)
           << "\", line "
           << toC(line);
  }
}
#endif


CodeArea *CodeArea::findBlock(ProgramCounter PC)
{
  CodeArea *aux = allBlocks;
  while (aux) {
    ByteCode *start = aux->getStart();
    if (start <= PC && PC < start + aux->size) {
      return aux;
    }
    aux = aux->nextBlock;
  }
  Assert(0);
  return NULL;
}


void CodeArea::unprotect(TaggedRef* t)
{
  gclist->remove(t);
}


void CodeGCList::remove(TaggedRef *t)
{
  for (CodeGCList *aux = this; aux!=NULL; aux = aux->next) {
    for (int i=0; i<codeGCListBlockSize; i++) {
      if (aux->block[i].u.tagged == t) {
        aux->block[i].u.tagged = NULL;
        aux->block[i].tag      = C_FREE;
        return;
      }
    }
  }

  Assert(0);
}



ProgramCounter CodeArea::writeCache(ProgramCounter PC)
{
  InlineCache *cache = (InlineCache *) PC;
  PC = writeInt(0, PC);
  PC = writeInt(0, PC);
  cache->invalidate();
  protectInlineCache(cache);
  return PC;
}

void CodeArea::allocateBlock(int sz)
{
  size = sz + 1;
  codeBlock  = new ByteCode[size]; /* allocation via malloc! */
  writeOpcode(ENDOFFILE,codeBlock+sz); /* mark the end, so that
                                        * displayCode and friends work */
  totalSize += size * sizeof(ByteCode);
  wPtr       = codeBlock;
  nextBlock  = allBlocks;
  allBlocks  = this;
  timeStamp  = time(0);
}


ProgramCounter CodeArea::writeTagged(TaggedRef t, ProgramCounter ptr)
{
  Assert(getStart()<=ptr && ptr < getStart()+size);
  ProgramCounter ret = writeWord(t,ptr);
  TaggedRef *tptr = (TaggedRef *)ptr;
  if (!needsNoCollection(*tptr)) {
    checkPtr(tptr);
    gclist = gclist->add(tptr);
  }
  return ret;
}


ProgramCounter CodeArea::writeAbstractionEntry(AbstractionEntry *p, ProgramCounter ptr)
{
  ProgramCounter ret = writeAddress(p,ptr);
  checkPtr(ptr);
  gclist = gclist->add((AbstractionEntry**)ptr);
  return ret;
}
