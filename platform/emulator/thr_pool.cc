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

//  Note that this can be called only after 'ozconf.init ()';
// (so, it's not a constructor of the 'ThreadsPool' class;)
void ThreadsPool::initThreads ()
{
  // private;
  nextPrioInd = -1;
  currentPriority = -1;
  currentQueue = (ThreadQueue *) NULL;

  // public part;
  currentThread = (Thread *) NULL;
  ozstat.createdThreads.incf();
  rootThread = am.mkRunnableThread (ozconf.systemPriority, am.rootBoard,0);

  // 
  threadBodyFreeList = NULL;
}

//
Bool ThreadsPool::isScheduled (Thread *thr)
{
  return (queues[thr->getPriority ()].isScheduled (thr));
}

//
void ThreadsPool::scheduleThreadOutline(Thread *th, int pri)
{
  ThreadQueue *insQueue = &queues[pri];
  
  if (!insQueue->isAllocated ()) {
    insQueue->allocate (THREAD_QUEUE_SIZE);
  }
  insQueue->enqueue (th);
  
  if (pri > currentPriority) {
    if (currentPriority != -1) {
      nextPrio[++nextPrioInd] = currentPriority;
    }
    currentPriority = pri;
    currentQueue = insQueue;
  } else {
    // look through the stack (nextPrio) and insert when necessary;
    int ix, jx;
    
    for (ix = nextPrioInd; ix >= 0; ix--) {
      if (nextPrio[ix] > pri) continue;	// not yet;
      
      if (nextPrio[ix] == pri) return;
      // i.e. it's O.k. already - it will be taken anyway;
      
      //  otherwise (nextPrio[ix] < pri) - insert a new entry;
      break;
    }
    
    Assert (ix < 0 || nextPrio[ix] < pri);
    // invariants: ix indexes the queue _before_ inserted one;
    //  (ix's entry stays at the place;)
    for (jx = nextPrioInd; jx > ix; jx--) 
      nextPrio[jx+1] = nextPrio[jx];
    
    nextPrio[ix+1] = pri;
    nextPrioInd++;
  }
}

void ThreadsPool::scheduleThread (Thread *th) {
  int pri = th->getPriority ();
  
  Assert (!th->isDeadThread ());
  Assert (th->isRunnable ());
  Assert (!(isScheduled (th)));
  
  if (pri == currentPriority) {
    currentQueue->enqueue (th);
  } else {
    scheduleThreadOutline (th, pri);
  }
}

void ThreadsPool::updateCurrentQueue()
{
  Assert(currentQueue->isEmpty());
  if (nextPrioInd != -1) {
    //  pick up the next one;
    int npri = nextPrio[nextPrioInd--];
    Assert ((npri < currentPriority));
    currentPriority = npri;
    currentQueue = &queues[npri];
  } else {
    //  reset 'currentQueue' and 'currentPriority';
    //  Note that already allocated queues stay allocated, of course.
    currentPriority = -1;
    currentQueue = (ThreadQueue *) NULL;
  }
}

void ThreadsPool::deleteThread(Thread *th1)
{
  Bool found=NO;
  ThreadQueue *thq = currentQueue;
  int prioInd = nextPrioInd;
  while (thq && !found) {
    int size=thq->getSize();
    while (size--) {
      Thread *th = thq->dequeue();
      if (th == th1) {
	found = OK;
      } else {
	thq->enqueue(th);
      }
    }

    if (found) {
      if (thq->isEmpty()) {
    
	// invariants: prioInd indexes the empty queue;
	for (int i = prioInd; i < nextPrioInd; i++) 
	  nextPrio[i] = nextPrio[i+1];
    
	nextPrioInd--;
	if (thq == currentQueue) {
	  updateCurrentQueue();
	}
      }
      break; //while
    }

    if (prioInd >= 0) {
      int pri = nextPrio[prioInd--];
      thq = &queues[pri];
    } else {
      thq = 0;
    }

  }
}

void ThreadsPool::rescheduleThread(Thread *th)
{
  deleteThread(th);
  scheduleThread(th);
}


Board * ThreadsPool::getHighestSolveDebug(void) 
{
  for (int i = TAB_SIZE; i--; ) {
    Board * b = queues[i].getHighestSolveDebug();
    if (b) 
      return b;
  }
  return NULL;
}

#ifdef OUTLINE
#define inline
#include "thrspool.icc"
#undef inline
#endif
