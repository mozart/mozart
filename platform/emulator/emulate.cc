/* This may look like C code, but it is really -*- C++ -*-

  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow,mehl,scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  The main engine
  ------------------------------------------------------------------------
*/

// Ozcar special...
//#define STEPWITHDEBUGINFO
//#define DEBUGMAGIC

#ifdef DEBUGMAGIC
 #include <sys/stat.h>
#endif

#ifdef INTERFACE
#pragma implementation "emulate.hh"
#endif

#include "am.hh"

#include "indexing.hh"

#include "genvar.hh"
#include "dictionary.hh"
#include "fdhook.hh"

/*
 * Object stuff
 */

inline
Abstraction *getSendMethod(Object *obj, TaggedRef label, SRecordArity arity, 
			   InlineCache *cache, RefsArray X)
{
  Assert(isFeature(label));

  if (obj->isClass())
      return NULL;

  return cache->lookup(obj,label,arity,X);
}

inline
Abstraction *getApplyMethod(Object *obj, ApplMethInfoClass *ami, 
			    SRecordArity arity, RefsArray X)
{
  Assert(isFeature(ami->methName));
  return ami->methCache.lookup(obj,ami->methName,arity,X);
}

inline
Abstraction *getApplyMethodForGenCall(Object *obj, TaggedRef label,
				      SRecordArity arity)
{
  Assert(isFeature(label));
  Bool defaultsUsed;
  Abstraction *ret = obj->getMethod(label,arity,am.xRegs,defaultsUsed);
  return defaultsUsed ? (Abstraction*) 0 : ret;
}

// -----------------------------------------------------------------------
// EXCEPTION

void AM::formatError(OZ_Term traceBack,OZ_Term loc)
{
  OZ_Term d = OZ_record(OZ_atom("d"),
			OZ_cons(OZ_atom("info"),
				cons(OZ_atom("stack"),
				     cons(OZ_atom("loc"),
					  OZ_nil()))));
  OZ_putSubtree(d,OZ_atom("stack"),traceBack);
  OZ_putSubtree(d,OZ_atom("loc"),loc);
  OZ_putSubtree(d,OZ_atom("info"),exception.info);

  exception.value=OZ_adjoinAt(exception.value,OZ_atom("debug"),d);
}

int AM::raise(OZ_Term cat, OZ_Term key, char *label,int arity,...) 
{
  OZ_Term exc=OZ_tuple(key,arity+1);
  OZ_putArg(exc,0,OZ_atom(label));

  va_list ap;
  va_start(ap,arity);

  for (int i = 0; i < arity; i++) {
    OZ_putArg(exc,i+1,va_arg(ap,OZ_Term));
  }

  va_end(ap);


  OZ_Term ret = OZ_record(cat,
			  cons(OZ_int(1),
			       cons(OZ_atom("debug"),OZ_nil())));
  OZ_putSubtree(ret,OZ_int(1),exc);
  OZ_putSubtree(ret,OZ_atom("debug"),NameUnit);

  exception.info = NameUnit;
  exception.debug = OZ_eq(cat,E_ERROR) ? TRUE : ozconf.errorDebug;
  exception.value = ret;
  return RAISE;
}

#define RAISE_APPLY(fun,args) \
  (void) e->raise(E_ERROR,E_KERNEL,"apply",2,fun,args); goto LBLraise;


void AM::enrichTypeException(char *fun, OZ_Term args)
{
  OZ_Term e = OZ_subtree(exception.value,OZ_int(1));
  OZ_putArg(e,1,OZ_atom(fun));
  OZ_putArg(e,2,args);
}

#define RAISE_TYPE1(fun,args)			\
  e->enrichTypeException(fun,args); goto LBLraise;

#define RAISE_TYPE1_FUN(fun,args)				\
  RAISE_TYPE1(fun,						\
	      appendI(args,cons(OZ_newVariable(),nil())));

#define RAISE_TYPE					\
   RAISE_TYPE1(builtinTab.getName((void *) biFun),	\
	       OZ_toList(predArity,X));

/*
 * Handle Failure macros (HF)
 */

#define HF_RAISE_FAILURE(T)					\
   {								\
     if (!e->isToplevel()) {					\
       if (!CTT->hasCatchFlag() || CBB!=CTT->getBoard()) {	\
	 goto LBLfailure;					\
       }							\
     }								\
     e->exception.info  = ozconf.errorDebug?T:NameUnit;		\
     e->exception.value = RecordFailure;			\
     e->exception.debug = ozconf.errorDebug;			\
     goto LBLraise;						\
   }

#define HF_FAIL       HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_DIS        HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_COND       HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_EQ(X,Y)    HF_RAISE_FAILURE(OZ_mkTupleC("eq",2,X,Y))
#define HF_TELL(X,Y)  HF_RAISE_FAILURE(OZ_mkTupleC("tell",2,X,Y))
#define HF_APPLY(N,A) HF_RAISE_FAILURE(OZ_mkTupleC("apply",2,N,A))
#define HF_BI	      HF_APPLY(OZ_atom(builtinTab.getName((void *) biFun)), \
			       OZ_toList(predArity,X));

#define CheckArity(arityExp,proc)					 \
if (predArity != arityExp && VarArity != arityExp) {			 \
  (void) e->raise(E_ERROR,E_KERNEL,"arity",2,proc,OZ_toList(predArity,X)); \
  goto LBLraise;							 \
}

// -----------------------------------------------------------------------
//

#define NOFLATGUARD   (shallowCP==NULL)

#define SHALLOWFAIL   if (shallowCP) { goto LBLshallowFail; }

#define IMPOSSIBLE(INSTR) error("%s: impossible instruction",INSTR)






// TOPLEVEL END
// -----------------------------------------------------------------------


#define DoSwitchOnTerm(indexTerm,table)					    \
      TaggedRef term = indexTerm;					    \
      DEREF(term,termPtr,_2);						    \
									    \
      if (!isLTuple(term)) {						    \
	TaggedRef *sp = sPointer;					    \
	ProgramCounter offset = switchOnTermOutline(term,termPtr,table,sp); \
	sPointer = sp;							    \
	JUMP(offset);							    \
      }									    \
									    \
      ProgramCounter offset = table->listLabel;				    \
      sPointer = tagged2LTuple(term)->getRef();				    \
      JUMP(offset);



#ifdef OUTLINE
#define inline
#endif

// -----------------------------------------------------------------------
// genCallInfo: self modifying code!

static
void genCallInfo(GenCallInfoClass *gci, TaggedRef pred, ProgramCounter PC)
{
  static int gencall = 0;
  static int gencallmethfailed = 0;
  static int gencallfailed = 0;
  gencall++;
  
  Assert(!isRef(pred));

  Abstraction *abstr = NULL;
  if (gci->isMethAppl) {
    if (!isObject(pred) ||
	NULL == (abstr = getApplyMethodForGenCall(tagged2Object(pred),
						  gci->mn,gci->arity))) {
      ApplMethInfoClass *ami = new ApplMethInfoClass(gci->mn,gci->arity);
      CodeArea::writeOpcode(gci->isTailCall ? TAILAPPLMETHG : APPLMETHG, PC);
      CodeArea::writeAddress(ami, PC+1);
      CodeArea::writeRegIndex(gci->regIndex, PC+2);
      gencallmethfailed++;
      return;
    }
  } else {
    if(!isAbstraction(pred)) 
      goto bombGenCall;

    abstr = tagged2Abstraction(pred);
    if (abstr->getArity() != getWidth(gci->arity)) 
      goto bombGenCall;
  }
  
  {
    /* ok abstr points to an abstraction */
    AbstractionEntry *entry = AbstractionTable::add(abstr);
    CodeArea::writeAddress(entry, PC+1);
    CodeArea::writeOpcode(gci->isTailCall ? FASTTAILCALL : FASTCALL, PC);
    return;
  }
  
bombGenCall:
  gencallfailed++;
  CodeArea::writeRegIndex(gci->regIndex,PC+1);
  CodeArea::writeArity(getWidth(gci->arity), PC+2);
  CodeArea::writeOpcode(gci->isTailCall ? TAILCALLG : CALLG,PC);
  return;
}

// -----------------------------------------------------------------------
// CALL HOOK


/* the hook functions return:
     TRUE: must reschedule
     FALSE: can continue
   */

Bool AM::emulateHookOutline(ProgramCounter PC, Abstraction *def, TaggedRef *arguments)
{
  // without signal blocking;
  if (isSetSFlag(ThreadSwitch)) {
    if (threadQueuesAreEmpty()) {
      restartThread();
    } else {
      return TRUE;
    }
  }
  if (isSetSFlag((StatusBit)(StartGC|UserAlarm|IOReady))) {
    return TRUE;
  }
  
  if (breakflag) {
    currentThread->startStepMode();
    currentThread->traced();
    breakflag = NO;
    debugStreamThread(currentThread);
  }

  if (def && debugmode() && currentThread->stepMode()) {
#ifndef STEPWITHDEBUGINFO
    //Board *b = am.currentBoard;
    //am.currentBoard = am.rootBoard;
    
    // this finds the debuginfo _after_ the call, but who cares? :-)
    ProgramCounter dbg = CodeArea::nextDebugInfo(PC);

    if (dbg != NOCODE) {

      TaggedRef tail    = threadStreamTail;
      TaggedRef newTail = OZ_newVariable();
    
      TaggedRef file, comment;
      int line, abspos;

      currentThread->stop();
      CodeArea::getDebugInfoArgs(dbg,file,line,abspos,comment);
      
      TaggedRef pairlist = 
	OZ_cons(OZ_pairA("thr",
			 OZ_mkTupleC("#",2,makeTaggedConst(currentThread),
				     OZ_int(currentThread->getID()))),
		OZ_cons(OZ_pairA("file", file),
			OZ_cons(OZ_pairAI("line", line),
				OZ_nil())));
      
      TaggedRef entry = OZ_recordInit(OZ_atom("step"), pairlist);
      OZ_unify(tail, OZ_cons(entry, newTail));
      threadStreamTail = newTail;

      //am.currentBoard = b;
      
      return TRUE;
    }
#endif
  }

  return FALSE;
}

inline
Bool AM::isNotPreemtiveScheduling(void)
{
  if (isSetSFlag(ThreadSwitch)) {
    if (threadQueuesAreEmpty())
      restartThread();
    else
      return FALSE;
  }
  return !isSetSFlag(StartGC);
}

inline
Bool AM::hookCheckNeeded()
{
#ifdef DEBUG_DET
  static int counter = 100;
  if (--counter < 0) {
    handleAlarm();   // simulate an alarm
    counter = 100;
  }
#endif
  
  return (isSetSFlag());
}


/* macros are faster ! */
#define emulateHookCall(e,def,arguments,Code)		\
    if (e->hookCheckNeeded()) {				\
      if (e->emulateHookOutline(PC, def, arguments)) {	\
	Code;						\
      }							\
    }

#define emulateHookPopTask(e,Code) emulateHookCall(e,0,0,Code)


#define CallPushCont(ContAdr) e->pushTaskInline(ContAdr,Y,G)

#define ChangeSelf(obj)				\
      e->changeSelf(obj);

#define SaveSelf				\
      e->saveSelf();


/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred,gRegs)						\
     Y = NULL;									\
     G = gRegs;									\
     emulateHookCall(e,Pred,X,							\
		     e->pushTask(Pred->getPC(),NULL,G,X,Pred->getArity());	\
		     goto LBLpreemption;);


// load a continuation into the machine registers PC,Y,G,X
#define LOADCONT(cont)				\
  {						\
      Continuation *tmpCont = cont;		\
      PC = tmpCont->getPC();			\
      Y = tmpCont->getY();			\
      G = tmpCont->getG();			\
      predArity = tmpCont->getXSize();		\
      tmpCont->getX(X);				\
  }

// -----------------------------------------------------------------------
// THREADED CODE

#if defined(RECINSTRFETCH) && defined(THREADED)
 Error: RECINSTRFETCH requires THREADED == 0;
#endif

#define INCFPC(N) PC += N

// #define WANT_INSTRPROFILE
#if defined(WANT_INSTRPROFILE) && defined(__GNUC__)
#define asmLbl(INSTR) asm(" " #INSTR ":");
#else
#define asmLbl(INSTR)
#endif


/* threaded code broken on linux, leads to memory leek,
 * this is a workaround
 * seems to work under 2.7.x
 */

#ifdef THREADED
#define Case(INSTR) INSTR##LBL : asmLbl(INSTR); 

// let gcc fill in the delay slot of the "jmp" instruction:
#define DISPATCH(INC) {							      \
  intlong aux = *(PC+INC);						      \
  INCFPC(INC);								      \
  goto* (void*) (aux|textBase);					   	      \
}

#else /* THREADED */

#define Case(INSTR)   case INSTR :
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#endif

#define JUMP(absAdr) Assert(absAdr!=0); PC=absAdr; DISPATCH(0)

#define ONREG(Label,R)      HelpReg = (R); goto Label
#define ONREG2(Label,R1,R2) HelpReg1 = (R1); HelpReg2 = (R2); goto Label


#ifdef FASTREGACCESS
#define RegAccess(Reg,Index) (*(RefsArray)((intlong) Reg + Index))
#define LessIndex(Index,CodedIndex) (Index <= CodedIndex/sizeof(TaggedRef))
#else
#define RegAccess(Reg,Index) (Reg[Index])
#define LessIndex(I1,I2) (I1<=I2)
#endif

#define Xreg(N) RegAccess(X,N)
#define Yreg(N) RegAccess(Y,N)
#define Greg(N) RegAccess(G,N)

#define XPC(N) Xreg(getRegArg(PC+N))

 
/* define REGOPT if you want the into register optimization for GCC */
#if defined(REGOPT) &&__GNUC__ >= 2 && (defined(GNUWIN32) || defined(LINUX_I486) || defined(MIPS) || defined(OSF1_ALPHA) || defined(SPARC)) && !defined(DEBUG_CHECK)
#define Into(Reg) asm(#Reg)

#if defined(LINUX_I486) || defined(GNUWIN32)

#define Reg1 asm("%esi")
#define XReg1 
#define Reg2
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#define Reg7
#endif

#ifdef SPARC
#define Reg1 asm("i0")
#define Reg2 asm("i1")
#define Reg3 asm("i2")
#define Reg4 asm("i3")
#define Reg5 asm("i4")
#define Reg6 asm("i5")
#define Reg7 asm("l0")
#endif

#ifdef OSF1_ALPHA
#define Reg1 asm("$9")
#define Reg2 asm("$10")
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#define Reg7
#endif

#ifdef MIPS
#define Reg1 asm("$16")
#define Reg2 asm("$17")
#define Reg3 asm("$18")
#define Reg4 asm("$19")
#define Reg5 asm("$20")
#define Reg6
#define Reg7
#endif

#else

#define Reg1
#define Reg2
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#define Reg7

#endif

/*
 * Handling of the READ/WRITE mode bit: 
 * last significant bit of sPointer set iff in WRITE mode
 */

#define SetReadMode
#define SetWriteMode (sPointer = (TaggedRef *)((long)sPointer+1));

#define InWriteMode (((long)sPointer)&1)

#define GetSPointerWrite(ptr) (TaggedRef*)(((long)ptr)-1)

// ------------------------------------------------------------------------
// outlined auxiliary functions
// ------------------------------------------------------------------------


#define SUSP_PC(TermPtr,RegsToSave,PC)		\
   e->pushTask(PC,Y,G,X,RegsToSave);		\
   addSusp(TermPtr,CTT);			\
   goto LBLsuspendThread;


void addSusp(TaggedRef *varPtr, Thread *thr)
{
  taggedBecomesSuspVar(varPtr)->addSuspension (thr);
}

void addSusp(TaggedRef var, Thread *thr)
{
  DEREF(var,varPtr,_1);
  Assert(isAnyVar(var));

  addSusp(varPtr,thr);
}


/*
 * create the suspension for builtins returning SUSPEND
 *
 * PRE: no reference chains !!
 */
void AM::suspendOnVarList(Thread *thr)
{
  // mm2 Assert(suspendVarList!=makeTaggedNULL());

  TaggedRef varList=suspendVarList;
  while (!isRef(varList)) {
    Assert(isCons(varList));
    
    addSusp(head(varList),thr);
    varList=tail(varList);
  }
  addSusp(varList,thr);
  suspendVarList=makeTaggedNULL();
}

void AM::suspendInline(int n, OZ_Term A,OZ_Term B,OZ_Term C)
{
  switch(n) { /* no break's used!! */
  case 3: { DEREF (C, ptr, _1); if (isAnyVar(C)) addSusp(ptr, currentThread); }
  case 2: { DEREF (B, ptr, _1); if (isAnyVar(B)) addSusp(ptr, currentThread); }
  case 1: { DEREF (A, ptr, _1); if (isAnyVar(A)) addSusp(ptr, currentThread); }
    break;
  default:
    Assert(0);
  }
}


static
TaggedRef makeMessage(SRecordArity srecArity, TaggedRef label, TaggedRef *X)
{
  int width = getWidth(srecArity);
  if (width == 0) {
    return label;
  }

  if (width == 2 && literalEq(label,AtomCons))
    return makeTaggedLTuple(new LTuple(X[0],X[1]));

  SRecord *tt;
  if(sraIsTuple(srecArity)) {
    tt = SRecord::newSRecord(label,width);
  } else {
    tt = SRecord::newSRecord(label,getRecordArity(srecArity));
  }
  for (int i = width-1;i >= 0; i--) {
    tt->setArg(i,X[i]);
  }
  TaggedRef ret = makeTaggedSRecord(tt);
  //  message("makeMessage: %s\n",toC(ret));
  return ret;
}

TaggedRef AM::createNamedVariable(int regIndex, TaggedRef name)
{
  Assert(isLiteral(name));
  toplevelVarsCount = regIndex;
  int size = getRefsArraySize(toplevelVars);
  if (LessIndex(size,regIndex)) {
    int newSize = int(size*1.5);
    message("resizing store for toplevel vars from %d to %d\n",size,newSize);
    toplevelVars = resize(toplevelVars,newSize);
    // no deletion of old array --> GC does it
  }
  SVariable *svar = new SVariable(currentBoard);
  TaggedRef ret = makeTaggedRef(newTaggedSVar(svar));
  VariableNamer::addName(ret,tagged2Literal(name)->getPrintName());
  return ret;
}

/*
 * Entailment handling for emulator
 *
 * check entailment and stability
 *  after thread is finished or top commit
 */
void AM::checkEntailment()
{
  DebugTrace(trace("check entailment",currentBoard));

  currentBoard->unsetNervous();

  // check for entailment of ASK and WAITTOP
  if ((currentBoard->isAsk() || currentBoard->isWaitTop()) && entailment()) {
    Board *bb = currentBoard;
    deinstallCurrent();
    int ret=commit(bb);
    Assert(ret);
    return;
  }

  // do solve
  if (!currentBoard->isSolve()) return;

  // try to reduce a solve board;
  SolveActor *solveAA = SolveActor::Cast(currentBoard->getActor());
  Board      *solveBB = currentBoard; 
 
  if (isStableSolve(solveAA)) {
    Assert(trail.isEmptyChunk());
    // all possible reduction steps require this; 

    if (!solveBB->hasSuspension()) {
      // 'solved';
      // don't unlink the subtree from the computation tree;
      trail.popMark();
      currentBoard->unsetInstalled();
      setCurrent(currentBoard->getParent());
      // don't decrement counter of parent board!

      if (!fastUnifyOutline(solveAA->getResult(), 
			    solveAA->genSolved(), OK)) {
	Assert(0);
      }
      return;
    }

    WaitActor *wa = solveAA->topChoice();

    if (wa == NULL) {
      // "stuck" (stable without distributing waitActors);
      // don't unlink the subtree from the computation tree; 
      trail.popMark();
      currentBoard->unsetInstalled();
      setCurrent (currentBoard->getParent());

      // don't decrement counter of parent board!

      if ( !fastUnifyOutline(solveAA->getResult(), 
			     solveAA->genStuck(), OK) ) {
	Assert(0);
      }
      return;
    }

    // to enumerate;

    if (wa->getChildCount()==1) {
      Assert(wa->isChoice());

      solveAA->popChoice();

      Board *waitBoard = wa->getChildRef();

      int ret=commit(waitBoard);
      Assert(ret);

      DebugCode (currentThread = (Thread *) NULL);

      wa->dispose();
      return;
    }

    // give back number of clauses
    trail.popMark();
    currentBoard->unsetInstalled();
    setCurrent(currentBoard->getParent());

    // don't decrement counter of parent board!

    if (!fastUnifyOutline(solveAA->getResult(),
			  solveAA->genChoice(wa->getChildCount()),
			  OK)) {
      Assert(0);
    }
    return;
  }

  if (solveAA->getThreads() == 0) {
    // There are some external suspensions to this solver!

    deinstallCurrent();

    TaggedRef newVar = makeTaggedRef(newTaggedUVar(currentBoard));
    TaggedRef result = solveAA->getResult();

    solveAA->setResult(newVar);

    if ( !fastUnifyOutline(result,
			   solveAA->genUnstable(newVar),
			   OK)) {
      Assert(0);
    }
    return;
  } 

  deinstallCurrent();
  return;
} 


/* &Var prevent Var to be allocated to a register,
 * increases chances that variables declared as "register"
 * will be really allocated to registers
 */

#define NoReg(Var) { void *p = &Var; }

void engine(Bool init) 
{  
// ------------------------------------------------------------------------
// *** Global Variables
// ------------------------------------------------------------------------
  /* ordered by importance: first variables will go into machine registers
   * if -DREGOPT is set
   */
  register ProgramCounter PC   Reg1 = 0;
  register RefsArray X         Reg2 = am.xRegs;
  register RefsArray Y         Reg3 = NULL;
  register TaggedRef *sPointer Reg4 = NULL;
  register AM *e               Reg5 = &am;
  register RefsArray G         Reg6 = NULL;

  Bool isTailCall              = NO;                NoReg(isTailCall);
  AWActor *CAA                 = NULL;
  DebugCheckT(Board *currentDebugBoard=0);

  RefsArray HelpReg1 = NULL, HelpReg2 = NULL;
  #define HelpReg sPointer  /* more efficient */

  /* shallow choice pointer */
  ByteCode *shallowCP = NULL;

  OZ_CFun biFun = NULL;     NoReg(biFun);
  ConstTerm *predicate;	    NoReg(predicate);
  int predArity;    	    NoReg(predArity);

  // short names
# define CBB (e->currentBoard)
# define CTT (e->currentThread)
# define CPP (CTT->getPriority())
# define CTS (e->cachedStack)

#ifdef CATCH_SEGV
  //
  //  In the case of an error, the control is passed either to 
  // LBLkillToplevelThread or LBLfailure; Note that both require
  // the presence of a runnable thread;
  switch (e->catchError()) {
  case 0:
    //  the environment is setted;
    break;

  case SEGVIO:
    error("segv");
  case BUSERROR:
    error("bus");
  }
#endif
  

#ifdef THREADED
# include "instrtab.hh"
  if (init) {
    CodeArea::init(instrTable);
    return;
  }
# define op (Opcode) -1
#else
  if (init) {
    CodeArea::init(NULL);
    return;
  }
  Opcode op = (Opcode) -1;
#endif

  goto LBLstart;

// ------------------------------------------------------------------------
// *** RUN: Main Loop
// ------------------------------------------------------------------------
 LBLpreemption:

  SaveSelf;
  Assert(CTT->getBoard()==CBB);
  e->scheduleThreadInline(CTT, CPP);
  CTT=0;

 LBLerror:
 LBLstart:

  Assert(CTT==0);

  if (e->isSetSFlag()) {  

    e->deinstallPath(e->rootBoard);

    if (e->isSetSFlag(StartGC)) {
      e->doGC();
    }


    if (e->isSetSFlag(UserAlarm)) {
      osBlockSignals();
      e->handleUser();
      osUnblockSignals();
    }
    if (e->isSetSFlag(IOReady)) {
      osBlockSignals();
      e->handleIO();
      osUnblockSignals();
    }
  }

  /* process switch */
  if (e->threadQueuesAreEmpty()) {
    e->suspendEngine();
  }

LBLinstallThread:
  CTT = e->getFirstThread();
  Assert(CTT);

  PC = NOCODE; // this is necessary for printing stacks (mm)

  DebugTrace (trace("new thread"));

  // Debugger
  if (CTT->stopped()) {
    CTT = 0;  // byebye...
    goto LBLstart;
  }

  //  now, we have *globally* am.currentThread;

  //
  //  No dead threads here;
  // (while, of course, their *tasks* may be dead already;)
  //  So, every runnable thread must be terminated through 
  // 'LBL{discard,kill}Thread', and it should not appear 
  // more than once in the threads pool;
  Assert (!(CTT->isDeadThread ()));
  //
  //  So, every thread in the threads pool *must* be runnable;
  Assert (CTT->isRunnable ());

  /*
   *  Lazy stack allocation;
   *
   *  Note that we cover the case for 'propagator' threads specially, 
   *  since they don't differ in 'suspended' and 'runnable' states;
   */
  if (CTT->isPropagator ()) {
    //
    //  First, get the home board of that propagator, 
    // and try to install it;
    Board *tmpBB = CTT->getBoard();
    DebugTrace (trace ("propagator thread", tmpBB));

    // 
    //  HERE: a special version of INSTALLPATH for propagators;
    //  This is because we have to go for the 'LBLdiscardThread'
    // if the installation is rejected;
    if (CBB != tmpBB) {
      switch (e->installPath (tmpBB)) {
      case INST_OK:
	break;

      case INST_REJECTED:
	//
	//  Note that once a propagator is failed, its thread is 
	// scheduled for execution again (!), and at this place it 
	// should be catched and forwarded to the 
	// 'LBLdiscardThread';
	DebugCode (CTT->removePropagator ());
	goto LBLdiscardThread;

      case INST_FAILED:
	//
	//  The thread must be killed;
	//  So, first go to the 'LBLfailure', and there it must be 
	// scheduled once again - and then the 'INST_REJECTED' case
	// hits ...
	DebugCode (CTT->removePropagator ());
	goto LBLfailure;
      }
    }

    CBB->unsetNervous();

    //    unsigned int starttime = osUserTime();
    switch (CTT->runPropagator()) {
    case SLEEP: 
      CTT->suspendPropagator ();
      if (e->currentSolveBoard != (Board *) NULL) {
	e->decSolveThreads (e->currentSolveBoard);
	//  but it's still "in solve";
      }
      CTT = 0;

      //ozstat.timeForPropagation.incf(osUserTime()-starttime);
      goto LBLstart;

    case SCHEDULED:
      CTT->scheduledPropagator ();
      if (e->currentSolveBoard != (Board *) NULL) {
	e->decSolveThreads (e->currentSolveBoard);
	//  but it's still "in solve";
      }
      CTT = 0;

      //      ozstat.timeForPropagation.incf(osUserTime()-starttime); 
      goto LBLstart;

    case PROCEED:
      // Note: CTT must be reset in 'LBLkillXXX';

      //ozstat.timeForPropagation.incf(osUserTime()-starttime);
      if (e->isToplevel ()) {
	goto LBLkillToplevelThread;
      } else {
	Assert (CTT->isPropagator () || CTT->isEmpty ());
	goto LBLkillThread;
      }

      //  Note that *propagators* never yield 'SUSPEND';
    case FAILED:
      //ozstat.timeForPropagation.incf(osUserTime()-starttime);

      // propagator failure never catched
      if (!e->isToplevel()) goto LBLfailure;

      {
	OZ_Propagator *prop = CTT->getPropagator();
	CTT = e->mkRunnableThread(PROPAGATOR_PRIORITY, CBB);
	e->restartThread();
	HF_APPLY(OZ_atom(builtinTab.getName((void *)(prop->getSpawner()))),
		 prop->getArguments());
      }

    default:
      Assert(0);
      goto LBLerror;
    }
  } else { 
    /*
     * install board
     */
    Board *bb=CTT->getBoard();

    if (CBB != bb) {
      switch (e->installPath(bb)) {
      case INST_OK:
	break;

      case INST_REJECTED:
	goto LBLdiscardThread;

      case INST_FAILED:
	goto LBLfailure;
      }
    }

    CBB->unsetNervous();
    DebugCheckT(currentDebugBoard=CBB);

    //  I.e. it's not a propagator - then just convert it 
    // to a full-fledged thread (with a task stack);
    CTT->makeRunning ();

    //
    e->restartThread();
  }

  //  INVARIANT:
  //  current thread always has a stack, and it might not 
  // be marked as dead;
  Assert(CTT->hasStack ());
  
  goto LBLpopTask;

  /*
   *  Kill a thread at the toplevel - i.e. just dispose it;
   *
   */
LBLkillToplevelThread:
  {
    Assert (CTT);
    Assert (e->isToplevel ());
    asmLbl(killToplevelThread);

    CBB->decSuspCount ();

    if (CTT == e->rootThread) {
      // 
      //  A special case: the "root" thread;
      e->rootThread->reInit (e->rootThread->getPriority (), e->rootBoard);
      CBB->incSuspCount();

      //  Anything else?
      e->checkToplevel ();

      if (e->rootThread->isEmpty ()) {
	//  still empty:
	CTT = 0;
	goto LBLstart;
      } else {
	//  ... no, we have fetched something - go ahead;
	goto LBLpreemption;
      }
    } else if (CTT->isPropagator()) {
      CTT->disposeRunnableThread ();
      CTT = 0;

      goto LBLstart;
    } else {

      CTT->disposeRunnableThread ();
      CTT = (Thread *) NULL;
      
      goto LBLstart;
    }
  }

  Assert(0);
  goto LBLerror;


  /*
   *  Kill the thread - decrement 'suspCounter'"s and counters of 
   * runnable threads in solve actors if any;
   *
   *  This label should be entered from the only place: when 
   * the task stack is detected to be empty in 'LBLpopTask';
   *
   *  Note: this code should ensure the presence of a (non-dead) 
   * runnable non-propagator thread with a stack when the control 
   * is passed to the 'LBLfailure';
   *
   *  Invariants: 
   *  - The thread must be yet alive;
   *  - It might be entered only if the task stack is empty
   *    OR it's a propagator;
   *  - It might not be entered in the toplevel;
   *  - 'CBB' must be alive (and not committed, of course);
   * 
   */
LBLkillThread:
  {
    Thread *tmpThread = CTT;
#ifdef DEBUG_CHECK
    if (CTT== e->rootThread) {
      printf("killThread: root\n");
    }
#endif
    Board *nb = 0;

    DebugTrace (trace ("kill thread", CBB));
    Assert (tmpThread);
    Assert (!(tmpThread->isDeadThread ()));
    Assert (tmpThread->isRunnable ());

    Assert (!(e->isToplevel ()));
    Assert (tmpThread->isPropagator () || tmpThread->isEmpty ());
    //  Note that during debugging the thread does not carry 
    // the board pointer (== NULL) wenn it's running;
    // Assert (CBB == tmpThread->getBoard());
    Assert (CBB != (Board *) NULL);
    Assert (!(CBB->isFailed ()));
    // 
    //  Note that if we are in a solve problem, then 
    // the thread should be marked correspondingly, and vice versa.
    //  In other words, we will test for the presence/absence 
    // of a 'currentSolveBoard' instead of 'isInSolve'!
    Assert (tmpThread->isInSolve () || !e->currentSolveBoard); 
    Assert (e->currentSolveBoard || !(tmpThread->isInSolve ()));
    asmLbl(killThread);

    CTT = (Thread *) NULL;
    tmpThread->disposeRunnableThread ();

    CBB->decSuspCount ();

    /*
     *  General comment - about checking for stability:
     *  In the case when the thread was originated in a solve board, 
     * we have to update the (runnable) threads counter there manually, 
     * check stability there ('AM::checkEntailment ()'), and proceed 
     * with further solve actors upstairs by means of 
     * 'AM::decSolveThreads ()' as usually.
     *  This is because the 'AM::decSolveThreads ()' just generates 
     * wakeups for solve boards where stability is suspected. But 
     * finally the stability check for them should be performed, 
     * and this (and 'LBLsuspendThread') labels are exactly the 
     * right places where it should be done!
     *  Note also that the order of decrementing (runnable) threads 
     * counters in solve actors is also essential: if some solve actor 
     * can be reduced, solve actors above it are getting *instable*
     * because of a new thread!
     *
     *  This code has been originally written by mm2 with some 
     * assistance from me (i.e. kost@). After that, it was fixed 
     * and simplified (in many iterations :-)) as follows.
     *
     */ 

    // 
    //  First, look at the current board, and if it's a solve one, 
    // decrement the (runnable) threads counter manually _and_
    // skip the 'AM::decSolveThreads ()' for it; 
    if (e->currentSolveBoard) {
      if (CBB->isSolve ()) {
	SolveActor *sa;

	//
	sa = SolveActor::Cast (CBB->getActor ());
	//  'nb' points to some board above the current one,
	// so, 'decSolveThreads' will start there!
	nb = sa->getBoard();

	//
	//  kost@ : optimize the most probable case!
	if (sa->decThreads () != 0) {
	  e->decSolveThreads (nb);
	  goto LBLstart;
	}
      } else {
	nb = CBB;
      }
    }

    // 
    //  ... and now, check the entailment here!
    //  Note again that 'decSolveThreads' should be done from 
    // the 'nb' board which is probably modified above!
    // 
    DebugCode (CTT = (Thread *) NULL);
    e->checkEntailment();
    // 
    //  deref nb, because it maybe just committed!
    if (nb) e->decSolveThreads (nb->derefBoard());
    goto LBLstart;
  }


  /*
   *  Discard the thread, i.e. just decrement solve thread counters 
   * everywhere it is needed, and dispose the body; 
   *  The main difference to the 'LBLkillThread' is that no 
   * entailment can be reached here, because it's tested already 
   * when the failure was processed;
   * 
   *  Invariants:
   *  - a runnable thread must be there;
   *  - the task stack must be empty (for proper ones), or
   *    it must be already marked as not having the propagator
   *    (in dedug mode, for propagators);
   *  - the home board of the thread must be failed;
   *
   */
LBLdiscardThread:
  {
    Thread *tmpThread = CTT;

    Assert (tmpThread);
    Assert (!(tmpThread->isDeadThread ()));
    Assert (tmpThread->isRunnable ());

    asmLbl(discardThread);

    //
    //  Note that we may not use the 'currentSolveBoard' test here,
    // because it may point to an irrelevant board!
    if (tmpThread->isInSolve ()) {
      Board *tmpBB = tmpThread->getBoard();

      if (tmpBB->isSolve ()) {
	//
	//  The same technique as by 'LBLkillThread';
	SolveActor *sa = SolveActor::Cast (tmpBB->getActor ());
	Assert (sa);
	Assert (sa->getSolveBoard () == tmpBB);

	e->decSolveThreads (sa->getBoard());
      } else {
	e->decSolveThreads (tmpBB);
      }
    }
    CTT = (Thread *) NULL;
    tmpThread->disposeRunnableThread ();

    goto LBLstart;
  }


  /*
   *  Suspend the thread in a generic way. It's used when something 
   * suspends in the sequential mode;
   *  Note that the thread stack should already contain the 
   * "suspended" task; 
   *
   *  Note that this code might be entered not only from within 
   * the toplevel, so, it has to handle everything correctly ...
   *
   *  Invariants:
   *  - CBB must be alive;
   *
   */
LBLsuspendThread:
  {
    Board *nb = 0;

    DebugTrace (trace("suspend runnable thread", CBB));

    Assert (CTT);
    CTT->unmarkPropagated();

    Assert (CBB);
    Assert (!(CBB->isFailed ()));
    //  see the note for the 'LBLkillThread';
    Assert (CTT->isInSolve () || !e->currentSolveBoard); 
    Assert (e->currentSolveBoard || !(CTT->isInSolve ()));

    asmLbl(suspendThread);

    //
    //  First, set the board and self, and perform special action for 
    // the case of blocking the root thread;
    Assert(CTT->getBoard()==CBB);
    SaveSelf;

#ifdef DEBUG_CHECK
    if (CTT==e->rootThread) {
      // printf("root blocked\n");
    }
#endif

    CTT = (Thread *) NULL;

    //  No counter decrement 'cause the thread is still alive!

    if (CBB->isRoot ()) {
      //
      //  Note that in this case no (runnable) thread counters 
      // in solve actors can be affected, just because this is 
      // a top-level thread;
      goto LBLstart;
    }

    // 
    //  So, from now it's somewhere in a deep guard;
    Assert (!(e->isToplevel ()));

    if (e->currentSolveBoard) {
      if (CBB->isSolve ()) {
	SolveActor *sa;

	//
	sa = SolveActor::Cast (CBB->getActor ());
	//  'nb' points to some board above the current one,
	// so, 'decSolveThreads' will start there!
	nb = sa->getBoard();

	//
	//  kost@ : optimize the most probable case!
	if (sa->decThreads () != 0) {
	  e->decSolveThreads (nb);
	  goto LBLstart;
	}
      } else {
	nb = CBB;
      }
    }

    // 
    DebugCode (CTT = (Thread *) NULL);
    e->checkEntailment();
    // 
    if (nb) e->decSolveThreads (nb->derefBoard());
    goto LBLstart;
  }

//
// ----------------- end killThread----------------------------------------
  
// ------------------------------------------------------------------------
// *** Emulate: execute continuation
// ------------------------------------------------------------------------
 LBLemulate:
  Assert(CBB==currentDebugBoard);

  JUMP( PC );

#ifndef THREADED
LBLdispatcher:

#ifdef RECINSTRFETCH
  CodeArea::recordInstr(PC);
#endif

  DebugTrace( if (!trace("emulate",CBB,CAA,PC,Y,G)) {
		goto LBLfailure;
	      });

  op = CodeArea::getOP(PC);
  // displayCode(PC,1);
  switch (op) {
#endif
    
// the instructions are classified into groups
// to find a certain class of instructions search for the String "CLASS:"
// -------------------------------------------------------------------------
// CLASS: TERM: MOVE/UNIFY/CREATEVAR/...
// -------------------------------------------------------------------------

#include "register.hh"

// ------------------------------------------------------------------------
// CLASS: (Fast-) Call/Execute Inline Funs/Rels
// ------------------------------------------------------------------------

  Case(FASTCALL)
    {
      CallPushCont(PC+3);
      goto LBLFastTailCall;
    }


  Case(FASTTAILCALL)
  LBLFastTailCall:
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);

      CallDoChecks(entry->getAbstr(),entry->getGRegs());

      IHashTable *table = entry->indexTable;
      if (table) {
	DoSwitchOnTerm(X[0],table);
      } else {
	JUMP(entry->getPC());
      }
    }

  Case(CALLBUILTIN)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      biFun = entry->getFun();
      predArity = getPosIntArg(PC+2);

      CheckArity(entry->getArity(),makeTaggedConst(entry));

      switch (biFun(predArity, X)){

      case PROCEED:	  DISPATCH(3);
      case FAILED:	  HF_BI;
      case RAISE:	  goto LBLraise;
      case BI_TYPE_ERROR: RAISE_TYPE;
      case BI_TERMINATE:  goto LBLpopTask;
	 
      case SUSPEND:
	e->pushTask(PC,Y,G,X,predArity);
	e->suspendOnVarList(CTT);
	goto LBLsuspendThread;

      case BI_PREEMPT:
	e->pushTask(PC+3,Y,G);
	goto LBLpreemption;

      case SLEEP:
      default:
	Assert(0);
      }
    }


  Case(INLINEREL1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel1 rel         = (InlineRel1)entry->getInlineFun();

  
      switch(rel(XPC(2))) {
      case PROCEED:
	DISPATCH(4);

      case SUSPEND:
	if (shallowCP) {
	  e->trail.pushIfVar(XPC(2));
	  goto LBLsuspendShallow;
	}
	e->pushTask(PC,Y,G,X,getPosIntArg(PC+3));
	e->suspendInline(1,XPC(2));
	goto LBLsuspendThread;
      case FAILED:
	SHALLOWFAIL;
	HF_APPLY(OZ_atom(entry->getPrintName()),
		 cons(XPC(2),nil()));

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(entry->getPrintName(),
		    cons(XPC(2),nil()));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEREL2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel2 rel         = (InlineRel2)entry->getInlineFun();

      switch(rel(XPC(2),XPC(3))) {
      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
	{
	  if (shallowCP) {
	    e->trail.pushIfVar(XPC(2));
	    e->trail.pushIfVar(XPC(3));
	    goto LBLsuspendShallow;
	  }

	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	  e->suspendInline(2,XPC(2),XPC(3));
	  goto LBLsuspendThread;
	}
      case FAILED:
	SHALLOWFAIL;
	HF_APPLY(OZ_atom(entry->getPrintName()),
		 cons(XPC(2),cons(XPC(3),nil())));

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(entry->getPrintName(),
		    cons(XPC(2),cons(XPC(3),nil())));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEREL3)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel3 rel         = (InlineRel3)entry->getInlineFun();

      switch(rel(XPC(2),XPC(3),XPC(4))) {
      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  if (shallowCP) {
	    e->trail.pushIfVar(XPC(2));
	    e->trail.pushIfVar(XPC(3));
	    e->trail.pushIfVar(XPC(4));
	    goto LBLsuspendShallow;
	  }

	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+5));
	  e->suspendInline(3,XPC(2),XPC(3),XPC(4));
	  goto LBLsuspendThread;
	}
      case FAILED:
	SHALLOWFAIL;
	HF_APPLY(OZ_atom(entry->getPrintName()),
		 cons(XPC(2),cons(XPC(3),cons(XPC(4),nil()))));

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(entry->getPrintName(),
		    cons(XPC(2),cons(XPC(3),cons(XPC(4),nil()))));


      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEFUN1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun1 fun         = (InlineFun1)entry->getInlineFun();

      // XPC(3) maybe the same register as XPC(2)
      switch(fun(XPC(2),XPC(3))) {
      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    goto LBLsuspendShallow;
	  }
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	  e->suspendInline(1,A);
	  goto LBLsuspendThread;
	}

      case FAILED:
	SHALLOWFAIL;
	Assert(0);

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1_FUN(entry->getPrintName(),
			cons(XPC(2),nil()));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEFUN2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun2 fun = (InlineFun2)entry->getInlineFun();

      
      // note: XPC(4) is maybe the same as XPC(2) or XPC(3) !!
      switch(fun(XPC(2),XPC(3),XPC(4))) {
      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  TaggedRef B=XPC(3);
	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    e->trail.pushIfVar(B);
	    goto LBLsuspendShallow;
	  }
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+5));
	  e->suspendInline(2,A,B);
	  goto LBLsuspendThread;
	}

      case FAILED:
	SHALLOWFAIL;
	HF_APPLY(OZ_atom(entry->getPrintName()),
		 cons(XPC(2),cons(XPC(3),cons(OZ_newVariable(), nil()))));
	//	error("inlinefun2 fail");

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1_FUN(entry->getPrintName(),
			cons(XPC(2),cons(XPC(3),nil())));

      case SLEEP:
      default:
	Assert(0);
      }
     }

  Case(INLINEDOT)
    {
      TaggedRef feature = getLiteralArg(PC+2);
      TaggedRef rec = XPC(1);
      DEREF(rec,_1,_2);
      if (isSRecord(rec)) {
	SRecord *srec = tagged2SRecord(rec);
	int index = ((InlineCache*)(PC+5))->lookup(srec,feature);
	if (index<0) {
	  (void) e->raise(E_ERROR,E_KERNEL,".", 2, XPC(1), feature);
	  goto LBLraise;
	}
	XPC(3) = srec->getArg(index);
	DISPATCH(7);	
      }
      {
	switch(dotInline(XPC(1),feature,XPC(3))) {
	case PROCEED: DISPATCH(7);
	case FAILED:
	  Assert(0);

	case SUSPEND:
	  {
	    TaggedRef A=XPC(1);
	    if (shallowCP) {
	      e->trail.pushIfVar(A);
	      goto LBLsuspendShallow;
	    }
	    e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	    e->suspendInline(1,A);
	    goto LBLsuspendThread;
	  }

	case RAISE:
	  goto LBLraise;

	case BI_TYPE_ERROR:
	  RAISE_TYPE1_FUN(".", cons(XPC(1), cons(feature, nil())));

	case SLEEP:
	default:
	  Assert(0);
	}
      }
    }

  Case(INLINEAT)
    {
      TaggedRef fea = getLiteralArg(PC+1);

      Assert(e->getSelf()!=NULL);
      SRecord *rec = e->getSelf()->getState();
      if (rec) {
	int index = ((InlineCache*)(PC+4))->lookup(rec,fea);
	if (index>=0) {
	  XPC(2) = rec->getArg(index);
	  DISPATCH(6);
	}
      }
      (void) e->raise(E_ERROR,E_OBJECT,"@",2,
		      rec?makeTaggedSRecord(rec):OZ_atom("noattributes"),fea);
      goto LBLraise;
    }

  Case(INLINEASSIGN)
    {      
      TaggedRef fea = getLiteralArg(PC+1);

      SRecord *rec = e->getSelf()->getState();
      if (rec) {
	int index = ((InlineCache*)(PC+4))->lookup(rec,fea);
	if (index>=0) {
	  Assert(isRef(*rec->getRef(index)) || !isAnyVar(*rec->getRef(index)));
	  rec->setArg(index,XPC(2));
	  DISPATCH(6);
	}
      }
      
      (void) e->raise(E_ERROR,E_OBJECT,"<-",3,
		      rec?makeTaggedSRecord(rec) :OZ_atom("noattributes"),
		      fea, XPC(2));
      goto LBLraise;
    }

  Case(INLINEUPARROW)
    {
      switch(uparrowInlineBlocking(XPC(1),XPC(2),XPC(3))) {
      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
	  Assert(!shallowCP);
	  OZ_suspendOnInternal2(XPC(1),XPC(2));
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	  e->suspendOnVarList(CTT);
	  goto LBLsuspendThread;

      case FAILED:
	HF_APPLY(OZ_atom("^"),
		 cons(XPC(1),cons(XPC(2),nil())));

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1_FUN("^",cons(XPC(1),cons(XPC(2),nil())));

      case SLEEP:
      default:
	Assert(0);
      }
    }


  Case(INLINEFUN3)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun3 fun = (InlineFun3)entry->getInlineFun();

      // note XPC(5) is maybe the same as XPC(2) or XPC(3) or XPC(4) !!
      switch(fun(XPC(2),XPC(3),XPC(4),XPC(5))) {
      case PROCEED:
	DISPATCH(7);

      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  TaggedRef B=XPC(3);
	  TaggedRef C=XPC(4);
	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    e->trail.pushIfVar(B);
	    e->trail.pushIfVar(C);
	    goto LBLsuspendShallow;
	  }
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+6));
	  e->suspendInline(3,A,B,C);
	  goto LBLsuspendThread;
	}

      case FAILED:
	SHALLOWFAIL;
	Assert(0);

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1_FUN(entry->getPrintName(),
			cons(XPC(2),cons(XPC(3),cons(XPC(4),nil()))));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEEQEQ)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun2 fun = (InlineFun2)entry->getInlineFun();

      // note XPC(4) is maybe the same as XPC(2) or XPC(3) !!
      switch (fun(XPC(2),XPC(3),XPC(4))) {
      case PROCEED:
	DISPATCH(6);
      case SUSPEND:
	{
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+5));
	  e->suspendOnVarList(CTT);
	  goto LBLsuspendThread;
	}

      case RAISE:
      case FAILED:
      case SLEEP:
      default:
	Assert(0);
      }
    }

#undef SHALLOWFAIL

// ------------------------------------------------------------------------
// CLASS: Shallow guards stuff
// ------------------------------------------------------------------------

  Case(SHALLOWGUARD)
    {
      shallowCP = PC;
      e->trail.pushMark();
      DISPATCH(3);
    }

  Case(SHALLOWTEST1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel1 rel         = (InlineRel1)entry->getInlineFun();

      switch(rel(XPC(2))) {

      case PROCEED: DISPATCH(5);
      case FAILED:  JUMP( getLabelArg(PC+3) );
      case SUSPEND:
	e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	addSusp (XPC(2), CTT);
	goto LBLsuspendThread;

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(entry->getPrintName(),
		    cons(XPC(2),nil()));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(SHALLOWTEST2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel2 rel         = (InlineRel2)entry->getInlineFun();

      switch(rel(XPC(2),XPC(3))) {

      case PROCEED: DISPATCH(6);
      case FAILED:  JUMP( getLabelArg(PC+4) );

      case SUSPEND:
	{
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+5));
	  OZ_Term A=XPC(2);
	  OZ_Term B=XPC(3);
	  DEREF(A,APtr,ATag); DEREF(B,BPtr,BTag);
	  Assert(isAnyVar(ATag) || isAnyVar(BTag));
	  if (isAnyVar (A)) addSusp(APtr, CTT);
	  if (isAnyVar (B)) addSusp(BPtr, CTT);
	  goto LBLsuspendThread;
	}

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(entry->getPrintName(),
		    cons(XPC(2),cons(XPC(3),nil())));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(SHALLOWTHEN)
    {
      if (e->trail.isEmptyChunk()) {
	shallowCP = NULL;
	e->trail.popMark();
	DISPATCH(1);
      }

    LBLsuspendShallow:
      {
	e->emptySuspendVarList();
	int argsToSave = getPosIntArg(shallowCP+2);
	e->pushTask(shallowCP,Y,G,X,argsToSave);
	shallowCP = NULL;
	e->reduceTrailOnShallow(CTT);
	goto LBLsuspendThread;
      }
    }

// -------------------------------------------------------------------------
// CLASS: Environment
// -------------------------------------------------------------------------

  Case(ALLOCATEL)
    {
      int posInt = getPosIntArg(PC+1);
      Assert(posInt > 0);
      Y = allocateY(posInt);
      DISPATCH(2);
    }

  Case(ALLOCATEL1)  { Y =  allocateY(1); DISPATCH(1); }
  Case(ALLOCATEL2)  { Y =  allocateY(2); DISPATCH(1); }
  Case(ALLOCATEL3)  { Y =  allocateY(3); DISPATCH(1); }
  Case(ALLOCATEL4)  { Y =  allocateY(4); DISPATCH(1); }
  Case(ALLOCATEL5)  { Y =  allocateY(5); DISPATCH(1); }
  Case(ALLOCATEL6)  { Y =  allocateY(6); DISPATCH(1); }
  Case(ALLOCATEL7)  { Y =  allocateY(7); DISPATCH(1); }
  Case(ALLOCATEL8)  { Y =  allocateY(8); DISPATCH(1); }
  Case(ALLOCATEL9)  { Y =  allocateY(9); DISPATCH(1); }
  Case(ALLOCATEL10) { Y = allocateY(10); DISPATCH(1); }

  Case(DEALLOCATEL)
    {
      Assert(!isFreedRefsArray(Y));
      if (Y) {
	deallocateY(Y);
	Y = NULL;
      }
      DISPATCH(1);
    }
// -------------------------------------------------------------------------
// CLASS: CONTROL: FAIL/SUCCESS/RETURN/SAVECONT
// -------------------------------------------------------------------------

  Case(FAILURE)
    {
      HF_FAIL;
    }


  Case(SUCCEED)
    DISPATCH(1);

  Case(SAVECONT)
    /*     e->pushTask(getLabelArg(PC+1),Y,G); */
    error("unused");
    DISPATCH(2);

  Case(EXHANDLER)
    e->pushTask(PC+2,Y,G);
    e->currentThread->pushCatch();
    JUMP(getLabelArg(PC+1));

  Case(POPEX)
    {
      TaskStack *taskstack = CTS;
      taskstack->discardFrame(C_CATCH_Ptr);
      /* remove unused continuation for handler */
      taskstack->discardFrame(NOCODE);
      DISPATCH(1);
    }

  Case(LOCKTHREAD)
    {
      ProgramCounter lbl = getLabelArg(PC+1);
      int reg      = regToInt(getRegArg(PC+2));
      int toSave   = getPosIntArg(PC+3);

      TaggedRef aux = X[reg];
      DEREF(aux,auxPtr,_1);
      if (isAnyVar(aux)) {
	SUSP_PC(auxPtr,toSave,PC);
      }
      
      if (!isLock(aux)) {
	(void) e->raise(E_ERROR,E_OBJECT,"attempt to lock closed object",0);
	goto LBLraise;
      }

      OzLock *lck = tagged2Lock(aux);
      if (!e->isToplevel()) {
	if (e->currentBoard != lck->getBoard()) {
	  (void) e->raise(E_ERROR,E_OBJECT,"attempt to lock object in guard",0);
	  goto LBLraise;
	}
      }
      
      Thread *cs = e->currentThread;
      if (lck->isLocked(cs)) {
	e->pushTask(lbl,Y,G);
	DISPATCH(4);
      }
      TaggedRef *var = lck->lock(cs);
      if (var==NULL) {
	e->pushTask(lbl,Y,G);
	CTS->pushLock(lck);
	DISPATCH(4);
      }
	
      SUSP_PC(var,toSave,PC);
    }

  Case(RETURN)
    LBLpopTask:
      {
	Assert(!CTT->isSuspended());
	Assert(CBB==currentDebugBoard);

	emulateHookPopTask(e, goto LBLpreemption);

	DebugCheckT(CAA = NULL);

      LBLpopTaskNoPreempt:
	TaskStack *taskstack     = CTS;
	TaskStackEntry *topCache = taskstack->getTop();
	PopFrameNoDecl(topCache,PC,Y,G);
	taskstack->setTop(topCache);
	JUMP(PC);
      }



// ------------------------------------------------------------------------
// CLASS: Definition
// ------------------------------------------------------------------------

  Case(DEFINITION)
    {
      Reg reg                     = getRegArg(PC+1);
      ProgramCounter nxt          = getLabelArg(PC+2);
      PrTabEntry *predd           = getPredArg(PC+3);
      AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+4);
      AssRegArray *list           = (AssRegArray*) getAdressArg(PC+5);
      
      
      int size = list->getSize();
      RefsArray gRegs = (size == 0) ? (RefsArray) NULL : allocateRefsArray(size);

      Abstraction *p = new Abstraction (predd, gRegs, CBB);

      if (predEntry) {
	predEntry->setPred(p);
      }

      for (int i = 0; i < size; i++) {
	switch ((*list)[i].kind) {
	case XReg: gRegs[i] = X[(*list)[i].number]; break;
	case YReg: gRegs[i] = Y[(*list)[i].number]; break;
	case GReg: gRegs[i] = G[(*list)[i].number]; break;
	}
      }
      Xreg(reg) = makeTaggedConst(p);
      JUMP(nxt);
    }

// -------------------------------------------------------------------------
// CLASS: CONTROL: FENCE/CALL/EXECUTE/SWITCH/BRANCH
// -------------------------------------------------------------------------
  
  Case(BRANCH)
    JUMP( getLabelArg(PC+1) );

  
  /*
   * weakDet(X) woken up and continues with next instruction,
   * WHENEVER something happens to X, even if variable is bound to another var
   */
  Case(WEAKDETX) ONREG(WeakDet,X);
  Case(WEAKDETY) ONREG(WeakDet,Y);
  Case(WEAKDETG) ONREG(WeakDet,G);
  WeakDet:
  {
    TaggedRef term = RegAccess(HelpReg,getRegArg(PC+1));
    DEREF(term,termPtr,tag);

    if (!isAnyVar(tag)) {
      DISPATCH(3);
    }
    int argsToSave = getPosIntArg(PC+2);

    INCFPC(3); /* suspend on NEXT instructions: WeakDET suspensions are
                  woken up always, even if variable is bound to another var */

    SUSP_PC(termPtr,argsToSave,PC);
  }


  /* det(X) wait until X will be ground */
  Case(DETX) ONREG(Det,X);
  Case(DETY) ONREG(Det,Y);
  Case(DETG) ONREG(Det,G);
  Det:
  {
    TaggedRef term = RegAccess(HelpReg,getRegArg(PC+1));
    DEREF(term,termPtr,tag);

    if (!isAnyVar(tag)) {
      DISPATCH(3);
    }
    /* INCFPC(3): dont do it */
    int argsToSave = getPosIntArg(PC+2);
    e->pushTask(PC,Y,G,X,argsToSave);
    if (isCVar (tag)) {
      (tagged2CVar (term))->addDetSusp(CTT);
    } else {
      addSusp (termPtr, CTT);
    }
    goto LBLsuspendThread;
  }

  Case(TAILSENDMSGX) isTailCall = OK; ONREG(SendMethod,X);
  Case(TAILSENDMSGY) isTailCall = OK; ONREG(SendMethod,Y);
  Case(TAILSENDMSGG) isTailCall = OK; ONREG(SendMethod,G);

  Case(SENDMSGX) isTailCall = NO; ONREG(SendMethod,X);
  Case(SENDMSGY) isTailCall = NO; ONREG(SendMethod,Y);
  Case(SENDMSGG) isTailCall = NO; ONREG(SendMethod,G);

 SendMethod:
  {
    TaggedRef label    = getLiteralArg(PC+1);
    TaggedRef origObj  = RegAccess(HelpReg,getRegArg(PC+2));
    TaggedRef object   = origObj;
    SRecordArity arity = (SRecordArity) getAdressArg(PC+3);

    /* compiler ensures: if object is in X[n], then n == arity+1,
     * so in case we have to suspend we save one additional argument */
    Assert(HelpReg!=X || getWidth(arity)==regToInt(getRegArg(PC+2)));

    DEREF(object,objectPtr,_2);
    if (isObject(object)) {
      Object *obj      = (Object *) tagged2Const(object);
      Abstraction *def = getSendMethod(obj,label,arity,(InlineCache*)(PC+4),X);
      if (def == NULL) {
	goto bombSend;
      }

      if (!isTailCall) CallPushCont(PC+6);
      ChangeSelf(obj);
      CallDoChecks(def,def->getGRegs());
      JUMP(def->getPC());
    }

    if (isAnyVar(object)) {
      SUSP_PC(objectPtr,getWidth(arity)+1,PC);
    }

    if (isProcedure(object)) 
      goto bombSend;

    RAISE_APPLY(object, cons(makeMessage(arity,label,X),nil()));

  bombSend:
    if (!isTailCall) PC = PC+6;
    X[0] = makeMessage(arity,label,X);
    predArity = 1;
    predicate = tagged2Const(object);
    goto LBLcall;
  }


  Case(TAILAPPLMETHX) isTailCall = OK; ONREG(ApplyMethod,X);
  Case(TAILAPPLMETHY) isTailCall = OK; ONREG(ApplyMethod,Y);
  Case(TAILAPPLMETHG) isTailCall = OK; ONREG(ApplyMethod,G);

  Case(APPLMETHX) isTailCall = NO; ONREG(ApplyMethod,X);
  Case(APPLMETHY) isTailCall = NO; ONREG(ApplyMethod,Y);
  Case(APPLMETHG) isTailCall = NO; ONREG(ApplyMethod,G);

 ApplyMethod:
  {
    ApplMethInfoClass *ami = (ApplMethInfoClass*) getAdressArg(PC+1);
    SRecordArity arity     = ami->arity;
    TaggedRef origObject   = RegAccess(HelpReg,getRegArg(PC+2));
    TaggedRef object       = origObject;
    Abstraction *def       = NULL;

    if (!isTailCall) PC = PC+3;

    DEREF(object,objectPtr,objectTag);
    if (!isObject(object)) {
      goto bombApply;
    }
    def = getApplyMethod((Object *) tagged2Const(object),ami,arity,X);
    if (def==NULL) {
      goto bombApply;
    }
    
    if (!isTailCall) { CallPushCont(PC); }
    CallDoChecks(def,def->getGRegs());
    JUMP(def->getPC());


  bombApply:
    if (e->methApplHdl == makeTaggedNULL()) {
      error("no apply handler");
    }

    X[1] = makeMessage(arity,ami->methName,X);
    X[0] = origObject;

    predArity = 2;
    predicate = tagged2Const(e->methApplHdl);
    goto LBLcall;
  }


  Case(CALLX) isTailCall = NO; ONREG(Call,X);
  Case(CALLY) isTailCall = NO; ONREG(Call,Y);
  Case(CALLG) isTailCall = NO; ONREG(Call,G);

  Case(TAILCALLX) isTailCall = OK; ONREG(Call,X);
  Case(TAILCALLY) isTailCall = OK; ONREG(Call,Y);
  Case(TAILCALLG) isTailCall = OK; ONREG(Call,G);

 Call:
   {
     {
       TaggedRef taggedPredicate = RegAccess(HelpReg,getRegArg(PC+1));
       predArity = getPosIntArg(PC+2);

       DEREF(taggedPredicate,predPtr,predTag);
       if (!isProcedure(taggedPredicate) && !isObject(taggedPredicate)) {
	 if (isAnyVar(predTag)) {
	   /* compiler ensures: if pred is in X[n], then n == arity+1,
	    * so we save one additional argument */
	   Assert(HelpReg!=X || predArity==regToInt(getRegArg(PC+1)));
	   SUSP_PC(predPtr,predArity+1,PC);
	 }
	 RAISE_APPLY(taggedPredicate, OZ_toList(predArity,X));
       }

       if (!isTailCall) PC = PC+3;
       predicate = tagged2Const(taggedPredicate);
     }

// -----------------------------------------------------------------------
// --- Call: entry point
// -----------------------------------------------------------------------

  LBLcall:
     BuiltinTabEntry *bi;

// -----------------------------------------------------------------------
// --- Call: Abstraction
// -----------------------------------------------------------------------

     {
       TypeOfConst typ = predicate->getType();

       if (typ==Co_Abstraction || typ==Co_Object) {
	 Abstraction *def;
	 if (typ==Co_Object) {
	   Object *o = (Object*) predicate;
	   if (o->isClass()) {
	     RAISE_APPLY(makeTaggedConst(predicate),
			 OZ_toList(predArity,X));
	   }
	   CheckArity(1,makeTaggedConst(o));
	   def = o->getAbstraction();
	   /* {Obj Msg} --> {Send Msg Methods Obj} */
	   X[predArity++] = makeTaggedConst(o->getSlowMethods());
	   X[predArity++] = makeTaggedConst(o);
	   if (!isTailCall) { 
	     CallPushCont(PC);
	   }
	   ChangeSelf(o); 
	 } else {
	   def = (Abstraction *) predicate;
	   CheckArity(def->getArity(), makeTaggedConst(def));
	   if (!isTailCall) { CallPushCont(PC); }
	 }
       
	 CallDoChecks(def,def->getGRegs());
	 JUMP(def->getPC());
       }


// -----------------------------------------------------------------------
// --- Call: Builtin
// -----------------------------------------------------------------------
       Assert(typ==Co_Builtin);

       bi = (BuiltinTabEntry *) predicate;
	   
       CheckArity(bi->getArity(),makeTaggedConst(bi));
	   
       if (e->breakflag) {
	 e->currentThread->startStepMode();
         e->currentThread->traced();
	 e->breakflag = NO;
	 debugStreamThread(e->currentThread);
       }

       if (e->debugmode() && e->currentThread->stepMode()) {
	 //enterCall(CBB,makeTaggedConst(bi),predArity,X);
#ifndef STEPWITHDEBUGINFO
	 //Board *b = e->currentBoard; 
	 //e->currentBoard = e->rootBoard;
      
	 ProgramCounter dbg = CodeArea::nextDebugInfo(PC);
	 if (dbg != NOCODE) {

	   TaggedRef tail    = e->threadStreamTail;
	   TaggedRef newTail = OZ_newVariable();
	   
	   TaggedRef file, comment;
	   int line, abspos;
		   
	   e->currentThread->stop();
	   CodeArea::getDebugInfoArgs(dbg,file,line,abspos,comment);
      
	   TaggedRef pairlist = 
	     OZ_cons(OZ_pairA("thr",
			      OZ_mkTupleC("#",2,makeTaggedConst(CTT),
					  OZ_int(CTT->getID()))),
		     OZ_cons(OZ_pairA("file", file),
			     OZ_cons(OZ_pairAI("line", line),
				     OZ_nil())));
	   
	   TaggedRef entry = OZ_recordInit(OZ_atom("step"), pairlist);
	   OZ_unify(tail, OZ_cons(entry, newTail));
	   e->threadStreamTail = newTail;
	   
	   //e->currentBoard = b;
	   
	   if (!isTailCall) e->pushTask(PC,Y,G);
	   e->pushCFun(bi->getFun(),X,predArity);     

	   goto LBLpreemption;
	 }
#endif
       }

       biFun=bi->getFun();
       OZ_Return res = biFun(predArity, X);
	     
       //if (e->isSetSFlag(DebugMode)) {
       //exitBuiltin(res,makeTaggedConst(bi),predArity,X);
       //}

       switch (res) {
	    
       case SUSPEND:
	 {
	   if (!isTailCall) e->pushTask(PC,Y,G);
	   e->pushCFun(biFun,X,predArity);
	   e->suspendOnVarList(CTT);
	   goto LBLsuspendThread;
	 }

       case PROCEED:
	 if (isTailCall) {
	   goto LBLpopTask;
	 }
	 JUMP(PC);
	 
       case SLEEP:         Assert(0);
       case RAISE:         goto LBLraise;
       case BI_TYPE_ERROR: RAISE_TYPE;
       case BI_TERMINATE:  goto LBLpopTask;
       case FAILED:        HF_BI;

       case BI_PREEMPT:
	 if (!isTailCall) {
	   e->pushTask(PC,Y,G);
	 }
	 goto LBLpreemption;
	 
       default: Assert(0);
       }
     }
// ------------------------------------------------------------------------
// --- Call: Builtin: raise
// ------------------------------------------------------------------------

   LBLraise:
     {
       DebugCheck(ozconf.stopOnToplevelFailure,
		  DebugTrace(tracerOn();trace("raise")));

       Assert(CTT && !CTT->isPropagator());

       shallowCP = 0; // failure in shallow guard can never be handled

       TaskStackEntry *lastTop=CTT->getTop();
       Bool foundHdl = CTT->findCatch();
       /* topmost entry points to handler now */

       if (e->exception.debug) {
	 OZ_Term traceBack = CTT->reflect(lastTop,CTT->getTop(),PC);
	 OZ_Term loc = e->dbgGetLoc(CBB);
	 e->formatError(traceBack,loc);
       }

       if (foundHdl) {
	 X[0] = e->exception.value;
	 goto LBLpopTask;
       }

       if (!e->isToplevel() &&
	   OZ_eq(OZ_label(e->exception.value),OZ_atom("failure"))) {
	 goto LBLfailure;
       }

       RefsArray argsArray = allocateRefsArray(1,NO);
       argsArray[0] = e->exception.value;
       CTT->pushCall(e->defaultExceptionHandler,argsArray,1);
       goto LBLpopTask;
     }
   }

// --------------------------------------------------------------------------
// --- end call/execute -----------------------------------------------------
// --------------------------------------------------------------------------

  
// -------------------------------------------------------------------------
// CLASS: Actors/Deep Guards
// -------------------------------------------------------------------------

  Case(WAIT)
    {
      CBB->setWaiting();
      CBB->decSuspCount();
      goto LBLsuspendBoard;
    }

  Case(WAITTOP)
    {
      CBB->decSuspCount();

      if ( e->entailment() ) {

	CTS->discardFrame(C_ACTOR_Ptr);

	e->trail.popMark();

	Board *tmpBB = CBB;

	e->setCurrent(CBB->getParent());
	DebugCheckT(currentDebugBoard=CBB);
	tmpBB->unsetInstalled();
	tmpBB->setCommitted(CBB);
	CBB->decSuspCount();

	DISPATCH(1);
      }
      CBB->setWaiting();
      CBB->setWaitTop();
      goto LBLsuspendBoard;
    }


  Case(ASK)
    {
      CBB->decSuspCount();

      // entailment ?
      if (e->entailment()) {
	CTS->discardFrame(C_ACTOR_Ptr);
	e->trail.popMark();
	Board *tmpBB = CBB;
	e->setCurrent(CBB->getParent());
	DebugCheckT(currentDebugBoard=CBB);
	tmpBB->unsetInstalled();
	tmpBB->setCommitted(CBB);
	CBB->decSuspCount();
	DISPATCH(1);
      }

    LBLsuspendBoard:
      CBB->setBody(PC+1, Y, G,NULL,0);
      Assert(CAA == AWActor::Cast (CBB->getActor()));

      e->deinstallCurrent();
      DebugCode(currentDebugBoard=CBB);

      // optimization for most usual case
      if (CAA->hasNext()) {
	LOADCONT(CAA->getNext());
	goto LBLemulate; // no thread switch allowed here (CAA)
      }

      goto LBLpopTask;
    }


  Case(CREATECOND)
    {
      ProgramCounter elsePC = getLabelArg(PC+1);
      int argsToSave = getPosIntArg(PC+2);

      CAA = new AskActor(CBB,CPP,CTT,
			 elsePC ? elsePC : NOCODE,
			 NOCODE, Y, G, X, argsToSave);
      CTS->pushActor(CAA);
      DISPATCH(3);
    }

  Case(CREATEOR)
    {
      CAA = new WaitActor(CBB, CPP, CTT, NOCODE, Y, G, X, 0, NO);
      CTS->pushActor(CAA);

      DISPATCH(1);
    }

  Case(CREATEENUMOR)
    {
      Board *bb = CBB;

      CAA = new WaitActor(bb, CPP, CTT, NOCODE, Y, G, X, 0, NO);
      CTS->pushActor(CAA);

      if (bb->isWait()) {
	WaitActor::Cast(bb->getActor())->pushChoice((WaitActor *) CAA);
      } else if (bb->isSolve()) {
	SolveActor::Cast(bb->getActor())->pushChoice((WaitActor *) CAA);
      }

      DISPATCH(1);
    }

  Case(CREATECHOICE)
    {
      Board *bb = CBB;

      CAA = new WaitActor(bb, CPP, CTT, NOCODE, Y, G, X, 0, OK);
      CTS->pushActor(CAA);

      if (bb->isWait()) {
	WaitActor::Cast(bb->getActor())->pushChoice((WaitActor *) CAA);
      } else if (bb->isSolve()) {
	SolveActor::Cast(bb->getActor())->pushChoice((WaitActor *) CAA);
      }

      DISPATCH(1);
    }

  Case(CLAUSE)
    {

      Board *bb = new Board(CAA,CAA->isAsk()?Bo_Ask:Bo_Wait);
      e->setCurrent(bb,OK);
      CBB->incSuspCount();
      DebugCode(currentDebugBoard=CBB);
      CBB->setInstalled();
      e->trail.pushMark();
      Assert(CAA->getThread()==CTT);
      DISPATCH(1);
    }

  Case(NEXTCLAUSE)
      CAA->nextClause(getLabelArg(PC+1));
      DISPATCH(2);

  Case(LASTCLAUSE)
      CAA->lastClause();
      DISPATCH(1);

  Case(THREAD)
    {
      ProgramCounter newPC = PC+2;
      ProgramCounter contPC = getLabelArg(PC+1);

      int prio = CPP;

      if (prio > DEFAULT_PRIORITY) {
	prio = DEFAULT_PRIORITY;
      }

#ifdef LINKEDTHREADS
      Bool link = am.currentThread->getID() < 0x80000000;
      Thread *tt = e->mkRunnableThread(prio, CBB, NO, link);
#else
      Thread *tt = e->mkRunnableThread(prio, CBB);
#endif

      ozstat.createdThreads.incf();
      RefsArray newY = Y==NULL ? (RefsArray) NULL : copyRefsArray(Y);
      tt->pushCont(newPC,newY,G,NULL);
      tt->setSelf(e->getSelf());

      e->scheduleThread (tt);

#ifdef LINKEDTHREADS
      if (link) {
	if (e->debugmode())
	  printf("born %ld parent %ld\n",tt->getID(),
		 e->currentThread->getID());
	tt->setParent(makeTaggedConst(e->currentThread));
	e->currentThread->addChildThread(makeTaggedConst(tt));
      }
#endif
      
#ifdef THREADARRAY
      e->threadArray[tt->getID()] = makeTaggedConst(tt);
#endif

      if (e->debugmode()) {
/*
 * Debugger: We write the "thread creation" event onto the
 *           global debug stream and stop it ONLY when 
 *             - the parent thread has id 1, i.e. it's a
 *               toplevel query
 *             - the magic file "/tmp/ozdebugmagic" exists 
 */
#ifdef DEBUGMAGIC
	struct stat dummy;

	if (e->currentThread->getID() == 1UL && 
	    !stat("/tmp/ozdebugmagic", &dummy)) {

	  debugStreamThread(tt);
	  unlink("/tmp/ozdebugmagic");
          tt->startStepMode();
          tt->traced();
	  tt->stop();
	}
#endif
      }

      JUMP(contPC);
    }

// -------------------------------------------------------------------------
// CLASS: MISC: ERROR/NOOP/default
// -------------------------------------------------------------------------

  Case(TASKEMPTYSTACK)  
    {
      Assert(Y==0 && G==0);
      CTS->restoreFrame();
      if (e->isToplevel ()) {
	goto LBLkillToplevelThread;
      } else {
	goto LBLkillThread;
      }
    }


  Case(TASKCALLCONT)
    {
      predicate = tagged2Const((TaggedRef)ToInt32(Y));
      RefsArray tmpX = G;
      Y = G = NULL;
      predArity = tmpX ? getRefsArraySize(tmpX) : 0;
      int i = predArity;
      while (--i >= 0) {
	X[i] = tmpX[i];
      }
      disposeRefsArray(tmpX);
      DebugTrace(trace("call cont task",CBB));
      isTailCall = OK;
      goto LBLcall;
    }

  Case(TASKLOCK)
    {
      OzLock *lck = (OzLock *) Y;
      Y = NULL;
      lck->unlock();
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKXCONT)
    {
      RefsArray tmpX = Y;
      Y = NULL;
      predArity = getRefsArraySize(tmpX);
      int i = predArity;
      while (--i >= 0) {
	X[i] = tmpX[i];
      }
      disposeRefsArray(tmpX);
      goto LBLpopTaskNoPreempt;
    }


  Case(TASKSETSELF)
      e->setSelf((Object*)Y);
      Y = NULL;
      goto LBLpopTaskNoPreempt;


  Case(TASKDEBUGCONT)
    {
      OzDebug *ozdeb = (OzDebug *) Y;
      Y = NULL;
      exitCall(PROCEED,ozdeb);
      goto LBLpopTaskNoPreempt;
    }


   Case(TASKCFUNCONT)
     {
       // 
       // by kost@ : 'solve actors' are represented via a c-function; 
       biFun = (OZ_CFun) (void*) Y;
       RefsArray tmpX = G;
       G = Y = NULL;
       if (tmpX != NULL) {
	 predArity = getRefsArraySize(tmpX);
	 int i = predArity;
	 while (--i >= 0) {
	   X[i] = tmpX[i];
	 }
       } else {
	 predArity = 0;
       }
       disposeRefsArray(tmpX);

       DebugTrace(trace("cfunc cont task",CBB));

       switch (biFun(predArity, X)) {
       case FAILED:        HF_BI;
       case PROCEED:       goto LBLpopTask;
       case RAISE:         goto LBLraise;
       case BI_TYPE_ERROR: RAISE_TYPE;

       case SUSPEND:
	 e->pushCFun(biFun,X,predArity);
	 e->suspendOnVarList(CTT);
	 goto LBLsuspendThread;

       case SLEEP:
       default:
	 Assert(0);
       }
     }

  Case(TASKLTQ)
     {
       Y = NULL;  // sa here unused
       asmLbl(ltq);
       Assert(e->currentBoard->isSolve());
       Assert(!e->isToplevel());
       Assert(CTS->isEmpty()); // approximates one LTQ task
       
       // postpone poping task from taskstack until 
       // local thread queue is empty
       SolveActor * sa = SolveActor::Cast(e->currentBoard->getActor());
       LocalThreadQueue * ltq = sa->getLocalThreadQueue();
	
       Assert(!ltq->isEmpty());
	
       unsigned int starttime = osUserTime();
       Thread * backup_currentThread = CTT;

       while (!ltq->isEmpty() && e->isNotPreemtiveScheduling()) {
	 Thread * thr = CTT = ltq->dequeue();
	  
	 Assert(!thr->isDeadThread());
	  
	 OZ_Return r = thr->runPropagator();

	 if (r == SLEEP) {
	   thr->suspendPropagator();
	 } else if (r == PROCEED) {
	   thr->closeDonePropagator();
	 } else if (r == FAILED) {
	   thr->closeDonePropagator();
	   CTT = backup_currentThread;
	   ozstat.timeForPropagation.incf(osUserTime()-starttime);
	   CTS->restoreFrame(); // RS: is this needed ???
	   // failure of propagator is never catched !
	   goto LBLfailure; // top-level failure not possible
	 } else {
	   Assert(r == SCHEDULED);
	   thr->scheduledPropagator();
	 }
       } 
	
       CTT = backup_currentThread;
       ozstat.timeForPropagation.incf(osUserTime()-starttime);
	
       if (ltq->isEmpty()) {
	 sa->resetLocalThreadQueue();
	 goto LBLpopTask;
       } else {
	 CTS->restoreFrame();
	 goto LBLpreemption;
       }
     }
    

  Case(TASKACTOR)
    {
      AWActor *aw = (AWActor *) Y;
      Y = NULL;

      Assert(!aw->isCommitted());

      if (aw->hasNext()) {
	LOADCONT(aw->getNext());
	CAA=aw;
	CTS->restoreFrame();
	goto LBLemulate; // no thread switch allowed here (CAA)
      }

      if (aw->isWait()) {

	WaitActor *wa = WaitActor::Cast(aw);
	/* test bottom commit */
	if (wa->hasNoChildren()) {
	  HF_DIS;
	}

	/* test unit commit */
	if (wa->hasOneChildNoChoice()) {
	  Board *waitBoard = wa->getLastChild();

	  if (!e->commit(waitBoard,CTT)) {
	    HF_DIS;
	  }

	  wa->dispose();

	  goto LBLpopTask;
	}

	// suspend wait actor
	CTS->restoreFrame();
	goto LBLsuspendThread;
      }

      Assert(aw->isAsk());

      AskActor *aa = AskActor::Cast(aw);

      //  should we activate the 'else' clause?
      if (aa->isLeaf()) {
	aa->setCommitted();
	CBB->decSuspCount();

	/* rule: if fi --> false */
	if (aa->getElsePC() == NOCODE) {
	  HF_COND;
	}

	LOADCONT(aa->getNext());
	PC=aa->getElsePC();
	goto LBLemulate;
      }

      CTS->restoreFrame();
      goto LBLsuspendThread;
    }

  Case(OZERROR)
      error("Emulate: OZERROR command executed");
      goto LBLerror;


  Case(DEBUGINFO)
    {
      TaggedRef filename = getLiteralArg(PC+1);
      int line           = smallIntValue(getNumberArg(PC+2));
      int absPos         = smallIntValue(getNumberArg(PC+3));
      TaggedRef comment  = getLiteralArg(PC+4);
      int noArgs         = smallIntValue(getNumberArg(PC+5));
      TaggedRef globals  = OZ_nil(); // CodeArea::globalVarNames(PC);

      if (!e->debugmode() || !CTT->stepMode() || strstr(toC(comment), "exit"))
	{
	  DISPATCH(6);
	}
      
      // else
#ifdef STEPWITHDEBUGINFO
      Board *b = e->currentBoard;       // how can we avoid this ugly hack?
      e->currentBoard = e->rootBoard;
      
      TaggedRef tail    = e->threadStreamTail;
      TaggedRef newTail = OZ_newVariable();
    
      CTT->stop();

      TaggedRef pairlist = 
	OZ_cons(OZ_pairA("thr",
			 OZ_mkTupleC("#",2,makeTaggedConst(CTT),
				     OZ_int(CTT->getID()))),
		OZ_cons(OZ_pairA("file", filename),
			OZ_cons(OZ_pairAI("line", line),
				OZ_nil())));
      
      TaggedRef entry = OZ_recordInit(OZ_atom("step"), pairlist);
      OZ_unify(tail, OZ_cons(entry, newTail));
      e->threadStreamTail = newTail;

      e->currentBoard = b;

      e->pushTask(PC+6,Y,G,X, noArgs==-1 ? 1 : noArgs+1);
      goto LBLpreemption;
#else
      DISPATCH(6);
#endif
    }
  
  Case(MARSHALLEDFASTCALL)
    {
      Assert(0);
    } 
      
  Case(GENCALL) 
    {
      GenCallInfoClass *gci = (GenCallInfoClass*)getAdressArg(PC+1);
      int arity = getPosIntArg(PC+2);
      Assert(arity==0); /* is filled in by procedure genCallInfo */
      TaggedRef pred = G[gci->regIndex];
      DEREF(pred,predPtr,_1);
      if (isAnyVar(pred)) {
	SUSP_PC(predPtr,getWidth(gci->arity),PC);
      }
      genCallInfo(gci,pred,PC);
      gci->dispose();
      DISPATCH(0);
    }


  /* The following must be different from the following,
   * otherwise definitionEnd breaks under threaded code
   */
  Case(GLOBALVARNAME)
  Case(TASKCATCH)
    {
      error("impossible");
    }

  Case(TESTLABEL1)
  Case(TESTLABEL2)
  Case(TESTLABEL3)
  Case(TESTLABEL4)

  Case(TEST1)
  Case(TEST2)
  Case(TEST3)
  Case(TEST4)

  Case(ENDOFFILE)
  Case(ENDDEFINITION)
  Case(LOCALVARNAME)


#ifndef THREADED
  default:
     error("emulate instruction: default should never happen");
     break;
   } /* switch*/
#endif


  // ----------------- end emulate ------------------------------------------

// ------------------------------------------------------------------------
// *** FAILURE
// ------------------------------------------------------------------------
 LBLshallowFail:
  {
    if (e->trail.isEmptyChunk()) {
      e->trail.popMark();
    } else {
      e->reduceTrailOnFail();
    }
    ProgramCounter nxt = getLabelArg(shallowCP+1);
    shallowCP = NULL;
    JUMP(nxt);
  }

  /*
   *  kost@ : There are now the following invariants:
   *  - Can be entered only in a deep guard;
   *  - current thread must be runnable.
   */
 LBLfailure:
  DebugTrace(trace("fail",CBB));

  Assert(!e->isToplevel());
  Assert(CTT);
  Assert(CTT->isRunnable());
  Assert(CBB->isInstalled());

  Actor *aa=CBB->getActor();
  Assert(!aa->isCommitted());

  if (aa->isAskWait()) {
    (AWActor::Cast(aa))->failChild(CBB);
  }

  Assert(!CBB->isFailed());
  CBB->setFailed();

  e->reduceTrailOnFail();
  CBB->unsetInstalled();
  e->setCurrent(aa->getBoard());
  DebugCheckT(currentDebugBoard=CBB);


  // currentThread is a thread forked in a local space or a propagator
  if (aa->isSolve()) {

    //  Reduce (i.e. with failure in this case) the solve actor;
    //  The solve actor goes simply away, and the 'failed' atom is bound to
    // the result variable; 
    aa->setCommitted();
    SolveActor *saa=SolveActor::Cast(aa);
    // don't decrement parent counter

    if (!e->fastUnifyOutline(saa->getResult(),saa->genFailed(),OK)) {
      // this should never happen?
      Assert(0);
    }

  } else {

    AWActor *aw=AWActor::Cast(aa);
    Thread *tt = aw->getThread ();

    Assert (tt);

    if (tt->isSuspended()) {

      Assert(!(e->isScheduled (tt)));

      //  The following must hold because 'tt' can suspend 
      // only in the board where the actor itself is located;
      Assert(tt->getBoard() == CBB);

      tt->suspThreadToRunnable();

      e->scheduleThread(tt);
    } else {
      if (tt==CTT) goto LBLpopTask;
    }
  }

#ifdef DEBUG_CHECK
  if (CTT==e->rootThread) {
    printf("fail root thread\n");
  }
#endif

  e->decSolveThreads(CBB);
  CTT->disposeRunnableThread();
  CTT = 0;

  goto LBLstart;
} // end engine

#ifdef OUTLINE
#undef inline
#endif
