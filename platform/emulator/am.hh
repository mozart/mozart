/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Kostja Popow, 1997-1999
 *    Michael Mehl, 1997-1999
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

#ifndef __AMH
#define __AMH

#include "base.hh"
#include "trail.hh"
#include "thr_pool.hh"
#include "value.hh"


/* -----------------------------------------------------------------------
 * StatusReg
 * -----------------------------------------------------------------------*/

typedef enum {
  ThreadSwitch   = 1 << 2, // choose a new process
  IOReady        = 1 << 3, // IO handler has signaled IO ready
  UserAlarm      = 1 << 4, // Alarm handler has signaled User Alarm
  StartGC        = 1 << 5, // need a GC
  TasksReady     = 1 << 6,
  ChildReady     = 1 << 7, // SIGCHLD raised
  SigPending     = 1 << 8,  // some signal caught
  TimerInterrupt = 1 << 9
} StatusBit;

/* -----------------------------------------------------------------------
 * spaces
 * -----------------------------------------------------------------------*/

Bool oz_isBelow(Board *below, Board *above);

// oz_isBetween returns
enum oz_BFlag {
  B_BETWEEN,
  B_NOT_BETWEEN,
  B_DEAD
};

oz_BFlag oz_isBetween(Board * to, Board * varHome);

/*===================================================================
 * see preparedCalls
 *=================================================================== */

class CallList {
public:
  USEFREELISTMEMORY;
  TaggedRef proc;
  RefsArray args;

  CallList *next;
  CallList(TaggedRef p, RefsArray a) : proc(p), args(a), next(NULL) {}
  void dispose() { freeListDispose(this,sizeof(*this)); }
};

/*
 * -----------------------------------------------------------------------
 * Tasks
 * -----------------------------------------------------------------------
 */

//
// "check" says 'TRUE' if there is some pending processing;
typedef Bool (*TaskCheckProc)(unsigned long clock, void *arg);
// 'process' says 'TRUE' if all the tasks are done;
typedef Bool (*TaskProcessProc)(unsigned long clock, void *arg);

Bool NeverDo_CheckProc(unsigned long, void*);

//
class TaskNode {
private:
  void *arg;                    // an opaque argument;
  TaskCheckProc check;          // both procedures take the same argument;
  unsigned int minInterval;     // ... between calls of 'check';
  Bool ready;                   // cached up;
  TaskProcessProc process;

  //
public:
  //
  // There is no task if check == NeverDo_CheckProc;
  TaskNode() {
    check = NeverDo_CheckProc;
    minInterval = 0;
    ready = FALSE;              // ready is used by 'AM::handleTasks()';
    DebugCode(arg = (void *) 0; process = (TaskProcessProc) 0);
  }
  ~TaskNode() {
    DebugCode(check = NeverDo_CheckProc);
    DebugCode(arg = (void *) 0; minInterval = 0);
    DebugCode(ready = NO; process = (TaskProcessProc) 0);
  }

  //
  Bool isFree() { return (check == NeverDo_CheckProc); }
  void setTask(void *aIn, TaskCheckProc cIn, TaskProcessProc pIn) {
    Assert(check == NeverDo_CheckProc);
    arg = aIn;
    check = cIn;
    ready = FALSE;
    process = pIn;
  }
  void dropTask() {
    check = NeverDo_CheckProc;
    minInterval = 0;
    ready = FALSE;
  }

  //
  void *getArg() {
    return (arg);
  }
  TaskCheckProc getCheckProc() {
    return (check);
  }
  TaskProcessProc getProcessProc() {
    Assert(check != NeverDo_CheckProc);
    return (process);
  }

  //
  void setMinimalTaskInterval(unsigned int ms) { minInterval = ms; }
  unsigned int getMinimalTaskInterval() { return (minInterval); }

  //
  void setReady() {
    Assert(check != NeverDo_CheckProc);
    ready = TRUE;
  }
  Bool isReady() {
    return (ready);
  }
  void dropReady() {
    Assert(check != NeverDo_CheckProc);
    ready = FALSE;
  }
};


/* -----------------------------------------------------------------------
 * OO
 * -----------------------------------------------------------------------*/

typedef int32 ChachedOORegs;

inline
Object *getObject(ChachedOORegs regs)
{
  return (Object*) ToPointer(regs&~3);
}


inline
ChachedOORegs setObject(ChachedOORegs regs, Object *o)
{
  return (ToInt32(o)|(regs&0x3));
}

/* -----------------------------------------------------------------------
 * clock
 * -----------------------------------------------------------------------*/

inline
int oz_msToClockTick(int ms)
{
  int clockMS = CLOCK_TICK/1000;
  return (ms+clockMS-1) / clockMS;
}

inline
int oz_clockTickToMs(int cl)
{
  int clockMS = CLOCK_TICK/1000;
  return cl * clockMS;
}

/* -----------------------------------------------------------------------
 * class AM
 * -----------------------------------------------------------------------*/

// this class contains the central global data
class AM  {
friend int engine(Bool init);
friend void scheduler();

private:
  Board *_currentBoard;
  Board *_rootBoard;

  // source level debugger
  TaggedRef debugStreamTail;
  Bool debugMode;

  // location of propagators on/off
  Bool propLocation;

  int statusReg;
  TaggedRef xRegs[NumberOfXRegisters];
  TaskStack *cachedStack;
  Object *cachedSelf;
  Bool _inEqEq;
  TaggedRef _currentUVarPrototype; // opt: cache

  TaggedRef _suspendVarList;
  CallList *preparedCalls;      // for BI_REPLACEBICALL

  int threadSwitchCounter;
  int userCounter;

  TaskNode *taskNodes;

  struct {
    int debug;
    TaggedRef value;
    TaggedRef info;
    ProgramCounter pc;
    RefsArray y;
    Abstraction *cap;
  } exception;

  Bool criticalFlag;  // if this is true we will NOT set Sflags
                      // from within signal handlers

  TaggedRef defaultExceptionHdl;

private:
  unsigned int lastThreadID;
public:
  unsigned int newId() {
    lastThreadID = (lastThreadID + 1) & THREAD_ID_MAX;
    return lastThreadID;
  }

private:

  // minimal interval at which tasks are checked/handled.
  // '0' means no minimal interval is required.
  unsigned int taskMinInterval;

  //
  OzSleep *sleepQueue;

  Bool _profileMode;

public:
  // internal clock in 'ms';
  unsigned long emulatorClock;

  void setProfileMode()   { _profileMode=TRUE; }
  void unsetProfileMode() { _profileMode=FALSE; }
  Bool profileMode()      { return _profileMode; }

private:
  Bool installingScript;  // ask TM
public:
  void setInstallingScript()   { installingScript=TRUE; }
  void unsetInstallingScript() { installingScript=FALSE; }
  Bool isInstallingScript(void) {return installingScript;}

public:
  ThreadsPool threadsPool;
  Trail trail;

public:

  AM() {};

  Board *currentBoard()  { return _currentBoard; }
  Board *rootBoard()     { return _rootBoard; }

  Bool inEqEq()          { return _inEqEq; }
  void setInEqEq(Bool b) { _inEqEq=b; }

  TaggedRef getX(int i) { return xRegs[i]; }
  TaggedRef getDefaultExceptionHdl() { return defaultExceptionHdl; }
  void setDefaultExceptionHdl(TaggedRef pred) {
    defaultExceptionHdl = pred;
  }

  TaggedRef getDebugStreamTail() { return debugStreamTail; }

  void debugStreamMessage(TaggedRef message) {
    Assert(_currentBoard==_rootBoard);
    TaggedRef newTail = OZ_newVariable();
    OZ_Return ret     = OZ_unify(debugStreamTail,oz_cons(message,newTail));
    debugStreamTail   = newTail;
    Assert(ret == PROCEED);
  }


  void setException(TaggedRef val,Bool d) {
    exception.value = val;
    exception.info = NameUnit;
    exception.debug = d;
  }
  void setExceptionInfo(TaggedRef inf) {
    if (exception.info == NameUnit) {
      exception.info=oz_nil();
    }
    exception.info = oz_cons(inf,exception.info);
  }
  TaggedRef getExceptionValue() { return exception.value; }
  Bool hf_raise_failure();

  // see builtins.cc
  inline OZ_Return eqeq(TaggedRef Ain,TaggedRef Bin);

  void setSelf(Object *o) {
    cachedSelf = o;
  }

  Object *getSelf() {
    return cachedSelf;
  }

  void changeSelf(Object * o) {
    Object *oldSelf = cachedSelf;
    if(o != oldSelf) {
      cachedStack->pushSelf(oldSelf);
      cachedSelf = o;
    }
  }

  int currentUVarPrototypeEq(TaggedRef t) {
    return _currentUVarPrototype == t;
  }
  TaggedRef currentUVarPrototype() {
    Assert(tagged2VarHome(_currentUVarPrototype)==_currentBoard);
#if defined(DEBUG_NO_UVAR)
    return makeTaggedRef(newTaggedUVar(_currentUVarPrototype));
#else
    return _currentUVarPrototype;
#endif
  }

  TaggedRef emptySuspendVarList(void) {
    TaggedRef tmp=_suspendVarList;
    _suspendVarList=oz_nil();
    return tmp;
  }

  TaggedRef getSuspendVarList(void) {
    return _suspendVarList;
  }

  void putSuspendVarList(TaggedRef slv) {
    _suspendVarList=slv;
  }

  int isEmptySuspendVarList(void) { return OZ_eq(_suspendVarList,oz_nil()); }

  void addSuspendVarList(TaggedRef t)
  {
    Assert(oz_isVariable(oz_deref(t)));

    _suspendVarList=oz_cons(t,_suspendVarList);
  }
  void addSuspendVarList(TaggedRef *t)
  {
    addSuspendVarList(makeTaggedRef(t));
  }
  OZ_Return suspendOnVarList(Thread *thr);
  void prepareCall(TaggedRef pred, TaggedRef arg0=0, TaggedRef arg1=0,
                   TaggedRef arg2=0, TaggedRef arg3=0, TaggedRef arg4=0);

  void prepareCall(TaggedRef pred, RefsArray args);

  void pushPreparedCalls(Thread *thr=0);
  void emptyPreparedCalls();
  Bool isEmptyPreparedCalls();

  void init(int argc,char **argv);
  void checkVersion();
  void exitOz(int status);
  void suspendEngine();
  void checkStatus(Bool block);

  Bool isCritical() { return criticalFlag; }

  void setSFlag(StatusBit flag)
  {
#ifdef DEBUG_STATUS
    printf("%c",flagChar(flag)); fflush(stdout);
#endif
    criticalFlag = OK;
    statusReg = (flag | statusReg);
    criticalFlag = NO;
  }

  void unsetSFlag(StatusBit flag)
  {
#ifdef DEBUG_STATUS
    printf("%c",tolower(flagChar(flag))); fflush(stdout);
#endif
    criticalFlag = OK;
    statusReg = (~flag & statusReg);
    criticalFlag = NO;
  }

  Bool isSetSFlag(StatusBit flag) { return ( statusReg & flag ) ? OK : NO; }
  int isSetSFlag()                { return statusReg; }

  Bool debugmode()          { return debugMode; }
  void setdebugmode(Bool x) { debugMode = x; }

  Bool isPropagatorLocation(void)    { return propLocation; }
  void setPropagatorLocation(Bool x) { propLocation = x; }

  void setCurrent(Board *c) {
    _currentBoard         = c;
    _currentUVarPrototype = makeTaggedUVar(c);
  }

  void gc(int msgLevel);  // ###
  void doGC();

  // unset the ThreadSwitch flag and reset the counter
  // only needed in emulate.cc
  void restartThread() {
    unsetSFlag(ThreadSwitch);
    threadSwitchCounter=oz_msToClockTick(TIME_SLICE);
  }

  void handleTasks();

  //
  // Tasks are handled with certain minimal interval. Setting it to
  // zero for a particular task means it does not need to be performed
  // now (but it must be ready that it can happen despite that);
  void setMinimalTaskInterval(void *arg, unsigned int ms);

  //
  Bool registerTask(void *arg, TaskCheckProc cIn, TaskProcessProc pIn);
  Bool removeTask(void *arg, TaskCheckProc cIn);
  void checkTasks();

  //
  unsigned long getEmulatorClock() { return (emulatorClock); }

  // yields time for blocking in 'select()';
  unsigned int waitTime();

  //
  // 'ms' is time since last call in milliseconds;
  void handleAlarm(int ms = -1);
  // 'SIGUSR2' notifies about presence of tasks. Right now these are
  // only virtual site messages;
  void handleUSR2();
  void handleUser();
  void insertUser(int t,TaggedRef node);
  void wakeUser();
  int  nextUser();
  Bool checkUser();
#ifdef DENYS_SIGNAL
  void doSignalHandler();
  void leaveSignalHandler();
#endif

};

extern AM am;

/* -----------------------------------------------------------------------
 * Abbreviations
 * -----------------------------------------------------------------------*/

#ifdef DEBUG_CHECK
inline Board *oz_rootBoard() { return am.rootBoard(); }
inline Board *oz_currentBoard() { return am.currentBoard(); }
inline Bool oz_isRootBoard(Board *bb) { return oz_rootBoard() == bb; }
inline Bool oz_isCurrentBoard(Board *bb) { return oz_currentBoard() == bb; }
inline Bool oz_onToplevel() { return oz_currentBoard() == oz_rootBoard(); }
inline Thread *oz_currentThread() { return am.threadsPool.currentThread(); }
inline OZ_Term oz_newName()
{
  return makeTaggedLiteral(Name::newName(oz_currentBoard()));
}
inline OZ_Term oz_newPort(OZ_Term val)
{
  return makeTaggedConst(new PortWithStream(oz_currentBoard(), val));
}
inline OZ_Term oz_newCell(OZ_Term val)
{
  return makeTaggedConst(new CellLocal(oz_currentBoard(),val));
}
inline
OZ_Term oz_newVariable() { return oz_newVar(oz_currentBoard()); }

#else

#define oz_rootBoard()        (am.rootBoard())
#define oz_currentBoard()     (am.currentBoard())
#define oz_isRootBoard(bb)    (oz_rootBoard()    == (bb))
#define oz_isCurrentBoard(bb) (oz_currentBoard() == (bb))
#define oz_onToplevel()       (oz_currentBoard() == oz_rootBoard())
#define oz_currentThread()    (am.threadsPool.currentThread())

#define oz_newName() makeTaggedLiteral(Name::newName(oz_currentBoard()))
#define oz_newPort(val) \
  makeTaggedConst(new PortWithStream(oz_currentBoard(), (val)))
#define oz_newCell(val) makeTaggedConst(new CellLocal(oz_currentBoard(),(val)))
#define oz_newVariable()         oz_newVar(oz_currentBoard())
#endif


inline
TaggedRef oz_newVariableOPT()
{
  TaggedRef *ret = (TaggedRef *) int32Malloc(sizeof(TaggedRef));
  *ret = am.currentUVarPrototype();
  return makeTaggedRef(ret);
}

/* -----------------------------------------------------------------------
 * Debugger
 * -----------------------------------------------------------------------*/

inline
int oz_newId()
{
  unsigned int currentThreadID = oz_currentThread() ?
    oz_currentThread()->getID() & THREAD_ID_MASK : 1;
  unsigned int newID=am.newId();
  return newID | (currentThreadID << THREAD_ID_SIZE);
}

/* -----------------------------------------------------------------------
 * Suspensions
 * ----------------------------------------------------------------------- */

#endif
