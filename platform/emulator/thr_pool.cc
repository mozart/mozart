/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "thrspool.hh"
#endif

#include "../include/config.h"
#include "am.hh"

int ThreadsPool::getRunnableNumber()
{
  return hiQueue.getRunnableNumber()
    +midQueue.getRunnableNumber()
    +lowQueue.getRunnableNumber()
    +1; // for am.currentThread!
}

//  Note that this can be called only after 'ozconf.init ()';
// (so, it's not a constructor of the 'ThreadsPool' class;)
void ThreadsPool::initThreads ()
{
  // private;
  hiQueue.allocate(THREAD_QUEUE_SIZE);
  midQueue.allocate(THREAD_QUEUE_SIZE);
  lowQueue.allocate(THREAD_QUEUE_SIZE);

  hiCounter = -1;
  lowCounter = -1;

  // public part;
  currentThread = (Thread *) NULL;
  ozstat.createdThreads.incf();
  rootThread = am.mkRunnableThread (SYSTEM_PRIORITY, am.rootBoard,0);

  // 
  threadBodyFreeList = NULL;
}

//
Bool ThreadsPool::isScheduled (Thread *thr)
{
  if (midQueue.isScheduled(thr)) return OK;
  if (hiQueue.isScheduled(thr)) return OK;
  return lowQueue.isScheduled(thr);
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


Board * ThreadsPool::getHighestSolveDebug(void) 
{
  Board *b;
  b = hiQueue.getHighestSolveDebug();
  if (b) return b;
  b = midQueue.getHighestSolveDebug();
  if (b) return b;
  return lowQueue.getHighestSolveDebug();
}

Thread *ThreadsPool::getFirstThreadOutline()
{
  /*
   * empty hiQueue
   */
  if (hiCounter < 0) {
    Assert(hiQueue.isEmpty());

    Assert(lowCounter >= 0); /* other case inline version */

    Assert(!lowQueue.isEmpty() || !midQueue.isEmpty());
    lowCounter--;
    if (lowCounter < 0 || midQueue.isEmpty()) {
      if (!lowQueue.isEmpty()) {
	lowCounter=ozconf.midLowRatio;
	return lowQueue.dequeue();
      } else {
	lowCounter=-1;
	return midQueue.dequeue();
      }
    }
    return midQueue.dequeue();
  }

  /*
   * use hiQueue, else mid/low
   */
  hiCounter--;
  if (hiCounter >= 0) {
    if (!hiQueue.isEmpty()) { return hiQueue.dequeue(); }
    hiCounter = -1;
    goto mid;
  }

  /*
   * use midQueue, else hiQueue
   */
  hiCounter=ozconf.hiMidRatio;
mid:
  if (lowCounter < 0) {
    return !midQueue.isEmpty() ? midQueue.dequeue() : hiQueue.dequeue();
  }

  /*
   * use midQueue, else lowQueue, else hiQueue
   */
  lowCounter--;
  if (lowCounter >= 0) {
    if (!midQueue.isEmpty()) return midQueue.dequeue();

    if (!lowQueue.isEmpty()) {
      lowCounter = ozconf.midLowRatio;
      return lowQueue.dequeue();
    } else {
      lowCounter = -1;
      return hiQueue.dequeue();
    }
  }

  /*
   * use lowQueue, else midQueue, else hiQueue
   */
  if (!lowQueue.isEmpty()) {
    lowCounter = ozconf.midLowRatio;
    return lowQueue.dequeue();
  } else {
    lowCounter = -1;
    return !midQueue.isEmpty() ? midQueue.dequeue() : hiQueue.dequeue();
  }
}

void ThreadsPool::scheduleThread(Thread *th,int pri)
{
  if (pri < 0) pri = th->getPriority();

  if (pri == MID_PRIORITY) {
    midQueue.enqueue(th);
  } else if (pri == HI_PRIORITY) {
    hiQueue.enqueue(th);
    if (hiCounter<0) hiCounter=ozconf.hiMidRatio;
  } else {
    lowQueue.enqueue(th);
    if (lowCounter<0) lowCounter=ozconf.midLowRatio;
  }
}

Bool ThreadsPool::threadQueuesAreEmptyOutline()
{
  if (!midQueue.isEmpty()) return NO;
  if (hiCounter > 0) {
    if (!hiQueue.isEmpty()) return NO;
  }
  if (lowCounter > 0) {
    if (!lowQueue.isEmpty()) return NO;
  }
  return OK;
}

#ifdef OUTLINE
#define inline
#include "thrspool.icc"
#undef inline
#endif
