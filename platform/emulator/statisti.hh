/*
 *  Authors:
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

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "mozart_cpi.hh"

#ifdef HEAP_PROFILE
# define ProfileCode(Code) Code
#else
# define ProfileCode(Code)
#endif

#define COUNTIT(WHAT,n) ozstat.WHAT += n
#define COUNT1(WHAT,n) ProfileCode(if (!isCollecting) {COUNTIT(WHAT,n);})
#define COUNT(WHAT)    COUNT1(WHAT,1)
#define CountMax(What,Value) ProfileCode(ozstat.What = max(ozstat.What,Value))

// also count during GC
#define CountGC(What,n) COUNTIT(WHAT,n)

#ifdef PROFILE_INSTR
#define PROFILE_INSTR_MAX 256
#endif

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
  StatCounter timeForGC;
  StatCounter timeUtime;
  StatCounter timeForPropagation;
  unsigned long timeIdle;

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

  PrTabEntry *currAbstr;
  void leaveCall(PrTabEntry *newproc);
  void heapAlloced(int sz);

  OZ_PropagatorProfile *currPropagator;
  void enterProp(OZ_PropagatorProfile *p) { currPropagator = p; p->incCalls(); }
  void leaveProp()                 { currPropagator = 0; }

  void initCount();
  void printCount(char *file);

#ifdef PROFILE_INSTR
  unsigned long instr[PROFILE_INSTR_MAX];
  unsigned long instrCollapsable[PROFILE_INSTR_MAX][PROFILE_INSTR_MAX];
  void printInstr();
  void printInstrCollapsable();
  void printInstrReset();
#endif

#ifdef HEAP_PROFILE
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
  long varVarUnify, nonvarNonvarUnify, varOptUnify, recRecUnify, varNonvarUnify;
  long maxStackDepth;
  long sizeClosures, numClosures, sizeGs;
  long sizeObjects,sizeRecords,sizeLists,sizeFloats,sizeFreeListObjs;
  long sizeStackVars;
  long sizeHeapChunks, sizeBitArrays;
  long sizeEnvs, numEnvAllocs, maxEnvSize;

  long fastcalls,optbicalls,nonoptcalls,inlinecalls,inlinedots,
    sendmsg,applmeth,nonoptbicalls,nonoptsendmsg;

  long numNewName, numNewNamedName;
  long numThreads;

  // those are also counted during GC
  long lenDeref, numDerefs, longestDeref;
#define maxDerefLength 10
  long lengthDerefs[maxDerefLength+1];

  void derefChain(int n);
  void printDeref(FILE *out);
#endif
};

extern Statistics ozstat;

void printTime(FILE *fd,char *s,unsigned int t);
void printMem(FILE *fd,char *s,double m);

#endif
