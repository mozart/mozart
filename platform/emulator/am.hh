/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __AMH
#define __AMH

#ifdef __GNUC__
#pragma interface
#endif

#include "types.hh"

#include "alarm.hh"
#include "codearea.hh"
#include "gc.hh"
// #include "genvar.hh"
#include "misc.hh"
#include "records.hh"
#include "statisti.hh"
#include "taskstk.hh"
#include "term.hh"
#include "trail.hh"
#include "unify.hh"

// -----------------------------------------------------------------------

typedef enum {
  ThreadSwitch  = 1 << 2, // choose a new process
  IOReady       = 1 << 3, // IO handler has signaled IO ready
  UserAlarm     = 1 << 4, // Alarm handler has signaled User Alarm
  StartGC       = 1 << 5, // need a GC
  DebugMode     = 1 << 6
} StatusBit;

enum JumpReturns {
  NOEXCEPTION = 0,
  SEGVIO = 1,
  BUSERROR = 2
};

enum InstType {
  INST_OK,
  INST_FAILED,
  INST_REJECTED
};

// this class contains the configurable parameters
class ConfigData {
public:
  int printDepth;

  int showForeignLoad;  // show message on load
  int showFastLoad;     // show message on fast load
  int showIdleMessage;  // show message on idle
  int showSuspension;   // show message when a suspension is created

  int stopOnToplevelFailure;  // enter the debugger on TOPLEVEL FAILURE

  int gcFlag;                 // request GC to run
  int gcVerbosity;            // GC verbosity level
  unsigned int heapMaxSize;
  unsigned int heapMargin;
  unsigned int heapIncrement;
  unsigned int heapIdleMargin;

  char *ozPath;
  char *linkPath;

  int systemPriority;
  int defaultPriority;
  int timeSlice;

  int taskStackSize;

  int errorVerbosity;

  int dumpCore;

  int cellHack;

  /* command line arguments visible from Oz */
  char **argV;
  int argC;

public:
  ConfigData() {};
  void init();
};

// this class contains the central global data
class AM {
friend void engine();
public:
  static int ProcessCounter;

  int statusReg;
  Trail trail;
  RebindTrail rebindTrail;
  RefsArray xRegs;
  RefsArray toplevelVars;

  Board *currentBoard;
  TaggedRef currentUVarPrototype; // opt: cache
  Board *rootBoard;

  Board *currentSolveBoard;       // current 'solve' board or NULL if none;
  Bool wasSolveSet;

  Statistics stat;
  ConfigData conf;

#ifdef DEBUG_CHECK
  Bool dontPropagate;
  // is used by consistency checking of a copy of a search tree;
#endif

  /* Threads */
  Thread *currentThread;
  Thread *threadsHead;
  Thread *threadsTail;

  Thread *rootThread;
  Toplevel *toplevelQueue;

  Thread *threadsFreeList;

  void initThreads();
  void printThreads();

  void AM::scheduleSuspCont(SuspContinuation *c, Bool wasExtSusp);
  void AM::scheduleSuspCCont(CFuncContinuation *c, Bool wasExtSusp,
                             Suspension *s=0);
  void AM::scheduleSolve(Board *b);
  void AM::scheduleWakeup(Board *b, Bool wasExtSusp);

  void AM::pushToplevel(ProgramCounter pc);
  void AM::checkToplevel();
  void AM::addToplevel(ProgramCounter pc);

  Thread *AM::newThread(int p,Board *h);
  void AM::disposeThread(Thread *th);
  Bool AM::isScheduled(Thread *th);
  void AM::scheduleThread(Thread *th);
  Bool AM::threadQueueIsEmpty();
  Thread *AM::getFirstThread();
  Thread *AM::unlinkThread(Thread *th);
  void AM::insertFromHead(Thread *th);
  void AM::insertAfter(Thread *th,Thread *here);
  void AM::insertFromTail(Thread *th);
  void AM::insertBefore(Thread *th, Thread *here);

public:
  AM() {};
  void init(int argc,char **argv);

  Bool criticalFlag;  // if this is true we will NOT set Sflags
                      // from within signal handlers

  Bool isCritical() { return criticalFlag; }

  void setSFlag(StatusBit flag)
  {
    criticalFlag = OK;
    statusReg = (flag | statusReg);
    criticalFlag = NO;
  }

  void unsetSFlag(StatusBit flag)
  {
    criticalFlag = OK;
    statusReg = (~flag & statusReg);
    criticalFlag = NO;
  }

  Bool isSetSFlag(StatusBit flag) { return ( statusReg & flag ) ? OK : NO; }
  Bool isSetSFlag() { return statusReg ? OK : NO; }

  void print();

  void setCurrent(Board *c, Bool checkNotGC=OK);
  InstType installPath(Board *to); // ###
  Bool installScript(Script &script);
  Bool install(Board *bb);
  void deinstallPath(Board *top);
  void deinstallCurrent();
  void reduceTrailOnUnitCommit();
  void reduceTrailOnSuspend();
  void reduceTrailOnFail();
  void reduceTrailOnShallow(Suspension *susp,int numbOfCons);

  // in emulate.cc
  Bool emulateHookOutline(Abstraction *def=NULL,
                          int arity=0, TaggedRef *arguments=NULL);
  Bool hookCheckNeeded();
  Suspension *mkSuspension(Board *b, int prio, ProgramCounter PC,
                           RefsArray Y, RefsArray G,
                           RefsArray X, int argsToSave);
  Suspension *mkSuspension(Board *b, int prio, OZ_CFun bi,
                           RefsArray X, int argsToSave);
  void suspendOnVar(TaggedRef A, int argsToSave, Board *b, ProgramCounter PC,
                    RefsArray X, RefsArray Y, RefsArray G, int prio);
  void suspendShallowTest2(TaggedRef A, TaggedRef B, int argsToSave,
                           Board *b,
                           ProgramCounter PC, RefsArray X, RefsArray Y,
                           RefsArray G, int prio);
  TaggedRef createNamedVariable(int regIndex, TaggedRef name);
  void suspendInlineRel(TaggedRef A, TaggedRef B, int noArgs,
                        OZ_CFun fun, ByteCode *shallowCP);
  void suspendInlineFun(TaggedRef A, TaggedRef B, TaggedRef C,
                        TaggedRef &Out,
                        int noArgs, OZ_CFun fun, InlineFun2 inFun,
                        ByteCode *shallowCP);

  Bool isToplevel();

  void gc(int msgLevel);  // ###
  void doGC();
  Bool idleGC();
// coping of trees (and terms);
  Board* copyTree (Board* node, Bool *isGround);

  void awakeNode(Board *node);

  // entailment check
  Bool entailment ();
  Bool isEmptyTrailChunk ();

  // Unification
  Bool unify(TaggedRef ref1, TaggedRef ref2, Bool prop = OK);
  Bool fastUnify(TaggedRef ref1, TaggedRef ref2, Bool prop);
  Bool fastUnifyOutline(TaggedRef ref1, TaggedRef ref2, Bool prop);
  Bool performUnify(TaggedRef *termPtr1, TaggedRef *termPtr2, Bool prop);
  void bindToNonvar(TaggedRef *varPtr, TaggedRef var, TaggedRef term, Bool prop);

  void rebind(TaggedRef *ref, TaggedRef ptr);
  Bool isLocalUVar(TaggedRef var);
  Bool isLocalSVar(TaggedRef var);
  Bool isLocalCVar(TaggedRef var);
  Bool isLocalVariable(TaggedRef var);
  Bool isInScope (Board *above, Board* node);

  void pushCall(Board *b, SRecord *def, int arity, RefsArray args);
  void pushDebug(Board *n, SRecord *def, int arity, RefsArray args);
  void pushTask(Board *n,ProgramCounter pc,
                RefsArray y,RefsArray g,RefsArray x=0,int i=0);
  void pushCFun(Board *n, OZ_CFun f, RefsArray x=0, int i=0);
  void pushNervous(Board *n);

  void genericBind(TaggedRef *varPtr, TaggedRef var,
                   TaggedRef *termPtr, TaggedRef term, Bool prop);
  void bind(TaggedRef *varPtr, TaggedRef var, TaggedRef *termPtr, Bool prop);
  void checkSuspensionList(TaggedRef taggedvar, TaggedRef term,
                           PropCaller calledBy = pc_propagator);
  Bool hasOFSSuspension(SuspList *suspList);
  void addFeatOFSSuspensionList(TaggedRef var, SuspList* suspList,
                                TaggedRef flist, Bool determined);
  SuspList * checkSuspensionList(SVariable * var, TaggedRef taggedvar,
                                 SuspList * suspList, TaggedRef term,
                                 PropCaller calledBy);
  Bool isBetween(Board * to, Board * varHome);
  void setExtSuspension (Board *varHome, Suspension *susp);
private:
  Bool _checkExtSuspension(Suspension * susp);
public:
  Bool checkExtSuspension(Suspension * susp) {
    if (susp->isExtSusp())
      return _checkExtSuspension(susp);
    return NO;
  }
  void incSolveThreads (Board *bb,int n=1);
  void decSolveThreads (Board *bb);
  Board *findStableSolve(Board *bb);

// debugging --> see file ../builtins/debug.C
  State getValue(TaggedRef feature, TaggedRef out);
  State setValue(TaggedRef feature, TaggedRef value);

  void restartThread();
};

extern AM am;

#ifdef OUTLINE
void updateExtSuspension(Board *varHome, Suspension *susp);
#else
#include "am.icc"
#endif

#endif
