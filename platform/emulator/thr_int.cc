/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#include "board.hh"
#include "am.hh"
#include "thr_int.hh"
#include "var_base.hh"
#include "thr_class.hh"


// debugger
void debugStreamTerm(Thread*);
void debugStreamReady(Thread*);

inline
Thread * _newThread(int prio, Board *bb) {
  Thread *th = new Thread(0,prio,bb,oz_newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();
  oz_checkDebug(th,bb);
  return th;
}

Thread * oz_newThread(int prio) {
  Board *bb  = oz_currentBoard();
  Thread *tt = _newThread(prio,bb);

  tt->markRunnable();

  if (!bb->isRoot())
    bb->incSolveThreads();
  
  am.threadsPool.scheduleThread(tt);
  return tt;
}


Thread * oz_newThreadToplevel() {
  Board *bb=oz_rootBoard();
  Thread *tt = _newThread(DEFAULT_PRIORITY,bb);
  tt->markRunnable();
  am.threadsPool.scheduleThread(tt);
  return tt;
}

Thread * oz_newThreadInject(Board *bb) {
  Thread *tt = _newThread(DEFAULT_PRIORITY,bb);
  tt->markRunnable();

  if (!bb->isRoot())
    bb->incSolveThreads();

  am.threadsPool.scheduleThread(tt);
  return tt;
}

Thread * oz_newThreadSuspended(int prio) {
  return _newThread(prio,oz_currentBoard());
}

Thread * oz_newThreadPropagate(Board *bb) {
  Thread *tt = _newThread(DEFAULT_PRIORITY,bb);
  tt->pushCall(BI_skip,0,0);
  return tt;
}


//  Dispose a thread.
void oz_disposeThread(Thread *tt) {
  tt->markDeadThread();

  if (am.debugmode() && tt->getTrace())
    debugStreamTerm(tt);
  
  am.threadsPool.freeThreadBody(tt);
}

void oz_wakeupThread(Thread *tt) {
  Assert(tt->isSuspended());

  tt->markRunnable();

  if (am.debugmode() && tt->getTrace())
    debugStreamReady(tt);
  
  am.threadsPool.scheduleThread(tt);
  
  Board * bb = GETBOARD(tt);
 
  if (!bb->isRoot()) {
    
    bb->incSolveThreads();

    if (tt->wasExtThread()) {
      do {
	bb->clearSuspList(tt);
	bb = bb->getParent();
      } while (!bb->isRoot());
    }

    tt->clearExtThread();
    
  }
}

