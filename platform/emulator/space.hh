/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
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

#ifndef __SPACEHH
#define __SPACEHH

#include "base.hh"
#include "am.hh"
#include "board.hh"
#include "solve.hh"

Bool oz_installScript(Script &script);

InstType oz_installPath(Board *to);
Bool oz_install(Board *bb);
//inline void oz_deinstallPath(Board *top);
//inline void oz_deinstallCurrent();
void oz_reduceTrailOnUnitCommit();
void oz_reduceTrailOnSuspend();
void oz_reduceTrailOnFail();
void oz_reduceTrailOnShallow();
void oz_reduceTrailOnEqEq();

void oz_checkStability();
int oz_handleFailure(Continuation *&cont, AWActor *&aa);
int oz_commit(Board *bb, Thread *tt=0);
void oz_failBoard();
void oz_merge(Board *bb, Board *to,int inc);

//  Check all the solve actors above for stabily
// (and, of course, wake them up if needed);
inline void oz_removeExtThread(Thread *tt);
Bool oz_solve_checkExtSuspList (SolveActor *sa);

void oz_setExtSuspensionOutlined(Suspension susp, Board *varHome);

Bool oz_isStableSolve(SolveActor *sa);

int oz_incSolveThreads(Board *bb);
#ifdef DEBUG_THREADCOUNT
void oz_decSolveThreads(Board *bb, char *);
#else
void oz_decSolveThreads(Board *bb);
#endif
DebugCode(Bool oz_isInSolveDebug(Board *bb);)

/* -----------------------------------------------------------------------
 * inlineS
 * -----------------------------------------------------------------------*/

inline
void oz_deinstallCurrent()
{
  oz_reduceTrailOnSuspend();
  oz_currentBoard()->unsetInstalled();
  am.setCurrent(oz_currentBoard()->getParent());
}

inline
void oz_deinstallPath(Board *top)
{
  Assert(!top->isCommitted() && !top->isFailed());

  while (!oz_isCurrentBoard(top)) {
    oz_deinstallCurrent();
  }
}

// entailment check
inline
Bool oz_entailment() {
  return (!oz_currentBoard()->hasSuspension()  // threads?
          && am.trail.isEmptyChunk());       // constraints?
}

#endif
