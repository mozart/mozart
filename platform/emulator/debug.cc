/*
 * Hydra Project, DFKI Saarbruecken,
 * Stuhlsatzenhausweg 3, W-66123 Saarbruecken, Phone (+49) 681 302-5312
 * $Id$
 * Author: scheidhr & mehl & lorenz
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

void dbgMessage(char *s)
{
  fprintf(stderr,s);
  fflush(stderr);
}

void dbgPrint(TaggedRef t)
{
  taggedPrint(t,ozconf.printDepth);
}

static Board* gotoRootBoard() {
  Board *b = am.currentBoard;
  am.currentBoard = am.rootBoard;
  return b;
}

static void gotoBoard(Board *b) {
  am.currentBoard = b;
}

void debugStreamSuspend(ProgramCounter PC, Thread *tt,
			TaggedRef name, TaggedRef args, bool builtin) {
  Board *bb = gotoRootBoard();

  TaggedRef tail    = am.threadStreamTail;
  TaggedRef newTail = OZ_newVariable();

  ProgramCounter debugPC = CodeArea::nextDebugInfo(PC);

  TaggedRef file, comment;
  int line, abspos;
  
  if (debugPC == NOCODE) {
    file    = OZ_atom("noDebugInfo");
    comment = OZ_atom("");
    line    = 1;
    abspos  = 1;
  }
  else
    CodeArea::getDebugInfoArgs(debugPC,file,line,abspos,comment);
  
  TaggedRef pairlist = 
    cons(OZ_pairA("thr",
		  OZ_mkTupleC("#",2,makeTaggedConst(tt),
			      OZ_int(tt->getID()))),
	 cons(OZ_pairA("file", file),
	      cons(OZ_pairAI("line", line),
		   cons(OZ_pairA("name", name),
			cons(OZ_pairA("args",args),
			     cons(OZ_pairA("builtin", 
					   builtin ? OZ_true() : OZ_false()),
			     nil()))))));
  
  TaggedRef entry = OZ_recordInit(OZ_atom("susp"), pairlist);
  OZ_unify(tail, OZ_cons(entry, newTail));
  am.threadStreamTail = newTail;
  gotoBoard(bb);
}

void debugStreamCont(Thread *tt) {
  Board *bb = gotoRootBoard();

  TaggedRef tail    = am.threadStreamTail;
  TaggedRef newTail = OZ_newVariable();

  TaggedRef pairlist = 
    cons(OZ_pairA("thr",
		  OZ_mkTupleC("#",2,makeTaggedConst(tt),
			      OZ_int(tt->getID()))),
	 nil());
  
  TaggedRef entry = OZ_recordInit(OZ_atom("cont"), pairlist);
  OZ_unify(tail, OZ_cons(entry, newTail));
  am.threadStreamTail = newTail;
  gotoBoard(bb);
}

void debugStreamThread(Thread *tt, Thread *p) {
  Board *bb = gotoRootBoard();

  TaggedRef tail    = am.threadStreamTail;
  TaggedRef newTail = OZ_newVariable();
  TaggedRef pairlist;

  if (p)
    pairlist = 
      cons(OZ_pairA("thr",
		    OZ_mkTupleC("#",2,makeTaggedConst(tt),
				OZ_int(tt->getID()))),
	   cons(OZ_pairA("par", OZ_mkTupleC("#",2,makeTaggedConst(p),
					    OZ_int(p->getID()))),
		nil()));
  else
    pairlist = 
      cons(OZ_pairA("thr",
		    OZ_mkTupleC("#",2,makeTaggedConst(tt),
				OZ_int(tt->getID()))),
	   nil());
  
  TaggedRef entry = OZ_recordInit(OZ_atom("thr"), pairlist);

  OZ_unify(tail, OZ_cons(entry, newTail));
  am.threadStreamTail = newTail;

  gotoBoard(bb);
}

void debugStreamTerm(Thread *tt) {
  Board *bb = gotoRootBoard();

  TaggedRef tail    = am.threadStreamTail;
  TaggedRef newTail = OZ_newVariable();

  TaggedRef pairlist = 
    cons(OZ_pairA("thr",
		  OZ_mkTupleC("#",2,makeTaggedConst(tt),
			      OZ_int(tt->getID()))),
	 nil());
  
  TaggedRef entry = OZ_recordInit(OZ_atom("term"), pairlist);
  OZ_unify(tail, OZ_cons(entry, newTail));
  am.threadStreamTail = newTail;

  gotoBoard(bb);
}

void debugStreamCall(ProgramCounter PC, char *name, int arity, 
		     TaggedRef *arguments, bool builtin) {

  ProgramCounter debugPC = CodeArea::nextDebugInfo(PC);
  
  if (debugPC != NOCODE) {
    Board *bb = gotoRootBoard();

    TaggedRef tail    = am.threadStreamTail;
    TaggedRef newTail = OZ_newVariable();
  
    TaggedRef file, comment;
    int line, abspos;

    time_t feedtime;
  
    am.currentThread->stop();

    CodeArea::getDebugInfoArgs(debugPC,file,line,abspos,comment);
    TaggedRef arglist = CodeArea::argumentList(arguments, arity);
  
    feedtime = CodeArea::findTimeStamp(debugPC);

    TaggedRef pairlist = 
      cons(OZ_pairA("thr",
		    OZ_mkTupleC("#",2,makeTaggedConst(am.currentThread),
				OZ_int(am.currentThread->getID()))),
	   cons(OZ_pairA("file", file),
		cons(OZ_pairAI("line", line),
		     cons(OZ_pairAA("name", name),
			  cons(OZ_pairA("args", arglist),
			       cons(OZ_pairA("builtin", 
					     builtin ? OZ_true() : OZ_false()),
				    cons(OZ_pairAI("time", feedtime),
				    OZ_nil())))))));
    
    TaggedRef entry = OZ_recordInit(OZ_atom("step"), pairlist);
    OZ_unify(tail, OZ_cons(entry, newTail));
    am.threadStreamTail = newTail;
    gotoBoard(bb);
  }
}

// ------------------ explore a thread's taskstack ---------------------------

OZ_C_proc_begin(BItaskStack,3)
{
  OZ_declareNonvarArg(0,in);
  OZ_declareIntArg(1,depth);
  OZ_declareArg(2,out);

  in = OZ_deref(in);
  if (!isThread(in)) { oz_typeError(0,"Thread"); }

  ConstTerm *rec = tagged2Const(in);
  Thread *thread = (Thread*) rec;

  if (thread->isDeadThread()) {
    return OZ_unify(out, nil());
  }

  if (!thread->hasStack()) {
    return OZ_unify(out, nil());
  }

  TaskStack *taskstack = thread->getTaskStackRef();
  return OZ_unify(out, taskstack->dbgGetTaskStack(NOCODE, depth+1));
}
OZ_C_proc_end

OZ_C_proc_begin(BIlocation,2)
{
  OZ_declareNonvarArg(0,in);
  OZ_declareArg(1,out);

  in = OZ_deref(in);
  if (!isThread(in)) { oz_typeError(0,"Thread"); }

  ConstTerm *rec = tagged2Const(in);
  Thread *thread = (Thread*) rec;

  if (thread->isDeadThread()) {
    return OZ_unify(out, nil());
  }

  return OZ_unify(out, am.dbgGetLoc(thread->getBoard()));
}
OZ_C_proc_end

OZ_C_proc_begin(BIsetStepMode,2)
{
  OZ_Term chunk = deref(OZ_getCArg(0));
  OZ_declareNonvarArg(1, yesno);
  
  ConstTerm *rec = tagged2Const(chunk);
  Thread *thread = (Thread*) rec;
  
  if (OZ_isTrue(yesno))
    thread->startStepMode();
  else if (OZ_isFalse(yesno))
    thread->stopStepMode();
  else warning("setStepMode: invalid argument");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BItraceThread,2)
{
  OZ_Term chunk = deref(OZ_getCArg(0));
  OZ_declareNonvarArg(1, yesno);
  
  ConstTerm *rec = tagged2Const(chunk);
  Thread *thread = (Thread*) rec;
  
  if (OZ_isTrue(yesno))
    thread->traced();
  else if (OZ_isFalse(yesno))
    thread->notTraced();
  else warning("traceThread: invalid argument");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIqueryDebugState,2)
{
  OZ_Term chunk = deref(OZ_getCArg(0));
  OZ_Term out   = OZ_getCArg(1);
  
  ConstTerm *rec = tagged2Const(chunk);
  Thread *thread = (Thread*) rec;

  return oz_unify(out, OZ_mkTupleC("debugState",
				   3,
				   thread->isTraced()  ? oz_atom("Traced")
				                       : oz_atom("notTraced"),
				   thread->stepMode()  ? oz_atom("stepON")
				                       : oz_atom("stepOFF"),
				   thread->stopped()   ? oz_atom("stopped")
				                       : oz_atom("running")
				   ));
			      
}
OZ_C_proc_end



OZ_C_proc_begin(BIspy, 1)
{
  OZ_nonvarArg(0);
  OZ_declareArg(0,predd);
  DEREF(predd,_1,_2);
  if (!isAbstraction(predd)) {
    OZ_warning("spy: abstraction expected, got: %s",toC(predd));
    return FAILED;
  }

  tagged2Abstraction(predd)->getPred()->setSpyFlag();
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

OZ_C_proc_begin(BIbreakpoint, 0)
{
  am.breakflag = OK;
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BInospy, 1)
{
  OZ_nonvarArg(0);
  OZ_declareArg(0,predd);
  DEREF(predd,_1,_2);
  if (!isAbstraction(predd)) {
    OZ_warning("nospy: abstraction expected, got: %s",toC(predd));
    return FAILED;
  }

  tagged2Abstraction(predd)->getPred()->unsetSpyFlag();
  return PROCEED;
}
OZ_C_proc_end



static Bool isSpied(TaggedRef def)
{
  if (isAbstraction(def)) {
    return tagged2Abstraction(def)->getPred()->getSpyFlag();
  }
  return NO;
}

static char *getPrintName(TaggedRef def)
{
  if (isAbstraction(def)) {
    return tagged2Abstraction(def)->getPrintName();
  } else if (isBuiltin(def)) {
    return tagged2Builtin(def)->getPrintName();
  } else {
    return NULL;
  }
}


static void setSpyFlag(TaggedRef def)
{
  if (isAbstraction(def)) {
    tagged2Abstraction(def)->getPred()->setSpyFlag();
  } else {
    fprintf(stderr,"Cannot set spy flag for builtins\n");
  }
}

static void unsetSpyFlag(TaggedRef def)
{
  if (isAbstraction(def)) {
    tagged2Abstraction(def)->getPred()->unsetSpyFlag();
  }
}


char *ternaryInfixes [] = {
  "+", "-", "*", "mod", "div", ".", "/",
  NULL
  };

char *binaryInfixes [] = {
  "<", "=<", ">", ">=", "=",
  NULL
  };


Bool isInTable(TaggedRef def, char **table)
{
  char *pn = getPrintName(def);
  for (char **i=table; *i; i++) {
    if (strcmp(*i,pn) == 0) {
      return OK;
    }
  }

  return NO;
}



/*
 * the machine level debugger starts here
 */

#define MaxLine 100

inline	void printLong(TaggedRef term, int depth) {
  taggedPrintLong(term, depth ? depth : ozconf.printDepth);
}

inline	void printShort(TaggedRef term) {
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
    board = am.currentBoard;
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
      printf("[S:0x%x=");
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
      am.currentThread->printLong(cout,10,0);
      break;
    case 'A':
      am.print();
      break;
    case 'B':
      am.printBoards();
      break;
    case 'D':
      {
	static ProgramCounter from=0;
	static int size=0;
	sscanf(&command[1],"%x %d",&from,&size);
	if (size == 0)
	  size = 10;
	CodeArea::display(from,size);
      }
      break;
    case 'M':
      {
	ProgramCounter from=0;
	int size=0;
	sscanf(&command[1],"%x %d",&from,&size);
	printf("%x:",from);
	for (int i = 0; i < 20; i++)
	  printf(" %d",getWord(from+i));
	printf("\n%x:\n",from+20);
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
	    sscanf(&command[1],"%d %d",&numb);
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
	    printShort(am.xRegs[numb]);
	    printf ("\n");
	  }
	  break;
	case 'X':
	  {
	    int numb=0,depth=0;
	    sscanf(&command[1],"%d %d",&numb,&depth);
	    printf ("X[%d]:\n", numb);
	    printLong(am.xRegs[numb],depth);
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

