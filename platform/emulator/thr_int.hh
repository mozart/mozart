/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#ifndef __TIHH
#define __TIHH

#include "base.hh"
#include "thr_class.hh"
#include "board.hh"
#include "am.hh"

// imports
int oz_incSolveThreads(Board *bb);
DebugCode(Bool oz_isInSolveDebug(Board *bb);)
void debugStreamTerm(Thread*);
void debugStreamReady(Thread*);

void oz_checkExtSuspension(Suspension susp, Board * home);
void oz_checkExtSuspensionOutlined(Suspension susp);
void oz_removeExtThreadOutlined(Thread *tt);

// exports
#define CheckExtSuspension(susp)		\
  if (((Suspension)susp).wasExtSuspension()) {	\
    oz_checkExtSuspensionOutlined(susp);	\
  }

/* -------------------------------------------------------------------------
 * Threads
 * ------------------------------------------------------------------------- */

static
inline
Thread * _newThread(int prio, Board *bb) {
  Thread *th = new Thread(S_RTHREAD | T_runnable,prio,bb,am.newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();
  oz_checkDebug(th,bb);
  return th;
}

inline
Thread * oz_newThread(int prio=DEFAULT_PRIORITY)
{
  Board *bb=oz_currentBoard();
  Thread *tt = _newThread(prio,bb);

  if (am.isBelowSolveBoard()) {
    int inSolve=oz_incSolveThreads(bb);
    Assert(inSolve);
    tt->setInSolve();
  } else {
    Assert(!oz_isInSolveDebug(bb));
  }
  am.threadsPool.scheduleThread(tt);
  return tt;
}


inline
Thread * oz_newThreadToplevel(int prio=DEFAULT_PRIORITY)
{
  Board *bb=oz_rootBoard();
  Thread *tt = _newThread(prio,bb);

  am.threadsPool.scheduleThread(tt);
  return tt;
}

inline
Thread * oz_newThreadInject(int prio,Board *bb)
{
  Thread *tt = _newThread(prio,bb);

  int inSolve = oz_incSolveThreads(bb);
  if (inSolve) {
    tt->setInSolve();
  }
  am.threadsPool.scheduleThread(tt);
  return tt;
}


inline
Thread * oz_newThreadSuspended(int prio=DEFAULT_PRIORITY)
{
  Board *bb = oz_currentBoard();
  Thread *th = new Thread(S_RTHREAD,prio,bb,am.newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();

  oz_checkDebug(th,bb);

  return th;
}

inline
Thread * oz_newThreadPropagate(Board *bb) 
{
  Thread *th = new Thread(S_WAKEUP,DEFAULT_PRIORITY,bb,am.newId());
  bb->incSuspCount();
  oz_checkDebug(th,bb);
  return th;
}


//  Dispose a thread.
inline
void oz_disposeThread(Thread *tt)
{
#ifdef DEBUG_THREADCOUNT
  if (tt->isRunnable() && tt->isLPQThread())
    existingLTQs -= 1;
#endif
  Assert(!tt->isSuspended() || !GETBOARD(tt)->checkAlive());

  tt->markDeadThread();

  if (am.debugmode() && tt->getTrace())
    debugStreamTerm(tt);
  
  switch (tt->getThrType()) {
  case S_RTHREAD: 
    am.threadsPool.freeThreadBody(tt);
    break;
    
  case S_WAKEUP: 
    break;
    
  default: 
    Assert(0);
  }
}

inline
static
void _wakeupThread(Thread *tt)
{
  Assert(tt->isSuspended());

  tt->markRunnable();

  if (am.debugmode() && tt->getTrace()) {
    //Thread *t; if ((t = oz_currentThread()) && t->isTraced())
    //  execBreakpoint(t);
    debugStreamReady(tt);
  }
  am.threadsPool.scheduleThread(tt);
}

inline
void oz_removeExtThread(Thread *tt) 
{
  if (tt->wasExtThread()) {
    oz_removeExtThreadOutlined(tt);
  }
}

inline 
void oz_wakeupThreadOPT(Thread *tt)
{
  _wakeupThread(tt);

  Assert(oz_isCurrentBoard(GETBOARD(tt)) || tt->isExtThread() ||
	 (oz_isCurrentBoard(GETBOARD(tt)->getParent())
	  && !GETBOARD(tt)->isSolve()));

  if (am.isBelowSolveBoard() || tt->isExtThread()) {
    Assert(oz_isInSolveDebug(GETBOARD(tt)));
    oz_incSolveThreads(GETBOARD(tt));
    tt->setInSolve();
    oz_removeExtThread(tt);
    tt->clearExtThread();
  } else {
    Assert(!oz_isInSolveDebug(GETBOARD(tt)));
  }
}

inline 
void oz_wakeupThread(Thread *tt)
{
  _wakeupThread(tt);
  
  int inSolve = oz_incSolveThreads(GETBOARD(tt));
  if (inSolve) {
    tt->setInSolve();
    oz_removeExtThread(tt);
    tt->clearExtThread();
  }
}

#endif
