/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow, mehl

      (main) AM::procedure;
      another things.
  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "am.hh"
#endif

#include "am.hh"

#include "genvar.hh"
#include "fdbuilti.hh"
#include "builtins.hh"
#include "ip.hh"

AM am;

/* -------------------------------------------------------------------------
 * Init and exit AM
 * -------------------------------------------------------------------------*/

static
void usage(int /* argc */,char **argv) {
  fprintf(stderr,
          "usage: %s <options>\n",
          argv[0]);
  fprintf(stderr, " -E: running under emacs\n");
  fprintf(stderr, " -d: debugging on\n");
  fprintf(stderr, " -quiet: no banner\n");
  fprintf(stderr, " -c <compiler>: start the compiler\n");
  fprintf(stderr, " -S <fifo>: connect to compiler via FIFO\n");
  fprintf(stderr, " -u <url>: start a compute server\n");
  fprintf(stderr, " -f <file>: execute precompiled file\n");
  fprintf(stderr, " -a <args> ...: application arguments\n");
  osExit(1);
}

static
char *getOptArg(int &i, int argc, char **argv)
{
  i++;
  if (i == argc) {
    fprintf(stderr,"Option '%s' requires argument.\n",argv[i-1]);
    usage(argc,argv);
    return NULL;
  }

  return argv[i];
}


static
void printBanner()
{
  version();

#if defined(DEBUG_DET) && !defined(WINDOWS)
  // windows dumps if I activate this
  warning("DEBUG_DET implies eager waking of sleep.");
#endif

#ifdef NO_LTQ
  warning("LTQ is turned off.");
#else
#ifdef LOCAL_THREAD_STACK
  warning("LTQ is a STACK.");
#endif
#endif

#ifdef DEBUG_CHECK
  printf("Compile Flags:"
         " DEBUG_CHECK"
#ifdef DEBUG_DET
         " DEBUG_DET"
#endif
#ifdef DEBUG_TRACE
         " DEBUG_TRACE"
#endif
#ifdef DEBUG_GC
         " DEBUG_GC"
#endif
#ifdef DEBUG_FD
         " DEBUG_FD"
#endif
#ifdef DEBUG_FSET
         " DEBUG_FSET"
#endif
#ifdef PROFILE_FD
         " PROFILE_FD"
#endif
#ifdef RECINSTRFETCH
         " RECINSTRFETCH=%d", RECINSTRFETCH
#endif
         );
#endif

  printf("\n");

#ifdef PROFILE
  printf("Compiled to support gprof-profiling.\n");
#ifdef DEBUG_DET
  printf("Deterministic scheduling.\n");
#else
  printf("Time-slice scheduling.\n");
#endif
#endif

#ifdef THREADED
  // printf("Using threaded code (abs jumps).\n");
#else
  printf("Not using threaded code.\n");
#endif

#ifdef PROFILE_FD
  printf("Compiled to support fd-profiling.\n");
#endif
}


extern void bigIntInit(); /* from value.cc */
extern void initffuns();  /* from initffuns.cc */

void AM::init(int argc,char **argv)
{
  Assert(makeTaggedNULL() == 0);
  ozconf.init();
  ProfileCode(ozstat.initCount());
  osInit();
  bigIntInit();
  initffuns();

  installingScript = FALSE;

  suspendVarList          = makeTaggedNULL();
  aVarUnifyHandler        = makeTaggedNULL();
  aVarBindHandler         = makeTaggedNULL();
  methApplHdl             = makeTaggedNULL();
  sendHdl                 = makeTaggedNULL();
  newHdl                  = makeTaggedNULL();
  defaultExceptionHandler = makeTaggedNULL();
  opiCompiler             = makeTaggedNULL();

  char *compilerName = OzCompiler;

  char *tmp;
  if ((tmp = getenv("OZPATH"))) {
    ozconf.ozPath = tmp;
    ozconf.linkPath = tmp;
  }

  if ((tmp = getenv("OZLINKPATH"))) {
    ozconf.linkPath = tmp;
  }

  char *compilerFIFO = NULL;  // path name where to connect to
  char *precompiledFile = NULL;
  char *url=0;
  Bool quiet = FALSE;
  int moreThanOne = 0;

  /* process command line arguments */
  ozconf.argV = NULL;
  ozconf.argC = 0;

  // Denys:
  // in case the emulator is invoked as the script `ozx'
  // then assume `-quiet' and use 1st arg as url

  if (argc>=2) {
    if (strcmp(argv[0],"ozx")==0) {
      quiet = TRUE;
      url   = argv[1];
    } else {
      int n = strlen(argv[0]);
      if (n >= 4 && strcmp(argv[0]+n-4,"/ozx")==0) {
        quiet = TRUE;
        url   = argv[1];
      }
    }
  }

  for (int i=url?2:1; i<argc; i++) {
    if (strcmp(argv[i],"-E")==0) {
      ozconf.runningUnderEmacs = 1;
      continue;
    }
    if (strcmp(argv[i],"-d")==0) {
#ifdef MM_DEBUG
      tracerOn();
#endif
      continue;
    }
    if (strcmp(argv[i],"-quiet")==0) {
      quiet = TRUE;
      continue;
    }
    if (strcmp(argv[i],"-c")==0) {
      moreThanOne++;
      compilerName = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-S")==0) {
      moreThanOne++;
      compilerFIFO = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-f")==0) {
      moreThanOne++;
      precompiledFile = getOptArg(i,argc,argv);
      continue;
    }

    if (strcmp(argv[i],"-u")==0) {
      moreThanOne++;
      url = getOptArg(i,argc,argv);
      continue;
    }

    if (strcmp(argv[i],"-a")==0) {
      ozconf.argC = argc-i-1;
      ozconf.argV = argv+i+1;
      break;
    }

    fprintf(stderr,"Unknown option '%s'.\n",argv[i]);
    usage(argc,argv);
  }

  if (moreThanOne > 1) {
    fprintf(stderr,"Atmost one of '-u', '-f', '-S' allowed.\n");
    usage(argc,argv);
   }

#ifdef DEBUG_CHECK
  if (!quiet)
    ozconf.showIdleMessage=1;
#endif

  if (quiet == FALSE) {
    printBanner();
  }

  isStandaloneF=NO;
  if (url) {
    isStandaloneF=OK;
  } else {
    if (compilerFIFO) {
      compStream = connectCompiler(compilerFIFO);
    } else if (precompiledFile) {
      compStream = useFile(precompiledFile);
      isStandaloneF=OK;
    } else if (compilerName) {
      compStream = execCompiler(compilerName);
    }

    if (compStream == NULL) {
      fprintf(stderr,"Cannot open code input\n");
      ossleep(5);
      osExit(1);
    }

    checkVersion();
  }


  engine(OK);

  initFDs();

  initMemoryManagement();

// not changeable
  // SizeOfWorkingArea,NumberOfXRegisters,NumberOfYRegisters


// internal registers
  statusReg    = (StatusBit)0;
  criticalFlag = NO;
  xRegs        = allocateStaticRefsArray(NumberOfXRegisters);

  rootBoard = new Board(NULL,Bo_Root);
  rootBoard->setInstalled();
  currentBoard = NULL;
  cachedStack  = NULL;
  cachedSelf   = NULL;
  shallowHeapTop = NULL;
  setCurrent(rootBoard,OK);
  currentSolveBoard = (Board *) NULL;
  wasSolveSet = NO;

  lastThreadID     = 0;
  lastFrameID      = 0;
  suspendDebug     = runChildren = NO;
  threadStreamTail = OZ_newVariable();

  initThreads();
  toplevelQueue = (Toplevel *) NULL;

  // builtins
  BuiltinTabEntry *entry = BIinit();
  if (!entry) {
    error("BIinit failed");
    osExit(1);
  }
  initLiterals();

  extern void initTagged();
  initTagged();

  toplevelVars      = allocateRefsArray(ozconf.numToplevelVars);
  toplevelVarsCount = 0;

  toplevelVars[0] = makeTaggedConst(entry);

  ioNodes = new IONode[osOpenMax()];

  if (!isStandalone()) {
    osWatchFD(compStream->csfileno(),SEL_READ);
  }

  osInitSignals();
  osSetAlarmTimer(CLOCK_TICK/1000);

  if (!url) {
    // --> make sure that we check for input from compiler
    setSFlag(IOReady);
  } else {
    if (!perdioInit()) {
      fprintf(stderr,"Perdio initialization for URL %s failed\n",url);
      exit(1);
    }

    OZ_Term v=oz_newVariable();
    OZ_Return ret = loadURL(url,v);
    if (ret!=PROCEED) {
      char *aux = (ret==RAISE) ? toC(exception.value) : "unknown error";
      fprintf(stderr,"Loading from URL '%s' failed: %s\n",url,aux);
      fprintf(stderr,"Maybe recompilation needed?\n");
      exit(1);
    }
    Thread *tt = am.mkRunnableThread(DEFAULT_PRIORITY, am.rootBoard);
    tt->pushCall(v, 0, 0);
    am.scheduleThread(tt);
  }

#ifdef DEBUG_CHECK
  dontPropagate = NO;
#endif

  profileMode = NO;

}

void AM::checkVersion()
{
  char s[100];
  char *ss = compStream->csgets(s,100);
  if (ss && ss[strlen(ss)-1] == '\n')
    ss[strlen(ss)-1] = '\0';
  if (ss != NULL && strcmp(ss,OZVERSION) != 0) {
    fprintf(stderr,"*** Wrong version from compiler\n");
    fprintf(stderr,"*** Expected: '%s', got: '%s'\n",OZVERSION,ss);
    ossleep(3);
    osExit(1);
  }
}

void AM::exitOz(int status)
{
  if (compStream) compStream->csclose();
  osExit(status);
}

/* -------------------------------------------------------------------------
 * Unification
 * -------------------------------------------------------------------------*/


Bool AM::fastUnifyOutline(TaggedRef ref1, TaggedRef ref2, ByteCode *scp)
{
  return fastUnify(ref1, ref2, scp);
}

Bool AM::isLocalUVarOutline(TaggedRef var, TaggedRef *varPtr)
{
  Board *bb=tagged2VarHome(var);
  if (bb->isCommitted()) {
    bb=bb->derefBoard();
    *varPtr=makeTaggedUVar(bb);
  }
  return  bb == currentBoard;
}

Bool AM::isLocalSVarOutline(SVariable *var)
{
  Board *home = var->getHomeUpdate();
  return home == currentBoard;
}


inline
Bool AM::installScript(Script &script)
{
  Bool ret = OK;
  installingScript = TRUE;
  for (int index = 0; index < script.getSize(); index++) {
    if (!unify(script[index].getLeft(),script[index].getRight())) {
      ret = NO;
      if (!isToplevel()) {
        break;
      }
    }
  }
  installingScript = FALSE;
#ifndef DEBUG_CHECK
  script.dealloc();
#else
  if (ret == OK)
    script.dealloc();
#endif
  return ret;
}

Bool AM::installScriptOutline(Script &script)
{
  return installScript(script);
}


inline
Board *getVarBoard(TaggedRef var)
{
  CHECK_ISVAR(var);

  if (isUVar(var))
    return tagged2VarHome(var);
  if (isSVar(var))
    return tagged2SVar(var)->getHome1();

  return taggedCVar2SVar(var)->getHome1();
}


inline
Bool AM::isMoreLocal(TaggedRef var1, TaggedRef var2)
{
  Board *board1 = getVarBoard(var1)->derefBoard();
  Board *board2 = getVarBoard(var2)->derefBoard();
  return isBelow(board1,board2);
}


/* Define a partial order on CVARs:
 *
 *               Lazy
 *                |
 *                |
 *              Perdio
 *                |
 *           +----------+
 *           |    |     |
 *             any other
*/


/* return -1 (v1=<v2), +1 (v1>=v2), 0 (dont care) */

int cmpCVar(GenCVariable *v1, GenCVariable *v2)
{
  TypeOfGenCVariable t1 = v1->getType();
  TypeOfGenCVariable t2 = v2->getType();
  if (t1==LazyVariable)   return  1;
  if (t2==LazyVariable)   return -1;
  if (t1==PerdioVariable) return  1;
  if (t2==PerdioVariable) return -1;
  return 0;
}


static Stack unifyStack(100,Stack_WithMalloc);
static Stack rebindTrail(100,Stack_WithMalloc);

inline
void rebind(TaggedRef *refPtr, TaggedRef *ptr2)
{
  rebindTrail.ensureFree(2);
  rebindTrail.push(refPtr,NO);
  rebindTrail.push(ToPointer(*refPtr),NO);
  doBind(refPtr,makeTaggedRef(ptr2));
}

#define PopRebindTrail(value,refPtr)                    \
    TaggedRef value   = ToInt32(rebindTrail.pop());     \
    TaggedRef *refPtr = (TaggedRef*) rebindTrail.pop();


Bool AM::unify(TaggedRef t1, TaggedRef t2, ByteCode *scp)
{
  Assert(shallowHeapTop && scp || shallowHeapTop==0 && scp==0);
  Assert(unifyStack.isEmpty()); /* unify is not reentrant */
  CHECK_NONVAR(t1); CHECK_NONVAR(t2);

  Bool result = NO;

  TaggedRef *termPtr1 = &t1;
  TaggedRef *termPtr2 = &t2;

loop:
  int argSize;

  COUNT(totalUnify);

  DEREFPTR(term1,termPtr1,tag1);
  DEREFPTR(term2,termPtr2,tag2);

  // identical terms ?
  if (term1 == term2 &&
      (!isUVar(term1) || termPtr1 == termPtr2)) {
    goto next;
  }

  if (isAnyVar(term1)) {
    if (isAnyVar(term2)) {
      goto var_var;
    } else {
      goto var_nonvar;
    }
  } else {
    if (isAnyVar(term2)) {
      Swap(term1,term2,TaggedRef);
      Swap(termPtr1,termPtr2,TaggedRef*);
      Swap(tag1,tag2,TypeOfTerm);
      goto var_nonvar;
    } else {
      goto nonvar_nonvar;
    }
  }


 /*************/
 var_nonvar:

  COUNT(varNonvarUnify);

  if (isCVar(tag1)) {
    if (tagged2CVar(term1)->unify(termPtr1, term1, termPtr2, term2, scp))
      goto next;
    goto fail;
  }

  bindToNonvar(termPtr1, term1, term2, scp);
  goto next;



 /*************/
 var_var:

  /*
   * The implemented partial order for binding variables to variables is:
   *   local -> global
   *   UVAR/SVAR -> CVAR (prefer binding nonCVars to CVars)
   *   UVAR      -> SVAR
   *   local newer -> local older
   */
  COUNT(varVarUnify);
  if (isNotCVar(tag1)) {
    if (isNotCVar(tag2) &&
        isMoreLocal(term2,term1) &&
        (!isLocalVariable(term1,termPtr1) ||
         (isUVar(term2) && !isUVar(term1)) ||
         heapNewer(termPtr2,termPtr1))) {
      genericBind(termPtr2, term2, termPtr1, *termPtr1);
    } else {
      genericBind(termPtr1, term1, termPtr2, *termPtr2);
    }
    goto next;
  }

  if (isNotCVar(tag2)) {
    genericBind(termPtr2, term2, termPtr1, *termPtr1);
    goto next;
  }

  Assert(isCVar(tag1) && isCVar(tag2));
  /* prefered binding of perdio vars */
  if (cmpCVar(tagged2CVar(term2),tagged2CVar(term1))==1) {
    Swap(term1,term2,TaggedRef);
    Swap(termPtr1,termPtr2,TaggedRef*);
  }
  if (tagged2CVar(term1)->unify(termPtr1,term1,termPtr2,term2,scp))
    goto next;
  goto fail;



 /*************/
 nonvar_nonvar:

  COUNT(nonvarNonvarUnify);

  if (tag1 != tag2)
    goto fail;

  switch ( tag1 ) {

  case FSETVALUE:
    if (((FSetValue *) tagged2FSetValue(term1))->unify(term2))
      goto next;
    goto fail;

  case OZCONST:
    if (tagged2Const(term1)->unify(term2,scp))
      goto next;
    goto fail;

  case LTUPLE:
    {
      COUNT(recRecUnify);
      LTuple *lt1 = tagged2LTuple(term1);
      LTuple *lt2 = tagged2LTuple(term2);

      rebind(termPtr2,termPtr1);
      argSize = 2;
      termPtr1 = lt1->getRef();
      termPtr2 = lt2->getRef();
      goto push;
    }

  case SRECORD:
    {
      COUNT(recRecUnify);
      SRecord *sr1 = tagged2SRecord(term1);
      SRecord *sr2 = tagged2SRecord(term2);

      if (! sr1->compareFunctor(sr2))
        goto fail;

      rebind(termPtr2,termPtr1);
      argSize  = sr1->getWidth();
      termPtr1 = sr1->getRef();
      termPtr2 = sr2->getRef();
      goto push;
    }

  case OZFLOAT:
  case BIGINT:
  case SMALLINT:
    if (numberEq(term1,term2))
      goto next;
    goto fail;

  case LITERAL:
    /* literals unify if their pointers are equal */
  default:
    goto fail;
  }


 /*************/

next:
  if (unifyStack.isEmpty()) {
    result = OK;
    goto exit;
  }

  termPtr2 = (TaggedRef*) unifyStack.pop();
  termPtr1 = (TaggedRef*) unifyStack.pop();
  argSize  = ToInt32(unifyStack.pop());
  // fall through

push:
  if (argSize>1) {
    unifyStack.ensureFree(3);
    unifyStack.push(ToPointer(argSize-1),NO);
    unifyStack.push(termPtr1+1,NO);
    unifyStack.push(termPtr2+1,NO);
  }
  goto loop;

fail:
  Assert(result==NO);
  unifyStack.mkEmpty();
  // fall through

exit:
  while (!rebindTrail.isEmpty ()) {
    PopRebindTrail(value,refPtr);
    doBind(refPtr,value);
  }

  return result;
}


/*
  This function checks if the current board is between "varHome" and "to"
  resp. equal to "to".
  */

BFlag AM::isBetween(Board *to, Board *varHome)
{
  while (1) {
    if (to == currentBoard) return B_BETWEEN;
    if (to == varHome) return B_NOT_BETWEEN;
    to = to->getParentAndTest();
    if (!to) return B_DEAD;
  }
}

Bool AM::isBelow(Board *below, Board *above)
{
  while (1) {
    if (below == above) return OK;
    if (below == rootBoard) return NO;
    below = below->getParent();
  }
}



inline
Bool AM::wakeUpThread(Thread *tt, Board *home)
{
  Assert (tt->isSuspended());
  Assert (tt->isRThread());

  switch (isBetween(GETBOARD(tt), home)) {
  case B_BETWEEN:
    suspThreadToRunnable(tt);
    scheduleThread(tt);
    return TRUE;

  case B_NOT_BETWEEN:
    return FALSE;

  case B_DEAD:
    //
    //  The whole thread is eliminated - because of the invariant
    // stated just before 'disposeSuspendedThread ()' in thread.hh;
    tt->markDeadThread();
    checkExtThread(tt);
    freeThreadBody(tt);
    return TRUE;

  default:
    Assert(0);
    return FALSE;
  }
}


inline
Bool AM::wakeUpBoard(Thread *tt, Board *home)
{
  Assert(tt->isSuspended());
  Assert(tt->getThrType() == S_WAKEUP);

  //
  //  Note:
  //  We use here the dereferenced board pointer, because:
  // - normally, there should be a *single* "wakeup" suspension
  //   per guard (TODO);
  // - when "unit commit" takes place, the rest of (suspended?) threads
  //   from that guard belong to the guard just above
  //   (or toplevel, of course) - we have to update
  //   the threads counter there;
  // - garbage collector moves the pointer anyway.
  //
  //  It's relevant (should be) for unit commits *only*;
  //  Implicitly move the thread upstairs - the threads counter
  // should be already updated before (during unit committing);
  Board *bb=GETBOARD(tt);

  //
  //  Do not propagate to the current board, but discard it;
  //  Do not propagate to the board which has a runnable
  // "wakeup" thread;
  //
  // Note that we don't need to schedule the wakeup for the board
  // because in both cases there is a thread which will check
  // entailment for us;
  if (bb == currentBoard || bb->isNervous ()) {
#ifdef DEBUG_CHECK
    // because of assertions in decSuspCount and getSuspCount
    if (bb->isFailed()) {
      tt->markDeadThread();
      checkExtThread(tt);
      return OK;
    }
#endif
    bb->decSuspCount();

    Assert(bb->getSuspCount() > 0);
    tt->markDeadThread();
    // checkExtThread(); // don't check here !
    return OK;
  }

  //
  //  Don't propagate to the variable's home board (again,
  // this can happen only in the case of unit commit), but we have
  // to schedule a wakeup for the new thread's home board,
  // because it could be the last thread in it - check entailment!
  if (bb == home && bb->getSuspCount() == 1) {
    wakeupToRunnable(tt);
    scheduleThread(tt);
    return OK;
  }

  //
  //  General case;
  switch (isBetween(bb, home)) {
  case B_BETWEEN:
    Assert(!currentBoard->isSolve() || currentSolveBoard);
    wakeupToRunnable(tt);
    scheduleThread(tt);
    return OK;

  case B_NOT_BETWEEN:
    return NO;

  case B_DEAD:
    tt->markDeadThread();
    checkExtThread(tt);
    return OK;

  default:
    Assert(0);
    return NO;
  }
}

//
//  Generic 'wakeUp';
//  Since this method is used at the only one place, it's inlined;
inline
Bool AM::wakeUp(Thread *tt, Board *home, PropCaller calledBy) {
  switch (tt->getThrType()) {
  case S_RTHREAD:
    return wakeUpThread(tt,home);
  case S_WAKEUP:
    return wakeUpBoard(tt,home);
  case S_PR_THR:
    return wakeUpPropagator(tt, home, calledBy);
  default:
    Assert(0);
    return FALSE;
  }
  return FALSE;         // just to keep gcc happy;
}

// val is used because it may be a variable which must suspend.
//  if det X then ... fi
//  X = Y
// --> if det Y then ... fi

SuspList * AM::checkSuspensionList(SVariable * var,
                                   SuspList * suspList,
                                   PropCaller calledBy)
{
  if (shallowHeapTop!=0)
    return suspList;

  SuspList * retSuspList = NULL;

  // see the reduction of solve actor by the enumeration;
  DebugCheck(dontPropagate == OK, return (suspList));

  PROFILE_CODE1(FDProfiles.inc_item(no_calls_checksusplist);)

  while (suspList) {
    PROFILE_CODE1(FDProfiles.inc_item(susps_per_checksusplist);)

    Thread *thr = suspList->getElem();

    if (thr->isDeadThread ()) {
      suspList = suspList->dispose();
      continue;
    }

PROFILE_CODE1
  (
   if (GETBOARD(var) == currentBoard) {
     if (GETBOARD(thr) == currentBoard)
       FDProfiles.inc_item(from_home_to_home_hits);
     else
       FDProfiles.inc_item(from_home_to_deep_hits);
   } else {
     Board * b = GETBOARD(thr);
     if (b == GETBOARD(var))
       FDProfiles.inc_item(from_deep_to_home_misses);
     else if (isBetween(b, GETBOARD(var))==B_BETWEEN)
       FDProfiles.inc_item(from_deep_to_deep_hits);
     else
       FDProfiles.inc_item(from_deep_to_deep_misses);
   }
   )

 // already runnable susps remain in suspList
    if (thr->isRunnable()) {
      if (thr->isPropagator()) {
        if (calledBy && !thr->isUnifyThread()) {
          switch (isBetween(GETBOARD(thr), GETBOARD(var))) {
          case B_BETWEEN:
            thr->markUnifyThread ();
            break;
          case B_DEAD:
            //  keep the thread itself alive - it will be discarded
            // *properly* in the emulator;
            suspList = suspList->dispose ();
            continue;
          case B_NOT_BETWEEN:
            break;
          }
        }
      } else {
        //  non-propagator, i.e. it just goes away;
        suspList = suspList->dispose();
        continue;
      }
    } else {
      if (wakeUp(thr, GETBOARD(var), calledBy)) {
        Assert (thr->isDeadThread () || thr->isRunnable ());
        suspList = suspList->dispose ();
        continue;
      }
    }

    // susp cannot be woken up therefore relink it
    SuspList * first = suspList;
    suspList = suspList->getNext();
    first->setNext(retSuspList);
    retSuspList = first;
  } // while

  return retSuspList;
}


Board *varHome(TaggedRef val) {
  if (isUVar(val)) {
    return tagged2VarHome(val);
  } else if (isSVar(val)) {
    return GETBOARD(tagged2SVar(val));
  } else {
    return GETBOARD(taggedCVar2SVar(val));
  }
}

Bool checkHome(TaggedRef *vPtr) {
  TaggedRef val = deref(*vPtr);

  return !isAnyVar(val) ||
    am.isBelow(am.currentBoard,varHome(val));
}


// exception from general rule that arguments are never variables!
//  term may be an
void AM::genericBind(TaggedRef *varPtr, TaggedRef var,
                     TaggedRef *termPtr, TaggedRef term)
     /* bind var to term;         */
{
  Assert(!isCVar(var) && !isRef(term));

  /* first step: do suspension */
  if (isSVar(var)) {
    checkSuspensionList(var, pc_std_unif);
  }

  /* second step: mark binding for non-local variable in trail;     */
  /* also mark such (i.e. this) variable in suspention list;        */
  if ( !isLocalVariable(var,varPtr)) {
    Assert(shallowHeapTop || checkHome(varPtr));
    trail.pushRef(varPtr,var);
  } else  { // isLocalVariable(var)
    if (isSVar(var)) {
      tagged2SVar(var)->dispose();
    }
  }

  doBind(varPtr,isAnyVar(term) ? makeTaggedRef(termPtr) : term);
}


void AM::doBindAndTrail(TaggedRef v, TaggedRef * vp, TaggedRef t)
{
  Assert(shallowHeapTop || checkHome(vp));
  trail.pushRef(vp, v);

  CHECK_NONVAR(t);
  *vp = t;

  Assert(isRef(*vp) || !isAnyVar(*vp));
}

/*
 * ... and install propagators
 */
void AM::doBindAndTrailAndIP(TaggedRef v, TaggedRef * vp, TaggedRef t,
                             GenCVariable * lv, GenCVariable * gv)
{
  lv->installPropagators(gv);
  Assert(shallowHeapTop || checkHome(vp));
  trail.pushRef(vp, v);

  CHECK_NONVAR(t);
  *vp = t;

  Assert(isRef(*vp) || !isAnyVar(*vp));
}

/*
 *
 *  Install every board from the currentBoard to 'n'
 * and move cursor to 'n'
 *
 *  Algorithm:
 *   find common parent board of 'to' and 'currentBoard'
 *   deinstall until common parent (go upward)
 *   install (go downward)
 *
 *  Pre-conditions:
 *  - 'to' ist not deref'd;
 *  - 'to' may be committed, failed or discarded;
 *
 *  Return values and post-conditions:
 *  - INST_OK:
 *      installation successful, currentBoard == 'to';
 *  - INST_FAILED:
 *      installation of *some* board on the "down" path has failed,
 *      'am.currentBoard' points to that board;
 *  - INST_REJECTED:
 *      *some* board on the "down" path is already failed or discarded,
 *      'am.currentBoard' stays unchanged;
 *
 */
InstType AM::installPath(Board *to)
{
  if (to->isInstalled()) {
    deinstallPath(to);
    return INST_OK;
  }

  Assert(to != rootBoard);

  Board *par=to->getParentAndTest();
  if (!par) {
    return INST_REJECTED;
  }

  InstType ret = installPath(par);
  if (ret != INST_OK) {
    return ret;
  }

  setCurrent(to);
  to->setInstalled();

  trail.pushMark();
  if (!installScript(to->getScriptRef())) {
    return INST_FAILED;
  }
  return INST_OK;
}

// only used in deinstall
// Three cases may occur:
// any global var G -> ground ==> add susp to G
// any global var G -> constrained local var ==> add susp to G
// unconstrained global var G1 -> unconstrained global var G2
//    ==> add susp to G1 and G2

void AM::reduceTrailOnSuspend()
{
  if (!trail.isEmptyChunk()) {
    int numbOfCons = trail.chunkSize();
    Board * bb = currentBoard;
    bb->newScript(numbOfCons);

    //
    // one single suspended thread for all;
    Thread *thr = mkWakeupThread(bb);

    for (int index = 0; index < numbOfCons; index++) {
      TaggedRef * refPtr, value;

      trail.popRef(refPtr, value);

      Assert(isRef(*refPtr) || !isAnyVar(*refPtr));
      Assert(isAnyVar(value));

      bb->setScript(index,refPtr,*refPtr);

      TaggedRef vv= *refPtr;
      DEREF(vv,vvPtr,_vvTag);
      if (isAnyVar(vv)) {
        addSuspAnyVar(vvPtr,thr,NO);  // !!! Makes space *not* unstable !!!
      }

      unBind(refPtr, value);

      // value is always global variable, so add always a thread;
      addSuspAnyVar(refPtr,thr);

    } // for
  } // if
  trail.popMark();
}

void AM::reduceTrailOnFail()
{
  while(!trail.isEmptyChunk()) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);
    unBind(refPtr,value);
  }
  trail.popMark();
}

inline void mkSusp(TaggedRef *ptr, Thread *t)
{
  if (t) {
    addSuspAnyVar(ptr,t);
  } else {
    am.addSuspendVarList(ptr);
  }
}

/*
 * shallow guards sometimes do not bind variables but only push them
 * if thread is NULL then we are called from '==':
 * return the list of variable in am.suspendVarList otherwise
 * add "thread" to susplist
 */
void AM::reduceTrailOnShallow(Thread *thread)
{
  suspendVarList = makeTaggedNULL();

  while(!trail.isEmptyChunk()) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);

    Assert(isAnyVar(value));

    TaggedRef oldVal = makeTaggedRef(refPtr);
    DEREF(oldVal,ptrOldVal,_1);

    unBind(refPtr,value);

    /* test if only trailed to create thread and not bound ? */
    if (refPtr!=ptrOldVal) {
      if (isAnyVar(oldVal)) {
        mkSusp(ptrOldVal,thread);
      }
    }

    mkSusp(refPtr,thread);
  }
  trail.popMark();
}

/* -------------------------------------------------------------------------
 * OFS
 * -------------------------------------------------------------------------*/
// Check if there exists an S_ofs (Open Feature Structure) suspension
// in the suspList (Used only for monitorArity)
Bool AM::hasOFSSuspension(SuspList *suspList)
{
  while (suspList) {
    Thread *thr = suspList->getElem ();
    if (!thr->isDeadThread () &&
        thr->isPropagator() && thr->isOFSThread ()) return TRUE;
    suspList = suspList->getNext();
  }
  return FALSE;
}


/* Add list of features to each OFS-marked suspension list 'flist' has
 * three possible values: a single feature (literal or integer), a
 * nonempty list of features, or NULL (no extra features).
 * 'determined'==TRUE iff the unify makes the OFS determined.  'var'
 * (which must be deref'ed) is used to make sure that features are
 * added only to variables that are indeed waiting for features. This
 * routine is inspired by am.checkSuspensionList, and must track all
 * changes to it.  */
void AM::addFeatOFSSuspensionList(TaggedRef var,
                                  SuspList* suspList,
                                  TaggedRef flist,
                                  Bool determ)
{
  while (suspList) {
    Thread *thr = suspList->getElem ();

    // The added condition ' || thr->isRunnable () ' is incorrect
    // since isPropagated means only that the thread is runnable
    if (thr->isDeadThread ()) {
      suspList=suspList->getNext();
      continue;
    }

    if (thr->isPropagator() && thr->isOFSThread ()) {
      MonitorArityPropagator *prop =
        (MonitorArityPropagator *) thr->getPropagator();

      Assert(sizeof(MonitorArityPropagator)==prop->sizeOf());

      // Only add features if var and fvar are the same:
      TaggedRef fvar=prop->getX();
      DEREF(fvar,_1,_2);
      if (var!=fvar) {
        suspList=suspList->getNext();
        continue;
      }
      // Only add features if the 'kill' variable is undetermined:
      TaggedRef killl=prop->getK();
      DEREF(killl,_,killTag);
      if (!isAnyVar(killTag)) {
        suspList=suspList->getNext();
        continue;
      }

      // Add the feature or list to the diff. list in FH and FT:
      if (flist) {
        if (isFeature(flist))
          prop->setFH(cons(flist,prop->getFH()));
        else {
          // flist must be a list
          Assert(isLTuple(flist));
          TaggedRef tmplist=flist;
          while (tmplist!=AtomNil) {
            prop->setFH(cons(head(tmplist),prop->getFH()));
            tmplist=tail(tmplist);
          }
        }
      }
      if (determ) {
        // FS is det.: tail of list must be bound to nil: (always succeeds)
        // Do *not* use unification to do this binding!
        TaggedRef tl=prop->getFT();
        DEREF(tl,tailPtr,tailTag);
        switch (tailTag) {
        case LITERAL:
          Assert(tl==AtomNil);
          break;
        case UVAR:
          doBind(tailPtr, AtomNil);
          break;
        default:
          Assert(FALSE);
        }
      }
    }

    suspList = suspList->getNext();
  }
}

/* -------------------------------------------------------------------------
 * MISC
 * -------------------------------------------------------------------------*/

int AM::awakeIO(int, void *var) {
  am.awakeIOVar((TaggedRef) var);
  return 1;
}

void AM::awakeIOVar(TaggedRef var)
{
  Assert(isToplevel());
  Assert(isCons(var));

  if (OZ_unify(OZ_head(var),OZ_tail(var)) != PROCEED) {
    warning("select or sleep failed");
  }
}

#ifdef DEBUG_CHECK
static Board *oldBoard = (Board *) NULL;
static Board *oldSolveBoard = (Board *) NULL;
#endif

void AM::setCurrent(Board *c, Bool checkNotGC)
{
  Assert(!c->isCommitted() && !c->isFailed());
  Assert(!checkNotGC || oldBoard == currentBoard);

  currentBoard = c;
  _currentUVarPrototype = makeTaggedUVar(c);
  DebugCheckT(oldBoard=c);

  if (c->isSolve ()) {
    Assert(!checkNotGC || oldSolveBoard == currentSolveBoard);

    currentSolveBoard = c;
    wasSolveSet = OK;
    DebugCode (oldSolveBoard = c);
  } else if (wasSolveSet == OK) {
    Assert(!checkNotGC || oldSolveBoard == currentSolveBoard);

    currentSolveBoard = c->getSolveBoard();
    wasSolveSet = NO;
    DebugCode (oldSolveBoard = currentSolveBoard);
  }
}

Bool AM::loadQuery(CompStream *fd)
{
  unsigned int starttime = osUserTime();

  ProgramCounter pc;

  Bool ret = CodeArea::load(fd,pc);

  if (ret == OK && pc != NOCODE) {
    addToplevel(pc);
  }

  ozstat.timeForLoading.incf(osUserTime()-starttime);

  return ret;
}


void AM::select(int fd, int mode, OZ_IOHandler fun, void *val)
{
  Assert(fd<osOpenMax());
  if (!isToplevel()) {
    warning("select only on toplevel");
    return;
  }
  ioNodes[fd].readwritepair[mode]=val;
  ioNodes[fd].handler[mode]=fun;
  osWatchFD(fd,mode);
}


void AM::acceptSelect(int fd, OZ_IOHandler fun, void *val)
{
  if (!isToplevel()) {
    warning("select only on toplevel");
    return;
  }

  ioNodes[fd].readwritepair[SEL_READ]=val;
  ioNodes[fd].handler[SEL_READ]=fun;
  osWatchAccept(fd);
}

int AM::select(int fd, int mode,TaggedRef l,TaggedRef r)
{
  if (!isToplevel()) {
    warning("select only on toplevel");
    return OK;
  }
  if (osTestSelect(fd,mode)==1) return OZ_unify(l,r);
  ioNodes[fd].readwritepair[mode]=(void *) cons(l,r);
  gcProtect((TaggedRef *) &ioNodes[fd].readwritepair[mode]);

  ioNodes[fd].handler[mode]=awakeIO;
  osWatchFD(fd,mode);
  return OK;
}

void AM::acceptSelect(int fd,TaggedRef l,TaggedRef r)
{
  if (!isToplevel()) {
    warning("acceptSelect only on toplevel");
    return;
  }

  ioNodes[fd].readwritepair[SEL_READ]=(void *) cons(l,r);
  gcProtect((TaggedRef *) &ioNodes[fd].readwritepair[SEL_READ]);

  ioNodes[fd].handler[SEL_READ]=awakeIO;
  osWatchAccept(fd);
}

void AM::deSelect(int fd)
{
  deSelect(fd,SEL_READ);
  deSelect(fd,SEL_WRITE);
}

void AM::deSelect(int fd,int mode)
{
  Assert(fd<osOpenMax());
  osClrWatchedFD(fd,mode);
  ioNodes[fd].readwritepair[mode]  = 0;
  (void) gcUnprotect((TaggedRef *) &ioNodes[fd].readwritepair[mode]);
  ioNodes[fd].handler[mode]  = 0;
}

// called if IOReady (signals are blocked)
void AM::handleIO()
{
  unsetSFlag(IOReady);
  int numbOfFDs = osFirstSelect();

  /* check input from compiler */
  if (compStream) {
    if (osNextSelect(compStream->csfileno(),SEL_READ) || /* do this FIRST, sideeffect! */
        !compStream->bufEmpty()) {
      do {
        loadQuery(compStream);
      } while(!compStream->bufEmpty());
      numbOfFDs--;
    }
  }

  // find the nodes to awake
  for (int index = 0; numbOfFDs > 0; index++) {
    for(int mode=SEL_READ; mode <= SEL_WRITE; mode++) {

      if (osNextSelect(index, mode) ) {

        numbOfFDs--;

        Assert(ioNodes[index].handler[mode]);
        if ((ioNodes[index].handler[mode])
            (index, ioNodes[index].readwritepair[mode])) {
          ioNodes[index].readwritepair[mode] = 0;
          (void) gcUnprotect((TaggedRef *)&ioNodes[index].readwritepair[mode]);
          ioNodes[index].handler[mode] = 0;
          osClrWatchedFD(index,mode);
        }
      }
    }
  }
}

void checkIO(){
  AM &e=am;
  if(e.isSetSFlag(IOReady)){
    osBlockSignals();
    e.handleIO();
    osUnblockSignals();}
}


// called from signal handler
void AM::checkIO()
{
  int numbOfFDs = osCheckIO();
  if (!isCritical() && (numbOfFDs > 0
                        || (compStream && !compStream->bufEmpty()))) {
    setSFlag(IOReady);
  }
}

/* -------------------------------------------------------------------------
 * Search
 * -------------------------------------------------------------------------*/

/*
 * increment/decrement the thread counter
 * in every solve board above
 * if "stable" generate a new thread "solve waker"
 * NOTE:
 *   there may be failed board in between which must be simply ignored
 *
 * kost@ to mm2:
 *   Michael, i don't think that it was a good idea to merge
 * these things together - this leads to efficiency penalties,
 * and less possibilities to make an assertion.
 *
 * RETURNS: OK if solveSpace found, else NO
 */

int AM::incSolveThreads(Board *bb)
{
  int ret = NO;
  while (!bb->isRoot()) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      ret = OK;
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      //
      Assert (!sa->isCommitted());

      //
      sa->incThreads ();

      //
      Assert (!(isStableSolve (sa)));
    }
    bb = bb->getParent();
  }
  return ret;
}

void AM::decSolveThreads(Board *bb)
{
  while (!bb->isRoot()) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());

      //
      // local optimization - check for threads first;
      if (sa->decThreads () == 0) {
        //
        // ... first - notification board below the failed solve board;
        if (!(sa->isCommitted ()) && isStableSolve (sa)) {
          scheduleThread(mkRunnableThread(DEFAULT_PRIORITY,bb));
        }
      } else {
        Assert (sa->getThreads () > 0);
      }
    }
    bb = bb->getParent();
  }
}

#ifdef DEBUG_CHECK
/*
 *  Just check whether the 'bb' is located beneath some (possibly dead)
 * solve board;
 */
Bool AM::isInSolveDebug (Board *bb)
{
  while (!bb->isRoot()) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      if (!sa->isCommitted()) {
        return (OK);
      }
    }
    bb = bb->getParent();
  }

  return (NO);
}
#endif

Bool AM::isStableSolve(SolveActor *sa)
{
  if (sa->getThreads() != 0)
    return NO;
  if (sa->getSolveBoard() == currentBoard &&
      !trail.isEmptyChunk())
    return NO;
  // simply "don't worry" if in all other cases it is too weak;
  return sa->areNoExtSuspensions();
}

/* ------------------------------------------------------------------------
 * Threads
 *
 * running thread: currentThread
 * toplevel: rootThread and toplevelQueue
 * ------------------------------------------------------------------------ */


/*
 * Toplevel is a queue of toplevel queries, which must be executed
 * sequentially.
 *   see checkToplevel()  called when disposing a thread
 *       addToplevel()    called when a new query arrives
 */
class Toplevel {
public:
  ProgramCounter pc;
  Toplevel *next;
  Toplevel(ProgramCounter p, Toplevel *nxt) : pc(p), next(nxt) {}
};

/*
 * push a new toplevel continuation to the rootThread
 * PRE: the rootThread is empty
 * called from checkToplevel and addToplevel
 */
void AM::pushToplevel(ProgramCounter pc)
{
  Assert(rootThread->isEmpty());
  // kost@ : MOD!!! TODO?
  // rootBoard->incSuspCount();
  rootThread->getTaskStackRef()->pushCont(pc,toplevelVars,NULL);
  if (rootThread!=currentThread && !isScheduledSlow(rootThread)) {
    Assert(rootThread->isRunnable());
    scheduleThread(rootThread);
  }
}


/*
 * in dispose: check if there are more toplevel tasks
 */
void AM::checkToplevel()
{
  if (toplevelQueue) {
    // Verbose((VERB_THREAD,"checkToplevel: pushNext: 0x%x\n",this));
    Toplevel **last;
    for (last = &toplevelQueue;
         ((*last)->next) != NULL;
         last = &((*last)->next)) {
    }
    pushToplevel((*last)->pc);
    *last = (Toplevel *) NULL;
  }
}

void AM::addToplevel(ProgramCounter pc)
{
  if (rootThread->isEmpty() && rootThread->isRunnable()) {
    // Verbose((VERB_THREAD,"addToplevel: push\n"));
    pushToplevel(pc);
  } else {
    // Verbose((VERB_THREAD,"addToplevel: delay\n"));
    toplevelQueue = new Toplevel(pc,toplevelQueue);
  }
}

/* -------------------------------------------------------------------------
 * Signals
 * -------------------------------------------------------------------------*/

void handlerUSR1()
{
  message("Error handler entered ****\n");

  CodeArea::writeInstr();

#ifdef MM_DEBUG
  tracerOn(); trace("halt");
#endif
  message("Error handler exit ****\n");
}

void handlerINT()
{
  message("SIG INT ****\n");
  am.exitOz(1);
}

void handlerTERM()
{
  message("SIG TERM ****\n");
  am.exitOz(0);
}

void handlerMessage()
{
  message("SIGNAL inside signal handler ***\n");
}

void handlerSEGV()
{
  CodeArea::writeInstr();
  message("**** segmentation violation ****\n");
  longjmp(am.engineEnvironment,SEGVIO);
}

void handlerBUS()
{
  CodeArea::writeInstr();
  message("**** bus error ****\n");
  longjmp(am.engineEnvironment,BUSERROR);
}

void handlerPIPE()
{
  message("write on a pipe or other socket with no one to read it ****\n");
}

void handlerCHLD()
{
  DebugCheckT(message("a child process' state changed ****\n"));
}

void handlerFPE()
{
  OZ_warning("signal: floating point exception");
}

// signal handler
void handlerALRM()
{
  am.handleAlarm();
}

/* -------------------------------------------------------------------------
 * Alarm handling
 * -------------------------------------------------------------------------*/

void AM::handleAlarm()
{
  if (ozstat.currPropagator) {
    ozstat.currPropagator->incSamples();
  } else if (ozstat.currAbstr) {
    ozstat.currAbstr->samples++;
  }

  if (isCritical()) {  /* wait for next ALRM signal */
    return;
  }

  if (threadSwitchCounter > 0) {
    if (--threadSwitchCounter == 0) {
      setSFlag(ThreadSwitch);
    }
  }

  /* load userCounter into a local variable: under win32 this
   * code is executed concurrently with other code manipulating
   * the userCounter
   */
  int uc = userCounter;
  Assert(uc>=0);
  if (uc > 0) {
    if (--uc == 0) {
      setSFlag(UserAlarm);
    }
    setUserAlarmTimer(uc);
  }

  checkGC();

  checkIO();
}

/* handleUserAlarm:
    if UserAlarm-SFLAG is set this method is called
    interrupts should already be disabled by the parent procedure
    */
void AM::handleUser()
{
  unsetSFlag(UserAlarm);
  int nextMS = wakeUser();
  setUserAlarmTimer(nextMS);
}

class OzSleep {
public:
  OzSleep *next;
  int time;    // in clock ticks
  TaggedRef node;
public:
  OzSleep(int t, TaggedRef n,OzSleep *a)
  : time(t), node(n), next(a)
  {
    OZ_protect(&node);
    Assert(t>=0);
  }
  ~OzSleep() {
    OZ_unprotect(&node);
  }
};

void AM::insertUser(int ms, TaggedRef node)
{
  int t = osMsToClockTick(ms);

  osBlockSignals();

  if (sleepQueue) {
    int rest = getUserAlarmTimer();
    Assert(isSetSFlag(UserAlarm) || rest > 0);

    if (rest <= t) {
      setUserAlarmTimer(rest);
      t = t-rest;
    } else {
      setUserAlarmTimer(t);
      sleepQueue->time = rest - t;
      t = 0;
    }
  } else {
    setUserAlarmTimer(t);
    t = 0;
  }

  osUnblockSignals();

  OzSleep **prev = &sleepQueue;
  for (OzSleep *a = *prev; a; prev = &a->next, a=a->next) {
    if (t <= a->time) {
      a->time = a->time - t;
      *prev = new OzSleep(t,node,a);
      return;
    }
  }
  *prev = new OzSleep(t,node,NULL);
}

int AM::wakeUser()
{
  if (!sleepQueue) {
    OZ_warning("wake: nobody to wake up");
    return 0;
  }
  while (sleepQueue && sleepQueue->time==0) {
    awakeIOVar(sleepQueue->node);
    OzSleep *tmp = sleepQueue->next;
    delete sleepQueue;
    sleepQueue = tmp;
  }
  if (sleepQueue) {
    int ret = sleepQueue->time;
    sleepQueue->time = 0;
    return ret;
  }
  return 0;
}



OZ_Term AM::dbgGetLoc(Board *bb) {
  OZ_Term out = nil();
  while (!bb->isRoot()) {
    if (bb->isSolve()) {
      out = cons(OZ_atom("space"),out);
    } else if (bb->isAsk()) {
      out = cons(OZ_atom("cond"),out);
    } else if (bb->isWait()) {
      out = cons(OZ_atom("dis"),out);
    } else {
      out = cons(OZ_atom("???"),out);
    }
    bb=bb->getParent();
  }
  return out;
}


void AM::checkDebugOutline(Thread *tt)
{
  Assert(debugmode());
  if (currentThread && tt->getThrType() == S_RTHREAD)
    if (currentThread == rootThread && !suspendDebug ||
        currentThread->isTraced() && !runChildren) {

      debugStreamThread(tt,currentThread);

      tt->traced();
      tt->startStepMode();
      tt->stop();
    }
}

//  Make a runnable thread with a single task stack entry <local thread queue>
Thread *AM::mkLTQ(Board *bb, int prio, SolveActor * sa)
{
  Thread *th = new Thread(S_RTHREAD | T_runnable | T_ltq,prio,bb,newId());
  th->setBody(allocateBody());
  bb->incSuspCount();
  checkDebug(th);
  Assert(bb == currentBoard);
  Assert(isInSolveDebug(bb));

  incSolveThreads(bb);
  th->setInSolve();

  th->pushLTQ(sa);

  return th;
}

// mm2: stop/resume has to be overhauled
void AM::stopThread(Thread *th)
{
  if (th->pStop()==0) {
    if (th==currentThread) {
      setSFlag(StopThread);
    }
    th->stop();
    if (th->isRunnable())
      th->unmarkRunnable();
  }
}

void AM::resumeThread(Thread *th)
{
  if (th->pCont()==0) {
    th->cont();

    if (!th->isDeadThread()) {
      if (th == currentThread) {
        Assert(isSetSFlag(StopThread));
        unsetSFlag(StopThread);
      } else {
        suspThreadToRunnable(th);
        if (!isScheduledSlow(th))
          scheduleThread(th);
      }
    }
  }
}


Board *rootBoard() { return am.rootBoard; }

int AM::commit(Board *bb, Thread *tt)
{
  Assert(!currentBoard->isCommitted());
  Assert(bb->getParent()==currentBoard);

  AWActor *aw = AWActor::Cast(bb->getActor());

  Assert(!tt || tt==aw->getThread());

  Continuation *cont=bb->getBodyPtr();

  bb->setCommitted(currentBoard);
  currentBoard->incSuspCount(bb->getSuspCount()-1);

  if (bb->isWait()) {
    Assert(bb->isWaiting());

    WaitActor *wa = WaitActor::Cast(aw);

    if (currentBoard->isWait()) {
      WaitActor::Cast(currentBoard->getActor())->mergeChoices(wa->getCpb());
    } else if (currentBoard->isSolve()) {
      SolveActor::Cast(currentBoard->getActor())->mergeChoices(wa->getCpb());
    } else {
      // forget the choice stack when committing to a conditional
    }

    if (!installScriptOutline(bb->getScriptRef())) {
      return 0;
    }
  }

  if (!tt) {
    tt=aw->getThread();
    Assert(tt->isSuspended());
    suspThreadToRunnable(tt);
    scheduleThread(tt);
    DebugCheckT(aw->setThread(0));
  }

  TaskStack *ts = tt->getTaskStackRef();
  ts->discardActor();
  if (aw->isAsk()) {
    AskActor::Cast(aw)->disposeAsk();
  } else {
    WaitActor::Cast(aw)->disposeWait();
  }

  ts->pushCont(cont->getPC(),cont->getY(),cont->getG());
  if (cont->getX()) ts->pushX(cont->getX());

  return 1;
}

// see variable.hh
void checkExtThread(Thread *elem, Board *home)
{
  if (am.currentSolveBoard) {
    am.setExtThreadOutlined(elem,home->derefBoard());
  }
}

void AM::setExtThreadOutlined(Thread *tt, Board *varHome)
{
  Board *bb = currentBoard;
  Bool wasFound = NO;
  Assert (!varHome->isCommitted());

  while (bb != varHome) {
    Assert (!bb->isRoot());
    Assert (!bb->isCommitted() && !bb->isFailed());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      sa->addSuspension(tt);
      wasFound = OK;
    }
    bb = bb->getParent();
  }

  if (wasFound) tt->setExtThread();
}

void AM::checkExtThreadOutlined(Thread *tt)
{
  Assert(tt->wasExtThread());

  Board *sb = GETBOARD(tt)->getSolveBoard();

  while (sb) {
    Assert(sb->isSolve());

    SolveActor *sa = SolveActor::Cast(sb->getActor());
    if (isStableSolve(sa)) {
      scheduleThread(mkRunnableThreadOPT(DEFAULT_PRIORITY,sb));
    }
    sb = GETBOARD(sa)->getSolveBoard();
  }
}

void AM::removeExtThreadOutlined(Thread *tt)
{
  Assert(tt->wasExtThread());

  Board *sb = GETBOARD(tt)->getSolveBoard ();

  while (sb) {
    Assert (sb->isSolve());

    SolveActor *sa = SolveActor::Cast(sb->getActor());
    sa->clearSuspList(tt);
    sb = GETBOARD(sa)->getSolveBoard();
  }
}


SuspList *AM::installPropagators(SuspList * local_list, SuspList * glob_list,
                                 Board * glob_home)
{
  Assert((local_list && glob_list && (local_list != glob_list)) ||
         !local_list || !glob_list);

  SuspList * aux = local_list, * ret_list = local_list;


  // mark up local suspensions to avoid copying them
  while (aux) {
    aux->getElem()->markTagged();
    aux = aux->getNext();
  }

  // create references to suspensions of global variable
  aux = glob_list;
  while (aux) {
    Thread *thr = aux->getElem();

    if (!thr->isDeadThread() &&
        thr->isPropagator() &&
        !thr->isTagged() && /* TMUELLER possible optimization
                                  isTaggedAndUntag */
        isBetween(GETBOARD(thr), glob_home) == B_BETWEEN) {
      ret_list = new SuspList (thr, ret_list);
    }

    aux = aux->getNext();
  }

  // unmark local suspensions
  aux = local_list;
  while (aux) {
    aux->getElem()->unmarkTagged();
    aux = aux->getNext();
  }

  return ret_list;
}

#ifdef OUTLINE
#define inline
#include "am.icc"
#undef inline
#endif
