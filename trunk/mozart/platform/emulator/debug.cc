/*
 *  Authors:
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

/*
 *  famous debug support
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
    for(int i = getRefsArraySize(arguments) - 2; i >= 0; i--)
      arglist = cons(arguments[i],arglist);
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
  return CodeArea::getFrameVariables(PC,Y,G);
}

// ------------------ debug stream messages ---------------------------

void debugStreamThread(Thread *thread, Thread *parent) {
  TaggedRef pairlist =
    cons(OZ_pairA("thr",makeTaggedConst(thread)),nil());
  if (parent != NULL)
    pairlist = cons(OZ_pairA("par",makeTaggedConst(parent)),pairlist);
  am.debugStreamMessage(OZ_recordInit(OZ_atom("thr"), pairlist));
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

OZ_C_proc_begin(BIdebugmode,1)
{
  return OZ_unify(OZ_getCArg(0),am.debugmode()? NameTrue: NameFalse);
}
OZ_C_proc_end

OZ_C_proc_begin(BIaddEmacsThreads,1)
{
  OZ_declareNonvarArg(0,yesno);
  if (OZ_isTrue(yesno))
    ozconf.addEmacsThreads = OK;
  else if (OZ_isFalse(yesno))
    ozconf.addEmacsThreads = NO;
  else
    oz_typeError(0,"Bool");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIaddSubThreads,1)
{
  OZ_declareNonvarArg(0,yesno);
  if (OZ_isTrue(yesno))
    ozconf.addSubThreads = OK;
  else if (OZ_isFalse(yesno))
    ozconf.addSubThreads = NO;
  else
    oz_typeError(0,"Bool");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIgetDebugStream,1)
{
  return OZ_unify(OZ_getCArg(0),am.getDebugStreamTail());
}
OZ_C_proc_end

OZ_C_proc_begin(BIthreadUnleash,2)
{
  oz_declareThreadArg(0,thread);
  OZ_declareIntArg(1,frameId);

  if (!thread->isDeadThread() && thread->hasStack())
    thread->getTaskStackRef()->unleash(frameId);

  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIsetStepFlag,2)
{
  oz_declareThreadArg(0,thread);
  oz_declareNonvarArg(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setStep(OK);
  else if (OZ_isFalse(yesno))
    thread->setStep(NO);
  else
    oz_typeError(1,"Bool");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIsetTraceFlag,2)
{
  oz_declareThreadArg(0,thread);
  oz_declareNonvarArg(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setTrace(OK);
  else if (OZ_isFalse(yesno))
    thread->setTrace(NO);
  else
    oz_typeError(1,"Bool");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIcheckStopped,2)
{
  oz_declareThreadArg(0,thread);
  oz_declareArg(1,out);
  return OZ_unify(out, thread->getStop() ? NameTrue : NameFalse);
}
OZ_C_proc_end

// ------------------

OZ_C_proc_begin(BIbreakpointAt, 4)
{
  OZ_declareArg    (0,file)
  OZ_declareIntArg (1,line);
  OZ_declareArg    (2,what);
  OZ_declareArg    (3,out);

  DEREF(file,_1,_2);
  
  DbgInfo *info = allDbgInfos;
  Bool    ok    = NO;

  while(info) {
    if (!atomcmp(file,info->file))
      if (line == info->line) {
	ok = OK;
	// the PC+2 in the next lines is due to the format of the
	// DEBUGENTRY instruction:
	if (OZ_isTrue(what))
	  CodeArea::writeTagged(OZ_int(-line),info->PC+2);
	else
	  CodeArea::writeTagged(OZ_int( line),info->PC+2);
      }
    info = info->next;
  }
  
  if (ok)
    return OZ_unify(out,OZ_true());
  else
    return OZ_unify(out,OZ_false());
}
OZ_C_proc_end

void execBreakpoint(Thread *t) {
  if (!t->getTrace() || !t->getStep()) {
    t->setTrace(OK);
    t->setStep(OK);
    debugStreamThread(t);
  }
}

OZ_C_proc_begin(BIbreakpoint, 0)
{
  if (am.debugmode())
    execBreakpoint(am.currentThread());
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIdisplayCode, 2)
{
  OZ_declareIntArg(0,pc);
  OZ_declareIntArg(1,size);
  displayCode((ProgramCounter)ToPointer(pc),size);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIprocedureCode, 2)
{
  oz_declareNonvarArg(0,proc);
  oz_declareArg(1,out);
  if (!isProcedure(proc)) {
    oz_typeError(0,"Procedure");
  }
  if (isBuiltin(proc)) {
    oz_typeError(0,"Procedure (no builtin)");
  }

  Abstraction *a=tagged2Abstraction(proc);
  return oz_unifyInt(out,ToInt32(a->getPred()->getPC()));
}
OZ_C_proc_end

OZ_C_proc_begin(BIlivenessX, 2)
{
  OZ_declareIntArg(0,pc);
  oz_declareArg(1,out);

  return oz_unifyInt(out,CodeArea::livenessX((ProgramCounter)ToPointer(pc),0,0));
}
OZ_C_proc_end


/*----------------------------------------------------------------------
 * the machine level debugger starts here
 */

#ifdef DEBUG_TRACE

#define MaxLine 100

inline void printLong(TaggedRef term, int depth) {
  taggedPrintLong(term, depth ? depth : ozconf.printDepth);
}

inline void printShort(TaggedRef term) {
  taggedPrint(term);
}

static Bool mode=NO;

void tracerOn()
{
  mode = OK;
}

void tracerOff()
{
  mode = NO;
}

Bool trace(char *s,Board *board,Actor *actor,
	   ProgramCounter PC,RefsArray Y,RefsArray G)
{
  static char command[MaxLine];

  if (!mode) {
    return OK;
  }
  if (!board) {
    board = am.currentBoard();
  }
  if (PC != NOCODE) {
    CodeArea::display(PC, 1);
  }
  board->printDebug();
  if (actor) {
    printf(" -- ");
    actor->print();
  }
  while (1) {
    printf("\nBREAK");
    if (am.isSetSFlag()) {
      printf("[S:0x%p=",board);
      if (am.isSetSFlag(ThreadSwitch)) {
	printf("P");
      }
      printf("]");
    }
    printf(" %s> ",s);
    fflush(stdout);
    if (osfgets(command,MaxLine,stdin) == (char *) NULL) {
      printf("read no input\n");
      printf("exit\n");
      sprintf(command,"e\n");
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
      mode = NO;
      return OK;
    case 'd':
      if (PC != NOCODE) {
	CodeArea::display(CodeArea::definitionStart(PC),1,stdout);
      }
      break;
    case 'e':
      printf("*** Leaving Oz\n");
      am.exitOz(0);
    case 'f':
      return NO;
    case 'p':
      board->printLongDebug();
      break;
    case 's':
      mode = OK;
      return OK;
    case 't':
      am.currentThread()->printLong(cout,10,0);
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
	CodeArea::display(from,size);
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
	     "A	print class AM",
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
	    CodeArea::display(PC, size ? size : 10);
	  }
	  break;
	case 'g':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("G[%d] = ", numb);
	    printShort(G[numb]);
	    printf ("\n");
	  }
	  break;
	case 'G':
	  {
	    int numb=0,depth=0;
	    sscanf(&command[1],"%d %d",&numb,&depth);
	    printf ( "G[%d]:\n", numb);
	    printLong(G[numb],depth);
	    printf ( "\n");
	  }
	  break;
	case 'x':
	  {
	    int numb=0,depth=0;
	    sscanf(&command[1],"%d %d",&numb,&depth);
	    printf ("X[%d] = ", numb);
	    printShort(am.getX(numb));
	    printf ("\n");
	  }
	  break;
	case 'X':
	  {
	    int numb=0,depth=0;
	    sscanf(&command[1],"%d %d",&numb,&depth);
	    printf ("X[%d]:\n", numb);
	    printLong(am.getX(numb),depth);
	  }
	  break;
	case 'y':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("Y[%d] = ", numb);
	    printShort(Y[numb]);
	    printf ("\n");
	  }
	  break;
	case 'Y':
	  {
	    int numb=0,depth=0;
	    sscanf(&command[1],"%d %d",&numb,&depth);
	    printf ("Y[%d]:\n", numb);
	    printLong(Y[numb],depth);
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
OZ_C_proc_begin(BIhalt, 0)
{
  mode=OK;
  return PROCEED;
}
OZ_C_proc_end

#endif
