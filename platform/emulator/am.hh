/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow, mehl

  ------------------------------------------------------------------------
*/

#ifndef __AMH
#define __AMH

#if defined(OUTLINE)
#define INLINE
#else
#define INLINE inline
#endif

#ifdef INTERFACE
#pragma interface
#endif

#include <setjmp.h>

#include "oz.h"
#include "oz_cpi.hh"

#include "types.hh"

#include "tagged.hh"
#include "value.hh"

#include "cont.hh"

#include "stack.hh"
#include "taskstk.hh"
#include "trail.hh"

#include "thrspool.hh"

// -----------------------------------------------------------------------

typedef enum {
  ThreadSwitch	= 1 << 2, // choose a new process
  IOReady	= 1 << 3, // IO handler has signaled IO ready
  UserAlarm	= 1 << 4, // Alarm handler has signaled User Alarm
  StartGC	= 1 << 5, // need a GC
  DebugMode	= 1 << 6,
  StopThread	= 1 << 7
} StatusBit;


// isBetween returns
enum BFlag {
  B_BETWEEN,
  B_NOT_BETWEEN,
  B_DEAD
};

enum JumpReturns {
  SEGVIO = 1,
  BUSERROR = 2
};

enum InstType {
  INST_OK,
  INST_FAILED,
  INST_REJECTED
};


class IONode {
public:
  OZ_IOHandler handler[2];
  void *readwritepair[2];
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
friend void engine(Bool init);
public:
  int threadSwitchCounter;
  int userCounter;

  int statusReg;
  Trail trail;
  RefsArray xRegs;  
  
  RefsArray toplevelVars;
  int toplevelVarsCount;

  Board *currentBoard;
  TaskStack *cachedStack;
  Object *cachedSelf;
  
  char *shallowHeapTop;
  
public:
  void changeSelf(Object *o);
  void saveSelf();
  void setSelf(Object *o) { cachedSelf = o; }
  Object *getSelf() { return cachedSelf; }

  TaggedRef _currentUVarPrototype; // opt: cache

  int currentUVarPrototypeEq(TaggedRef t) {
    return _currentUVarPrototype == t;
  }
  TaggedRef currentUVarPrototype() {
    Assert(currentBoard=tagged2VarHome(_currentUVarPrototype));
#ifdef OPT_VAR_IN_STRUCTURE
    return _currentUVarPrototype;
#else
    return makeTaggedRef(newTaggedUVar(_currentUVarPrototype));
#endif
  }

  Board *rootBoard;

  Board *currentSolveBoard;       // current 'solve' board or NULL if none;
  Bool wasSolveSet; 

  CompStream *compStream;
  Bool isStandaloneF;
  Bool isStandalone() { return isStandaloneF; }

  jmp_buf engineEnvironment;

  IONode *ioNodes;		// node that must be waked up on io

#ifdef DEBUG_CHECK
  Bool dontPropagate;
  // is used by consistency checking of a copy of a search tree; 
#endif

  TaggedRef suspendVarList;
  void emptySuspendVarList(void) { suspendVarList = makeTaggedNULL(); }
  void addSuspendVarList(TaggedRef * t);
  void suspendOnVarList(Thread *thr);

  void stopThread(Thread *th);
  void resumeThread(Thread *th);

  void formatError(OZ_Term traceBack,OZ_Term loc);
  void formatFailure(OZ_Term traceBack,OZ_Term loc);
  int raise(OZ_Term cat, OZ_Term key, char *label, int arity, ...);
  struct {
    int debug;
    TaggedRef value;
    TaggedRef info;
  } exception;
  void enrichTypeException(char *fun, OZ_Term args);

  void suspendInline(int n,
		     OZ_Term A,OZ_Term B=makeTaggedNULL(),
		     OZ_Term C=makeTaggedNULL());

  TaggedRef aVarUnifyHandler;
  TaggedRef aVarBindHandler;

  TaggedRef methApplHdl;
  TaggedRef sendHdl;
  TaggedRef newHdl;

  TaggedRef defaultExceptionHandler;

  TaggedRef opiCompiler;

  unsigned int lastThreadID;
  unsigned int lastFrameID;

  // Debugging stuff
  Bool suspendDebug, runChildren;
  TaggedRef threadStreamTail;
  Toplevel *toplevelQueue;

  void printBoards();

  void pushToplevel(ProgramCounter pc);
  void checkToplevel();
  void addToplevel(ProgramCounter pc);

  int catchError() { return setjmp(engineEnvironment); }
public:
  AM() {};
  void init(int argc,char **argv);
  void checkVersion();
  void exitOz(int status);
  void suspendEngine();

  Bool criticalFlag;  // if this is true we will NOT set Sflags
                      // from within signal handlers

  Bool isCritical() { return criticalFlag; }

// #define DEBUG_STATUS
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
    case DebugMode:    return 'D';
    default:           return 'X';
    }
  }
#endif
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

  Bool debugmode() { return isSetSFlag(DebugMode); }
  int newId() {
    return ++lastThreadID % MAX_ID;
  }
  void checkDebug(Thread *tt) {
    if (debugmode()) checkDebugOutline(tt);
  }
  void checkDebugOutline(Thread *tt);

  void checkStatus(); // in emulate.cc

  void print();

private:
  Bool installingScript;

public:
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
  void reduceTrailOnShallow(Thread *);

  // in emulate.cc
  Bool emulateHookOutline(ProgramCounter PC, Abstraction *def,TaggedRef *arguments);
  Bool hookCheckNeeded();
  Bool isNotPreemtiveScheduling(void);

  INLINE RunnableThreadBody* allocateBody();
  INLINE Thread *newThreadInternal(int prio, Board *bb);
  INLINE Thread *mkRunnableThread(int prio, Board *bb);
  INLINE Thread *mkRunnableThreadOPT(int prio, Board *bb);
  Thread *mkLTQ(Board *bb, int prio, SolveActor * sa);
  Thread *mkWakeupThread(Board *bb);
  Thread *mkPropagator(Board *bb, int prio, OZ_Propagator *pro);
  INLINE Thread *mkSuspendedThread(Board *bb, int prio);

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

  Bool isToplevel() { return currentBoard == rootBoard; }

  void gc(int msgLevel);  // ###
  void doGC();
  // coping of trees (and terms);
  Board* copyTree(Board* node, Bool *isGround);

  static int awakeIO(int fd, void *var);
  void awakeIOVar(TaggedRef var);

  // entailment check
  Bool entailment();

  void checkStability();
  int handleFailure(Continuation *&cont, AWActor *&aa);
  int commit(Board *bb, Thread *tt=0);

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

  void setUserAlarmTimer(int ticks) { userCounter=ticks; }
  int getUserAlarmTimer() { return userCounter; } 

  OzSleep *sleepQueue;
  void insertUser(int t,TaggedRef node);
  int wakeUser();

  Bool isStableSolve(SolveActor *sa);

  OZ_Term dbgGetLoc(Board*);

  Bool profileMode;
};

extern AM am;

#include "cpi_heap.hh"
#include "cpbag.hh"

#include "actor.hh"
#include "board.hh"

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

#include "perdio.hh"

#ifndef OUTLINE
#include "am.icc"
#endif

#endif
