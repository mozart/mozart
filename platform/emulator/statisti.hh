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

#ifdef __GNUC__
#pragma interface
#endif

#ifdef AM_PROFILE
#   define IncfProfCounter(C,N) am.stat.C += N
#else
#   define IncfProfCounter(C,N)
#endif

class Statistics {
private:
  static int Statistics_gcSoFar;
  int gc_level;
  unsigned gc_utime;
  unsigned gc_usedMem;
  unsigned gc_allocMem;
public:
  Statistics();
  void print(FILE *fd);
  void printIdle(FILE *fd);

  void reset();


  void initGcMsg(int level);
  void printGcMsg(void);

  int getGCTime() { return sumTimeForGC+timeForGC; }
  int getCopyTime() { return sumTimeForCopy+timeForCopy; }
  int getLoadingTime() { return sumTimeForLoading+timeForLoading; }

#ifdef PROFILE
  int allocateCounter, deallocateCounter, procCounter,
    waitCounter, askCounter,
    localVariableCounter, protectedCounter;
#endif

  int wakeUpCont;
  int wakeUpContOpt;
  int wakeUpNode;
  int wakeUpBI;

private:
  // for the solve combinator
  int solveDistributed;
  int solveSolved;
  int solveFailed;

public:

  void incSolveDistributed(void) {solveDistributed++;}
  void incSolveSolved(void) {solveSolved++;}
  void incSolveFailed(void) {solveFailed++;}

  unsigned int timeForGC;
  unsigned int timeForCopy;
  double heapAllocated;
  unsigned int timeForLoading;
  unsigned int timeSinceLastIdle;

  unsigned int sumTimeForGC;
  unsigned int sumTimeForCopy;
  unsigned int sumTimeForLoading;
};

#endif
