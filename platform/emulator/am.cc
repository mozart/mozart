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
#include "var_all.hh"
#include "codearea.hh"
#include "fdomn.hh"
#include "trace.hh"
#include "space.hh"

AM am;

/* -------------------------------------------------------------------------
 * Init and exit AM
 * -------------------------------------------------------------------------*/

static
void usage(int /* argc */,char **argv) {
  fprintf(stderr,
          "usage: %s <options>\n",
          argv[0]);
  fprintf(stderr, " -d           : debugging on\n");
  fprintf(stderr, " -init <file> : init functor\n");
  fprintf(stderr, " -u <url>     : application functor\n");
  fprintf(stderr, " -x <hex>     : virtual site identifier\n");
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
#ifdef DEBUG_FD_CONSTRREP
          " DEBUG_FD_CONSTRREP"
#endif
#ifdef DEBUG_FD
          " DEBUG_FD"
#endif
#ifdef DEBUG_FSET
          " DEBUG_FSET"
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

#ifdef THREADED
  // fprintf(stderr, "Using threaded code (abs jumps).\n");
#else
  fprintf(stderr, "Not using threaded code.\n");
#endif

}


extern void initBuiltins();
extern void bigIntInit(); /* from value.cc */
extern void initffuns();  /* from initffuns.cc */
extern void initVirtualProperties();
extern void initExtensions(); /* extensions */

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

  defaultExceptionHdl = makeTaggedNULL();

  preparedCalls = NULL;

  char *home = getenv("OZHOME");

  if (!home)
    home ="unknown";


  ozconf.ozHome = home;

  char *url = NULL;
  char *initFile = getenv("OZINIT");

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

    if (strcmp(argv[i],"--gui")==0 ||
        strcmp(argv[i], "-gui")==0) {
      ozconf.gui=1;
      continue;
    }

    if (strcmp(argv[i],"-d")==0) {
#ifdef DEBUG_TRACE
      ozd_tracerOn();
#endif
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

  _rootBoard = new Board();
  _currentBoard = NULL;
  cachedStack  = NULL;
  cachedSelf   = NULL;
  setInEqEq(FALSE);
  setCurrent(_rootBoard);

  lastThreadID    = 0;
  debugMode       = NO;
  debugStreamTail = OZ_newVariable();

  propLocation    = NO;

  threadsPool.initThreads();

  // builtins
  initLiterals();

  initBuiltins();

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
  genFreeListManager=new GenFreeListManager();
  idCounter = new FatInt();

  //
  initSite();
  initMarshaler();

  //
  initExtensions();

  // init x args
  {
    for (int i=NumberOfXRegisters; i--; )
      xRegs[i] = taggedVoidValue;
  }
#ifdef PICKLE2TEXTHACK
  if (p2t) {
    extern int pickle2text();
    Bool aux = pickle2text();
    exit(aux ? 0 : 1);
  }
#endif

  Thread *tt = oz_newThread();

  if (initFile) {
    TaggedRef functor   = oz_newVariable();
    TaggedRef procedure = oz_newVariable();
    TaggedRef export    = oz_newVariable();

    // Construct import for functor:
    TaggedRef boot_module =
      OZ_recordInit(AtomExport,
                    oz_cons(oz_pair2(AtomObtain,BI_obtain_native),
                            oz_nil()));
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

// mm2: missing ifdef VIRTUAL_SITE?

//
Bool NeverDo_CheckProc(unsigned long, void*)
{
  return (NO);
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
      ready = ready && (tn->getProcessProc())(emulatorClock, tn->getArg());
    }
  }

  //
  if (!ready)
    setSFlag(TasksReady);
}

// a variable that is set to 0 or to a pointer to a procedure
// to check the status of children processes (see contrib/os/process.cc)

void (*oz_child_handle)() = 0;

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

void AM::suspendEngine()
{
  oz_installPath(_rootBoard);

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

    if (!threadsPool.threadQueuesAreEmpty())
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
    oz_installPath(oz_rootBoard());
    osBlockSignals();
  }

  unsetSFlag(TimerInterrupt);
  handleAlarm(); // must be done first since it might set other flags

  if (isSetSFlag(StartGC))
    doGC();

  if (isSetSFlag(UserAlarm))
    handleUser();

  if (isSetSFlag(IOReady))
    oz_io_handle();

  if (isSetSFlag(SigPending)) {
    pushSignalHandlers();
    unsetSFlag(SigPending);
  }

  // kost@: first check signals, then handle tasks! Otherwise
  // e.g. 'usr2' can be disregarded;
  if (isSetSFlag(TasksReady))
    handleTasks();

  if (isSetSFlag(ChildReady)) {
    unsetSFlag(ChildReady);
    if (oz_child_handle!=0) (*oz_child_handle)();
  }

  if (block)
    osUnblockSignals();
}

//
// mm2: VIRTUAL_SITES?
// kost@ : not only (also managing tcp/ip connections' cache and
// ports' flow control).
//
void AM::setMinimalTaskInterval(void *arg, unsigned int ms)
{
  unsigned int accMinTime = 0;

  //
  for (int i = 0; i < MAXTASKS; i++) {
    TaskNode *tn = &taskNodes[i];

    //
    if (!tn->isFree()) {
      if (tn->getArg() == arg)
        // for all tasks with 'arg', if there are multiple;
        tn->setMinimalTaskInterval(ms);

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
    if ((*(tn->getCheckProc()))(emulatorClock, tn->getArg())) {
      tn->setReady();
      tasks = TRUE;
    }
  }

  if (tasks)
    setSFlag(TasksReady);
}

/* -------------------------------------------------------------------------
 * Signals
 * -------------------------------------------------------------------------*/

void handlerUSR1(int)
{
  message("Error handler entered ****\n");

  CodeArea::writeInstr();

#ifdef DEBUG_TRACE
  ozd_tracerOn(); ozd_trace("halt");
#endif
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
  DebugCheckT(message("a child process' state changed ****\n"));
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
  am.emulatorClock += CLOCK_TICK/1000;

  if (am.isCritical()) /* wait for next ALRM signal */
    return;

  am.setSFlag(TimerInterrupt);
}

//
// 'USR2' serves right now only virtual sites;
void handlerUSR2(int)
{
  am.handleUSR2();
}

/* -------------------------------------------------------------------------
 * Alarm handling
 * -------------------------------------------------------------------------*/

void AM::handleAlarm(int ms)
{
  if (ms>0)
    emulatorClock += (unsigned long) ms;

  if (ozstat.currPropagator) {
    ozstat.currPropagator->incSamples();
  } else if (ozstat.currAbstr) {
    ozstat.currAbstr->samples++;
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
  while (oz_isCons(_suspendVarList)) {
    OZ_Term v=oz_head(_suspendVarList);
    Assert(oz_isVariable(*tagged2Ref(v)));

    OZ_Return ret = oz_var_addSuspINLINE(tagged2Ref(v),thr);
    if (ret != SUSPEND) {
      am.emptySuspendVarList();
      return ret;
    }
    _suspendVarList=oz_tail(_suspendVarList);
  }
  return SUSPEND;
}
