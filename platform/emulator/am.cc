/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "am.hh"

#ifdef OUTLINE
#define inline
#include "am.icc"
#undef inline
#endif

#include "os.hh"
#include "board.hh"
#include "threadInterface.hh"
#include "allgenvar.hh"
#include "codearea.hh"
#include "fdomn.hh"
#include "extension.hh"

AM am;

/* -------------------------------------------------------------------------
 * Init and exit AM
 * -------------------------------------------------------------------------*/

static
void usage(int /* argc */,char **argv) {
  fprintf(stderr,
	  "usage: %s <options>\n",
	  argv[0]);
  fprintf(stderr, " -copyallways : ignore copy once flag\n");
  fprintf(stderr, " -d           : debugging on\n");
  fprintf(stderr, " -init <file> : load and execute init procedure\n");
  fprintf(stderr, " -u <url>     : start a compute server\n");
  fprintf(stderr, " -x <hex>     : start as a virtual site\n");
  fprintf(stderr, " -b <file>    : boot from assembly code\n");
  fprintf(stderr, " -- <args> ...: application arguments\n");
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
void printBanner(char*initFile)
{
#ifdef NO_LTQ
  warning("LTQ is turned off.");
#endif

#ifdef DEBUG_CHECK
  fprintf(stderr,
	  "Compile Flags:"
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
#ifdef RECINSTRFETCH
	  " RECINSTRFETCH=%d", RECINSTRFETCH
#endif
	 );
  fprintf(stderr, "\n");
#endif

#ifdef PROFILE_EMULATOR
  fprintf(stderr, "Compiled to support gprof-profiling.\n");
#ifdef DEBUG_DET
  fprintf(stderr, "Deterministic scheduling.\n");
#else
  fprintf(stderr, "Time-slice scheduling.\n");
#endif
#endif

#ifdef THREADED
  // fprintf(stderr, "Using threaded code (abs jumps).\n");
#else
  fprintf(stderr, "Not using threaded code.\n");
#endif

}


#ifdef MODULES_LINK_STATIC
OZ_BI_proto(ozma_readProc);
#endif

extern void initBuiltins();
extern void bigIntInit(); /* from value.cc */
extern void initffuns();  /* from initffuns.cc */
extern void initVirtualProperties();

void AM::init(int argc,char **argv)
{  
  Assert(makeTaggedNULL() == 0);
  Assert(PROCEED && !FAILED);

  ozconf.init();
  ProfileCode(ozstat.initCount());
  osInit();
  bigIntInit();
  initffuns();

  installingScript = FALSE;

  defaultExceptionHdl     = makeTaggedNULL();

  preparedCalls = NULL;

  char *tmp;
  if ((tmp = getenv("OZPATH"))) {
    ozconf.ozPath = tmp;
  }

  ozconf.copyallways = getenv("OZCOPYALLWAYS") ? 1 : 0;

  char *url = NULL;
  char *initFile = getenv("OZINIT");
  char *assemblyCodeFile = NULL;
  
  /* process command line arguments */
  ozconf.argV = NULL;
  ozconf.argC = 0;

  /* enter emulator path */
  ozconf.emuhome = strdup(argv[0]);

  {
    char * last_slash = 0;
    char * c = ozconf.emuhome;

    while (*c) {
      if (*c == '/')
	last_slash = c;
      c++;
    }
    
    if (last_slash) 
      *last_slash = 0;
  }

#ifdef PICKLE2TEXTHACK
    int p2t = 0;
#endif

  for (int i=url?2:1; i<argc; i++) {
#ifdef PICKLE2TEXTHACK
    if (strcmp(argv[i],"--pickle2text")==0) {
      p2t = 1;
      break;
    }
#endif

    if (strcmp(argv[i],"-d")==0) {
#ifdef DEBUG_TRACE
      ozd_tracerOn();
#endif
      continue;
    }
    if (strcmp(argv[i],"-copyallways")==0) {
      ozconf.copyallways = 1;
      continue;
    }

    if (strcmp(argv[i],"-u")==0) {
      url = getOptArg(i,argc,argv);
      ozconf.url = url;
      continue;
    }

    if (strcmp(argv[i],"-b")==0) {
      assemblyCodeFile = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-init")==0) {
      initFile = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"--")==0) {
      ozconf.argC = argc-i-1;
      ozconf.argV = argv+i+1;
      break;
    }

    fprintf(stderr,"Unknown option '%s'.\n",argv[i]);
    usage(argc,argv);
  }

  if (url && assemblyCodeFile) {
    fprintf(stderr,"Options '-u' and '-b' are mutually exclusive.\n");
    usage(argc,argv);
  }

  if (initFile && *initFile && assemblyCodeFile) {
    fprintf(stderr,"no init file allowed with assembly code file.\n");
    usage(argc,argv);
  }

#ifdef DEBUG_CHECK
  ozconf.showIdleMessage=1;
#endif

  if (!p2t && !initFile && !assemblyCodeFile) {
    char* ini = "/share/Init.ozf";
    int m = strlen(ozconf.ozHome);
    int n = m+strlen(ini)+1;
    char*s = new char[n];
    strcpy(s,ozconf.ozHome);
    strcpy(s+m,ini);
    if (access(s,F_OK)==0) initFile = s;
    else delete[] s;
  }
  if (initFile && *initFile=='\0') initFile=0;

  if (!p2t && !initFile && !assemblyCodeFile) {
    fprintf(stderr,"neither init file nor assembly code.\n");
    usage(argc,argv);
  }

  printBanner(initFile);
  if (getenv("OZ_TRACE_LOAD"))
    if (initFile)
      fprintf(stderr,"Init file: %s\n",initFile);
    else
      fprintf(stderr,"No init file\n");

  (void) engine(OK);

  initFDs();
  
  initMemoryManagement();

// not changeable
  // SizeOfWorkingArea,NumberOfXRegisters,NumberOfYRegisters


// internal registers
  statusReg    = (StatusBit)0;
  criticalFlag = NO;

  _rootBoard = new Board(NULL,Bo_Root);
  _rootBoard->setInstalled();
  _currentBoard = NULL;
  cachedStack  = NULL;
  cachedSelf   = NULL;
  setShallowHeapTop(NULL);
  setCurrent(_rootBoard,OK);
  _currentSolveBoard = (Board *) NULL; 
  wasSolveSet = NO; 

  lastThreadID    = 0;
  debugMode       = NO;
  debugStreamTail = OZ_newVariable();

  threadsPool.initThreads();

  // builtins
  initLiterals();

  initBuiltins();
  
#ifdef WINDOWS
  extern initWinSockets();
  initWinSockets();
#endif

  initVirtualProperties();

  extern void initTagged();
  initTagged();

  emptySuspendVarList(); // must be after initLiterals

  //
  taskNodes = new TaskNode[MAXTASKS];

  //
  osInitSignals();
  osSetAlarmTimer(CLOCK_TICK/1000);

  //
  if (!perdioInit()) {
    warning("Perdio initialization failed");
  }

#ifdef PICKLE2TEXTHACK
  if (p2t) {
    extern int pickle2text();
    Bool aux = pickle2text();
    exit(aux ? 0 : 1);
  }
#endif

  Thread *tt = oz_newThread();

  if (assemblyCodeFile) {

    OZ_CFun f = 0;

#ifndef MODULES_LINK_STATIC

    char * libname = "/ozma.so";
    int n = strlen(ozconf.emuhome);
    char * libfile = new char[n + strlen(libname)];

    strcpy(libfile, ozconf.emuhome);

    strcpy(libfile + n, libname);

    printf("Loading ozma library: %s\n",libfile);

    OZ_Term out;
    OZ_Term ret = osDlopen(libfile, out);

    if (ret) {
      fprintf(stderr, "Cannot open ozma library: %s.\n",toC(ret));
      osExit(1);
    }

    delete[] libfile;

    void* handle = OZ_getForeignPointer(out);

    f = (OZ_CFun)  osDlsym(handle,"ozma_readProc");

    if (!f) {
      fprintf(stderr,"builtin ozma_readProc not found");
      osExit(1);
    }
      
#else

    printf("Ozma library statically linked\n");

    f = ozma_readProc;
#endif

    OZ_Term args[2] = { oz_atom(assemblyCodeFile),0 };
    OZ_Return r=(*f)(args,0);
    if (r!=PROCEED) {
      fprintf(stderr,"assembling %s failed",assemblyCodeFile);
      osExit(1);
    }
    tt->pushCall(args[1], 0, 0);
  }

  if (initFile) {
    TaggedRef functor   = oz_newVariable();
    TaggedRef procedure = oz_newVariable();
    TaggedRef export    = oz_newVariable();

    // Construct import for functor:
    TaggedRef boot_module = 
      OZ_recordInit(AtomExport,
		    oz_cons(oz_pair2(AtomManager,BI_boot_manager),oz_nil()));
    TaggedRef boot_import = 
      OZ_recordInit(AtomExport,
		    oz_cons(oz_pair2(AtomBoot,   boot_module),oz_nil()));

    // Task3: execute functor's code
    tt->pushCall(procedure,boot_import,export);

    // Task2: lookup functor's code
    tt->pushCall(BI_dot,functor,AtomApply,procedure);

    // Task1: load functor
    tt->pushCall(BI_load,oz_atom(initFile),functor);
  }

  //
  sleepQueue = (OzSleep *) 0;
  emulatorClock = 0;
  taskMinInterval = DEFAULT_MIN_INTERVAL;

  unsetProfileMode();
}

#ifdef VIRTUALSITES
//
// We have to reclaim the shared memory somehow;
extern void virtualSitesExit();
#endif

void AM::exitOz(int status)
{
#ifdef VIRTUALSITES
  virtualSitesExit();
#endif
  osExit(status);
}

/* -------------------------------------------------------------------------
 * Unification
 * -------------------------------------------------------------------------*/


Bool AM::isLocalUVarOutline(TaggedRef var, TaggedRef *varPtr)
{
  Board *bb=tagged2VarHome(var);
  if (bb->isCommitted()) { // mm2: is this needed?
    bb=bb->derefBoard();
    *varPtr=makeTaggedUVar(bb);
  }
  return oz_isCurrentBoard(bb);
}

Bool AM::isLocalSVarOutline(SVariable *var)
{
  Board *home = var->getHomeUpdate();
  return oz_isCurrentBoard(home);
}

inline
static 
Board *getVarBoard(TaggedRef var)
{
  CHECK_ISVAR(var);

  if (isUVar(var))
    return tagged2VarHome(var);
  return tagged2SVarPlus(var)->getHome1();
}  

inline
static
Bool isMoreLocal(TaggedRef var1, TaggedRef var2)
{
  Board *board1 = getVarBoard(var1)->derefBoard();
  Board *board2 = getVarBoard(var2)->derefBoard();
  return oz_isBelow(board1,board2);
}


static
int cmpCVar(GenCVariable *v1, GenCVariable *v2)
{
  TypeOfGenCVariable t1 = v1->getType();
  TypeOfGenCVariable t2 = v2->getType();
  return t1-t2;
}

// global vars!!!
static Stack unifyStack(100,Stack_WithMalloc);
static Stack rebindTrail(100,Stack_WithMalloc);

inline
static
void rebind(TaggedRef *refPtr, TaggedRef *ptr2)
{
  rebindTrail.ensureFree(2);
  rebindTrail.push(refPtr,NO);
  rebindTrail.push(ToPointer(*refPtr),NO);
  doBind(refPtr,makeTaggedRef(ptr2));
}

#define PopRebindTrail(value,refPtr)			\
    TaggedRef value   = ToInt32(rebindTrail.pop()); 	\
    TaggedRef *refPtr = (TaggedRef*) rebindTrail.pop(); 


OZ_Return oz_unify(TaggedRef t1, TaggedRef t2, ByteCode *scp)
{
  Assert(am.checkShallow(scp));
  Assert(unifyStack.isEmpty()); /* unify is not reentrant */
  CHECK_NONVAR(t1); CHECK_NONVAR(t2);

  OZ_Return result = FAILED;

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

  if (oz_isVariable(term1)) {
    if (oz_isVariable(term2)) {
      goto var_var;
    } else {
      goto var_nonvar; 
    }
  } else {
    if (oz_isVariable(term2)) {
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
    int res = oz_cv_bindINLINE(tagged2CVar(term1),termPtr1, term2, scp);
    if (res == PROCEED)
      goto next;
    result = res;
    goto fail;
  }
  
  oz_bindToNonvar(termPtr1, term1, term2, scp);
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
  if (isUVar(tag1)) {
    if (isUVar(tag2) && 
	isMoreLocal(term2,term1) &&
	(!am.isLocalVariable(term1,termPtr1) ||
	 heapNewer(termPtr2,termPtr1))) {
      oz_bind(termPtr2, term2, makeTaggedRef(termPtr1));
    } else {
      oz_bind(termPtr1, term1, makeTaggedRef(termPtr2));
    }
    goto next;
  }
  
  if (isUVar(tag2)) {
    oz_bind(termPtr2, term2, makeTaggedRef(termPtr1));
    goto next;
  }

  // FUT

  Assert(isCVar(tag1) && isCVar(tag2));
  /* prefered binding of perdio vars */
  if (cmpCVar(tagged2CVar(term1),tagged2CVar(term2))>0) {
    Swap(term1,term2,TaggedRef);
    Swap(termPtr1,termPtr2,TaggedRef*);
  }


cvar:
  {
    int res = oz_cv_unifyINLINE(tagged2CVar(term1),termPtr1,
				makeTaggedRef(termPtr2), scp);
    if (res == PROCEED)
      goto next;
    result = res;
    goto fail;
  }


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
    if (floatEq(term1,term2))
      goto next;
    else
      goto fail;
   
  case SMALLINT:
    if (smallIntEq(term1,term2))
      goto next;
    goto fail;
    
  case OZCONST:
    switch (tagged2Const(term1)->getType()) {
    case Co_BigInt:
      if (bigIntEq(term1,term2))
	goto next;
      break;
    case Co_Extension:
      {
	int res = tagged2Extension(term1)->eqV(term2);
	if (res == PROCEED)
	  goto next;
	result = res;
	break;
      }
    default:
      break;
    }
    goto fail;

  case LITERAL:
    /* literals and constants unify if their pointers are equal */
  default:
    goto fail;
  }


 /*************/

next:
  if (unifyStack.isEmpty()) {
    result = PROCEED;
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
  Assert(result!=PROCEED);
  unifyStack.mkEmpty();
  // fall through

exit:
  while (!rebindTrail.isEmpty ()) {
    PopRebindTrail(value,refPtr);
    doBind(refPtr,value);
  }

  return result;
}

#ifdef DEBUG_CHECK
static
Board *varHome(TaggedRef val) {
  if (isUVar(val)) {
    return tagged2VarHome(val);
  } else {
    return GETBOARD(tagged2SVarPlus(val));
  }
}
#endif

#ifdef DEBUG_CHECK
static
Bool checkHome(TaggedRef *vPtr) {
  TaggedRef val = oz_deref(*vPtr);

  return !oz_isVariable(val) ||
    oz_isBelow(am.currentBoard(),varHome(val));
}
#endif

/*
 * oz_bind: bind var to term
 * Note: does not handle CVARs specifically
 */

void oz_bind(TaggedRef *varPtr, TaggedRef var, TaggedRef term)
{
  /* first step: do suspension */
  if (isCVar(var)) {
    oz_checkSuspensionList(tagged2SVarPlus(var), pc_std_unif);
  }

  /* second step: push binding for non-local variable on trail;     */
  if ( !am.isLocalVariable(var,varPtr)) {
    Assert(am.inShallowGuard() || checkHome(varPtr));
    am.doTrail(varPtr,var);
  } else  {
    if (isCVar(var)) {
      tagged2CVar(var)->dispose();
    }
  }

  /* third step: bind */
  doBind(varPtr,term);
}

void oz_bind_global(TaggedRef var, TaggedRef term)
{
  DEREF(var,varPtr,varTag);

  Assert(oz_isVariable(var));

  /* first step: do suspension */
  if (isCVar(var)) {
    oz_wakeupAll(tagged2SVarPlus(var));
  }

  /* free memory */
  if (isCVar(var)) {
    tagged2CVar(var)->dispose();
  }

  /* second step: bind */
  doBind(varPtr,term);
}


// mm2: why not inline?
void AM::doBindAndTrail(TaggedRef * vp, TaggedRef t)
{
  Assert(inShallowGuard() || checkHome(vp));
  trail.pushRef(vp, *vp);

  
  CHECK_NONVAR(t);
  *vp = t;

  Assert(oz_isRef(*vp) || !oz_isVariable(*vp));
}

/* -------------------------------------------------------------------------
 * MISC
 * -------------------------------------------------------------------------*/

#ifdef DEBUG_CHECK
static Board *oldBoard = (Board *) NULL;
static Board *oldSolveBoard = (Board *) NULL; 
#endif

void AM::setCurrent(Board *c, Bool checkNotGC)
{
  Assert(!c->isCommitted() && !c->isFailed());
  Assert(!checkNotGC || oz_isCurrentBoard(oldBoard));

  _currentBoard = c;
  _currentUVarPrototype = makeTaggedUVar(c);
  DebugCheckT(oldBoard=c);

  if (c->isSolve ()) {
    Assert(!checkNotGC || oldSolveBoard == _currentSolveBoard);

    _currentSolveBoard = c;
    wasSolveSet = OK; 
    DebugCode (oldSolveBoard = c); 
  } else if (wasSolveSet == OK) {
    Assert(!checkNotGC || oldSolveBoard == _currentSolveBoard);

    _currentSolveBoard = c->getSolveBoard();
    wasSolveSet = NO;
    DebugCode (oldSolveBoard = _currentSolveBoard); 
  }
}

//
Bool NeverDo_CheckProc(unsigned long, void*)
{
  return (NO);
}

// mm2: misssing ifdef VIRTUAL_SITE?

//
// kost@ : The problem with tasks is that we cannot block on them like
// we can on i/o. So, if there are tasks to be done, we say we want to
// wait for availability of 'stderr' (thus, always (i hope!!?));
void AM::handleTasks()
{
  Bool ready = TRUE;

  //
  osClrWatchedFD(fileno(stderr), SEL_WRITE);
  unsetSFlag(TasksReady);

  //
  for (int i = 0; i < MAXTASKS; i++) {
    TaskNode *tn = &taskNodes[i];
    //
    // Apply 'checkProc' from a task with the corresponding argument;
    if (tn->isReady()) {
      tn->dropReady();
      ready = ready && (tn->getProcessProc())(emulatorClock, tn->getArg());
    }
  }

  //
  if (!ready) {
    setSFlag(TasksReady);
    osWatchFD(fileno(stderr), SEL_WRITE);
  }
}

void AM::suspendEngine()
{
  oz_deinstallPath(_rootBoard);

#ifdef DEBUG_THREADCOUNT
  printf("(AM::suspendEngine LTQs=%d) ", existingLTQs); fflush(stdout);
#endif

  ozstat.printIdle(stdout);

  osBlockSignals(OK);

  //
  // kost@ : Alarm timer will be reset later (and we don't need to
  // disturb 'select' when we know how long to wait in it).
  // Note also that the 'SIGALRM'-based scheme for aborting indefinite
  // waiting does not work: the singal can arrive before 'select()'...
  osSetAlarmTimer(0);

  while (1) {

    if (isSetSFlag(UserAlarm)) {
      handleUser();
    }

    if (isSetSFlag(IOReady)) {
      oz_io_handle();
    }

    if (isSetSFlag(TasksReady)) {
      handleTasks();
    }    
    
    if (!threadsPool.threadQueuesAreEmpty()) {
      break;
    }

    // mm2: test if system is idle (not yet working: perdio test is missing)
#ifdef TEST_IDLE
    if (!nextUser() && !hasPendingSelect()) {
      fprintf(stderr,"System is idle: terminating\n");
      exitOz(-1);
    }
#endif

    unsigned long idle_start = osTotalTime();

    //
    unsigned int sleepTime = waitTime();

    //
    osBlockSelect(sleepTime);
    // here 'sleepTime' contains #msecs really spent in waiting;

    //
    setSFlag(IOReady);

    // 
    // kost@ : we have to simulate an effect of the alarm (since no
    // explicit alarm signal is sent while in 'osBlockSelect()');
    handleAlarm(sleepTime);

    ozstat.timeIdle += (osTotalTime() - idle_start);

    wakeUser();
  }

  ozstat.printRunning(stdout);
  
  // restart alarm
  osSetAlarmTimer(CLOCK_TICK/1000);

  osUnblockSignals();
}

void AM::checkStatus()
{
  if (isSetSFlag(StartGC)) {
    oz_deinstallPath(_rootBoard);
    doGC();
  }
  if (isSetSFlag(UserAlarm)) {
    oz_deinstallPath(_rootBoard);
    osBlockSignals();
    handleUser();
    osUnblockSignals();
  }
  if (isSetSFlag(IOReady)) {
    oz_deinstallPath(_rootBoard);
    osBlockSignals();
    oz_io_handle();
    osUnblockSignals();
  }
  if (isSetSFlag(TasksReady)) {
    oz_deinstallPath(oz_rootBoard());
    osBlockSignals();
    handleTasks();
    osUnblockSignals();
  }    
}


// mm2: VIRTUAL_SITES?
//
// Returns 'TRUE' if the task has been successfully registered;
Bool AM::registerTask(void *arg, TaskCheckProc cIn, TaskProcessProc pIn)
{
  for (int i = 0; i < MAXTASKS; i++) {
    TaskNode *tn = &taskNodes[i];

    //
    if (tn->isFree()) {
      tn->setTask(arg, cIn, pIn);
      return (TRUE);
    }
  }

  //
  return (FALSE);
}

//
// Returns 'TRUE' if the task has been successfully removed;;
Bool AM::removeTask(void *arg, TaskCheckProc cIn)
{
  //
  for (int i = 0; i < MAXTASKS; i++) {
    TaskNode *tn = &taskNodes[i];

    //
    if (!tn->isFree() &&
	tn->getArg() == arg &&
	tn->getCheckProc() == cIn) {
      tn->dropTask();
      return (TRUE);
    }
  }

  //
  return (FALSE);
}
  

//
// and another one;
void AM::checkTasks()
{
  Bool tasks = FALSE;

  //
  for (int i = 0; i < MAXTASKS; i++) {
    TaskNode *tn = &taskNodes[i];

    //
    // Apply 'checkProc' from a task with the corresponding argument;
    if ((*(tn->getCheckProc()))(emulatorClock, tn->getArg())) {
      tn->setReady();
      tasks = TRUE;
    }
  }

  if (tasks) {
    setSFlag(TasksReady);
    osWatchFD(fileno(stderr), SEL_WRITE);
  }
}

/* -------------------------------------------------------------------------
 * Signals
 * -------------------------------------------------------------------------*/

void handlerUSR1()
{
  message("Error handler entered ****\n");

  CodeArea::writeInstr();

#ifdef DEBUG_TRACE
  ozd_tracerOn(); ozd_trace("halt");
#endif
  message("Error handler exit ****\n");
}

void handlerINT()
{
  prefixError();
  message("SIG INT ****\n");
  am.exitOz(1);
}

void handlerTERM()
{
  prefixError();
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
  error("**** segmentation violation ****\n");
  am.exitOz(1);
}

void handlerBUS()
{
  CodeArea::writeInstr();
  error("**** bus error ****\n");
  am.exitOz(1);
}

void handlerPIPE()
{
  //
  // kost@ : let's check for a dead machine;
  if (isDeadSTDOUT())
    am.exitOz(1);
  //
  prefixError();
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

//
// Signal handler;
void handlerALRM()
{
  am.handleAlarm(CLOCK_TICK/1000);
}

//
// 'USR2' serves right now only virtual sites;
void handlerUSR2()
{
  am.handleUSR2();

}

/* -------------------------------------------------------------------------
 * Alarm handling
 * -------------------------------------------------------------------------*/

void AM::handleAlarm(unsigned int ms)
{
  emulatorClock = emulatorClock + (unsigned long) ms;

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

  if (checkUser())
    setSFlag(UserAlarm);

  checkGC();

  oz_io_check();

  // tasks are actually checked twice - here and in the 'USR2'
  // handler; but these are very light-weight checks;
  checkTasks();
}

//
void AM::handleUSR2()
{
  checkTasks();
}

/* handleUserAlarm:
    if UserAlarm-SFLAG is set this method is called
    interrupts should already be disabled by the parent procedure
    */
void AM::handleUser()
{
  unsetSFlag(UserAlarm);
  wakeUser();
}

class OzSleep {
public:
  OzSleep *next;
  unsigned int time;    // absolut time in ms, when this guy must be woken up
  TaggedRef node;
public:
  OzSleep(int t, TaggedRef n,OzSleep *a)
    : next(a), time(t), node(n)
  {
    OZ_protect(&node);
    Assert(t>=0);
  }
  ~OzSleep() { OZ_unprotect(&node); }
};

void AM::insertUser(int ms, TaggedRef node)
{
  osBlockSignals();

  unsigned int wakeupAt = osTotalTime() + ms;

  OzSleep **prev = &sleepQueue;
  for (OzSleep *aux = *prev; aux; prev = &aux->next, aux=aux->next) {
    if (wakeupAt <= aux->time) {
      *prev = new OzSleep(wakeupAt,node,aux);
      goto exit;
    }
  }
  *prev = new OzSleep(wakeupAt,node,NULL);

exit:
  osUnblockSignals();
}


int AM::nextUser()
{
  return (sleepQueue==NULL) ? 0 : max(1,sleepQueue->time - osTotalTime());
}

//
// Yields time for blocking in 'select()';
unsigned int AM::waitTime()
{
  unsigned int sleepTime;

  //
  // kost@ : --> EK: this should be replaced by 'setMinimalTaskInterval()';
#if defined(SLOWNET)
  sleepTime = CLOCK_TICK/1000;
#else
  if (taskMinInterval)
    sleepTime = min(nextUser(), taskMinInterval);
  else 
    sleepTime = nextUser();
  // don't sleep less than 'CLOCK_TICK/1000' ms;
  sleepTime = max(sleepTime, CLOCK_TICK/1000);
#endif

  return (sleepTime);
}

Bool AM::checkUser()
{
  return (sleepQueue!=NULL && sleepQueue->time <= osTotalTime());
}

void AM::wakeUser()
{
  unsigned int now = osTotalTime();

  while (sleepQueue && sleepQueue->time<=now) {
    oz_io_awakeVar(sleepQueue->node);
    OzSleep *aux = sleepQueue->next;
    delete sleepQueue;
    sleepQueue = aux;
  }
}



void oz_checkDebugOutline(Thread *tt)
{
  Assert(am.debugmode());
  if (oz_currentThread() && tt->getThrType() == S_RTHREAD)
    if (oz_currentThread()->getTrace()) {
      tt->setTrace(OK);
      tt->setStep(OK);
    }
}


#ifdef DEBUG_STATUS
  /*
   * Print capital letter, when flag is set and
   * lower case letter when unset.
   */ 
char flagChar(StatusBit flag)
{
  switch (flag) {
  case ThreadSwitch: return 'T';
  case IOReady:      return 'I';
  case UserAlarm:    return 'U';
  case StartGC:      return 'G';
  default:           return 'X';
  }
}
#endif

void AM::prepareCall(TaggedRef pred, RefsArray args)
{
  CallList **aux = &preparedCalls;
  while(*aux) {
    aux = &(*aux)->next;
  }
  *aux = new CallList(pred,args);
}


void AM::prepareCall(TaggedRef pred, TaggedRef arg0, TaggedRef arg1, 
		     TaggedRef arg2, TaggedRef arg3, TaggedRef arg4)
{
  int argno = 0;
  if (arg0) argno++;
  if (arg1) argno++;
  if (arg2) argno++;
  if (arg3) argno++;
  if (arg4) argno++;

  RefsArray a = allocateRefsArray(argno);
  if (arg0) a[0]=arg0;
  if (arg1) a[1]=arg1;
  if (arg2) a[2]=arg2;
  if (arg3) a[3]=arg3;
  if (arg4) a[4]=arg4;
  prepareCall(pred,a);
}


void AM::pushPreparedCalls(Thread *thr)
{
  Assert(preparedCalls != NULL);
  while(preparedCalls) {
    CallList *aux = preparedCalls;
    if (thr) {
      thr->pushCallNoCopy(aux->proc,aux->args);
    } else {
      cachedStack->pushCallNoCopy(aux->proc,aux->args);
    }
    preparedCalls = aux->next;
    aux->dispose();
  }
}

void AM::suspendOnVarList(Thread *thr)
{
  while (oz_isCons(_suspendVarList)) {
    OZ_Term v=oz_head(_suspendVarList);
    Assert(oz_isVariable(*tagged2Ref(v)));
    
    addSuspAnyVar(tagged2Ref(v),thr);
    _suspendVarList=oz_tail(_suspendVarList);
  }
}

