/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
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

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "thr_pool.hh"
#endif

#include "thr_pool.hh"

void ThreadsPool::init(void) {
  hi = mid = 0;
  _q[ HI_PRIORITY].init();
  _q[MID_PRIORITY].init();
  _q[LOW_PRIORITY].init();
}

int ThreadsPool::getRunnableNumber() {
  return
    _q[ HI_PRIORITY].getSize() +
    _q[MID_PRIORITY].getSize() +
    _q[LOW_PRIORITY].getSize() +
    1;
}

Bool ThreadsPool::isScheduledSlow(Thread * thr) {
  return (_q[MID_PRIORITY].isIn(thr) ||
	  _q[ HI_PRIORITY].isIn(thr) ||
	  _q[LOW_PRIORITY].isIn(thr));
}

void ThreadsPool::deleteThread(Thread * thr) {
  _q[ HI_PRIORITY].remove(thr);
  _q[MID_PRIORITY].remove(thr);
  _q[LOW_PRIORITY].remove(thr);
}

void ThreadsPool::rescheduleThread(Thread *th) {
  deleteThread(th);
  scheduleThread(th);
}

