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

#include "codeArea.hh"
#include "gc.hh"
#include "genvar.hh"
#include "misc.hh"
#include "records.hh"
#include "statistics.hh"
#include "taskstack.hh"
#include "term.hh"
#include "trail.hh"

// -----------------------------------------------------------------------

typedef enum {
  READ,
  WRITE
} AMModus;

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

extern AM am;

// this class contains the central global data
class AM {
friend void engine();
public:
  int statusReg;
  Trail trail;
  RebindTrail rebindTrail;
  RefsArray xRegs;
  RefsArray globalStore;

  Thread *currentThread;
  TaskStack* currentTaskStack;    // opt: cache
  Thread *rootThread;

  Board *currentBoard;
  TaggedRef currentUVarPrototype; // opt: cache
  Board *rootBoard;

public:
  AM() {};
  void init(int argc,char **argv);

  void setSFlag(StatusBit flag) { statusReg = (flag | statusReg); }
  void unsetSFlag(StatusBit flag) { statusReg = (~flag & statusReg); }
  Bool isSetSFlag(StatusBit flag) { return ( statusReg & flag ) ? OK : NO; }
  Bool isSetSFlag() { return statusReg ? OK : NO; }

  void print();

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
  Bool smallGC();
// coping of trees (and terms);
  Board* copyTree (Board* node, Bool *isGround);

  void awakeNode(Board *node);

  // entailment check
  Bool entailment ();
  Bool isEmptyTrailChunk ();

  // Unification
  Bool unify(TaggedRef ref1, TaggedRef ref2);
  Bool fastUnify(TaggedRef ref1, TaggedRef ref2);
  Bool unify(TaggedRef *ref1, TaggedRef ref2);
  Bool unify(TaggedRef *ref1, TaggedRef *ref2);
  Bool performUnify(TaggedRef *ref1, TaggedRef *ref2);

  void rebind(TaggedRef *ref, TaggedRef ptr);
  Bool isLocalUVar(TaggedRef var);
  Bool isLocalSVar(TaggedRef var);
  Bool isLocalCVar(TaggedRef var);
  Bool isLocalVariable(TaggedRef var);
  Bool isInScope (Board *above, Board* node);
  void pushTask(Board *n,ProgramCounter pc,
                       RefsArray y,RefsArray g,RefsArray x=NULL,int i=0);
  void genericBind(TaggedRef *varPtr, TaggedRef var,
                   TaggedRef *termPtr, TaggedRef term);
  void bindToNonvar(TaggedRef *varPtr, TaggedRef var, TaggedRef term);
  void bind(TaggedRef *varPtr, TaggedRef var, TaggedRef *termPtr);
  void checkSuspensionList(TaggedRef taggedvar, TaggedRef term,
                                  SVariable *rightVar = NULL);
  SuspList* checkSuspensionList(SVariable* var, TaggedRef taggedvar,
                                SuspList* suspList,
                                TaggedRef term, SVariable* rightVar = NULL);
  Bool isBetween(Board *to, Board *varHome); // ###

// debugging --> see file ../builtins/debug.C
  State getValue(TaggedRef feature, TaggedRef out);
  State setValue(TaggedRef feature, TaggedRef value);
};


// this class contains the configurable parameters
class ConfigData {
public:
  int printDepth;

  int showForeignLoad;  // show message on load
  int showFastLoad;     // show message on fast load
  int showIdleMessage;  // show message on idle


  int gcFlag;                 // request GC to run
  int gcVerbosity;            // GC verbosity level

  char *ozPath;
  char *linkPath;

  int systemPriority;
  int defaultPriority;
  int timeSlice;

  int clockTick;

  int taskStackSize;
public:
  ConfigData();
};

extern ConfigData conf;

#ifndef OUTLINE
#include "am.icc"
#endif

#endif
