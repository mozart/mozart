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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

StringHashTable CodeArea::atomTab(10000);
StringHashTable CodeArea::nameTab(1000);
CodeArea *CodeArea::allBlocks = NULL;

#ifdef THREADED
void **CodeArea::globalInstrTable = 0;
#ifndef INLINEOPCODEMAP
AddressHashTable *CodeArea::opcodeTable = 0;
#endif
#endif




inline
Literal *addToLiteralTab(const char *str, StringHashTable *table,
                         Bool isName, Bool needsDup) {
  Literal *found = (Literal *) table->htFind(str);

  if (found != (Literal *) htEmpty) {
    return found;
  }

  if (needsDup)
    str = strdup(str);

  if (isName) {
    found = NamedName::newNamedName(str);
  } else {
    found = Atom::newAtom(str);
  }

  table->htAdd(str, found);
  return found;
}


OZ_Term OZ_atom(OZ_CONST char *str)
{
  Assert(str != NULL);
  Literal *lit=addToLiteralTab(str,&CodeArea::atomTab,NO,OK);
  return makeTaggedLiteral(lit);
}

OZ_Term oz_atomNoDup(OZ_CONST char *str) {
  Assert(str != NULL);
  Literal *lit=addToLiteralTab(str,&CodeArea::atomTab,NO,NO);
  return makeTaggedLiteral(lit);
}

TaggedRef oz_uniqueName(const char *str)
{
  Assert(str != NULL);
  Literal *lit = addToLiteralTab(str,&CodeArea::nameTab,OK,OK);
  lit->setFlag(Lit_isUniqueName);
  return makeTaggedLiteral(lit);
}


AbstractionEntry *AbstractionEntry::allEntries = NULL;

int CodeArea::getTotalSize(void) {
  int ts = 0;
  for (CodeArea * ca = allBlocks; ca ; ca = ca->nextBlock)
    ts += ca->size * sizeof(ByteCode);
  return ts;
}


#define DISPATCH() PC += sizeOf(op); break



const char *getBIName(ProgramCounter PC)
{
  Builtin* entry = (Builtin*) getAdressArg(PC);
  return entry->getPrintName();
}


#ifdef DEBUG_CHECK

ProgramCounter CodeArea::printDef(ProgramCounter PC,FILE *out)
{
  ProgramCounter definitionPC = definitionStart(PC);
  if (definitionPC == NOCODE) {
    fprintf(out,"***\tspecial task or on toplevel (PC=%s)\n",
            opcodeToString(getOpcode(PC)));
    fflush(out);
    return definitionPC;
  }

  XReg reg;
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

#endif

TaggedRef CodeArea::dbgGetDef(ProgramCounter PC, ProgramCounter definitionPC,
                              int frameId, RefsArray *Y, Abstraction *CAP)
{
  XReg reg;
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
    oz_cons(OZ_pair2(AtomData,makeTaggedConst(CAP)),
        oz_cons(OZ_pair2(AtomFile,file),
            oz_cons(OZ_pair2(AtomLine,OZ_int(line < 0? -line: line)),
                oz_cons(OZ_pair2(AtomColumn,OZ_int(colum)),
                    oz_cons(OZ_pair2(AtomPC,OZ_int((int) PC)),
                        oz_cons(OZ_pair2(AtomKind,AtomCall),
                            oz_cons(OZ_pair2(AtomOrigin,AtomProcedureFrame),
                                pairlist)))))));
  if (frameId != -1)
    pairlist =
      oz_cons(OZ_pair2(AtomFrameID,OZ_int(frameId)),pairlist);
  else
    pairlist =
      oz_cons(OZ_pair2(AtomVars,getFrameVariables(PC,Y,CAP)),pairlist);

  return OZ_recordInit(AtomEntry,pairlist);
}

TaggedRef CodeArea::getFrameVariables(ProgramCounter PC,
                                      RefsArray *Y, Abstraction *CAP) {
  TaggedRef locals = oz_nil();
  TaggedRef globals = oz_nil();

  ProgramCounter aux = definitionEnd(PC);

  if (aux != NOCODE) {
    aux += sizeOf(getOpcode(aux));

    for (int i=0; getOpcode(aux) == LOCALVARNAME; i++) {
      if (Y) {
        TaggedRef aux1 = getLiteralArg(aux+1);
        if (!oz_eq(aux1, AtomEmpty) && Y->getArg(i) != NameVoidRegister) {
          TaggedRef r = Y->getArg(i);
          if (r == makeTaggedNULL())
            r = OZ_atom("<eliminated by garbage collection>");
          locals = oz_cons(OZ_mkTupleC("#", 2, aux1, r), locals);
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
          TaggedRef r = CAP->getG(i);
          if (r == makeTaggedNULL())
            r = OZ_atom("<eliminated by garbage collection>");
          globals = oz_cons(OZ_mkTupleC("#", 2, aux1, r), globals);
        }
        aux += sizeOf(getOpcode(aux));
      }
      globals = reverseC(globals);
    }
  }

  TaggedRef pairlist =
    oz_cons(OZ_pair2(AtomY, locals),
         oz_cons(OZ_pair2(AtomG, globals),
              oz_nil()));

  TaggedRef ret = OZ_recordInit(AtomV, pairlist);
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
    case TASKDEBUGCONT:
    case TASKCALLCONT:
    case TASKLOCK:
    case TASKSETSELF:
    case TASKCATCH:
    case TASKEMPTYSTACK:
    case TASKPROFILECALL:
    case ENDOFFILE:
    case OZERROR:
    case GLOBALVARNAME:   // last instr in CodeArea::init
      return NOCODE;
    default:
      DISPATCH();
    }
  }
}

void CodeArea::getDefinitionArgs(ProgramCounter PC,
                                 XReg &reg, int &next,
                                 TaggedRef &file, int &line, int &colum,
                                 TaggedRef &predName)
{
  Assert(getOpcode(PC) == DEFINITION || getOpcode(PC) == DEFINITIONCOPY);
  PrTabEntry *pred = getPredArg(PC+3);

  reg      = XRegToInt(getXRegArg(PC+1));
  next     = getLabelArg(PC+2);
  if (pred!=NULL) {
    file     = pred->getFile();
    line     = pred->getLine();
    colum    = pred->getColumn();
    predName = OZ_atom((OZ_CONST char*)pred->getPrintName());
  } else {
    file     = AtomEmpty;
    line     = colum = 0;
    predName = AtomEmpty;
  }
}

void AbstractionEntry::setPred(Abstraction * ab) {
  Assert(!isCopyable() && !abstr);
  Assert(ab);
  abstr = makeTaggedConst(ab);
  pc    = ab->getPC();

  // indexing on X[0] optimized !!!
// kost@ : this a contra-optimization, at least - now;
//    if (pc != NOCODE &&
//        CodeArea::getOpcode(pc) == MATCHX &&
//        getXRegArg(pc+1) == 0) {
//      listpc = pc + ((IHashTable *) getAdressArg(pc+2))->lookupLTuple();
//    } else if (pc != NOCODE &&
//           CodeArea::getOpcode(pc) == TESTLISTX &&
//           getXRegArg(pc+1) == 0) {
//      listpc = pc+3;
//    } else {
//      listpc = pc;
//    }
}


#ifdef DEBUG_CHECK

extern "C" void displayCode(ProgramCounter from, int ssize)
{
  CodeArea::display(from,ssize,stderr);
  fflush(stderr);
}

extern "C" void displayDef(ProgramCounter from, int ssize)
{
  ProgramCounter start=CodeArea::printDef(from,stderr);
  if (start != NOCODE) CodeArea::display(start,ssize,stderr,from);
}


inline
ProgramCounter computeLabelArg(ProgramCounter pc, ProgramCounter arg)
{
  return pc+getLabelArg(arg);
}


static
void printLoc(FILE *ofile,Builtin * bi, OZ_Location *loc) {
  if (bi->getInArity()) {
    fprintf(ofile,"[");
    for (int i=0; i<bi->getInArity(); i++) {
      fprintf(ofile,"%sx(%d)",i?" ":"",loc->getIndex(i));
    }
    fprintf(ofile,"]");
  } else
    fprintf(ofile,"nil");
  fprintf(ofile,"#");
  if (bi->getOutArity()) {
    fprintf(ofile,"[");
    for (int i=0; i<bi->getOutArity(); i++) {
      fprintf(ofile,"%sx(%d)",i?" ":"",loc->getIndex(i));
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
    case SKIP:
    case RETURN:
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
    case PROFILEPROC:
    case POPEX:
    case TASKXCONT:
    case TASKDEBUGCONT:
    case TASKCALLCONT:
    case TASKLOCK:
    case TASKSETSELF:
    case TASKCATCH:
    case TASKEMPTYSTACK:
    case TASKPROFILECALL:
      fprintf(ofile, "\n");
      DISPATCH();

    case DEBUGENTRY:
    case DEBUGEXIT:
      {
        fprintf(ofile, "(%s ",  toC(getTaggedArg(PC+1)));
        fprintf(ofile, "%s ",   toC(getTaggedArg(PC+2)));
        fprintf(ofile, "%s ",   toC(getTaggedArg(PC+3)));
        fprintf(ofile, "%s)\n", toC(getTaggedArg(PC+4)));
        DISPATCH();
      }

    case PUTLISTX:
    case FUNRETURNX:
    case SETVALUEX:
    case GETLISTX:
    case UNIFYVALUEX:
    case GETVARIABLEX:
    case SETVARIABLEX:
    case UNIFYVARIABLEX:
    case GETRETURNX:
    case CREATEVARIABLEX:
    case GETSELF:
      fprintf(ofile, "(x(%d))\n", XRegToInt(getXRegArg(PC+1)));
      DISPATCH();

    case PUTLISTY:
    case FUNRETURNY:
    case SETVALUEY:
    case GETLISTY:
    case UNIFYVALUEY:
    case GETVARIABLEY:
    case SETVARIABLEY:
    case UNIFYVARIABLEY:
    case GETRETURNY:
    case CREATEVARIABLEY:
    case CLEARY:
      fprintf(ofile, "(y(%d))\n", YRegToInt(getYRegArg(PC+1)));
      DISPATCH();

    case FUNRETURNG:
    case SETVALUEG:
    case GETLISTG:
    case UNIFYVALUEG:
    case GETRETURNG:
    case SETSELFG:
      fprintf(ofile, "(g(%d))\n", GRegToInt(getGRegArg(PC+1)));
      DISPATCH();

    case CREATEVARIABLEMOVEX:
      fprintf (ofile, "(x(%d) x(%d))\n",
               XRegToInt(getXRegArg(PC+1)),
               XRegToInt(getXRegArg(PC+2)));
      DISPATCH();

    case CREATEVARIABLEMOVEY:
      fprintf (ofile, "(y(%d) x(%d))\n",
               YRegToInt(getYRegArg(PC+1)),
               XRegToInt(getXRegArg(PC+2)));
      DISPATCH();

    case GETLISTVALVARX:
      fprintf (ofile, "(x(%d) x(%d) x(%d))\n",
               XRegToInt(getXRegArg(PC+1)),
               XRegToInt(getXRegArg(PC+2)),
               XRegToInt(getXRegArg(PC+3)));
      DISPATCH();

    case MOVEMOVEXYXY:
      fprintf (ofile, "(x(%d) y(%d) x(%d) y(%d))\n",
              XRegToInt(getXRegArg(PC+1)), YRegToInt(getYRegArg(PC+2)),
              XRegToInt(getXRegArg(PC+3)), YRegToInt(getYRegArg(PC+4)));
      DISPATCH();

    case MOVEMOVEYXXY:
      fprintf (ofile, "(y(%d) x(%d) x(%d) y(%d))\n",
              YRegToInt(getYRegArg(PC+1)), XRegToInt(getXRegArg(PC+2)),
              XRegToInt(getXRegArg(PC+3)), YRegToInt(getYRegArg(PC+4)));
      DISPATCH();

    case MOVEMOVEYXYX:
      fprintf (ofile, "(y(%d) x(%d) y(%d) x(%d))\n",
               YRegToInt(getYRegArg(PC+1)), XRegToInt(getXRegArg(PC+2)),
               YRegToInt(getYRegArg(PC+3)), XRegToInt(getXRegArg(PC+4)));
      DISPATCH();

    case MOVEMOVEXYYX:
      fprintf (ofile, "(x(%d) y(%d) y(%d) x(%d))\n",
              XRegToInt(getXRegArg(PC+1)), YRegToInt(getYRegArg(PC+2)),
              YRegToInt(getYRegArg(PC+3)), XRegToInt(getXRegArg(PC+4)));
      DISPATCH();

    case TESTLITERALX:
    case TESTNUMBERX:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf (ofile,
                 "(x(%d) %s %p)\n",
                 XRegToInt(getXRegArg(PC+1)),
                 toC(tagged),
                 computeLabelArg(PC,PC+3));
        DISPATCH();
      }

    case TESTLITERALY:
    case TESTNUMBERY:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf (ofile,
                 "(y(%d) %s %p)\n",
                 YRegToInt(getYRegArg(PC+1)),
                 toC(tagged),
                 computeLabelArg(PC,PC+3));
        DISPATCH();
      }

    case TESTLITERALG:
    case TESTNUMBERG:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf (ofile,
                 "(g(%d) %s %p)\n",
                 GRegToInt(getGRegArg(PC+1)),
                 toC(tagged),
                 computeLabelArg(PC,PC+3));
        DISPATCH();
      }

    case TESTRECORDX:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf(ofile, "(%d %s ", XRegToInt(getXRegArg(PC+1)), toC(tagged));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " %p)\n", computeLabelArg(PC,PC+4));
        DISPATCH();
      }

    case TESTRECORDY:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf(ofile, "(%d %s ", YRegToInt(getYRegArg(PC+1)), toC(tagged));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " %p)\n", computeLabelArg(PC,PC+4));
        DISPATCH();
      }

    case TESTRECORDG:
      {
        TaggedRef tagged = getTaggedArg(PC+2);
        fprintf(ofile, "(%d %s ", GRegToInt(getGRegArg(PC+1)), toC(tagged));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " %p)\n", computeLabelArg(PC,PC+4));
        DISPATCH();
      }

    case TESTLISTX:
      {
        fprintf(ofile,
                "(%d %p)\n",
                XRegToInt(getXRegArg(PC+1)),
                computeLabelArg(PC,PC+2));
        DISPATCH();
      }

    case TESTLISTY:
      {
        fprintf(ofile,
                "(%d %p)\n",
                YRegToInt(getYRegArg(PC+1)),
                computeLabelArg(PC,PC+2));
        DISPATCH();
      }

    case TESTLISTG:
      {
        fprintf(ofile,
                "(%d %p)\n",
                GRegToInt(getGRegArg(PC+1)),
                computeLabelArg(PC,PC+2));
        DISPATCH();
      }

    case TESTBOOLX:
      {
        fprintf (ofile,
                 "(%d %p %p)\n",
                 XRegToInt(getXRegArg(PC+1)),
                 computeLabelArg(PC,PC+2),
                 computeLabelArg(PC,PC+3));
        DISPATCH();
      }

    case TESTBOOLY:
      {
        fprintf (ofile,
                 "(%d %p %p)\n",
                 YRegToInt(getYRegArg(PC+1)),
                 computeLabelArg(PC,PC+2),
                 computeLabelArg(PC,PC+3));
        DISPATCH();
      }

    case TESTBOOLG:
      {
        fprintf (ofile,
                 "(%d %p %p)\n",
                 GRegToInt(getGRegArg(PC+1)),
                 computeLabelArg(PC,PC+2),
                 computeLabelArg(PC,PC+3));
        DISPATCH();
      }

    case TESTLE:
    case TESTLT:
      fprintf (ofile,
               "(x(%d) x(%d) x(%d) %p)\n",
               XRegToInt(getXRegArg(PC+1)),
               XRegToInt(getXRegArg(PC+2)),
               XRegToInt(getXRegArg(PC+3)),
               computeLabelArg(PC,PC+4));
      DISPATCH();

    case INLINEPLUS1:
    case INLINEMINUS1:
      fprintf (ofile,
               "(x(%d) x(%d))\n",
               XRegToInt(getXRegArg(PC+1)),
               XRegToInt(getXRegArg(PC+2)));
      DISPATCH();

    case INLINEPLUS:
    case INLINEMINUS:
      {
        fprintf (ofile,
                 "(x(%d) x(%d) x(%d))\n",
                 XRegToInt(getXRegArg(PC+1)),
                 XRegToInt(getXRegArg(PC+2)),
                 XRegToInt(getXRegArg(PC+3)));
      }
      DISPATCH();

    case INLINEDOT:
      {
        TaggedRef literal = getLiteralArg(PC+2);
        fprintf (ofile,
                 "(x(%d) %s x(%d))\n",
                 XRegToInt(getXRegArg(PC+1)),
                 toC(literal),
                 XRegToInt(getXRegArg(PC+3)));
      }
      DISPATCH();

    case INLINEAT:
    case INLINEASSIGN:
      {
        TaggedRef literal = getLiteralArg(PC+1);
        fprintf (ofile,
                 "(%s x(%d))\n",
                 toC(literal),
                 XRegToInt(getXRegArg(PC+2)));
      }
      DISPATCH();

    case CALLBI: // mm2
      fprintf(ofile, "(%s ", getBIName(PC+1));
      printLoc(ofile,GetBI(PC+1),GetLoc(PC+2));
      fprintf(ofile, ")\n");
      DISPATCH();

    case TESTBI: // mm2
      fprintf(ofile, "(%s ", getBIName(PC+1));
      printLoc(ofile,GetBI(PC+1),GetLoc(PC+2));
      fprintf(ofile, " %p)\n",computeLabelArg(PC,PC+3));
      DISPATCH();

    case CALLGLOBAL:
      {
        fprintf(ofile,"(%d %d)\n"
                ,XRegToInt(getXRegArg(PC+1)),getPosIntArg(PC+2));
        DISPATCH();
      }

    case FASTCALL:
    case FASTTAILCALL:
    case CALLPROCEDUREREF:
      {
        AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
        fprintf(ofile,"(%p[pc:%p] %d)\n",entry,
                entry->getPC(),getPosIntArg(PC+2));
        DISPATCH();
      }

    case SETPROCEDUREREF:
      {
        AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
        fprintf(ofile,"(%p[pc:%p])\n",entry,
                entry->getPC());
        DISPATCH();
      }

    case SENDMSGX:
    case TAILSENDMSGX:
      {
        TaggedRef mn = getLiteralArg(PC+1);
        fprintf(ofile, "(%s %d ", toC(mn), XRegToInt(getXRegArg(PC+2)));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " cache)\n");
        DISPATCH();
      }

    case SENDMSGY:
    case TAILSENDMSGY:
      {
        TaggedRef mn = getLiteralArg(PC+1);
        fprintf(ofile, "(%s %d ", toC(mn), YRegToInt(getYRegArg(PC+2)));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " cache)\n");
        DISPATCH();
      }

    case SENDMSGG:
    case TAILSENDMSGG:
      {
        TaggedRef mn = getLiteralArg(PC+1);
        fprintf(ofile, "(%s %d ", toC(mn), GRegToInt(getGRegArg(PC+2)));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " cache)\n");
        DISPATCH();
      }

    case CALLX:
    case TAILCALLX:
    case CONSCALLX:
    case TAILCONSCALLX:
      {
        XReg reg = XRegToInt(getXRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, getPosIntArg(PC+2));
      }
      DISPATCH();

    case CALLY:
    case CONSCALLY:
      {
        YReg reg = YRegToInt(getYRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, getPosIntArg(PC+2));
      }
      DISPATCH();

    case CALLG:
    case TAILCALLG:
    case CONSCALLG:
    case TAILCONSCALLG:
      {
        GReg reg = GRegToInt(getGRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, getPosIntArg(PC+2));
      }
      DISPATCH();

    case DECONSCALLX:
    case TAILDECONSCALLX:
      {
        XReg reg = XRegToInt(getXRegArg(PC+1));
        fprintf(ofile, "(%d)\n", reg);
      }
      DISPATCH();

    case DECONSCALLY:
      {
        YReg reg = YRegToInt(getXRegArg(PC+1));
        fprintf(ofile, "(%d)\n", reg);
      }
      DISPATCH();

    case DECONSCALLG:
    case TAILDECONSCALLG:
      {
        GReg reg = GRegToInt(getXRegArg(PC+1));
        fprintf(ofile, "(%d)\n", reg);
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
      {
        fprintf(ofile, "(%s %d)\n", toC(getNumberArg(PC+1)),
                XRegToInt(getXRegArg(PC+2)));
      }
      DISPATCH();

    case GETNUMBERY:
      {
        fprintf(ofile, "(%s %d)\n", toC(getNumberArg(PC+1)),
                YRegToInt(getYRegArg(PC+2)));
      }
      DISPATCH();

    case GETNUMBERG:
      {
        fprintf(ofile, "(%s %d)\n", toC(getNumberArg(PC+1)),
                GRegToInt(getGRegArg(PC+2)));
      }
      DISPATCH();

    case MOVEGY:
    case UNIFYVALVARGY:
      {
        GReg reg = GRegToInt(getGRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, YRegToInt(getYRegArg(PC+2)));
      }
      DISPATCH();

    case MOVEGX:
    case UNIFYVALVARGX:
      {
        GReg reg = GRegToInt(getGRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, XRegToInt(getXRegArg(PC+2)));
      }
      DISPATCH();

    case MOVEYX:
    case GETVARVARYX:
    case UNIFYVALVARYX:
      {
        YReg reg = YRegToInt(getYRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, XRegToInt(getXRegArg(PC+2)));
      }
      DISPATCH();

    case UNIFYXG:
      {
        XReg reg = XRegToInt(getXRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, GRegToInt(getGRegArg(PC+2)));
      }
      DISPATCH();

    case MOVEXY:
    case UNIFYXY:
    case GETVARVARXY:
    case UNIFYVALVARXY:
      {
        XReg reg = XRegToInt(getXRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, YRegToInt(getYRegArg(PC+2)));
      }
      DISPATCH();

    case MOVEXX:
    case UNIFYXX:
    case GETVARVARXX:
    case UNIFYVALVARXX:
      {
        XReg reg = XRegToInt(getXRegArg(PC+1));
        fprintf(ofile, "(%d %d)\n", reg, XRegToInt(getXRegArg(PC+2)));
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

        XReg reg;
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
            switch ((*list)[k].getKind()) {
            case K_XReg: fprintf(ofile,"x(%d)",(*list)[k].getIndex()); break;
            case K_YReg: fprintf(ofile,"y(%d)",(*list)[k].getIndex()); break;
            case K_GReg: fprintf(ofile,"g(%d)",(*list)[k].getIndex()); break;
            }
            if (k != size - 1)
              fprintf(ofile, " ");
          }
          fprintf(ofile, "])\n");
        }
      }
      DISPATCH();

    case PUTRECORDX:
    case GETRECORDX:
      {
        TaggedRef literal = getLiteralArg(PC+1);
        fprintf(ofile, "(%s ", toC(literal));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+2);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " %d)\n", XRegToInt(getXRegArg(PC+3)));
      }
      DISPATCH();

    case PUTRECORDY:
    case GETRECORDY:
      {
        TaggedRef literal = getLiteralArg(PC+1);
        fprintf(ofile, "(%s ", toC(literal));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+2);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " %d)\n", YRegToInt(getYRegArg(PC+3)));
      }
      DISPATCH();

    case GETRECORDG:
      {
        TaggedRef literal = getLiteralArg(PC+1);
        fprintf(ofile, "(%s ", toC(literal));
        SRecordArity sra = (SRecordArity) getAdressArg(PC+2);
        if (sraIsTuple(sra))
          fprintf(ofile, "%d", getTupleWidth(sra));
        else
          fprintf(ofile, "%s", toC(sraGetArityList(sra)));
        fprintf(ofile, " %d)\n", GRegToInt(getGRegArg(PC+3)));
      }
      DISPATCH();

    case PUTCONSTANTX:
    case GETLITERALX:
      {
        TaggedRef tagged = getTaggedArg(PC+1);

        fprintf(ofile, "(%s %d)\n", toC(tagged),
                 XRegToInt(getXRegArg(PC+2)));
      }
      DISPATCH();

    case PUTCONSTANTY:
    case GETLITERALY:
      {
        TaggedRef tagged = getTaggedArg(PC+1);

        fprintf(ofile, "(%s %d)\n", toC(tagged),
                 YRegToInt(getYRegArg(PC+2)));
      }
      DISPATCH();

    case GETLITERALG:
      {
        TaggedRef tagged = getTaggedArg(PC+1);

        fprintf(ofile, "(%s %d)\n", toC(tagged),
                 GRegToInt(getGRegArg(PC+2)));
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
    case EXHANDLER:
      fprintf(ofile, "(%p)\n", computeLabelArg(PC,PC+1));
      DISPATCH();

    case MATCHX:
      {
        fprintf(ofile, "(%d ...)\n", XRegToInt(getXRegArg(PC+1)));
      }
      DISPATCH();

    case MATCHY:
      {
        fprintf(ofile, "(%d ...)\n", YRegToInt(getYRegArg(PC+1)));
      }
      DISPATCH();

    case MATCHG:
      {
        fprintf(ofile, "(%d ...)\n", GRegToInt(getGRegArg(PC+1)));
      }
      DISPATCH();

    case LOCKTHREAD:
      {
        ProgramCounter lbl = computeLabelArg(PC,PC+1);
        int n      = XRegToInt(getXRegArg(PC+2));
        fprintf(ofile, "(%p x(%d))\n", lbl, n);
      }
      DISPATCH();

    case CALLMETHOD:
      {
        CallMethodInfo *cmi = (CallMethodInfo*)getAdressArg(PC+1);
        fprintf(ofile, "(%p[ri:%d l:%s", cmi,cmi->regIndex, toC(cmi->mn));
        fprintf(ofile, " a:%s] %d)\n", toC(sraGetArityList(cmi->arity)),
                getPosIntArg(PC+2));
        DISPATCH();
      }

    case CALLCONSTANT:
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

#endif


ProgramCounter
  C_XCONT_Ptr,
  C_DEBUG_CONT_Ptr,
  C_CALL_CONT_Ptr,
  C_LOCK_Ptr,
  C_SET_SELF_Ptr,
  C_SET_ABSTR_Ptr,
  C_CATCH_Ptr,
  C_EMPTY_STACK;


CodeArea * CodeArea::skipInGC = NULL;

CodeArea::~CodeArea(void)
{
  Assert(this != CodeArea::skipInGC);
  // Find dynamically allocate data structures that must be deallocated
  ProgramCounter PC = getStart();

  while (OK) {
    Opcode op = getOpcode(PC);
    DebugCode(ProgramCounter prevPC = PC;);

    //
    switch (op) {
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
      PC += 1;
      break;

    case ENDOFFILE:
      Assert(PC+1 == getStart() + size);
      goto finish;

    case DEFINITIONCOPY:
    case DEFINITION:
      // PrTabEntry"s and AbstractionEntry"s are (supposed to be)
      // reclaimed by the GC directly.
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
    case DECONSCALLX:
    case DECONSCALLY:
    case DECONSCALLG:
    case TAILDECONSCALLX:
    case TAILDECONSCALLG:
      PC += 2;
      break;
    case INLINEMINUS1:
    case INLINEPLUS1:
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
    case CONSCALLX:
    case CONSCALLY:
    case CONSCALLG:
    case TAILCONSCALLX:
    case TAILCONSCALLG:
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
      // Records are in heap;
      // TODO: record arity"s are not GC"ed?
      PC += 4;
      break;

    case CALLCONSTANT:
    case GETLITERALG:
    case GETLITERALY:
    case GETLITERALX:
    case PUTCONSTANTY:
    case PUTCONSTANTX:
      // tagged in heap;
      PC += 3;
      break;

    case LOCALVARNAME:
    case GLOBALVARNAME:
    case UNIFYLITERAL:
    case SETCONSTANT:
      // tagged in heap;
      PC += 2;
      break;

    case SETPROCEDUREREF:
      // AbstractionEntry"s are (supposed to be) reclaimed by the GC
      // directly.
      PC += 2;
      break;

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
      delete ((CallMethodInfo*) getAdressArg(PC+1));
      PC += 3;
      break;

    case FASTTAILCALL:
    case FASTCALL:
    case CALLPROCEDUREREF:
      // AbstractionEntry"s are (supposed to be) reclaimed by the GC
      // directly.
      PC += 3;
      break;

    case TAILSENDMSGG:
    case TAILSENDMSGY:
    case TAILSENDMSGX:
    case SENDMSGG:
    case SENDMSGY:
    case SENDMSGX:
      // Tagged is in the heap;
      // TODO: record arity"s are not GC"ed?
      // caches are ignored;
      PC += 6;
      break;

    case INLINEASSIGN:
    case INLINEAT:
      // Tagged is in the heap;
      // caches are ignored;
      PC += 5;
      break;

    case TESTLITERALG:
    case TESTLITERALY:
    case TESTLITERALX:
      // Tagged is in the heap;
      PC += 4;
      break;

    case TESTRECORDG:
    case TESTRECORDY:
    case TESTRECORDX:
      // Tagged is in the heap;
      // TODO: record arity"s are not GC"ed?
      PC += 5;
      break;

    case MATCHG:
    case MATCHY:
    case MATCHX:
      ((IHashTable *) getAdressArg(PC+2))->deallocate();
      PC += 3;
      break;

    case DEBUGEXIT:
      // Tagged"s are in the heap;
      PC += 5;
      break;

    case DEBUGENTRY:
      // TODO: DbgInfo"s are not GC"ed!
      PC += 5;
      break;

    case INLINEDOT:
      // Tagged is in the heap;
      // caches are ignored;
      PC += 6;
      break;

      // special: contain OZ_Location;
    case TESTBI:
      ((OZ_Location *) getAdressArg(PC+2))->deallocate();
      PC += 4;
      break;
    case CALLBI:
      ((OZ_Location *) getAdressArg(PC+2))->deallocate();
      PC += 3;
      break;

    default:
      Assert(0);
      break;
    }

    Assert(PC == prevPC + sizeOf(op));
  }

 finish:
  // kost@ : activate it if something bizarre is happening;
  //  #ifdef DEBUG_CHECK
  //    memset(getStart(),-1,size*sizeof(ByteCode));
  //  #else
  delete [] getStart();
  //  #endif
}


CodeArea::CodeArea(int sz) {
  allocateBlock(sz);
  referenced = NO;
}

void CodeArea::init(void **instrTable)
{
#ifdef THREADED
  globalInstrTable = instrTable;
#ifndef INLINEOPCODEMAP
  opcodeTable = new AddressHashTable((int) (OZERROR*1.5));
  for (int i=0; i<=OZERROR; i++) {
    opcodeTable->htAdd(globalInstrTable[i], ToPointer(i));
  }
#endif
#endif
  // apparently, the magic number given to the CodeArea constructor
  // is 2*(1+number of task types).  The 1 is for GLOBALVARNAME
  // which is not really a task type.
  CodeArea *code = new CodeArea(20);
  C_XCONT_Ptr        = code->getStart();
  C_DEBUG_CONT_Ptr   = writeOpcode(TASKXCONT,       C_XCONT_Ptr);
  C_CALL_CONT_Ptr    = writeOpcode(TASKDEBUGCONT,   C_DEBUG_CONT_Ptr);
  C_LOCK_Ptr         = writeOpcode(TASKCALLCONT,    C_CALL_CONT_Ptr);
  C_SET_SELF_Ptr     = writeOpcode(TASKLOCK,        C_LOCK_Ptr);
  C_SET_ABSTR_Ptr    = writeOpcode(TASKSETSELF,     C_SET_SELF_Ptr);
  C_CATCH_Ptr        = writeOpcode(TASKPROFILECALL, C_SET_ABSTR_Ptr);
  C_EMPTY_STACK      = writeOpcode(TASKCATCH,       C_CATCH_Ptr);
  ProgramCounter aux = writeOpcode(TASKEMPTYSTACK,  C_EMPTY_STACK);
  /* mark end with GLOBALVARNAME, so definitionEnd works properly */
  (void) writeOpcode(GLOBALVARNAME,aux);
  CodeArea::skipInGC = code;
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


ProgramCounter CodeArea::writeCache(ProgramCounter PC) {
  InlineCache *cache = (InlineCache *) PC;
  PC = writeInt(0, PC);
  PC = writeInt(0, PC);
  cache->invalidate();
  return PC;
}

void CodeArea::allocateBlock(int sz)
{
  size = sz + 1;
  codeBlock  = new ByteCode[size]; /* allocation via malloc! */
  ProgramCounter ptr = codeBlock;
  const ProgramCounter end = codeBlock + size;
  while (ptr < end)
    // kost@ : whatever opcode representation is used:
    ptr = writeOpcode(ENDOFFILE, ptr);
  Assert(getOpcode(codeBlock+sz) == ENDOFFILE);
  wPtr       = codeBlock;
  nextBlock  = allBlocks;
  allBlocks  = this;
}



/*
 * OZ_ID_LOC
 *
 */

#define MAX_LOC_HOPELESS   8
#define MAX_LOC_HASH       61

#define LOC_FP_SHIFT       4
#define LOC_FP_MASK        15

OZ_LocList * OZ_Location::cache[MAX_LOC_HASH];
TaggedRef  * OZ_Location::new_map[NumberOfXRegisters];


OZ_Location * OZ_ID_LOC;

void initOzIdLoc(void) {
  OZ_Location::initCache();
  OZ_Location::initLocation();
  for (int i=NumberOfXRegisters; i--;  )
    OZ_Location::set(i,i);
  OZ_ID_LOC = OZ_Location::getLocation(NumberOfXRegisters);
}

TaggedRef OZ_Location::getArgs(Builtin * bi) {
  TaggedRef out=oz_nil();
  int i;
  for (i=bi->getOutArity(); i--; ) {
    out=oz_cons(oz_newVariable(),out);
  }
  for (i=bi->getInArity(); i--; ) {
    out=oz_cons(getInValue(i),out);
  }
  return out;
}

TaggedRef OZ_Location::getInArgs(Builtin * bi) {
  TaggedRef out=oz_nil();
  for (int i=bi->getInArity(); i--; )
    out=oz_cons(getInValue(i),out);
  return out;
}

void OZ_Location::initCache(void) {
  for (int i=MAX_LOC_HASH; i--; )
    cache[i] = (OZ_LocList *) NULL;
}

OZ_Location * OZ_Location::getLocation(int n) {
  int fp = -1;
  int sfp;

  if (n <= MAX_LOC_HOPELESS) {

    /*
     * Compute finger print:
     *   The finger print must contain the size literally!
     *
     */

    fp = 0;

    int i;
    for (i = n; i--; )
      fp = (fp << 1) + getNewIndex(i);

    sfp = fp % MAX_LOC_HASH;

    fp = (fp << LOC_FP_SHIFT) + n;

    OZ_LocList * ll = cache[sfp];

    while (ll) {

      if ((ll->loc->fingerprint >> LOC_FP_SHIFT) != (fp >> LOC_FP_SHIFT))
        goto next;

      if ((ll->loc->fingerprint & LOC_FP_MASK) < n)
        goto next;

      for (i = n; i--; )
        if (ll->loc->map[i] != new_map[i])
          goto next;

      // Cache hit!
      return ll->loc;

    next:
      ll = ll->next;

    }

  }

  OZ_Location * l = alloc(n);

  l->fingerprint = fp;

  if (fp != -1) {
    cache[sfp] = new OZ_LocList(l,cache[sfp]);
  }

  for (int i = n; i--; )
    l->map[i] = new_map[i];

  return l;

}
