/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Kostja Popow, 1997-1999
 *    Michael Mehl, 1997-1999
 *    Denys Duchier, 1997-1999
 *    Christian Schulte, 1997-1999
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include <setjmp.h>
#include "am.hh"
#include "gname.hh"
#include "os.hh"
#include "board.hh"
#include "thr_int.hh"
#include "var_base.hh"
#include "codearea.hh"
#include "fdomn.hh"
#include "pickle.hh"
#include "cpi.hh"

AM am;

/* -------------------------------------------------------------------------
 * Init and exit AM
 * -------------------------------------------------------------------------*/

static
void usage(int /* argc */,char **argv) {
  fprintf(stderr,
	  "usage: %s <options>\n",
	  argv[0]);
  fprintf(stderr, " -init <file> : init functor\n");
  fprintf(stderr, " -u <url>     : application functor\n");
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
void printBanner()
{
#ifdef NO_LTQ
  OZ_warning("LTQ is turned off.");
#endif

#ifdef DEBUG_CHECK
  fprintf(stderr,
	  "FLAGS:"
	  " DEBUG_CHECK"
#ifdef THREADED
          " THREADED"
#endif
#ifdef DEBUG_DET
	  " DEBUG_DET"
#endif
#ifdef DEBUG_GC
	  " DEBUG_GC"
#endif
#ifdef DEBUG_FD_CONSTRREP
	  " DEBUG_FD_CONSTRREP"
#endif
#ifdef DEBUG_FD
	  " DEBUG_FD"
#endif
#ifdef DEBUG_FSET
	  " DEBUG_FSET"
#endif
#ifdef DEBUG_MEM
	  " DEBUG_MEM"
#endif
#ifdef DEBUG_FSET_CONSTRREP
	  " DEBUG_FSET_CONSTRREP"
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

}


extern void initOzIdLoc(void);
extern void initBuiltins();
extern void bigIntInit(); /* from value.cc */
extern void initVirtualProperties();
extern void initExtensions(); /* extensions */
#ifdef DENYS_EVENTS
extern void initEvents();
#endif

void AM::init(int argc,char **argv)
{  
  Assert(makeTaggedNULL() == 0);
  Assert(PROCEED && !FAILED);

#ifdef DEBUG_CHECK
  {
    char *eh = osgetenv("OZ_EHOOK");
    if (eh) {
      fprintf(stderr, "Waiting 10 secs... hook up (pid %d)!\n", osgetpid());
      fflush(stderr);
      ossleep(10);
    }
  }
#endif

  init_cmem();
  ozconf.init();
  osInit();
  AssRegArray::init();

  defaultExceptionHdl = makeTaggedNULL();

  preparedCalls = NULL;

  char *home = osgetenv("OZHOME");
  
  if (!home)
    home ="unknown";
#ifdef WINDOWS
  else
    home =strdup(home);
#endif

    
  ozconf.ozHome = home;

  char *url = NULL;
  Bool traceLoad = osgetenv("OZ_TRACE_LOAD") != NULL;
  char *initFile = osgetenv("OZINIT");
  
  /* process command line arguments */
  ozconf.argV = NULL;
  ozconf.argC = 0;

  /* enter emulator path */
  ozconf.emuexe  = strdup(argv[0]);
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

    int p2t = 0;

  for (int i=url?2:1; i<argc; i++) {
    if (strcmp(argv[i],"--pickle2text")==0) {
      p2t = 1;
      break;
    }

    if (strcmp(argv[i],"--gui")==0 ||
	strcmp(argv[i], "-gui")==0) {
      ozconf.gui=1;
      continue;
    }

    if (strcmp(argv[i],"-u")==0) {
      url = getOptArg(i,argc,argv);
      ozconf.url = url;
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

  if (!p2t && !initFile) {
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

  if (!p2t && !initFile) {
    fprintf(stderr,"No init file found.\n");
    usage(argc,argv);
  }

  printBanner();
  if (traceLoad)
    if (initFile)
      fprintf(stderr,"Init file: %s\n",initFile);
    else
      fprintf(stderr,"No init file\n");

  (void) engine(OK);

  initFDs();

  initMemoryManagement();

  bigIntInit();

// not changeable
  // SizeOfWorkingArea,NumberOfXRegisters,NumberOfYRegisters


// internal registers
  statusReg    = (StatusBit)0;
  criticalFlag = NO;

  _rootBoard = new Board();
  _currentBoard = NULL;
  cachedStack  = NULL;
  cachedSelf   = NULL;
  gcStep = OddGCStep;
  copyStep = 0;
  setInEqEq(FALSE);
  setCurrent(_rootBoard, _rootBoard->getOptVar());

  lastThreadID    = 0;
  debugMode       = NO;
  debugStreamTail = OZ_newVariable();

  propLocation    = NO;

  threadsPool.init();
  SuspList::init();

  // builtins
  initLiterals();
  initCPI();

  DBG_STEP_ATOM   = (Atom *) tagged2Literal(AtomDebugStep);
  DBG_NOSTEP_ATOM = (Atom *) tagged2Literal(AtomDebugNoStep);
  DBG_EXIT_ATOM   = (Atom *) tagged2Literal(AtomDebugExit);

  initBuiltins();
  
  initVirtualProperties();

  emptySuspendVarList(); // must be after initLiterals

#ifndef DENYS_EVENTS
  //
  taskNodes = new TaskNode[MAXTASKS];
#endif

  //
  osInitSignals();
  osSetAlarmTimer(CLOCK_TICK/1000);

  //
  genFreeListManager=new GenFreeListManager();
  idCounter = new FatInt();

  //
  initSite();
  initPickleMarshaler();

  //
  initExtensions();

#ifdef DENYS_EVENTS
  //
  initEvents();
#endif

  // init x args
  {
    for (int i=NumberOfXRegisters; i--; )
      XREGS[i] = taggedVoidValue;
  }
  initOzIdLoc();

  if (p2t) {
    extern int pickle2text();
    Bool aux = pickle2text();
    exit(aux ? 0 : 1);
  }

  Thread *tt = oz_newThread();

  if (initFile) {
    TaggedRef functor   = oz_newVariable();
    TaggedRef procedure = oz_newVariable();
    TaggedRef expo      = oz_newVariable();

    // Construct import for functor:
    TaggedRef boot_module = 
      OZ_recordInit(AtomExport,
		    oz_mklist(oz_pair2(AtomGetInternal,BI_get_internal),
			      oz_pair2(AtomGetNative,BI_get_native)));

    TaggedRef boot_import = 
      OZ_recordInit(AtomExport,
		    oz_mklist(oz_pair2(AtomBoot,   boot_module)));
    
    // Task3: execute functor's code
    tt->pushCall(procedure,RefsArray::make(boot_import,expo));

    // Task2: lookup functor's code
    tt->pushCall(BI_dot,RefsArray::make(functor,AtomApply,procedure));

    // Task1: load functor
    tt->pushCall(BI_load,RefsArray::make(oz_atom(initFile),functor));
  }

  //
  sleepQueue = (OzSleep *) 0;
#ifdef DENYS_EVENTS
  requestedTimer=0;
#endif
  taskMinInterval = DEFAULT_MIN_INTERVAL;

  unsetProfileMode();
}

void AM::exitOz(int status)
{
  static int gotHereOnce = 0;

  if (!gotHereOnce) {
    gotHereOnce = 1;
    (*dpExit)();
    osExit(status);
  }
}

/* -------------------------------------------------------------------------
 * MISC
 * -------------------------------------------------------------------------*/

#ifndef DENYS_EVENTS
// mm2: missing ifdef VIRTUAL_SITE?

//
Bool NeverDo_CheckProc(LongTime *, void*)
{
  return (NO);
}

char * LongTime::toString() {
  static char s[2*sizeof(unsigned long)*8+2];
  if(high==0)
    sprintf(s,"%ld",low);
  else
    sprintf(s,"%ld%032ld",high,low);
  return s;
}

//
// kost@ : The problem with tasks is that we cannot block on them like
// we can on i/o. So, if there are tasks to be done, we say we want to
// wait for availability of 'stderr' (thus, always (i hope!!?));
void AM::handleTasks()
{
  Bool ready = TRUE;

  //
  unsetSFlag(TasksReady);

  //
  for (int i = 0; i < MAXTASKS; i++) {
    TaskNode *tn = &taskNodes[i];
    //
    // Apply 'checkProc' from a task with the corresponding argument;
    if (tn->isReady()) {
      tn->dropReady();
      ready = ready && (tn->getProcessProc())(&emulatorClock, tn->getArg());
    }
  }

  //
  if (!ready)
    setSFlag(TasksReady);
}
#endif

// a variable that is set to 0 or to a pointer to a procedure
// to check the status of children processes (see contrib/os/process.cc)

#ifndef DENYS_EVENTS
void (*oz_child_handle)() = 0;
#endif

// The engine goes to sleep by performing an indefinite select,
// but we also want the engine to wake up when a child process
// terminates.  The SIGCHLD signal does interrupt the select and
// cause it to return, but, in order for this to happen, signals
// must be unblocked: this is the 1st point.  The 2nd point is
// that there is a race condition: the SIGCHLD signal may be
// delivered between the time signals are reenabled and the
// select call is performed, in which case the engine would go
// to sleep for ever even though we should take care of the child
// that has terminated.  The only solution I could think of is
// to let the SIGCHLD handler perform a longjump out of this
// critical region.
// * where to longjmp to is stored in global variable wake_jmp
// * whether to longjmp is indicated by global variable use_wake_jmp

//
// kost@ : once again, about critical sections and race conditions.
// Now the race condition is avoided for *all* signals using the
// scheme i used previously for SIGUSR2 (exploited by e.g. virtual
// sites): the generic signal handler adds 'stderr' to the list of
// FD"s watched for output, which is *normally* always available and
// thus 'select()' will not block.

#ifdef WINDOWS
jmp_buf wake_jmp;
#define sigsetjmp(X,Y) setjmp(X)
#define siglongjmp(X,Y) longjmp(X,Y)
#else /* !WINDOWS */
sigjmp_buf wake_jmp;
#endif
volatile int use_wake_jmp = 0;

void AM::suspendEngine(void) {

  _rootBoard->install();

  ozstat.printIdle(stdout);

  //
  osBlockSignals(OK);

  //
  // kost@ : Alarm timer will be reset later (and we don't need to
  // disturb 'select' when we know how long to wait in it).
  // Note also that the 'SIGALRM'-based scheme for aborting indefinite
  // waiting does not work: the signal can arrive before 'select()'...
  osSetAlarmTimer(0);

  while (1) {

    //
    // kost@ : drop 'stderr' from the mask: 'select' must fall through
    // only when some signals will be delivered since
    // 'osBlockSignals()' above (which can happen after
    // 'osUnblockSignals()' below);
    osClrWatchedFD(fileno(stderr), SEL_WRITE);

    checkStatus(NO);

    if (!threadsPool.isEmpty())
      break;

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

    // here we set up wake_jmp so that we can longjmp out of a
    // signal handler
    if (sigsetjmp(wake_jmp,1)==0) {
      // indicate to certain signal handlers that they should
      // perform a siglongjump using wake_jmp
      use_wake_jmp=1;
      // reenable appropriate signal handlers
      // i.e. all since alarm is disabled anyway
#ifndef WINDOWS
      osUnblockSignals();
#endif
      // now perform blocking select
      osBlockSelect(sleepTime);
      // it returned normally
      // we block all signals again, but it is ok if we don't make
      // it through because a signal handler is invoked and does
      // a siglongjump that returns to the `else' below.  Either
      // way the engine is going to be properly woken up.
#ifndef WINDOWS
      osBlockSignals(NO);
#endif
      use_wake_jmp=0;
      // here 'sleepTime' contains #msecs really spent in waiting;
      setSFlag(IOReady);
    } else {
      // siglongjmp'ed out of a signal handler
      // note that the all blocking mask has been restored
      use_wake_jmp=0;
      // we must compute the time spent in waiting in sleepTime
      // since it probably didn't get a chance to be updated by
      // returning normally from osBlockSelect
      sleepTime=(osTotalTime() - idle_start);
    }

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

#ifdef DENYS_SIGNAL
void AM::leaveSignalHandler() {
  if (use_wake_jmp) {
    use_wake_jmp;
    siglongjmp(wake_jmp,1);
  }
}

void AM::doSignalHandler() {
  setSFlag(SigPending);
  leaveSignalHandler();
}
#endif // DENYS_SIGNAL

void AM::checkStatus(Bool block)
{
  if (!isSetSFlag())
    return;

  if (block) {
    _rootBoard->install();
    osBlockSignals();
  }

  unsetSFlag(TimerInterrupt);
  handleAlarm(); // must be done first since it might set other flags

  if (isSetSFlag(StartGC))
    doGCollect();

  if (isSetSFlag(UserAlarm))
    handleUser();

  if (isSetSFlag(IOReady))
    oz_io_handle();

  if (isSetSFlag(SigPending)) {
    pushSignalHandlers();
    unsetSFlag(SigPending);
  }

#ifndef DENYS_EVENTS
  // kost@: first check signals, then handle tasks! Otherwise
  // e.g. 'usr2' can be disregarded;
  if (isSetSFlag(TasksReady))
    handleTasks();
#endif

  if (isSetSFlag(ChildReady)) {
    unsetSFlag(ChildReady);
#ifdef DENYS_EVENTS
    OZ_eventPush(oz_atom("SIGCHLD"));
#else
    if (oz_child_handle!=0) (*oz_child_handle)();
#endif
  }
  
  if (block)
    osUnblockSignals();
}

#ifndef DENYS_EVENTS
//
// mm2: VIRTUAL_SITES?
// kost@ : not only (also managing tcp/ip connections' cache and
// ports' flow control).
//
void AM::setMinimalTaskInterval(void *arg, unsigned int ms)
{
  unsigned int accMinTime = 0;
  DebugCode(Bool taskExists = FALSE;)
    
  //
  for (int i = 0; i < MAXTASKS; i++) {
    TaskNode *tn = &taskNodes[i];

    //
    if (!tn->isFree()) {
      if (tn->getArg() == arg){
	// for all tasks with 'arg', if there are multiple;
	tn->setMinimalTaskInterval(ms);
	DebugCode(taskExists = TRUE;)
      }
      //
      unsigned int tnMinTime = tn->getMinimalTaskInterval();
      if (tnMinTime) {
	if (accMinTime)
	  accMinTime = min(accMinTime, tnMinTime);
	else
	  accMinTime = tnMinTime;
      }
    }
  }
  Assert(taskExists!=FALSE);
  //
  taskMinInterval = accMinTime;
}

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
    if ((*(tn->getCheckProc()))(&emulatorClock, tn->getArg())) {
      tn->setReady();
      tasks = TRUE;
    }
  }

  if (tasks)
    setSFlag(TasksReady);
}
#endif

/* -------------------------------------------------------------------------
 * Signals
 * -------------------------------------------------------------------------*/

void handlerUSR1(int)
{
  message("Error handler entered ****\n");

  CodeArea::writeInstr();

  message("Error handler exit ****\n");
}

void handlerSEGV(int)
{
  CodeArea::writeInstr();
  OZ_error("**** segmentation violation ****\n");
  am.exitOz(1);
}

void handlerBUS(int)
{
  CodeArea::writeInstr();
  OZ_error("**** bus error ****\n");
  am.exitOz(1);
}

void handlerPIPE(int)
{
  //
  // kost@ : let's check for a dead machine;
  // if (isDeadSTDOUT())
    //am.exitOz(1);
  //
  //prefixError();
  //message("write on a pipe or other socket with no one to read it ****\n");
}

void handlerCHLD(int)
{
  // DebugCheckT(message("a child process' state changed ****\n"));
  am.setSFlag(ChildReady);
  if (use_wake_jmp) {
    use_wake_jmp=0;
    siglongjmp(wake_jmp,1);
  }
}

//
// Signal handler;
void handlerALRM(int)
{
  am.emulatorClock.increaseTime(CLOCK_TICK/1000);

  if (am.isCritical()) /* wait for next ALRM signal */
    return;

  am.setSFlag(TimerInterrupt);
}

//
// 'USR2' serves right now only virtual sites;
void handlerUSR2(int)
{
#ifdef DENYS_EVENTS
  static TaggedRef sigusr2 = oz_atom("SIGUSR2");
  OZ_eventPush(sigusr2);
#else
  am.handleUSR2();
#endif
}

/* -------------------------------------------------------------------------
 * Alarm handling
 * -------------------------------------------------------------------------*/

void AM::handleAlarm(int ms)
{
  if (ms>0) 
    emulatorClock.increaseTime((unsigned long) ms);

  if (am.profileMode()) {
    if (ozstat.currPropagator) {
      ozstat.currPropagator->incSamples();
    } else if (ozstat.currAbstr) {
      ozstat.currAbstr->getProfile()->samples++;
    }
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

#ifndef DENYS_EVENTS
  // tasks are actually checked twice - here and in the 'USR2'
  // handler; but these are very light-weight checks;
  checkTasks();
#endif
}

#ifndef DENYS_EVENTS
//
void AM::handleUSR2()
{
  checkTasks();
}
#endif

/* handleUserAlarm:
    if UserAlarm-SFLAG is set this method is called
    interrupts should already be disabled by the parent procedure
    */
void AM::handleUser()
{
  unsetSFlag(UserAlarm);
  wakeUser();
}

#ifndef DENYS_EVENTS
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
#endif

#ifdef DENYS_EVENTS
void OZ_setTimer(int i)
{
  am.setTimer(i);
}
#endif

int AM::nextUser()
{
#ifdef DENYS_EVENTS
  return (requestedTimer==0) ? 0 : max(1,requestedTimer-osTotalTime());
#else
  return (sleepQueue==NULL) ? 0 : max(1,sleepQueue->time - osTotalTime());
#endif
}

//
// Yields time for blocking in 'select()';
unsigned int AM::waitTime()
{
  unsigned int sleepTime;
  unsigned int nu = nextUser();

  //
  if (taskMinInterval)
    sleepTime = nu ? min(nu, taskMinInterval) : taskMinInterval;
  else
    sleepTime = nu;

  //
  return (sleepTime);
}

Bool AM::checkUser()
{
#ifdef DENYS_EVENTS
  return (requestedTimer!=0 && requestedTimer<=osTotalTime());
#else
  return (sleepQueue!=NULL && sleepQueue->time <= osTotalTime());
#endif
}

void AM::wakeUser()
{
#ifdef DENYS_EVENTS
  if (requestedTimer!=0 && requestedTimer<=osTotalTime()) {
    requestedTimer=0;
    OZ_eventPush(oz_atom("timer"));
  }
#else
  unsigned int now = osTotalTime();
  while (sleepQueue && sleepQueue->time<=now) {
    oz_io_awakeVar(sleepQueue->node);
    OzSleep *aux = sleepQueue->next;
    delete sleepQueue;
    sleepQueue = aux;
  }
#endif
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


void AM::prepareCall(TaggedRef pred, RefsArray * args)
{
  CallList **aux = &preparedCalls;
  while(*aux) {
    aux = &(*aux)->next;
  }
  *aux = new CallList(pred,args);
}

void AM::pushPreparedCalls(Thread *thr)
{
  Assert(preparedCalls != NULL);
  while(preparedCalls) {
    CallList *aux = preparedCalls;
    if (thr) {
      thr->pushCall(aux->proc,aux->args);
    } else {
      cachedStack->pushCall(aux->proc,aux->args);
    }
    preparedCalls = aux->next;
    aux->dispose();
  }
}

Bool AM::isEmptyPreparedCalls()
{
  return preparedCalls==0;
}

void AM::emptyPreparedCalls()
{
  while(preparedCalls) {
    CallList *aux = preparedCalls;
    preparedCalls = aux->next;
    aux->dispose();
  }
}

OZ_Return AM::suspendOnVarList(Thread *thr)
{
  while (oz_isLTuple(_suspendVarList)) {
    OZ_Term v=oz_head(_suspendVarList);
    Assert(oz_isVar(*tagged2Ref(v)));
    OZ_Return ret = oz_var_addSusp(tagged2Ref(v),thr);
    if (ret != SUSPEND) {
      am.emptySuspendVarList();
      return ret;
    }
    _suspendVarList=oz_tail(_suspendVarList);
  }
  return SUSPEND;
}

//
TaggedRef oz_newVariable(Board *b)
{
  TaggedRef *ret = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ret = b->getOptVar();
  return makeTaggedRef(ret);
}

OZ_Return oz_addSuspendVarList(TaggedRef * t) {
  return am.addSuspendVarListInline(t);
}

OZ_Return oz_addSuspendVarList(TaggedRef t) {
  return am.addSuspendVarListInline(t);
}

OZ_Return oz_addSuspendVarList2(TaggedRef t1,TaggedRef t2) {
  DEREF(t1,t1ptr);
  Assert(!oz_isRef(t1));
  if (oz_isVarOrRef(t1))
    (void) am.addSuspendVarListInline(t1ptr);
  DEREF(t2,t2ptr);
  Assert(!oz_isRef(t2));
  if (oz_isVarOrRef(t2))
    (void) am.addSuspendVarListInline(t2ptr);
  return SUSPEND;
}

OZ_Return oz_addSuspendVarList3(TaggedRef t1, TaggedRef t2, TaggedRef t3) {
  DEREF(t1,t1ptr);
  Assert(!oz_isRef(t1));
  if (oz_isVarOrRef(t1))
    (void) am.addSuspendVarListInline(t1ptr);
  DEREF(t2,t2ptr);
  Assert(!oz_isRef(t2));
  if (oz_isVarOrRef(t2))
    (void) am.addSuspendVarListInline(t2ptr);
  DEREF(t3,t3ptr);
  Assert(!oz_isRef(t3));
  if (oz_isVarOrRef(t3))
    (void) am.addSuspendVarListInline(t3ptr);
  return SUSPEND;
}

OZ_Return oz_addSuspendInArgs1(OZ_Term * _OZ_LOC[]) {
  TaggedRef t = OZ_in(0);
  DEREF(t,tptr);
  return am.addSuspendVarListInline(tptr);
}

OZ_Return oz_addSuspendInArgs2(OZ_Term * _OZ_LOC[]) {
  TaggedRef t1 = OZ_in(0);
  DEREF(t1,t1ptr);
  Assert(!oz_isRef(t1));
  if (oz_isVarOrRef(t1))
    (void) am.addSuspendVarListInline(t1ptr);
  TaggedRef t2 = OZ_in(1);
  DEREF(t2,t2ptr);
  Assert(!oz_isRef(t2));
  if (oz_isVarOrRef(t2))
    (void) am.addSuspendVarListInline(t2ptr);
  return SUSPEND;
}

OZ_Return oz_addSuspendInArgs3(OZ_Term * _OZ_LOC[]) {
  TaggedRef t1 = OZ_in(0);
  DEREF(t1,t1ptr);
  Assert(!oz_isRef(t1));
  if (oz_isVarOrRef(t1))
    (void) am.addSuspendVarListInline(t1ptr);
  TaggedRef t2 = OZ_in(1);
  DEREF(t2,t2ptr);
  Assert(!oz_isRef(t2));
  if (oz_isVarOrRef(t2))
    (void) am.addSuspendVarListInline(t2ptr);
  TaggedRef t3 = OZ_in(2);
  DEREF(t3,t3ptr);
  Assert(!oz_isRef(t3));
  if (oz_isVarOrRef(t3))
    (void) am.addSuspendVarListInline(t3ptr);
  return SUSPEND;
}

#if OUTLINE_SETEXCEPTIONINFO
void AM::setExceptionInfo(TaggedRef inf) {
    if (exception.info == NameUnit) {
      exception.info=oz_nil();
    }
    exception.info = oz_cons(inf,exception.info);
  }
#endif

#if OUTLINE_HF_RAISE_FAILURE
Bool AM::hf_raise_failure()
{
  if (!oz_onToplevel() && !oz_currentThread()->isCatch())
    return OK;

  exception.info  = NameUnit;
  exception.value = RecordFailure;
  exception.debug = ozconf.errorDebug;
  return NO;
}
#endif
