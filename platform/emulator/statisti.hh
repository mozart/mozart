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

#include "tagged.hh"

#ifdef __GNUC__
#pragma interface
#endif

#ifdef AM_PROFILE
#   define IncfProfCounter(C,N) am.stat.C += N
#else
#   define IncfProfCounter(C,N)
#endif


class StatCounter {
public:
  unsigned long sinceIdle;
  unsigned long total;
  void reset()  { sinceIdle = total = 0; }
  StatCounter() { reset(); }
  incf(int n=1) { total+=n; }
  void idle()   { sinceIdle = total; }
  unsigned int sinceidle()   { return total-sinceIdle; }
};


class Statistics {
private:
  unsigned int gcStarttime;
  unsigned int gcStartmem;

public:
  StatCounter gcCollected;
  StatCounter timeForCopy;
  StatCounter timeForLoading;
  StatCounter timeForGC;
  StatCounter timeUtime;

  StatCounter heapUsed; /* total == memory used not including getUsedMemory() */
                        /* sinceIdle == memory reported during last idle */

  // for the solve combinator
  StatCounter solveDistributed;
  StatCounter solveSolved;
  StatCounter solveFailed;

  Statistics();
  void print(FILE *fd);
  void printIdle(FILE *fd);

  void reset();

  void initGcMsg(int level);
  void printGcMsg(int level);

  void getStatistics(TaggedRef rec, TaggedRef enu);

#ifdef PROFILE
  /* these should alos use class StatCounter */
  int allocateCounter, deallocateCounter, procCounter,
    waitCounter, askCounter,
    localVariableCounter, protectedCounter;
#endif

  void incSolveDistributed(void) { solveDistributed.incf();}
  void incSolveSolved(void)      { solveSolved.incf(); }
  void incSolveFailed(void)      { solveFailed.incf(); }
};

#endif
