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

#ifdef INTERFACE
#pragma interface
#endif

#include <setjmp.h>

#include "types.hh"

#include "tagged.hh"
#include "term.hh"
#include "constter.hh"

#include "stack.hh"
#include "taskstk.hh"
#include "trail.hh"

#include "thrspool.hh"

// -----------------------------------------------------------------------

typedef enum {
  ThreadSwitch  = 1 << 2, // choose a new process
  IOReady       = 1 << 3, // IO handler has signaled IO ready
  UserAlarm     = 1 << 4, // Alarm handler has signaled User Alarm
  StartGC       = 1 << 5, // need a GC
  DebugMode     = 1 << 6
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
  TaggedRef readwritepair[2];
};


// this class contains the central global data
class AM : public ThreadsPool {
friend void engine();
public:
  int threadSwitchCounter;
  int userCounter;

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

  CompStream *compStream;
  Bool isStandaloneF;
  Bool isStandalone() { return isStandaloneF; }

  jmp_buf engineEnvironment;

  IONode *ioNodes;              // node that must be waked up on io

#ifdef DEBUG_CHECK
  Bool dontPropagate;
  // is used by consistency checking of a copy of a search tree;
#endif

  TaggedRef suspendVarList;
  void emptySuspendVarList(void) { suspendVarList = makeTaggedNULL(); }
  void addSuspendVarList(TaggedRef * t);
  void suspendOnVarList(Thread *thr);

  TaggedRef exception;

  void suspendInline(int n,
                     OZ_Term A,OZ_Term B=makeTaggedNULL(),OZ_Term C=makeTaggedNULL());


  TaggedRef aVarUnifyHandler;
  TaggedRef aVarBindHandler;
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
  void reduceTrailOnShallow();

  // in emulate.cc
  Bool emulateHookOutline(Abstraction *def=NULL,
                          int arity=0, TaggedRef *arguments=NULL);
  Bool hookCheckNeeded();
  Thread *mkSuspThread ();
  void suspendCond(AskActor *aa);

  TaggedRef createNamedVariable(int regIndex, TaggedRef name);
  void handleToplevelBlocking();

  Bool isToplevel();
  Bool isToplevelFailure();

  void gc(int msgLevel);  // ###
  void doGC();
  Bool idleGC();
  // coping of trees (and terms);
  Board* copyTree (Board* node, Bool *isGround);

  void awakeIOVar(TaggedRef var);

  // entailment check
  Bool entailment();
  Bool isEmptyTrailChunk();
  int checkEntailment(Continuation *&contAfter,Actor *&aa);
  int handleFailure(Continuation *&cont, AWActor *&aa);

  void defaultExceptionHandler(OZ_Term val, ProgramCounter PC,
                               TaskStackEntry *oldTos);

  // Unification
  Bool unify(TaggedRef ref1, TaggedRef ref2, Bool prop = OK);
  Bool fastUnify(TaggedRef ref1, TaggedRef ref2, Bool prop);
  Bool fastUnifyOutline(TaggedRef ref1, TaggedRef ref2, Bool prop);
  Bool performUnify(TaggedRef *termPtr1, TaggedRef *termPtr2, Bool prop);
  void bindToNonvar(TaggedRef *varPtr, TaggedRef var, TaggedRef term, Bool prop);

  void rebind(TaggedRef *ref, TaggedRef ptr);
  void doBindAndTrail(TaggedRef v, TaggedRef * vp, TaggedRef t);
  void doBindAndTrailAndIP(TaggedRef v, TaggedRef * vp, TaggedRef t,
                               GenCVariable * lv, GenCVariable * gv,
                               Bool prop);

  Bool isLocalUVar(TaggedRef var);
  Bool isLocalSVar(TaggedRef var);
  Bool isLocalSVar(SVariable *var);
  Bool isLocalCVar(TaggedRef var);
  Bool isLocalVariable(TaggedRef var);

  void pushCall(TaggedRef def, int arity, RefsArray args);
  void pushDebug(TaggedRef def, int arity, RefsArray args);
  void pushTask(ProgramCounter pc,
                RefsArray y,RefsArray g,RefsArray x=0,int i=0);
  void pushTaskOutline(ProgramCounter pc,
                       RefsArray y,RefsArray g,RefsArray x=0,int i=0);
  void pushCFun(OZ_CFun f, RefsArray x=0, int i=0);
  void pushLocal();

 private:
  void genericBind(TaggedRef *varPtr, TaggedRef var,
                   TaggedRef *termPtr, TaggedRef term, Bool prop);
 public:

  void checkSuspensionList(TaggedRef taggedvar,
                           PropCaller calledBy = pc_propagator);
  Bool hasOFSSuspension(SuspList *suspList);
  void addFeatOFSSuspensionList(TaggedRef var, SuspList* suspList,
                                TaggedRef flist, Bool determined);
  SuspList * checkSuspensionList(SVariable * var, TaggedRef taggedvar,
                                 SuspList * suspList, PropCaller calledBy);
  BFlag isBetween(Board * to, Board * varHome);
  void incSolveThreads (Board *bb,int n=1);
  void decSolveThreads (Board *bb);
  DebugCode (Bool isInSolveDebug (Board *bb);)

  // debugging --> see file ../builtins/debug.C
  State getValue(TaggedRef feature, TaggedRef out);
  State setValue(TaggedRef feature, TaggedRef value);

  void restartThread();

  void handleIO();
  Bool loadQuery(CompStream *fd);
  OZ_Bool select(int fd,int mode,TaggedRef l,TaggedRef r);
  void deSelect(int fd);
  void checkIO();

  void handleAlarm();
  void handleUser();
  int setUserAlarmTimer(int ms);

  OzSleep *sleepQueue;
  void insertUser(int t,TaggedRef node);
  int wakeUser();

  Bool isStableSolve(SolveActor *sa);
};

extern AM am;

#include "dllstack.hh"

#include "actor.hh"
#include "board.hh"

#include "hashtbl.hh"

#include "statisti.hh"

#include "lps.hh"

#include "thread.hh"
#include "susplist.hh"
#include "variable.hh"

#include "solve.hh"

#include "opcodes.hh"
#include "codearea.hh"

#include "bignum.hh"

#include "records.hh"

#include "builtins.hh"
#include "compiler.hh"
#include "debug.hh"
#include "os.hh"
#include "verbose.hh"
#include "cell.hh"
#include "objects.hh"

#ifndef OUTLINE
#include "am.icc"
#endif

#endif
