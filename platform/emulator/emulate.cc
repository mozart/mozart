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

#ifdef __GNUC__
#pragma implementation "emulate.hh"
#endif

#include "am.hh"

#include "indexing.hh"

#include "genvar.hh"
#include "fdhook.hh"


// -----------------------------------------------------------------------
// TOPLEVEL FAILURE (HF = Handle Failure)



#define HF_BODY(MSG_SHORT,MSG_LONG)                                           \
  if (allowTopLevelFailureMsg) {                                              \
    if (ozconf.errorVerbosity > 0) {                                          \
      toplevelErrorHeader();                                                  \
      {MSG_SHORT;}                                                            \
      if (ozconf.errorVerbosity > 1) {                                        \
        message("\n");                                                        \
        {MSG_LONG;}                                                           \
        e->currentThread->printDebug(PC);                                     \
      }                                                                       \
      errorTrailer();                                                         \
    } else {                                                                  \
      message("Toplevel Failure\n");                                          \
    }                                                                         \
  } else {                                                                    \
    allowTopLevelFailureMsg = TRUE;                                           \
  }                                                                           \
  DebugCheck(ozconf.stopOnToplevelFailure, tracerOn();trace("toplevel failed"));



/* called if t1=t2 fails */
void failureUnify(AM *e, char *msgShort, TaggedRef arg1, TaggedRef arg2,
                  char *msgLong, ProgramCounter PC)
{
  HF_BODY(message(msgShort, OZ_toC(arg1),
                  (arg2 == makeTaggedNULL()) ? "" : OZ_toC(arg2)),
          message(msgLong));
}


#define HF_UNIFY(MSG_SHORT,T1,T2,MSG_LONG)                                    \
   if (!e->isToplevel()) { goto LBLfailure; }                                 \
   failureUnify(e,MSG_SHORT,T1,T2,MSG_LONG,PC);                               \
   goto LBLkillThread;



#define HF_FAIL(MSG_SHORT,MSG_LONG)                                           \
   if (!e->isToplevel()) { goto LBLfailure; }                                 \
   HF_BODY(MSG_SHORT,MSG_LONG);                                               \
   goto LBLkillThread;


void failureNomsg(AM *e, ProgramCounter PC) { HF_BODY(,); }

#define HF_NOMSG                                                              \
   if (!e->isToplevel()) { goto LBLfailure; }                                 \
   failureNomsg(e,PC);                                                        \
   goto LBLkillThread;



// always issue the message
#define HF_WARN(MSG_SHORT,MSG_LONG)                                           \
  if (ozconf.errorVerbosity > 0) {                                            \
    warningHeader();                                                          \
    { MSG_SHORT; }                                                            \
    if (ozconf.errorVerbosity > 1) {                                          \
       message("\n");                                                         \
       { MSG_LONG; }                                                          \
    }                                                                         \
    errorTrailer();                                                           \
  }                                                                           \
  HF_NOMSG;



#define NOFLATGUARD()   (shallowCP==NULL)

#define SHALLOWFAIL if (!NOFLATGUARD()) { goto LBLshallowFail; }


#define CheckArity(arity,arityExp,pred,cont)                                  \
if (arity != arityExp && VarArity != arityExp) {                              \
  HF_WARN(applFailure(pred);                                                  \
          message("Wrong number of arguments: expected %d got %d\n",arityExp,arity),); \
}

// TOPLEVEL END
// -----------------------------------------------------------------------


#define DoSwitchOnTerm(indexTerm,table)                                       \
      TaggedRef term = indexTerm;                                             \
      DEREF(term,_1,_2);                                                      \
                                                                              \
      if (!isLTuple(term)) {                                                  \
        TaggedRef *sp = sPointer;                                             \
        ProgramCounter offset = switchOnTermOutline(term,table,sp);           \
        sPointer = sp;                                                        \
        JUMP(offset);                                                         \
      }                                                                       \
                                                                              \
      ProgramCounter offset = table->listLabel;                               \
      sPointer = tagged2LTuple(term)->getRef();                               \
      JUMP(offset);                                                           \



static
ProgramCounter switchOnTermOutline(TaggedRef term, IHashTable *table,
                                   TaggedRef *&sP)
{
  ProgramCounter offset = table->getElse();
  if (isSTuple(term)) {
    if (table->functorTable) {
      Literal *lname = tagged2STuple(term)->getLabelLiteral();
      int hsh = lname ? table->hash(lname->hash()) : 0;
      offset = table->functorTable[hsh]
            ->lookup(lname,tagged2STuple(term)->getSize(),offset);
      sP = tagged2STuple(term)->getRef();
    }
    return offset;
  }

  if (isLiteral(term)) {
    if (table->literalTable) {
      int hsh = table->hash(tagged2Literal(term)->hash());
      offset = table->literalTable[hsh]->lookup(tagged2Literal(term),offset);
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

Bool Board::isFailureInBody ()
{
  Assert(isWaiting());
  if (isWaitTop()) {
    return (NO);
  }
  Opcode op = CodeArea::adressToOpcode(CodeArea::getOP(body.getPC()));
  return (op == FAILURE);
}

// -----------------------------------------------------------------------
// CALL HOOK


#ifdef OUTLINE
#define inline
#endif

/* the hook functions return:
     TRUE: must reschedule
     FALSE: can continue
   */

Bool AM::emulateHookOutline(Abstraction *def,
                            int arity,
                            TaggedRef *arguments)
{
  // without signal blocking;
  if (isSetSFlag(ThreadSwitch)) {
    if (threadQueueIsEmpty()
        || getNextThPri () < currentThread->getPriority()) {
      restartThread();
    } else {
      return TRUE;
    }
  }
  if (isSetSFlag((StatusBit)(StartGC|UserAlarm|IOReady))) {
    return TRUE;
  }

  if (def && isSetSFlag(DebugMode)) {
    enterCall(currentBoard,def,arity,arguments);
  }

  return FALSE;
}

inline
Bool AM::hookCheckNeeded()
{
#ifdef DEBUG_DET
  static int counter = 100;
  if (--counter == 0) {
    handleAlarm();   // simulate an alarm
    counter = 100;
  }
#endif

  return (isSetSFlag());
}

/* macros are faster ! */
#define emulateHook(e,def,arity,arguments) \
 (e->hookCheckNeeded() && e->emulateHookOutline(def, arity, arguments))

#define emulateHook0(e) emulateHook(e,NULL,0,NULL)


#define CallPushCont(ContAdr) e->pushTask(ContAdr,Y,G)

/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred,gRegs,Arity,CheckMode)                      \
     G = gRegs;                                                               \
     if (CheckMode) e->currentThread->checkCompMode(Pred->getCompMode()); \
     if (emulateHook(e,Pred,Arity,X)) {                                       \
        e->pushTaskOutline(Pred->getPC(),NULL,G,X,Arity);                     \
        goto LBLschedule;                                                     \
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

#if defined(RECINSTRFETCH) && defined(THREADED)
 Error: RECINSTRFETCH requires THREADED == 0;
#endif

#define INCFPC(N) PC += N

// #define WANT_INSTRPROFILE
#if defined(WANT_INSTRPROFILE)
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

#ifdef XXLINUX
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher
#else

// let gcc fill in the delay slot of the "jmp" instruction:
#define DISPATCH(INC) {                                                       \
  intlong aux = *(PC+INC);                                                    \
  INCFPC(INC);                                                                \
  goto* (void*) (aux|textBase);                                               \
}
#endif /* LINUX */

#else /* THREADED */

#define Case(INSTR)   case INSTR :
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#endif

#define JUMP(absAdr) PC = absAdr; DISPATCH(0)

#define ONREG(Label,R)      HelpReg = (R); goto Label
#define ONREG2(Label,R1,R2) HelpReg = (R1); HelpReg2 = (R2); goto Label


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
#if defined(REGOPT) &&__GNUC__ >= 2 && (defined(LINUX_I486) || defined(MIPS) || defined(OSF1_ALPHA) || defined(SPARC)) && !defined(DEBUG_CHECK)
#define Into(Reg) asm(#Reg)

#ifdef LINUX_I486
/* This does NOT pay off */
/*   #define Reg1 asm("%esi") */
#define Reg1
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

// free the memory of the thread (there are no references to it anymore)
void ThreadsPool::disposeThread(Thread *th)
{
  Verbose((VERB_THREAD,"Thread::dispose = 0x%x\n",this));
  if (th == rootThread) {
    rootThread->init(th->getPriority(),am.rootBoard,PARMODE);
    am.checkToplevel();
  } else {
    /* dispose thread: */
    th->board = (Board *) threadsFreeList;
    threadsFreeList = th;
  }
}

void AM::handleToplevelBlocking()
{
  prefixError();
  message("******************************************\n");
  message("The toplevel thread is blocked.\n");
  message("\n");
  message("(Hint: don't forget to use task ... end)\n");
  message("\n");
  currentThread->printDebug(NOCODE);
  message("******************************************\n");
  rootThread=newThread(currentThread->getPriority(),rootBoard,PARMODE);
  checkToplevel();
  currentThread=0;
}

#define CHECKSEQ \
if (e->currentThread->compMode == ALLSEQMODE) { \
  if (e->currentThread==e->rootThread) {        \
    e->handleToplevelBlocking();                \
  }                                             \
  goto LBLkillThread;                           \
}                                               \
goto LBLpopTask;

inline
void addSusp(TaggedRef var,Suspension *susp)
{
  DEREF(var,varPtr,_1);
  Assert(isAnyVar(var));

  taggedBecomesSuspVar(varPtr)->addSuspension(susp);
}

inline
void addSusp(TaggedRef *varPtr,Suspension *susp)
{
  taggedBecomesSuspVar(varPtr)->addSuspension(susp);
}

/*
 * create the suspension for builtins returning SUSPEND
 *
 * PRE: no reference chains !!
 */
void AM::suspendOnVarList(Suspension *susp)
{
  Assert(suspendVarList!=makeTaggedNULL());

  TaggedRef varList=suspendVarList;
  while (!isRef(varList)) {
    Assert(isCons(varList));

    addSusp(head(varList),susp);
    varList=tail(varList);
  }
  addSusp(varList,susp);
  suspendVarList=makeTaggedNULL();
}

//inline
Suspension *AM::mkSuspension(int prio, ProgramCounter PC,
                             RefsArray Y, RefsArray G,
                             RefsArray X, int argsToSave)
{
  switch (currentThread->getCompMode()) {
  case ALLSEQMODE:
    pushTask(PC,Y,G,X,argsToSave);
    return new Suspension(currentThread);
  case SEQMODE:
    {
      pushTask(PC,Y,G,X,argsToSave);
      Thread *th=newThread(currentThread->getPriority(),currentBoard,SEQMODE);
      th->getSeqFrom(currentThread);
      return new Suspension(th);
    }
  case PARMODE:
    return new Suspension(currentBoard,prio,PC,Y,G,X,argsToSave);
  default:
    Assert(0);
    return 0;
  }
}

//inline
Suspension *AM::mkSuspension(int prio, OZ_CFun bi,
                             RefsArray X, int argsToSave)
{
  switch (currentThread->getCompMode()) {
  case ALLSEQMODE:
    pushCFun(bi,X,argsToSave);
    return new Suspension(currentThread);
  case SEQMODE:
    {
      pushCFun(bi,X,argsToSave);
      Thread *th=newThread(currentThread->getPriority(),currentBoard,SEQMODE);
      th->getSeqFrom(currentThread);
      return new Suspension(th);
    }
  case PARMODE:
    return new Suspension(currentBoard,prio,bi,X,argsToSave);
  default:
    Assert(0);
    return 0;
  }
}

void AM::suspendInline(int prio,OZ_CFun fun,int n,
                       OZ_Term A,OZ_Term B,OZ_Term C,OZ_Term D)
{
  static RefsArray X = allocateStaticRefsArray(4);
  X[0]=A;
  X[1]=B;
  X[2]=C;
  X[3]=D;

  Suspension *susp=mkSuspension(prio,fun,X,n);
  while (--n>=0) {
    DEREF(X[n],ptr,_1);
    if (isAnyVar(X[n])) addSusp(ptr,susp);
  }

  suspendVarList = makeTaggedNULL();   // mm2 please check
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

TaggedRef AM::createNamedVariable(int regIndex, TaggedRef name)
{
  int size = getRefsArraySize(toplevelVars);
  if (LessIndex(size,regIndex)) {
    int newSize = int(size*1.5);
    message("resizing store for toplevel vars from %d to %d\n",size,newSize);
    toplevelVars = resize(toplevelVars,newSize);
    // no deletion of old array --> GC does it
  }
  SVariable *svar = new SVariable(currentBoard);
  TaggedRef ret = makeTaggedRef(newTaggedSVar(svar));
  VariableNamer::addName(ret,name);
  return ret;
}


/*
 * Entailment handling for emulator
 *
 * check entailment and stability
 *  after thread is finished or top commit
 */
enum CE_RET {
  CE_CONT,
  CE_SOLVE_CONT,
  CE_NOTHING,
  CE_FAIL
};

int AM::checkEntailment(Continuation *&contAfter, Actor *&aa)
{

loop:
  DebugTrace(trace("check entailment",currentBoard));

  currentBoard->unsetNervous();

  if (currentBoard->isAsk()) {
    if ( entailment() ) {
      aa = currentBoard->getActor();
      contAfter=currentBoard->getBodyPtr();

      trail.popMark();

      Board *tmpBB = currentBoard;

      setCurrent(currentBoard->getParentFast());
      tmpBB->unsetInstalled();
      tmpBB->setCommitted(currentBoard);

      currentBoard->decSuspCount();

      return CE_CONT;
    }
    return CE_NOTHING;
  }

  if (currentBoard->isWait ()) {
// WAITTTOP
    if (currentBoard->isWaitTop()) {

// WAITTOP: top commit
      if ( entailment() ) {
        trail.popMark();

        Board *tmpBB = currentBoard;

        setCurrent(currentBoard->getParentFast());
        tmpBB->unsetInstalled();
        tmpBB->setCommitted(currentBoard);
        currentBoard->decSuspCount();
        goto loop;
      }

      Assert(!WaitActor::Cast(currentBoard->getActor())->hasOneChild());

      return CE_NOTHING;
    }

    Assert(!currentBoard->isWaiting () ||
           !WaitActor::Cast(currentBoard->getActor())->hasOneChild());
    return CE_NOTHING;

    // WAIT: no rule
  }

  if (currentBoard->isSolve ()) {
    // try to reduce a solve board;
    DebugCheck ((currentBoard->isReflected () == OK),
                error ("trying to reduce an already reflected solve actor"));

    SolveActor *solveAA = SolveActor::Cast (currentBoard->getActor ());
    Board      *solveBB = currentBoard;

    if (isStableSolve(solveAA)) {
      DebugCheck ((trail.isEmptyChunk () == NO),
                  error ("non-empty trail chunk for solve board"));
      // all possible reduction steps require this;

      if (solveBB->hasSuspension () == NO) {
        // 'solved';
        // don't unlink the subtree from the computation tree;
        trail.popMark ();
        currentBoard->unsetInstalled ();
        setCurrent (currentBoard->getParentFast());
        currentBoard->decSuspCount ();

        DebugCheckT (solveBB->setReflected ());
        // statistic
        ozstat.incSolveSolved();
        if ( !fastUnifyOutline(solveAA->getResult(), solveAA->genSolved(), OK) ) {
          return CE_FAIL;
        }
        return CE_NOTHING;
      } else {
        // 'stable' (stuck) or enumeration;

        // wake up propagators to be woken up on stablity
        if (solveAA->stable_wake()) {
          LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
                            return CE_FAIL;);
          if (!isStableSolve(solveAA))
            deinstallCurrent();
            return CE_NOTHING;
          goto loop;
        }

        WaitActor *wa = solveAA->getDisWaitActor ();

        if (wa == (WaitActor *) NULL) {
          // "stuck" (stable without distributing waitActors);
          // don't unlink the subtree from the computation tree;
          trail.popMark ();
          currentBoard->unsetInstalled ();
          setCurrent (currentBoard->getParentFast());
          currentBoard->decSuspCount ();

          DebugCheckT (solveBB->setReflected ());
          if ( !fastUnifyOutline(solveAA->getResult(), solveAA->genStuck(), OK) ) {
            return CE_FAIL;
          }
          return CE_NOTHING;
        } else {
          // to enumerate;
          DebugCheck ((wa->hasOneChild () == OK),
                      error ("wait actor for enumeration with single clause?"));
          DebugCheck (((WaitActor::Cast (wa))->hasNext () == OK),
                      error ("wait actor for distribution has a continuation"));
          DebugCheck ((solveBB->hasSuspension () == NO),
                      error ("solve board by the enumertaion without suspensions?"));

          Bool stableWait = (wa->getChildCount() == 2 &&
                             (wa->getChildRefAt(1))->isFailureInBody() == OK);

          if (stableWait == OK && solveAA->isEatWaits()) {
            Board *waitBoard = wa->getChildRef();

            waitBoard->setCommitted(solveBB);
            if (!installScript(waitBoard->getScriptRef())) {
              return CE_FAIL;
            }

            solveBB->incSuspCount(waitBoard->getSuspCount()-1);

            if (waitBoard->isWaitTop()) {
              goto loop;
            }

            aa=wa;
            contAfter=waitBoard->getBodyPtr();
            return CE_SOLVE_CONT;
          }

          if (solveAA->isGuided()) {

            TaggedRef guide = deref(solveAA->getGuidance());

            if (isCons(guide)) {
              // it is asserted that each element is an int or an pair of
              // ints (see above)
              TaggedRef guideHead = headDeref(guide);

              solveAA->setGuidance(tail(guide));

              int clauseNo, noOfClauses;

              if (isInt(guideHead)) {
                clauseNo = smallIntValue(guideHead) - 1;
                noOfClauses = ((clauseNo >= 0) &&
                               (clauseNo < wa->getChildCount())) ? 1 : 0;
              } else {
                // now we have a pair of integers
                clauseNo = 0;
                noOfClauses =
                  wa->selectChildren(smallIntValue(leftDeref(guideHead))-1,
                                     smallIntValue(rightDeref(guideHead))-1);
              }

              if (noOfClauses == 0) {
                trail.popMark ();
                currentBoard->unsetInstalled ();
                setCurrent (currentBoard->getParentFast());
                currentBoard->decSuspCount ();

                if (!fastUnifyOutline(solveAA->getResult(),
                                      solveAA->genFailed(),
                                      OK)) {
                  return CE_FAIL;
                }
                return CE_NOTHING;
              } else if (noOfClauses == 1) {

                Board *waitBoard = wa->getChildRefAt(clauseNo);

                waitBoard->setCommitted(solveBB);

                if (!installScript(waitBoard->getScriptRef())) {
                  return CE_FAIL;
                }

                solveBB->incSuspCount(waitBoard->getSuspCount()-1);

                if (waitBoard->isWaitTop()) {
                  goto loop;
                }

                aa=wa;
                contAfter=waitBoard->getBodyPtr();
                return CE_SOLVE_CONT;

              } else {
                solveAA->pushWaitActor(wa);
                goto loop;
              }

            } else {
              // put back wait actor
              solveAA->pushWaitActor(wa);
              // give back number of clauses
              trail.popMark ();
              currentBoard->unsetInstalled ();
              setCurrent (currentBoard->getParentFast());
              currentBoard->decSuspCount ();

              DebugCheckT (solveBB->setReflected ());
              if (!fastUnifyOutline(solveAA->getResult(),
                                       solveAA->genChoice(wa->getChildCount()),
                                       OK)) {
                return CE_FAIL;
              }
              return CE_NOTHING;
            }
          } else {

            Board *waitBoard = wa->getChild();
            wa->decChilds();

            if (stableWait) {
              (void) wa->getChild ();  // remove the last child;
              wa->decChilds ();
              DebugCheck((wa->hasNoChilds () == NO),
                         error ("error in the '... [] true then false ro' case"));
              trail.popMark ();
              currentBoard->unsetInstalled ();
              setCurrent (currentBoard->getParentFast());
              currentBoard->decSuspCount ();

              waitBoard->setActor (wa);
              ((AWActor *) wa)->addChild (waitBoard);
              solveAA->setBoardToInstall (waitBoard,wa->getCompMode());
              DebugCheckT (solveBB->setReflected ());
              // statistics
              ozstat.incSolveDistributed();
              if ( !fastUnifyOutline(solveAA->getResult(), solveAA->genEnumedFail() ,OK)) {
                return CE_FAIL;
              }
              return CE_NOTHING;
            } else {
              // 'proper' enumeration;
              trail.popMark ();
              currentBoard->unsetInstalled ();
              setCurrent (currentBoard->getParentFast());
              currentBoard->decSuspCount ();

              WaitActor *nwa = new WaitActor (wa);
              solveBB->decSuspCount ();   // since WaitActor::WaitActor adds one;
              waitBoard->setActor (nwa);
              ((AWActor *) nwa)->addChild (waitBoard);
              wa->unsetBoard ();  // may not copy the actor and rest of boards too;
              solveAA->setBoardToInstall (waitBoard,wa->getCompMode());

              //  Now, the following state has been reached:
              // The waitActor with the 'rest' of actors is unlinked from the
              // computation space; instead, a new waitActor (*nwa) is linked to it,
              // and all the tree from solve blackboard (*solveBB) will be now copied.
              // Moreover, the copy has already the 'boardToInstall' setted properly;
              Board *newSolveBB = copyTree (solveBB, (Bool *) NULL);

              // ... and now set the original waitActor backward;
              waitBoard->setFailed();   // this subtree is discarded;
              wa->setBoard (solveBB);          // original waitActor;
              // the subtrees (new and old ones) are still linked to the
              // computation tree;
              if (wa->hasOneChild () == OK) {
                solveAA->setBoardToInstall (wa->getChild (),wa->getCompMode());
              } else {
                solveAA->setBoardToInstall ((Board *) NULL,0);
                // ... since we have set previously board-to-install for copy;
                solveAA->pushWaitActor (wa);
                //  If this waitActor has yet more than one clause, it can be
                // distributed again ... Moreover, it must be considered first.
              }
              DebugCheckT (solveBB->setReflected ());
              DebugCheckT (newSolveBB->setReflected ());
              // ... and now there are two proper branches of search problem;

              // statistics
              ozstat.incSolveDistributed();
              if ( !fastUnifyOutline(solveAA->getResult(),
                                        solveAA->genEnumed(newSolveBB),
                                        OK)) {
                return CE_FAIL;
              }
              return CE_NOTHING;
            }
          }
        }
      }
    } else if (solveAA->isDebug() && solveAA->getThreads() == 0) {
      // There are some external suspensions to this solver!

      deinstallCurrent();

      TaggedRef newVar = makeTaggedRef(newTaggedUVar(currentBoard));
      TaggedRef result = solveAA->getResult();

      solveAA->setResult(newVar);

      if ( !fastUnifyOutline(result,
                             solveAA->genUnstable(newVar),
                             OK)) {
        return CE_FAIL;
      }
      return CE_NOTHING;
    }
    deinstallCurrent();
    return CE_NOTHING;
  }

  Assert(currentBoard->isRoot());
  return CE_NOTHING;
}

// aux debugging;
#define VERBMSG(S,A1,A2)                                                   \
    fprintf(verbOut,"(em) %s (arg#1 0x%x, arg#2 0x%x) :%d\n",              \
            S,A1,A2,__LINE__);                                             \
    fflush(verbOut);


/* &Var prevent Var to be allocated to a register,
 * increases chances that variables declared as "register"
 * will be really allocated to registers
 */

#define NoReg(Var) { void *p = &Var; }


void engine()
{
// ------------------------------------------------------------------------
// *** Global Variables
// ------------------------------------------------------------------------
  /* ordered by iomportance: first variables will go into machine registers
   * if -DREGOPT is set
   */
  register ProgramCounter PC   Reg1 = 0;
  register RefsArray X         Reg2 = am.xRegs;
  register RefsArray Y         Reg3 = NULL;
  register TaggedRef *sPointer Reg4 = NULL;
  register AM *e               Reg5 = &am;
  register RefsArray G         Reg6 = NULL;

  int XSize = 0; NoReg(XSize);

  Bool isTailCall              = NO;                NoReg(isTailCall);
  Suspension* &currentTaskSusp = FDcurrentTaskSusp; NoReg(currentTaskSusp);
  AWActor *CAA                 = NULL;
  Board *tmpBB                 = NULL;              NoReg(tmpBB);

# define CBB (e->currentBoard)
# define CPP (e->currentThread->getPriority())

  RefsArray HelpReg = NULL, HelpReg2 = NULL;
  OZ_CFun biFun = NULL;     NoReg(biFun);

  /* shallow choice pointer */
  ByteCode *shallowCP = NULL;

  /* which kind of solve combinator to choose */
  Bool isEatWaits    = NO;
  Bool isSolveGuided = NO;
  Bool isSolveDebug  = NO;

  Chunk *predicate; NoReg(predicate);
  int predArity;    NoReg(predArity);

#ifdef CATCH_SEGV
  switch (e->catchError()) {

  case NOEXCEPTION:
    break;

  case SEGVIO:
    HF_FAIL(message("segmentation violation"),);
    break;
   case BUSERROR:
    HF_FAIL(message("bus error"),);
    break;
  }
#endif


#ifdef THREADED
# include "instrtab.hh"
  CodeArea::globalInstrTable = instrTable;
# define op (Opcode) -1
#else
  Opcode op = (Opcode) -1;
#endif

  goto LBLstart;

// ------------------------------------------------------------------------
// *** RUN: Main Loop
// ------------------------------------------------------------------------
 LBLschedule:

  e->currentThread->board=CBB;
  e->scheduleThread(e->currentThread);
  e->currentThread=(Thread *) NULL;

 LBLerror:
 LBLstart:


  if (e->isSetSFlag()) {

    e->deinstallPath(e->rootBoard);

    if (e->isSetSFlag(StartGC)) {
      e->doGC();
    }

    osBlockSignals();

    if (e->isSetSFlag(UserAlarm)) {
      e->handleUser();
    }
    if (e->isSetSFlag(IOReady)) {
      e->handleIO();
    }

    osUnblockSignals();
  }

  /* process switch */
  if (e->threadQueueIsEmpty()) {
    e->suspendEngine();
  }

  e->currentThread = e->getFirstThread();

  DebugTrace(trace("new thread"));

  e->restartThread();

  /*
   * install board
   */
  {
    Board *bb=e->currentThread->getBoardFast();
    if (CBB != bb) {
    LBLinstallLoop:
      switch (e->installPath(bb)) {
      case INST_REJECTED:
        if (!e->currentThread->taskStack.discardLocalTasks()) {
          goto LBLkillDiscardedThread;
        }
        bb=bb->getParentFast();
        goto LBLinstallLoop;
      case INST_FAILED:
        while (bb != CBB) {
          if (!e->currentThread->taskStack.discardLocalTasks()) {
            break;
          }
          bb=bb->getParentFast();
        }
        goto LBLfailure;
      case INST_OK:
        break;
      }
    }
    CBB->unsetNervous();
  }

// ------------------------------------------------------------------------
// *** pop a task
// ------------------------------------------------------------------------
 LBLpopTask:
  {
    asmLbl(popTask);
    DebugCheckT(Board *fsb);
    if (emulateHook0(e)) {
      goto LBLschedule;
    }

    DebugCheckT(CAA = NULL);

    TaskStack *taskstack = &e->currentThread->taskStack;
    TaskStackEntry *topCache = taskstack->getTop();
    TaskStackEntry topElem=TaskStackPop(topCache-1);
    ContFlag cFlag = (ContFlag) (ToInt32(topElem) & 0xf);


    /* RS: Optimize most probable case:
     *  - do not handle C_CONT in switch --> faster
     *  - assume cFlag == C_CONT implies stack does not contain empty mark
     *  - topCache maintained more efficiently
     */
    if (cFlag == C_CONT) {
      Assert(!taskstack->isEmpty(topElem));
      PC = (ProgramCounter) TaskStackPop(topCache-2);
      Y = (RefsArray) TaskStackPop(topCache-3);
      G = (RefsArray) TaskStackPop(topCache-4);
      taskstack->setTop(topCache-4);
      goto LBLemulate;
    }

    if (taskstack->isEmpty(topElem)) {
      goto LBLkillThread;
    }

    topCache--;
    switch (cFlag){
    case C_COMP_MODE:
      taskstack->setTop(topCache);
      e->currentThread->compMode=TaskStack::getCompMode(topElem);
      goto LBLpopTask;
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
        disposeRefsArray(tmpX);
      }
      taskstack->setTop(topCache);
      goto LBLemulate;

    case C_DEBUG_CONT:
      {
        OzDebug *ozdeb = (OzDebug *) TaskStackPop(--topCache);
        taskstack->setTop(topCache);

        exitCall(PROCEED,ozdeb);
        goto LBLpopTask;
      }

    case C_CALL_CONT:
      {
        predicate = (Chunk *) TaskStackPop(--topCache);
        RefsArray tmpX = (RefsArray) TaskStackPop(--topCache);
        predArity = tmpX ? getRefsArraySize(tmpX) : 0;
        int i = predArity;
        while (--i >= 0) {
          X[i] = tmpX[i];
        }
        disposeRefsArray(tmpX);
        taskstack->setTop(topCache);
        DebugTrace(trace("call cont",CBB));
        isTailCall = OK;
        goto LBLcall;
      }
    case C_LOCAL:
      {
        taskstack->setTop(topCache);
        CAA = AWActor::Cast (CBB->getActor());
        e->deinstallCurrent();
        if (CAA->hasNext()) {
          LOADCONT(CAA->getNext());
          goto LBLemulate;
        }
        goto LBLpopTask;
      }

    case C_SOLVE:
      {
        taskstack->setTop(topCache);
        Assert(CBB->isSolve());
        Assert(!CBB->isCommitted() && !CBB->isFailed());
        SolveActor *sa = SolveActor::Cast(CBB->getActor());

        Assert(!CBB->isReflected());

        sa->decThreads ();      // get rid of threads - '1' in creator;
        CBB->decSuspCount();

        Continuation *cont;
        Actor *aa;
        switch (e->checkEntailment(cont,aa)) {
        case CE_FAIL:
          HF_NOMSG;
        case CE_SOLVE_CONT:
          Assert(CBB->isSolve());
          e->pushSolve();
          sa->incThreads();
          CBB->incSuspCount();
          e->createTask();
          LOADCONT(cont);
          goto LBLemulate;
        case CE_CONT:
          {
            Thread *tt = e->createThread(aa->getPriority(),
                                         aa->getCompMode());
            tt->pushCont(cont->getPC(),cont->getY(),cont->getG(),
                         cont->getX(),cont->getXSize(),NO);
            goto LBLpopTask;
          }
        case CE_NOTHING:
          goto LBLpopTask;
        }
      }
    case C_NERVOUS:
      {
        // by kost@ : 'SolveActor::Waker' can produce such task
        // (if the search problem is stable by its execution);
        taskstack->setTop(topCache);

        DebugTrace(trace("nervous",CBB));
        goto LBLpopTask;
      }

    case C_CFUNC_CONT:
      {
        // by kost@ : 'solve actors' are represented via the c-function;
        biFun = (OZ_CFun) TaskStackPop(--topCache);
        currentTaskSusp = (Suspension*) TaskStackPop(--topCache);
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
        /* RS: dont't know, if we can dispose for FDs */
        if (currentTaskSusp == NULL) {
          disposeRefsArray(tmpX);
        }
        taskstack->setTop(topCache);

        if (currentTaskSusp != NULL && currentTaskSusp->isDead()) {
          currentTaskSusp = NULL;
          goto LBLpopTask;
        }

        DebugTrace(trace("cfunc cont",CBB));

        LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()));

        switch (biFun(XSize, X)) {
        case FAILED:
          killPropagatedCurrentTaskSusp();
          LOCAL_PROPAGATION(localPropStore.reset());
        localhack0:
          HF_FAIL(applFailure(biFun), printArgs(X,XSize));
        case PROCEED:
          killPropagatedCurrentTaskSusp();
          LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
                            goto localhack0;);
          goto LBLpopTask;
        case SUSPEND:
          {
            killPropagatedCurrentTaskSusp();
            LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
                              goto localhack0;);
            Suspension *susp = e->mkSuspension(CPP,biFun,X,XSize);
            e->suspendOnVarList(susp);
            CHECKSEQ;
          }
        default:
          Assert(0);
          goto LBLerror;
        } // switch
      }

    default:
      Assert(0);
      goto LBLerror;
    }  // switch
  }

// ----------------- end popTask -----------------------------------------

LBLkillDiscardedThread:
  {
    e->decSolveThreads(e->currentThread->getBoardFast());
    e->disposeThread(e->currentThread);
    e->currentThread=0;
    goto LBLstart;
  }

LBLkillThread:
  {
    DebugTrace(trace("kill thread",CBB));
    Thread *tmpThread = e->currentThread;
    if (tmpThread) {  /* may happen if catching SIGSEGV and SIGBUS */
      e->currentThread=(Thread *) NULL;
      if (!tmpThread->isSuspended()) {
        CBB->decSuspCount();
      }
      Board *nb=0;
      Board *orgNB=0;
      if (e->currentSolveBoard) {
        nb = CBB;
        Assert(!nb->isReflected());
        if (CBB->isSolve()) {
            SolveActor *sa
              = SolveActor::Cast(CBB->getActor());
            sa->decThreads();
            orgNB=nb;
            nb=sa->getBoardFast();
        }
      }
      if (!tmpThread->isSuspended()) {
        e->disposeThread(tmpThread);
      }
      tmpThread=0;
      if (CBB->isRoot()) {
        goto LBLstart;
      }

      Continuation *cont;
      Actor *aa;
      switch (e->checkEntailment(cont,aa)) {
      case CE_FAIL:
        if (nb) e->decSolveThreads(nb);
        HF_NOMSG;
      case CE_SOLVE_CONT: /* no special case */
      case CE_CONT:
        e->currentThread = e->newThread(aa->getPriority(),CBB,
                                        aa->getCompMode());
        LOADCONT(cont);
        CBB->incSuspCount();
        /* optimization for:
         *  if (nb) e->decSolveThreads(nb);
         *  e->incSolveThreads(CBB);
         */
        if (orgNB==CBB) {
          SolveActor *sa = SolveActor::Cast(CBB->getActor());
          sa->incThreads();
        }
        e->restartThread();
        goto LBLemulateHook;
      case CE_NOTHING:
        // deref nb, because maybe committed ??
        if (nb) e->decSolveThreads(nb->getBoardFast());
        goto LBLstart;
      }
    }
    goto LBLstart;
  }

// ----------------- end killThread----------------------------------------

// ------------------------------------------------------------------------
// *** Emulate if no task
// ------------------------------------------------------------------------

 LBLemulateHook:
  if (emulateHook0(e)) {
    e->pushTaskOutline(PC,Y,G,X,XSize);
    goto LBLschedule;
  }
  goto LBLemulate;

// ------------------------------------------------------------------------
// *** Emulate: execute continuation
// ------------------------------------------------------------------------
 LBLemulate:

  JUMP( PC );

 LBLdispatcher:

#if defined(THREADED) && defined(XXLINUX)
  /* threaded code broken under linux */
  goto* (void*) (*PC);
#endif

#ifdef SLOW_DEBUG_CHECK
  /* These tests make the emulator really sloooooww */
  osBlockSignals(OK);
  DebugCheckT(osUnblockSignals());
  DebugCheck ((e->currentSolveBoard != CBB->getSolveBoard ()),
              error ("am.currentSolveBoard and real solve board mismatch"));

  Assert(!isFreedRefsArray(Y));
#endif

#ifndef THREADED
  op = CodeArea::getOP(PC);
#endif

#ifdef RECINSTRFETCH
  CodeArea::recordInstr(PC);
#endif

  DebugTrace( if (!trace("emulate",CBB,CAA,PC,Y,G)) {
                goto LBLfailure;
              });

#ifndef THREADED
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
      CallPushCont(PC+2);
      goto LBLFastTailCall;
    }


  Case(FASTTAILCALL)
  LBLFastTailCall:
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);

      Assert((e->currentThread->compMode&1) == entry->getAbstr()->getCompMode());
      CallDoChecks(entry->getAbstr(),entry->getGRegs(),entry->getAbstr()->getArity(),NO);

      Y = NULL; // allocateL(0);
      // set pc
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
      OZ_CFun fun = entry->getFun();
      int arityGot = getPosIntArg(PC+2);
      int arity = entry->getArity();

      CheckArity(arityGot,arity,entry,PC+3);

      LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()));

      switch (fun(arity, X)){
      case SUSPEND:
        {
          e->pushTaskOutline(PC+3,Y,G,0,0);
          Suspension *susp = e->mkSuspension(CPP,fun,X,arity);
          e->suspendOnVarList(susp);
          CHECKSEQ;
        }
      case FAILED:
        killPropagatedCurrentTaskSusp();
        LOCAL_PROPAGATION(localPropStore.reset());
      localhack1:
        HF_FAIL(applFailure(fun), printArgs(X,arity));
      case PROCEED:
        killPropagatedCurrentTaskSusp();
        LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
                          goto localhack1;);
        DISPATCH(3);
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
          e->emptySuspendVarList();
          e->trail.pushIfVar(XPC(2));
          DISPATCH(4);
        }
        e->pushTaskOutline(PC+4,Y,G,X,getPosIntArg(PC+3));
        e->suspendInline(CPP,entry->getFun(),1,XPC(2));
        CHECKSEQ;
      case FAILED:
        SHALLOWFAIL;
        HF_FAIL(applFailure(entry), printArgs(1,XPC(2)));
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
            e->emptySuspendVarList();
            e->trail.pushIfVar(XPC(2));
            e->trail.pushIfVar(XPC(3));
            DISPATCH(5);
          }

          e->pushTaskOutline(PC+5,Y,G,X,getPosIntArg(PC+4));
          e->suspendInline(CPP,entry->getFun(),2,XPC(2),XPC(3));
          CHECKSEQ;
        }
      case FAILED:
        SHALLOWFAIL;
        HF_FAIL(applFailure(entry), printArgs(2,XPC(2),XPC(3)));
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
          TaggedRef B=XPC(3) = makeTaggedRef(newTaggedUVar(CBB));
          if (shallowCP) {
            e->emptySuspendVarList();
            e->trail.pushIfVar(A);
            DISPATCH(5);
          }
          e->pushTaskOutline(PC+5,Y,G,X,getPosIntArg(PC+4));
          e->suspendInline(CPP,entry->getFun(),2,A,B);
          CHECKSEQ;
        }

      case FAILED:
        SHALLOWFAIL;
        HF_FAIL(applFailure(entry), printArgs(1,XPC(2)));
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
          TaggedRef C=XPC(4) = makeTaggedRef(newTaggedUVar(CBB));
          if (shallowCP) {
            e->emptySuspendVarList();
            e->trail.pushIfVar(A);
            e->trail.pushIfVar(B);
            DISPATCH(6);
          }
          e->pushTaskOutline(PC+6,Y,G,X,getPosIntArg(PC+5));
          e->suspendInline(CPP,entry->getFun(),3,A,B,C);
          CHECKSEQ;
        }

      case FAILED:
        SHALLOWFAIL;
        HF_FAIL(applFailure(entry), printArgs(2,XPC(2),XPC(3)));
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
          TaggedRef D=XPC(5) = makeTaggedRef(newTaggedUVar(CBB));
          if (shallowCP) {
            e->emptySuspendVarList();
            e->trail.pushIfVar(A);
            e->trail.pushIfVar(B);
            e->trail.pushIfVar(C);
            DISPATCH(7);
          }
          e->pushTaskOutline(PC+7,Y,G,X,getPosIntArg(PC+6));
          e->suspendInline(CPP,entry->getFun(),4,A,B,C,D);
          CHECKSEQ;
        }

      case FAILED:
        SHALLOWFAIL;
        HF_FAIL(applFailure(entry), printArgs(3,XPC(2),XPC(3),XPC(4)));
      }
    }

  Case(INLINEEQEQ)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun2 fun = (InlineFun2)entry->getInlineFun();

      // note XPC(4) is maybe the same as XPC(2) or XPC(3) !!
      switch (fun(XPC(2),XPC(3),XPC(4))) {
      case FAILED:  Assert(0);
      case PROCEED:
        DISPATCH(6);
      case SUSPEND:
        {
          TaggedRef A=XPC(2);
          TaggedRef B=XPC(3);
          TaggedRef C=XPC(4) = makeTaggedRef(newTaggedUVar(CBB));
          e->pushTaskOutline(PC+6,Y,G,X,getPosIntArg(PC+5));
          e->suspendInline(CPP,entry->getFun(),3,A,B,C);
          CHECKSEQ;
        }
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
        {
          Suspension *susp = e->mkSuspension(CPP,
                                             PC,Y,G,X,getPosIntArg(PC+4));
          addSusp(XPC(2),susp);
        }
        CHECKSEQ;
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
          Suspension *susp=e->mkSuspension(CPP,
                                           PC,Y,G,X,getPosIntArg(PC+5));
          OZ_Term A=XPC(2);
          OZ_Term B=XPC(3);
          DEREF(A,APtr,ATag); DEREF(B,BPtr,BTag);
          Assert(isAnyVar(ATag) || isAnyVar(BTag));
          if (isAnyVar(A)) addSusp(APtr,susp);
          if (isAnyVar(B)) addSusp(BPtr,susp);
          CHECKSEQ;
        }
      }
    }

  Case(SHALLOWTHEN)
    {
      if (e->trail.isEmptyChunk()) {
        shallowCP = NULL;
        e->trail.popMark();
        DISPATCH(1);
      }

      int argsToSave = getPosIntArg(shallowCP+2);
      Suspension *susp = e->mkSuspension(CPP,
                                         shallowCP,Y,G,X,argsToSave);
      shallowCP = NULL;
      e->reduceTrailOnShallow();
      e->suspendOnVarList(susp);
      CHECKSEQ;
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
      if (!isDirtyRefsArray(Y)) {
        deallocateY(Y);
      }
      Y=NULL;
      DISPATCH(1);
    }
// -------------------------------------------------------------------------
// CLASS: CONTROL: FAIL/SUCCESS/RETURN/SAVECONT
// -------------------------------------------------------------------------

  Case(FAILURE)
    {
      HF_FAIL(, message("Executing 'false'\n"));
    }


  Case(SUCCEED)
    DISPATCH(1);

  Case(SAVECONT)
    {
      e->pushTask(getLabelArg(PC+1),Y,G);
      DISPATCH(2);
    }

  Case(RETURN)
  {
    goto LBLpopTask;
  }


// ------------------------------------------------------------------------
// CLASS: Definition
// ------------------------------------------------------------------------

  Case(DEFINITION)
    {
      Reg reg                     = getRegArg(PC+1);
      ProgramCounter nxt          = getLabelArg(PC+2);
      PrTabEntry *predd           = getPredArg(PC+5);
      AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+6);

      AssRegArray &list = predd->gRegs;
      int size = list.getSize();
      RefsArray gRegs = (size == 0) ? (RefsArray) NULL : allocateRefsArray(size);

      Abstraction *p = new Abstraction (predd, gRegs, CBB);

      if (predEntry) {
        predEntry->setPred(p);
      }

      for (int i = 0; i < size; i++) {
        switch (list[i].kind) {
        case XReg: gRegs[i] = X[list[i].number]; break;
        case YReg: gRegs[i] = Y[list[i].number]; break;
        case GReg: gRegs[i] = G[list[i].number]; break;
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


  /* weakDet(X) woken up, WHENEVER something happens to X
   *            (important if X is a FD var) */
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
    /* INCFPC(3); do NOT suspend on next instructions: DET suspensions are
                  woken up always, even if variable is bound to another var */

    int argsToSave = getPosIntArg(PC+2);
    Suspension *susp = e->mkSuspension(CPP,PC,Y,G,X,argsToSave);
    addSusp(makeTaggedRef(termPtr),susp);
    CHECKSEQ;
  };


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
    Suspension *susp = e->mkSuspension(CPP,PC,Y,G,X,argsToSave);
    if (isCVar(tag)) {
      tagged2CVar(term)->addDetSusp(susp);
    } else {
      addSusp(makeTaggedRef(termPtr),susp);
    }
    CHECKSEQ;
  }

  Case(TAILSENDMSGX) isTailCall = OK; ONREG(SendMethod,X);
  Case(TAILSENDMSGY) isTailCall = OK; ONREG(SendMethod,Y);
  Case(TAILSENDMSGG) isTailCall = OK; ONREG(SendMethod,G);

  Case(SENDMSGX) isTailCall = NO; ONREG(SendMethod,X);
  Case(SENDMSGY) isTailCall = NO; ONREG(SendMethod,Y);
  Case(SENDMSGG) isTailCall = NO; ONREG(SendMethod,G);

 SendMethod:
  {
    TaggedRef label   = getLiteralArg(PC+1);
    TaggedRef origObj = RegAccess(HelpReg,getRegArg(PC+2));
    TaggedRef object  = origObj;
    int arity         = getPosIntArg(PC+3);

    PC = isTailCall ? 0 : PC+4;

    DEREF(object,_1,_2);
    if (!isObject(object)) {
      if (isAnyVar(object)) {
        X[0] = makeMethod(arity,label,X);
        X[1] = origObj;
        predArity = 2;
        predicate = chunkCast(e->suspCallHandler);
        goto LBLcall;
      }

      if (isProcedure(object))
        goto bombSend;

      HF_WARN(applFailure(object),
              message("send method application\n");
              printArgs(X+3,arity));  // mm2
    }

    {
      Abstraction *def = getSendMethod(object,label,arity,X);
      if (def == NULL) {
        goto bombSend;
      }

      if (!isTailCall) { CallPushCont(PC); }
      CallDoChecks(def,def->getGRegs(),arity+3,OK);
      Y = NULL; // allocateL(0);
      JUMP(def->getPC());
    }

  bombSend:
    X[0] = makeMethod(arity,label,X);
    predArity = 1;
    predicate = chunkCast(object);
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
    TaggedRef label        = getLiteralArg(PC+1);
    TaggedRef origObject   = RegAccess(HelpReg,getRegArg(PC+2));
    TaggedRef object       = origObject;
    int arity              = getPosIntArg(PC+3);
    Abstraction *def       = NULL;

    PC = isTailCall ? 0 : PC+4;

    DEREF(object,objectPtr,objectTag);
    if (!isObject(object) ||
        NULL == (def = getApplyMethod(object,label,arity-3+3,X[0]))) {
      goto bombApply;
    }

    if (!isTailCall) { CallPushCont(PC); }
    CallDoChecks(def,def->getGRegs(),arity,OK);
    Y = NULL; // allocateL(0);
    JUMP(def->getPC());


  bombApply:
    if (methApplHdl == makeTaggedNULL()) {
      HF_WARN(,
              message("Application handler not set (apply method)\n"));
    }

    TaggedRef method = makeMethod(arity-3,label,X);
    X[4] = X[2];   // outState
    X[3] = X[1];   // ooSelf
    X[2] = method;
    X[1] = X[0];   // inState
    X[0] = origObject;

    predArity = 5;
    predicate = chunkCast(methApplHdl);
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

       PC = isTailCall ? 0 : PC+3;

       DEREF(taggedPredicate,predPtr,predTag);
       if (!isConstChunk(taggedPredicate)) {
         if (isAnyVar(predTag)) {
           X[predArity++] = makeTaggedRef(predPtr);
           predicate = chunkCast(e->suspCallHandler);
           goto LBLcall;
         }
         HF_WARN(applFailure(taggedPredicate),
                 printArgs(X,predArity));
       }

       predicate = chunkCast(taggedPredicate);
     }

// -----------------------------------------------------------------------
// --- Call: entry point
// -----------------------------------------------------------------------

  LBLcall:
     Builtin *bi;

// -----------------------------------------------------------------------
// --- Call: Abstraction
// -----------------------------------------------------------------------

    TypeOfConst typ = predicate->getType();

    switch (typ) {
    case Co_Abstraction:
    case Co_Object:
      {
        Abstraction *def;
        if (typ==Co_Object) {
          /* {Obj Msg} --> {Obj Msg Methods Self} */
          Object *o = (Object*) predicate;
          if (o->getIsClass()) {
            HF_FAIL(message("classes cannot be applied "),);
          }
          if (predArity != 1) {
            HF_FAIL(message("Object application: expect one argument, got: %d",predArity),);
          }
          def = o->getAbstraction();
          X[predArity++] = o->getSlowMethods();
          X[predArity++] = makeTaggedConst(predicate);
        } else {
          def = (Abstraction *) predicate;
        }
        CheckArity(predArity, def->getArity(), def, PC);
        if (!isTailCall) { CallPushCont(PC); }
        CallDoChecks(def,def->getGRegs(),def->getArity(),OK);
        Y = NULL; // allocateL(0);

        JUMP(def->getPC());
      }


// -----------------------------------------------------------------------
// --- Call: Builtin
// -----------------------------------------------------------------------
    case Co_Builtin:
      {
        bi = (Builtin *) predicate;

        CheckArity(predArity, bi->getArity(),bi,PC);

        switch (bi->getType()) {

        case BIsolve:
          {
            isSolveGuided = NO;  isEatWaits = NO;  isSolveDebug = NO;
            goto LBLBIsolve;
          }
        case BIsolveEatWait:
          {
            isSolveGuided = NO;  isEatWaits = OK;  isSolveDebug = NO;
            goto LBLBIsolve;
          }
        case BIsolveGuided:
          {
            isSolveGuided = OK;  isEatWaits = NO;  isSolveDebug = NO;
            goto LBLBIsolve;
          }
        case BIsolveGuidedEatWait:
          {
            isSolveGuided = OK;  isEatWaits = OK;  isSolveDebug = NO;
            goto LBLBIsolve;
          }
        case BIsolveDebugGuided:
          {
            isSolveGuided = OK;  isSolveDebug = OK;
            goto LBLBIsolve;
          }
        case BIsolveCont:    goto LBLBIsolveCont;
        case BIsolved:       goto LBLBIsolved;

        case BIDefault:
          {
            LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()));

            if (e->isSetSFlag(DebugMode)) {
              enterCall(CBB,bi,predArity,X);
            }

            OZ_Bool res = bi->getFun()(predArity, X);
            if (e->isSetSFlag(DebugMode)) {
              exitBuiltin(res,bi,predArity,X);
            }

            killPropagatedCurrentTaskSusp();

            switch (res) {

            case SUSPEND:
              LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()));

              predicate = bi->getSuspHandler();
              if (!predicate) {
                if (!isTailCall) e->pushTaskOutline(PC,Y,G);
                Suspension *susp=e->mkSuspension(CPP,
                                                 bi->getFun(),X,predArity);
                e->suspendOnVarList(susp);
                CHECKSEQ;
              }
              goto LBLcall;
            case FAILED:
              LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()));
              LOCAL_PROPAGATION(localPropStore.reset());
            localHack0:
              HF_FAIL(applFailure(bi), printArgs(X,predArity));
            case PROCEED:
              LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
                                   goto localHack0;);
              if (emulateHook0(e)) {
                if (!isTailCall) {
                  e->pushTaskOutline(PC,Y,G);
                }
                goto LBLschedule;
              }
              if (isTailCall) {
                goto LBLpopTask;
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
      HF_WARN(applFailure(makeTaggedConst(predicate)),
                      );
    } // end switch on type of predicate

// ------------------------------------------------------------------------
// --- Call: Builtin: solve
// ------------------------------------------------------------------------

   LBLBIsolve:
     {
       DebugTrace(trace("solve",CBB));
       TaggedRef x0 = X[0];
       DEREF (x0, _0, x0Tag);

       if (isAnyVar (x0Tag) == OK) {
         predicate = bi->getSuspHandler();
         if (!predicate) {
           HF_WARN(applFailure(bi),
                   message("No suspension handler\n"));
         }
         goto LBLcall;
       }

       if (!isConst(x0) ||
           !(tagged2Const(x0)->getType () == Co_Abstraction ||
             tagged2Const(x0)->getType () == Co_Builtin)) {
         HF_FAIL (,
                  message("Application failed: no procedure in solve combinator\n"));

       }

       TaggedRef output, guidance;

       if (isSolveDebug) {
         // Check the third argument whether it is true or false
         TaggedRef flag = isSolveGuided ? X[2] : X[1];
         DEREF(flag, _1, flagTag);

         if (isAnyVar(flagTag)) {
           predicate = bi->getSuspHandler();
           if (!predicate) {
             HF_WARN(applFailure(bi),
                     message("No suspension handler\n"));
           }
           goto LBLcall;
         }

         if (isLiteral(flagTag)) {
           if (sameLiteral(flag,NameTrue)) {
             isEatWaits = OK;
           } else if (sameLiteral(flag,NameFalse)) {
             isEatWaits = NO;
           } else {
             HF_FAIL (,
             message("Illegal wait flag in solve combinator\n"));
           }
         } else {
           HF_FAIL (,
             message("Illegal wait flag in solve combinator\n"));
         }
       }

       if (isSolveGuided) {
         // This is a guided solver, check whether the input list is ground
         TaggedRef guide = deref(X[1]);

         while (isCons(guide)) {
           TaggedRef head = headDeref(guide);

           if (!(isInt(head) ||
                 (isPair(head) &&
                  isInt(leftDeref(head)) &&
                  isInt(rightDeref(head)))))
             break;

           guide = tailDeref(guide);
         }

         if (!isNil(guide)) {
           predicate = bi->getSuspHandler();
           if (!predicate) {
             HF_WARN(applFailure(bi),
                     message("No suspension handler\n"));
           }
           goto LBLcall;
         }


         output   = isSolveDebug ? X[3] : X[2];
         guidance = X[1];
       } else {
         output   = isSolveDebug ? X[2] : X[1];
         guidance = 0;
       }


       // put continuation if any;
       if (isTailCall == NO)
         e->pushTaskOutline(PC, Y, G);

       // create solve actor(x1);
       // Note: don't perform any derefencing on X[1];
       SolveActor *sa = new SolveActor (CBB,CPP,
                                        e->currentThread->getCompMode(),
                                        output, guidance);

       if (isSolveGuided) sa->setGuided();
       if (isEatWaits)    sa->setEatWaits();
       if (isSolveDebug)  sa->setDebug();

       e->setCurrent(new Board(sa, Bo_Solve), OK);
       CBB->setInstalled();
       e->trail.pushMark();
       sa->setSolveBoard(CBB);

       // put ~'solve actor';
       // Note that CBB is already the 'solve' board;
       e->pushSolve();    // no args;
       e->createTask();

       // apply the predicate;
       predArity = 1;
       predicate = (Abstraction *) tagValueOf (x0);
       X[0] = makeTaggedRef (sa->getSolveVarRef ());
       isTailCall = OK;
       goto LBLcall;   // spare a task - call the predicate directly;
     }

// ------------------------------------------------------------------------
// --- Call: Builtin: solveCont
// ------------------------------------------------------------------------

   LBLBIsolveCont:
     {
       DebugTrace(trace("solve cont",CBB));
       if (((OneCallBuiltin *)bi)->isSeen () == OK) {
         HF_FAIL(,
                 message("once-only abstraction applied more than once\n"));
       }

       Board *solveBB =
         (Board *) tagValueOf ((((OneCallBuiltin *) bi)->getGRegs ())[0]);
       // VERBMSG("solve continuation",((void *) bi),((void *) solveBB));
       // kost@ 22.12.94: 'hasSeen' has new implementation now;
       ((OneCallBuiltin *) bi)->hasSeen ();
       DebugCheck ((solveBB->isSolve () == NO),
                   error ("no 'solve' blackboard  in solve continuation builtin"));
       DebugCheck((solveBB->isCommitted () == OK ||
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

       // install (i.e. perform 'unit commit') 'board-to-install' if any;
       int compMode;
       Board *boardToInstall = solveAA->getBoardToInstall (compMode);
       if (boardToInstall != (Board *) NULL) {
         DebugCheck ((boardToInstall->isCommitted () == OK ||
                      boardToInstall->isFailed () == OK),
                     error ("boardToInstall is already committed"));
         boardToInstall->setCommitted (CBB);
#ifdef DEBUG_CHECK
         if ( !e->installScript (boardToInstall->getScriptRef ()) ) {
           LOCAL_PROPAGATION(HF_FAIL(,));
           // error ("installScript has failed in solveCont");
           message("installScript has failed in solveCont (0x%x to 0x%x)\n",
                    (void *) boardToInstall, (void *) solveBB);
         }
#else

         LOCAL_PROPAGATION(
           if (!e->installScript (boardToInstall->getScriptRef ())) {
             HF_NOMSG;
           }
         )

         NO_LOCAL_PROPAGATION(
           (void) e->installScript (boardToInstall->getScriptRef ());
         )
#endif
         // add the suspensions of the committed board and remove
         // its suspension itself;
         CBB->incSuspCount (boardToInstall->getSuspCount () - 1);
         // get continuation of 'board-to-install' if any;
         if (boardToInstall->isWaitTop () == NO) {
           if (isTailCall == NO)
             e->pushTaskOutline(PC,Y,G);
           else
             isTailCall = NO;
           e->currentThread->checkCompMode(compMode);
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
       if ( !e->fastUnifyOutline(solveAA->getSolveVar(), X[0], OK) ) {
         HF_NOMSG;
       }

       if (isTailCall) {
         goto LBLpopTask;
       }
       goto LBLemulate;
     }

// ------------------------------------------------------------------------
// --- Call: Builtin: solved
// ------------------------------------------------------------------------

   LBLBIsolved:
     {
       DebugTrace(trace("solved",CBB));
       TaggedRef valueIn = (((SolvedBuiltin *) bi)->getGRegs ())[0];
       Assert(!isRef(valueIn));

       if (isConst(valueIn) && tagged2Const(valueIn)->getType() == Co_Board) {
         Board *solveBB =
           (Board *) tagValueOf ((((SolvedBuiltin *) bi)->getGRegs ())[0]);
         // VERBMSG("solved",((void *) bi),((void *) solveBB));
         Assert(solveBB->isSolve());
         DebugCheck((solveBB->isCommitted () == OK ||
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

         // adjoin the list of or-actors to the list in actual solve actor!!!
         Board *currentSolveBB = e->currentSolveBoard;
         if (currentSolveBB == (Board *) NULL) {
           DebugCheckT(message("solved is applied not inside of a search problem?\n"));
         } else {
           SolveActor::Cast(currentSolveBB->getActor())->pushWaitActorsStackOf(solveAA);
         }

         if ( !e->fastUnifyOutline(solveAA->getSolveVar(), X[0], OK) ) {
           HF_NOMSG;
         }
       } else {
         if ( !e->fastUnifyOutline(valueIn, X[0], OK) ) {
           HF_NOMSG;
         }
       }

       if (isTailCall) {
         goto LBLpopTask;
       }
       JUMP(PC);
     }

   }

// --------------------------------------------------------------------------
// --- end call/execute -----------------------------------------------------
// --------------------------------------------------------------------------


  Case(WAIT)
    {
      Assert(CBB->isWait() && !CBB->isCommitted());

      CBB->decSuspCount();
      {
        TaskStackEntry topElem = e->currentThread->taskStack.pop();
        Assert((ContFlag) (ToInt32(topElem) & 0xf) == C_LOCAL);
      }
      /* unit commit */
      WaitActor *aa = WaitActor::Cast(CBB->getActor());
      if (aa->hasOneChild()) {
        Board *waitBoard = CBB;
        e->reduceTrailOnUnitCommit();
        waitBoard->unsetInstalled();
        e->setCurrent(aa->getBoardFast());

        waitBoard->setCommitted(CBB);   // by kost@ 4.10.94
        Bool ret = e->installScript(waitBoard->getScriptRef());
        if (!ret) {
          HF_NOMSG;
        }
        Assert(ret!=NO);
        CBB->incSuspCount(waitBoard->getSuspCount()-1);
        DISPATCH(1);
      }

      // not commitable, suspend
      CBB->setWaiting();
      goto LBLsuspendBoard;
    }


  Case(WAITTOP)
    {
      /* top commit */
      CBB->decSuspCount();
      {
        TaskStackEntry topElem = e->currentThread->taskStack.pop();
        Assert((ContFlag) (ToInt32(topElem) & 0xf) == C_LOCAL);
      }
      if ( e->entailment() ) {
        e->trail.popMark();

        tmpBB = CBB;

        e->setCurrent(CBB->getParentFast());
        tmpBB->unsetInstalled();
        tmpBB->setCommitted(CBB);
        CBB->decSuspCount();
        goto LBLpopTask;
      }

      /* unit commit for WAITTOP */
      WaitActor *aa = WaitActor::Cast(CBB->getActor());
      Board *bb = CBB;
      if (aa->hasOneChild()) {
        e->reduceTrailOnUnitCommit();
        bb->unsetInstalled();
        e->setCurrent(aa->getBoardFast());

        bb->setCommitted(CBB);    // by kost@ 4.10.94
        Bool ret = e->installScript(bb->getScriptRef());
        if (!ret) {
          HF_NOMSG;
        }
        Assert(ret != NO);
        CBB->incSuspCount(bb->getSuspCount()-1);
        goto LBLpopTask;
      }

      /* suspend WAITTOP */
      CBB->setWaitTop();
      CBB->setWaiting();
      goto LBLsuspendBoardWaitTop;
    }

  Case(ASK)
    {
      CBB->decSuspCount();
      {
        TaskStackEntry topElem = e->currentThread->taskStack.pop();
        Assert((ContFlag) (ToInt32(topElem) & 0xf) == C_LOCAL);
      }
      // entailment ?
      if (e->entailment()) {
        e->trail.popMark();
        tmpBB = CBB;
        e->setCurrent(CBB->getParentFast());
        tmpBB->unsetInstalled();
        tmpBB->setCommitted(CBB);
        CBB->decSuspCount();
        DISPATCH(1);
      }

    LBLsuspendBoard:

      CBB->setBody(PC+1, Y, G,NULL,0);

    LBLsuspendBoardWaitTop:
      markDirtyRefsArray(Y);
      DebugTrace(trace("suspend clause",CBB,CAA));

      CAA = AWActor::Cast (CBB->getActor());

      e->deinstallCurrent();
      if (CAA->hasNext()) {

      LBLexecuteNext:
        DebugTrace(trace("next clause",CBB,CAA));

        LOADCONT(CAA->getNext());

        goto LBLemulate; // no thread switch allowed here (CAA)
      }

      DebugTrace(trace("suspend actor",CBB,CAA));

      goto LBLpopTask;
    }


// -------------------------------------------------------------------------
// CLASS: NODES: CREATE/END
// -------------------------------------------------------------------------

  Case(CREATECOND)
    {
      ProgramCounter elsePC = getLabelArg(PC+1);
      int argsToSave = getPosIntArg(PC+2);

      CAA = new AskActor(CBB,CPP,e->currentThread->getCompMode(),
                         elsePC ? elsePC : NOCODE,
                         NOCODE, Y, G, X, argsToSave);
      DISPATCH(3);
    }

  Case(CREATEOR)
    {
      ProgramCounter elsePC = getLabelArg (PC+1);
      int argsToSave = getPosIntArg (PC+2);
      CAA = new WaitActor(CBB,CPP,e->currentThread->getCompMode(),
                          NOCODE,Y,G,X,argsToSave);
      DISPATCH(3);
    }

  Case(CREATEENUMOR)
    {
      ProgramCounter elsePC = getLabelArg (PC+1);
      int argsToSave = getPosIntArg (PC+2);

      CAA = new WaitActor(CBB,CPP,e->currentThread->getCompMode(),
                          NOCODE, Y, G, X, argsToSave);
      if (e->currentSolveBoard != (Board *) NULL) {
        SolveActor *sa= SolveActor::Cast (e->currentSolveBoard->getActor ());
        sa->pushWaitActor (WaitActor::Cast (CAA));
      }
      DISPATCH(3);
    }

  Case(WAITCLAUSE)
    {
      // create a node
      e->setCurrent(new Board(CAA,Bo_Wait),OK);
      if (e->currentThread->compMode == ALLSEQMODE) {
        e->currentThread->compMode = SEQMODE;
      }
      e->pushLocal();
      markDirtyRefsArray(Y);
      CBB->setInstalled();
      e->trail.pushMark();
      DebugCheckT(CAA=NULL);
      IncfProfCounter(waitCounter,sizeof(Board));
      DISPATCH(1);
    }

  Case(ASKCLAUSE)
    {
      e->setCurrent(new Board(CAA,Bo_Ask),OK);
      if (e->currentThread->compMode == ALLSEQMODE) {
        e->currentThread->compMode = SEQMODE;
      }
      e->pushLocal();
      markDirtyRefsArray(Y);
      CBB->setInstalled();
      e->trail.pushMark();
      DebugCheckT(CAA=NULL);
      IncfProfCounter(askCounter,sizeof(Board));
      DISPATCH(1);
    }


  Case(ELSECLAUSE)
    DISPATCH(1);

  Case(THREAD)
    {
      markDirtyRefsArray(Y);
      ProgramCounter newPC = PC+2;
      ProgramCounter contPC = getLabelArg(PC+1);

      int prio = CPP;
      int defPrio = ozconf.defaultPriority;
      if (prio > defPrio) {
        prio = defPrio;
      }

      Thread *tt = e->createThread(prio,e->currentThread->getCompMode());

      tt->pushCont(newPC,Y,G,NULL,0,OK);
      JUMP(contPC);
    }


  Case(NEXTCLAUSE)
      CAA->nextClause(getLabelArg(PC+1));
      DISPATCH(2);

  Case(LASTCLAUSE)
      CAA->lastClause();
      DISPATCH(1);

// -------------------------------------------------------------------------
// CLASS: MISC: ERROR/NOOP/default
// -------------------------------------------------------------------------

  Case(ERROR)
      error("Emulate: ERROR command executed");
      goto LBLerror;


  Case(DEBUGINFO)
    {
      /*
      TaggedRef filename = getLiteralArg(PC+1);
      int line           = smallIntValue(getNumberArg(PC+2));
      int absPos         = smallIntValue(getNumberArg(PC+3));
      int comment        = getLiteralArg(PC+4);
      int noArgs         = smallIntValue(getNumberArg(PC+5));
      printf("%s in line %d in file: %s\n",
             OZ_toC(comment),line,OZ_toC(filename));
      if (noArgs != -1) {
        printf("\t");
        for (int i=0; i<noArgs; i++) {
          printf("%s ",OZ_toC(X[i]));
        }
      }
      printf("\n");
      */
      DISPATCH(6);
    }

  Case(SWITCHCOMPMODE)
    /* brute force: don't know exactly, when to mark Y as dirty (RS) */
    markDirtyRefsArray(Y);
    e->currentThread->switchCompMode();
    /*
     * quick bug fix to handle toplevel blocking (mm2)
     *  problem: optimization in setCompMode removes all mode switches from
     *  task stack, so that the detection of toplevel blocking doesn't work.
     */
    if (e->currentThread==e->rootThread &&
        e->currentThread->getCompMode() == PARMODE &&
        e->currentThread->taskStack.isEmpty()) {
      e->currentThread->taskStack.pushCompMode(ALLSEQMODE);
    }
    DISPATCH(1);

  Case(TESTLABEL1)
  Case(TESTLABEL2)
  Case(TESTLABEL3)
  Case(TESTLABEL4)

  Case(TEST1)
  Case(TEST2)
  Case(TEST3)
  Case(TEST4)

  Case(INLINEDOT)

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
    shallowCP = NULL;
    JUMP(nxt);
  }

 LBLfailure:
  {
    DebugTrace(trace("fail",CBB));
    Assert(CBB->isInstalled());
    Actor *aa=CBB->getActor();
    if (aa->isAskWait()) {
      (AWActor::Cast(aa))->failChild(CBB);
    }
    CBB->setFailed();
    e->reduceTrailOnFail();
    CBB->unsetInstalled();
    e->setCurrent(aa->getBoardFast());
    if (!e->currentThread->taskStack.discardLocalTasks()) {
      e->currentThread->board=CBB;
      CBB->incSuspCount();
    }

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
          CBB->decSuspCount();
          goto LBLemulateHook;
        }

/* rule: if fi --> false */
        HF_FAIL(,message("reducing 'if fi' to 'false'\n"));
      }
    } else if (aa->isWait ()) {
      if ((WaitActor::Cast (aa))->hasNext()) {
        CAA = WaitActor::Cast (aa);
        goto LBLexecuteNext;
      }
/* rule: or ro (bottom commit) */
      if ((WaitActor::Cast (aa))->hasNoChilds()) {
        aa->setCommitted();
        HF_FAIL(,
                message("bottom commit\n"));
      }
/* rule: or <sigma> ro (unit commit rule) */
      if ((WaitActor::Cast (aa))->hasOneChild()) {
        Board *waitBoard = (WaitActor::Cast (aa))->getChild();
        DebugTrace(trace("reduce actor unit commit",waitBoard,aa));
        if (waitBoard->isWaiting()) {
          waitBoard->setCommitted(CBB); // do this first !!!
          if (!e->installScript(waitBoard->getScriptRef())) {
            HF_FAIL(,
                    message("unit commit failed\n"));
          }

          /* add the suspension from the committed board
             remove the suspension for the board itself */
          CBB->incSuspCount(waitBoard->getSuspCount()-1);

          /* unit commit & WAITTOP */
          if (waitBoard->isWaitTop()) {
            goto LBLpopTask;
          }

          /* unit commit & WAIT, e.g. or X = 1 ... then ... [] false ro */
          LOADCONT(waitBoard->getBodyPtr());
          Assert(PC != NOCODE);

          goto LBLemulateHook;
        }
      }
    } else {
      if (ozconf.showSolveFailure) {
        HF_BODY("solve failed","");
      }
      //  Reduce (i.e. with failure in this case) the solve actor;
      //  The solve actor goes simply away, and the 'failed' atom is bound to
      // the result variable;
      aa->setCommitted();
      CBB->decSuspCount();
      // for statistic purposes
      ozstat.incSolveFailed();
      if ( !e->fastUnifyOutline(SolveActor::Cast(aa)->getResult(),
                                SolveActor::Cast(aa)->genFailed(),
                                OK) ) {
        HF_NOMSG;
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
