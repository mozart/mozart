/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifdef LINKED_QUEUES
#include "thr_lqueue.cc"
#else

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "thr_queue.hh"
#endif

#include "thr_queue.hh"
#include "thr_class.hh"

Bool ThreadQueue::isScheduledSlow(Thread *thr)
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

#endif /* !LINKED_QUEUES */
