/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(__GNUC__) && !defined(NOPRAGMA)
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
  rootThread = newThread (ozconf.defaultPriority, am.rootBoard,PARMODE);
}

void ThreadsPool::scheduleThreadOutline (Thread *th, int pri)
{
  ThreadQueue *insQueue = &queues[pri];

  Assert ((pri != currentPriority));

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
      if (nextPrio[ix] > pri) continue; // not yet;

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


#ifdef OUTLINE
#define inline
#include "thrspool.icc"
#undef inline
#endif
