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

// more include's at end!

// -----------------------------------------------------------------------

typedef enum {
  ThreadSwitch  = 1 << 2, // choose a new process
  IOReady       = 1 << 3, // IO handler has signaled IO ready
  UserAlarm     = 1 << 4, // Alarm handler has signaled User Alarm
  StartGC       = 1 << 5, // need a GC
  DebugMode     = 1 << 6,
  StopThread    = 1 << 7
} StatusBit;


// isBetween returns
enum BFlag {
  B_BETWEEN,
  B_NOT_BETWEEN,
  B_DEAD
};

enum InstType {
  INST_OK,
  INST_FAILED,
  INST_REJECTED
};



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

// this class contains the central global data
class AM : public ThreadsPool {
friend int engine(Bool init);
friend void scheduler();
friend Board *ozx_rootBoard();

private:
  Board *_currentBoard;
  Board *_rootBoard;
  Board *_currentSolveBoard;       // current 'solve' board or NULL if none;

  // source level debugger
  TaggedRef debugStreamTail;

  int statusReg;
  Trail trail;
  TaggedRef xRegs[NumberOfXRegisters];
  TaskStack *cachedStack;
  Object *cachedSelf;
  char *shallowHeapTop;
  TaggedRef _currentUVarPrototype; // opt: cache

  TaggedRef suspendVarList;

  int threadSwitchCounter;
  int userCounter;

  RefsArray toplevelVars;
  int toplevelVarsCount;

  Bool wasSolveSet;

  CompStream *compStream;
  Bool isStandaloneF;

#ifdef DEBUG_CHECK
  Bool dontPropagate;
  // is used by consistency checking of a copy of a search tree;
#endif

  struct {
    int debug;
    TaggedRef value;
    TaggedRef info;
    ProgramCounter pc;
  } exception;

  Bool criticalFlag;  // if this is true we will NOT set Sflags
                      // from within signal handlers

  TaggedRef aVarUnifyHandler;
  TaggedRef aVarBindHandler;
  TaggedRef defaultExceptionHdl;

  TaggedRef opiCompiler;

  unsigned int lastThreadID;

  Toplevel *toplevelQueue;

  Bool installingScript;  // ask TM

  OzSleep *sleepQueue;

  Bool profileMode;
public:

  AM() {};

  Board *currentBoard()         { return _currentBoard; }
  Board *currentSolveBoard()    { return _currentSolveBoard; }
  Board *rootBoardGC()          { return _rootBoard; }
  int isCurrentBoard(Board *bb) { return bb==_currentBoard; }
  int isRootBoard(Board *bb)    { return bb==_rootBoard; }
  int isBelowSolveBoard()       { return _currentSolveBoard!=0; }
  Bool inShallowGuard()         { return shallowHeapTop!=0; }

  TaggedRef getOpiCompiler()       { return opiCompiler; }
  void setOpiCompiler(TaggedRef o) { opiCompiler = o; }

  TaggedRef getAVarBindHandler() { return aVarBindHandler; }
  TaggedRef getAVarUnifyHandler() { return aVarUnifyHandler; }
  void setAVarHandler(TaggedRef u, TaggedRef b) {
    aVarUnifyHandler = u;
    aVarBindHandler = b;
  }
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

  CompStream *getCompStream() { return compStream; }

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

  Bool isStandalone() { return isStandaloneF; }

  void emptySuspendVarList(void) { suspendVarList = makeTaggedNULL(); }
  void addSuspendVarList(TaggedRef * t)
  {
    Assert(isAnyVar(*t));

    if (suspendVarList==makeTaggedNULL()) {
      suspendVarList=makeTaggedRef(t);
    } else {
      suspendVarList=cons(makeTaggedRef(t),suspendVarList);
    }
  }
  void addSuspendVarList(TaggedRef t)
  {
    Assert(isAnyVar(deref(t)));

    if (!suspendVarList) {
      suspendVarList=t;
    } else {
      suspendVarList=cons(t,suspendVarList);
    }
  }

  void pushToplevel(ProgramCounter pc);
  void checkToplevel();
  void addToplevel(ProgramCounter pc);

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
  int isSetSFlag()                { return statusReg && (statusReg & ~DebugMode); }

  Bool debugmode() { return isSetSFlag(DebugMode); }
  void checkDebug(Thread *tt, Board *bb) {
    if (debugmode() && isRootBoard(bb)) checkDebugOutline(tt);
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
  Thread *mkLTQ(Board *bb, int prio, SolveActor * sa);
  Thread *mkWakeupThread(Board *bb);
  Thread *mkPropagator(Board *bb, int prio, OZ_Propagator *pro);
  INLINE Thread *mkSuspendedThread(Board *bb, int prio);
  INLINE int newId();

  INLINE void suspThreadToRunnableOPT(Thread *tt);
  INLINE void suspThreadToRunnable(Thread *tt);
  INLINE void wakeupToRunnable(Thread *tt);
  INLINE void propagatorToRunnable(Thread *tt);
  INLINE void updateSolveBoardPropagatorToRunnable(Thread *tt);

  // wake up cconts and board conts
  INLINE Bool wakeUp(Thread *tt,Board *home, PropCaller calledBy);
  INLINE Bool wakeUpPropagator(Thread *tt, Board *home,
                        PropCaller calledBy = pc_propagator);
  INLINE Bool wakeUpBoard(Thread *tt, Board *home);
  INLINE Bool wakeUpThread(Thread *tt, Board *home);
  INLINE OZ_Return runPropagator(Thread *tt);

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
  INLINE void checkExtThread(Thread *tt);
  INLINE void removeExtThread(Thread *tt);

  void setExtThreadOutlined(Thread *tt, Board *varHome);

  //  special allocator for thread's bodies;
  INLINE void freeThreadBody(Thread *tt);

  //
  //  it asserts that the suspended thread is 'external' (see beneath);
  void checkExtThreadOutlined(Thread *tt);
  void removeExtThreadOutlined(Thread *tt);

  //
  //  (re-)Suspend a propagator again; (was: 'reviveCurrentTaskSusp');
  //  It does not take into account 'solve threads', i.e. it must
  // be done externally - if needed;
  INLINE void suspendPropagator(Thread *tt);
  INLINE void scheduledPropagator(Thread *tt);

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
  INLINE void closeDonePropagator(Thread *tt);

  INLINE void closeDonePropagatorCD(Thread *tt);
  INLINE void closeDonePropagatorThreadCD(Thread *tt);

  SuspList *installPropagators(SuspList *local_list, SuspList *glob_list,
                               Board *glob_home);

  TaggedRef createNamedVariable(int regIndex, TaggedRef name);
  void handleToplevelBlocking();

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
  Bool unify(TaggedRef ref1, TaggedRef ref2, ByteCode *scp=0);
  Bool fastUnify(TaggedRef ref1, TaggedRef ref2, ByteCode *);
  Bool fastUnifyOutline(TaggedRef ref1, TaggedRef ref2, ByteCode *);
  void bindToNonvar(TaggedRef *varPtr, TaggedRef var, TaggedRef term,
                    ByteCode *);

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
  Bool isMoreLocal(TaggedRef var1, TaggedRef var2);

  void genericBind(TaggedRef *varPtr, TaggedRef var,
                   TaggedRef *termPtr, TaggedRef term);

  void checkSuspensionList(TaggedRef taggedvar,
                           PropCaller calledBy = pc_propagator);
  Bool hasOFSSuspension(SuspList *suspList);
  void addFeatOFSSuspensionList(TaggedRef var, SuspList* suspList,
                                TaggedRef flist, Bool determined);
  SuspList * checkSuspensionList(SVariable * var,
                                 SuspList * suspList, PropCaller calledBy);
  BFlag isBetween(Board * to, Board * varHome);
  Bool  isBelow(Board *below, Board *above);
  int incSolveThreads(Board *bb);
  void decSolveThreads(Board *bb);
  DebugCode(Bool isInSolveDebug(Board *bb);)

  void restartThread();

  void handleIO();
  Bool loadQuery(CompStream *fd);
  void select(int fd, int mode, OZ_IOHandler fun, void *val);
  void acceptSelect(int fd, OZ_IOHandler fun, void *val);
  int select(int fd,int mode, TaggedRef l, TaggedRef r);
  void acceptSelect(int fd, TaggedRef l, TaggedRef r);
  void deSelect(int fd);
  void deSelect(int fd,int mode);
  void checkIO();

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
#include "compiler.hh"
#include "os.hh"


#ifndef OUTLINE
#include "am.icc"
#endif

#endif
