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
#include "thr_pool.hh"
#include "value.hh"

//make it easy to experiment with inlining these again
//#define OUTLINE_SETEXCEPTIONINFO 0
//#define OUTLINE_HF_RAISE_FAILURE 0

#ifndef OUTLINE_SETEXCEPTIONINFO
#if defined(__GNUC__) && __GNUC__>2
#define OUTLINE_SETEXCEPTIONINFO 1
#else
#define OUTLINE_SETEXCEPTIONINFO 0
#endif
#endif

#ifndef OUTLINE_HF_RAISE_FAILURE
#if defined(__GNUC__) && __GNUC__>2
#define OUTLINE_HF_RAISE_FAILURE 1
#else
#define OUTLINE_HF_RAISE_FAILURE 0
#endif
#endif

/* -----------------------------------------------------------------------
 * StatusReg
 * -----------------------------------------------------------------------*/

typedef enum {
  TimerInterrupt = 1 << 1, // reflects the arrival of an alarm from OS;
  IOReady	 = 1 << 2, // IO handler has signaled IO ready
  UserAlarm	 = 1 << 3, // some Oz delays are to be processed;
  StartGC	 = 1 << 4, // need a GC
  TasksReady     = 1 << 5, //
  SigPending	 = 1 << 6  //
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
  RefsArray * args;

  CallList * next;
  CallList(TaggedRef p, RefsArray * a) : proc(p), args(a), next(NULL) {}
  void dispose() { oz_freeListDispose(this,sizeof(*this)); }
};

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

// Using only one unsigned long for time (in ms) gives a maximum lifetime
// of 49 days for an emulator. Therefore, use two unsigned longs in an ADT.
#define WRAP_TIME ULONG_MAX

class LongTime {
private:
  unsigned long low;
  unsigned long high;

public:
  LongTime() {low=high=0;}

  inline void increaseTime(unsigned int interval) {
    if(WRAP_TIME-interval>low)      // No overflow
      low+=interval;
    else {                          // Would create overflow
      low -= (WRAP_TIME-interval); 
      high++;                       // Assumes interval << WRAP_TIME
    }
  }

  inline Bool operator<=(const LongTime &t2) {
    return (high<t2.high) || 
      ( (high==t2.high) && (low<=t2.low) );
  }

  inline Bool operator>(const LongTime &t2) {
    return (high>t2.high) || 
      ( (high==t2.high) && (low>t2.low) );
  }

  inline Bool operator!=(const LongTime &t2) {
    return ((low != t2.low) || (high != t2.high));
  }

  // This is assumed to be used only to compare times that are rather close
  // to each other and thus fit in an int.
  inline int operator-(const LongTime &t2) {
    if(this->high==t2.high)
      return this->low-t2.low; 
    else if(this->high==t2.high+1) {
      return (WRAP_TIME-t2.low)+this->low; // this->low+WRAP_TIME - t2.low
                                           // rewritten to avoid
                                           // overflow
    }
    else 
      OZ_error("Taking difference with times too far apart.");
    return -1;
  }

  char *toString();
};

/*
 * -----------------------------------------------------------------------
 * Tasks
 * -----------------------------------------------------------------------
 */
#ifndef DENYS_EVENTS
//
// "check" says 'TRUE' if there is some pending processing;
typedef Bool (*TaskCheckProc)(LongTime *clock, void *arg);
// 'process' says 'TRUE' if all the tasks are done;
typedef Bool (*TaskProcessProc)(LongTime *clock, void *arg);

Bool NeverDo_CheckProc(LongTime *, void*);

//
class TaskNode {
private:
  void *arg;			// an opaque argument;
  TaskCheckProc check;		// both procedures take the same argument;
  unsigned int minInterval;	// ... between calls of 'check';
  Bool ready;			// cached up;
  TaskProcessProc process;

  //
public:
  //
  // There is no task if check == NeverDo_CheckProc;
  TaskNode() {
    check = NeverDo_CheckProc;
    minInterval = 0;
    ready = FALSE;		// ready is used by 'AM::handleTasks()';
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
#endif

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
friend int run_thread(Thread *);

private:
  Board  *_currentBoard;
  Board  *_rootBoard;
  Thread *_currentThread;
  Bool   _currentBoardIsRoot;

  // source level debugger
  TaggedRef debugStreamTail;
  Bool debugMode;

  // location of propagators on/off
  Bool propLocation;

  // kost@ : The flags that affect the decision to run
  // 'checkStatus()'.  In particular, 'childReady' does not, which
  // means that 'childReady' is checked first when some other flag(s)
  // is set, e.g. UserAlarm.
  int statusReg;
  // "second-class" flags. NOTE: these flag(s) do not require
  // protection by the 'criticalFlag';
  int childReady;

  //
  TaskStack *cachedStack;
  Object *cachedSelf;

  //
  int gcStep;
  int copyStep;

public:
  TaskStack * getCachedStack(void) {
    return cachedStack;
  }

private:
  Bool _inEqEq;
  TaggedRef _currentOptVar;	// (former uvar"s;)
  TaggedRef _saveCurrentOptVar;	// (former uvar"s;)

  TaggedRef _suspendVarList;
  CallList *preparedCalls;      // for BI_REPLACEBICALL

  int userCounter;

#ifndef DENYS_EVENTS
  TaskNode *taskNodes;
#endif

  struct {
    int debug;
    TaggedRef value;
    TaggedRef info;
    ProgramCounter pc;
    RefsArray * y;
    Abstraction *cap;
  } exception;

  // if this is true we will NOT set Sflags from within signal
  // handlers;
  Bool criticalFlag;

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
#ifdef DENYS_EVENTS
  int requestedTimer;
public:
  void setTimer(int i) { requestedTimer=i; }
private:
#endif

  Bool _profileMode;

public:
  // internal clock in 'ms';
  LongTime emulatorClock;
  int suspCnt;

  void setProfileMode()   { _profileMode=TRUE; }
  void unsetProfileMode() { _profileMode=FALSE; }
  Bool profileMode()      { return _profileMode; }
  
public:
  ThreadsPool threadsPool;

public:

  AM() {};

  Board *currentBoard()   { return _currentBoard; }
  Thread *currentThread() { return _currentThread; }
  Board *rootBoard()      { return _rootBoard; }

  Bool inEqEq()          { return _inEqEq; }
  void setInEqEq(Bool b) { 
    _inEqEq=b; 
    if (b) {
      _saveCurrentOptVar = _currentOptVar;
      _currentOptVar     = makeTaggedNULL();
    } else {
      _currentOptVar = _saveCurrentOptVar;
    }
  }

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
  void setExceptionInfo(TaggedRef inf)
#if OUTLINE_SETEXCEPTIONINFO
    ;
#else
  {
    if (exception.info == NameUnit) {
      exception.info=oz_nil();
    }
    exception.info = oz_cons(inf,exception.info);
  }
#endif
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

  //
  // kost@ : "optimized var" is a local var without suspensions (a
  // former local uvar). Faked non-locality for '==' undoes this
  // optimization (so, full-fledged binding & unification are used).
  // An OptVar body is constant and is shared between all OptVar"s in
  // a space. Note that an OptVar from a merged space is not
  // recognized as such; instead, a regular binding routine is used.
  int isOptVar(TaggedRef t) {
    // Important invariant:
    //  In EqEq-mode, the _currentOptVar has been invalidated to NULL
    //  Switching back restores _currentOptVar (see also setEqEq)
    Assert(_inEqEq || _currentOptVar);
    return t == _currentOptVar;
  }

  TaggedRef getCurrentOptVar() {
    // kost@ : cannot inline tests of the _currentOptVar;
    return (_currentOptVar);
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

  int isEmptySuspendVarList(void) { 
    return oz_eq(_suspendVarList,oz_nil()); 
  }

  OZ_Return addSuspendVarListInline(TaggedRef * t) {
    _suspendVarList=oz_cons(makeTaggedRef(t),_suspendVarList);
    return SUSPEND;
  }
  OZ_Return addSuspendVarListInline(TaggedRef t) {
    DEREF(t,tptr);
    Assert(oz_isVar(t));
    return addSuspendVarListInline(tptr);
  }

  OZ_Return suspendOnVarList(Thread *thr);

  void prepareCall(TaggedRef pred, RefsArray * args);

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

  //
  void setChildReady() { childReady = OK; }

  int isSetSFlag(StatusBit flag) { return statusReg & flag; }
  int isSetSFlag()               { return statusReg; }

  Bool debugmode()          { return debugMode; }
  void setdebugmode(Bool x) { debugMode = x; }

  Bool isPropagatorLocation(void)    { return propLocation; }
  void setPropagatorLocation(Bool x) { propLocation = x; }

  // kost@ : it takes the corresponding OptVar in order to be able to
  // inline it;
  void setCurrent(Board *c, TaggedRef ov) {
    _currentBoard         = c;
    _currentOptVar        = ov;
    _currentBoardIsRoot   = (c == _rootBoard);
  }
  void setCurrentThread(Thread * t) {
    _currentThread = t;
  }
  int isCurrentRoot(void) {
    Assert(_currentBoardIsRoot == (_currentBoard == _rootBoard));
    return _currentBoardIsRoot;
  }

  //
  void gCollect(int msgLevel);  // ###
  void doGCollect();
  //
  void nextGCStep() {
    gcStep ^= EvenGCStep;
    copyStep = 0;
  }
  int getGCStep() { return (gcStep); }
  void nextCopyStep() { copyStep++; }
  int getCopyStep() { return (copyStep); }

#ifndef DENYS_EVENTS
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
#endif
  //
  LongTime *getEmulatorClock() { return (&emulatorClock); }
  int getSuspCnt() { return (suspCnt); }

  // yields time for blocking in 'select()';
  unsigned int waitTime();

  //
  // 'ms' is time since last call in milliseconds;
  void handleAlarm(int ms = -1);
  // 'SIGUSR2' notifies about presence of tasks. Right now these are 
  // only virtual site messages;
#ifndef DENYS_EVENTS
  void handleUSR2();
#endif
  void handleUser();
#ifndef DENYS_EVENTS
  void insertUser(int t,TaggedRef node);
#endif
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
inline Bool oz_isOptVar(TaggedRef t) { return (am.isOptVar(t)); }
inline int  oz_onToplevel() { return am.isCurrentRoot(); }
inline int oz_getGCStep() { return am.getGCStep(); }
inline int oz_getCopyStep() { return am.getCopyStep(); }
inline Thread *oz_currentThread() { return am.currentThread(); }
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

#else

#define oz_rootBoard()        (am.rootBoard())
#define oz_currentBoard()     (am.currentBoard())
#define oz_isRootBoard(bb)    (oz_rootBoard()    == (bb))
#define oz_isCurrentBoard(bb) (oz_currentBoard() == (bb))
#define oz_isOptVar(t)        (am.isOptVar(t))
#define oz_onToplevel()       (am.isCurrentRoot())
#define oz_getGCStep()        (am.getGCStep())
#define oz_getCopyStep()      (am.getCopyStep())
#define oz_currentThread()    (am.currentThread())

#define oz_newName() makeTaggedLiteral(Name::newName(oz_currentBoard()))
#define oz_newPort(val) \
  makeTaggedConst(new PortWithStream(oz_currentBoard(), (val)))
#define oz_newCell(val) makeTaggedConst(new CellLocal(oz_currentBoard(),(val)))
#endif

//
// The 'make new var in a space' is not inlined since it uses the
// 'Board' class;
inline
TaggedRef oz_newVariable()
{
  TaggedRef *ret = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
  *ret = am.getCurrentOptVar();	// cached in AM;
  return (makeTaggedRef(ret));
}

TaggedRef oz_newVariable(Board *b);

OZ_Return oz_addSuspendVarList(TaggedRef*);
OZ_Return oz_addSuspendVarList(TaggedRef);
OZ_Return oz_addSuspendVarList2(TaggedRef,TaggedRef);
OZ_Return oz_addSuspendVarList3(TaggedRef,TaggedRef,TaggedRef);
OZ_Return oz_addSuspendVarList4(TaggedRef,TaggedRef,TaggedRef,TaggedRef);

OZ_Return oz_addSuspendInArgs1(OZ_Term * _OZ_LOC[]);
OZ_Return oz_addSuspendInArgs2(OZ_Term * _OZ_LOC[]);
OZ_Return oz_addSuspendInArgs3(OZ_Term * _OZ_LOC[]);
OZ_Return oz_addSuspendInArgs4(OZ_Term * _OZ_LOC[]);


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

