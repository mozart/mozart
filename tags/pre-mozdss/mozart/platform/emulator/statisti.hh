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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

  void init();
  void reset();

  int getAtomMemory();
  int getNameMemory();

  void initGcMsg(int level);
  void printGcMsg(int level);

  void incSolveAlt(void)     { solveAlt.incf();}
  void incSolveCloned(void)  { solveCloned.incf();}
  void incSolveCreated(void) { solveCreated.incf();}
  void incSolveSolved(void)  { solveSolved.incf(); }
  void incSolveFailed(void)  { solveFailed.incf(); }

  PrTabEntry *currAbstr;
  
  void initProfile(void);

  void leaveCall(PrTabEntry *newproc);
  void heapAlloced(int sz);

  OZ_PropagatorProfile *currPropagator;
  void enterProp(OZ_PropagatorProfile *p) { currPropagator = p; p->incCalls(); }
  void leaveProp()                 { currPropagator = 0; }

#ifdef PROFILE_INSTR
  unsigned long instr[PROFILE_INSTR_MAX];
  unsigned long instrCollapsable[PROFILE_INSTR_MAX][PROFILE_INSTR_MAX];
  void printInstr();
  void printInstrCollapsable();
  void printInstrReset();
#endif

};

extern Statistics ozstat;

void printTime(FILE *fd,char *s,unsigned int t);
void printMem(FILE *fd,char *s,double m);

#endif
