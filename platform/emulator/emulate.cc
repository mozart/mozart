/* This may look like C code, but it is really -*- C++ -*-

  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow,mehl,scheidhr
  The main engine
  ------------------------------------------------------------------------
*/

#include "am.hh"

#include "indexing.hh"

#include "genvar.hh"
#include "dictionary.hh"
#include "fdhook.hh"

#define PROP_TIME

/*
 * Object stuff
 */

inline
Abstraction *getSendMethod(Object *obj, TaggedRef label, SRecordArity arity,
                           InlineCache *cache, RefsArray X)
{
  Assert(isFeature(label));
  return cache->lookup(obj,label,arity,X);
}

inline
Abstraction *getApplyMethod(Object *obj, ApplMethInfoClass *ami,
                            SRecordArity arity, RefsArray X)
{
  Assert(isFeature(ami->methName));
  return ami->methCache.lookup(obj,ami->methName,arity,X);
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

#define RAISE_APPLY(fun,args)                                           \
  (void) e->raise(E_ERROR,E_KERNEL,"apply",2,fun,args); goto LBLraise;


void AM::enrichTypeException(char *fun, OZ_Term args)
{
  OZ_Term e = OZ_subtree(exception.value,OZ_int(1));
  OZ_putArg(e,1,OZ_atom(fun));
  OZ_putArg(e,2,args);
}

#define RAISE_TYPE1(fun,args)                   \
  e->enrichTypeException(fun,args); goto LBLraise;

#define RAISE_TYPE1_FUN(fun,args)                               \
  RAISE_TYPE1(fun,                                              \
              appendI(args,cons(OZ_newVariable(),nil())));

#define RAISE_TYPE                                      \
   RAISE_TYPE1(builtinTab.getName((void *) biFun),      \
               OZ_toList(predArity,X));

/*
 * Handle Failure macros (HF)
 */

bool hf_raise_failure(AM *e, TaggedRef t)
{
  if (!e->isToplevel() &&
      (!e->currentThread->hasCatchFlag() ||
       e->currentBoard!=e->currentThread->getBoard())) {
    return OK;
  }
  e->exception.info  = ozconf.errorDebug?t:NameUnit;
  e->exception.value = RecordFailure;
  e->exception.debug = ozconf.errorDebug;
  return NO;
}

#define HF_RAISE_FAILURE(T)                     \
   if (hf_raise_failure(e,T))                   \
     goto LBLfailure;                           \
   goto LBLraise;


#define HF_FAIL       HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_DIS        HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_COND       HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_EQ(X,Y)    HF_RAISE_FAILURE(OZ_mkTupleC("eq",2,X,Y))
#define HF_TELL(X,Y)  HF_RAISE_FAILURE(OZ_mkTupleC("tell",2,X,Y))
#define HF_APPLY(N,A) HF_RAISE_FAILURE(OZ_mkTupleC("apply",2,N,A))
#define HF_BI         HF_APPLY(OZ_atom(builtinTab.getName((void *) biFun)), \
                               OZ_toList(predArity,X));

#define CheckArity(arityExp,proc)                                        \
if (predArity != arityExp && VarArity != arityExp) {                     \
  (void) e->raise(E_ERROR,E_KERNEL,"arity",2,proc,OZ_toList(predArity,X)); \
  goto LBLraise;                                                         \
}

// -----------------------------------------------------------------------
//

#define NOFLATGUARD   (shallowCP)

#define SHALLOWFAIL   if (shallowCP) { goto LBLshallowFail; }

#define IMPOSSIBLE(INSTR) error("%s: impossible instruction",INSTR)






// TOPLEVEL END
// -----------------------------------------------------------------------


#define DoSwitchOnTerm(indexTerm,table)                                     \
      TaggedRef term = indexTerm;                                           \
      DEREF(term,termPtr,_2);                                               \
                                                                            \
      if (!isLTuple(term)) {                                                \
        TaggedRef *sp = sPointer;                                           \
        ProgramCounter offset = switchOnTermOutline(term,termPtr,table,sp); \
        sPointer = sp;                                                      \
        JUMP(offset);                                                       \
      }                                                                     \
                                                                            \
      ProgramCounter offset = table->listLabel;                             \
      sPointer = tagged2LTuple(term)->getRef();                             \
      JUMP(offset);



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
// genCallInfo: self modifying code!

static
Bool genCallInfo(GenCallInfoClass *gci, TaggedRef pred, ProgramCounter PC)
{
  Assert(!isRef(pred));

  Abstraction *abstr = NULL;
  if (gci->isMethAppl) {
    if (!isObject(pred)) goto insertMethApply;

    Bool defaultsUsed;
    abstr = tagged2Object(pred)->getMethod(gci->mn,gci->arity,
                                           am.xRegs,defaultsUsed);
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
// CALL HOOK


/* the hook functions return:
     TRUE: must reschedule
     FALSE: can continue
   */

Bool AM::emulateHookOutline(ProgramCounter PC, Abstraction *def,
                            TaggedRef *arguments) {
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

  if (def && debugmode() && !currentSolveBoard) {
    OzDebug *dbg;
    int frameId   = ++lastFrameID % MAX_ID;

    OZ_Term dinfo = nil();
    for (int i=def->getArity()-1; i>=0; i--) {
      dinfo = cons(arguments[i],dinfo);
    }
    time_t feedtime = CodeArea::findTimeStamp(PC);
    dinfo = cons(OZ_int(frameId),cons(OZ_int(feedtime),dinfo));

    if (currentThread->stepMode() /* || def->getPred()->getSpyFlag() */) {
      ProgramCounter debugPC = CodeArea::nextDebugInfo(PC);
      if (debugPC != NOCODE) {
        debugStreamCall(debugPC, def->getPrintName(), def->getArity(),
                        arguments, 0, frameId);
        dbg = new OzDebug(DBG_STEP,dinfo);
        currentThread->pushDebug(dbg);
        return TRUE;
      }
      else {
        dbg = new OzDebug(DBG_NOOP,dinfo);
        currentThread->pushDebug(dbg);
        return FALSE;
      }
    }
    else {
      dbg = new OzDebug(DBG_NEXT,dinfo);
      currentThread->pushDebug(dbg);
    }
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

void AM::suspendEngine()
{
  deinstallPath(rootBoard);

  ozstat.printIdle(stdout);

  osBlockSignals(OK);

  while (1) {

    if (isSetSFlag(UserAlarm)) {
      handleUser();
    }

    if (isSetSFlag(IOReady) || (compStream && !compStream->bufEmpty())) {
      handleIO();
    }

    if (!threadQueuesAreEmpty()) {
      break;
    }

    if (compStream && isStandalone() && !compStream->cseof()) {
      loadQuery(compStream);
      continue;
    }
    Assert(!compStream || compStream->bufEmpty());

    int ticksleft = osBlockSelect(userCounter);
    setSFlag(IOReady);

    if (userCounter>0 && ticksleft==0) {
      handleUser();
      continue;
    }

    setUserAlarmTimer(ticksleft);
  }

  ozstat.printRunning(stdout);

  // restart alarm
  osSetAlarmTimer(CLOCK_TICK/1000);

  osUnblockSignals();
}


//
//  As said (in thread.hh), lazy allocation of stack can be toggled
// just by moving the code to the '<something>ToRunnable ()';
inline
void Thread::makeRunning ()
{
  Assert (isRunnable ());

  //
  //  Note that this test covers also the case when a runnable thread
  // was suspended in a sequential mode: it had already a stack,
  // so we don't have to do anything now;
  switch (getThrType ()) {

  case S_WAKEUP:
    //
    //  Wakeup;
    //  No regions were pre-allocated, - so just make a new one;
    setHasStack ();
    item.threadBody = am.allocateBody ();

    getBoard()->setNervous ();
    // no break here

  case S_RTHREAD:
    am.cachedStack = getTaskStackRef();
    am.cachedSelf = getSelf();
    setSelf(0);
    break;                      // nothing to do;

  case S_PR_THR:
    warning("Thread::makeRunning hits a propagator");
    //  This case is intentially moved back, since normally it has
    // to be catched already in the emulator;
    break;

  default:
    Assert(0);
  }
}

/* ********************************************************************** */

/* macros are faster ! */
#define emulateHookCall(e,def,arguments,Code)           \
    if (e->hookCheckNeeded()) {                         \
      if (e->emulateHookOutline(PC, def, arguments)) {  \
        Code;                                           \
        goto LBLpreemption;                             \
      }                                                 \
    }

#define emulateHookPopTask(e) emulateHookCall(e,0,0,)


#define ChangeSelf(obj)                         \
      e->changeSelf(obj);

#define SaveSelf                                \
      e->saveSelf();

#define PushCont(PC,Y,G)  CTS->pushCont(PC,Y,G,0);

/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred,gRegs)                                            \
     Y = NULL;                                                              \
     G = gRegs;                                                             \
     emulateHookCall(e,Pred,X,                                              \
                     e->pushContX(Pred->getPC(),NULL,G,X,Pred->getArity())); \

// load a continuation into the machine registers PC,Y,G,X
#define LOADCONT(cont)                          \
  {                                             \
      Continuation *tmpCont = cont;             \
      PC = tmpCont->getPC();                    \
      Y = tmpCont->getY();                      \
      G = tmpCont->getG();                      \
      predArity = tmpCont->getXSize();          \
      tmpCont->getX(X);                         \
  }

// -----------------------------------------------------------------------
// THREADED CODE

#if defined(RECINSTRFETCH) && defined(THREADED)
 Error: RECINSTRFETCH requires THREADED == 0;
#endif

#define INCFPC(N) PC += N

#define WANT_INSTRPROFILE
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
#define DISPATCH(INC) {                                                       \
  intlong aux = *(PC+INC);                                                    \
  INCFPC(INC);                                                                \
  goto* (void*) (aux|textBase);                                               \
}

#else /* THREADED */

#define Case(INSTR)   case INSTR :
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#endif

#define JUMP(absAdr) Assert(absAdr!=0 && absAdr!=NOCODE); PC=absAdr; DISPATCH(0)

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
#define GetBI(PC) ((BuiltinTabEntry*) getAdressArg(PC))


/* define REGOPT if you want the into register optimization for GCC */
#if defined(REGOPT) &&__GNUC__ >= 2 && (defined(GNUWIN32) || defined(LINUX_I486) || defined(MIPS) || defined(OSF1_ALPHA) || defined(SPARC)) && !defined(DEBUG_CHECK)
#define Into(Reg) asm(#Reg)

#if defined(LINUX_I486) || defined(GNUWIN32)
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


#define SUSP_PC(TermPtr,RegsToSave,PC)          \
   e->pushContX(PC,Y,G,X,RegsToSave);           \
   addSusp(TermPtr,CTT);        \
   goto LBLsuspendThread;


void addSusp(TaggedRef *varPtr, Thread *thr)
{
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
  TaggedRef ret = oz_newVariable();
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

#ifdef DEBUG_NONMONOTONIC
  cout << "AM::checkEntailment" << endl << flush;
#endif

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
      currentBoard->unsetInstalled();
      setCurrent (currentBoard->getParent());

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
  ConstTerm *predicate;     NoReg(predicate);
  int predArity;            NoReg(predArity);

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

    if (e->isSetSFlag(StartGC)) {
      e->deinstallPath(e->rootBoard);
      e->doGC();
    }

    if (e->isSetSFlag(StopThread)) {
      e->unsetSFlag(StopThread);
    }

    if (e->isSetSFlag(UserAlarm)) {
      e->deinstallPath(e->rootBoard);
      osBlockSignals();
      e->handleUser();
      osUnblockSignals();
    }
    if (e->isSetSFlag(IOReady)) {
      e->deinstallPath(e->rootBoard);
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
        CTS = CTT->getTaskStackRef();
        e->restartThread();
        HF_APPLY(OZ_atom(builtinTab.getName((void *)(prop->getHeaderFunc()))),
                 prop->getParameters());
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
    CTT->unmarkRunnable();

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

    if (e->debugmode() && CTT->isTraced()) {
      Frame *auxtos = CTT->getTaskStackRef()->getTop();
      GetFrame(auxtos,debugPC,Y,G);

      ProgramCounter debuginfoPC;
      RefsArray _Y,_G;
      do {
        GetFrameNoDecl(auxtos,debuginfoPC,_Y,_G);
      } while (debuginfoPC == C_DEBUG_CONT_Ptr);

      TaggedRef name = OZ_atom("Unknown");
      TaggedRef args = nil();

      // the following code is basically the same as in
      // TaskStack::dbgGetTaskStack (print.cc)
      // For now, only suspending on builtins gives correct name & args

      if (debugPC==C_CFUNC_CONT_Ptr) {
        OZ_CFun biFun    = (OZ_CFun) (void*) Y;
        RefsArray X      = (RefsArray) G;

        if (X)
          for (int i=getRefsArraySize(X)-1; i>=0; i--)
            args = cons(X[i],args);
        else
          args = nil();

        name = OZ_atom(builtinTab.getName((void *) biFun));
        debugStreamSuspend(debuginfoPC,CTT,name,args,1);
      }
      else {
        debugStreamSuspend(debuginfoPC,CTT,name,args,0);
      }
    }

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
      PushCont(PC+3,Y,G);   // PC+3 goes into a temp var (mm2)
      // goto LBLFastTailCall; // is not optimized away (mm2)
    }


  Case(FASTTAILCALL)
    //  LBLFastTailCall:
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);

      COUNT(fastcalls);
      ProfileCode(entry->getAbstr()->getPred()->numCalled++);

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
      COUNT(bicalls);
      BuiltinTabEntry* entry = GetBI(PC+1);
      biFun = entry->getFun();
      predArity = getPosIntArg(PC+2);

      CheckArity(entry->getArity(),makeTaggedConst(entry));

      switch (biFun(predArity, X)){

      case PROCEED:       DISPATCH(3);
      case FAILED:        HF_BI;
      case RAISE:         goto LBLraise;
      case BI_TYPE_ERROR: RAISE_TYPE;

      case SUSPEND:
        e->pushContX(PC,Y,G,X,predArity);
        e->suspendOnVarList(CTT);
        goto LBLsuspendThread;

      case BI_PREEMPT:
        PushCont(PC+3,Y,G);
        goto LBLpreemption;

      case BI_REPLACEBICALL:
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

      OZ_Return res = rel(XPC(2));
      if (res==PROCEED) { DISPATCH(4); }

      switch(res) {
      case SUSPEND:
        if (shallowCP) {
          e->trail.pushIfVar(XPC(2));
          goto LBLsuspendShallow;
        }
        e->pushContX(PC,Y,G,X,getPosIntArg(PC+3));
        e->suspendInline(1,XPC(2));
        goto LBLsuspendThread;
      case FAILED:
        SHALLOWFAIL;
        HF_APPLY(OZ_atom(GetBI(PC+1)->getPrintName()),
                 cons(XPC(2),nil()));

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

      OZ_Return res = rel(XPC(2),XPC(3));
      if (res==PROCEED) { DISPATCH(5); }

      switch(res) {

      case SUSPEND:
        {
          if (shallowCP) {
            e->trail.pushIfVar(XPC(2));
            e->trail.pushIfVar(XPC(3));
            goto LBLsuspendShallow;
          }

          e->pushContX(PC,Y,G,X,getPosIntArg(PC+4));
          e->suspendInline(2,XPC(2),XPC(3));
          goto LBLsuspendThread;
        }
      case FAILED:
        SHALLOWFAIL;
        HF_APPLY(OZ_atom(GetBI(PC+1)->getPrintName()),
                 cons(XPC(2),cons(XPC(3),nil())));

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

      OZ_Return res = rel(XPC(2),XPC(3),XPC(4));
      if (res==PROCEED) { DISPATCH(6); }

      switch(res) {
      case SUSPEND:
        {
          if (shallowCP) {
            e->trail.pushIfVar(XPC(2));
            e->trail.pushIfVar(XPC(3));
            e->trail.pushIfVar(XPC(4));
            goto LBLsuspendShallow;
          }

          e->pushContX(PC,Y,G,X,getPosIntArg(PC+5));
          e->suspendInline(3,XPC(2),XPC(3),XPC(4));
          goto LBLsuspendThread;
        }
      case FAILED:
        SHALLOWFAIL;
        HF_APPLY(OZ_atom(GetBI(PC+1)->getPrintName()),
                 cons(XPC(2),cons(XPC(3),cons(XPC(4),nil()))));

      case RAISE:
        goto LBLraise;

      case BI_TYPE_ERROR:
        RAISE_TYPE1(GetBI(PC+1)->getPrintName(),
                    cons(XPC(2),cons(XPC(3),cons(XPC(4),nil()))));


      case SLEEP:
      default:
        Assert(0);
      }
    }

  Case(INLINEFUN1)
    {
      COUNT(inlinecalls);
      InlineFun1 fun = (InlineFun1)GetBI(PC+1)->getInlineFun();

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
          e->pushContX(PC,Y,G,X,getPosIntArg(PC+4));
          e->suspendInline(1,A);
          goto LBLsuspendThread;
        }

      case RAISE:            goto LBLraise;
      case BI_REPLACEBICALL:
        predArity = getPosIntArg(PC+4);
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
          e->pushContX(PC,Y,G,X,getPosIntArg(PC+5));
          e->suspendInline(2,A,B);
          goto LBLsuspendThread;
        }

      case RAISE:
        goto LBLraise;

      case BI_REPLACEBICALL:
        predArity = getPosIntArg(PC+5);
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
          (void) e->raise(E_ERROR,E_KERNEL,".", 2, XPC(1), feature);
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
            e->pushContX(PC,Y,G,X,getPosIntArg(PC+4));
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
      Object *self = e->getSelf();

      Assert(e->getSelf()!=NULL);
      RecOrCell state = self->getState();
      SRecord *rec;
      if (stateIsCell(state)) {
        rec = getState(state,NO,fea,XPC(2));
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
        XPC(2) = rec->getArg(index);
        DISPATCH(6);
      }
      (void) e->raise(E_ERROR,E_OBJECT,"@",2,makeTaggedSRecord(rec),fea);
      goto LBLraise;
    }

  Case(INLINEASSIGN)
    {
      TaggedRef fea = getLiteralArg(PC+1);

      Object *self = e->getSelf();

      if (!e->isToplevel() && e->currentBoard != self->getBoard()) {
        (void) e->raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("object"));
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

      (void) e->raise(E_ERROR,E_OBJECT,"<-",3, makeTaggedSRecord(rec), fea, XPC(2));
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
          e->pushContX(PC,Y,G,X,getPosIntArg(PC+4));
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
      COUNT(inlinecalls);
      InlineFun3 fun = (InlineFun3)GetBI(PC+1)->getInlineFun();

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
          e->pushContX(PC,Y,G,X,getPosIntArg(PC+6));
          e->suspendInline(3,A,B,C);
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

      // note XPC(4) is maybe the same as XPC(2) or XPC(3) !!
      OZ_Return res = fun(XPC(2),XPC(3),XPC(4));
      if (res==PROCEED) { DISPATCH(6); }

      Assert(res==SUSPEND);
      e->pushContX(PC,Y,G,X,getPosIntArg(PC+5));
      e->suspendOnVarList(CTT);
      goto LBLsuspendThread;
    }

#undef SHALLOWFAIL

// ------------------------------------------------------------------------
// CLASS: Shallow guards stuff
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
      InlineRel1 rel = (InlineRel1)GetBI(PC+1)->getInlineFun();

      OZ_Return res = rel(XPC(2));
      if (res==PROCEED) { DISPATCH(5); }
      if (res==FAILED)  { JUMP(getLabelArg(PC+3)); }

      switch(res) {

      case SUSPEND:
        e->pushContX(PC,Y,G,X,getPosIntArg(PC+4));
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
      InlineRel2 rel = (InlineRel2)GetBI(PC+1)->getInlineFun();

      OZ_Return res = rel(XPC(2),XPC(3));
      if (res==PROCEED) { DISPATCH(6); }
      if (res==FAILED)  { JUMP(getLabelArg(PC+4)); }

      switch(res) {

      case SUSPEND:
        {
          e->pushContX(PC,Y,G,X,getPosIntArg(PC+5));
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
        RAISE_TYPE1(GetBI(PC+1)->getPrintName(),
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
        e->shallowHeapTop = NULL;
        e->trail.popMark();
        DISPATCH(1);
      }

    LBLsuspendShallow:
      {
        e->emptySuspendVarList();
        int argsToSave = getPosIntArg(shallowCP+2);
        e->pushContX(shallowCP,Y,G,X,argsToSave);
        shallowCP = NULL;
        e->shallowHeapTop = NULL;
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

  Case(DEALLOCATEL1)    { deallocateY(Y,1);  Y=0; DISPATCH(1); }
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
// CLASS: CONTROL: FAIL/SUCCESS/RETURN/SAVECONT
// -------------------------------------------------------------------------

  Case(FAILURE)
    {
      HF_FAIL;
    }


  Case(SUCCEED)
    DISPATCH(1);

  Case(SAVECONT)
    /*     PushCont(getLabelArg(PC+1),Y,G); */
    error("unused");
    DISPATCH(2);

  Case(EXHANDLER)
    PushCont(PC+2,Y,G);
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
      TaggedRef aux      = XPC(2);
      int toSave         = getPosIntArg(PC+3);

      DEREF(aux,auxPtr,_1);
      if (isAnyVar(aux)) {
        SUSP_PC(auxPtr,toSave,PC);
      }

      if (!isLock(aux)) {
        /* arghhhhhhhhhh! fucking exceptions (RS) */
        (void) oz_raise(E_ERROR,E_KERNEL,
                  "type",5,NameUnit,NameUnit,
                  OZ_atom("Lock"),
                  OZ_int(1),
                  OZ_string(""));
        RAISE_TYPE1("lock",cons(aux,nil()));
        goto LBLraise;
      }

      OzLock *t = (OzLock*)tagged2Tert(aux);
      Thread *th=e->currentThread;


      switch(t->getTertType()){
      case Te_Local:{
        if(!e->isToplevel()){
          if (e->currentBoard != ((LockLocal*)t)->getBoard()) {
            (void) e->raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("lock"));
            goto LBLraise;}}
        if(((LockLocal*)t)->hasLock(th)) {goto has_lock;}
        if(((LockLocal*)t)->lockB(th)) {goto got_lock;}
        goto no_lock;}
      case Te_Frame:{
        if(((LockFrame*)t)->hasLock(th)) {goto has_lock;}
        if(((LockFrame*)t)->lockB(e->currentThread)){goto got_lock;}
        goto no_lock;}
      case Te_Proxy:{
        ((LockProxy*)t)->lock(th);
        goto no_lock;}
      case Te_Manager:{
        if(((LockManager*)t)->hasLock(th)) {goto has_lock;}
        if(((LockManager*)t)->lockB(th)){goto got_lock;}
        goto no_lock;}}

      Assert(0);

      got_lock:
         PushCont(lbl,Y,G);
         CTS->pushLock(t);
         DISPATCH(4);

      has_lock:
         PushCont(lbl,Y,G);
         DISPATCH(4);

      no_lock:
        PushCont(lbl,Y,G);
        CTS->pushLock(t);
        e->pushContX((PC+4),Y,G,X,toSave);      /* ATTENTION */
        goto LBLsuspendThread;

    }

  Case(RETURN)
    LBLpopTask:
      {
        Assert(!CTT->isSuspended());
        Assert(CBB==currentDebugBoard);

        emulateHookPopTask(e);

        DebugCheckT(CAA = NULL);

      LBLpopTaskNoPreempt:
        Assert(CTS==CTT->getTaskStackRef());
        PopFrameNoDecl(CTS,PC,Y,G);
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

      ProfileCode(predd->numClosures++);

      if (predd->getPC()==NOCODE) {
        predd->PC = PC+sizeOf(DEFINITION);
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
    e->pushContX(PC,Y,G,X,argsToSave);
    if (isCVar (tag)) {
      tagged2CVar(term)->addDetSusp(CTT,termPtr);
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
      Object *obj      = tagged2Object(object);
      Abstraction *def = getSendMethod(obj,label,arity,(InlineCache*)(PC+4),X);
      if (def == NULL) {
        goto bombSend;
      }

      if (!isTailCall) PushCont(PC+6,Y,G);
      ChangeSelf(obj);
      CallDoChecks(def,def->getGRegs());
      COUNT(sendmsg);
      ProfileCode(def->getPred()->numCalled++);
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
    def = getApplyMethod(tagged2Object(object),ami,arity,X);
    if (def==NULL) {
      goto bombApply;
    }

    if (!isTailCall) { PushCont(PC,Y,G); }
    COUNT(applmeth);
    ProfileCode(def->getPred()->numCalled++);
    CallDoChecks(def,def->getGRegs());
    JUMP(def->getPC());


  bombApply:
    Assert(e->methApplHdl != makeTaggedNULL());

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

       DEREF(taggedPredicate,predPtr,_1);

       if (isAbstraction(taggedPredicate)) {
         Abstraction *def = tagged2Abstraction(taggedPredicate);
         PrTabEntry *pte = def->getPred();
         CheckArity(pte->getArity(), taggedPredicate);
         if (!isTailCall) { PushCont(PC+3,Y,G); }
         CallDoChecks(def,def->getGRegs());
         JUMP(pte->getPC());
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
           Assert(e->sendHdl != makeTaggedNULL());
           def = tagged2Abstraction(e->sendHdl);
           /* {Obj Msg} --> {Send Msg Class Obj} */
           X[predArity++] = makeTaggedConst(o->getClass());
           X[predArity++] = makeTaggedConst(o);
         } else {
           def = (Abstraction *) predicate;
         }
         CheckArity(def->getArity(), makeTaggedConst(def));
         if (!isTailCall) { PushCont(PC,Y,G); }

         ProfileCode(def->getPred()->numCalled++);
         CallDoChecks(def,def->getGRegs());
         JUMP(def->getPC());
       }

// -----------------------------------------------------------------------
// --- Call: Builtin
// -----------------------------------------------------------------------
       Assert(typ==Co_Builtin);
       COUNT(nonoptbicalls);

       bi = (BuiltinTabEntry *) predicate;

       CheckArity(bi->getArity(),makeTaggedConst(bi));

       if (e->debugmode() && e->currentThread->stepMode()) {
         char *name = builtinTab.getName((void *) bi->getFun());
         ProgramCounter debugPC = CodeArea::nextDebugInfo(PC);

         if (debugPC != NOCODE) {
           debugStreamCall(debugPC, name, predArity, X, 1, 0);

           if (!isTailCall) PushCont(PC,Y,G);

           time_t feedtime = CodeArea::findTimeStamp(PC);
           OZ_Term dinfo = cons(OZ_int(0),cons(OZ_int(feedtime),nil()));
           OzDebug *dbg  = new OzDebug(DBG_STEP,dinfo);
           CTT->pushDebug(dbg);
           CTT->pushCFun(bi->getFun(),X,predArity,OK);
           goto LBLpreemption;
         }
       }

       biFun=bi->getFun();
       OZ_Return res = biFun(predArity, X);

       //if (e->isSetSFlag(DebugMode)) {
       //exitBuiltin(res,makeTaggedConst(bi),predArity,X);
       //}

       switch (res) {

       case SUSPEND:
         {
           if (!isTailCall) PushCont(PC,Y,G);

           if (e->debugmode()) {
             time_t feedtime = CodeArea::findTimeStamp(PC);
             OZ_Term dinfo = cons(OZ_int(0),cons(OZ_int(feedtime),nil()));
             OzDebug *dbg  = new OzDebug(DBG_NEXT,dinfo);
             e->currentThread->pushDebug(dbg);
           }

           CTT->pushCFun(biFun,X,predArity,OK);
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

         e->pushContX(PC,Y,G,X,predArity);
         CTS->pushFrame(auxPC,auxY,auxG);
       }
       if (e->suspendVarList) {
         e->suspendOnVarList(CTT);
         goto LBLsuspendThread;
       }
       goto LBLpopTask;
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
         foundHdl=CTT->getTaskStackRef()->findCatch(&traceBack,e->debugmode());
         if (PC != NOCODE) traceBack = cons(CodeArea::dbgGetDef(PC),traceBack);

         OZ_Term loc = e->dbgGetLoc(CBB);
         e->formatError(traceBack,loc);

       } else {
         foundHdl = CTT->getTaskStackRef()->findCatch();
       }

       if (foundHdl) {
         X[0] = e->exception.value;
         goto LBLpopTaskNoPreempt;
       }

       if (!e->isToplevel() &&
           OZ_eq(OZ_label(e->exception.value),OZ_atom("failure"))) {
         goto LBLfailure;
       }

       if (e->debugmode()) {
         OZ_Term exc = e->exception.value;
         if (OZ_isRecord(exc) &&
             OZ_eq(OZ_label(exc),OZ_atom("system")) &&
             tagged2SRecord(exc)->getFeature(OZ_int(1)) != makeTaggedNULL() &&
             OZ_eq(OZ_label(OZ_subtree(exc,OZ_int(1))),OZ_atom("kernel")) &&
             OZ_eq(OZ_subtree(OZ_subtree(exc,OZ_int(1)),OZ_int(1)),
                   OZ_atom("terminate")))
           ;
         else {
           execBreakpoint(CTT,NO);
           debugStreamRaise(CTT,e->exception.value);
           goto LBLpreemption;
         }
       }
       // else
       RefsArray argsArray = allocateRefsArray(1,NO);
       argsArray[0] = e->exception.value;
       CTT->pushCall(e->defaultExceptionHandler,argsArray,1);
       goto LBLpopTask; // changed from LBLpopTaskNoPreempt; -BL 26.3.97
     }
   }

// --------------------------------------------------------------------------
// --- end call/execute -----------------------------------------------------
// --------------------------------------------------------------------------


// -------------------------------------------------------------------------
// CLASS: Actors/Deep Guards
// -------------------------------------------------------------------------

  Case(WAIT)
    LBLwait:
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

      CAA = new AskActor(CBB,CTT,
                         elsePC ? elsePC : NOCODE,
                         NOCODE, Y, G, X, argsToSave);
      CTS->pushActor(CAA);
      DISPATCH(3);
    }

  Case(CREATEOR)
    {
      CAA = new WaitActor(CBB, CTT, NOCODE, Y, G, X, 0, NO);
      CTS->pushActor(CAA);

      DISPATCH(1);
    }

  Case(CREATEENUMOR)
    {
      Board *bb = CBB;

      CAA = new WaitActor(bb, CTT, NOCODE, Y, G, X, 0, NO);
      CTS->pushActor(CAA);

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
      CTS->pushActor(CAA);

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
      Board *bb = new Board(CAA,CAA->isAsk()?Bo_Ask:Bo_Wait);
      e->setCurrent(bb,OK);
      CBB->incSuspCount();
      DebugCode(currentDebugBoard=CBB);
      CBB->setInstalled();
      e->trail.pushMark();
      Assert(CAA->getThread()==CTT);
      DISPATCH(1);
    }

  Case(EMPTYCLAUSE)
    {
      Assert(CAA->isWait());
      Board *bb = new Board(CAA, Bo_Wait | Bo_Waiting);

      bb->setBody(PC+1, Y, G, NULL,0);

      // optimization for most usual case
      if (CAA->hasNext()) {
        LOADCONT(CAA->getNext());
        goto LBLemulate; // no thread switch allowed here (CAA)
      }

      goto LBLpopTask;
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

      Thread *tt = e->mkRunnableThread(prio, CBB);

      COUNT(numThreads);
      ozstat.createdThreads.incf();
      RefsArray newY = Y==NULL ? (RefsArray) NULL : copyRefsArray(Y);

      if (e->debugmode()) {
        Frame *aux = CTT->getTaskStackRef()->getTop();
        GetFrame(aux,ozdebugPC,Y,G);

        if (ozdebugPC==C_DEBUG_CONT_Ptr) {
          // inherit ozdebug from parent thread
          OzDebug *dbg = (OzDebug*) Y;
          tt->pushDebug(dbg->copy());
        }
      }

      tt->pushCont(newPC,newY,G,NULL);
      tt->setSelf(e->getSelf());

      e->scheduleThread (tt);

      JUMP(contPC);
    }

// -------------------------------------------------------------------------
// CLASS: MISC: ERROR/NOOP/default
// -------------------------------------------------------------------------

  Case(TASKEMPTYSTACK)
    {
      Assert(Y==0 && G==0);
      CTS->pushEmpty();
      if (e->isToplevel ()) {
        goto LBLkillToplevelThread;
      } else {
        goto LBLkillThread;
      }
    }


  Case(TASKCALLCONT)
    {
      TaggedRef taggedPredicate = (TaggedRef)ToInt32(Y);

      predArity = G ? getRefsArraySize(G) : 0;

      DEREF(taggedPredicate,predPtr,predTag);
      if (!isProcedure(taggedPredicate) && !isObject(taggedPredicate)) {
        if (isAnyVar(predTag)) {
          SUSP_PC(predPtr,0,PC);
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
        ((LockFrame*)lck)->unlock();
        break;
      case Te_Proxy:
        e->raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("lock"));
        goto LBLraise;
      case Te_Manager:
        ((LockManager*)lck)->unlock();
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
      e->setSelf((Object*)Y);
      Y = NULL;
      goto LBLpopTaskNoPreempt;


  Case(TASKDEBUGCONT)
    {
      OzDebug     *ozdeb = (OzDebug *) Y;
      OzDebugDoit dothis = ozdeb->dothis;
      TaggedRef   info   = ozdeb->info;

      Y = NULL;
      ozdeb->dispose();

      switch (dothis) {
      case DBG_NOOP : {
        break;
      }
      case DBG_NEXT : {
        if (CTT->isTraced() && CTT->stepMode()) {
          debugStreamExit(info);
          goto LBLpreemption;
        }
        break;
      }
      case DBG_STEP : {
        if (CTT->isTraced() && !CTT->contFlag()) {
          debugStreamExit(info);
          goto LBLpreemption;
        }
        break;
      }
      default:
        ;
      }
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

       case BI_REPLACEBICALL:
         PC = NOCODE;
         goto LBLreplaceBICall;

       case SUSPEND:
         CTT->pushCFun(biFun,X,predArity,OK);
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

#ifdef DEBUG_LTQ
       cout << "sa=" << sa << " emu " << " thr="
            << e->currentThread << endl << flush;
#endif

       Assert(!ltq->isEmpty());

#ifdef PROP_TIME
       unsigned int starttime = osUserTime();
#endif

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
#ifdef PROP_TIME
           ozstat.timeForPropagation.incf(osUserTime()-starttime);
#endif
           CTS->pushLTQ(sa); // RS: is this needed ???
           // failure of propagator is never catched !
           goto LBLfailure; // top-level failure not possible
         } else {
           Assert(r == SCHEDULED);
           thr->scheduledPropagator();
         }
       }

       CTT = backup_currentThread;
#ifdef PROP_TIME
       ozstat.timeForPropagation.incf(osUserTime()-starttime);
#endif

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


  Case(TASKACTOR)
    {
      AWActor *aw = (AWActor *) Y;
      Y = NULL;

      Assert(!aw->isCommitted());

      if (aw->hasNext()) {
        LOADCONT(aw->getNext());
        CAA=aw;
        CTS->pushActor(aw);
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
        CTS->pushActor(aw);
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

      CTS->pushActor(aw);
      goto LBLsuspendThread;
    }

  Case(OZERROR)
      error("Emulate: OZERROR command executed");
      goto LBLerror;


  Case(DEBUGINFO)
    {
      int line = smallIntValue(getNumberArg(PC+2));
      if (line<0) {
        execBreakpoint(e->currentThread);
      }
      DISPATCH(6);

      /*
        TaggedRef filename = getLiteralArg(PC+1);
        int absPos         = smallIntValue(getNumberArg(PC+3));
        TaggedRef comment  = getLiteralArg(PC+4);
        int noArgs         = smallIntValue(getNumberArg(PC+5));
       */
    }

  Case(MARSHALLEDFASTCALL)
    {
      TaggedRef pred = getTaggedArg(PC+1);
      int tailcallAndArity  = getPosIntArg(PC+2);

      DEREF(pred,predPtr,_1);
      if (isAnyVar(pred)) {
        SUSP_PC(predPtr,tailcallAndArity>>1,PC);
      }

      Abstraction *abstr = tagged2Abstraction(pred);

      OZ_unprotect((TaggedRef*)(PC+1));
      AbstractionEntry *entry = AbstractionTable::add(abstr);
      CodeArea::writeOpcode((tailcallAndArity&1) ? FASTTAILCALL : FASTCALL, PC);
      CodeArea::writeAddress(entry, PC+1);
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

      if (genCallInfo(gci,pred,PC)) {
        gci->dispose();
        DISPATCH(0);
      }

      isTailCall = gci->isTailCall;
      if (!isTailCall) PC = PC+3;

      /* the following adapted from bombApply */
      Assert(e->methApplHdl != makeTaggedNULL());

      X[1] = makeMessage(gci->arity,gci->mn,X);
      X[0] = pred;

      predArity = 2;
      predicate = tagged2Const(e->methApplHdl);
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
    shallowCP         = NULL;
    e->shallowHeapTop = NULL;
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

    if (!e->fastUnifyOutline(saa->getResult(),saa->genFailed(),0)) {
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
