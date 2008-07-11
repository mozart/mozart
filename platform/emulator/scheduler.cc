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
#include "thr_int.hh"
#include "prop_int.hh"
#include "builtins.hh"
#include "debug.hh"


inline
int run_thread(Thread * ct) {
  am.setCurrentThread(ct);

  ozstat.leaveCall(ct->getAbstr());
  ct->setAbstr(NULL);
  am.cachedStack = ct->getTaskStackRef();
  am.cachedSelf  = (OzObject *) 0;

  int ret = engine(NO);
  ct->setAbstr(ozstat.currAbstr);
  ozstat.leaveCall(NULL);
  
  if (am.getSelf()) {
    ct->pushSelf(am.getSelf());
    am.cachedSelf = (OzObject *) NULL;
  }

  am.setCurrentThread((Thread *) NULL);

  return ret;
}

void scheduler(void) {

  register Thread * ct;
  register Board  * cb;

  do {

    /*
     * Do everything that is expensive and ugly: garbage collection...
     *
     */

    am.checkStatus(OK);


    /*
     * Get thread from queue
     *
     */

    do {

      ct = am.threadsPool.getNext();
      if (ct == (Thread *) NULL) {
	am.suspendEngine();
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

    if (!cb->install()) {
      oz_disposeThread(ct);
      continue;
    }

    Assert(oz_currentBoard() == cb);
    switch (run_thread(ct)) {
      
    case T_PREEMPT:
      am.threadsPool.scheduleThread(ct);
      continue;
      
    case T_SUSPEND:
      Assert(!cb->isFailed());
      ct->unsetRunnable();
      
      if (cb->isRoot()) {
	if (am.debugmode() && ct->isTrace())
	  debugStreamBlocked(ct);
      } else {
	cb->checkStability();
      }
      continue;
      
    case T_TERMINATE:
      Assert(!ct->isDead() && ct->isRunnable() && ct->isEmpty());
      if (!cb->isRoot()) {
	cb->decSuspCount();
        cb->checkStability();
      }
      break;
      
    case T_FAILURE:
      cb->fail();
      break;
      
    case T_ERROR:
    default:
      Assert(0);
    }
   
    oz_disposeThread(ct);
    
  } while(1);

}

