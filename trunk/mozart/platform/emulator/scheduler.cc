/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#include "value.hh"
#include "trace.hh"
#include "thr_int.hh"
#include "prop_int.hh"
#include "builtins.hh"
#include "debug.hh"
#include "space.hh"


/*
 * Checking stability
 *
 */

inline
void checkStability(Thread * ct, Board * cb) {
  if (cb->isRoot()) 
    return;

  Assert(!cb->isFailed() && !cb->isCommitted());

  Board * pb = cb->getParent();

  if (cb->decThreads() != 0) {
    pb->decSolveThreads();
    return;
  }
    
  cb->checkStability();
    
  Assert(!pb->isCommitted());

  pb->decSolveThreads();
}



void scheduler(void) {
  register AM * const e	= &am;

  register Thread * ct;
  register Board  * cb;

  do {

    /*
     * Do everything that is expensive and ugly: garbage collection...
     *
     */

    e->checkStatus(OK);


    /*
     * Get thread from queue
     *
     */

    do {

      ct = e->threadsPool.getNext();

      if (ct == (Thread *) NULL) {
	e->suspendEngine();
	continue;
      }

      if (!ct->isStop())
	break;

    } while (1);

    Assert(ct);

    // The thread must be alive and kicking, ready to rumble!
    Assert(!ct->isDead() && ct->isRunnable());


    /*
     * Install space in which thread is situated
     *
     */

    cb = ct->getBoardInternal()->derefBoard();

    // Normalize the current thread's board
    ct->setBoardInternal(cb);

    if (!cb->isAlive()) {
      // Some space superordinated to ct's home is dead
      
      if (!cb->isRoot())
	cb->getParent()->decSolveThreads();
    
      oz_disposeThread(ct);
      continue;

    }

    if (!oz_installPath(cb)) {
      cb = oz_currentBoard();
      goto LBLfailure;
    }

    Assert(oz_currentBoard() == cb);

    e->restartThread(); // start a new time slice
    // fall through


    /*
     * Run thread
     *
     */

  LBLrunThread:
    {

      e->setCurrentThread(ct);
      ozstat.leaveCall(ct->abstr);
      ct->abstr = 0;
      e->cachedStack = ct->getTaskStackRef();
      e->cachedSelf  = (Object *) 0;
      
      int ret = engine(NO);
      
      ct->setAbstr(ozstat.currAbstr);
      ozstat.leaveCall(NULL);

      if (e->getSelf()) {
	ct->pushSelf(e->getSelf());
	e->cachedSelf = (Object *) NULL;
      }
      
      switch (ret) {

      case T_PREEMPT:
	am.threadsPool.scheduleThread(ct);
	break;

      case T_SUSPEND:
	Assert(!cb->isFailed());
	ct->unsetRunnable();
	  
	if (cb->isRoot()) {
	  if (e->debugmode() && ct->isTrace())
	    debugStreamBlocked(ct);
	} else {
	  checkStability(ct,cb);
	}
	
	break;

      case T_TERMINATE:
	Assert(!ct->isDead() && ct->isRunnable() && ct->isEmpty());
	cb->decSuspCount();
	oz_disposeThread(ct);
	checkStability(ct,cb);
	break;
	
      case T_FAILURE:
	goto LBLfailure;

      case T_ERROR:
      default:
	Assert(0);
      }

      continue;
    }
    
    
    /*
     * Fail Thread
     *
     *  - can be entered only in subordinated space
     *  - current thread must be runnable
     */

  LBLfailure:
    {
      // Note that cb might be different from the thread's home:
      // this can happen while trying to install the space!
      Assert(ct->isRunnable());
      
      Board * pb = cb->getParent();

      Assert(!cb->isRoot());
      
      cb->setFailed();
      
      e->trail.unwindFailed();
      
      am.setCurrent(pb);
      
      if (!oz_unify(cb->getStatus(),cb->genFailed())) {
	Assert(0);
      }
     
      pb->decSolveThreads();

      // tmueller: this experimental
#ifdef NAME_PROPAGATORS
      if (!e->isPropagatorLocation())
	oz_disposeThread(ct);
#endif

      continue;
    }
   
  } while(1);

}

