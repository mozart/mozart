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

#include <setjmp.h>
#include <unistd.h>
#include <sys/types.h>

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
  ThreadSwitch	= 1 << 2, // choose a new process
  IOReady	= 1 << 3, // IO handler has signaled IO ready
  UserAlarm	= 1 << 4, // Alarm handler has signaled User Alarm
  StartGC	= 1 << 5, // need a GC
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
extern jmp_buf engineEnvironment;

class AM {

public:

// --------------- Status Register ----------------------

// DATA
  StatusBit statusReg;
  void setSFlag(StatusBit flag) {
    statusReg = (StatusBit) (flag | statusReg);
  }
  void unsetSFlag(StatusBit flag) {
    statusReg = (StatusBit) (~flag & statusReg);
  }
  Bool isSetSFlag(StatusBit flag) { return ( statusReg & flag ) ? OK : NO; }
  Bool isSetSFlag() { return statusReg ? OK : NO; }

  
  AM() {};
  // initializing everything
  void init(int argc,char **argv);
  void initSignal();
  void initIO();
  void exitOz(int status);


// central control
  friend void engine();                   // the main loop

// tree move
  InstType installPath(Board *to);
  Bool installScript(ConsList &script);
  Bool install(Board *bb);
  void deinstallPath(Board *top);
  void deinstallCurrent();
  void reduceTrailOnUnitCommit();
  void reduceTrailOnSuspend();
  void reduceTrailOnFail();
  void reduceTrailOnShallow(Suspension *susp,int numbOfCons);

  Bool isToplevel();

// gc
  void gc(int msgLevel);
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

  /* load query form a file of byte codes */
  void acknowledgeCompiler(Bool result);
  Bool loadQuery(FILE *fd, Bool ack = OK);
  void waitCompiler();


  Statistics stat;
  int printDepthVal;

  // wheter we want to see messages when
  // loading foreign or precompiled files
  Bool foreignLoad;
  Bool fastLoad;
  Bool idleMessage;

  Trail trail;
  // gc ok
  RebindTrail rebindTrail;
  // gc ok
  RefsArray xRegs;  
  // global registers

  RefsArray globalStore;

  long timeSinceLastIdle;
  long timeForGC;
  long timeForCopy;
  double heapAllocated;
  long timeForLoading;
  
  int gcFlag;                 // request GC to run
  int gcVerbosity;            // request GC to run
  int clockTick;

  char *compilerFile;
  char *ozPath;
  char *linkPath;

  TaskStack* currentTaskStack;

  Suspension* currentTaskSusp;
  void reviveCurrentTaskSusp(void);
  void killCurrentTaskSusp(void);

  /* we cache the look of a UVar to speed up
   * creation of and tests on variables
   */
  TaggedRef currentUVarPrototype;

  void rebind (TaggedRef *ref, TaggedRef ptr);
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
  Bool isBetween(Board *to, Board *varHome);

// debugging --> see file ../builtins/debug.C
  State getValue(TaggedRef feature, TaggedRef out);
  State setValue(TaggedRef feature, TaggedRef value);

  void undoTrailing(int n);

// ----------------------------------------
// IO

  Board *ioNodes[FD_SETSIZE];  // node that must be waked up on io
  fd_set globalReadFDs;              // mask of active nodes in ioNodes[]
  static FILE *QueryFILE; // file descriptor for code connection from compiler
  static char *QueryFileName; // if we are running stand alone,
    // this is the name of the file we get our input from
  fd_set watchedFDs;

  void startIOHandler(int fd);
  void clearIOSockets();
  void suspendEngine();
  void handleIO();
  void openIO(int fd);
  void closeIO(int fd);
  Bool setIORequest (int fd);

  Bool isStandalone() { return QueryFileName == NULL ? NO : OK; }
};


#ifndef OUTLINE
#include "am.icc"
#endif

#endif
