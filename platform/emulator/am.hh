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
#define DECSOLVETHREADS(BB, S) decSolveThreads(BB, S)
#else
#define DECSOLVETHREADS(BB, S) decSolveThreads(BB)
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
typedef Bool (*TaskCheckProc)(void *arg);
typedef void (*TaskProcessProc)(void *arg);

Bool NeverDo_CheckProc(void *va);

//
class TaskNode {
private:
  void *arg;                    // an opaque argument;
  TaskCheckProc check;          // both procedures take the same argument;
  Bool ready;                   // cached up;
  TaskProcessProc process;

  //
public:
  // There is no task if check==NULL;
  TaskNode() : check((TaskCheckProc) NULL) {
    check = NeverDo_CheckProc;  //
    DebugCode(arg = (void *) 0; ready = NO; process = (TaskProcessProc) 0);
  }
  ~TaskNode() {
    check = NeverDo_CheckProc;
    DebugCode(arg = (void *) 0; ready = NO; process = (TaskProcessProc) 0);
  }

  //
  Bool isFree() { return (check == (TaskCheckProc) NULL); }
  void setTask(void *aIn, TaskCheckProc cIn, TaskProcessProc pIn) {
    Assert(check == (TaskCheckProc) NULL);
    arg = aIn;
    check = cIn;
    ready = FALSE;
    process = pIn;
  }
  void dropTask() { check = (TaskCheckProc) NULL; }

  //
  void *getArg() {
    return (arg);
  }
  TaskCheckProc getCheckProc() {
    Assert(check != (TaskCheckProc) NULL);
    return (check);
  }
  TaskProcessProc getProcessProc() {
    Assert(check != (TaskCheckProc) NULL);
    return (process);
  }

  //
  void setReady() {
    Assert(check != (TaskCheckProc) NULL);
    ready = TRUE;
  }
  Bool isReady() {
    Assert(check != (TaskCheckProc) NULL);
    return (ready);
  }
  void dropReady() {
    Assert(check != (TaskCheckProc) NULL);
    ready = FALSE;
  }
};

//
// By now we need only two - one for input between virtual sites, and
// another - for pending (because of locks at the receiver site)
// sends;
#define MAXTASKS        2


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
class AM : public ThreadsPool {
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
  Trail trail;
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

  TaggedRef opiCompiler;

  unsigned int lastThreadID;

  Bool installingScript;  // ask TM

  OzSleep *sleepQueue;

  Bool profileMode;
public:

  AM() {};

  Board *currentBoard()         { return _currentBoard; }
  Board *currentSolveBoard()    { return _currentSolveBoard; }
  Board *rootBoardGC()          { return _rootBoard; }
  int isCurrentBoard(Board *bb) { return bb==_currentBoard; }
  int isBelowSolveBoard()       { return _currentSolveBoard!=0; }
  Bool inShallowGuard()         { return shallowHeapTop!=0; }
#ifdef DEBUG_CHECK
  Bool checkShallow(ByteCode *scp) {
    return shallowHeapTop && scp || shallowHeapTop==0 && scp==0;
  }
#endif
  TaggedRef getOpiCompiler()       { return opiCompiler; }
  void setOpiCompiler(TaggedRef o) { opiCompiler = o; }

  TaggedRef getX(int i) { return xRegs[i]; }
  TaggedRef getDefaultExceptionHdl() { return defaultExceptionHdl; }
  void setDefaultExceptionHdl(TaggedRef pred) {
    defaultExceptionHdl = pred;
  }

  TaggedRef getDebugStreamTail() { return debugStreamTail; }

  void debugStreamMessage(TaggedRef message) {
    Assert(onToplevel());
    TaggedRef newTail = OZ_newVariable();
    OZ_Return ret     = OZ_unify(debugStreamTail,cons(message,newTail));
    debugStreamTail   = newTail;
    Assert(ret == PROCEED);
  }


  void setException(TaggedRef val, TaggedRef inf, Bool d) {
    exception.value = val;
    exception.info = inf;
    exception.debug = d;
  }
  TaggedRef getExceptionValue() { return exception.value; }
  Bool hf_raise_failure();

  // see builtins.cc
  inline OZ_Return eqeq(TaggedRef Ain,TaggedRef Bin);

  void changeSelf(Object *o);
  void saveSelf();
  void setSelf(Object *o) { cachedSelf = o; }
  Object *getSelf() { return cachedSelf; }

  void setProfileMode(Bool p) { profileMode = p; }

  int currentUVarPrototypeEq(TaggedRef t) {
    return _currentUVarPrototype == t;
  }
  TaggedRef currentUVarPrototype() {
    Assert(isCurrentBoard(tagged2VarHome(_currentUVarPrototype)));
#ifdef OPT_VAR_IN_STRUCTURE
    return _currentUVarPrototype;
#else
    return makeTaggedRef(newTaggedUVar(_currentUVarPrototype));
#endif
  }

  TaggedRef emptySuspendVarList(void) {
    TaggedRef tmp=_suspendVarList;
    _suspendVarList=nil();
    return tmp;
  }

  int isEmptySuspendVarList(void) { return OZ_eq(_suspendVarList,nil()); }

  void addSuspendVarList(TaggedRef t)
  {
    Assert(oz_isVariable(oz_deref(t)));

    _suspendVarList=cons(t,_suspendVarList);
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
  void checkDebug(Thread *tt, Board *bb) {
    if (debugmode() && bb==_rootBoard) checkDebugOutline(tt);
  }
  void checkDebugOutline(Thread *tt);


  void setCurrent(Board *c, Bool checkNotGC=OK);
  InstType installPath(Board *to); // ###
  Bool installScript(Script &script);
  Bool installScriptOutline(Script &script);
  Bool isInstallingScript(void) {return installingScript;}
  Bool install(Board *bb);
  void deinstallPath(Board *top);
  INLINE void deinstallCurrent();
  void reduceTrailOnUnitCommit();
  void reduceTrailOnSuspend();
  void reduceTrailOnFail();
  void reduceTrailOnShallow();
  void reduceTrailOnEqEq();

  // in emulate.cc
  Bool emulateHookOutline();
  Bool hookCheckNeeded();
  Bool isNotPreemptiveScheduling(void);

  INLINE RunnableThreadBody* allocateBody();
  INLINE Thread *newThreadInternal(int prio, Board *bb);
  INLINE Thread *mkRunnableThread(int prio, Board *bb);
  INLINE Thread *mkRunnableThreadOPT(int prio, Board *bb);
  Thread *mkLPQ(Board *bb, int prio);
  inline Thread *mkWakeupThread(Board *bb);
  Propagator * mkPropagator(Board *bb, int prio, OZ_Propagator *pro);
  INLINE Thread *mkSuspendedThread(Board *bb, int prio);
  INLINE int newId();

  INLINE void suspThreadToRunnableOPT(Thread *tt);
  INLINE void suspThreadToRunnable(Thread *tt);
  inline void wakeupToRunnable(Thread *tt);
  INLINE void propagatorToRunnable(Thread *tt);
  INLINE void updateSolveBoardPropagatorToRunnable(Thread *tt);

  // wake up cconts and board conts
  void wakeupAny(Suspension susp, Board * bb);
  inline Bool wakeUp(Suspension susp, Board * home, PropCaller calledBy);
  INLINE Bool wakeUpPropagator(Propagator * prop, Board *home,
                        PropCaller calledBy = pc_propagator);
  inline Bool wakeUpBoard(Thread *tt, Board *home);
  inline Bool wakeUpThread(Thread *tt, Board *home);
  INLINE OZ_Return runPropagator(Propagator *);

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

  //  Check all the solve actors above for stabily
  // (and, of course, wake them up if needed);
  INLINE void checkExtSuspension(Suspension);
  INLINE void removeExtThread(Thread *tt);

  void setExtSuspensionOutlined(Suspension susp, Board *varHome);

  //  special allocator for thread's bodies;
  INLINE void freeThreadBody(Thread *tt);

  //
  //  it asserts that the suspended thread is 'external' (see beneath);
  void checkExtSuspensionOutlined(Suspension susp);
  void removeExtThreadOutlined(Thread *tt);

  //
  //  (re-)Suspend a propagator again; (was: 'reviveCurrentTaskSusp');
  //  It does not take into account 'solve threads', i.e. it must
  // be done externally - if needed;
  INLINE void suspendPropagator(Propagator *);
  INLINE void scheduledPropagator(Propagator *);

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
  INLINE void closeDonePropagator(Propagator *);

  INLINE void closeDonePropagatorCD(Propagator *);
  INLINE void closeDonePropagatorThreadCD(Propagator *);

  SuspList *installPropagators(SuspList *local_list, SuspList *glob_list,
                               Board *glob_home);

  Bool onToplevel() { return _currentBoard == _rootBoard; }

  void gc(int msgLevel);  // ###
  void doGC();
  // coping of trees (and terms);
  Board* copyTree(Board* node, Bool *isGround);

  static int awakeIO(int fd, void *var);
  void awakeIOVar(TaggedRef var);

  // entailment check
  Bool entailment() {
    return (!currentBoard()->hasSuspension()  // threads?
            && trail.isEmptyChunk());       // constraints?
  }

  void checkStability();
  int handleFailure(Continuation *&cont, AWActor *&aa);
  int commit(Board *bb, Thread *tt=0);
  void failBoard();

  // Unification
  void doTrail(TaggedRef *vp, TaggedRef v) {
    trail.pushRef(vp,v);
  }
  void doBindAndTrail(TaggedRef v, TaggedRef * vp, TaggedRef t);
  void doBindAndTrailAndIP(TaggedRef v, TaggedRef * vp, TaggedRef t,
                           GenCVariable * lv, GenCVariable * gv);

  Bool isLocalUVarOutline(TaggedRef var,TaggedRef *varPtr);
  Bool isLocalSVarOutline(SVariable *var);
  Bool isLocalUVar(TaggedRef var,TaggedRef *varPtr);
  Bool isLocalSVar(TaggedRef var);
  Bool isLocalSVar(SVariable *var);
  Bool isLocalCVar(TaggedRef var);
  Bool isLocalVariable(TaggedRef var,TaggedRef *varPtr);

  void checkSuspensionList(TaggedRef taggedvar,
                           PropCaller calledBy = pc_propagator);
  Bool hasOFSSuspension(SuspList *suspList);
  void addFeatOFSSuspensionList(TaggedRef var, SuspList* suspList,
                                TaggedRef flist, Bool determined);
  SuspList * checkSuspensionList(SVariable * var,
                                 SuspList * suspList, PropCaller calledBy);
  int incSolveThreads(Board *bb);
#ifdef DEBUG_THREADCOUNT
  void decSolveThreads(Board *bb, char *);
#else
  void decSolveThreads(Board *bb);
#endif
  DebugCode(Bool isInSolveDebug(Board *bb);)

  void restartThread();

  void handleIO();
  void handleTasks();
  void select(int fd, int mode, OZ_IOHandler fun, void *val);
  void acceptSelect(int fd, OZ_IOHandler fun, void *val);
  int select(int fd,int mode, TaggedRef l, TaggedRef r);
  void acceptSelect(int fd, TaggedRef l, TaggedRef r);
  void deSelect(int fd);
  void deSelect(int fd,int mode);
  void checkIO();

  Bool registerTask(void *arg, TaskCheckProc cIn, TaskProcessProc pIn);
  Bool removeTask(void *arg, TaskCheckProc cIn);
  void checkTasks();

  void handleAlarm();
  void handleUser();
  void insertUser(int t,TaggedRef node);
  void wakeUser();
  int  nextUser();
  Bool checkUser();

  Bool isStableSolve(SolveActor *sa);
};

extern AM am;

#include "cpi_heap.hh"
#include "cpbag.hh"

#include "hashtbl.hh"

#include "statisti.hh"

#include "lps.hh"

#include "debug.hh"

#include "thread.hh"
#include "susplist.hh"
#include "variable.hh"

#include "solve.hh"

#include "opcodes.hh"
#include "codearea.hh"

#include "builtins.hh"
#include "os.hh"

#ifndef OUTLINE
#include "am.icc"
#endif

#endif
