/* This may look like C code, but it is really -*- C++ -*-

  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow,mehl,scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

  exported variables/s: no

  exported procedures: for class AM;

  ------------------------------------------------------------------------

  internal static variables: no

  internal procedures: no

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
#include "genvar.hh"
#include "indexing.hh"
#include "objects.hh"
#include "board.hh"
#include "thread.hh"
#include "tracer.hh"

// -----------------------------------------------------------------------
// TOPLEVEL FAILURE

#define HANDLE_FAILURE(pc,MSG) { \
  if (e->isToplevel()) { \
    prefixError(); message("*** TOPLEVEL FAILED: "); \
    { MSG; } message("\n"); \
    DebugTrace(tracerOn(); trace("toplevel failed")); \
    if (pc) { JUMP(pc); } else { goto LBLfindWork; } \
  } else { goto LBLfailure; } \
}
#define HANDLE_FAILURE1(pc,MSG) { \
  if (e->isToplevel()) { \
    prefixError(); message("*** TOPLEVEL FAILED: "); \
    { MSG; } message("\n"); \
    DebugTrace(tracerOn(); trace("toplevel failed")); \
    if (pc) { JUMP(pc); } else { goto LBLfindWork; } \
  } else { MSG; goto LBLfailure; } \
}


#define SHALLOWFAIL							      \
  if (shallowCP) {							      \
    e->reduceTrailOnFail();						      \
    ProgramCounter next = getLabelArg(shallowCP+1);			      \
    shallowCP = NULL;							      \
    JUMP(next);								      \
  }


// TOPLEVEL END
// -----------------------------------------------------------------------

// profiling

#ifdef AM_PROFILE
#   define IncfProfCounter(C,N) am.stat.C += N
#else
#   define IncfProfCounter(C,N)
#endif

// -----------------------------------------------------------------------

#define DoSwitchOnTerm(indexTerm,table)					      \
      TaggedRef term = indexTerm;					      \
      DEREF(term,_1,_2);						      \
									      \
      ProgramCounter offset;						      \
									      \
      if (isLTuple(term)) {						      \
	offset = table->listLabel;					      \
	sPointer = tagged2LTuple(term)->getRef();			      \
	JUMP(offset);							      \
      }									      \
      									      \
      offset = table->getElse();					      \
									      \
      if (isSTuple(term)) {						      \
	if (table->functorTable) {					      \
	  Atom *name = tagged2STuple(term)->getLabelAtom();		      \
	  int hsh = name ? table->hash(name->hash()) : 0;     		      \
	  offset = table->functorTable[hsh]				      \
	    ->lookup(name,tagged2STuple(term)->getSize(),offset);	      \
	  sPointer = tagged2STuple(term)->getRef();			      \
	}								      \
	JUMP(offset);							      \
      }									      \
									      \
      if (isLiteral(term)) {						      \
	if (table->atomTable) {						      \
	  int hsh = table->hash(tagged2Atom(term)->hash());		      \
	  offset = table->atomTable[hsh]->lookup(tagged2Atom(term),offset);   \
	}								      \
	JUMP(offset);							      \
      }									      \
									      \
      if (isNotCVar(term)) { 				              \
	JUMP(table->varLabel);						      \
      } 								      \
									      \
      if (isSmallInt(term)) {						      \
	if (table->numberTable) {					      \
	  int hsh = table->hash(smallIntHash(term));	      		      \
	  offset = table->numberTable[hsh]->lookup(term,offset);	      \
	}								      \
	JUMP(offset);							      \
      }									      \
									      \
      if (isFloat(term)) {						      \
	if (table->numberTable) {					      \
	  int hsh = table->hash(tagged2Float(term)->hash());		      \
	  offset = table->numberTable[hsh]->lookup(term,offset);	      \
	}								      \
									      \
      } 								      \
									      \
      if (isBigInt(term)) {						      \
	if (table->numberTable) {					      \
	  int hsh = table->hash(tagged2BigInt(term)->hash());		      \
	  offset = table->numberTable[hsh]->lookup(term,offset);	      \
	}								      \
      } 								      \
									      \
      if (isCVar(term)) {                                                \
        JUMP(tagged2CVar(term)->index(offset, table));                        \
      }                                                                       \
                                                                              \
      JUMP(offset)


// -----------------------------------------------------------------------
// CALL HOOK


#ifdef OUTLINE
#define inline
#endif

inline HookValue emulateHook()
{
#ifdef DEBUG_DET
  Alarm::Handle();   // simulate an alarm
#endif

  if (am.isSetSFlag()) {
    // without signal blocking;
    if (Thread::CheckSwitch()) {
      return (HOOK_SCHEDULE);
    }
    if (am.isSetSFlag(StartGC)) {
      return (HOOK_SCHEDULE);
    }

    blockSignals();
    // & with blocking of signals;
    if (am.isSetSFlag(UserAlarm)) {
      Alarm::HandleUser();
    }
    if (am.isSetSFlag(IOReady)) {
      am.handleIO();
    }

    unblockSignals();

    return (HOOK_OK);
  } else {
    return (HOOK_OK);
  }
}


inline TaggedRef makeMethod(int arity, Atom *label, TaggedRef *X)
{
  if (arity == 0) {
    return makeTaggedAtom(label);
  } else {
    if (arity == 2 && sameLiteral(makeTaggedAtom(label),AtomCons)) {
      return makeTaggedLTuple(new LTuple(X[3],X[4]));
    } else {
      STuple *tuple = STuple::newSTuple(makeTaggedAtom(label),arity);
      for (int i = arity-1;i >= 0; i--) {
	tuple->setArg(i,X[i+3]);
      }
      return makeTaggedSTuple(tuple);
    }
  }
}

/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred,IsEx,ContAdr,Arity)	                      \
								      \
     if (! IsEx) {e->pushTask(CBB,ContAdr,Y,G);} 		      \
     G = Pred->getGRegs();					      \
       								      \
     switch (emulateHook()) {					      \
     case HOOK_SCHEDULE:					      \
       e->pushTask(CBB,Pred->getPC(),NULL,G,X,Arity); 	   	      \
       goto LBLschedule;					      \
     case HOOK_FIND:						      \
       e->pushTask(CBB,Pred->getPC(),NULL,G,X,Arity);    	      \
       goto LBLfindWorkDir;					      \
     }

// -----------------------------------------------------------------------
// THREADED CODE

/* display code during execution */
//#define DISPLAY_CODE CodeArea::display(PC);
#define DISPLAY_CODE ;

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

#if defined(RECINSTRFETCH) && THREADED > 0
 Error: RECINSTRFETCH requires THREADED == 0;
#endif

#define INCFPC(N) PC += N

#if defined THREADED && THREADED > 0

#if THREADED == 2
#   define DODISPATCH goto* (void *)help;
#else
#   define DODISPATCH goto* *(void **)(((char *)instrTable)+help);
#endif

#if defined(WANT_INSTRPROFILE) && defined(sparc)
#   define INSTRUCTION(INSTR)   INSTR##LBL: asm(" " #INSTR ":");
#else
#   define INSTRUCTION(INSTR)   INSTR##LBL: 
#endif

// let gcc fill in the delay slot of the "jmp" instruction:
#   define DISPATCH(INC) { \
       int help=*(PC+INC);\
       INCFPC(INC); \
       DISPLAY_CODE;\
       DODISPATCH; }

#else THREADED == 0

#   define DISPATCH(INC) \
         INCFPC(INC);\
         DISPLAY_CODE;\
         goto LBLdispatcher

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
#else
#define RegAccess(Reg,Index) (Reg[Index])
#endif

#define Xreg(N) RegAccess(X,N)
#define Yreg(N) RegAccess(Y,N)
#define Greg(N) RegAccess(G,N)



/* install new board, continue only if successful
   opt:
     if already installed do nothing
 */

#define INSTALLPATH(bb)							      \
  if (CBB != bb) {							      \
    switch (e->installPath(bb)) {					      \
    case INST_REJECTED:							      \
      goto LBLfindWork;							      \
    case INST_FAILED:							      \
      goto LBLfailure;							      \
    }									      \
  }
void engine() {
  
// ------------------------------------------------------------------------
// *** Global Variables
// ------------------------------------------------------------------------

/* define REGOPT if you want the into register optimization for GCC */
/*#define REGOPT    /* fuer Fortgeschrittene!  */
#if __GNUC__ >= 2 && defined(sparc) && !defined(DEBUG_CHECK) && defined(REGOPT)
#define Into(Reg) asm(#Reg)
#else
#define Into(Reg)
#endif


  register ProgramCounter PC Into(i0) = 0;

  register TaggedRef *sPointer Into(i1) = NULL;

  register AMModus mode Into(i2);

  register AM *e Into(i3) = &am;
  register RefsArray X Into(i4) = e->xRegs;
  register RefsArray Y Into(i5) = NULL;
  register RefsArray G Into(l0) = NULL;

  int XSize = 0;

  Bool isExecute = NO;
#ifdef DEBUG_CHECK
  Bool isExecutePlus = NO;
  RefsArray GPlus, YPlus;
#endif
  Suspension* &currentTaskSusp = am.currentTaskSusp;
  Actor *CAA = NULL;
  Board *tmpBB = NULL;
  Board *&CBB = Board::Current;

  RefsArray HelpReg1 = NULL, HelpReg2 = NULL;
  
  BIFun biFun = NULL;

// shallow choice pointer
  ByteCode *shallowCP = NULL;

  SRecord *predicate;
  int predArity;
  ProgramCounter contAdr;
#if defined THREADED && THREADED > 0

#  include "instrTable.hh"

#if THREADED == 2
  CodeArea::globalInstrTable = instrTable;
#endif

#else

  Opcode op = (Opcode) -1;

#endif THREADED

  switch (setjmp(engineEnvironment)) {

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

  Thread::ScheduleCurrent();

 LBLerror:
 LBLstart:
  DebugTrace(trace("start"));

// ------------------------------------------------------------------------
// *** gc
// ------------------------------------------------------------------------
  if (e->isSetSFlag(StartGC)) {
    e->doGC();
  }

// ------------------------------------------------------------------------
// *** process switch
// ------------------------------------------------------------------------
  Thread::Start();


// ------------------------------------------------------------------------
// *** FindWork: pop a node
// ------------------------------------------------------------------------
 LBLfindWork:
// third: do work
  {
    switch (emulateHook()) {
    case HOOK_SCHEDULE:
      goto LBLschedule;
    case HOOK_FIND:
      goto LBLfindWorkDir;
    }

 LBLfindWorkDir:

    DebugCheckT(CAA = NULL);

    // POPTASK
    if (!e->currentTaskStack) {
      {
	Thread *c = Thread::GetCurrent();
	if (c->isNervous()) {
	  tmpBB = c->popBoard();
	  goto LBLTaskNervous;
	}
	if (c->isWarm()) {
	  Suspension *susp = c->popSuspension();
	  tmpBB = susp->getNode();
	  SuspContinuation *cont = susp->getCont();
	  if (cont) {
	    PC = cont->getPC();
	    Y = cont->getY();
	    G = cont->getG();
	    XSize = cont->getXSize();
	    cont->getX(X);
	    goto LBLTaskCont;
	  } else {
	    CFuncContinuation *ccont = susp->getCCont();
	    if (ccont) {
	      biFun = ccont->getCFunc();
	      currentTaskSusp = susp;
	      XSize = ccont->getXSize();
	      ccont->getX(X);
	      goto LBLTaskCFuncCont;
	    } else {
	      goto LBLTaskNervous;
	    }
	  }
	}
	DebugCheck(!c->isNormal()||(c->taskStack && !c->taskStack->isEmpty()),
		   error("engine::POPTASK"));
	goto LBLTaskEmpty;
      }
    } else {
      TaskStack *taskStack = e->currentTaskStack;
      TaskStackEntry *topCache = taskStack->getTop() - 1;
      TaskStackEntry e = TaskStackPop(topCache);
      if (taskStack->isEmpty(e)) {
	goto LBLTaskEmpty;
      }
      tmpBB = (Board *) e;
      switch (getContFlag(tmpBB)){
      case C_CONT:
	PC = (ProgramCounter) TaskStackPop(--topCache);
	Y = (RefsArray) TaskStackPop(--topCache);
	G = (RefsArray) TaskStackPop(--topCache);
	taskStack->setTop(topCache);
	goto LBLTaskCont;
      case C_XCONT:
	tmpBB = clrContFlag(tmpBB, C_XCONT);
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
	goto LBLTaskCont;
      case C_NERVOUS:
	error("mm2: never here");
	tmpBB = clrContFlag(tmpBB, C_NERVOUS);
	taskStack->setTop(topCache);
	goto LBLTaskNervous;
      case C_CFUNC_CONT:
	error("mm2: never here");
	tmpBB = clrContFlag(tmpBB, C_CFUNC_CONT);
	if (taskStack->isEmpty(e)) goto LBLTaskEmpty;
	biFun = (BIFun) TaskStackPop(--topCache);
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
	goto LBLTaskCFuncCont;
      default:
	error("POPTASK: unexpected task found.");
      }
    }

  LBLTaskEmpty:
    Thread::FinishCurrent();
    goto LBLstart;

  LBLTaskNervous:
    // nervous already done ?
    if (!tmpBB->isNervous()) {
      goto LBLfindWork;
    }

    DebugTrace(trace("nervous",tmpBB));

    INSTALLPATH(tmpBB);

    goto LBLreduce;


  LBLTaskCFuncCont:

    tmpBB->removeSuspension();
    INSTALLPATH(tmpBB);

    switch (biFun(XSize, X)) {
    case FAILED:
      if (currentTaskSusp != NULL &&
	  currentTaskSusp->isPropagated() == OK) {
	currentTaskSusp->markDead();
      }
      currentTaskSusp = NULL;
      HANDLE_FAILURE(0,
		     message("call of (*0x%x)(int, TaggedRef[]) failed",
			     biFun); biFun = NULL;
		     for (int i = 0; i < XSize; i++)
		        message("\nArg %d: %s",i+1,tagged2String(X[i]));
		     );
    case PROCEED:
      DebugCheckT(biFun = NULL);
      if (currentTaskSusp != NULL &&
	  currentTaskSusp->isPropagated() == OK) {
	currentTaskSusp->markDead();
      }
      currentTaskSusp = NULL;
      goto LBLreduce;
    default:
      error("Unexpected return value by c-function.");
    } // switch


  LBLTaskCont:
    DebugCheckT(GPlus = G; YPlus = Y);
    tmpBB->removeSuspension();

    INSTALLPATH(tmpBB);

    goto LBLemulate;
  }

// ----------------- end findWork -----------------------------------------

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

#if !defined THREADED || THREADED == 0

 LBLdispatcher:

  DebugCheck(blockSignals() == NO,
	     error("signalmask not zero"));
  DebugCheckT(unblockSignals());

  op = CodeArea::getOP(PC);

  DebugCheck(isFreedRefsArray(Y),
	     error("Referencing freed environment"););

#ifdef RECINSTRFETCH
  CodeArea::recordInstr(PC);
#endif

  DebugTrace( if (!trace("emulate",CBB,CAA,PC,Y,G))
	      {
		goto LBLfailure;
	      }
	      );


  switch (op) {

#endif THREADED

// the instructions are classified into groups
// to find a certain class of instructions search for the String "CLASS:"
// -------------------------------------------------------------------------
// CLASS: TERM: MOVE/UNIFY/CREATEVAR/...
// -------------------------------------------------------------------------

#include "register.hh"

// ------------------------------------------------------------------------
// *** (Fast-) Call/Execute Inline Funs/Rels
// ------------------------------------------------------------------------

  INSTRUCTION(FASTCALL)
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
      Abstraction *abstr = entry->getPred();
      INCFPC(2);

      DebugCheck(abstr == NULL,
		 error("FastCall: abstraction expected"));

      CallDoChecks(abstr,NO,PC,abstr->getArity());

      // set pc
      IHashTable *table = entry->indexTable;
      if (table) {
	DoSwitchOnTerm(X[0],table);
      } else {
	JUMP(abstr->getPC());
      }
    }


  INSTRUCTION(FASTEXECUTE)
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
      Abstraction *abstr = entry->getPred();

      DebugCheck(abstr == NULL,
		 error("FastCall: abstraction expected"));

      CallDoChecks(abstr,OK,PC,abstr->getArity());

      // set pc
      IHashTable *table = entry->indexTable;
      if (table) {
	DoSwitchOnTerm(X[0],table);
      } else {
	JUMP(abstr->getPC());
      }
    }

  INSTRUCTION(CALLBUILTIN)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      BIFun fun = entry->getFun();
      int arityExp = getPosIntArg(PC+2);
      int arity = entry->getArity();

      if (arity != arityExp && VarArity != arity) {
	warning("call: %s/%d with %d args", entry->getPrintName(), arity,arityExp);
	HANDLE_FAILURE(PC+3, ;);
      }

      switch (fun(arity, X)){
      case SUSPEND:
	warning("call builtin: SUSPEND unexpected\n");
	// no break here
      case FAILED:
	HANDLE_FAILURE(PC+3,
		       message("call: builtin %s/%d failed",
			       entry->getPrintName(),
			       arity);
		       for (int i = 0; i < arity; i++)
		       { message("\nArg %d: %s",i+1,tagged2String(X[i])); }
		       );
      case PROCEED:
      default:
	DISPATCH(3);
      }
    }


  INSTRUCTION(INLINEREL1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel1 rel         = (InlineRel1)entry->getInlineFun();

      TaggedRef A = Xreg(getRegArg(PC+2));
      OZ_Bool res = rel(A);

      switch(res) {

      case PROCEED:
	DISPATCH(3);

      case SUSPEND:
	{
	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    DISPATCH(3);
	  }

	  TaggedRef saveX0 = X[0];

	  X[0] = A;

	  DEREF(A,APtr,_1);
	  OZ_Suspension *susp = OZ_makeSuspension(entry->getFun(), X, 1);
	  OZ_addSuspension(APtr,susp);

	  X[0] = saveX0;

	  DISPATCH(3);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+3,
			 message("INLINEREL = {`%s` %s}",
				 entry->getPrintName(),
				 tagged2String(A)));
	}
      }
    }

  INSTRUCTION(INLINEREL2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel2 rel         = (InlineRel2)entry->getInlineFun();

      TaggedRef A = Xreg(getRegArg(PC+2));
      TaggedRef B = Xreg(getRegArg(PC+3));
      OZ_Bool res = rel(A,B);

      switch(res) {

      case PROCEED:
	DISPATCH(4);

      case SUSPEND:
	{
	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    e->trail.pushIfVar(B);
	    DISPATCH(4);
	  }

	  TaggedRef saveX0 = X[0];
	  TaggedRef saveX1 = X[1];

	  X[0] = A;
	  X[1] = B;

	  DEREF(A,APtr,_1); DEREF(B,BPtr,_2);
	  OZ_Suspension *susp = OZ_makeSuspension(entry->getFun(), X, 2);
	  if (isAnyVar(A)) OZ_addSuspension(APtr,susp);
	  if (isAnyVar(B)) OZ_addSuspension(BPtr,susp);

	  X[0] = saveX0;
	  X[1] = saveX1;

	  DISPATCH(4);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+4,
			 message("INLINEREL = {`%s` %s %s}",
				 entry->getPrintName(),
				 tagged2String(A),
				 tagged2String(B)));
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
      TaggedRef &Out         = Xreg(getRegArg(PC+3));

      TaggedRef A = Xreg(getRegArg(PC+2));
      OZ_Bool res = fun(A,Out);

      switch(res) {

      case PROCEED:
	DISPATCH(4);

      case SUSPEND:
	{
	  TaggedRef newVar = makeTaggedRef(newTaggedUVar(CBB));

	  Xreg(getRegArg(PC+3)) = newVar;

	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    DISPATCH(4);
	  }

	  TaggedRef saveX0 = X[0];
	  TaggedRef saveX1 = X[1];

	  X[0] = A;
	  X[1] = newVar;

	  DEREF(A,APtr,_1);
	  OZ_Suspension *susp = OZ_makeSuspension(entry->getFun(), X, 2);
	  OZ_addSuspension(APtr,susp);

	  X[0] = saveX0;
	  X[1] = saveX1;

	  DISPATCH(4);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+4,
			 Out = makeTaggedRef(newTaggedUVar(CBB));
			 message("INLINEFUN = {`%s` %s}",
				 entry->getPrintName(),
				 tagged2String(A)));
	}
      }
    }

  INSTRUCTION(INLINEFUN2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun2 fun = (InlineFun2)entry->getInlineFun();
      TaggedRef &Out = Xreg(getRegArg(PC+4));

      TaggedRef A      = Xreg(getRegArg(PC+2));
      TaggedRef B      = Xreg(getRegArg(PC+3));

      OZ_Bool res = fun(A,B,Out);

      switch(res) {

      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
	{
	  TaggedRef newVar = makeTaggedRef(newTaggedUVar(CBB));

	  Xreg(getRegArg(PC+4)) = newVar;

	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    e->trail.pushIfVar(B);
	    DISPATCH(5);
	  }

	  TaggedRef saveX0 = X[0];
	  TaggedRef saveX1 = X[1];
	  TaggedRef saveX2 = X[2];

	  X[0] = A;
	  X[1] = B;
	  X[2] = newVar;

	  DEREF(A,APtr,_1); DEREF(B,BPtr,_2);
	  OZ_Suspension *susp = OZ_makeSuspension(entry->getFun(), X, 3);
	  if (isAnyVar(A))
	    OZ_addSuspension(APtr,susp);
	  // special hack for exchangeCell: suspends only on the first argument
	  if (isAnyVar(B) && fun != BIexchangeCellInline)
	    OZ_addSuspension(BPtr,susp);

	  X[0] = saveX0;
	  X[1] = saveX1;
	  X[2] = saveX2;

	  DISPATCH(5);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+5,
			 Out = makeTaggedRef(newTaggedUVar(CBB));
			 message("INLINEFUN = {`%s` %s %s}",
				 entry->getPrintName(),
				 tagged2String(A),
				 tagged2String(B)));
	}
      }
    }


  INSTRUCTION(INLINEFUN3)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun3 fun = (InlineFun3)entry->getInlineFun();

      TaggedRef A    = Xreg(getRegArg(PC+2));
      TaggedRef B    = Xreg(getRegArg(PC+3));
      TaggedRef C    = Xreg(getRegArg(PC+4));
      TaggedRef &Out = Xreg(getRegArg(PC+5));

      OZ_Bool res = fun(A,B,C,Out);

      switch(res) {

      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  TaggedRef newVar = makeTaggedRef(newTaggedUVar(CBB));

	  Xreg(getRegArg(PC+5)) = newVar;

	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    e->trail.pushIfVar(B);
	    e->trail.pushIfVar(C);
	    DISPATCH(6);
	  }

	  TaggedRef saveX0 = X[0];
	  TaggedRef saveX1 = X[1];
	  TaggedRef saveX2 = X[2];
	  TaggedRef saveX3 = X[3];

	  X[0] = A; X[1] = B; X[2] = C; X[3] = newVar;

	  DEREF(A,APtr,_1); DEREF(B,BPtr,_2); DEREF(C,CPtr,_3);
	  OZ_Suspension *susp = OZ_makeSuspension(entry->getFun(), X, 4);
	  if (isAnyVar(A)) OZ_addSuspension(APtr,susp);
	  if (isAnyVar(B)) OZ_addSuspension(BPtr,susp);
	  if (isAnyVar(C)) OZ_addSuspension(CPtr,susp);

	  X[0] = saveX0; X[1] = saveX1;
	  X[2] = saveX2; X[3] = saveX3;

	  DISPATCH(6);
	}
      case FAILED:
	{
	  SHALLOWFAIL;
	  HANDLE_FAILURE(PC+6,
			 Out = makeTaggedRef(newTaggedUVar(CBB));
			 message("INLINEFUN = {`%s` %s %s %s}",
				 entry->getPrintName(),
				 tagged2String(A),
				 tagged2String(C),
				 tagged2String(B)));
	}
      }
    }
#undef SHALLOWFAIL

// ------------------------------------------------------------------------
// *** Shallow guards stuff
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

      OZ_Bool res = rel(Xreg(getRegArg(PC+2)));

      switch(res) {

      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
        {
	  TaggedRef A = Xreg(getRegArg(PC+2));
	  DEREF(A,APtr,ATag);
	  if (isAnyVar(ATag)) {
	    int argsToSave   = getPosIntArg(PC+4);
	    Suspension *susp =
	      new Suspension(new SuspContinuation(CBB,
						  PC, Y, G, X, argsToSave));
	    SVariable *cvar = taggedBecomesSuspVar(APtr);
	    CBB->addSuspension();
	    cvar->addSuspension(susp);
	  }
	  goto LBLreduce;
	}

      case FAILED:
	JUMP( getLabelArg(PC+3) );
      }
    }

  INSTRUCTION(SHALLOWTEST2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel2 rel         = (InlineRel2)entry->getInlineFun();

      OZ_Bool res = rel(Xreg(getRegArg(PC+2)),
			Xreg(getRegArg(PC+3)));

      switch(res) {

      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  TaggedRef A    = Xreg(getRegArg(PC+2));
	  TaggedRef B    = Xreg(getRegArg(PC+3));
	  DEREF(A,APtr,ATag); DEREF(B,BPtr,BTag);
	  int argsToSave    = getPosIntArg(PC+5);
	  Suspension *susp  =
	    new Suspension(new SuspContinuation(CBB,PC,Y,G,X,argsToSave));
	  SVariable *acvar;
	  SVariable *bcvar;
	  CBB->addSuspension();

	  DebugCheck(!isAnyVar(ATag) && !isAnyVar(BTag),
		     error ("in shallow test two values"););

	  if (isAnyVar(ATag)) {
	    acvar = taggedBecomesSuspVar(APtr);
	    acvar->addSuspension(susp);
	  }
	  if (isAnyVar(BTag)) {
	    bcvar = taggedBecomesSuspVar(BPtr);
	    bcvar->addSuspension(susp);
	  }
	  goto LBLreduce;
	}
      case FAILED:
	JUMP( getLabelArg(PC+4) );

      }
    }

  INSTRUCTION(SHALLOWTHEN)
    {
      int argsToSave = getPosIntArg(shallowCP+2);
      int numbOfCons = e->trail.chunkSize();

      if (numbOfCons == 0) {
	e->trail.popMark();
	shallowCP = NULL;
	DISPATCH(1);
      }

      Suspension *susp = 
	new Suspension(new SuspContinuation(CBB,shallowCP,Y,G,X,argsToSave));

      CBB->addSuspension();
      e->reduceTrailOnShallow(susp,numbOfCons);
      shallowCP = NULL;
      goto LBLreduce;
    }



  INSTRUCTION(ALLOCATEL)
    {
      int posInt = getPosIntArg(PC+1);

      DebugCheck(posInt==0, error("allocate: should be > 0"););
      IncfProfCounter(allocateCounter,(posInt+1)*sizeof(TaggedRef));

      Y = allocateY(posInt);
      DISPATCH(2);
    }

  INSTRUCTION(DEALLOCATEL)
    {
      DebugCheck(isFreedRefsArray(Y),
		 error("Freeing freed environment"););

      if (!isDirtyRefsArray(Y)) {
	IncfProfCounter(deallocateCounter,(getRefsArraySize(Y)+1)*sizeof(TaggedRef));
	deallocateY(Y);
      }

      Y=NULL;
      DISPATCH(1);
    }

  INSTRUCTION(FAILURE)
    {
      HANDLE_FAILURE(PC+1,message("'false'"));
    }


  INSTRUCTION(SUCCEED)
// NOOP
    DISPATCH(1);

  INSTRUCTION(SAVECONT)
    {
      e->pushTask(CBB,getLabelArg(PC+1),Y,G);

      DISPATCH(2);
    }

  INSTRUCTION(RETURN)
  {
    goto LBLreduce;
  }


  INSTRUCTION(SWITCHONTERMY)
    ONREG1(SwitchOnTerm,Y);

  INSTRUCTION(SWITCHONTERMG)
    ONREG1(SwitchOnTerm,G);

  INSTRUCTION(SWITCHONTERMX)
    ONREG1(SwitchOnTerm,X);

    SwitchOnTerm:
    {
      Reg reg = getRegArg(PC+1);
      IHashTable *table = (IHashTable *) (PC+2);

      DoSwitchOnTerm(RegAccess(HelpReg1,reg),table);
    }


// ------------------------------------------------------------------------
// *** Misc stuff: seldom used --> put at the end
// ------------------------------------------------------------------------

  INSTRUCTION(DEFINITIONX)
    ONREG1(Definition,X);

  INSTRUCTION(DEFINITIONY)
    ONREG1(Definition,Y);

  INSTRUCTION(DEFINITIONG)
    ONREG1(Definition,G);

   Definition:
    {
      Reg reg = getRegArg(PC+1);
      ProgramCounter next = getLabelArg(PC+2);
      PrTabEntry *pred = getPredArg(PC+3);
      AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+4);

      AssRegArray &list = pred->gRegs;
      int size = list.getSize();
      RefsArray gRegs = (size == 0) ? NULL : allocateRefsArray(size);

      Abstraction *p = new Abstraction(pred,gRegs);
      if (!e->fastUnify(RegAccess(HelpReg1,reg),makeTaggedSRecord(p))) {
	HANDLE_FAILURE(next,
		       message("definition %s/%d = %s",
			       p->getPrintName(),
			       p->getArity(),
			       tagged2String(RegAccess(HelpReg1,reg)))
		       );
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
      JUMP( next ); // change PC after reading all arguments
    }

// -------------------------------------------------------------------------
// CLASS: CONTROL: FAIL/SUCCESS/FENCE/CALL/EXECUTE/SWITCH/BRANCH
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
      SVariable *cvar = taggedBecomesSuspVar(termPtr);
      Suspension *susp =
	new Suspension(new SuspContinuation(CBB,
					    PC, Y, G, X, argsToSave));
      cvar->addSuspension (susp);
      CBB->addSuspension();
      goto LBLreduce;
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
    Atom *label      = getAtomArg(PC+1);
    TaggedRef object = RegAccess(HelpReg1,getRegArg(PC+2));
    int arity        = getPosIntArg(PC+3);
    ProgramCounter contadr = isExecute ? 0 : getLabelArg(PC+4);

    DEREF(object,_1,objectTag);
    if (!isSRecord(objectTag)) {
      if (isAnyVar(objectTag)) {
	if (isExecute) { DISPATCH(4); } else { DISPATCH(5); }
      }

      warning("send method: no abstraction or builtin: %s",tagged2String(object));
      HANDLE_FAILURE1(contadr,message("send method: no abstraction or builtin: %s",
				      tagged2String(object)));
    }

    Abstraction *def = getSendMethod(object,label,arity,X);
    if (def == NULL) {
      goto bombSend;
    }

    CallDoChecks(def,isExecute,contadr,arity+3);
    Y = NULL; // allocateL(0);

    JUMP(def->getPC());


  bombSend:
    contAdr = contadr;
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
    Atom *label            = getAtomArg(PC+1);
    TaggedRef origObject   = RegAccess(HelpReg1,getRegArg(PC+2));
    TaggedRef object       = origObject;
    int arity              = getPosIntArg(PC+3);
    ProgramCounter contadr = isExecute ? 0 : getLabelArg(PC+4);

    DEREF(object,_1,objectTag);
    if (!isSRecord(objectTag)) {
      if (isAnyVar(objectTag)) {
	if (isExecute) { DISPATCH(4); } else { DISPATCH(5); }
      }

      warning("apply method: no abstraction or builtin: %s",tagged2String(object));
      HANDLE_FAILURE1(contadr,message("apply method: no abstraction or builtin: %s",
				      tagged2String(object)));
    }

    Abstraction *def = getApplyMethod(object,label,arity-3+3,X[0]);
    if (def == NULL) {
      goto bombApply;
    }

    CallDoChecks(def,isExecute,contadr,arity);
    Y = NULL; // allocateL(0);

    JUMP(def->getPC());


  bombApply:
    if (methApplHdl == makeTaggedNULL()) {
      HANDLE_FAILURE1(contadr,
		      message("apply method: method Application Handler not set"));
    }

    TaggedRef method = makeMethod(arity-3,label,X);
    contAdr = contadr;
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

    /* TODO: deallocateL */

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
     TaggedRef functorTerm;
     Builtin *bi;
     predArity = getPosIntArg(PC+2);

     DebugCheckT(isExecutePlus = isExecute);

     // argument 3 is continuation adress
     // code after call is suspension handler
     contAdr = isExecute ? 0 : getLabelArg(PC+3);

     functorTerm = RegAccess(HelpReg1,getRegArg(PC+1));

     {
       DEREF(functorTerm,functorTermPtr,functorTag);

       if (!isSRecord(functorTag)) {
	 if (isAnyVar(functorTag)) {
	   if (isExecute) {
	     DISPATCH(3);
	   } else {
	     DISPATCH(4);
	   }
	 }

	 warning("call: no abstraction or builtin: %s",
		 tagged2String(functorTerm));
	 HANDLE_FAILURE1(contAdr,message("call: no abstraction or builtin: %s",
					 tagged2String(functorTerm)));
       }
     }
     predicate = tagged2SRecord(functorTerm);
     TypeOfRecord type;

// -----------------------------------------------------------------------
// --- Call: Loop
// -----------------------------------------------------------------------

  LBLcall:

// -----------------------------------------------------------------------
// --- Call: Abstraction
// -----------------------------------------------------------------------

    type = predicate->getType();

    switch (type) {
    case R_ABSTRACTION:
    case R_OBJECT:
    LBLabstractionEntry:
      {
	Abstraction *def = (Abstraction *) predicate;

        // arity check
	if (def->getArity() != predArity && VarArity != def->getArity()) {
	  warning("call: %s/%d with %d args",
		  def->getPrintName(),
		  def->getArity(),predArity);
	  HANDLE_FAILURE1(contAdr, message("call: %s/%d with %d args",
					   def->getPrintName(),
					   def->getArity(),predArity));
	}

	CallDoChecks(def,isExecute,contAdr,def->getArity());
	Y = NULL; // allocateL(0);

        // set pc
	JUMP(def->getPC());
      }


// -----------------------------------------------------------------------
// --- Call: Builtin
// -----------------------------------------------------------------------
    case R_BUILTIN:
    LBLbuiltinEntry:
      {
	bi = (Builtin *) predicate;

        // arity check
	if (bi->getArity() != predArity && VarArity != bi->getArity()) {
	  warning("call: builtin %s/%d with %d args",
		  bi->getPrintName(),
		  bi->getArity(),predArity);
	  HANDLE_FAILURE1(contAdr,message("call: builtin %s/%d with %d args",
					  bi->getPrintName(),
					  bi->getArity(),predArity));

	}

	State retVal;

	switch (bi->getType()) {

	case BIApply:
	  goto LBLBIapply;

	case BILoadFile:
	  goto LBLBIloadFile;

	case BIDefault:
	  switch (bi->getFun()(predArity, X)){
	  case SUSPEND:
	    predicate = bi->getSuspHandler();
	    if (!predicate) {
	      warning("call: builtin %s/%d: no suspension handler",
		       bi->getPrintName(),
		       bi->getArity());
	      HANDLE_FAILURE1(contAdr,
			      message("call: builtin %s/%d: "
				      "no suspension handler",
				      bi->getPrintName(),
				      bi->getArity()));

	    }
	    goto LBLcall;
	  case FAILED:
	    HANDLE_FAILURE(contAdr,
			   message("call: builtin %s/%d failed",
				    bi->getPrintName(),
				    bi->getArity());
			   for (int i = 0; i < predArity; i++)
			   { message("\nArg %d: %s",i+1,tagged2String(X[i])); }
			   );
	  case PROCEED:
	    if (isExecute) {
	      goto LBLreduce;
	    }
	    switch (emulateHook ()) {
	    case HOOK_SCHEDULE:
	      e->pushTask(CBB, contAdr,Y,G);
	      goto LBLschedule;
	    case HOOK_FIND:
	      e->pushTask(CBB, contAdr,Y,G);
	      goto LBLfindWorkDir;
	    }
	    JUMP(contAdr);
	  default:
	    error("builtin: bad return value");
	    goto LBLerror;
	  }
	default:
	  break;
	}
	error("emulate: call: builtin %s not correctly specified",
	      bi->getPrintName());
	goto LBLerror;
      } // end builtin
    default:
      warning("call: no abstraction or builtin: %s",
	      tagged2String(makeTaggedSRecord(predicate)));
      HANDLE_FAILURE(contAdr,message("call: no abstraction or builtin: %s",
				     tagged2String(makeTaggedSRecord(predicate))));
    } // end switch on type of predicate

// ------------------------------------------------------------------------
// --- Call Builtin Apply
// ------------------------------------------------------------------------

  LBLBIapply:
    {
      TaggedRef term0 = X[0];
      DEREF(term0,_0,tag0);
      TaggedRef term1 = X[1];
      DEREF(term1,_1,tag1);

      if (isAnyVar(tag0) || isAnyVar(tag1)) {
	warning("call: apply: arguments not determined");
	HANDLE_FAILURE1(contAdr,
			message("call: apply: arguments not determined"));
      }

      if (!isSRecord(term0)) {
	warning("call: apply: no predicate: %s",
		tagged2String(term0));
	HANDLE_FAILURE1(contAdr,message("call: apply: no predicate: %s",
					tagged2String(term0)));
      }

      predicate = tagged2SRecord(term0);

      switch (tag1) {
      case ATOM:
	predArity = 0;
      LBLtrueApply:
	switch (predicate->getType()) {
	case R_BUILTIN:
	  goto LBLbuiltinEntry;
	case R_ABSTRACTION:
	case R_OBJECT:
	  goto LBLabstractionEntry;
	default:
	  error("apply: impossible");
	  goto LBLerror;
	}
      case STUPLE:
	{
	  STuple *s = tagged2STuple(term1);
	  // XRegs initialization
	  for (int i = 0; i < s->getSize(); i++) {
	  X[i] = s->getArg(i);
	}
	  predArity = s->getSize();
	  goto LBLtrueApply;
	}
      case LTUPLE:
	{
	  LTuple *l = tagged2LTuple(term1);
	  // XRegs initialization
	  X[0] = l->getHead();
	  X[1] = l->getTail();
	  predArity = 2;

	  goto LBLtrueApply;
	}

      default:
	warning("call: apply: tuple argument expected");
	HANDLE_FAILURE1(contAdr,
			message("call: apply: tuple argument expected"));
      }
      error("call: builtin apply: impossible: never be here");
    }

// ------------------------------------------------------------------------
// --- Call: Builtin: Load File
// ------------------------------------------------------------------------

   LBLBIloadFile:
     {
       TaggedRef term0 = X[0];
       DEREF(term0,_0,tag0);

       if (!isXAtom(term0)) {
	 warning("call: loadFile: 1. arg must be a file name");
	   HANDLE_FAILURE1(contAdr,
			   message("call: loadFile: 1. arg must be a file name"));
       }
       char *file = tagged2Atom(term0)->getPrintName();

       FILE *fd = fopen(file,"r");

       if (fd == NULL) {
	 warning("call: loadFile: cannot open file '%s'",file);
	 HANDLE_FAILURE1(contAdr,
			 message("call: loadFile: cannot open file '%s'",file));
       }

       if (e->fastLoad) {
	 message("Fast loading file '%s'\n",file);
       }

       // begin critical region
       blockSignals();

       (void) e->loadQuery(fd,OK);	/* "NO" means: do not acknowledge */

       fclose(fd);

       unblockSignals();
       // end critical region

       if (isExecute) {
	 goto LBLreduce;
       }
       JUMP(contAdr);
     }

  }


// --------------------------------------------------------------------------
// --- end call/execute -----------------------------------------------------
// --------------------------------------------------------------------------

  INSTRUCTION(WAIT)
    {
      DebugCheck(!CBB->isWait(),error("WAIT"));
      DebugCheck(CBB->isCommitted(),error("WAIT"));

      /* unit commit */
      WaitActor *aa = CastWaitActor(CBB->getActor());
      if (aa->hasOneChild()) {
	Board *bb = CBB;
	e->reduceTrailOnUnitCommit();
        bb->unsetInstalled();
	Board::SetCurrent(aa->getBoard()->getBoardDeref());

	Bool ret = e->installScript(bb->getScriptRef());
	DebugCheck(!ret,error("WAIT"));
	bb->setCommitted(CBB);
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

	 Board::SetCurrent(CBB->getParentBoard()->getBoardDeref());
	 tmpBB->unsetInstalled();
	 tmpBB->setCommitted(CBB);
	 CBB->removeSuspension();

	 goto LBLreduce;
       }

      /* unit commit for WAITTOP */
      WaitActor *aa = CastWaitActor(CBB->getActor());
      Board *bb = CBB;
      if (aa->hasOneChild()) {
	e->reduceTrailOnUnitCommit();
        bb->unsetInstalled();
	Board::SetCurrent(aa->getBoard()->getBoardDeref());

	Bool ret = e->installScript(bb->getScriptRef());
	DebugCheck(!ret,error("WAITTOP"));
	bb->setCommitted(CBB);
	CBB->removeSuspension();
	goto LBLreduce;
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
	Board::SetCurrent(CBB->getParentBoard()->getBoardDeref());
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

      CAA = CBB->getActor();

      if (CAA->hasNext()) {

	e->deinstallCurrent();

      LBLexecuteNext:
	DebugTrace(trace("next clause",CBB,CAA));

	LOADCONT(CAA->getNext());

	goto LBLemulate;
      }

      // suspend a actor
      DebugTrace(trace("suspend actor",CBB,CAA));

      goto LBLfindWork;
    }


// -------------------------------------------------------------------------
// CLASS: NODES: CREATE/END
// -------------------------------------------------------------------------

  INSTRUCTION(CREATECOND)
    {
      ProgramCounter elsePC = getLabelArg(PC+1);
      int argsToSave = getPosIntArg(PC+2);

      CAA = new AskActor(CBB,Thread::GetCurrentPriority(),
			 elsePC ? elsePC : NOCODE,
			 NOCODE, Y, G, X, argsToSave);

      DISPATCH(3);
    }

  INSTRUCTION(CREATEOR)
    {
      ProgramCounter elsePC = getLabelArg (PC+1);
      int argsToSave = getPosIntArg (PC+2);

      CAA = new WaitActor(CBB,Thread::GetCurrentPriority(),
			  NOCODE, Y, G, X, argsToSave);

      DISPATCH(3);
    }

  INSTRUCTION(CREATEENUMOR)
    {
      ProgramCounter elsePC = getLabelArg (PC+1);
      int argsToSave = getPosIntArg (PC+2);

      CAA = new WaitActor(CBB,Thread::GetCurrentPriority(),
			  NOCODE, Y, G, X, argsToSave);
      CAA->setDisWait();
      DISPATCH(3);
    }

  INSTRUCTION(WAITCLAUSE)
    {
      // create a node
      Board::NewCurrentWait(CAA);
      DebugCheckT(CAA=NULL);
      IncfProfCounter(waitCounter,sizeof(Board));
      e->trail.pushMark();
      DISPATCH(1);
    }

  INSTRUCTION(ASKCLAUSE)
    {
      Board::NewCurrentAsk(CAA);
      DebugCheckT(CAA=NULL);
      IncfProfCounter(askCounter,sizeof(Board));
      e->trail.pushMark();
      DISPATCH(1);
    }


  INSTRUCTION(ELSECLAUSE)
    DISPATCH(1);

  INSTRUCTION(PROCESS)
    {
      markDirtyRefsArray(Y);
      ProgramCounter newPC = PC+2;
      ProgramCounter contPC = getLabelArg(PC+1);

      int prio =
	Thread::GetCurrentPriority() > Thread::GetUserPriority()
	? Thread::GetUserPriority()
	: Thread::GetCurrentPriority();

      Thread *tt = new Thread(prio);
      IncfProfCounter(procCounter,sizeof(Thread)+TaskStack::DefaultSize*4);
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
  INSTRUCTION(SEQOBSOLETE)
  INSTRUCTION(SEQWAITOBSOLETE)
  INSTRUCTION(EXCEPTIONOBSOLETE)
    {
      error("Emulate: ERROR command executed");
      goto LBLerror;
    }


  INSTRUCTION(DEBUGINFO)
    {

// mm2: should be done in assembler !!
      error("DEBUGINFO: not longer suuported");
      DISPATCH(4);
    }

  INSTRUCTION(NOOP)
    DISPATCH(1);


  INSTRUCTION(TESTLABEL1)
  INSTRUCTION(TESTLABEL2)
  INSTRUCTION(TESTLABEL3)
  INSTRUCTION(TESTLABEL4)

  INSTRUCTION(TEST1)
  INSTRUCTION(TEST2)
  INSTRUCTION(TEST3)
  INSTRUCTION(TEST4)

  INSTRUCTION(ENDOFFILE)
  INSTRUCTION(LABEL)
    warning("emulate: Unimplemented command");
    goto LBLreduce;

#if ! defined THREADED || THREADED == 0
  default:
    warning("emulate instruction: default should never happen");
    break;
   } /* switch*/
#endif


// ----------------- end emulate ------------------------------------------



// ------------------------------------------------------------------------
// *** REDUCE Actor
// ------------------------------------------------------------------------
 LBLreduceActor:
  {
    DebugTrace(trace("reduce actor",CBB,CAA));

    if ( CAA->hasNext() ) {
      goto LBLexecuteNext;
    }

    if (CAA->isAsk()) {
/* check if else clause must be activated */
      if ( CAA->isLeaf() ) {

/* rule: if else ... fi
   push the else cont on parent && remove actor */
	LOADCONT(CAA->getNext());
	PC = CastAskActor(CAA)->getElsePC();
	if (PC != NOCODE) {
	  CAA->setCommitted();
	  CBB->removeSuspension();
	  goto LBLemulateIfProcess;
	}

/* rule: if fi --> false */
	CAA->setCommitted(); // mm2 necessary ???

	HANDLE_FAILURE(0,message("reducing 'if fi' to 'false'"));
      }
    } else {
      WaitActor *aa = CastWaitActor(CAA);
      
/* rule: or <sigma> ro (unit commit rule) */
      if (aa->hasOneChild()) {
	Board *waitBoard = aa->getChild();
	if (waitBoard->isWaiting()) {
	  waitBoard->setCommitted(CBB); // do this first !!!
	  if (!e->installScript(waitBoard->getScriptRef())) {
	    HANDLE_FAILURE(0,
			   message("unit commit failed");
			   CBB->removeSuspension();
			   CAA->setCommitted();
			   goto LBLreduce;
			   );
	  }

	  if (waitBoard->isWaitTop() &&
	      !waitBoard->hasSuspension()) {
	    CAA->setCommitted();
	    CBB->removeSuspension();
	    goto LBLreduce;
	  }

	  LOADCONT(waitBoard->getBodyPtr());

/* unit commit & WAIT
   e.g. or X = 1 ... then ... [] false ro */
/* unit commit & WAITTOP
   or guard not completed they can never happen ???
   e.g. or X = 1 [] false ro
    --> surrounding 'ask' may fire or 'process' may be removed */

	  DebugCheck(PC == NOCODE,error("reduce actor"));

	  goto LBLemulateIfProcess;
	}
      }
    }

/* no rule: suspend longer */
    goto LBLfindWork;
  }

// ----------------- end reduce actor --------------------------------------


// ------------------------------------------------------------------------
// *** Emulate if process node is ok
// ------------------------------------------------------------------------
 LBLemulateIfProcess:
  if (e->isSetSFlag(ThreadSwitch)) {
    e->pushTask(CBB,PC,Y,G,X,XSize);
    goto LBLschedule;
  }
  goto LBLemulate;

// ----------------- end emulate if process -------------------------------


// ------------------------------------------------------------------------
// *** REDUCE CONTROL NODE
// ------------------------------------------------------------------------

 LBLreduce:
  DebugTrace(trace("reduce control node",CBB));

  CBB->unsetNervous();

  if (CBB->isAsk()) {
    if ( e->entailment() ) {
      LOADCONT(CBB->getBodyPtr());

      e->trail.popMark();

      tmpBB = CBB;

      tmpBB->getActor()->setCommitted();
      Board::SetCurrent(CBB->getParentBoard()->getBoardDeref());
      tmpBB->unsetInstalled();
      tmpBB->setCommitted(CBB);

      CBB->removeSuspension();

      goto LBLemulateIfProcess;
    }
  } else if (CBB->isWait ()) {
// WAITTTOP
    if (CBB->isWaitTop()) {

// WAITTOP: top commit
      if ( e->entailment() ) {
	goto LBLtopCommit;
      }

      DebugCheck(CastWaitActor(CBB->getActor())->hasOneChild(),
		 error("reduce: waittop: can not happen");
		 goto LBLerror;);

// WAITTOP: no rule
      goto LBLfindWork;
    }

    DebugCheck(CastWaitActor(CBB->getActor())->hasOneChild(),
	       error("reduce: wait: unit commit can not happen");
	       goto LBLerror;);
    goto LBLfindWork;

// WAIT: no rule
  } else {
    DebugCheck(!CBB->isRoot(),error("reduce"));
  }
  goto LBLfindWork;

// ----------------- end reduce -------------------------------------------

// ------------------------------------------------------------------------
// *** FAILURE
// ------------------------------------------------------------------------
 LBLfailure:
  DebugTrace(trace("fail",CBB));

  CAA=Board::FailCurrent();
  goto LBLreduceActor;

// ----------------- end failure ------------------------------------------
}

#ifdef OUTLINE
#undef inline
#endif
