/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "thrqueue.hh"
#endif

#include "tagged.hh"
#include "value.hh"
#include "cont.hh"
#include "actor.hh"
#include "board.hh"
#include "stack.hh"
#include "taskstk.hh"
#include "am.hh"
#include "thread.hh"

#include "thrqueue.hh"


Bool ThreadQueue::isScheduled (Thread *thr)
{
  int currentSize = size;
  int currentHead = head;
  int mod = maxsize - 1;

  while (currentSize) {
    if (queue[currentHead] == thr) return (OK);
    INC(currentHead);
    currentSize--;
  }

  return (NO);
}

void ThreadQueueImpl::resize () {
  int new_maxsize = (maxsize * 3) >> 1;
  Thread ** new_queue =
    (Thread **) heapMalloc ((size_t) (sizeof(Thread *) * new_maxsize));

  int index = 0;
  int currentSize = size;
  int currentHead = head;

  DebugCode(message("Resizing thread queue 0x%x --> 0x%x.\n",
                    maxsize, new_maxsize));
  while (currentSize) {
    new_queue[index++] = queue[currentHead];
    INC(currentHead);
    currentSize--;
  }

  freeListDispose (queue, (size_t) (maxsize * sizeof (Thread *)));
  queue = new_queue;
  head = 0;
  tail = size - 1;
  maxsize = new_maxsize;
}

Board * ThreadQueueImpl::getHighestSolveDebug(void)
{
  int asize = size;
  int ahead = head;
  int mod = maxsize - 1;

  while (asize) {
    Board * c = queue[ahead]->getBoard()->getHighestSolveDebug();
    if (c)
      return c;
    INC(ahead);
    asize--;
  }

  return NULL;
}

int ThreadQueueImpl::getRunnableNumber()
{
  int ret=0;
  int j=head;
  for (int i=size; i > 0; i--) {
    ret+=queue[j]->getRunnableNumber();
    INC(j);
  }
  return ret;
}

void ThreadQueueImpl::deleteThread(Thread *th)
{
  int ahead = head;

  for (int i = size; i > 0 ; i--) {
    if (queue[ahead] == th) {
      for (int j = i-1; j > 0; j--) {
        int last=ahead;
        INC(ahead);
        queue[last] = queue[ahead];
      }
      size--;
      tail = tail-1; if (tail < 0 && size>0) tail = maxsize-1;
      return;
    }
    INC(ahead);
  }
}
