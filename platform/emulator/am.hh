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
#include "genvar.hh"
#include "misc.hh"
#include "records.hh"
#include "statisti.hh"
#include "taskstk.hh"
#include "term.hh"
#include "trail.hh"
#include "unify.hh"

// -----------------------------------------------------------------------

typedef enum {
  READ,
  WRITE
} AMModus;

typedef enum {
  ThreadSwitch	= 1 << 2, // choose a new process
  IOReady	= 1 << 3, // IO handler has signaled IO ready
  UserAlarm	= 1 << 4, // Alarm handler has signaled User Alarm
  StartGC	= 1 << 5, // need a GC
  DebugMode	= 1 << 6
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

  int showForeignLoad;	// show message on load
  int showFastLoad;	// show message on fast load
  int showIdleMessage;	// show message on idle

  int stopOnToplevelFailure;  // enter the debugger on TOPLEVEL FAILURE

  int gcFlag;                 // request GC to run
  int gcVerbosity;            // GC verbosity level
  int heapMaxSize;
  int heapMargin;
  int heapIncrement;
  int heapIdleMargin;
  
  char *ozPath;
  char *linkPath;

  int systemPriority;
  int defaultPriority;
  int timeSlice;

  int taskStackSize;

  int errorVerbosity;

  /* command line arguments visible from Oz */
  char **argV;
  int argC;
  
public:
  ConfigData();
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

  Thread *currentThread;
  TaskStack* currentTaskStack;    // opt: cache
  Thread *rootThread;

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
  Bool installScript(ConsList &script);
  Bool install(Board *bb);
  void deinstallPath(Board *top);
  void deinstallCurrent();
  void reduceTrailOnUnitCommit();
  void reduceTrailOnSuspend();
  void reduceTrailOnFail();
  void reduceTrailOnShallow(Suspension *susp,int numbOfCons);

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
  Bool unify(TaggedRef ref1, TaggedRef ref2);
  Bool fastUnify(TaggedRef ref1, TaggedRef ref2);
#ifdef FASTSS
  Bool fastUnifyOutline(TaggedRef term1, TaggedRef *term1Ptr, TaggedRef term2);
#endif
  Bool unify(TaggedRef *ref1, TaggedRef ref2);
  Bool unify(TaggedRef *ref1, TaggedRef *ref2);
  Bool performUnify(TaggedRef *ref1, TaggedRef *ref2);
  void bindToNonvar(TaggedRef *varPtr, TaggedRef var, TaggedRef term);

  void rebind(TaggedRef *ref, TaggedRef ptr);
  Bool isLocalUVar(TaggedRef var);
  Bool isLocalSVar(TaggedRef var);
  Bool isLocalCVar(TaggedRef var);
  Bool isLocalVariable(TaggedRef var);
  Bool isInScope (Board *above, Board* node);

  TaskStack *ensureTaskStack();
  
  void pushCall(Board *b, SRecord *def, int arity, RefsArray args);
  void pushDebug(Board *n, SRecord *def, int arity, RefsArray args);
  void pushTask(Board *n,ProgramCounter pc,
		RefsArray y,RefsArray g,RefsArray x=NULL,int i=0);
  void pushTaskOutline(Board *n,ProgramCounter pc,
		       RefsArray y,RefsArray g,RefsArray x=NULL,int i=0);
  void pushCFun(Board *n, OZ_CFun f, RefsArray x=NULL, int i=0);
  void pushNervous (Board *n); 
  void genericBind(TaggedRef *varPtr, TaggedRef var,
		   TaggedRef *termPtr, TaggedRef term);
  void bind(TaggedRef *varPtr, TaggedRef var, TaggedRef *termPtr);
  void checkSuspensionList(TaggedRef taggedvar, TaggedRef term,
			   SVariable * rightVar,
			   PropCaller calledBy = pc_propagator);
  SuspList * checkSuspensionList(SVariable * var, TaggedRef taggedvar,
				 SuspList * suspList, TaggedRef term,
				 SVariable * rightVar, PropCaller calledBy);
  Bool isBetween(Board * to, Board * varHome);
  void setExtSuspension (Board *varHome, Suspension *susp);
  Bool checkExtSuspension (Suspension *susp);
  void incSolveThreads (Board *bb);
  void decSolveThreads (Board *bb);

// debugging --> see file ../builtins/debug.C
  State getValue(TaggedRef feature, TaggedRef out);
  State setValue(TaggedRef feature, TaggedRef value);

  void RestartProcess();
};

extern AM am;

#ifdef OUTLINE
void updateExtSuspension(Board *varHome, Suspension *susp);
#else
#include "am.icc"
#endif

#endif
