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
#include "board.hh"

int ThreadsPool::getRunnableNumber() {
  return 
     hiQueue.getRunnableNumber()
    +midQueue.getRunnableNumber()
    +lowQueue.getRunnableNumber()
    +1; // for am.currentThread!
}

//  Note that this can be called only after 'ozconf.init ()';
// (so, it's not a constructor of the 'ThreadsPool' class;)
void ThreadsPool::initThreads ()
{
#ifndef LINKED_QUEUES
  // private;
  hiQueue.allocate(QUEUEMINSIZE);
  midQueue.allocate(QUEUEMINSIZE);
  lowQueue.allocate(QUEUEMINSIZE);
#endif

  hi  = ozconf.hiMidRatio;
  mid = ozconf.midLowRatio;

  threadBodyFreeList = NULL;
}

//
Bool ThreadsPool::isScheduledSlow(Thread *thr)
{
  if (midQueue.isScheduledSlow(thr)) return OK;
  if (hiQueue.isScheduledSlow(thr)) return OK;
  return lowQueue.isScheduledSlow(thr);
}

void ThreadsPool::deleteThread(Thread *th)
{
  midQueue.deleteThread(th);
  hiQueue.deleteThread(th);
  lowQueue.deleteThread(th);
}

void ThreadsPool::rescheduleThread(Thread *th)
{
  deleteThread(th);
  scheduleThread(th);
}

