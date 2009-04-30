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
  bb->incSuspCount();
  return th;
}

Thread * oz_newThread(int prio) {
  Board *bb  = oz_currentBoard();
  Thread *tt = _newThread(prio,bb);

  tt->setRunnable();

  if (!bb->isRoot())
    bb->incRunnableThreads();
  
  am.threadsPool.scheduleThread(tt);
  return tt;
}


Thread * oz_newThreadToplevel() {
  Board *bb=oz_rootBoard();
  Thread *tt = _newThread(DEFAULT_PRIORITY,bb);
  tt->setRunnable();
  am.threadsPool.scheduleThread(tt);
  return tt;
}

Thread * oz_newThreadInject(Board *bb) {
  Thread *tt = _newThread(DEFAULT_PRIORITY,bb);
  tt->setRunnable();

  if (!bb->isRoot())
    bb->incRunnableThreads();

  am.threadsPool.scheduleThread(tt);
  return tt;
}

Thread * oz_newThreadSuspended(int prio) {
  return _newThread(prio,oz_currentBoard());
}

Thread * oz_newThreadSuspended(Board* bb, int prio) {
  return _newThread(prio,bb);
}

Thread * oz_newThreadPropagate(Board *bb) {
  Thread *tt = _newThread(DEFAULT_PRIORITY,bb);
  tt->pushCall(BI_skip,NULL);
  return tt;
}


//  Dispose a thread.
void oz_disposeThread(Thread *tt) {
  tt->setDead();

  if (am.debugmode() && tt->isTrace())
    debugStreamTerm(tt);
  
  tt->disposeStack();
}

void oz_wakeupThread(Thread *tt) {
  Assert(tt->isSuspended());
  //printf("oz_wakeupThread1\n");fflush(stdout);
  tt->setRunnable();
  //printf("oz_wakeupThread2\n");fflush(stdout);
  if (am.debugmode() && tt->isTrace())
    debugStreamReady(tt);
  //printf("oz_wakeupThread3\n");fflush(stdout);
  am.threadsPool.scheduleThread(tt);
  //printf("oz_wakeupThread4\n");fflush(stdout);
  Board * bb = GETBOARD(tt);
  //printf("oz_wakeupThread5\n");fflush(stdout);
  if (!bb->isRoot()) {
    
    bb->incRunnableThreads();
    //printf("oz_wakeupThread6\n");fflush(stdout);
    if (tt->isExternal()) {
      //printf("oz_wakeupThread7\n");fflush(stdout);
      do {
	bb->clearSuspList(tt);
	//printf("oz_wakeupThread8 root:%p bb:%p\n",oz_rootBoard(),bb);fflush(stdout);
	bb = bb->getParent();
	//printf("oz_wakeupThread10\n");fflush(stdout);
      } while (!bb->isRoot());
      //printf("oz_wakeupThread9\n");fflush(stdout);
    }
  
    tt->unsetExternal();
    
  }
  //printf("oz_wakeupThread end\n");fflush(stdout);
}
