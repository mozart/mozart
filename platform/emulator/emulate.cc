/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

// The main engine

#include <stdarg.h>

#include "runtime.hh"

#include "indexing.hh"

#include "genvar.hh"
#include "dictionary.hh"
#include "fdhook.hh"
#include "perdio.hh"
#include "marshaler.hh"
#include "copycode.hh"

OZ_C_proc_proto(BIfail);     // builtins.cc

// -----------------------------------------------------------------------
// Object stuff
// -----------------------------------------------------------------------

inline
Abstraction *getSendMethod(Object *obj, TaggedRef label, SRecordArity arity, 
			   InlineCache *cache, RefsArray X)
{
  Assert(isFeature(label));
  return cache->lookup(obj->getClass(),label,arity,X);
}

inline
Abstraction *getApplyMethod(ObjectClass *cl, ApplMethInfoClass *ami, 
			    SRecordArity arity, RefsArray X)
{
  Assert(isFeature(ami->methName));
  return ami->methCache.lookup(cl,ami->methName,arity,X);
}

// -----------------------------------------------------------------------
// *** EXCEPTION stuff
// -----------------------------------------------------------------------


// check if failure has to be raised as exception on thread
int canOptimizeFailure(AM *e, Thread *tt)
{
  if (tt->hasCatchFlag() || e->onToplevel()) { // catch failure
    if (tt->isSuspended()) {
      tt->pushCFun(BIfail,0,0,NO);
      e->suspThreadToRunnableOPT(tt);
      e->scheduleThread(tt);
    } else {
      printf("WEIRD: failure detected twice");
#ifdef DEBUG_CHECK
      PopFrame(tt->getTaskStackRef(),PC,Y,G);
      Assert(PC==C_CFUNC_CONT_Ptr);
      Assert(((OZ_CFun)(void*)Y)==BIfail);
      tt->pushCFun(BIfail,0,0,NO);
#endif
    }
    return NO;
  } else {
    return OK;
  }
}

static
TaggedRef formatError(TaggedRef info,TaggedRef val,
		      OZ_Term traceBack,OZ_Term loc)
{
  OZ_Term d = OZ_record(OZ_atom("d"),
			cons(OZ_atom("info"),
			     cons(OZ_atom("stack"),
				  cons(OZ_atom("loc"),
				       nil()))));
  OZ_putSubtree(d,OZ_atom("stack"),traceBack);
  OZ_putSubtree(d,OZ_atom("loc"),loc);
  OZ_putSubtree(d,OZ_atom("info"),info);

  return OZ_adjoinAt(val,OZ_atom("debug"),d);
}


#define RAISE_APPLY(fun,args)						\
  (void) oz_raise(E_ERROR,E_KERNEL,"apply",2,fun,args); goto LBLraise;


static
void enrichTypeException(TaggedRef value,const char *fun, OZ_Term args)
{
  OZ_Term e = OZ_subtree(value,OZ_int(1));
  OZ_putArg(e,1,OZ_atom(fun));
  OZ_putArg(e,2,args);
}

#define RAISE_TYPE1(fun,args)			\
  enrichTypeException(e->exception.value,fun,args); goto LBLraise;

#define RAISE_TYPE1_FUN(fun,args)				\
  RAISE_TYPE1(fun,						\
	      appendI(args,cons(OZ_newVariable(),nil())));

#define RAISE_TYPE					\
   RAISE_TYPE1(builtinTab.getName((void *) biFun),	\
	       OZ_toList(predArity,X));

/*
 * Handle Failure macros (HF)
 */

Bool AM::hf_raise_failure(TaggedRef t)
{
  if (!onToplevel() &&
      (!currentThread()->hasCatchFlag() || 
       !isCurrentBoard(GETBOARD(currentThread())))) {
    return OK;
  }
  exception.info  = ozconf.errorDebug?t:NameUnit;
  exception.value = RecordFailure;
  exception.debug = ozconf.errorDebug;
  return NO;
}

#define HF_RAISE_FAILURE(T)			\
   if (e->hf_raise_failure(T))			\
     goto LBLfailure;				\
   goto LBLraise;


#define HF_FAIL       HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_DIS        HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_COND       HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_EQ(X,Y)    HF_RAISE_FAILURE(OZ_mkTupleC("eq",2,X,Y))
#define HF_TELL(X,Y)  HF_RAISE_FAILURE(OZ_mkTupleC("tell",2,X,Y))
#define HF_APPLY(N,A) HF_RAISE_FAILURE(OZ_mkTupleC("apply",2,N,A))
#define HF_BI	      HF_APPLY(OZ_atom(builtinTab.getName((void *) biFun)), \
			       OZ_toList(predArity,X));

#define CheckArity(arityExp,proc)					   \
if (predArity != arityExp) {						   \
  (void) oz_raise(E_ERROR,E_KERNEL,"arity",2,proc,OZ_toList(predArity,X)); \
  goto LBLraise;							   \
}

/*
 * make an record
 *  the subtrees are initialized with new variables
 */
static
TaggedRef mkRecord(TaggedRef label,SRecordArity ff)
{
  SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));
  srecord->initArgs();
  return makeTaggedSRecord(srecord);
}

// -----------------------------------------------------------------------
// *** ???
// -----------------------------------------------------------------------

#define NOFLATGUARD   (shallowCP)

#define SHALLOWFAIL   if (shallowCP) { goto LBLshallowFail; }

#define IMPOSSIBLE(INSTR) error("%s: impossible instruction",INSTR)

#define DoSwitchOnTerm(indexTerm,table)					\
      TaggedRef term = indexTerm;					\
      DEREF(term,termPtr,_2);						\
									\
      if (!isLTuple(term)) {						\
	TaggedRef *sp = sPointer;					\
	int offset = switchOnTermOutline(term,termPtr,table,sp);	\
	sPointer = sp;							\
	JUMPRELATIVE(offset);							\
      }									\
									\
      int offset = table->listLabel;					\
      sPointer = tagged2LTuple(term)->getRef();				\
      JUMPRELATIVE(offset);



#ifdef OUTLINE
#define inline
#endif

inline
TaggedRef fastnewTaggedUVar(AM *e)
{
  TaggedRef *ret = (TaggedRef *) int32Malloc(sizeof(TaggedRef));
  *ret = e->currentUVarPrototype();
  return makeTaggedRef(ret);
}


// -----------------------------------------------------------------------
// *** genCallInfo: self modifying code!
// -----------------------------------------------------------------------

static
Bool genCallInfo(GenCallInfoClass *gci, TaggedRef pred, ProgramCounter PC,
		 TaggedRef *X)
{
  Assert(!isRef(pred));

  Abstraction *abstr = NULL;
  if (gci->isMethAppl) {
    if (!isObjectClass(pred)) goto insertMethApply;

    Bool defaultsUsed;
    abstr = tagged2ObjectClass(pred)->getMethod(gci->mn,gci->arity,
						X,defaultsUsed);
    /* fill cache and try again later */
    if (abstr==NULL) return NO;
    if (defaultsUsed) goto insertMethApply; 
  } else {
    if(!isAbstraction(pred)) goto bombGenCall;

    abstr = tagged2Abstraction(pred);
    if (abstr->getArity() != getWidth(gci->arity)) 
      goto bombGenCall;
  }

  {
    /* ok abstr points to an abstraction */
    AbstractionEntry *entry = AbstractionTable::add(abstr);
    CodeArea::writeAddress(entry, PC+1);
    CodeArea::writeOpcode(gci->isTailCall ? FASTTAILCALL : FASTCALL, PC);
    return OK;
  }
  

insertMethApply:
  {
    ApplMethInfoClass *ami = new ApplMethInfoClass(gci->mn,gci->arity);
    CodeArea::writeOpcode(gci->isTailCall ? TAILAPPLMETHG : APPLMETHG, PC);
    CodeArea::writeAddress(ami, PC+1);
    CodeArea::writeRegIndex(gci->regIndex, PC+2);
    return OK;
  }

bombGenCall:
  CodeArea::writeRegIndex(gci->regIndex,PC+1);
  CodeArea::writeArity(getWidth(gci->arity), PC+2);
  CodeArea::writeOpcode(gci->isTailCall ? TAILCALLG : CALLG,PC);
  return OK;
}



// -----------------------------------------------------------------------
// *** CALL HOOK
// -----------------------------------------------------------------------


/* the hook functions return:
     TRUE: must reschedule
     FALSE: can continue
   */

Bool AM::emulateHookOutline() {
  // without signal blocking;
  if (isSetSFlag(ThreadSwitch)) {
    if (threadQueuesAreEmpty()) {
      restartThread();
    } else {
      return TRUE;
    }
  }
  if (isSetSFlag((StatusBit)(StartGC|UserAlarm|IOReady|StopThread))) {
    return TRUE;
  }

  return FALSE;
}

inline
Bool AM::isNotPreemptiveScheduling(void)
{
  if (isSetSFlag()) {
    if (isSetSFlag(ThreadSwitch)) {
      if (threadQueuesAreEmpty())
	restartThread();
      else
	return FALSE;
    }
    return !isSetSFlag();
  } else {
    return TRUE;
  }
}

#define DET_COUNTER 10000
inline
Bool AM::hookCheckNeeded()
{
#if defined(DEBUG_DET)
  static int counter = DET_COUNTER;
  if (--counter < 0) {
    handleAlarm();   // simulate an alarm
    counter = DET_COUNTER;
  }
#endif
  
  return (isSetSFlag());
}


// -----------------------------------------------------------------------
// ??? <- Bob, Justus und Peter
// -----------------------------------------------------------------------

/* macros are faster ! */
#define emulateHookCall(e,Code)				\
    if (e->hookCheckNeeded()) {				\
      if (e->emulateHookOutline()) {			\
	Code;						\
        goto LBLpreemption;				\
      }							\
    }

#define emulateHookPopTask(e) emulateHookCall(e,)


#define ChangeSelf(obj)				\
      e->changeSelf(obj);

#define SaveSelf				\
      CTT->setAbstr(ozstat.currAbstr);		\
      ozstat.leaveCall(NULL);			\
      e->saveSelf();


#define PushCont(PC,Y,G)  CTS->pushCont(PC,Y,G);
#define PushContX(PC,Y,G,X,n)  pushContX(CTS,PC,Y,G,X,n)

// outlined:
void pushContX(TaskStack *stk, 
	       ProgramCounter pc,RefsArray y,RefsArray g, RefsArray x, int n)
{
  stk->pushCont(pc,y,g); 
  stk->pushX(x,n);
}


/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred,gRegs)					     \
     Y = NULL;								     \
     G = gRegs;								     \
     emulateHookCall(e,PushContX(Pred->getPC(),NULL,G,X,Pred->getArity()));

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
// *** THREADED CODE
// -----------------------------------------------------------------------

#if defined(RECINSTRFETCH) && defined(THREADED)
 Error: RECINSTRFETCH requires THREADED == 0;
#endif

#define INCFPC(N) PC += N

#if !defined(DEBUG_EMULATOR) && !defined(DISABLE_INSTRPROFILE) && defined(__GNUC__)
#define asmLbl(INSTR) asm(" " #INSTR ":");
#else
#define asmLbl(INSTR)
#endif

#ifdef THREADED
#define Case(INSTR) INSTR##LBL : asmLbl(INSTR); 

#ifdef DELAY_SLOT
// let gcc fill in the delay slot of the "jmp" instruction:
#define DISPATCH(INC) {				\
  intlong aux = *(PC+INC);			\
  INCFPC(INC);					\
  goto* (void*) (aux|textBase);			\
}
#else
#define DISPATCH(INC) {				\
  INCFPC(INC);					\
  goto* (void*) ((*PC)|textBase);		\
}
#endif

#else /* THREADED */

#define Case(INSTR)   case INSTR :  asmLbl(INSTR); 
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#endif

#define JUMPRELATIVE(offset) Assert(offset!=0); INCFPC(offset); DISPATCH(0)
#define JUMPABSOLUTE(absaddr) PC=absaddr; DISPATCH(0)

#define ONREG(Label,R)      HelpReg = (R); goto Label
#define ONREG2(Label,R1,R2) HelpReg1 = (R1); HelpReg2 = (R2); goto Label


#ifdef FASTREGACCESS
#define RegAccess(Reg,Index) (*(RefsArray)((intlong) Reg + Index))
#define LessIndex(Index,CodedIndex) \
                       (Index <= (int)(CodedIndex/sizeof(TaggedRef)))
#else
#define RegAccess(Reg,Index) (Reg[Index])
#define LessIndex(I1,I2) (I1<=I2)
#endif

#define Xreg(N) RegAccess(X,N)
#define Yreg(N) RegAccess(Y,N)
#define Greg(N) RegAccess(G,N)

#define XPC(N) Xreg(getRegArg(PC+N))
#define GetBI(PC) ((BuiltinTabEntry*) getAdressArg(PC))

#if defined(LINUX_I486) || defined(GNUWIN32) || defined(SOLARIS_I486) || defined(FREEBSD_I486)
#define OZ_I486
#endif

/* define REGOPT if you want the into register optimization for GCC */
#if defined(REGOPT) &&__GNUC__ >= 2 && (defined(OZ_I486) || defined(MIPS) || defined(OSF1_ALPHA) || defined(SPARC)) && !defined(DEBUG_CHECK)
#define Into(Reg) asm(#Reg)

#ifdef OZ_I486
#define Reg1 asm("%esi")
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

#endif/*
 * Handling of the READ/WRITE mode bit: 
 * last significant bit of sPointer set iff in WRITE mode
 */

#define SetReadMode
#define SetWriteMode (sPointer = (TaggedRef *)((long)sPointer+1));

#define InWriteMode (((long)sPointer)&1)

#define GetSPointerWrite(ptr) (TaggedRef*)(((long)ptr)-1)

#ifdef DEBUG_LIVENESS
extern void checkLiveness(ProgramCounter PC, int n, TaggedRef *X, int max);
#define CheckLiveness(PC,n) checkLiveness(PC,n,X,NumberOfXRegisters)
#else
#define CheckLiveness(PC,n)
#endif

// ------------------------------------------------------------------------
// ???
// ------------------------------------------------------------------------

#define SUSP_PC(TermPtr,RegsToSave,PC)		\
   CheckLiveness(PC,RegsToSave);		\
   PushContX(PC,Y,G,X,RegsToSave);		\
   addSusp(TermPtr,CTT);			\
   goto LBLsuspendThread;


void addSusp(TaggedRef *varPtr, Thread *thr)
{
  if(thr->getPStop()==0)
    addSuspAnyVar(varPtr,thr);
}



void addSusp(TaggedRef var, Thread *thr)
{
  DEREF(var,varPtr,tag);
  Assert(isAnyVar(var));

  addSusp(varPtr,thr);
}


/*
 * create the suspension for builtins returning SUSPEND
 *
 * PRE: no reference chains !!
 */
#define SUSPENDONVARLIST			\
{						\
  suspendOnVarList(e->suspendVarList,CTT);	\
  e->suspendVarList=0;				\
  goto LBLsuspendThread;			\
}

static
void suspendOnVarList(TaggedRef varList,Thread *thr)
{
  while (!isRef(varList)) {
    Assert(isCons(varList));
    
    addSusp(head(varList),thr);
    varList=tail(varList);
  }
  addSusp(varList,thr);
}

static
void suspendInline(Thread *th, OZ_Term A,OZ_Term B=0,OZ_Term C=0)
{
  if (C) { DEREF (C, ptr, _1); if (isAnyVar(C)) addSusp(ptr, th); }
  if (B) { DEREF (B, ptr, _1); if (isAnyVar(B)) addSusp(ptr, th); }
  { DEREF (A, ptr, _1); if (isAnyVar(A)) addSusp(ptr, th); }
}

// -----------------------------------------------------------------------
// outlined auxiliary functions
// -----------------------------------------------------------------------

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
  TaggedRef ret = oz_newVariable();
  VariableNamer::addName(ret,tagged2Literal(name)->getPrintName());
  return ret;
}


inline
Bool AM::entailment()
{
  return (!currentBoard()->hasSuspension()  // threads?
	  && trail.isEmptyChunk());       // constraints?
}

/*
 * check stability after thread is finished
 */
void AM::checkStability()
{
  // try to reduce a solve board;
  SolveActor *solveAA = SolveActor::Cast(currentBoard()->getActor());
  Board      *solveBB = currentBoard();
 
  if (isStableSolve(solveAA)) {
    Assert(trail.isEmptyChunk());
    // all possible reduction steps require this; 

    if (!solveBB->hasSuspension()) {
      // 'solved';
      // don't unlink the subtree from the computation tree;
      trail.popMark();
      currentBoard()->unsetInstalled();
      setCurrent(currentBoard()->getParent());
      // don't decrement counter of parent board!

      if (!fastUnifyOutline(solveAA->getResult(), 
			    solveAA->genSolved(), 0)) {
	Assert(0);
      }
      return;
    }

    // check for nonmonotonic propagators
    solveAA->scheduleNonMonoSuspList();
    if (! isStableSolve(solveAA))
      return;

    WaitActor *wa = solveAA->getChoice();

    if (wa == NULL) {
      // "stuck" (stable without distributing waitActors);
      // don't unlink the subtree from the computation tree; 
      trail.popMark();
      currentBoard()->unsetInstalled();
      setCurrent(currentBoard()->getParent());

      // don't decrement counter of parent board!

      if ( !fastUnifyOutline(solveAA->getResult(), 
			     solveAA->genStuck(), 0) ) {
	Assert(0);
      }
      return;
    }

    // to enumerate;

    if (wa->getChildCount()==1) {
      Assert(wa->isChoice());

      Board *waitBoard = wa->getChildRef();

      int ret=commit(waitBoard);
      Assert(ret);
      DebugCode(unsetCurrentThread());
      return;
    }

    // give back number of clauses
    trail.popMark();
    currentBoard()->unsetInstalled();
    setCurrent(currentBoard()->getParent());

    // don't decrement counter of parent board!

    if (!fastUnifyOutline(solveAA->getResult(),
			  solveAA->genChoice(wa->getChildCount()),
			  0)) {
      Assert(0);
    }
    return;
  }

  if (solveAA->getThreads() == 0) {
    // There are some external suspensions to this solver!

    deinstallCurrent();

    TaggedRef newVar = oz_newVariable();
    TaggedRef result = solveAA->getResult();

    solveAA->setResult(newVar);

    if ( !fastUnifyOutline(result,
			   solveAA->genUnstable(newVar),
			   0)) {
      Assert(0);
    }
    return;
  } 

  deinstallCurrent();
  return;
} 


// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ************************************************************************
// ************************************************************************
// THE BIG EMULATE LOOP STARTS
// ************************************************************************
// ************************************************************************
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------


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
#ifdef MANY_REGISTERS
  register TaggedRef *X        Reg2 = am.xRegs;
#else
  register TaggedRef * const X Reg2 = am.xRegs;
#endif
  register RefsArray Y         Reg3 = NULL;
  register TaggedRef *sPointer Reg4 = NULL;
  register AM * const e	       Reg5 = &am;
  register RefsArray G         Reg6 = NULL;

  Bool isTailCall              = NO;                NoReg(isTailCall);
  AWActor *CAA                 = NULL;
  DebugCheckT(Board *currentDebugBoard=0);

  RefsArray HelpReg1 = NULL, HelpReg2 = NULL;
  #define HelpReg sPointer  /* more efficient */

  /* shallow choice pointer */
  ByteCode *shallowCP = NULL;

  ConstTerm *predicate;	    NoReg(predicate);
  int predArity;    	    NoReg(predArity);

  TaggedRef auxTaggedA, auxTaggedB;
  int auxInt;
  char *auxString;

  // short names
# define CBB (e->currentBoard())
# define CTT (e->currentThread())
# define CPP (CTT->getPriority())
# define CTS (e->cachedStack)


#ifdef THREADED
# include "instrtab.hh"
  if (init) {
    CodeArea::init(instrTable);
    return;
  }
#else
  if (init) {
    CodeArea::init(NULL);
    return;
  }
  Opcode op = (Opcode) -1;
#endif

  goto LBLstart;

// ------------------------------------------------------------------------
// *** Error
// ------------------------------------------------------------------------

LBLerror:
  error("impossible state in emulator: LBLerror");
  goto LBLstart;


// ------------------------------------------------------------------------
// *** preempt current thread
// ------------------------------------------------------------------------

LBLpreemption:
  asmLbl(PREEMPT_THREAD);
  SaveSelf;
  Assert(GETBOARD(CTT)==CBB);
  /*  Assert(CTT->isRunnable()|| (CTT->isStopped())); ATTENTION */
  e->scheduleThreadInline(CTT, CPP);
  e->unsetCurrentThread();

  // fall through

// ------------------------------------------------------------------------
// *** execute runnable thread
// ------------------------------------------------------------------------
LBLstart:
  asmLbl(GET_THREAD);
  Assert(CTT==0);

  // check status register
  if (e->isSetSFlag()) {
    e->checkStatus();
  }

  if (e->threadQueuesAreEmpty()) {
    e->suspendEngine();
  }
  e->setCurrentThread(e->getFirstThread());
  Assert(CTT);

  PC = NOCODE; // this is necessary for printing stacks (mm)

  DebugTrace(trace("runnable thread->running"));

  // source level debugger & Thread.suspend
  if (CTT->getStop() || CTT->getPStop()) {
    e->unsetCurrentThread();  // byebye...
    goto LBLstart;
  }

  //  Every runnable thread must be terminated through 
  // 'LBL{discard,kill}Thread', and it should not appear 
  // more than once in the threads pool;
  Assert(!CTT->isDeadThread() && CTT->isRunnable());

  asmLbl(INSTALL_BOARD);
  // Install board
  {
    Board *bb=GETBOARD(CTT);
    if (CBB != bb) {
      switch (e->installPath(bb)) {
      case INST_OK:
	break;
      case INST_REJECTED:
	DebugCode (if (CTT->isPropagator()) CTT->removePropagator());
	goto LBLdiscardThread;
      case INST_FAILED:
	DebugCode (if (CTT->isPropagator()) CTT->removePropagator());
	goto LBLfailure;
      }
    }
  }

  // Constraints are implicitly propagated
  CBB->unsetNervous();

  if (CTT->isPropagator()) {
    // Propagator
    //    unsigned int starttime = osUserTime();
    switch (e->runPropagator(CTT)) {
    case SLEEP: 
      e->suspendPropagator(CTT);
      if (e->isBelowSolveBoard()) {
	e->decSolveThreads(e->_currentSolveBoard);
	//  but it's still "in solve";
      }
      e->unsetCurrentThread();

      goto LBLstart;

    case SCHEDULED:
      e->scheduledPropagator(CTT);
      if (e->isBelowSolveBoard()) {
	e->decSolveThreads (e->_currentSolveBoard);
	//  but it's still "in solve";
      }
      e->unsetCurrentThread();

      goto LBLstart;

    case PROCEED:
      // Note: CTT must be reset in 'LBLkillXXX';

      goto LBLterminateThread;

      //  Note that *propagators* never yield 'SUSPEND';
    case FAILED:
      // propagator failure never catched
      if (!e->onToplevel()) goto LBLfailure;

      {
	OZ_Propagator *prop = CTT->getPropagator();
	e->setCurrentThread(e->mkRunnableThreadOPT(PROPAGATOR_PRIORITY, CBB));
	CTS = CTT->getTaskStackRef();
	e->restartThread();
	HF_APPLY(OZ_atom(builtinTab.getName((void *)(prop->getHeader()
						     ->getHeaderFunc()))),
		 prop->getParameters());
      }
    }
  } else {
    DebugCheckT(currentDebugBoard=CBB);

    Assert(CTT->isRunnable());

    //
    //  Note that this test covers also the case when a runnable thread
    // was suspended in a sequential mode: it had already a stack, 
    // so we don't have to do anything now;
    if (CTT->getThrType() == S_WAKEUP) {
      //
      //  Wakeup;
      //  No regions were pre-allocated, - so just make a new one;
      CTT->setHasStack();
      CTT->item.threadBody = e->allocateBody();
      
      GETBOARD(CTT)->setNervous();
    } else {
      Assert(CTT->getThrType() == S_RTHREAD);
    }

    e->cachedStack = CTT->getTaskStackRef();
    e->cachedSelf  = CTT->getSelf();
    CTT->setSelf(0);
    ozstat.leaveCall(CTT->abstr);
    CTT->abstr = 0;
    e->restartThread(); // start a new time slice
    goto LBLpopTask;
  }


  goto LBLerror;


// ------------------------------------------------------------------------
// *** Thread is terminated
// ------------------------------------------------------------------------

  /*
   *  Kill the thread - decrement 'suspCounter'"s and counters of 
   * runnable threads in solve actors if any
   */
LBLterminateThread:
  {
    asmLbl(TERMINATE_THREAD);

    DebugTrace(trace("kill thread", CBB));
    Assert(CTT);
    Assert(!CTT->isDeadThread());
    Assert(CTT->isRunnable());
    Assert(CTT->isPropagator() || CTT->isEmpty());

    //  Note that during debugging the thread does not carry 
    // the board pointer (== NULL) wenn it's running;
    // Assert (CBB == CTT->getBoard());

    Assert(CBB != (Board *) NULL);
    Assert(!CBB->isFailed());

    Assert(e->onToplevel() ||
	   ((CTT->isInSolve() || !e->isBelowSolveBoard()) &&
	    (e->isBelowSolveBoard() || !CTT->isInSolve())));

    if (CTT == e->rootThread()) {
      e->rootThread()->reInit();
      e->checkToplevel();
      if (e->rootThread()->isEmpty()) {
	e->unsetCurrentThread();
	goto LBLstart;
      } else {
	goto LBLpreemption;
      }
    }

    CBB->decSuspCount();

    e->disposeRunnableThread(CTT);
    e->unsetCurrentThread();

    // fall through to checkEntailmentAndStability
  }

// ------------------------------------------------------------------------
// *** Check entailment and stability
// ------------------------------------------------------------------------

LBLcheckEntailmentAndStability:
  {
    /*     *  General comment - about checking for stability:
     *  In the case when the thread was originated in a solve board, 
     * we have to update the (runnable) threads counter there manually, 
     * check stability there ('AM::checkStability ()'), and proceed 
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
     */ 

    // maybe optimize?
    if (e->onToplevel()) goto LBLstart;

    Board *nb = 0; // notification board

    // 
    //  First, look at the current board, and if it's a solve one, 
    // decrement the (runnable) threads counter manually _and_
    // skip the 'AM::decSolveThreads ()' for it; 
    if (e->isBelowSolveBoard()) {
      if (CBB->isSolve ()) {
	SolveActor *sa;

	//
	sa = SolveActor::Cast (CBB->getActor ());
	//  'nb' points to some board above the current one,
	// so, 'decSolveThreads' will start there!
	nb = GETBOARD(sa);

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
    DebugCode(e->unsetCurrentThread());

    DebugTrace(trace("check entailment",CBB));

#ifdef DEBUG_NONMONOTONIC
    cout << "checkEntailment" << endl << flush;
#endif

    CBB->unsetNervous();

  // check for entailment of ASK and WAITTOP
    if ((CBB->isAsk() || CBB->isWaitTop()) && e->entailment()) {
      Board *bb = CBB;
      e->deinstallCurrent();
      int ret=e->commit(bb);
      Assert(ret);
    } else if (CBB->isSolve()) {
      e->checkStability();
    }

    // 
    //  deref nb, because it maybe just committed!
    if (nb) e->decSolveThreads (nb->derefBoard());
    goto LBLstart;
  }

// ------------------------------------------------------------------------
// *** Discard Thread
// ------------------------------------------------------------------------

  /*
   *  Discard the thread, i.e. just decrement solve thread counters 
   * everywhere it is needed, and dispose the body; 
   *  The main difference to the 'LBLterminateThread' is that no 
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
    asmLbl(DISCARD_THREAD);

    Assert(CTT);
    Assert(!CTT->isDeadThread());
    Assert(CTT->isRunnable());

    //
    //  Note that we may not use the 'currentSolveBoard' test here,
    // because it may point to an irrelevant board!
    if (CTT->isInSolve()) {
      Board *tmpBB = GETBOARD(CTT);

      if (tmpBB->isSolve()) {
	//
	//  The same technique as by 'LBLterminateThread';
	SolveActor *sa = SolveActor::Cast (tmpBB->getActor ());
	Assert (sa);
	Assert (sa->getSolveBoard () == tmpBB);

	e->decSolveThreads(GETBOARD(sa));
      } else {
	e->decSolveThreads (tmpBB);
      }
    }
    e->disposeRunnableThread(CTT);
    e->unsetCurrentThread();

    goto LBLstart;
  }

// ------------------------------------------------------------------------
// *** Suspend Thread
// ------------------------------------------------------------------------

  /*
   *  Suspend the thread in a generic way. It's used when something 
   * suspends in the sequential mode;
   *  Note that the thread stack should already contain the 
   * "suspended" task; 
   *
   *  Note that this code might be entered not only from within 
   * the toplevel, so, it has to handle everything correctly ...
   *   *  Invariants:
   *  - CBB must be alive;
   *
   */
LBLsuspendThread:
  {
    asmLbl(SUSPEND_THREAD);

    DebugTrace(trace("suspend runnable thread", CBB));

    Assert(CTT);
    CTT->unmarkRunnable();

    Assert(CBB);
    Assert(!CBB->isFailed());
    //  see the note for the 'LBLterminateThread';
    Assert(CTT->isInSolve() || !e->isBelowSolveBoard());
    Assert(e->isBelowSolveBoard() || !CTT->isInSolve());

    //
    //  First, set the board and self, and perform special action for 
    // the case of blocking the root thread;
    Assert(GETBOARD(CTT)==CBB);
    SaveSelf;

#ifdef DEBUG_ROOT_THREAD
    // this can happen if \sw -threadedqueries,
    // or in non-threaded \feeds, e.g. suspend for I/O
    if (CTT==e->rootThread()) {
      printf("root blocked\n");
    }
#endif

    if (e->debugmode() && CTT->getTrace()) {
      debugStreamBlocked(CTT);
    } else if (CTT->getNoBlock() && CAA == NULL) {
      (void) oz_raise(E_ERROR,E_KERNEL,"block",1,makeTaggedConst(CTT));
      CTT->markRunnable();
      goto LBLraise;
    }

    e->unsetCurrentThread();

    //  No counter decrement 'cause the thread is still alive!

    if (e->onToplevel()) {
      //
      //  Note that in this case no (runnable) thread counters 
      // in solve actors can be affected, just because this is 
      // a top-level thread;
      goto LBLstart;
    }

    // 
    //  So, from now it's somewhere in a deep guard;
    Assert (!(e->onToplevel ()));

    goto LBLcheckEntailmentAndStability;
  }

// ------------------------------------------------------------------------
// *** Emulator: execute instructions
// ------------------------------------------------------------------------

 LBLemulate:
  asmLbl(EMULATE);
  Assert(CBB==currentDebugBoard);

  JUMPABSOLUTE( PC );

  asmLbl(END_EMULATE);
#ifndef THREADED
LBLdispatcher:
  asmLbl(DISPATCH);

#ifdef RECINSTRFETCH
  CodeArea::recordInstr(PC);
#endif

  DebugTrace( if (!trace("emulate",CBB,CAA,PC,Y,G)) {
		goto LBLfailure;
	      });

  op = CodeArea::getOP(PC);
  // displayCode(PC,1);

#ifdef PROFILE_INSTR
  if (op < PROFILE_INSTR_MAX) ozstat.instr[op]++;
#endif
 
  switch (op) {
#endif

// -------------------------------------------------------------------------
// INSTRUCTIONS: TERM: MOVE/UNIFY/CREATEVAR/...
// -------------------------------------------------------------------------

#include "register.hh"

// ------------------------------------------------------------------------
// INSTRUCTIONS: (Fast-) Call/Execute Inline Funs/Rels
// ------------------------------------------------------------------------

  Case(FASTCALL)
    {
      PushCont(PC+3,Y,G);   // PC+3 goes into a temp var (mm2)
      // goto LBLFastTailCall; // is not optimized away (mm2)
    }


  Case(FASTTAILCALL)
    //  LBLFastTailCall:
    {
      AbstractionEntry *entry = 
	(AbstractionEntry *)getAdressArg(PC+1);
      
      COUNT(fastcalls);
      CallDoChecks(entry->getAbstr(),entry->getGRegs());

      IHashTable *table = entry->indexTable;
      if (table) {
	PC = entry->getPC();
	DoSwitchOnTerm(X[0],table);
      } else {
	JUMPABSOLUTE(entry->getPC());
      }
    }

  Case(CALLBUILTIN)
    {
      COUNT(bicalls);
      BuiltinTabEntry* entry = GetBI(PC+1);
      OZ_CFun biFun = entry->getFun();

      // CheckArity(entry->getArity(),makeTaggedConst(entry));

#ifdef PROFILE_BI
      entry->incCounter();
#endif
      switch (biFun(-2, X)) { // -2 == don't check arity

      case PROCEED:	  DISPATCH(3);
      case FAILED:	  HF_BI;
      case RAISE:	  goto LBLraise;
      case BI_TYPE_ERROR: RAISE_TYPE;
	 
      case SUSPEND:
	predArity = getPosIntArg(PC+2);
	PushContX(PC,Y,G,X,predArity);
	SUSPENDONVARLIST;

      case BI_PREEMPT:
	predArity = getPosIntArg(PC+2);
	PushContX(PC+3,Y,G,X,predArity);
	goto LBLpreemption;

      case BI_REPLACEBICALL: 
	predArity = getPosIntArg(PC+2);
	PC += 3;
	goto LBLreplaceBICall;

      case SLEEP:
      default:
	Assert(0);
      }
    }


  Case(INLINEREL1)
    {
      COUNT(inlinecalls);
      InlineRel1 rel         = (InlineRel1)GetBI(PC+1)->getInlineFun();
#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif

      Assert(rel != NULL);

      OZ_Return res = rel(XPC(2));
      if (res==PROCEED) { DISPATCH(4); }
      if (res==FAILED) {
	SHALLOWFAIL;
	HF_APPLY(OZ_atom(GetBI(PC+1)->getPrintName()),
		 cons(XPC(2),nil()));
      }

      switch(res) {
      case SUSPEND:
	if (shallowCP) {
	  e->trail.pushIfVar(XPC(2));
	  goto LBLsuspendShallow;
	}
	CheckLiveness(PC,getPosIntArg(PC+3));
	PushContX(PC,Y,G,X,getPosIntArg(PC+3));
	suspendInline(CTT,XPC(2));
	goto LBLsuspendThread;

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(GetBI(PC+1)->getPrintName(),
		    cons(XPC(2),nil()));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEREL2)
    {
      COUNT(inlinecalls);
      InlineRel2 rel         = (InlineRel2)GetBI(PC+1)->getInlineFun();
      Assert(rel != NULL);

#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif
      OZ_Return res = rel(XPC(2),XPC(3));
      if (res==PROCEED) { DISPATCH(5); }

      if (res==FAILED) {
	SHALLOWFAIL;
	HF_APPLY(OZ_atom(GetBI(PC+1)->getPrintName()),
		 cons(XPC(2),cons(XPC(3),nil())));
      }

      switch(res) {

      case SUSPEND:
	if (shallowCP) {
	  e->trail.pushIfVar(XPC(2));
	  e->trail.pushIfVar(XPC(3));
	  goto LBLsuspendShallow;
	}
	
	CheckLiveness(PC,getPosIntArg(PC+4));
	PushContX(PC,Y,G,X,getPosIntArg(PC+4));
	suspendInline(CTT,XPC(2),XPC(3));
	goto LBLsuspendThread;

      case BI_PREEMPT:
	CheckLiveness(PC,getPosIntArg(PC+4));
	PushContX(PC+5,Y,G,X,getPosIntArg(PC+4));
	goto LBLsuspendThread;

      case RAISE:
	goto LBLraise;

      case BI_REPLACEBICALL: 
	predArity = getPosIntArg(PC+4);
	PC += 5;
	goto LBLreplaceBICall;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(GetBI(PC+1)->getPrintName(),
		    cons(XPC(2),cons(XPC(3),nil())));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEREL3)
    {
      COUNT(inlinecalls);
      InlineRel3 rel = (InlineRel3)GetBI(PC+1)->getInlineFun();
      Assert(rel != NULL);

#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif
      OZ_Return res = rel(XPC(2),XPC(3),XPC(4));
      if (res==PROCEED) { DISPATCH(6); }
      if (res==FAILED) {
	SHALLOWFAIL;
	HF_APPLY(OZ_atom(GetBI(PC+1)->getPrintName()),
		 cons(XPC(2),cons(XPC(3),cons(XPC(4),nil()))));
      }
      
      switch(res) {
      case SUSPEND:
	if (shallowCP) {
	  e->trail.pushIfVar(XPC(2));
	  e->trail.pushIfVar(XPC(3));
	  e->trail.pushIfVar(XPC(4));
	  goto LBLsuspendShallow;
	}
	
	CheckLiveness(PC,getPosIntArg(PC+5));
	PushContX(PC,Y,G,X,getPosIntArg(PC+5));
	suspendInline(CTT,XPC(2),XPC(3),XPC(4));
	goto LBLsuspendThread;

      case RAISE:
	goto LBLraise;

      case BI_PREEMPT:
	CheckLiveness(PC,getPosIntArg(PC+5));
	PushContX(PC+6,Y,G,X,getPosIntArg(PC+5));
	goto LBLsuspendThread;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(GetBI(PC+1)->getPrintName(),
		    cons(XPC(2),cons(XPC(3),cons(XPC(4),nil()))));


      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEMINUS)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);
      TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
      
      if ( isSmallInt(tagA) && isSmallInt(tagB) ) {
	XPC(3) = makeInt(smallIntValue(A) - smallIntValue(B));
	DISPATCH(5);
      } 
      
      if (isFloat(tagA) && isFloat(tagB)) {
	XPC(3) = oz_float(floatValue(A) - floatValue(B));
	DISPATCH(5);
      }
            
      auxTaggedA = XPC(1);
      auxTaggedB = XPC(2);
      auxInt     = 4;
      auxString = "-";

      // abuse predArity
      predArity = (int) BIminusOrPlus(NO,A,B,XPC(3));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEPLUS)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);
      TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
      
      if ( isSmallInt(tagA) && isSmallInt(tagB) ) {
	XPC(3) = makeInt(smallIntValue(A) + smallIntValue(B));
	DISPATCH(5);
      } 
      
      if (isFloat(tagA) && isFloat(tagB)) {
	XPC(3) = oz_float(floatValue(A) + floatValue(B));
	DISPATCH(5);
      }
            
      auxTaggedA = XPC(1);
      auxTaggedB = XPC(2);
      auxInt     = 4;
      auxString = "+";

      // abuse predArity
      predArity = (int) BIminusOrPlus(OK,A,B,XPC(3));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEMINUS1)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);

      if (isSmallInt(tagA)) {
	/* INTDEP */
	int res = (int)A - (1<<tagSize);
	if ((int)A > res) {
	  XPC(2) = res;
	  DISPATCH(4);
	}
      }

      auxTaggedA = XPC(1);
      auxTaggedB = makeTaggedSmallInt(1);
      auxInt     = 3;
      auxString = "-1";

      // abuse predArity
      predArity = (int) BIminusOrPlus(NO,A,makeTaggedSmallInt(1),XPC(2));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEPLUS1)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);

      if (isSmallInt(tagA)) {
	/* INTDEP */
	int res = (int)A + (1<<tagSize);
	if ((int)A < res) {
	  XPC(2) = res;
	  DISPATCH(4);
	}
      }

      auxTaggedA = XPC(1);
      auxTaggedB = makeTaggedSmallInt(1);
      auxInt     = 3;
      auxString = "+1";

      // abuse predArity
      predArity = (int) BIminusOrPlus(OK,A,auxTaggedB,XPC(2));
      goto LBLhandlePlusMinus;
    }


  LBLhandlePlusMinus:
  {
      OZ_Return res = (OZ_Return) predArity;

      switch(res) {
      case PROCEED:       DISPATCH(auxInt+1);
      case BI_TYPE_ERROR: RAISE_TYPE1_FUN(auxString, 
					  cons(auxTaggedA,cons(auxTaggedB,nil())));

      case SUSPEND:
	{
	  if (shallowCP) {
	    e->trail.pushIfVar(auxTaggedA);
	    e->trail.pushIfVar(auxTaggedB);
	    goto LBLsuspendShallow;
	  }
	  CheckLiveness(PC,getPosIntArg(PC+auxInt));
	  PushContX(PC,Y,G,X,getPosIntArg(PC+auxInt));
	  suspendInline(CTT,auxTaggedA,auxTaggedB);
	  goto LBLsuspendThread;
	}
      default:    Assert(0);
      }
    }

  Case(INLINEFUN1)
    {
      COUNT(inlinecalls);
      InlineFun1 fun = (InlineFun1)GetBI(PC+1)->getInlineFun();
      Assert(fun != NULL);

#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif
      // XPC(3) maybe the same register as XPC(2)
      OZ_Return res = fun(XPC(2),XPC(3));
      if (res==PROCEED) { DISPATCH(5); }
      if (res==FAILED) {
	SHALLOWFAIL;
	Assert(0);
      }

      switch(res) {
      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    goto LBLsuspendShallow;
	  }
	  CheckLiveness(PC,getPosIntArg(PC+4));
	  PushContX(PC,Y,G,X,getPosIntArg(PC+4));
	  suspendInline(CTT,A);
	  goto LBLsuspendThread;
	}

      case RAISE:            goto LBLraise;


/* Must save output register too !!! (RS) */
#define MaxToSave(OutReg,LivingRegs) \
	max(getRegArg(PC+OutReg)+1,getPosIntArg(PC+LivingRegs));

      case BI_REPLACEBICALL: 
	predArity = MaxToSave(3,4);
	PC += 5;
	goto LBLreplaceBICall;


      case BI_TYPE_ERROR:
	RAISE_TYPE1_FUN(GetBI(PC+1)->getPrintName(),
			cons(XPC(2),nil()));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEFUN2)
    {
      COUNT(inlinecalls);
      InlineFun2 fun = (InlineFun2)GetBI(PC+1)->getInlineFun();
      Assert(fun != NULL);
      
#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif
      // note: XPC(4) is maybe the same as XPC(2) or XPC(3) !!
      OZ_Return res = fun(XPC(2),XPC(3),XPC(4));

      if (res==PROCEED) { DISPATCH(6); }
      if (res==FAILED) {
	SHALLOWFAIL;
	HF_APPLY(OZ_atom(GetBI(PC+1)->getPrintName()),
		 cons(XPC(2),cons(XPC(3),cons(OZ_newVariable(), nil()))));
      }

      switch(res) {
      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  TaggedRef B=XPC(3);
	  if (shallowCP) {
	    e->trail.pushIfVar(A);
	    e->trail.pushIfVar(B);
	    goto LBLsuspendShallow;
	  }
	  CheckLiveness(PC,getPosIntArg(PC+5));
	  PushContX(PC,Y,G,X,getPosIntArg(PC+5));
	  suspendInline(CTT,A,B);
	  goto LBLsuspendThread;
	}

      case RAISE:
	goto LBLraise;


      case BI_REPLACEBICALL: 
	predArity = MaxToSave(4,5);
	PC += 6;
	goto LBLreplaceBICall;

      case BI_TYPE_ERROR:
	RAISE_TYPE1_FUN(GetBI(PC+1)->getPrintName(),
			cons(XPC(2),cons(XPC(3),nil())));

      case SLEEP:
      default:
	Assert(0);
      }
     }

  Case(INLINEDOT)
    {
      COUNT(inlinedots);
      TaggedRef feature = getLiteralArg(PC+2);
      TaggedRef rec = XPC(1);
      DEREF(rec,_1,_2);
      if (isSRecord(rec)) {
	SRecord *srec = tagged2SRecord(rec);
	int index = ((InlineCache*)(PC+5))->lookup(srec,feature);
	if (index<0) {
	  (void) oz_raise(E_ERROR,E_KERNEL,".", 2, XPC(1), feature);
	  goto LBLraise;
	}
	XPC(3) = srec->getArg(index);
	DISPATCH(7);	
      }
      {
	OZ_Return res = dotInline(XPC(1),feature,XPC(3));
	if (res==PROCEED) { DISPATCH(7); }

	switch(res) {
	case SUSPEND:
	  {
	    TaggedRef A=XPC(1);
	    if (shallowCP) {
	      e->trail.pushIfVar(A);
	      goto LBLsuspendShallow;
	    }
	    CheckLiveness(PC,getPosIntArg(PC+4));
	    PushContX(PC,Y,G,X,getPosIntArg(PC+4));
	    suspendInline(CTT,A);
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
      Object *self = e->getSelf();

      Assert(e->getSelf()!=NULL);
      RecOrCell state = self->getState();
      SRecord *rec;
      if (stateIsCell(state)) {
	rec = getState(state,NO,fea,XPC(2));
	if (rec==NULL) {
	  predArity = MaxToSave(2,3);
	  PC += 6;
	  goto LBLreplaceBICall;
	}
      } else {
	rec = getRecord(state);
      }
      Assert(rec!=NULL);
      int index = ((InlineCache*)(PC+4))->lookup(rec,fea);
      if (index>=0) {
	XPC(2) = rec->getArg(index);
	DISPATCH(6);
      }
      (void) oz_raise(E_ERROR,E_OBJECT,"@",2,makeTaggedSRecord(rec),fea);
      goto LBLraise;
    }

  Case(INLINEASSIGN)
    {      
      TaggedRef fea = getLiteralArg(PC+1);

      Object *self = e->getSelf();

      if (!e->onToplevel() && !e->isCurrentBoard(GETBOARD(self))) {
	(void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("object"));
	goto LBLraise;
     }

      RecOrCell state = self->getState();
      SRecord *rec;
      if (stateIsCell(state)) {
	rec = getState(state,OK,fea,XPC(2));
	if (rec==NULL) {
	  predArity = getPosIntArg(PC+3);
	  PC += 6;
	  goto LBLreplaceBICall;
	}
      } else {
	rec = getRecord(state);
      }
      Assert(rec!=NULL);
      int index = ((InlineCache*)(PC+4))->lookup(rec,fea);
      if (index>=0) {
	Assert(isRef(*rec->getRef(index)) || !isAnyVar(*rec->getRef(index)));
	rec->setArg(index,XPC(2));
	DISPATCH(6);
      }
      
      (void) oz_raise(E_ERROR,E_OBJECT,"<-",3, makeTaggedSRecord(rec), fea, XPC(2));
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
	  CheckLiveness(PC,getPosIntArg(PC+4));
	  PushContX(PC,Y,G,X,getPosIntArg(PC+4));
	  SUSPENDONVARLIST;

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
      COUNT(inlinecalls);
      InlineFun3 fun = (InlineFun3)GetBI(PC+1)->getInlineFun();
      Assert(fun != NULL);

#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif
      // note XPC(5) is maybe the same as XPC(2) or XPC(3) or XPC(4) !!
      OZ_Return res = fun(XPC(2),XPC(3),XPC(4),XPC(5));
      if (res==PROCEED) { DISPATCH(7); }
      if (res==FAILED)  { SHALLOWFAIL; Assert(0); }

      switch(res) {

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
	  CheckLiveness(PC,getPosIntArg(PC+6));
	  PushContX(PC,Y,G,X,getPosIntArg(PC+6));
	  suspendInline(CTT,A,B,C);
	  goto LBLsuspendThread;
	}

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1_FUN(GetBI(PC+1)->getPrintName(),
			cons(XPC(2),cons(XPC(3),cons(XPC(4),nil()))));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEEQEQ)
    {
      InlineFun2 fun = (InlineFun2)GetBI(PC+1)->getInlineFun();
      Assert(fun != NULL);

#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif
      // note XPC(4) is maybe the same as XPC(2) or XPC(3) !!
      OZ_Return res = fun(XPC(2),XPC(3),XPC(4));
      if (res==PROCEED) { DISPATCH(6); }

      Assert(res==SUSPEND);
      CheckLiveness(PC,getPosIntArg(PC+5));
      PushContX(PC,Y,G,X,getPosIntArg(PC+5));
      SUSPENDONVARLIST;
    }

#undef SHALLOWFAIL

// ------------------------------------------------------------------------
// INSTRUCTIONS: Shallow guards stuff
// ------------------------------------------------------------------------

  Case(SHALLOWGUARD)
    {
      shallowCP = PC;
      e->shallowHeapTop = heapTop;
      e->trail.pushMark();
      DISPATCH(3);
    }

  Case(SHALLOWTEST1)
    {
      COUNT(inlinecalls);
      InlineRel1 rel = (InlineRel1)GetBI(PC+1)->getInlineFun();
      Assert(rel != NULL);

#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif
      OZ_Return res = rel(XPC(2));
      if (res==PROCEED) { DISPATCH(5); }
      if (res==FAILED)  { JUMPRELATIVE(getLabelArg(PC+3)); }

      switch(res) {

      case SUSPEND:
	CheckLiveness(PC,getPosIntArg(PC+4));
	PushContX(PC,Y,G,X,getPosIntArg(PC+4));
	addSusp (XPC(2), CTT);
	goto LBLsuspendThread;

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(GetBI(PC+1)->getPrintName(),cons(XPC(2),nil()));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(SHALLOWTEST2)
    {
      COUNT(inlinecalls);
      InlineRel2 rel = (InlineRel2)GetBI(PC+1)->getInlineFun();
      Assert(rel != NULL);
      
#ifdef PROFILE_BI
      GetBI(PC+1)->incCounter();
#endif
      OZ_Return res = rel(XPC(2),XPC(3));
      if (res==PROCEED) { DISPATCH(6); }
      if (res==FAILED)  { JUMPRELATIVE(getLabelArg(PC+4)); }

      switch(res) {

      case SUSPEND:
	{
	  CheckLiveness(PC,getPosIntArg(PC+5));
	  PushContX(PC,Y,G,X,getPosIntArg(PC+5));
	  suspendInline(CTT,XPC(2),XPC(3));
	  goto LBLsuspendThread;
	}

      case RAISE:
	goto LBLraise;

      case BI_TYPE_ERROR:
	RAISE_TYPE1(GetBI(PC+1)->getPrintName(),
		    cons(XPC(2),cons(XPC(3),nil())));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(TESTLESS)
    {
      COUNT(inlinecalls);
      
      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);
      TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
      
      if (tagA == tagB) {
	if (tagA == SMALLINT) {
	  if (smallIntLess(A,B)) 
	    goto LessThenCase;
	  else
	    goto LessElseCase;
	}
	
	if (isFloat(tagA)) {
	  if (floatValue(A) < floatValue(B))
	    goto LessThenCase;
	  else
	    goto LessElseCase;
	}
	
	if (tagA == LITERAL) {
	  if (isAtom(A) && isAtom(B)) {
	    if  (strcmp(tagged2Literal(A)->getPrintName(),
			   tagged2Literal(B)->getPrintName()) < 0)
	      goto LessThenCase;
	    else
	      goto LessElseCase;
	  }
	}
      }
      predArity = (int) BILessOrLessEq(OK,XPC(1),XPC(2));
      auxString = "<";
      goto LBLhandleLess;
    }

  Case(TESTLESSEQ)
    {
      COUNT(inlinecalls);
      
      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);
      TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
      
      if (tagA == tagB) {
	if (tagA == SMALLINT) {
	  if (smallIntLE(A,B)) 
	    goto LessThenCase;
	  else
	    goto LessElseCase;
	}
	
	if (isFloat(tagA)) {
	  if (floatValue(A) <= floatValue(B))
	    goto LessThenCase;
	  else
	    goto LessElseCase;
	}
	
	if (tagA == LITERAL) {
	  if (isAtom(A) && isAtom(B)) {
	    if  (strcmp(tagged2Literal(A)->getPrintName(),
			   tagged2Literal(B)->getPrintName()) <= 0)
	      goto LessThenCase;
	    else
	      goto LessElseCase;
	  }
	}
      }
      predArity = (int) BILessOrLessEq(NO,XPC(1),XPC(2));
      auxString = "=<";
      goto LBLhandleLess;

    }
  
    {
    LessThenCase:
      DISPATCH(5); 
    LessElseCase: 
      JUMPRELATIVE(getLabelArg(PC+3));
    }


  LBLhandleLess:
    {
      OZ_Return res = (OZ_Return) predArity;
      switch(res) {
	
      case PROCEED: goto LessThenCase;
      case FAILED:  goto LessElseCase;
	
      case SUSPEND:
	{
	  CheckLiveness(PC,getPosIntArg(PC+4));
	  PushContX(PC,Y,G,X,getPosIntArg(PC+4));
	  suspendInline(CTT,XPC(1),XPC(2));
	  goto LBLsuspendThread;
	}
      
      case BI_TYPE_ERROR:
	RAISE_TYPE1(auxString,cons(XPC(1),cons(XPC(2),nil())));
	
      default:
	Assert(0);
      }
    }
    
  Case(SHALLOWTHEN)
    {
      if (e->trail.isEmptyChunk()) {
	shallowCP = NULL;
	e->shallowHeapTop = NULL;
	e->trail.popMark();
	DISPATCH(1);
      }

    LBLsuspendShallow:
      {
	e->emptySuspendVarList();
	int argsToSave = getPosIntArg(shallowCP+2);
	CheckLiveness(shallowCP,argsToSave);
	PushContX(shallowCP,Y,G,X,argsToSave);
	shallowCP = NULL;
	e->shallowHeapTop = NULL;
	e->reduceTrailOnShallow();
	goto LBLsuspendThread;
      }
    }

// -------------------------------------------------------------------------
// INSTRUCTIONS: Environment
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

  Case(DEALLOCATEL1)  { deallocateY(Y,1);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL2)  { deallocateY(Y,2);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL3)  { deallocateY(Y,3);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL4)  { deallocateY(Y,4);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL5)  { deallocateY(Y,5);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL6)  { deallocateY(Y,6);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL7)  { deallocateY(Y,7);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL8)  { deallocateY(Y,8);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL9)  { deallocateY(Y,9);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL10) { deallocateY(Y,10); Y=0; DISPATCH(1); }

  Case(DEALLOCATEL)
    {
      if (Y) {
	deallocateY(Y);
	Y = NULL;
      }
      DISPATCH(1);
    }
// -------------------------------------------------------------------------
// INSTRUCTIONS: CONTROL: FAIL/SUCCESS/RETURN
// -------------------------------------------------------------------------

  Case(FAILURE)
    {
      HF_FAIL;
    }


  Case(SKIP)
    DISPATCH(1);

  Case(EXHANDLER)
    PushCont(PC+2,Y,G);
    e->currentThread()->pushCatch();
    JUMPRELATIVE(getLabelArg(PC+1));

  Case(POPEX)
    {
      TaskStack *taskstack = CTS;
      taskstack->discardCatch();
      /* remove unused continuation for handler */
      taskstack->discardFrame(NOCODE);
      DISPATCH(1);
    }

  Case(LOCKTHREAD)
{
  int lbl = getLabelArg(PC+1);
  TaggedRef aux      = XPC(2);
  int toSave         = getPosIntArg(PC+3);
  
  DEREF(aux,auxPtr,_1);
  if (isAnyVar(aux)) {
    SUSP_PC(auxPtr,toSave,PC);}
  
  if (!isLock(aux)) {
    /* arghhhhhhhhhh! fucking exceptions (RS) */
    (void) oz_raise(E_ERROR,E_KERNEL,"type",5,NameUnit,NameUnit,OZ_atom("Lock"),
    OZ_int(1),
    OZ_string(""));
    RAISE_TYPE1("lock",cons(aux,nil()));
    goto LBLraise;
  }
  
  OzLock *t = (OzLock*)tagged2Tert(aux);
  Thread *th=e->currentThread();
  
  if(t->getTertType()==Te_Local){
    if(!e->onToplevel()){
      if (!e->isCurrentBoard(GETBOARD((LockLocal*)t))) {
	(void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("lock"));
	goto LBLraise;}}
    if(((LockLocal*)t)->hasLock(th)) {goto has_lock;}
    if(((LockLocal*)t)->lockB(th)) {goto got_lock;}
    goto no_lock;}

  if(!e->onToplevel()){
    (void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("lock"));}    
  
  switch(t->getTertType()){
  case Te_Frame:{
    if(((LockFrame*)t)->hasLock(th)) {goto has_lock;}
    if(((LockFrame*)t)->lockB(e->currentThread())){goto got_lock;}
    goto no_lock;}
  case Te_Proxy:{
    ((LockProxy*)t)->lock(th);
    goto no_lock;}
  case Te_Manager:{
    if(((LockManager*)t)->hasLock(th)) {goto has_lock;}
    if(((LockManager*)t)->lockB(th)){goto got_lock;}
    goto no_lock;}
  default:
    Assert(0);}
  
  got_lock:
    PushCont(PC+lbl,Y,G);
    CTS->pushLock(t);
    DISPATCH(4);
  
  has_lock:
    PushCont(PC+lbl,Y,G);
    DISPATCH(4);
  
  no_lock:
    PushCont(PC+lbl,Y,G);
    CTS->pushLock(t);
    CheckLiveness(PC+4,toSave);
    PushContX((PC+4),Y,G,X,toSave);      /* ATTENTION */
    goto LBLsuspendThread;
  }

  Case(RETURN)

    LBLpopTask:
      {
	emulateHookPopTask(e);

	Assert(!CTT->isSuspended());
	Assert(CBB==currentDebugBoard);
	DebugCheckT(CAA = NULL);

      LBLpopTaskNoPreempt:
	Assert(CTS==CTT->getTaskStackRef());
	PopFrameNoDecl(CTS,PC,Y,G);
	JUMPABSOLUTE(PC);
      }

// ------------------------------------------------------------------------
// INSTRUCTIONS: Definition
// ------------------------------------------------------------------------

  Case(DEFINITIONCOPY)
    isTailCall = OK; // abuse for indicating that we have to copy
    goto LBLDefinition;

  Case(DEFINITION)
      isTailCall = NO;

    LBLDefinition:
    {
      Reg reg                     = getRegArg(PC+1);
      int nxt                     = getLabelArg(PC+2);
      PrTabEntry *predd           = getPredArg(PC+3);
      AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+4);
      AssRegArray *list           = (AssRegArray*) getAdressArg(PC+5);

      if (predd->getPC()==NOCODE) {
        predd->PC = PC+sizeOf(DEFINITION);
      }
      
      predd->numClosures++;
      
      if (predd->numClosures > 1 && predd->copyOnce) {
	(void) oz_raise(E_ERROR,E_SYSTEM,"onceOnlyFunctor",0);
	goto LBLraise;
      }

      if (isTailCall) {
	TaggedRef dict = deref(Xreg(reg));
	if (isDictionary(dict))
	  predd->PC = copyCode(predd->PC,tagged2Dictionary(dict),predd->copyOnce==NO);
	else 
	  warning("DEFINITIONCOPY: dictionary expected: %s\n",toC(dict));
      }
      int size = list->getSize();
      RefsArray gRegs = (size == 0) ? (RefsArray) NULL : allocateRefsArray(size);

      Abstraction *p = new Abstraction (predd, gRegs, CBB);

      COUNT1(sizeClosures,sizeof(Abstraction)+(size+1)*sizeof(TaggedRef));
      COUNT(numClosures);
      COUNT1(sizeGs,size);

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
      JUMPRELATIVE(nxt);
    }

// -------------------------------------------------------------------------
// INSTRUCTIONS: CONTROL: FENCE/CALL/EXECUTE/SWITCH/BRANCH
// -------------------------------------------------------------------------
  
  Case(BRANCH)
    JUMPRELATIVE( getLabelArg(PC+1) );

  
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
      Object *obj      = tagged2Object(object);
      Abstraction *def = getSendMethod(obj,label,arity,(InlineCache*)(PC+4),X);
      if (def == NULL) {
	goto bombSend;
      }

      if (!isTailCall) PushCont(PC+6,Y,G);
      ChangeSelf(obj);
      CallDoChecks(def,def->getGRegs());
      COUNT(sendmsg);
      JUMPABSOLUTE(def->getPC());
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
    TaggedRef origCls   = RegAccess(HelpReg,getRegArg(PC+2));
    TaggedRef cls       = origCls;
    Abstraction *def       = NULL;

    if (!isTailCall) PC = PC+3;

    DEREF(cls,clsPtr,clsTag);
    if (!isObjectClass(cls)) {
      goto bombApply;
    }
    def = getApplyMethod(tagged2ObjectClass(cls),ami,arity,X);
    if (def==NULL) {
      goto bombApply;
    }
    
    if (!isTailCall) { PushCont(PC,Y,G); }
    COUNT(applmeth);
    CallDoChecks(def,def->getGRegs());
    JUMPABSOLUTE(def->getPC());


  bombApply:
    Assert(tagged2ObjectClass(cls)->getFallbackApply());

    X[1] = makeMessage(arity,ami->methName,X);
    X[0] = origCls;

    predArity = 2;
    predicate = tagged2Const(tagged2ObjectClass(cls)->getFallbackApply());
    goto LBLcall;
  }


  Case(CALLX) isTailCall = NO; ONREG(Call,X);
  Case(CALLY) isTailCall = NO; ONREG(Call,Y);
  Case(CALLG) isTailCall = NO; ONREG(Call,G);

  Case(TAILCALLX) isTailCall = OK; ONREG(Call,X);
  Case(TAILCALLY) isTailCall = OK; ONREG(Call,Y);
  Case(TAILCALLG) isTailCall = OK; ONREG(Call,G);

 Call:
  asmLbl(TAILCALL);
   {
     {
       TaggedRef taggedPredicate = RegAccess(HelpReg,getRegArg(PC+1));
       predArity = getPosIntArg(PC+2);

       DEREF(taggedPredicate,predPtr,_1);

       if (isAbstraction(taggedPredicate)) {
         Abstraction *def = tagged2Abstraction(taggedPredicate);
	 PrTabEntry *pte = def->getPred();
         CheckArity(pte->getArity(), taggedPredicate);
         if (!isTailCall) { PushCont(PC+3,Y,G); }
         CallDoChecks(def,def->getGRegs());
         JUMPABSOLUTE(pte->getPC());
       }

       if (!isProcedure(taggedPredicate) && !isObject(taggedPredicate)) {
	 if (isAnyVar(taggedPredicate)) {
	   /* compiler ensures: if pred is in X[n], then n == arity+1,
	    * so we save one additional argument */
	   Assert(HelpReg!=X || predArity==regToInt(getRegArg(PC+1)));
	   SUSP_PC(predPtr,predArity+1,PC);
	 }
	 RAISE_APPLY(taggedPredicate,OZ_toList(predArity,X));
       }

       if (!isTailCall) PC = PC+3;
       predicate = tagged2Const(taggedPredicate);
     }

// -----------------------------------------------------------------------
// --- Call: entry point
// -----------------------------------------------------------------------

  LBLcall:
     COUNT(nonoptcalls);
     BuiltinTabEntry *bi;

// -----------------------------------------------------------------------
// --- Call: Abstraction
// -----------------------------------------------------------------------

     {
       TypeOfConst typ = predicate->getType();

       if (typ==Co_Abstraction || typ==Co_Object) {
	 Abstraction *def;
	 if (typ==Co_Object) {
	   COUNT(nonoptsendmsg);
	   Object *o = (Object*) predicate;
	   Assert(o->getClass()->getFallbackSend());
	   def = tagged2Abstraction(o->getClass()->getFallbackSend());
	   /* {Obj Msg} --> {Send Msg Class Obj} */
	   X[predArity++] = makeTaggedConst(o->getClass());
	   X[predArity++] = makeTaggedConst(o);
	 } else {
	   def = (Abstraction *) predicate;
	 }
	 CheckArity(def->getArity(), makeTaggedConst(def));
	 if (!isTailCall) { PushCont(PC,Y,G); }
       
	 CallDoChecks(def,def->getGRegs());
	 JUMPABSOLUTE(def->getPC());
       }

// -----------------------------------------------------------------------
// --- Call: Builtin
// -----------------------------------------------------------------------
       Assert(typ==Co_Builtin);
       COUNT(nonoptbicalls);
     
       bi = (BuiltinTabEntry *) predicate;
	
       CheckArity(bi->getArity(),makeTaggedConst(bi));
	   
       OZ_CFun biFun = bi->getFun();
#ifdef PROFILE_BI
       bi->incCounter();
#endif
       OZ_Return res = biFun(predArity, X);
	     
       switch (res) {
	    
       case SUSPEND:
	 {
	   if (!isTailCall) PushCont(PC,Y,G);

	   CTT->pushCFun(biFun,X,predArity,OK);
	   SUSPENDONVARLIST;
	 }

       case PROCEED:
	 if (isTailCall) {
	   goto LBLpopTask;
	 }
	 JUMPABSOLUTE(PC);
	 
       case SLEEP:         Assert(0);
       case RAISE:         goto LBLraise;
       case BI_TYPE_ERROR: RAISE_TYPE;
       case FAILED:        HF_BI;

       case BI_PREEMPT:
	 if (!isTailCall) {
	   PushCont(PC,Y,G);
	 }
	 goto LBLpreemption;
	 
      case BI_REPLACEBICALL: 
	if (isTailCall) {
	  PC=NOCODE;
	}
	goto LBLreplaceBICall;

       default: Assert(0);
       }
     }
// ------------------------------------------------------------------------
// --- Call: Builtin: replaceBICall
// ------------------------------------------------------------------------

   LBLreplaceBICall:
     {
       if (PC != NOCODE) {
	 PopFrame(CTS,auxPC,auxY,auxG);

	 PushContX(PC,Y,G,X,predArity);
	 CTS->pushFrame(auxPC,auxY,auxG);
       }
#if 0
       // don't like that much flickering for now ... -BL
       if (e->debugmode() && CTT->getTrace())
	 debugStreamUpdate(CTT);
#endif
       if (e->suspendVarList) {
	 SUSPENDONVARLIST;
       }
       goto LBLpopTask;
     }
   }
// --------------------------------------------------------------------------
// --- end call/execute -----------------------------------------------------
// --------------------------------------------------------------------------

// -------------------------------------------------------------------------
// INSTRUCTIONS: Actors/Deep Guards
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

      if ( e->entailment() ) { // OPT commit()
	e->trail.popMark();
	Board *tmpBB = CBB;
	e->setCurrent(CBB->getParent());
	DebugCheckT(currentDebugBoard=CBB);
	tmpBB->unsetInstalled();
	tmpBB->setCommitted(CBB);
	CBB->decSuspCount();
	CTS->discardActor();
	WaitActor::Cast(CAA)->disposeWait();
	CAA = NULL;
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
      if (e->entailment()) { // OPT commit()
	e->trail.popMark();
	Board *tmpBB = CBB;
	e->setCurrent(CBB->getParent());
	DebugCheckT(currentDebugBoard=CBB);
	tmpBB->unsetInstalled();
	tmpBB->setCommitted(CBB);
	CBB->decSuspCount();
	CTS->discardActor();
	AskActor::Cast(CAA)->disposeAsk();
	CAA = NULL;
	DISPATCH(1);
      }

    LBLsuspendBoard:
      CBB->setBody(PC+1, Y, G,NULL,0);
      Assert(CAA == AWActor::Cast(CBB->getActor()));

      e->deinstallCurrent();
      DebugCode(currentDebugBoard=CBB);

    LBLcheckFlat2:
      Assert(!CAA->isCommitted());
      Assert(CAA->getThread()==CTT);

      if (CAA->hasNext()) {
	LOADCONT(CAA->getNext());
	goto LBLemulate; // no thread switch allowed here (CAA)
      }

      if (CAA->isWait()) {
	WaitActor *wa = WaitActor::Cast(CAA);
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
	  goto LBLpopTask;
	}

	// suspend wait actor
	goto LBLsuspendThread;
      }

      Assert(CAA->isAsk());
      {
	AskActor *aa = AskActor::Cast(CAA);

	//  should we activate the 'else' clause?
	if (aa->isLeaf()) { // OPT commit()
	  CTS->discardActor();
	  aa->setCommitted();
	  CBB->decSuspCount();

	  LOADCONT(aa->getNext());
	  PC = aa->getElsePC();

	  aa->disposeAsk();

	  // rule: if else fail fi --> fail
	  // mm2: this should be removed (compiler should generate code!)
	  if (PC == NOCODE) {
	    HF_COND;
	  }

	  goto LBLemulate;
	}

	goto LBLsuspendThread;
      }
    }

  Case(CREATECOND)
    {
      ProgramCounter elsePC = PC+getLabelArg(PC+1);
      int argsToSave = getPosIntArg(PC+2);

      CAA = new AskActor(CBB,CTT,
			 elsePC ? elsePC : NOCODE,
			 NOCODE, Y, G, X, argsToSave);
      CTS->pushActor(CAA,PC);
      CBB->incSuspCount(); 
      DISPATCH(3);
    }

  Case(CREATEOR)
    {
      CAA = new WaitActor(CBB, CTT, NOCODE, Y, G, X, 0, NO);
      CTS->pushActor(CAA,PC);
      CBB->incSuspCount(); 

      DISPATCH(1);
    }

  Case(CREATEENUMOR)
    {
      Board *bb = CBB;

      CAA = new WaitActor(bb, CTT, NOCODE, Y, G, X, 0, NO);
      CTS->pushActor(CAA,PC);
      CBB->incSuspCount(); 

      if (bb->isWait()) {
	WaitActor::Cast(bb->getActor())->addChoice((WaitActor *) CAA);
      } else if (bb->isSolve()) {
	SolveActor::Cast(bb->getActor())->addChoice((WaitActor *) CAA);
      }

      DISPATCH(1);
    }

  Case(CREATECHOICE)
    {
      Board *bb = CBB;

      CAA = new WaitActor(bb, CTT, NOCODE, Y, G, X, 0, OK);
      CTS->pushActor(CAA,PC);
      CBB->incSuspCount(); 

      Assert(CAA->isChoice());

      if (bb->isWait()) {
	WaitActor::Cast(bb->getActor())->addChoice((WaitActor *) CAA);
      } else if (bb->isSolve()) {
	SolveActor::Cast(bb->getActor())->addChoice((WaitActor *) CAA);
      }

      DISPATCH(1);
    }

  Case(CLAUSE)
    {
      Board *bb;
      if (CAA->isAsk()) {
	bb = new Board(CAA,Bo_Ask);
	AskActor::Cast(CAA)->addAskChild();
      } else {
	bb = new Board(CAA,Bo_Wait);
	WaitActor::Cast(CAA)->addWaitChild(bb);
      }
      e->setCurrent(bb,OK);
      CBB->incSuspCount();
      DebugCode(currentDebugBoard=CBB);
      CBB->setInstalled();
      e->trail.pushMark();
      Assert(CAA->getThread()==CTT);
      DISPATCH(1);
    }

  // == CLAUSE, WAIT
  Case(EMPTYCLAUSE)
    {
      Assert(CAA->isWait());
      Board *bb = new Board(CAA, Bo_Wait | Bo_Waiting);
      WaitActor::Cast(CAA)->addWaitChild(bb);

      bb->setBody(PC+1, Y, G, NULL,0);

      goto LBLcheckFlat2;
    }

  Case(NEXTCLAUSE)
      CAA->nextClause(PC+getLabelArg(PC+1));
      DISPATCH(2);

  Case(LASTCLAUSE)
      CAA->lastClause();
      DISPATCH(1);

  Case(THREAD)
    {
      ProgramCounter newPC = PC+2;
      int contPC = getLabelArg(PC+1);

      int prio = CPP;

      if (prio > DEFAULT_PRIORITY) {
	prio = DEFAULT_PRIORITY;
      }

      Thread *tt = e->mkRunnableThreadOPT(prio, CBB);

      COUNT(numThreads);
      ozstat.createdThreads.incf();
      RefsArray newY = Y==NULL ? (RefsArray) NULL : copyRefsArray(Y);

      tt->getTaskStackRef()->pushCont(newPC,newY,G);
      tt->setSelf(e->getSelf());
      tt->setAbstr(ozstat.currAbstr);

      e->scheduleThread (tt);
      
      JUMPRELATIVE(contPC);
    }

  Case(THREADX)
    {
      ProgramCounter newPC = PC+2;
      int n = getPosIntArg(PC+1);
      int contPC = getLabelArg(PC+2);

      int prio = CPP;

      if (prio > DEFAULT_PRIORITY) {
	prio = DEFAULT_PRIORITY;
      }

      Thread *tt = e->mkRunnableThreadOPT(prio, CBB);

      COUNT(numThreads);
      ozstat.createdThreads.incf();

      tt->getTaskStackRef()->pushCont(newPC,0,G);
      tt->getTaskStackRef()->pushX(X,n);
      tt->setSelf(e->getSelf());
      tt->setAbstr(ozstat.currAbstr);

      e->scheduleThread(tt);
      
      JUMPRELATIVE(contPC);
    }

// -------------------------------------------------------------------------
// INSTRUCTIONS: MISC: ERROR/NOOP/default
// -------------------------------------------------------------------------

  Case(TASKEMPTYSTACK)  
    {
      Assert(Y==0 && G==0);
      CTS->pushEmpty();   // mm2?
      goto LBLterminateThread;
    }

  Case(TASKACTOR)  
    {
      // this is the second part of Space.choose (see builtin.cc)
      WaitActor *wa = WaitActor::Cast((Actor *) Y);
      CTS->restoreFrame();
      if (wa->getChildCount() != 1) {
	goto LBLsuspendThread;
      }
      Board *bb = wa->getChildRef();
      Assert(bb->isWait());

      if (!am.commit(bb,CTT)) {
	goto LBLfailure; // ???
      }
      goto LBLpopTask;
    }

  Case(TASKPROFILECALL)
    {
      ozstat.leaveCall((PrTabEntry*)Y);
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKCALLCONT)
    {
      TaggedRef taggedPredicate = (TaggedRef)ToInt32(Y);

      predArity = G ? getRefsArraySize(G) : 0;

      DEREF(taggedPredicate,predPtr,predTag);
      if (!isProcedure(taggedPredicate) && !isObject(taggedPredicate)) {
	if (isAnyVar(predTag)) {
	  CTS->pushCallNoCopy(makeTaggedRef(predPtr),G);
	  addSusp(predPtr,CTT);
	  goto LBLsuspendThread;
	}
	RAISE_APPLY(taggedPredicate,OZ_toList(predArity,G));
      }

      RefsArray tmpX = G;
      Y = G = NULL;
      int i = predArity;
      while (--i >= 0) {
	X[i] = tmpX[i];
      }
      disposeRefsArray(tmpX);
      DebugTrace(trace("call cont task",CBB));
      isTailCall = OK;

      predicate=tagged2Const(taggedPredicate);
      goto LBLcall;
    }

  Case(TASKLOCK)
    {
      OzLock *lck = (OzLock *) Y;
      Y = NULL;
      switch(lck->getTertType()){
      case Te_Local:
	((LockLocal*)lck)->unlock();
	break;
      case Te_Frame:
	((LockFrame*)lck)->unlock(am.currentThread());
	break;
      case Te_Proxy:
	oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("lock"));
	goto LBLraise;
      case Te_Manager:
	((LockManager*)lck)->unlock(am.currentThread());
	break;}
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
    {
      e->setSelf((Object*)Y);
      Y = NULL;
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKDEBUGCONT)
    {
      error("Emulate: TASKDEBUGCONT instruction executed");
      goto LBLerror;
    }
  
  Case(TASKCFUNCONT)
     {
       // 
       // by kost@ : 'solve actors' are represented via a c-function; 
       OZ_CFun biFun = (OZ_CFun) (void*) Y;
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

       case BI_REPLACEBICALL: 
	 PC = NOCODE;
	 goto LBLreplaceBICall;

       case SUSPEND:
	 CTT->pushCFun(biFun,X,predArity,OK);
	 SUSPENDONVARLIST;

      case BI_PREEMPT:
	goto LBLpreemption;

       case SLEEP:
       default:
	 Assert(0);
       }
     }

  Case(TASKLTQ)
     {
       Y = NULL;  // sa here unused
       Assert(e->currentBoard()->isSolve());
       Assert(!e->onToplevel());
       Assert(CTS->isEmpty()); // approximates one LTQ task
       
       // postpone poping task from taskstack until 
       // local thread queue is empty
       SolveActor * sa = SolveActor::Cast(e->currentBoard()->getActor());
       LocalThreadQueue * ltq = sa->getLocalThreadQueue();

#ifdef DEBUG_LTQ
       cout << "sa=" << sa << " emu " << " thr=" 
	    << e->currentThread() << endl << flush;
#endif

       Assert(!ltq->isEmpty());

       unsigned int starttime = 0;

       if (ozconf.timeDetailed)
	 starttime = osUserTime();
       
       Thread * backup_currentThread = CTT;

       while (!ltq->isEmpty() && e->isNotPreemptiveScheduling()) {
	 Thread * thr = ltq->dequeue();
	 e->setCurrentThread(thr);
	 Assert(!thr->isDeadThread());
	  
	 OZ_Return r = e->runPropagator(thr);

	 if (r == SLEEP) {
	   e->suspendPropagator(thr);
	 } else if (r == PROCEED) {
	   e->closeDonePropagator(thr);
	 } else if (r == FAILED) {
	   e->closeDonePropagator(thr);
	   e->setCurrentThread(backup_currentThread);

	   if (ozconf.timeDetailed)
	     ozstat.timeForPropagation.incf(osUserTime()-starttime);

	   CTS->pushLTQ(sa); // RS: is this needed ???
	   // failure of propagator is never catched !
	   goto LBLfailure; // top-level failure not possible
	 } else {
	   Assert(r == SCHEDULED);
	   e->scheduledPropagator(thr);
	 }
       } 
	
       e->setCurrentThread(backup_currentThread);

       if (ozconf.timeDetailed)
	 ozstat.timeForPropagation.incf(osUserTime()-starttime);


       if (ltq->isEmpty()) {
	 sa->resetLocalThreadQueue();
#ifdef DEBUG_LTQ
	 cout << "sa emu sa=" << sa << " EMPTY" << endl << flush;
#endif
	 goto LBLpopTask;
       } else {
#ifdef DEBUG_LTQ
	 cout << "sa emu sa=" << sa << " PREEMPTIVE" << endl << flush;
#endif
	 CTS->pushLTQ(sa);
	 Assert(sa->getLocalThreadQueue());
	 goto LBLpreemption;
       }
     }
    
  Case(OZERROR)
    {
      error("Emulate: OZERROR instruction executed");
      goto LBLerror;
    }

  Case(DEBUGENTRY)
    {
      if ((e->debugmode() || CTT->getTrace()) && e->onToplevel()) {
	int line = smallIntValue(getNumberArg(PC+2));
	if (line < 0) {
	  execBreakpoint(e->currentThread());
	}

	OzDebug *dbg = new OzDebug(PC,Y,G);

	TaggedRef kind = getTaggedArg(PC+4);
	if (literalEq(kind,AtomDebugCall)) {
	  // save abstraction and arguments:
	  int arity = -1;
	  switch (CodeArea::getOpcode(PC+6)) {
	  case CALLX:
	    dbg->data = Xreg(getRegArg(PC+7));
	    arity = getPosIntArg(PC+8);
	    break;
	  case CALLY:
	    dbg->data = Yreg(getRegArg(PC+7));
	    arity = getPosIntArg(PC+8);
	    break;
	  case CALLG:
	    dbg->data = Greg(getRegArg(PC+7));
	    arity = getPosIntArg(PC+8);
	    break;
	  case CALLBUILTIN:
	    dbg->data = makeTaggedConst(GetBI(PC+7));
	    arity = getPosIntArg(PC+8);
	    break;
	  case GENFASTCALL:
	  case FASTCALL:
	  case FASTTAILCALL:
	    {
	      Abstraction *abstr =
		((AbstractionEntry *) getAdressArg(PC+7))->getAbstr();
	      dbg->data = makeTaggedConst(abstr);
	      arity = abstr->getArity();
	    }
	    break;
	  case MARSHALLEDFASTCALL:
	    dbg->data = getTaggedArg(PC+7);
	    arity = getPosIntArg(PC+8) >> 1;
	    break;
	  case WEAKDETX:
	    dbg->data = OZ_atom("weakDet");
	    dbg->arguments = allocateRefsArray(2,NO);
	    dbg->arguments[0] = Xreg(getRegArg(PC+7));
	    dbg->arguments[1] = makeTaggedNULL();
	    break;
	  case WEAKDETY:
	    dbg->data = OZ_atom("weakDet");
	    dbg->arguments = allocateRefsArray(2,NO);
	    dbg->arguments[0] = Yreg(getRegArg(PC+7));
	    dbg->arguments[1] = makeTaggedNULL();
	    break;
	  case WEAKDETG:
	    dbg->data = OZ_atom("weakDet");
	    dbg->arguments = allocateRefsArray(2,NO);
	    dbg->arguments[0] = Greg(getRegArg(PC+7));
	    dbg->arguments[1] = makeTaggedNULL();
	    break;
	  default:
	    break;
	  }
	  if (arity >= 0) {
	    dbg->arguments = allocateRefsArray(arity+1,NO);
	    for (int i = 0; i < arity; i++) {
	      dbg->arguments[i] = Xreg(intToReg(i));
	    }
	    dbg->arguments[arity] = makeTaggedNULL();
	  }
	} else if (literalEq(kind, AtomDebugLock)) {
	  // save the lock:
	  switch (CodeArea::getOpcode(PC+6)) {
	  case LOCKTHREAD:
	    dbg->data = Xreg(getRegArg(PC+8));
	    break;
	  default:
	    break;
	  }
	} else if (literalEq(kind, AtomDebugCond)) {
	  // look whether we can determine the arbiter:
	  switch (CodeArea::getOpcode(PC+6)) {
	  case TESTLITERALX:
	  case TESTNUMBERX:
	  case TESTBOOLX:
	  case SWITCHONTERMX:
	    dbg->data = Xreg(getRegArg(PC+7));
	    break;
	  case TESTLITERALY:
	  case TESTNUMBERY:
	  case TESTBOOLY:
	  case SWITCHONTERMY:
	    dbg->data = Yreg(getRegArg(PC+7));
	    break;
	  case TESTLITERALG:
	  case TESTNUMBERG:
	  case TESTBOOLG:
	  case SWITCHONTERMG:
	    dbg->data = Greg(getRegArg(PC+7));
	    break;
	  default:
	    break;
	  }
	}

	if (CTT->getStep()) {
	  CTT->pushDebug(dbg,DBG_STEP);
	  debugStreamEntry(dbg,CTT->getTaskStackRef()->getFrameId());
	  int regsToSave = getPosIntArg(PC+5);
	  INCFPC(6);
	  PushContX(PC,Y,G,X,regsToSave);
	  goto LBLpreemption;
	} else {
	  CTT->pushDebug(dbg,DBG_NOSTEP);
	}
      }

      DISPATCH(6);
    }
  
  Case(DEBUGEXIT)
    {
      OzDebug *dbg;
      OzDebugDoit dothis;
      CTT->popDebug(dbg, dothis);

      if (dbg != (OzDebug *) NULL) {
	Assert(literalEq(getLiteralArg(dbg->PC+4),getLiteralArg(PC+4)));
	Assert(dbg->Y == Y && dbg->G == G);

	switch (dothis) {
	case DBG_STEP:
	  if (CTT->getTrace()) {
	    dbg->PC = PC;
	    CTT->pushDebug(dbg,DBG_EXIT);
	    debugStreamExit(dbg,CTT->getTaskStackRef()->getFrameId());
	    PushContX(PC,Y,G,X,getPosIntArg(PC+5));
	    goto LBLpreemption;
	  }
	  break;
	case DBG_NOSTEP:
	case DBG_EXIT:
	  break;
	}

	dbg->dispose();
      }

      DISPATCH(6);
    }

  Case(GENFASTCALL)
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
      Bool tailcall           =  getPosIntArg(PC+2);

      if (entry->getAbstr() == 0) {
	(void) oz_raise(E_ERROR,E_SYSTEM,"inconsistentFastcall",0);
	goto LBLraise;
      }
      CodeArea::writeOpcode(tailcall ? FASTTAILCALL : FASTCALL, PC);
      DISPATCH(0);
    }

  Case(MARSHALLEDFASTCALL)
    {
      TaggedRef pred = getTaggedArg(PC+1);
      int tailcallAndArity  = getPosIntArg(PC+2);

      DEREF(pred,predPtr,_1);
      if (isAnyVar(pred)) {
	SUSP_PC(predPtr,tailcallAndArity>>1,PC);
      }

      OZ_unprotect((TaggedRef*)(PC+1));

      if (!changeMarshaledFastCall(PC,pred,tailcallAndArity)) {
	RAISE_APPLY(pred,cons(OZ_atom("proc or builtin expected."),nil()));
      }

      DISPATCH(0);
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

      if (genCallInfo(gci,pred,PC,X)) {
	gci->dispose();
	DISPATCH(0);
      }

      isTailCall = gci->isTailCall;
      if (!isTailCall) PC = PC+3;

      /* the following adapted from bombApply */
      Assert(tagged2ObjectClass(pred)->getFallbackApply());

      X[1] = makeMessage(gci->arity,gci->mn,X);
      X[0] = pred;
      
      predArity = 2;
      predicate = tagged2Const(tagged2ObjectClass(pred)->getFallbackApply());
      goto LBLcall;
    }


  /* The following must be different from the following,
   * otherwise definitionEnd breaks under threaded code
   */

  Case(GLOBALVARNAME)
    {
      error("under threaded code this must be different from LOCALVARNAME,");
      error("otherwise CodeArea::adressToOpcode will not work.");
    }

  Case(LOCALVARNAME)
    {
      error("under threaded code this must be different from GLOBALVARNAME,");
      error("otherwise CodeArea::adressToOpcode will not work.");
    }

  Case(PROFILEPROC)
    {
      static int sizeOfDef = -1;
      if (sizeOfDef==-1) sizeOfDef = sizeOf(DEFINITION);
      
      Assert(CodeArea::getOpcode(PC-sizeOfDef) == DEFINITION);
      PrTabEntry *pred = getPredArg(PC-sizeOfDef+3); /* this is faster */

      pred->numCalled++;
      if (pred!=ozstat.currAbstr) {
	CTS->pushAbstr(ozstat.currAbstr);
	ozstat.leaveCall(pred);
      }
      
      DISPATCH(1);
    }

  Case(TASKCATCH)
    {
      error("impossible");
    }

  Case(ENDOFFILE)
    {
      error("Emulate: ENDOFFILE instruction executed");
      goto LBLerror;
    }

  Case(ENDDEFINITION)
    {
      error("Emulate: ENDDEFINITION instruction executed");
      goto LBLerror;
    }


  Case(TESTLABEL1)
  Case(TESTLABEL2)
  Case(TESTLABEL3)
  Case(TESTLABEL4)

  Case(TEST1)
  Case(TEST2)
  Case(TEST3)
  Case(TEST4)

#ifndef THREADED
  default:
     error("emulate instruction: default should never happen");
     break;
   } /* switch*/
#endif


// ----------------- end instructions -------------------------------------


// ------------------------------------------------------------------------
// *** FAILURE
// ------------------------------------------------------------------------

LBLshallowFail:
  {
    asmLbl(SHALLOW_FAIL);
    if (e->trail.isEmptyChunk()) {
      e->trail.popMark();
    } else {
      e->reduceTrailOnFail();
    }
    PC                 = shallowCP;
    shallowCP          = NULL;
    e->shallowHeapTop  = NULL;
    JUMPRELATIVE(getLabelArg(PC+1));
  }

  /*
   *  kost@ : There are now the following invariants:
   *  - Can be entered only in a deep guard;
   *  - current thread must be runnable.
   */
LBLfailure:
   {
  asmLbl(DEEP_FAIL);
  DebugTrace(trace("fail",CBB));

  Assert(!e->onToplevel());
  Assert(CTT);
  Assert(CTT->isRunnable());
  Assert(CBB->isInstalled());

  Actor *aa=CBB->getActor();
  Assert(!aa->isCommitted());

  if (aa->isAsk()) {
    (AskActor::Cast(aa))->failAskChild();
  }
  if (aa->isWait()) {
    (WaitActor::Cast(aa))->failWaitChild(CBB);
  }

  Assert(!CBB->isFailed());
  CBB->setFailed();

  e->reduceTrailOnFail();
  CBB->unsetInstalled();
  e->setCurrent(GETBOARD(aa));
  DebugCheckT(currentDebugBoard=CBB);


  // currentThread is a thread forked in a local space or a propagator
  if (aa->isSolve()) {

    //  Reduce (i.e. with failure in this case) the solve actor;
    //  The solve actor goes simply away, and the 'failed' atom is bound to
    // the result variable; 
    aa->setCommitted();
    SolveActor *saa=SolveActor::Cast(aa);
    // don't decrement parent counter

    if (!e->fastUnifyOutline(saa->getResult(),saa->genFailed(),0)) {
      // this should never happen?
      Assert(0);
    }

  } else if (CTT == AWActor::Cast(aa)->getThread()) {
    // pseudo flat guard
    Assert(CAA==aa);
    goto LBLcheckFlat2;
  } else {
    AWActor *aw = AWActor::Cast(aa);
    Thread *tt = aw->getThread();

    Assert(CTT != tt && GETBOARD(tt) == CBB);
    Assert(!aw->isCommitted() && !aw->hasNext());

    if (aw->isWait()) {
      WaitActor *wa = WaitActor::Cast(aw);
      /* test bottom commit */
      if (wa->hasNoChildren()) {
	if (canOptimizeFailure(e,tt)) goto LBLfailure;
      } else {
	Assert(!e->isScheduledSlow(tt));
	/* test unit commit */
	if (wa->hasOneChildNoChoice()) {
	  Board *waitBoard = wa->getLastChild();
	  int succeeded = e->commit(waitBoard);
	  if (!succeeded) {
	    if (canOptimizeFailure(e,tt)) goto LBLfailure;
	  }
	}
      }
    } else {
      Assert(!e->isScheduledSlow(tt));
      Assert(aw->isAsk());

      AskActor *aa = AskActor::Cast(aw);

      //  should we activate the 'else' clause?
      if (aa->isLeaf()) {  // OPT commit()
	aa->setCommitted();
	CBB->decSuspCount();
	TaskStack *ts = tt->getTaskStackRef();
	ts->discardActor();

	/* rule: if fi --> false */
	if (aa->getElsePC() == NOCODE) {
	  aa->disposeAsk();
	  if (canOptimizeFailure(e,tt)) goto LBLfailure;
	} else {
	  Continuation *tmpCont = aa->getNext();
	  ts->pushCont(aa->getElsePC(),
		       tmpCont->getY(), tmpCont->getG());
	  if (tmpCont->getX()) ts->pushX(tmpCont->getX());
	  aa->disposeAsk();
	  e->suspThreadToRunnableOPT(tt);
	  e->scheduleThread(tt);
	}
      }
    }
  }

#ifdef DEBUG_CHECK
  if (CTT==e->rootThread()) {
    printf("fail root thread\n");
  }
#endif

  e->decSolveThreads(CBB);
  e->disposeRunnableThread(CTT);
  e->unsetCurrentThread();

  goto LBLstart;
   }

// ------------------------------------------------------------------------
// --- Call: Builtin: raise
// ------------------------------------------------------------------------

   LBLraise:
     {
       DebugCheck(ozconf.stopOnToplevelFailure,
		  DebugTrace(tracerOn();trace("raise")));

       Assert(CTT && !CTT->isPropagator());

       shallowCP         = 0; // failure in shallow guard can never be handled
       e->shallowHeapTop = 0;

       Bool foundHdl;

       if (e->exception.debug) {

	 OZ_Term traceBack;
	 foundHdl =
	   CTT->getTaskStackRef()->findCatch(PC,&traceBack,e->debugmode());
	 
	 OZ_Term loc = oz_getLocation(CBB);
	 e->exception.value = formatError(e->exception.info,e->exception.value,
					  traceBack,loc);

       } else {
	 foundHdl = CTT->getTaskStackRef()->findCatch();
       }
       
       if (foundHdl) {
	 if (e->debugmode() && CTT->getTrace())
	   debugStreamUpdate(CTT);
	 X[0] = e->exception.value;
	 goto LBLpopTaskNoPreempt;
       }

       if (!e->onToplevel() &&
	   OZ_eq(OZ_label(e->exception.value),OZ_atom("failure"))) {
	 goto LBLfailure;
       }

       if (e->debugmode()) {
	 OZ_Term exc = e->exception.value;
	 // ignore system(kernel(terminate)) exception:
	 if (OZ_isRecord(exc) &&
	     OZ_eq(OZ_label(exc),OZ_atom("system")) &&
	     tagged2SRecord(exc)->getFeature(OZ_int(1)) != makeTaggedNULL() &&
	     OZ_eq(OZ_label(OZ_subtree(exc,OZ_int(1))),OZ_atom("kernel")) &&
	     OZ_eq(OZ_subtree(OZ_subtree(exc,OZ_int(1)),OZ_int(1)),
		   OZ_atom("terminate")))
	   ;
	 else {
	   CTT->setTrace(OK);
	   CTT->setStep(OK);
	   debugStreamException(CTT,e->exception.value);
	   goto LBLpreemption;
	 }
       }
       // else
       RefsArray argsArray = allocateRefsArray(1,NO);
       argsArray[0] = e->exception.value;
       if (e->defaultExceptionHdl) {
	 CTT->pushCall(e->defaultExceptionHdl,argsArray,1);
       } else {
	 if (!am.isStandalone()) 
	   printf("\021");
	 printf("Exception raise:\n   %s\n",toC(argsArray[0]));
	 fflush(stdout);
       }
       goto LBLpopTask; // changed from LBLpopTaskNoPreempt; -BL 26.3.97
     }
} // end engine

#ifdef OUTLINE
#undef inline
#endif

