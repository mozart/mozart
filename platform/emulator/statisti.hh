/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#ifdef INTERFACE
#pragma interface
#endif

#ifdef HEAP_PROFILE
# define ProfileCode(Code) Code
#else
# define ProfileCode(Code)
#endif


#define COUNT1(WHAT,n) ProfileCode(ozstat.WHAT += n)
#define COUNT(WHAT)    COUNT1(WHAT,1) 
#define CountMax(What,Value) ProfileCode(ozstat.What = max(ozstat.What,Value))

class StatCounter {
public:
  unsigned long sinceIdle;
  unsigned long total;
  void reset()  { sinceIdle = total = 0; }
  StatCounter() { reset(); }
  void incf(int n=1) { total+=n; }
  void idle()   { sinceIdle = total; }
  unsigned int sinceidle()   { return total-sinceIdle; }
};


class Statistics {
private:
  unsigned int gcStarttime;
  unsigned int gcStartmem;

public:
  unsigned int gcLastActive;
  StatCounter gcCollected;
  StatCounter timeForCopy;
  StatCounter timeForLoading;
  StatCounter timeForGC;
  StatCounter timeUtime;
  StatCounter timeForPropagation;

  StatCounter heapUsed; /* total == memory used not including getUsedMemory() */
                        /* sinceIdle == memory reported during last idle */

  // for the solve combinator
  StatCounter solveAlt;
  StatCounter solveCloned;
  StatCounter solveCreated;
  StatCounter solveSolved;
  StatCounter solveFailed;

  StatCounter propagatorsCreated;
  StatCounter propagatorsInvoked;
  StatCounter fdvarsCreated;

  StatCounter createdThreads;
  StatCounter runableThreads;

  Statistics();

  void printIdle(FILE *fd);
  void printRunning(FILE *fd);

  void reset();

  int getAtomMemory();
  int getNameMemory();
  int getCodeMemory();

  void initGcMsg(int level);
  void printGcMsg(int level);

  void incSolveAlt(void)     { solveAlt.incf();}
  void incSolveCloned(void)  { solveCloned.incf();}
  void incSolveCreated(void) { solveCreated.incf();}
  void incSolveSolved(void)  { solveSolved.incf(); }
  void incSolveFailed(void)  { solveFailed.incf(); }

#ifdef HEAP_PROFILE
  void initCount();
  void printCount();
  long literal;
  long ozfloat;
  long bigInt;
  long scriptLen; // length of all scripts
  long refsArray;
  long refsArrayLen; // length of all refsArrays
  long continuation;
  long suspCFun;
  long suspCont;
  long sTuple;
  long sTupleLen;
  long lTuple;
  long sRecord;
  long sRecordLen;
  long suspList;
  long uvar;
  long svar;
  long cvar;
  long dynamicTable, dynamicTableLen;
  long taskStack,taskStackLen;
  long cSolve,cACont,cCatch,cLocal,cCont,cXCont,cSetCaa,cDebugCont,cExceptHandler;
  long cCallCont;
  long abstraction,flatObject,cell,space,chunk;
  long heapChunk,thread;
  long board,objectClass;
  long askActor,waitActor,solveActor,waitChild;


  // RS
  long freeListAllocated, freeListDisposed;
  long totalAllocated;
  long varVarUnify, recRecUnify,totalUnify;
  long maxStackDepth;
  long sizeClosures, numClosures, sizeGs;
  long sizeStackVars;
  long sizeEnvs, numEnvAllocs, maxEnvSize;

  long fastcalls,bicalls,nonoptcalls,inlinecalls,inlinedots,
    sendmsg,applmeth,nonoptbicalls,nonoptsendmsg;
  
  long numNewName, numNewNamedName;
  long numThreads;

  void derefChain(int n);
  void printDeref();
  long lenDeref;
  long numDerefs;
  long longestDeref;
  const int maxDerefLength = 10;
  long lengthDerefs[maxDerefLength+1];
#endif
};

extern Statistics ozstat;

#endif
