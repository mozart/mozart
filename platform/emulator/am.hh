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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __AMH
#define __AMH

#include "base.hh"
#include "trail.hh"
#include "thrspool.hh"

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
  char *_shallowHeapTop;
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

  Board *currentBoard()             { return _currentBoard; }
  Board *currentSolveBoard()        { return _currentSolveBoard; }
  Board *rootBoardGC()              { return _rootBoard; }
  int isBelowSolveBoard()           { return _currentSolveBoard!=0; }

  Bool inShallowGuard()             { return _shallowHeapTop!=0; }
  void setShallowHeapTop(char *sht) { _shallowHeapTop=sht; }
#ifdef DEBUG_CHECK
  Bool checkShallow(ByteCode *scp) {
    return inShallowGuard() ? scp!=0 : scp==0;
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

  void gc(int msgLevel);  // ###
  void doGC();
  // coping of trees (and terms);
  Board* copyTree(Board* node, Bool *isGround);

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

  // unset the ThreadSwitch flag and reset the counter
  // only needed in emulate.cc
  void restartThread() {
    unsetSFlag(ThreadSwitch);
    threadSwitchCounter=oz_msToClockTick(TIME_SLICE);
  }

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
 * Unification
 * -----------------------------------------------------------------------*/

OZ_Return oz_unify(OZ_Term t1, OZ_Term t2, ByteCode *scp=0);
void oz_bind(OZ_Term *varPtr, OZ_Term var, OZ_Term term);
void oz_bind_global(OZ_Term var, OZ_Term term);

inline
void oz_bindToNonvar(OZ_Term *varPtr, OZ_Term var,
                     OZ_Term a, ByteCode *scp=0)
{
  // most probable case first: local UVar
  // if (isUVar(var) && isCurrentBoard(tagged2VarHome(var))) {
  // more efficient:
  if (am.currentUVarPrototypeEq(var) && scp==0) {
    doBind(varPtr,a);
  } else {
    oz_bind(varPtr,var,a);
  }
}

/* -----------------------------------------------------------------------
 * Abbreviations
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


inline Thread* oz_currentThread() { return am.threadsPool.currentThread(); }


/* -----------------------------------------------------------------------
 * values
 * -----------------------------------------------------------------------*/

#define oz_newName() makeTaggedLiteral(Name::newName(oz_currentBoard()))

#define oz_newPort(val) \
  makeTaggedConst(new PortWithStream(oz_currentBoard(), (val)))

#define oz_sendPort(p,v) sendPort(p,v)

#define oz_newCell(val) makeTaggedConst(new CellLocal(oz_currentBoard(), (val)))
  // access, assign

#define oz_string(s)      OZ_string(s)

inline
OZ_Term oz_newChunk(OZ_Term val)
{
  Assert(val==oz_deref(val));
  Assert(oz_isRecord(val));
  return makeTaggedConst(new SChunk(oz_currentBoard(), val));
}

#define oz_newVar(bb)            makeTaggedRef(newTaggedUVar(bb))
#define oz_newVariable()         oz_newVar(oz_currentBoard())
#define oz_newToplevelVariable() oz_newVar(oz_rootBoard())

/* -----------------------------------------------------------------------
 * Debugger
 * -----------------------------------------------------------------------*/

void oz_checkDebugOutline(Thread *tt);

inline
void oz_checkDebug(Thread *tt, Board *bb) {
  if (am.debugmode() && oz_isRootBoard(bb)) oz_checkDebugOutline(tt);
}

#ifndef OUTLINE
#include "am.icc"
#endif

#endif
