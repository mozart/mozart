/*
 *  Authors:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *    Benjamin Lorenz <lorenz@ps.uni-sb.de>
 *    Michael Mehl <mehl@dfki.de>
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *
 *  Copyright:
 *    Benjamin Lorenz <lorenz@ps.uni-sb.de>
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "debug.hh"
#endif

#include <string.h>
#include <signal.h>
#include <setjmp.h>

#include "runtime.hh"
#include "debug.hh"
#include "codearea.hh"

TaggedRef OzDebug::toRecord(const char *label, Thread *thread, int frameId) {
  TaggedRef pairlist = nil();
  if (data != makeTaggedNULL()) {
    pairlist = cons(OZ_pairA("data",data),pairlist);
  }
  if (arguments != (RefsArray) NULL) {
    TaggedRef arglist = nil();
    for(int i = getRefsArraySize(arguments) - 2; i >= 0; i--) {
      if (arguments[i] == makeTaggedNULL())
	arguments[i] = OZ_newVariable();
      arglist = cons(arguments[i],arglist);
    }
    pairlist = cons(OZ_pairA("args",arglist),pairlist);
  }
  if (frameId == -1) {
    pairlist = cons(OZ_pairA("vars",getFrameVariables()),pairlist);
  } else {
    pairlist = cons(OZ_pairAI("frameID",frameId),pairlist);
  }
  int iline = smallIntValue(getNumberArg(PC+2));
  pairlist =
    cons(OZ_pairAI("time",CodeArea::findTimeStamp(PC)),
	 cons(OZ_pairA("thr",makeTaggedConst(thread)),
	      cons(OZ_pairA("file",getTaggedArg(PC+1)),
		   cons(OZ_pairAI("line",iline < 0? -iline: iline),
			cons(OZ_pairA("column",getTaggedArg(PC+3)),
			     cons(OZ_pairA("origin",
					   OZ_atom("OzDebug::toRecord")),
				  cons(OZ_pairAI("PC",(int)PC),
				       cons(OZ_pairA("kind",
						     getTaggedArg(PC+4)),
					    pairlist))))))));

  return OZ_recordInit(OZ_atom(label), pairlist);
}

TaggedRef OzDebug::getFrameVariables() {
  return CodeArea::getFrameVariables(PC,Y,CAP);
}

// ------------------ debug stream messages ---------------------------

void debugStreamBreakpoint(Thread *thread) {
  TaggedRef pairlist =
    cons(OZ_pairA("thr",makeTaggedConst(thread)),nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("breakpoint"), pairlist));
}

void debugStreamBlocked(Thread *thread) {
  TaggedRef pairlist =
    cons(OZ_pairA("thr",makeTaggedConst(thread)),nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("blocked"), pairlist));
}

void debugStreamReady(Thread *thread) {
  TaggedRef pairlist =
    cons(OZ_pairA("thr",makeTaggedConst(thread)),nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("ready"), pairlist));
}

void debugStreamTerm(Thread *thread) {
  TaggedRef pairlist =
    cons(OZ_pairA("thr",makeTaggedConst(thread)),nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("term"), pairlist));
}

void debugStreamException(Thread *thread, TaggedRef exc) {
  am.currentThread()->setStop(OK);

  TaggedRef pairlist =
    cons(OZ_pairA("thr",makeTaggedConst(thread)),
	 cons(OZ_pairA("exc",exc),nil()));
  am.debugStreamMessage(OZ_recordInit(OZ_atom("exception"), pairlist));
}

void debugStreamEntry(OzDebug *dbg, int frameId) {
  am.currentThread()->setStop(OK);
  am.debugStreamMessage(dbg->toRecord("entry",am.currentThread(),frameId));
}

void debugStreamExit(OzDebug *dbg, int frameId) {
  am.currentThread()->setStep(OK);
  am.currentThread()->setStop(OK);
  am.debugStreamMessage(dbg->toRecord("exit",am.currentThread(),frameId));
}

void debugStreamUpdate(Thread *thread) {
  TaggedRef pairlist =
    cons(OZ_pairA("thr",makeTaggedConst(thread)),nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("update"), pairlist));
}

// ------------------ Debugging Builtins ---------------------------

OZ_BI_define(BIdebugmode,0,1)
{
  OZ_RETURN(am.debugmode()? NameTrue: NameFalse);
} OZ_BI_end

OZ_BI_define(BIgetDebugStream,0,1)
{
  OZ_RETURN(am.getDebugStreamTail());
} OZ_BI_end

OZ_BI_define(BIthreadUnleash,2,0)
{
  oz_declareThreadIN(0,thread);
  OZ_declareIntIN(1,frameId);

  if (!thread->isDeadThread() && thread->hasStack())
    thread->getTaskStackRef()->unleash(frameId);

  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIsetStepFlag,2,0)
{
  oz_declareThreadIN(0,thread);
  oz_declareNonvarIN(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setStep(OK);
  else if (OZ_isFalse(yesno))
    thread->setStep(NO);
  else
    oz_typeError(1,"Bool");
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIsetTraceFlag,2,0)
{
  oz_declareThreadIN(0,thread);
  oz_declareNonvarIN(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setTrace(OK);
  else if (OZ_isFalse(yesno))
    thread->setTrace(NO);
  else
    oz_typeError(1,"Bool");
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIcheckStopped,1,1)
{
  oz_declareThreadIN(0,thread);
  OZ_RETURN(thread->getStop() ? NameTrue : NameFalse);
} OZ_BI_end

// ------------------

OZ_BI_define(BIbreakpointAt, 3,1)
{
  OZ_declareIN    (0,file)
  OZ_declareIntIN (1,line);
  OZ_declareIN    (2,what);

  DEREF(file,_1,_2);

  DbgInfo *info = allDbgInfos;
  Bool    ok    = NO;
  char *inFile  = ozstrdup(toC(file));
  char *fullFile, *stripFile;

  while (info) {
    if (line == info->line) {
      fullFile  = toC(info->file);
      stripFile = strrchr(fullFile,'/');
      // the strstr approach might be somewhat too fuzzy...
      if (strstr(inFile,stripFile?stripFile+1:fullFile)) {
	ok = OK;
	// the PC+2 in the next lines is due to the format of the
	// DEBUGENTRY instruction:
	if (OZ_isTrue(what))
	  CodeArea::writeTagged(OZ_int(-line),info->PC+2);
	else
	  CodeArea::writeTagged(OZ_int( line),info->PC+2);
      }
    }
    info = info->next;
  }
  OZ_RETURN(ok? OZ_true(): OZ_false());
} OZ_BI_end

void execBreakpoint(Thread *t) {
  if (!t->getTrace() || !t->getStep()) {
    t->setTrace(OK);
    t->setStep(OK);
    debugStreamBreakpoint(t);
  }
}

OZ_BI_define(BIbreakpoint, 0,0)
{
  if (am.debugmode() && am.onToplevel())
    execBreakpoint(am.currentThread());
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIdisplayDef, 2,0)
{
  OZ_declareIntIN(0,pc);
  OZ_declareIntIN(1,size);
  displayDef((ProgramCounter)ToPointer(pc),size);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIdisplayCode, 2,0)
{
  OZ_declareIntIN(0,pc);
  OZ_declareIntIN(1,size);
  displayCode((ProgramCounter)ToPointer(pc),size);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIprocedureCode, 1,1)
{
  oz_declareNonvarIN(0,proc);
  if (!oz_isProcedure(proc)) {
    oz_typeError(0,"Procedure");
  }
  if (oz_isBuiltin(proc)) {
    oz_typeError(0,"Procedure (no builtin)");
  }

  Abstraction *a=tagged2Abstraction(proc);
  OZ_RETURN_INT(ToInt32(a->getPred()->getPC()));
} OZ_BI_end

OZ_BI_define(BIprocedureCoord, 1,1)
{
  oz_declareNonvarIN(0,proc);
  if (!oz_isProcedure(proc)) {
    oz_typeError(0,"Procedure");
  }
  if (oz_isBuiltin(proc)) {
    oz_typeError(0,"Procedure (no builtin)");
  }
  Abstraction *a=tagged2Abstraction(proc);
  ProgramCounter PC = a->getPred()->getPC();
  ProgramCounter definitionPC = CodeArea::definitionStart(PC);
  if (definitionPC != NOCODE) {
    Reg reg;
    int next;
    TaggedRef file, line, column, predName;
    CodeArea::getDefinitionArgs(definitionPC,reg,next,file,line,column,
				predName);
    TaggedRef pairlist =
      cons(OZ_pairA("file",file),
	   cons(OZ_pairA("line",line),
		cons(OZ_pairAI("PC",ToInt32(definitionPC)),nil())));
    if (column != makeTaggedNULL())
      pairlist = cons(OZ_pairA("column",column),pairlist);
    OZ_RETURN(OZ_recordInit(OZ_atom("def"), pairlist));
  } else   // should never happen
    OZ_RETURN(NameUnit);
} OZ_BI_end

OZ_BI_define(BIlivenessX, 1,1)
{
  OZ_declareIntIN(0,pc);

  OZ_RETURN_INT(CodeArea::livenessX((ProgramCounter)ToPointer(pc),0,0));
} OZ_BI_end


/*----------------------------------------------------------------------
 * the machine level debugger starts here
 */

#ifdef DEBUG_TRACE

#define MaxLine 100

static Bool mode=NO;

void ozd_tracerOn()
{
  mode = OK;
}

void ozd_tracerOff()
{
  mode = NO;
}

Bool ozd_trace(char *s, ProgramCounter PC,RefsArray Y,Abstraction *CAP)
{
  static char command[MaxLine];
  static int skip=0;

  if (!mode) {
    return OK;
  }
  if (PC != NOCODE) {
    displayCode(PC, 1);
  } else {
    am.currentBoard()->print();
  }

  if (skip > 0) {
    skip--;
    return OK;
  }
  while (1) {
    printf("\nBREAK");
    if (am.isSetSFlag()) {
      printf("[S:0x%p=",am.currentBoard());
      if (am.isSetSFlag(ThreadSwitch)) {
	printf("P");
      }
      printf("]");
    }
    printf(" %s> ",s);
    fflush(stdout);
    if (osfgets(command,MaxLine,stdin) == (char *) NULL) {
      printf("read no input\n");
      sprintf(command,"s\n");
    } else if (feof(stdin)) {
      clearerr(stdin);
      printf("exit\n");
      sprintf(command,"e\n");
    }
    if (command[0] == '\n') {
      sprintf(command,"s\n");
    }
    char *c = &command[1];
    while (*c != '\n') c++;
    *c = '\0';

    switch (command[0]) {
    case 'a':
      printAtomTab();
      break;
    case 'b':
      builtinTab.print();
      break;
    case 'c':
      {
	sscanf(&command[1],"%d",&skip);
	if (skip==0) mode = NO;
	return OK;
      }
    case 'd':
      if (PC != NOCODE) {
	displayDef(PC,0);
      }
      break;
    case 'e':
      printf("*** Leaving Oz\n");
      am.exitOz(0);
    case 'f':
      return NO;
    case 'p':
      am.currentBoard()->printLong();
      break;
    case 's':
      mode = OK;
      return OK;
    case 't':
      am.currentThread()->printLong();
      break;
    case 'A':
      ozd_printAM();
      break;
    case 'B':
      ozd_printBoards();
      break;
    case 'D':
      {
	static ProgramCounter from=0;
	static int size=0;
	sscanf(&command[1],"%p %d",&from,&size);
	if (size == 0)
	  size = 10;
	displayCode(from,size);
      }
      break;
    case 'M':
      {
	ProgramCounter from=0;
	int size=0;
	sscanf(&command[1],"%p %d",&from,&size);
	printf("%p:",from);
	for (int i = 0; i < 20; i++)
	  printf(" %d",getWord(from+i));
	printf("\n%p:\n",from+20);
      }
      break;
    case 'T':
      am.printThreads();
      break;

    case '?':
      {
// CC does not like strings with CR
	static char *help[]
	  = {"\nOnline Help:",
	     "^D      = e",
	     "RET     = s",
	     "a       print atom tab",
	     "b       print builtin tab",
	     "c       continue (debug mode off)",
	     "d       display current definition",
	     "e       exit oz",
	     "f       fail",
	     "p       print current board (long)",
	     "s       continue with full debug mode",
	     "t       print current taskstack",
	     "A print class AM",
	     "D %x %d Display code",
	     "M %x %d view Memory <from> <len>",
	     "T       print class Thread",
	     "?       this help",
	     "\nin emulate mode additionally",
	     "d %d    display current line(s) code",
	     "x %d    display X register",
	     "X %d    display X register (long)",
	     "        for y,g also",
	     NULL};
	for (char **s = help; *s; s++) {
	  printf("%s\n",*s);
	}
	break;
      }
    default:
      if (PC != NOCODE) {
	switch (command[0]) {
	case 'd':
	  {
	    int size=0;
	    sscanf(&command[1],"%d",&size);
	    displayCode(PC, size ? size : 10);
	  }
	  break;
	case 'g':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf("G[%d] = ", numb);
	    fflush(stdout);
	    if (CAP) oz_print(CAP->getG(numb));
	    printf ("\n");
	  }
	  break;
	case 'G':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ( "G[%d]:\n", numb);
	    if (CAP) ozd_printLong(CAP->getG(numb));
	    printf ( "\n");
	  }
	  break;
	case 'x':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("X[%d] = ", numb);
	    fflush(stdout);
	    oz_print(am.getX(numb));
	    printf ("\n");
	  }
	  break;
	case 'X':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("X[%d]:\n", numb);
	    ozd_printLong(am.getX(numb));
	  }
	  break;
	case 'y':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("Y[%d] = ", numb);
	    fflush(stdout);
	    if (Y) oz_print(Y[numb]);
	    printf ("\n");
	  }
	  break;
	case 'Y':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("Y[%d]:\n", numb);
	    if (Y) ozd_printLong(Y[numb]);
	  }
	  break;
	default:
	  printf("unknown emulate command '%s'\n",command);
	  printf("help with '?'\n");
	  break;
	}
      } else {
	printf("unknown command '%s'\n",command);
	printf("help with '?'\n");
      }
      break;
    }
  }
}

// mm2: I need this builtin for debugging!
OZ_BI_define(BIhalt, 0,0)
{
  mode=OK;
  return PROCEED;
} OZ_BI_end

#endif
