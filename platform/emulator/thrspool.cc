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
  hiQueue.allocate(QUEUEMINSIZE);
  midQueue.allocate(QUEUEMINSIZE);
  lowQueue.allocate(QUEUEMINSIZE);

  hiCounter = -1;
  lowCounter = -1;

  // public part;
  currentThread = (Thread *) NULL;
  ozstat.createdThreads.incf();

  rootThread = am.mkRunnableThread(DEFAULT_PRIORITY, am.rootBoard);
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
  Assert(hiCounter>=0 || lowCounter>=0); // otherwise inline version
  /*
   * empty hiQueue
   */
  if (hiCounter < 0) {
    Assert(hiQueue.isEmpty());
    Assert(!lowQueue.isEmpty() || !midQueue.isEmpty());

lowMid:
    if (lowCounter == 0 || midQueue.isEmpty()) {
      Assert(!lowQueue.isEmpty());
      Thread *th = lowQueue.dequeue();
      lowCounter = lowQueue.isEmpty() ? -1 : ozconf.midLowRatio;
      return th;
    }
    lowCounter--;
    return midQueue.dequeue();
  }

  /*
   * use hiQueue, else mid/low
   */
  if (hiCounter > 0 || (lowCounter < 0 && midQueue.isEmpty())) {
    Thread *th = hiQueue.dequeue();
    hiCounter = hiQueue.isEmpty() ? -1 :
      (hiCounter==0 ? ozconf.hiMidRatio : hiCounter-1);
    return th;
  }
  Assert(hiCounter==0);
  hiCounter=ozconf.hiMidRatio;

  Assert(lowCounter>=0 || !midQueue.isEmpty());
  goto lowMid;
}

void ThreadsPool::scheduleThread(Thread *th,int pri)
{
  Assert(!isScheduledSlow(th));
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
  if (hiCounter >= 0) {
    if (!hiQueue.isEmpty()) return NO;
  } else {
    Assert(hiQueue.isEmpty());
  }
  if (lowCounter >= 0) {
    if (!lowQueue.isEmpty()) return NO;
  } else {
    Assert(lowQueue.isEmpty());
  }
  return OK;
}

#ifdef OUTLINE
#define inline
#include "thrspool.icc"
#undef inline
#endif
