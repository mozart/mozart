/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#ifndef __AMH
#define __AMH

#include "oz.h"
#include "oz_cpi.hh"

#include "base.hh"

#include "value.hh"

#include "cont.hh"

#include "taskstk.hh"

#include "trail.hh"

#include "thrspool.hh"

#include "actor.hh"
#include "board.hh"

// more includes at end!

#ifdef DEBUG_THREADCOUNT
#define DECSOLVETHREADS(BB, S) oz_decSolveThreads(BB, S)
#else
#define DECSOLVETHREADS(BB, S) oz_decSolveThreads(BB)
#endif

/* -----------------------------------------------------------------------
 * StatusReg
 * -----------------------------------------------------------------------*/

typedef enum {
  ThreadSwitch  = 1 << 2, // choose a new process
  IOReady       = 1 << 3, // IO handler has signaled IO ready
  UserAlarm     = 1 << 4, // Alarm handler has signaled User Alarm
  StartGC       = 1 << 5, // need a GC
  TasksReady    = 1 << 6
} StatusBit;

/* -----------------------------------------------------------------------
 * spaces
 * -----------------------------------------------------------------------*/

enum InstType {
  INST_OK,
  INST_FAILED,
  INST_REJECTED
};


Bool oz_isBelow(Board *below, Board *above);

// oz_isBetween returns
enum oz_BFlag {
  B_BETWEEN,
  B_NOT_BETWEEN,
  B_DEAD
};

oz_BFlag oz_isBetween(Board *to, Board *varHome);

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
  Bool ready;                   // cached up;
  TaskProcessProc process;

  //
public:
  //
  // There is no task if check == NeverDo_CheckProc;
  TaskNode() {
    check = NeverDo_CheckProc;
    ready = FALSE;              // ready is used by 'AM::handleTasks()';
    DebugCode(arg = (void *) 0; process = (TaskProcessProc) 0);
  }
  ~TaskNode() {
    DebugCode(check = NeverDo_CheckProc);
    DebugCode(arg = (void *) 0; ready = NO; process = (TaskProcessProc) 0);
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
 * class AM
 * -----------------------------------------------------------------------*/

// this class contains the central global data
class AM  {
friend int engine(Bool init);
friend void scheduler();
friend inline Board *oz_rootBoard();
friend inline Board *oz_currentBoard();

private:
  Board *_currentBoard;
  Board *_rootBoard;
  Board *_currentSolveBoard;       // current 'solve' board or NULL if none;

  // source level debugger
  TaggedRef debugStreamTail;
  Bool debugMode;

  int statusReg;
  TaggedRef xRegs[NumberOfXRegisters];
  TaskStack *cachedStack;
  Object *cachedSelf;
  char *shallowHeapTop;
  TaggedRef _currentUVarPrototype; // opt: cache

  TaggedRef _suspendVarList;
  CallList *preparedCalls;      // for BI_REPLACEBICALL

  int threadSwitchCounter;
  int userCounter;

  Bool wasSolveSet;

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

  unsigned int lastThreadID;

  // internal clock in 'ms';
  unsigned long emulatorClock;

  // minimal interval at which tasks are checked/handled.
  // '0' means no minimal interval is required.
  unsigned int taskMinInterval;

  //
  OzSleep *sleepQueue;

private:
  Bool _profileMode;
public:
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

  Board *currentBoard()         { return _currentBoard; }
  Board *currentSolveBoard()    { return _currentSolveBoard; }
  Board *rootBoardGC()          { return _rootBoard; }
  int isBelowSolveBoard()       { return _currentSolveBoard!=0; }
  Bool inShallowGuard()         { return shallowHeapTop!=0; }
#ifdef DEBUG_CHECK
  Bool checkShallow(ByteCode *scp) {
    return shallowHeapTop && scp || shallowHeapTop==0 && scp==0;
  }
#endif

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

  void changeSelf(Object *o);
  void saveSelf();
  void setSelf(Object *o) { cachedSelf = o; }
  Object *getSelf() { return cachedSelf; }

  int currentUVarPrototypeEq(TaggedRef t) {
    return _currentUVarPrototype == t;
  }
  TaggedRef currentUVarPrototype() {
    Assert(tagged2VarHome(_currentUVarPrototype)==_currentBoard);
#ifdef OPT_VAR_IN_STRUCTURE
    return _currentUVarPrototype;
#else
    return makeTaggedRef(newTaggedUVar(_currentUVarPrototype));
#endif
  }

  TaggedRef emptySuspendVarList(void) {
    TaggedRef tmp=_suspendVarList;
    _suspendVarList=oz_nil();
    return tmp;
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
  void suspendOnVarList(Thread *thr);
  void prepareCall(TaggedRef pred, TaggedRef arg0=0, TaggedRef arg1=0,
                   TaggedRef arg2=0, TaggedRef arg3=0, TaggedRef arg4=0);

  void prepareCall(TaggedRef pred, RefsArray args);

  void pushPreparedCalls(Thread *thr=0);

  void init(int argc,char **argv);
  void checkVersion();
  void exitOz(int status);
  void suspendEngine();
  void checkStatus();

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


  void setCurrent(Board *c, Bool checkNotGC=OK);

  // in emulate.cc
  Bool hookCheckNeeded();
  Bool isNotPreemptiveScheduling(void);

  INLINE int newId();

  //
  //  Note: killing the suspended thread *might not* make any
  // actor reducible OR reducibility must be tested somewhere else !!!
  //
  //  Invariant:
  // There can be no threads which are suspended not in its
  // "proper" home, i.e. not in the comp. space where it is started;
  //
  //  Note that this is true for wakeups, continuations etc. anyway,
  // *and* this is also true for threads suspended in the sequential
  // mode! The point is that whenever a thread tries to suspend in a
  // deep guard, a new (local) thread is created which carries the
  // rest of the guard (Hi, Michael!);
  //
  //  Note also that these methods don't decrement suspCounters,
  // thread counters or whatever because their home board might
  // not exist at all - such things should be done outside!
  INLINE void disposeSuspendedThread(Thread *tt);
  //
  //  It marks the thread as dead and disposes it;
  INLINE void disposeRunnableThread(Thread *tt);
  //
  INLINE void disposeThread(Thread *tt);

  void gc(int msgLevel);  // ###
  void doGC();
  // coping of trees (and terms);
  Board* copyTree(Board* node, Bool *isGround);

  // entailment check
  Bool entailment() {
    return (!currentBoard()->hasSuspension()  // threads?
            && trail.isEmptyChunk());       // constraints?
  }

  // Unification
  void doTrail(TaggedRef *vp, TaggedRef v) {
    trail.pushRef(vp,v);
  }
  void doBindAndTrail(TaggedRef * vp, TaggedRef t);

#define DoBindAndTrailAndIP(vp,t,lv,gv) {       \
  lv->GenCVariable::installPropagators(gv);     \
  am.doBindAndTrail(vp,t);                      \
  }

  Bool isLocalUVarOutline(TaggedRef var,TaggedRef *varPtr);
  Bool isLocalSVarOutline(SVariable *var);
  Bool isLocalUVar(TaggedRef var,TaggedRef *varPtr);
  Bool isLocalSVar(TaggedRef var);
  Bool isLocalSVar(SVariable *var);
  Bool isLocalCVar(TaggedRef var);
  Bool isLocalVariable(TaggedRef var,TaggedRef *varPtr);

  void restartThread();

  void handleTasks();

  //
  // Tasks are handled with certain minimal interval;
  // Right now there is no way to fall back with intervals...
  void setMinimalTaskInterval(unsigned int ms) {
    Assert(ms);
    Assert(ms >= (CLOCK_TICK/1000));
    taskMinInterval = min(taskMinInterval, ms);
  }
  DebugCode(void dropMinimalTaskInterval() { taskMinInterval = 0; })

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
  void handleAlarm(unsigned int ms);
  // 'SIGUSR2' notifies about presence of tasks. Right now these are
  // only virtual site messages;
  void handleUSR2();
  void handleUser();
  void insertUser(int t,TaggedRef node);
  void wakeUser();
  int  nextUser();
  Bool checkUser();

};

extern AM am;


/* -----------------------------------------------------------------------
 * Spaces
 * -----------------------------------------------------------------------*/

inline
Board *oz_rootBoard() { return am._rootBoard; }

inline
Board *oz_currentBoard() { return am._currentBoard; }

inline
int oz_isRootBoard(Board *bb) { return bb==oz_rootBoard(); }

inline
int oz_isCurrentBoard(Board *bb) { return oz_currentBoard() == bb; }

inline
int oz_onToplevel() { return oz_currentBoard() == oz_rootBoard(); }


/* -----------------------------------------------------------------------
 * Threads
 * -----------------------------------------------------------------------*/

inline Thread* oz_currentThread() { return am.threadsPool.currentThread(); }

INLINE Thread *oz_newThreadInternal(int prio, Board *bb);
INLINE Thread *oz_mkRunnableThread(int prio, Board *bb);
INLINE Thread *oz_mkRunnableThreadOPT(int prio, Board *bb);
Thread *oz_mkLPQ(Board *bb, int prio);
inline Thread *oz_mkWakeupThread(Board *bb);
Propagator * oz_mkPropagator(Board *bb, int prio, OZ_Propagator *pro);
INLINE Thread *oz_mkSuspendedThread(Board *bb, int prio);

INLINE void oz_suspThreadToRunnableOPT(Thread *tt);
INLINE void oz_suspThreadToRunnable(Thread *tt);
inline void oz_wakeupToRunnable(Thread *tt);
INLINE void oz_propagatorToRunnable(Thread *tt);
INLINE void oz_updateSolveBoardPropagatorToRunnable(Thread *tt);

//
//  (re-)Suspend a propagator again; (was: 'reviveCurrentTaskSusp');
//  It does not take into account 'solve threads', i.e. it must
// be done externally - if needed;
INLINE void oz_suspendPropagator(Propagator *);
INLINE void oz_scheduledPropagator(Propagator *);

//
//  Terminate a propagator thread which is (still) marked as runnable
// (was: 'killPropagatedCurrentTaskSusp' with some variations);
//
//  This might be used only from the local propagation queue,
// because it doesn't check for entaiment, stability, etc.
// Moreover, such threads are NOT counted in solve actors
// and are not marked as "inSolve" EVEN in the "running" state!
//
//  Philosophy (am i right, Tobias?):
// When some propagator returns 'PROCEED' and still has the
// 'runnable' flag set, then it's done.
INLINE void oz_closeDonePropagator(Propagator *);

INLINE void oz_closeDonePropagatorCD(Propagator *);
INLINE void oz_closeDonePropagatorThreadCD(Propagator *);


/* -----------------------------------------------------------------------
 * XXX
 * -----------------------------------------------------------------------*/

SuspList *oz_checkAnySuspensionList(SuspList *suspList,Board *home,
                                    PropCaller calledBy);

Bool oz_installScript(Script &script);

InstType oz_installPath(Board *to);
Bool oz_install(Board *bb);
void oz_deinstallPath(Board *top);
void oz_deinstallCurrent();
void oz_reduceTrailOnUnitCommit();
void oz_reduceTrailOnSuspend();
void oz_reduceTrailOnFail();
void oz_reduceTrailOnShallow();
void oz_reduceTrailOnEqEq();


// wake up cconts and board conts
void oz_wakeupAny(Suspension susp, Board * bb);
inline Bool oz_wakeUp(Suspension susp, Board * home, PropCaller calledBy);
INLINE Bool oz_wakeUpPropagator(Propagator * prop, Board *home,
                        PropCaller calledBy = pc_propagator);
inline Bool oz_wakeUpBoard(Thread *tt, Board *home);
inline Bool oz_wakeUpThread(Thread *tt, Board *home);
INLINE OZ_Return oz_runPropagator(Propagator *);

//  Check all the solve actors above for stabily
// (and, of course, wake them up if needed);
INLINE void oz_checkExtSuspension(Suspension);
INLINE void oz_removeExtThread(Thread *tt);

void oz_setExtSuspensionOutlined(Suspension susp, Board *varHome);

//  it asserts that the suspended thread is 'external' (see beneath);
void oz_checkExtSuspensionOutlined(Suspension susp);
void oz_removeExtThreadOutlined(Thread *tt);

void oz_checkStability();
int oz_handleFailure(Continuation *&cont, AWActor *&aa);
int oz_commit(Board *bb, Thread *tt=0);
void oz_failBoard();

Bool oz_isStableSolve(SolveActor *sa);

int oz_incSolveThreads(Board *bb);
#ifdef DEBUG_THREADCOUNT
void oz_decSolveThreads(Board *bb, char *);
#else
void oz_decSolveThreads(Board *bb);
#endif
DebugCode(Bool oz_isInSolveDebug(Board *bb);)



/* -----------------------------------------------------------------------
 * Debugger
 * -----------------------------------------------------------------------*/

void oz_checkDebugOutline(Thread *tt);

inline
void oz_checkDebug(Thread *tt, Board *bb) {
  if (am.debugmode() && oz_isRootBoard(bb)) oz_checkDebugOutline(tt);
}


#include "cpi_heap.hh"
#include "cpbag.hh"

#include "hashtbl.hh"

#include "statisti.hh"

#include "lps.hh"

#include "debug.hh"

#include "thread.hh"
#include "susplist.hh"
#include "variable.hh"

inline
void oz_checkSuspensionList(SVariable *var,
                            PropCaller calledBy=pc_propagator)
{
  var->setSuspList(oz_checkAnySuspensionList(var->getSuspList(),
                                             GETBOARD(var),calledBy));
}

#include "solve.hh"

#include "opcodes.hh"
#include "codearea.hh"

#include "os.hh"

#ifndef OUTLINE
#include "am.icc"
#endif

#endif
