/* This may look like C code, but it is really -*- C++ -*-

  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow,mehl,scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

  ------------------------------------------------------------------------
*/

#ifdef __GNUC__
#pragma implementation "emulate.hh"
#endif

#include "../include/config.h"
#include "types.hh"

#include "actor.hh"
#include "alarm.hh"
#include "am.hh"
#include "builtins.hh"
#include "debug.hh"
#include "genvar.hh"
#include "indexing.hh"
#include "io.hh"
#include "objects.hh"
#include "board.hh"
#include "thread.hh"
#include "fdhook.hh"


extern State BIexchangeCellInline(TaggedRef c, TaggedRef out,TaggedRef &in);
extern TaggedRef getSuspHandlerBool(InlineFun2);


// -----------------------------------------------------------------------
// TOPLEVEL FAILURE

#define HANDLE_FAILURE(pc,MSG) { 					      \
  if (e->isToplevel()) { 						      \
    prefixError(); message("*** TOPLEVEL FAILED: "); 			      \
    { MSG; } message("\n"); 						      \
    if (e->conf.stopOnToplevelFailure) { tracerOn(); trace("toplevel failed"); } \
    if (pc) { JUMP(pc); } else { goto LBLpopTask; } 			      \
  } else { goto LBLfailure; } 						      \
}

#define HANDLE_FAILURE1(pc,MSG)						      \
   warning MSG;								      \
   HANDLE_FAILURE(pc,message MSG);


#define SHALLOWFAIL							      \
  if (shallowCP) {							      \
    e->reduceTrailOnFail();						      \
    ProgramCounter nxt = getLabelArg(shallowCP+1);			      \
    shallowCP = NULL;							      \
    JUMP(nxt);								      \
  }


#define CheckArity(arity,arityExp,pred,cont) 				      \
     if (arity != arityExp && VarArity != arity) {			      \
	warning("call: %s/%d with %d args", pred->getPrintName(),	      \
		arity, arityExp);					      \
	HANDLE_FAILURE(cont, ;);					      \
      }


// TOPLEVEL END
// -----------------------------------------------------------------------


#define DoSwitchOnTerm(indexTerm,table)					      \
      TaggedRef term = indexTerm;					      \
      DEREF(term,_1,_2);						      \
									      \
      if (!isLTuple(term)) {						      \
	TaggedRef *sp = sPointer;					      \
	ProgramCounter offset = switchOnTermOutline(term,table,sp);	      \
	sPointer = sp;							      \
	JUMP(offset);							      \
      }									      \
									      \
      ProgramCounter offset = table->listLabel;				      \
      sPointer = tagged2LTuple(term)->getRef();				      \
      JUMP(offset);							      \



static ProgramCounter switchOnTermOutline(TaggedRef term, IHashTable *table,
					  TaggedRef *&sP)
{
  ProgramCounter offset = table->getElse();
  if (isSTuple(term)) {
    if (table->functorTable) {
      Atom *name = tagged2STuple(term)->getLabelAtom();
      int hsh = name ? table->hash(name->hash()) : 0;
      offset = table->functorTable[hsh]
	    ->lookup(name,tagged2STuple(term)->getSize(),offset);
      sP = tagged2STuple(term)->getRef();
    }
    return offset;
  }

  if (isLiteral(term)) {
    if (table->atomTable) {
      int hsh = table->hash(tagged2Atom(term)->hash());
      offset = table->atomTable[hsh]->lookup(tagged2Atom(term),offset);
    }
    return offset;
  }

  if (isNotCVar(term)) {
    return table->varLabel;
  }

  if (isSmallInt(term)) {
    if (table->numberTable) {
      int hsh = table->hash(smallIntHash(term));
      offset = table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isFloat(term)) {
    if (table->numberTable) {
      int hsh = table->hash(tagged2Float(term)->hash());
      offset = table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isBigInt(term)) {
    if (table->numberTable) {
      int hsh = table->hash(tagged2BigInt(term)->hash());
      offset =table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isCVar(term)) {
    return (table->index(tagged2CVar(term),offset));
  }

  return offset;
}

// -----------------------------------------------------------------------
// CALL HOOK


#ifdef OUTLINE
#define inline
#endif

#define GET_CURRENT_PRIORITY() e->currentThread->getPriority()

/* the hook functions return:
     TRUE: must reschedule
     FALSE: can continue
   */

static
Bool emulateHookOutline(AM *e, Abstraction *def=NULL,
			int arity=0,
			TaggedRef *arguments=NULL)
{
  // without signal blocking;
  if (e->isSetSFlag(ThreadSwitch)) {
    if (Thread::QueueIsEmpty()
	|| Thread::GetHead()->getPriority() < e->currentThread->getPriority()){
      e->RestartProcess();
    } else {
      return TRUE;
    }
  }
  if (e->isSetSFlag(StartGC)) {
    return TRUE;
  }

  blockSignals();
  // & with blocking of signals;
  if (e->isSetSFlag(UserAlarm)) {
    Alarm::HandleUser();
  }
  if (e->isSetSFlag(IOReady)) {
    IO::handleIO();
  }
  
  unblockSignals();
  
  if (def && e->isSetSFlag(DebugMode)) {
    enterCall(e->currentBoard,def,arity,arguments);
  }

  return FALSE;
}



inline
Bool hookCheckNeeded(AM *e)
{
#ifdef DEBUG_DET
  Alarm::Handle();   // simulate an alarm
#endif
  
  return (e->isSetSFlag());
}





/* macros are faster ! */
#define emulateHook(e,def,arity,arguments) \
 (!hookCheckNeeded(e) ? FALSE : emulateHookOutline(e, def, arity, arguments))

#define emulateHook0(e) emulateHook(e,NULL,0,NULL)

/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred,gRegs,IsEx,ContAdr,Arity)			      \
									      \
     if (! IsEx) {e->pushTask(CBB,ContAdr,Y,G);}			      \
     G = gRegs;								      \
									      \
     if (emulateHook(e,Pred,Arity,X)) {					      \
	e->pushTaskOutline(CBB,Pred->getPC(),NULL,G,X,Arity);		      \
	goto LBLschedule;						      \
     }

// load a continuation into the machine registers PC,Y,G,X
#define LOADCONT(cont) \
  { \
      Continuation *tmpCont = cont; \
      PC = tmpCont->getPC(); \
      Y = tmpCont->getY(); \
      G = tmpCont->getG(); \
      XSize = tmpCont->getXSize(); \
      tmpCont->getX(X); \
  }

// -----------------------------------------------------------------------
// THREADED CODE

#ifndef THREADED
#define THREADED 0
#endif

#if defined(RECINSTRFETCH) && THREADED > 0
 Error: RECINSTRFETCH requires THREADED == 0;
#endif

#define INCFPC(N) PC += N

#if THREADED > 0

#if THREADED == 2
#   define DODISPATCH goto* (void *)help;
#else
#   define DODISPATCH goto* *(void **)(((char *)instrTable)+help);
#endif

//#define WANT_INSTRPROFILE
#if defined(WANT_INSTRPROFILE) && defined(sparc)
#   define INSTRUCTION(INSTR)   INSTR##LBL: asm(" " #INSTR ":");
#else
#   define INSTRUCTION(INSTR)   INSTR##LBL: 
#endif

// let gcc fill in the delay slot of the "jmp" instruction:
#   define DISPATCH(INC) { int help=*(PC+INC); INCFPC(INC); DODISPATCH; }

#else // THREADED == 0

#   define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#if defined(__GNUC__) && defined(WANT_INSTRPROFILE)
#   define INSTRUCTION(INSTR)   case INSTR: asm(" " #INSTR ":");
#else
#   define INSTRUCTION(INSTR)   case INSTR: 
#endif
#endif //THREADED

#define JUMP(absAdr) PC = absAdr; DISPATCH(0)

#define ONREG1(Label,R1)     HelpReg1 = (R1); goto Label
#define ONREG2(Label,R1,R2)  HelpReg1 = (R1); HelpReg2 = (R2)


#ifdef FASTREGACCESS
#define RegAccess(Reg,Index) (*(RefsArray)((char*)Reg + Index))
#define LessIndex(Index,CodedIndex) (Index <= CodedIndex/sizeof(TaggedRef))
#else
#define RegAccess(Reg,Index) (Reg[Index])
#define LessIndex(I1,I2) (I1<=I2)
#endif

#define Xreg(N) RegAccess(X,N)
#define Yreg(N) RegAccess(Y,N)
#define Greg(N) RegAccess(G,N)

#define XPC(N) Xreg(getRegArg(PC+N))

 
/* install new board, continue only if successful
   opt:
     if already installed do nothing
 */

#define INSTALLPATH(bb)							      \
  if (CBB != bb) {							      \
    switch (e->installPath(bb)) {					      \
    case INST_REJECTED:                                                       \
      currentTaskSusp = NULL;						      \
      goto LBLpopTask;							      \
    case INST_FAILED:							      \
      currentTaskSusp = NULL;						      \
      goto LBLfailure;							      \
    case INST_OK:							      \
      break;								      \
    }									      \
  }



/* define REGOPT if you want the into register optimization for GCC */
#if __GNUC__ >= 2 && defined(sparc) && !defined(DEBUG_CHECK) && defined(REGOPT)
#define Into(Reg) asm(#Reg)
#else
#define Into(Reg)
#endif



// ------------------------------------------------------------------------
// outlined auxiliary functions
// ------------------------------------------------------------------------

static
void suspendOnVar(TaggedRef A, int argsToSave, Board *b, ProgramCounter PC,
		  RefsArray X, RefsArray Y, RefsArray G, int prio)
{
  DEREF(A,APtr,ATag);
  if (isAnyVar(ATag)) {
    Suspension *susp =
      new Suspension(new SuspContinuation(b,prio,PC,Y,G,X,argsToSave));
    b->incSuspCount();
    taggedBecomesSuspVar(APtr)->addSuspension(susp);
  }
}

static
void suspendInlineRel(TaggedRef A, TaggedRef B, int noArgs,
		      OZ_CFun fun, AM *e, ByteCode *shallowCP)
{
  Assert(noArgs==1 || noArgs==2);
  
  static RefsArray X = allocateStaticRefsArray(2);
  
  if (shallowCP) {
    e->trail.pushIfVar(A);
    if (noArgs>1) e->trail.pushIfVar(B);
    return;
  }

  X[0] = A;
  X[1] = B;
  
  OZ_Suspension susp = OZ_makeSuspension(fun,X,noArgs);
  if (OZ_isVariable(A)) OZ_addSuspension(A,susp);
  if (noArgs>1 && OZ_isVariable(B)) OZ_addSuspension(B,susp);
}


static
void suspendInlineFun(TaggedRef A, TaggedRef B, TaggedRef C, TaggedRef &Out,
		      int noArgs,
		      BuiltinTabEntry *entry, AM *e, ByteCode *shallowCP)
{
  static RefsArray X = allocateStaticRefsArray(4);

  TaggedRef newVar = makeTaggedRef(newTaggedUVar(e->currentBoard));
  Out = newVar;
  
  if (shallowCP) {
    e->trail.pushIfVar(A);
    if (noArgs>=3) e->trail.pushIfVar(B);
    if (noArgs>=4) e->trail.pushIfVar(C);
    return;
  }

  int i=0;
  X[i++] = A;
  if (noArgs>=3) X[i++] = B;
  if (noArgs>=4) X[i++] = C;
  X[i] = newVar;
  
  OZ_Suspension susp = OZ_makeSuspension(entry->getFun(), X, noArgs);

  if (OZ_isVariable(A)) OZ_addSuspension(A,susp);

  // special hack for exchangeCell: suspends only on the first argument
  if (noArgs == 3 &&
      OZ_isVariable(B) && (InlineFun2) entry->getInlineFun() != BIexchangeCellInline) {
    OZ_addSuspension(B,susp);
  } else {
    if (noArgs>=3 && OZ_isVariable(B)) OZ_addSuspension(B,susp);
    if (noArgs>=4 && OZ_isVariable(C)) OZ_addSuspension(C,susp);
  }
}


static
TaggedRef makeMethod(int arity, TaggedRef label, TaggedRef *X)
{
  if (arity == 0) {
    return label;
  } else {
    if (arity == 2 && sameLiteral(label,AtomCons)) {
      return makeTaggedLTuple(new LTuple(X[3],X[4]));
    } else {
      STuple *tuple = STuple::newSTuple(label,arity);
      for (int i = arity-1;i >= 0; i--) {
	tuple->setArg(i,X[i+3]);
      }
      return makeTaggedSTuple(tuple);
    }
  }
}

static
TaggedRef createNamedVariable(int regIndex, TaggedRef name, AM *e)
{
  int size = getRefsArraySize(e->toplevelVars);
  if (LessIndex(size,regIndex)) {
    int newSize = int(size*1.5);
    message("resizing store for toplevel vars from %d to %d\n",size,newSize);
    e->toplevelVars = resize(e->toplevelVars,newSize);
    // no deletion of old array --> GC does it
  }
  SVariable *svar = new SVariable(e->currentBoard, name);
  return makeTaggedRef(newTaggedSVar(svar));
}

static
STuple *newSTupleOutline(TaggedRef atom, int arity)
{
  return STuple::newSTuple(atom,arity);
}

static
SRecord *newSRecordOutline(Arity *ff, TaggedRef label)
{
  return new SRecord(ff,label,-1,NO);
}

void engine() {
  
// ------------------------------------------------------------------------
// *** Global Variables
// ------------------------------------------------------------------------

  register ProgramCounter PC   Into(i0) = 0;
  register TaggedRef *sPointer Into(i1) = NULL;
  register AMModus mode        Into(i2);
  register AM *e               Into(i3) = &am;
  register RefsArray X         Into(i4) = e->xRegs;
  register RefsArray Y         Into(i5) = NULL;
  register RefsArray G         Into(l0) = NULL;

  int XSize = 0;

  Bool isExecute = NO;
  Suspension* &currentTaskSusp = FDcurrentTaskSusp;
  AWActor *CAA = NULL;
  Board *tmpBB = NULL;

# define CBB (e->currentBoard)

  RefsArray HelpReg1 = NULL, HelpReg2 = NULL;
  OZ_CFun biFun = NULL;

// shallow choice pointer
  ByteCode *shallowCP = NULL;

  SRecord *predicate;
  int predArity;

#if THREADED > 0
#  include "instrTable.hh"
#if THREADED == 2
  CodeArea::globalInstrTable = instrTable;
#endif
#else
  Opcode op = (Opcode) -1;
#endif

  switch (setjmp(IO::engineEnvironment)) {

  case NOEXCEPTION:
    break;
    
  case SEGVIO:
    message("Trying to catch segmentation violation...\n");
    e->currentTaskStack->makeEmpty();
    break;
  case BUSERROR:
    message("Trying to catch bus error...\n");
    e->currentTaskStack->makeEmpty();
    break;
  }
  
  goto LBLstart;

// ------------------------------------------------------------------------
// *** RUN: Main Loop
// ------------------------------------------------------------------------
 LBLschedule:

  e->currentThread->schedule();
  e->currentThread=(Thread *) NULL;

 LBLerror:
 LBLstart:

// ------------------------------------------------------------------------
// *** gc
// ------------------------------------------------------------------------
  if (e->isSetSFlag(StartGC)) {
    e->doGC();
  }

// ------------------------------------------------------------------------
// *** process switch
// ------------------------------------------------------------------------
  if (Thread::QueueIsEmpty()) {
    IO::suspendEngine();
  }

  e->currentThread = Thread::GetFirst();

  if (e->currentThread->isNormal()) {
    e->currentTaskStack = e->currentThread->getTaskStack();
  } else {
    e->currentTaskStack = (TaskStack *) NULL;
  }

  DebugTrace(trace("thread switched"));

  e->RestartProcess();


// ------------------------------------------------------------------------
// *** pop a task
// ------------------------------------------------------------------------
 LBLpopTask:
  {
    DebugCheckT(Board *fsb);
    if (emulateHook0(e)) {
      goto LBLschedule;
    }

    DebugCheckT(CAA = NULL);

    // POPTASK
    if (!e->currentTaskStack) {
      {
	Thread *c = e->currentThread;
	if (c->isNervous()) {
	  tmpBB = c->popBoard()->getBoardDeref();
	  if (!tmpBB) {
	    goto LBLTaskEmpty;
	  }
	  DebugCheck (((fsb = tmpBB->getSolveBoard ()) != NULL &&
		      fsb->isReflected () == OK),
		      error ("activity under reduced solve actor"));
	  goto LBLTaskNervous;
	}
	if (c->isSuspCont()) {
	  SuspContinuation *cont = c->popSuspCont();
	  tmpBB = cont->getNode()->getBoardDeref();
	  if (!tmpBB) {
	    goto LBLTaskEmpty;
	  }
	  PC = cont->getPC();
	  Y = cont->getY();
	  G = cont->getG();
	  XSize = cont->getXSize();
	  cont->getX(X);
	  DebugCheck (((fsb = tmpBB->getSolveBoard ()) != NULL &&
		      fsb->isReflected () == OK),
		      error ("activity under reduced solve actor"));
	  disposeRefsArray(cont->getX());
	  cont->dispose();
	  goto LBLTaskCont;
	}	  
	if (c->isSuspCCont()) {
	  CFuncContinuation *ccont = c->popSuspCCont();
	  tmpBB = ccont->getNode()->getBoardDeref();
	  if (!tmpBB) {
	    goto LBLTaskEmpty;
	  }
	  biFun = ccont->getCFunc();
	  currentTaskSusp = c->getResSusp();
	  XSize = ccont->getXSize();
	  ccont->getX(X);
	  DebugCheck (((fsb = tmpBB->getSolveBoard ()) != NULL &&
		      fsb->isReflected () == OK),
		      error ("activity under reduced solve actor"));
	  goto LBLTaskCFuncCont;
	}
	Assert(c->isNormal() && (!c->u.taskStack || c->u.taskStack->isEmpty()));
	goto LBLTaskEmpty;
      }
    } else {
      TaskStack *taskStack = e->currentTaskStack;
      TaskStackEntry *topCache = taskStack->getTop() - 1;
      TaggedBoard tb = (TaggedBoard) TaskStackPop(topCache);
      if (taskStack->isEmpty((TaskStackEntry) tb)) {
	goto LBLTaskEmpty;
      }
      ContFlag cFlag = getContFlag(tb);
      tmpBB = getBoard(tb,cFlag)->getBoardDeref();

      switch (cFlag){
      case C_CONT:
	PC = (ProgramCounter) TaskStackPop(--topCache);
	Y = (RefsArray) TaskStackPop(--topCache);
	G = (RefsArray) TaskStackPop(--topCache);
	taskStack->setTop(topCache);
	if (!tmpBB) {
	  goto LBLpopTask;
	}
	DebugCheck (((fsb = tmpBB->getSolveBoard ()) != NULL &&
		     fsb->isReflected () == OK),
		    error ("activity under reduced solve actor"));
	goto LBLTaskCont;
      case C_XCONT:
	PC = (ProgramCounter) TaskStackPop(--topCache);
	Y = (RefsArray) TaskStackPop(--topCache);
	G = (RefsArray) TaskStackPop(--topCache);
	{
	  RefsArray tmpX = (RefsArray) TaskStackPop(--topCache);
	  XSize = getRefsArraySize(tmpX);
	  int i = XSize;
	  while (--i >= 0) {
	    X[i] = tmpX[i];
	  }
	}
	taskStack->setTop(topCache);
	if (!tmpBB) {
	  goto LBLpopTask;
	}
	DebugCheck (((fsb = tmpBB->getSolveBoard ()) != NULL &&
		     fsb->isReflected () == OK),
		    error ("activity under reduced solve actor"));
	goto LBLTaskCont;

      case C_DEBUG_CONT:
	{
	  OzDebug *ozdeb = (OzDebug *) TaskStackPop(--topCache);
	  taskStack->setTop(topCache);
	  if (!tmpBB) {
	    goto LBLpopTask;
	  }
	  tmpBB->removeSuspension();

	  if (CBB != tmpBB) {
	    switch (e->installPath(tmpBB)) {
	    case INST_REJECTED:
	      exitCall(FAILED,ozdeb);
	      goto LBLpopTask;
	    case INST_FAILED:
	      exitCall(FAILED,ozdeb);
	      goto LBLfailure;
	    case INST_OK:
	      break;
	    }
	  }

	  exitCall(PROCEED,ozdeb);
	  goto LBLcheckEntailment;
	}

      case C_CALL_CONT:
	{
	  predicate = (SRecord *) TaskStackPop(--topCache);
	  RefsArray tmpX = (RefsArray) TaskStackPop(--topCache);
	  predArity = tmpX ? getRefsArraySize(tmpX) : 0;
	  int i = predArity;
	  while (--i >= 0) {
	    X[i] = tmpX[i];
	  }
	  taskStack->setTop(topCache);
	  if (!tmpBB) {
	    goto LBLpopTask;
	  }
	  tmpBB->removeSuspension();
	  isExecute = OK;
	  goto LBLcall;
	}
      case C_NERVOUS:
	// by kost@ : 'SolveActor::Waker' can produce such task
	// (if the search problem is stable by its execution); 
	taskStack->setTop(topCache);
	if (!tmpBB) {
	  goto LBLpopTask;
	}
	DebugCheck (((fsb = tmpBB->getSolveBoard ()) != NULL &&
		     fsb->isReflected () == OK),
		    error ("activity under reduced solve actor"));
	goto LBLTaskNervous;
      case C_CFUNC_CONT:
	// by kost@ : 'solve actors' are represented via the c-function; 
	biFun = (OZ_CFun) TaskStackPop(--topCache);
	currentTaskSusp = (Suspension*) TaskStackPop(--topCache);
	{
	  RefsArray tmpX = (RefsArray) TaskStackPop(--topCache);
	  if (tmpX != NULL) {
	    XSize = getRefsArraySize(tmpX);
	    int i = XSize;
	    while (--i >= 0) {
	      X[i] = tmpX[i];
	    }
	  } else {
	    XSize = 0;
	  }
	}
	taskStack->setTop(topCache);
	if (!tmpBB) {
	  currentTaskSusp = NULL;
	  goto LBLpopTask;
	}
	DebugCheck (((fsb = tmpBB->getSolveBoard ()) != NULL &&
		     fsb->isReflected () == OK),
		    error ("activity under reduced solve actor"));
	goto LBLTaskCFuncCont;
      default:
	error("engine: POPTASK: unexpected task found.");
	goto LBLerror;
      }
    }

  LBLTaskEmpty:
    if (e->currentThread->isSolve () == OK) {
      Board *nb = e->currentThread->getNotificationBoard ();
      e->decSolveThreads (nb);
    }
    e->currentThread->dispose();
    e->currentThread=(Thread *) NULL;
    goto LBLstart;

  LBLTaskNervous:
    // nervous already done ?
    if (!tmpBB->isNervous()) {
      goto LBLpopTask;
    }

    DebugTrace(trace("nervous",tmpBB));

    INSTALLPATH(tmpBB);

    goto LBLcheckEntailment;

  LBLTaskCFuncCont:

    tmpBB->removeSuspension();

    if (currentTaskSusp != NULL && currentTaskSusp->isDead()) {
      currentTaskSusp = NULL;
      goto LBLpopTask;
    }

    INSTALLPATH(tmpBB);

    switch (biFun(XSize, X)) {
    case FAILED:
      killPropagatedCurrentTaskSusp();
      HANDLE_FAILURE(0,
		     message("call of %s(int, TaggedRef[]) failed",
			     builtinTab.getName((void *) biFun));
		     biFun = NULL;
		     for (int i = 0; i < XSize; i++)
		        message("\nArg %d: %s",i+1,tagged2String(X[i]));
		     );
    case PROCEED:
      LOCAL_PROPAGATION(if (localPropStore.isEnabled())
			if (! localPropStore.do_propagation())
			goto LBLfailure);
      killPropagatedCurrentTaskSusp();
      goto LBLcheckEntailment;
    default:
      error("Unexpected return value by c-function.");
      goto LBLerror;
    } // switch


  LBLTaskCont:
    tmpBB->removeSuspension();

    INSTALLPATH(tmpBB);

    goto LBLemulate;
  }

  error("never here");
  goto LBLerror;

// ----------------- end popTask -----------------------------------------

// ------------------------------------------------------------------------
// *** Emulate if no task
// ------------------------------------------------------------------------

 LBLemulateHook:
  if (emulateHook0(e)) {
    e->pushTaskOutline(CBB,PC,Y,G,X,XSize);
    goto LBLschedule;
  }
  goto LBLemulate;

// ------------------------------------------------------------------------
// *** Emulate: execute continuation
// ------------------------------------------------------------------------
 LBLemulate:

#ifdef DEBUGXREGS
// for Debugging only
  for (int i = 0; i < getRefsArraySize(xRegs); i++) {
    xRegs[i] = 0;
  }
#endif

  JUMP( PC );

#if THREADED == 0

 LBLdispatcher:

  DebugCheck(blockSignals() == NO,
	     error("signalmask not zero"));
  DebugCheckT(unblockSignals());
  DebugCheck ((e->currentSolveBoard != CBB->getSolveBoard ()),
	      error ("am.currentSolveBoard and real solve board mismatch"));

  op = CodeArea::getOP(PC);

  Assert(!isFreedRefsArray(Y));

#ifdef RECINSTRFETCH
  CodeArea::recordInstr(PC);
#endif

  DebugTrace( if (!trace("emulate",CBB,CAA,PC,Y,G)) {
		goto LBLfailure;
	      });

  switch (op) {

#endif // THREADED

// the instructions are classified into groups
// to find a certain class of instructions search for the String "CLASS:"
// -------------------------------------------------------------------------
// CLASS: TERM: MOVE/UNIFY/CREATEVAR/...
// -------------------------------------------------------------------------

#include "register.hh"

// ------------------------------------------------------------------------
// CLASS: (Fast-) Call/Execute Inline Funs/Rels
// ------------------------------------------------------------------------

  INSTRUCTION(FASTCALL)
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
      INCFPC(2);

      CallDoChecks(entry->getAbstr(),entry->getGRegs(),NO,PC,
		   entry->getAbstr()->getArity());

      Y = NULL; // allocateL(0);
      // set pc
      IHashTable *table = entry->indexTable;
      if (table) {
	DoSwitchOnTerm(X[0],table);
      } else {
	JUMP(entry->getPC());
      }
    }


  INSTRUCTION(FASTEXECUTE)
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);

      CallDoChecks(entry->getAbstr(),entry->getGRegs(),OK,PC,
		   entry->getAbstr()->getArity());

      Y = NULL; // allocateL(0);
      // set pc
      IHashTable *table = entry->indexTable;
      if (table) {
	DoSwitchOnTerm(X[0],table);
      } else {
	JUMP(entry->getPC());
      }
    }

  INSTRUCTION(CALLBUILTIN)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      OZ_CFun fun = entry->getFun();
      int arityExp = getPosIntArg(PC+2);
      int arity = entry->getArity();

      CheckArity(arity,arityExp,entry,PC+3);

      switch (fun(arity, X)){
      case SUSPEND:
	warning("call builtin: SUSPEND unexpected\n");
	// no break here
      case FAILED:
	killPropagatedCurrentTaskSusp();
	HANDLE_FAILURE(PC+3,
		       message("call: builtin %s/%d failed",
			       entry->getPrintName(),
			       arity);
		       for (int i = 0; i < arity; i++)
		       { message("\nArg %d: %s",i+1,tagged2String(X[i])); }
		       );
      case PROCEED:
	LOCAL_PROPAGATION(if (localPropStore.isEnabled())
			  if (! localPropStore.do_propagation())
			  goto LBLfailure);
	killPropagatedCurrentTaskSusp();
	DISPATCH(3);
      default:
	error("Unexpected return value at CALLBUILTIN.");
	goto LBLerror;
      }
    }


  INSTRUCTION(INLINEREL1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel1 rel         = (InlineRel1)entry->getInlineFun();

      OZ_Bool res = rel(XPC(2));

      switch(res) {

      case PROCEED:
	DISPATCH(3);

      case SUSPEND:
	{
	  suspendInlineRel(XPC(2),makeTaggedNULL(),1,entry->getFun(),e,shallowCP);
	  DISPATCH(3);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+3,
			 message("INLINEREL = {`%s` %s}",
				 entry->getPrintName(),tagged2String(XPC(2))));
	}
      }
    }

  INSTRUCTION(INLINEREL2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel2 rel         = (InlineRel2)entry->getInlineFun();

      OZ_Bool res = rel(XPC(2),XPC(3));

      switch(res) {

      case PROCEED:
	DISPATCH(4);

      case SUSPEND:
	{
	  suspendInlineRel(XPC(2),XPC(3),2,entry->getFun(),e,shallowCP);
	  DISPATCH(4);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+4,
			 message("INLINEREL = {`%s` %s %s}",
				 entry->getPrintName(),
				 tagged2String(XPC(2)),tagged2String(XPC(3))));
	}
      }
    }

  /* bug fixed 14.1.93:
      if inline functions fail on toplevel a new variable has to be stored
      into Out
      */
  INSTRUCTION(INLINEFUN1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun1 fun         = (InlineFun1)entry->getInlineFun();

      OZ_Bool res = fun(XPC(2),XPC(3));

      switch(res) {

      case PROCEED:
	DISPATCH(4);

      case SUSPEND:
	{
	  suspendInlineFun(XPC(2),makeTaggedNULL(),makeTaggedNULL(),XPC(3),2,
			   entry,e,shallowCP);
	  DISPATCH(4);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+4,
			 XPC(3) = makeTaggedRef(newTaggedUVar(CBB));
			 message("INLINEFUN = {`%s` %s}",
				 entry->getPrintName(),tagged2String(XPC(2))));
	}
      }
    }

  INSTRUCTION(INLINEFUN2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun2 fun = (InlineFun2)entry->getInlineFun();

      OZ_Bool res = fun(XPC(2),XPC(3),XPC(4));

      switch(res) {

      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
	suspendInlineFun(XPC(2),XPC(3),makeTaggedNULL(),XPC(4),3,entry,e,shallowCP);
	DISPATCH(5);

      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+5,
			 XPC(4) = makeTaggedRef(newTaggedUVar(CBB));
			 message("INLINEFUN = {`%s` %s %s}",
				 entry->getPrintName(),
				 tagged2String(XPC(2)),
				 tagged2String(XPC(3))));


	}
      }
    }


  INSTRUCTION(INLINEFUN3)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun3 fun = (InlineFun3)entry->getInlineFun();

      OZ_Bool res = fun(XPC(2),XPC(3),XPC(4),XPC(5));

      switch(res) {

      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  suspendInlineFun(XPC(2),XPC(3),XPC(4),XPC(5),4,entry,e,shallowCP);
	  DISPATCH(6);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+6,
			 XPC(5) = makeTaggedRef(newTaggedUVar(CBB));
			 message("INLINEFUN = {`%s` %s %s %s}",
				 entry->getPrintName(),
				 tagged2String(XPC(2)),
				 tagged2String(XPC(3)),
				 tagged2String(XPC(4))));
	}
      }
    }

  INSTRUCTION(INLINEEQEQ)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun2 fun = (InlineFun2)entry->getInlineFun();

      OZ_Bool res = fun(XPC(2),XPC(3),XPC(4));

      switch(res) {

      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  TaggedRef A = XPC(2);
	  TaggedRef B = XPC(3);

	  TaggedRef newVar = makeTaggedRef(newTaggedUVar(CBB));
	  int argsToSave = getPosIntArg(PC+5);

	  XPC(4) = newVar;

	  e->pushTaskOutline(CBB,PC+6,Y,G,X,argsToSave);

	  X[0] = A;
	  X[1] = B;
	  X[2] = newVar;

	  TaggedRef handler = getSuspHandlerBool(fun);
	  if (handler != makeTaggedNULL()) {
	    predicate = tagged2SRecord(handler);
	    isExecute = OK;
	    predArity = 3;
	    goto LBLcall;
	  }
	  
	  HANDLE_FAILURE(PC+6,message("susp handler for ==B, \\=B not set"));
	}

#ifdef DEBUG_CHECK
      case FAILED:
	error("{`%s` %s %s} unexpectedly failed",
	      entry->getPrintName(), tagged2String(XPC(2)), tagged2String(XPC(3)));
	break;
#endif
      }
    }


#undef SHALLOWFAIL

// ------------------------------------------------------------------------
// CLASS: Shallow guards stuff
// ------------------------------------------------------------------------

  INSTRUCTION(SHALLOWGUARD)
    {
      shallowCP = PC;
      e->trail.pushMark();
      DISPATCH(3);
    }

  INSTRUCTION(SHALLOWTEST1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel1 rel         = (InlineRel1)entry->getInlineFun();

      OZ_Bool res = rel(XPC(2));

      switch(res) {

      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
        {
	  suspendOnVar(XPC(2),getPosIntArg(PC+4),
		       CBB,PC,X,Y,G,GET_CURRENT_PRIORITY());
	  goto LBLcheckEntailment;
	}

      case FAILED:
	JUMP( getLabelArg(PC+3) );
      }
    }

  INSTRUCTION(SHALLOWTEST2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel2 rel         = (InlineRel2)entry->getInlineFun();

      OZ_Bool res = rel(XPC(2),XPC(3));

      switch(res) {

      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  TaggedRef A    = XPC(2);
	  TaggedRef B    = XPC(3);
	  DEREF(A,APtr,ATag); DEREF(B,BPtr,BTag);
	  int argsToSave    = getPosIntArg(PC+5);
	  Suspension *susp  =
	    new Suspension(new SuspContinuation(CBB,
						GET_CURRENT_PRIORITY(),
						PC,Y,G,X,argsToSave));
	  CBB->incSuspCount();

	  Assert(isAnyVar(ATag) || isAnyVar(BTag));

	  if (isAnyVar(ATag)) {
	    taggedBecomesSuspVar(APtr)->addSuspension(susp);
	  }
	  if (isAnyVar(BTag)) {
	    taggedBecomesSuspVar(BPtr)->addSuspension(susp);
	  }
	  goto LBLcheckEntailment;
	}
      case FAILED:
	JUMP( getLabelArg(PC+4) );

      }
    }

  INSTRUCTION(SHALLOWTHEN)
    {
      int numbOfCons = e->trail.chunkSize();

      if (numbOfCons == 0) {
	e->trail.popMark();
	shallowCP = NULL;
	DISPATCH(1);
      }

      int argsToSave = getPosIntArg(shallowCP+2);
      Suspension *susp = 
        new Suspension(new SuspContinuation(CBB,
                                            GET_CURRENT_PRIORITY(),
                                            shallowCP, Y, G, X, argsToSave));

      CBB->incSuspCount();
      e->reduceTrailOnShallow(susp,numbOfCons);
      shallowCP = NULL;
      goto LBLcheckEntailment;
    }


// -------------------------------------------------------------------------
// CLASS: Environment
// -------------------------------------------------------------------------

  INSTRUCTION(ALLOCATEL)
    {
      int posInt = getPosIntArg(PC+1);
      Assert(posInt > 0);

      IncfProfCounter(allocateCounter,(posInt+1)*sizeof(TaggedRef));

      Y = allocateY(posInt);
      DISPATCH(2);
    }

  INSTRUCTION(ALLOCATEL1)  { Y =  allocateY(1); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL2)  { Y =  allocateY(2); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL3)  { Y =  allocateY(3); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL4)  { Y =  allocateY(4); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL5)  { Y =  allocateY(5); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL6)  { Y =  allocateY(6); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL7)  { Y =  allocateY(7); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL8)  { Y =  allocateY(8); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL9)  { Y =  allocateY(9); DISPATCH(1); }
  INSTRUCTION(ALLOCATEL10) { Y = allocateY(10); DISPATCH(1); }

  INSTRUCTION(DEALLOCATEL)
    {
      Assert(!isFreedRefsArray(Y));

      if (!isDirtyRefsArray(Y)) {
	IncfProfCounter(deallocateCounter,(getRefsArraySize(Y)+1)*sizeof(TaggedRef));
	deallocateY(Y);
      }

      Y=NULL;
      DISPATCH(1);
    }
// -------------------------------------------------------------------------
// CLASS: CONTROL: FAIL/SUCCESS/RETURN/SAVECONT
// -------------------------------------------------------------------------

  INSTRUCTION(FAILURE)
    {
      HANDLE_FAILURE(PC+1,message("'false'"));
    }


  INSTRUCTION(SUCCEED)
    DISPATCH(1);

  INSTRUCTION(SAVECONT)
    {
      e->pushTask(CBB,getLabelArg(PC+1),Y,G);

      DISPATCH(2);
    }

  INSTRUCTION(RETURN)
  {
    goto LBLcheckEntailment;
  }


// ------------------------------------------------------------------------
// CLASS: Definition
// ------------------------------------------------------------------------

  INSTRUCTION(DEFINITIONX)
    ONREG1(Definition,X);

  INSTRUCTION(DEFINITIONY)
    ONREG1(Definition,Y);

  INSTRUCTION(DEFINITIONG)
    ONREG1(Definition,G);

   Definition:
    {
      Reg reg                     = getRegArg(PC+1);
      ProgramCounter nxt          = getLabelArg(PC+2);
      PrTabEntry *predd           = getPredArg(PC+5);
      AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+6);

      AssRegArray &list = predd->gRegs;
      int size = list.getSize();
      RefsArray gRegs = (size == 0) ? NULL : allocateRefsArray(size);
      
      Abstraction *p = new Abstraction (predd, gRegs, new Name(e->currentBoard));
      TaggedRef term = RegAccess(HelpReg1,reg);
      if (!e->fastUnify(term,makeTaggedSRecord(p))) {
	HANDLE_FAILURE(nxt,
		       message("definition %s/%d = %s",
			       p->getPrintName(),p->getArity(),tagged2String(term)););
      }

      if (predEntry) {
	predEntry->setPred(p);
      }

      for (int i = 0; i < size; i++) {
	switch (list[i].kind) {
	case XReg:
	  gRegs[i] = X[list[i].number];
	  break;
	case YReg:
	  gRegs[i] = Y[list[i].number];
	  break;
	case GReg:
	  gRegs[i] = G[list[i].number];
	  break;
	}
      }
      JUMP(nxt);
    }

// -------------------------------------------------------------------------
// CLASS: CONTROL: FENCE/CALL/EXECUTE/SWITCH/BRANCH
// -------------------------------------------------------------------------
  
  INSTRUCTION(BRANCH)
    JUMP( getLabelArg(PC+1) );

  INSTRUCTION(DETX)
    ONREG1(Det,X);

  INSTRUCTION(DETY)
    ONREG1(Det,Y);

  INSTRUCTION(DETG)
    ONREG1(Det,G);

  Det:
  {
    TaggedRef term = RegAccess(HelpReg1,getRegArg(PC+1));
    DEREF(term,termPtr,tag);

    int argsToSave = getPosIntArg(PC+2);

    if (isAnyVar(tag)) {
      INCFPC(3);
      Suspension *susp =
	new Suspension(new SuspContinuation(CBB,
					    GET_CURRENT_PRIORITY(),
					    PC, Y, G, X, argsToSave));
      CBB->incSuspCount();
      taggedBecomesSuspVar(termPtr)->addSuspension (susp);
      goto LBLcheckEntailment; // mm2 ???
    } else {
      DISPATCH(3);
    }
  };



  INSTRUCTION(EXECUTEMETHODX)
    isExecute = OK;
    ONREG1(SendMethod,X);
  INSTRUCTION(EXECUTEMETHODY)
    isExecute = OK;
    ONREG1(SendMethod,Y);
  INSTRUCTION(EXECUTEMETHODG)
    isExecute = OK;
    ONREG1(SendMethod,G);

  INSTRUCTION(SENDMETHODX)
    isExecute = NO;
    ONREG1(SendMethod,X);
  INSTRUCTION(SENDMETHODY)
    isExecute = NO;
    ONREG1(SendMethod,Y);
  INSTRUCTION(SENDMETHODG)
    isExecute = NO;
    ONREG1(SendMethod,G);

 SendMethod:
  {
    TaggedRef label   = getAtomArg(PC+1);
    TaggedRef origObj = RegAccess(HelpReg1,getRegArg(PC+2));
    TaggedRef object  = origObj;
    int arity         = getPosIntArg(PC+3);

    PC = isExecute ? 0 : PC+4;

    DEREF(object,_1,objectTag);
    if (!isSRecord(objectTag)) {
      if (isAnyVar(objectTag)) {
	X[0] = makeMethod(arity,label,X);
	X[1] = origObj;
	predArity = 2;
	extern TaggedRef suspCallHandler; // mm2
	predicate = tagged2SRecord(suspCallHandler);
	goto LBLcall;
      }

      warning("send method: no abstraction or builtin: %s",tagged2String(object));
      HANDLE_FAILURE(PC,;)
    }

    Abstraction *def = getSendMethod(object,label,arity,X);
    if (def == NULL) {
      goto bombSend;
    }

    CallDoChecks(def,def->getGRegs(),isExecute,PC,arity+3);
    Y = NULL; // allocateL(0);

    JUMP(def->getPC());


  bombSend:
    X[0] = makeMethod(arity,label,X);
    predArity = 1;
    predicate = tagged2SRecord(object);
    goto LBLcall;
  }



  INSTRUCTION(METHEXECUTEX)
    isExecute = OK;
    ONREG1(ApplyMethod,X);
  INSTRUCTION(METHEXECUTEY)
    isExecute = OK;
    ONREG1(ApplyMethod,Y);
  INSTRUCTION(METHEXECUTEG)
    isExecute = OK;
    ONREG1(ApplyMethod,G);

  INSTRUCTION(METHAPPLX)
    isExecute = NO;
    ONREG1(ApplyMethod,X);
  INSTRUCTION(METHAPPLY)
    isExecute = NO;
    ONREG1(ApplyMethod,Y);
  INSTRUCTION(METHAPPLG)
    isExecute = NO;
    ONREG1(ApplyMethod,G);

 ApplyMethod:
  {
    TaggedRef label        = getAtomArg(PC+1);
    TaggedRef origObject   = RegAccess(HelpReg1,getRegArg(PC+2));
    TaggedRef object       = origObject;
    int arity              = getPosIntArg(PC+3);
    Abstraction *def;

    PC = isExecute ? 0 : PC+4;

    DEREF(object,objectPtr,objectTag);
    if (!isSRecord(objectTag) ||
	NULL == (def = getApplyMethod(object,label,arity-3+3,X[0]))) {
      goto bombApply;
    }
    
    CallDoChecks(def,def->getGRegs(),isExecute,PC,arity);
    Y = NULL; // allocateL(0);

    JUMP(def->getPC());


  bombApply:
    if (methApplHdl == makeTaggedNULL()) {
      HANDLE_FAILURE1(PC,("apply method: method Application Handler not set"));
    }

    TaggedRef method = makeMethod(arity-3,label,X);
    X[4] = X[2];   // outState
    X[3] = X[1];   // ooSelf
    X[2] = method;
    X[1] = X[0];   // inState
    X[0] = origObject;

    predArity = 5;
    predicate = tagged2SRecord(methApplHdl);
    goto LBLcall;
  }


  INSTRUCTION(CALLX)
    isExecute = NO;
    ONREG1(Call,X);

  INSTRUCTION(CALLY)
    isExecute = NO;
    ONREG1(Call,Y);

  INSTRUCTION(CALLG)
    isExecute = NO;
    ONREG1(Call,G);

  INSTRUCTION(EXECUTEX)
    isExecute = OK;
    ONREG1(Call,X);

  INSTRUCTION(EXECUTEY)
    isExecute = OK;
    ONREG1(Call,Y);

  INSTRUCTION(EXECUTEG)
    isExecute = OK;
    ONREG1(Call,G);

 Call:

   {
     {
       TaggedRef taggedPredicate = RegAccess(HelpReg1,getRegArg(PC+1));
       predArity = getPosIntArg(PC+2);

       PC = isExecute ? 0 : PC+3;

       DEREF(taggedPredicate,predPtr,predTag);
       if (!isSRecord(predTag)) {
	 if (isAnyVar(predTag)) {
	   X[predArity++] = (TaggedRef) predPtr;
	   extern TaggedRef suspCallHandler; // mm2
	   predicate = tagged2SRecord(suspCallHandler);
	   goto LBLcall;
	 }
	 HANDLE_FAILURE1(PC,("call: no abstraction or builtin: %s",
				  tagged2String(taggedPredicate)));
       }

       predicate = tagged2SRecord(taggedPredicate);
     }

// -----------------------------------------------------------------------
// --- Call: entry point
// -----------------------------------------------------------------------

  LBLcall:
     Builtin *bi;

// -----------------------------------------------------------------------
// --- Call: Abstraction
// -----------------------------------------------------------------------

    TypeOfRecord typ = predicate->getType();

    switch (typ) {
    case R_ABSTRACTION:
    case R_OBJECT:
      {
	Abstraction *def = (Abstraction *) predicate;

        CheckArity(def->getArity(), predArity, def, PC);
	CallDoChecks(def,def->getGRegs(),isExecute,PC,def->getArity());
	Y = NULL; // allocateL(0);

	JUMP(def->getPC());
      }


// -----------------------------------------------------------------------
// --- Call: Builtin
// -----------------------------------------------------------------------
    case R_BUILTIN:
      {
	bi = (Builtin *) predicate;

	CheckArity(bi->getArity(),predArity,bi,PC);

	switch (bi->getType()) {

	case BIsolve:
	  goto LBLBIsolve;

	case BIsolveCont:
	  goto LBLBIsolveCont;

	case BIsolved:
	  goto LBLBIsolved;

	case BIDefault:
	  {
	    if (e->isSetSFlag(DebugMode)) {
	      enterCall(e->currentBoard,bi,predArity,X);
	    }
	    OZ_Bool res = bi->getFun()(predArity, X);
	    if (e->isSetSFlag(DebugMode)) {
	      exitBuiltin(res,bi,predArity,X);
	    }

	    killPropagatedCurrentTaskSusp();

	    switch (res) {
	    
	    case SUSPEND:
	      predicate = bi->getSuspHandler();
	      if (!predicate) {
		HANDLE_FAILURE1(PC,
				("call: builtin %s/%d: no suspension handler",
				 bi->getPrintName(),
				 bi->getArity()));
		
	      }
	      goto LBLcall;
	    case FAILED:
	      HANDLE_FAILURE(PC,
			     message("call: builtin %s/%d failed",
				     bi->getPrintName(),bi->getArity());
			     for (int i = 0; i < predArity; i++)
			     { message("\nArg %d: %s",i+1,tagged2String(X[i])); }
			     );
	    case PROCEED:
	      LOCAL_PROPAGATION(if (localPropStore.isEnabled())
				if (! localPropStore.do_propagation())
				goto LBLfailure);
	      if (emulateHook0(e)) {
		if (!isExecute) {
		  e->pushTaskOutline(CBB,PC,Y,G);
		}
		goto LBLschedule;
	      }
	      if (isExecute) {
		goto LBLcheckEntailment;
	      }
	      JUMP(PC);
	    default:
	      error("builtin: bad return value");
	      goto LBLerror;
	    }
	  }
	default:
	  break;
	}
	error("emulate: call: builtin %s not correctly specified",
	      bi->getPrintName());
	goto LBLerror;
      } // end builtin
    default:
      HANDLE_FAILURE1(PC,("call: no abstraction or builtin: %s",
			       tagged2String(makeTaggedSRecord(predicate))));
    } // end switch on type of predicate

// ------------------------------------------------------------------------
// --- Call: Builtin: solve
// ------------------------------------------------------------------------

   LBLBIsolve:
     {
       TaggedRef x0 = X[0];
       DEREF (x0, _0, x0Tag);

       if (isAnyVar (x0Tag) == OK) {
	 predicate = bi->getSuspHandler();
	 if (!predicate) {
	   warning("call: builtin %s/%d: no suspension handler",
		   bi->getPrintName(),
		   bi->getArity());
	   HANDLE_FAILURE(PC, message(""));
	 }
	 goto LBLcall;
       }

       if (isSRecord (x0Tag) == NO ||
	   !(tagged2SRecord (x0)->getType () == R_ABSTRACTION ||
	     tagged2SRecord (x0)->getType () == R_BUILTIN)) {
	 warning("call: no abstraction or builtin in solve");
	 HANDLE_FAILURE (PC, message(""));
       }

       // put continuation if any;
       if (isExecute == NO)
	 e->pushTaskOutline(CBB, PC, Y, G);

       // create solve actor(x1);
       // Note: don't perform any derefencing on X[1];
       SolveActor *sa = new SolveActor (CBB, GET_CURRENT_PRIORITY(), X[1]);

       e->setCurrent(new Board(sa, Bo_Solve), OK);
       CBB->setInstalled();
       e->trail.pushMark();
       sa->setSolveBoard(CBB);

       // put ~'solve actor';
       // Note that CBB is already the 'solve' board; 
       e->pushCFun(CBB, SolveActor::Waker);    // no args;
       
       // apply the predicate;
       predArity = 1;
       predicate = (Abstraction *) tagValueOf (x0);
       X[0] = makeTaggedRef (sa->getSolveVarRef ());
       isExecute = OK;
       goto LBLcall;   // spare a task - call the predicate directly; 
     }

// ------------------------------------------------------------------------
// --- Call: Builtin: solveCont
// ------------------------------------------------------------------------

   LBLBIsolveCont:
     {
       if (((OneCallBuiltin *)bi)->isSeen () == OK) {
	 HANDLE_FAILURE(PC, message("not first call of solve continuation"));
       }

       ((OneCallBuiltin *) bi)->hasSeen ();
       Board *solveBB =
	 (Board *) tagValueOf ((((OneCallBuiltin *) bi)->getGRegs ())[0]);
       DebugCheck ((solveBB->isSolve () == NO),
		   error ("no 'solve' blackboard  in solve continuation builtin"));
       DebugCheck((solveBB->isCommitted () == OK ||
		   solveBB->isDiscarded () == OK ||
		   solveBB->isFailed () == OK), 
		  error ("Solve board in solve continuation builtin is gone"));
       SolveActor *solveAA = SolveActor::Cast (solveBB->getActor ()); 

       // link and commit the board (actor will be committed too); 
       solveBB->setCommitted (CBB);
       CBB->incSuspCount (solveBB->getSuspCount ()); // similar to the 'unit commit'
       DebugCheck (((solveBB->getScriptRef ()).getSize () != 0),
		   error ("non-empty script in solve blackboard"));

       // adjoin the list of or-actors to the list in actual solve actor!!!
       Board *currentSolveBB = e->currentSolveBoard;
       if (currentSolveBB == (Board *) NULL) {
	 DebugCheckT (message ("solveCont is applied not inside of a search problem?\n"));
       } else {
	 SolveActor::Cast (currentSolveBB->getActor ())->pushWaitActorsStackOf (solveAA);
       }

       // install (i.e. perform 'unit commmit') 'board-to-install' if any;
       Board *boardToInstall = solveAA->getBoardToInstall ();
       if (boardToInstall != (Board *) NULL) {
	 DebugCheck ((boardToInstall->isCommitted () == OK ||
		      boardToInstall->isFailed () == OK ||
		      boardToInstall->isDiscarded () == OK),
		     error ("boardToInstall is already committed")); 
	 boardToInstall->setCommitted (CBB);
#ifdef DEBUG_CHECK
	 if ( !e->installScript (boardToInstall->getScriptRef ()) ) {
	   // error ("installScript has failed in solveCont");
	   message ("installScript has failed in solveCont (0x%x to 0x%x)\n",
		    (void *) boardToInstall, (void *) solveBB);
	 }
#else
	 (void) e->installScript (boardToInstall->getScriptRef ());
#endif
	 // add the suspensions of the committed board and remove
	 // its suspension itself;
	 CBB->incSuspCount (boardToInstall->getSuspCount () - 1); 
	 // get continuation of 'board-to-install' if any;
	 if (boardToInstall->isWaitTop () == NO) {
	   if (isExecute == NO)
	     e->pushTask(CBB,PC,Y,G);
	   else
	     isExecute = NO;
	   LOADCONT(boardToInstall->getBodyPtr ());
	 }
       }
       // NB: 
       //    Every 'copyTree' call should compress a path in a computation space, 
       //    and not only for copy of, but for an original subtree too. 
       //    Otherwise we would get a long chain of already-commited blackboards 
       //    of former solve-actors; 
       // NB2:
       //    CBB can not become reducible after the applying of solveCont,
       //    since its childCount can not become smaller. 
       if ( !e->fastUnify (solveAA->getSolveVar (), X[0]) ) {
	 warning ("unification of variable in solveCont failed");
	 HANDLE_FAILURE (NULL, ;);
       }

       if (isExecute) {
	 goto LBLcheckEntailment;
       }
       goto LBLemulate;
     }

// ------------------------------------------------------------------------
// --- Call: Builtin: solved
// ------------------------------------------------------------------------

   LBLBIsolved:
     {
       TaggedRef valueIn = (((SolvedBuiltin *) bi)->getGRegs ())[0]; 
       DebugCheck ((isRef (valueIn) == OK), 
		   error ("reference is found in 'solved' builtin"));

       if (tagTypeOf (valueIn) == CONST) {
	 Board *solveBB =
	   (Board *) tagValueOf ((((SolvedBuiltin *) bi)->getGRegs ())[0]);
	 DebugCheck ((solveBB->isSolve () == NO),
		     error ("no 'solve' blackboard  in solve continuation builtin"));
	 DebugCheck((solveBB->isCommitted () == OK ||
		     solveBB->isDiscarded () == OK ||
		     solveBB->isFailed () == OK), 
		    error ("Solve board in solve continuation builtin is gone"));

	 SolveActor::Cast (solveBB->getActor ())->setBoard (CBB);

	 Bool isGround;
	 Board *newSolveBB = (Board *) e->copyTree (solveBB, &isGround);
	 SolveActor *solveAA = SolveActor::Cast (newSolveBB->getActor ());

	 if (isGround == OK) {
	   TaggedRef solveTRef = solveAA->getSolveVar ();  // copy of; 
	   DEREF(solveTRef, _0, _1);
	   (((SolvedBuiltin *) bi)->getGRegs ())[0] = solveTRef; 
	   goto LBLBIsolved;
	 }

	 // link and commit the board;
	 newSolveBB->setCommitted (CBB);
	 CBB->incSuspCount (newSolveBB->getSuspCount ());
	 // similar to the 'unit commit'
	 DebugCheck (((newSolveBB->getScriptRef ()).getSize () != 0),
		     error ("non-empty script in solve blackboard"));

	 if ( !e->fastUnify (solveAA->getSolveVar (), X[0]) ) {
	   warning ("unification of variable in solved failed");
	   HANDLE_FAILURE (NULL, ;);
	 }
       } else {
	 if ( !e->fastUnify (valueIn, X[0]) ) {
	   warning ("unification of variable in solved failed");
	   HANDLE_FAILURE (NULL, ;);
	 }
       }

       if (isExecute) {
	 goto LBLcheckEntailment;
       }
       JUMP(PC);
     }

   }

// --------------------------------------------------------------------------
// --- end call/execute -----------------------------------------------------
// --------------------------------------------------------------------------

  
  INSTRUCTION(WAIT)
    {
      Assert(CBB->isWait() && !CBB->isCommitted());

      /* unit commit */
      WaitActor *aa = WaitActor::Cast(CBB->getActor());
      if (aa->hasOneChild()) {
	Board *waitBoard = CBB;
	e->reduceTrailOnUnitCommit();
        waitBoard->unsetInstalled();
	e->setCurrent(aa->getBoard()->getBoardDeref());

	waitBoard->setCommitted(CBB);   // by kost@ 4.10.94
	Bool ret = e->installScript(waitBoard->getScriptRef());
	Assert(ret!=NO);
	CBB->incSuspCount(waitBoard->getSuspCount()-1);
	DISPATCH(1);
      }

      // not commitable, suspend
      CBB->setWaiting();
      goto LBLsuspendBoard;
    }

  INSTRUCTION(WAITTOP)
    {

      /* top commit */
      if ( e->entailment() )
	
      LBLtopCommit:
       {
	 e->trail.popMark();

	 tmpBB = CBB;

	 e->setCurrent(CBB->getParentBoard()->getBoardDeref());
	 tmpBB->unsetInstalled();
	 tmpBB->setCommitted(CBB);
	 CBB->removeSuspension();

	 goto LBLcheckEntailment;
       }

      /* unit commit for WAITTOP */
      WaitActor *aa = WaitActor::Cast(CBB->getActor());
      Board *bb = CBB;
      if (aa->hasOneChild()) {
	e->reduceTrailOnUnitCommit();
        bb->unsetInstalled();
	e->setCurrent(aa->getBoard()->getBoardDeref());

	bb->setCommitted(CBB);    // by kost@ 4.10.94
	Bool ret = e->installScript(bb->getScriptRef());
	Assert(ret != NO);
	CBB->incSuspCount(bb->getSuspCount());
	CBB->removeSuspension();
	goto LBLcheckEntailment;
      }

      /* suspend WAITTOP */
      CBB->setWaitTop();
      CBB->setWaiting();
      goto LBLsuspendBoardWaitTop;
    }

  INSTRUCTION(ASK)
    {
      // entailment ?
      if (e->entailment()) {
	e->trail.popMark();
	tmpBB = CBB;
	e->setCurrent(CBB->getParentBoard()->getBoardDeref());
	tmpBB->unsetInstalled();
	tmpBB->setCommitted(CBB);
	CBB->removeSuspension();
	DISPATCH(1);
      }

    LBLsuspendBoard:

      CBB->setBody(PC+1, Y, G,NULL,0);

    LBLsuspendBoardWaitTop:
      markDirtyRefsArray(Y);
      DebugTrace(trace("suspend clause",CBB,CAA));

      CAA = AWActor::Cast (CBB->getActor());

      if (CAA->hasNext()) {

	e->deinstallCurrent();

      LBLexecuteNext:
	DebugTrace(trace("next clause",CBB,CAA));

	LOADCONT(CAA->getNext());

	goto LBLemulate; // no thread switch allowed here (CAA)
      }

      // suspend a actor
      DebugTrace(trace("suspend actor",CBB,CAA));

      goto LBLpopTask;
    }


// -------------------------------------------------------------------------
// CLASS: NODES: CREATE/END
// -------------------------------------------------------------------------

  INSTRUCTION(CREATECOND)
    {
      ProgramCounter elsePC = getLabelArg(PC+1);
      int argsToSave = getPosIntArg(PC+2);

      CAA = new AskActor(CBB, GET_CURRENT_PRIORITY(),
			 elsePC ? elsePC : NOCODE,
			 NOCODE, Y, G, X, argsToSave);

      DISPATCH(3);
    }

  INSTRUCTION(CREATEOR)
    {
      ProgramCounter elsePC = getLabelArg (PC+1);
      int argsToSave = getPosIntArg (PC+2);

      CAA = new WaitActor(CBB, GET_CURRENT_PRIORITY(),
			  NOCODE, Y, G, X, argsToSave);

      DISPATCH(3);
    }

  INSTRUCTION(CREATEENUMOR)
    {
      ProgramCounter elsePC = getLabelArg (PC+1);
      int argsToSave = getPosIntArg (PC+2);

      CAA = new WaitActor(CBB, GET_CURRENT_PRIORITY(),
			  NOCODE, Y, G, X, argsToSave);
      CAA->setDisWait();
      if (e->currentSolveBoard != (Board *) NULL) {
	SolveActor *sa= SolveActor::Cast (e->currentSolveBoard->getActor ());
	sa->pushWaitActor (WaitActor::Cast (CAA));
      }
      DISPATCH(3);
    }

  INSTRUCTION(WAITCLAUSE)
    {
      // create a node
      e->setCurrent(new Board(CAA,Bo_Wait),OK);
      CBB->setInstalled();
      e->trail.pushMark();
      DebugCheckT(CAA=NULL);
      IncfProfCounter(waitCounter,sizeof(Board));
      DISPATCH(1);
    }

  INSTRUCTION(ASKCLAUSE)
    {
      e->setCurrent(new Board(CAA,Bo_Ask),OK);
      CBB->setInstalled();
      e->trail.pushMark();
      DebugCheckT(CAA=NULL);
      IncfProfCounter(askCounter,sizeof(Board));
      DISPATCH(1);
    }


  INSTRUCTION(ELSECLAUSE)
    DISPATCH(1);

  INSTRUCTION(THREAD)
    {
      markDirtyRefsArray(Y);
      ProgramCounter newPC = PC+2;
      ProgramCounter contPC = getLabelArg(PC+1);

      int prio = GET_CURRENT_PRIORITY();
      int defPrio = e->conf.defaultPriority;
      if (prio > defPrio) {
	prio = defPrio;
      }

      Thread *tt = new Thread(prio);
      if (e->currentSolveBoard != (Board *) NULL) {
	e->incSolveThreads (e->currentSolveBoard);
	tt->setNotificationBoard (e->currentSolveBoard);
      }
      IncfProfCounter(procCounter,sizeof(Thread));
      tt->pushTask(CBB,newPC,Y,G);
      tt->schedule();
      JUMP(contPC);
    }


  INSTRUCTION(NEXTCLAUSE)
    {
      CAA->nextClause(getLabelArg(PC+1));
      DISPATCH(2);
    }


  INSTRUCTION(LASTCLAUSE)
    {
      CAA->lastClause();
      DISPATCH(1);
    }

// -------------------------------------------------------------------------
// CLASS: MISC: ERROR/NOOP/default
// -------------------------------------------------------------------------

  INSTRUCTION(ERROR)
    {
      error("Emulate: ERROR command executed");
      goto LBLerror;
    }


  INSTRUCTION(DEBUGINFO)
    {
      TaggedRef filename = getAtomArg(PC+1);
      int line           = smallIntValue(getNumberArg(PC+2));
      int absPos         = smallIntValue(getNumberArg(PC+3));
      DISPATCH(4);
    }

  INSTRUCTION(TESTLABEL1)
  INSTRUCTION(TESTLABEL2)
  INSTRUCTION(TESTLABEL3)
  INSTRUCTION(TESTLABEL4)

  INSTRUCTION(TEST1)
  INSTRUCTION(TEST2)
  INSTRUCTION(TEST3)
  INSTRUCTION(TEST4)

  INSTRUCTION(ENDOFFILE)
  INSTRUCTION(ENDDEFINITION)
    warning("emulate: Unimplemented command");
    goto LBLcheckEntailment;

#if ! defined THREADED || THREADED == 0
  default:
    warning("emulate instruction: default should never happen");
    break;
   } /* switch*/
#endif


// ----------------- end emulate ------------------------------------------


// ------------------------------------------------------------------------
// *** REDUCE Board
// ------------------------------------------------------------------------

 LBLcheckEntailment:
  DebugTrace(trace("reduce board",CBB));

  /* optimize: builtin called on toplevel mm (29.8.94) */
  if (CBB == e->rootBoard) {
    goto LBLpopTask;
  }

  CBB->unsetNervous();

  if (CBB->isAsk()) {
    if ( e->entailment() ) {
      LOADCONT(CBB->getBodyPtr());

      e->trail.popMark();

      tmpBB = CBB;

      e->setCurrent(CBB->getParentBoard()->getBoardDeref());
      tmpBB->unsetInstalled();
      tmpBB->setCommitted(CBB);

      CBB->removeSuspension();

      goto LBLemulateHook;
    }
  } else if (CBB->isWait ()) {
// WAITTTOP
    if (CBB->isWaitTop()) {

// WAITTOP: top commit
      if ( e->entailment() ) {
	goto LBLtopCommit;
      }

      DebugCheck(WaitActor::Cast(CBB->getActor())->hasOneChild(),
		 error("reduce: waittop: can not happen"));

// WAITTOP: no rule
      goto LBLpopTask;
    }

    DebugCheck((CBB->isWaiting () == OK &&
		WaitActor::Cast(CBB->getActor())->hasOneChild()),
	       error("reduce: wait: unit commit can not happen"));
    goto LBLpopTask;

// WAIT: no rule
  }  else if (CBB->isSolve ()) {
    // try to reduce a solve board;
    DebugCheck ((CBB->isReflected () == OK),
		error ("trying to reduce an already reflected solve actor"));
    if (SolveActor::Cast (CBB->getActor ())->isStable () == OK) {
      DebugCheck ((e->trail.isEmptyChunk () == NO),
		  error ("non-empty trail chunk for solve board"));
      // all possible reduction steps require this; 
      e->trail.popMark ();
      CBB->unsetInstalled ();
      SolveActor *solveAA = SolveActor::Cast (CBB->getActor ());
      Board *solveBB = CBB; 
      e->setCurrent ((CBB->getParentBoard ())->getBoardDeref ());
      CBB->removeSuspension ();

      if (solveBB->hasSuspension () == NO) {
	// 'solved';
	// don't unlink the subtree from the computation tree;
	DebugCheckT (solveBB->setReflected ());
	if ( !e->fastUnify (solveAA->getResult (), solveAA->genSolved ()) ) {
	  warning ("unification of solved tuple with variable has failed");
	  HANDLE_FAILURE (NULL, ;);
	}
      } else {
	// 'stabe' (stuck) or enumeration;
	WaitActor *wa = solveAA->getDisWaitActor ();
	if (wa == (WaitActor *) NULL) {
	  // "stuck" (stable without distributing waitActors);
	  // don't unlink the subtree from the computation tree; 
	  DebugCheckT (solveBB->setReflected ());
	  if ( !e->fastUnify (solveAA->getResult (), solveAA->genStuck ()) ) {
	    warning ("unification of solved tuple with variable has failed");
	    HANDLE_FAILURE (NULL, ;);
	  }
	} else {
	  // to enumerate;
	  DebugCheck ((wa->hasOneChild () == OK),
		      error ("wait actor for enumeration with single clause?"));
	  DebugCheck (((WaitActor::Cast (wa))->hasNext () == OK),
		      error ("wait actor for distribution has a continuation"));
	  DebugCheck ((solveBB->hasSuspension () == NO),
		      error ("solve board by the enumertaion without suspensions?"));
	  Board *waitBoard = wa->getChild ();
	  wa->decChilds ();
	  if (wa->hasOneChild () == OK &&
	      (wa->getChildRef ())->isFailureInBody () == OK) {
	    // special optimized case: or E_1 then E_2 [] E_3 then false ro;
	    (void) wa->getChild ();  // remove the last child;
	    wa->decChilds ();
	    DebugCheck((wa->hasNoChilds () == NO),
		       error ("error in the '... [] true then false ro' case"));
	    waitBoard->setActor (wa);
	    ((AWActor *) wa)->addChild (waitBoard);
	    solveAA->setBoardToInstall (waitBoard);
	    DebugCheckT (solveBB->setReflected ());
	    if ( !e->fastUnify (solveAA->getResult (), solveAA->genEnumedFail () )) {
	      warning ("unification of distributed tuple with variable has failed");
	      HANDLE_FAILURE (NULL, ;);
	    }
	  } else {
	    // 'proper' enumeration; 
	    WaitActor *nwa = new WaitActor (wa);
	    solveBB->removeSuspension ();   // since WaitActor::WaitActor adds one; 
	    waitBoard->setActor (nwa);
	    ((AWActor *) nwa)->addChild (waitBoard);
	    wa->unsetBoard ();  // may not copy the actor and rest of boards too;
	    solveAA->setBoardToInstall (waitBoard);

	    //  Now, the following state has been reached:
	    // The waitActor with the 'rest' of actors is unlinked from the 
	    // computation space; instead, a new waitActor (*nwa) is linked to it, 
	    // and all the tree from solve blackboard (*solveBB) will be now copied.
	    // Moreover, the copy has already the 'boardToInstall' setted properly;
	    Board *newSolveBB = e->copyTree (solveBB, (Bool *) NULL);
/*
 *  F*ck!
 *  We are trying here to detect the unit-commit failure by the application of
 * the copy. So, just make new one and try to perform unit commit without
 * propagation ...
#ifdef DEBUG_CHECK
	    Board *tmpSolveBB = e->copyTree (solveBB, (Bool *) NULL);
	    Board *bbti =
	      SolveActor::Cast (tmpSolveBB->getActor ())->getBoardToInstall ();
	    e->dontPropagate = OK;
	    Board *cb = e->currentBoard;
	    tmpSolveBB->setInstalled ();
	    e->setCurrent (tmpSolveBB);
	    if ( !e->installScript (bbti->getScriptRef ()) ) {
	      error ("BANG!!!");
	    }
	    tmpSolveBB->unsetInstalled ();
	    e->setCurrent (cb);
	    e->dontPropagate = NO;
#endif
 */
	    // ... and now set the original waitActor backward;
	    waitBoard->flags |= Bo_Failed;   // this subtree is discarded;
	    wa->setBoard (solveBB);          // original waitActor;
	    // the subtrees (new and old ones) are still linked to the
	    // computation tree; 
	    if (wa->hasOneChild () == OK) {
	      solveAA->setBoardToInstall (wa->getChild ());
	    } else {
	      solveAA->setBoardToInstall ((Board *) NULL);
	      // ... since we have set previously board-to-install for copy;
	      solveAA->pushWaitActor (wa);
	      //  If this waitActor has yet more than one clause, it can be
	      // distributed again ... Moreover, it must be considered first. 
	    }
	    DebugCheckT (solveBB->setReflected ());
	    DebugCheckT (newSolveBB->setReflected ());
	    // ... and now there are two proper branches of search problem;

	    if ( !e->fastUnify (solveAA->getResult (),
				solveAA->genEnumed (newSolveBB) )) {
	      warning ("unification of distributed tuple with variable has failed");
	      HANDLE_FAILURE (NULL, ;);
	    }
	  }
	}
      }
    }
  } else {
    Assert(CBB->isRoot());
  }
  goto LBLpopTask;

// ----------------- end reduce -------------------------------------------

// ------------------------------------------------------------------------
// *** FAILURE
// ------------------------------------------------------------------------
 LBLfailure:
  {
    DebugTrace(trace("fail",CBB));
    Assert(CBB->isInstalled() != NO);
    Actor *aa=CBB->getActor();
    if (aa->isAskWait()) {
      (AWActor::Cast(aa))->failChild(CBB);
    }
    CBB->flags |= Bo_Failed;
    e->reduceTrailOnFail();
    CBB->unsetInstalled();
    e->setCurrent(aa->getBoard()->getBoardDeref());

// ------------------------------------------------------------------------
// *** REDUCE Actor
// ------------------------------------------------------------------------

    DebugTrace(trace("reduce actor",CBB,aa));

    if (aa->isAsk()) {
      if ((AskActor::Cast (aa))->hasNext () == OK) {
	CAA = AskActor::Cast (aa);
	goto LBLexecuteNext;
      }
/* check if else clause must be activated */
      if ( (AskActor::Cast (aa))->isLeaf() ) {

/* rule: if else ... fi
   push the else cont on parent && remove actor */
	aa->setCommitted();
	LOADCONT((AskActor::Cast (aa))->getNext());
	PC = AskActor::Cast(aa)->getElsePC();
	if (PC != NOCODE) {
	  CBB->removeSuspension();
	  goto LBLemulateHook;
	}

/* rule: if fi --> false */
	HANDLE_FAILURE(NULL,message("reducing 'if fi' to 'false'"));
      }
    } else if (aa->isWait ()) {
      if ((WaitActor::Cast (aa))->hasNext () == OK) {
	CAA = WaitActor::Cast (aa);
	goto LBLexecuteNext;
      }
/* rule: or ro (bottom commit) */
      if ((WaitActor::Cast (aa))->hasNoChilds()) {
	aa->setCommitted();
	HANDLE_FAILURE(NULL,
		       message("bottom commit");
		       CBB->removeSuspension();
		       goto LBLcheckEntailment;
		       );
      }
/* rule: or <sigma> ro (unit commit rule) */
      if ((WaitActor::Cast (aa))->hasOneChild()) {
	Board *waitBoard = (WaitActor::Cast (aa))->getChild();
	DebugTrace(trace("reduce actor unit commit",waitBoard,aa));
	if (waitBoard->isWaiting()) {
	  waitBoard->setCommitted(CBB); // do this first !!!
	  if (!e->installScript(waitBoard->getScriptRef())) {
	    HANDLE_FAILURE(NULL,
			   message("unit commit failed");
			   CBB->removeSuspension();
			   goto LBLcheckEntailment;
			   );
	  }

	  /* add the suspension from the committed board
	     remove the suspension for the board itself */
	  CBB->incSuspCount(waitBoard->getSuspCount()-1);

	  /* unit commit & WAITTOP */
	  if (waitBoard->isWaitTop()) {
	    if (!waitBoard->hasSuspension()) {
	      goto LBLcheckEntailment;
	    }

	    /* or guard not completed:  e.g. or task X = 1 end [] false ro */
	    goto LBLpopTask;
	  }

	  /* unit commit & WAIT, e.g. or X = 1 ... then ... [] false ro */
	  LOADCONT(waitBoard->getBodyPtr());
	  Assert(PC != NOCODE);

	  goto LBLemulateHook;
	}
      }
    } else {
      //  Reduce (i.e. with failure in this case) the solve actor;
      //  The solve actor goes simply away, and the 'failed' atom is bound to
      // the result variable; 
      aa->setCommitted();
      CBB->removeSuspension();
      if ( !e->fastUnify (SolveActor::Cast (aa)->getResult (),
			  SolveActor::Cast (aa)->genFailed ()) ) {
	warning ("unification of atom 'failed' with variable has failed");
	HANDLE_FAILURE (NULL, ;);
      }
    }

/* no rule: suspend longer */
    goto LBLpopTask;
  }

// ----------------- end reduce actor --------------------------------------


// ----------------- end failure ------------------------------------------
}

#ifdef OUTLINE
#undef inline
#endif
