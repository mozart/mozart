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
#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "thr_lqueue.hh"
#endif

#include "thr_lqueue.hh"
#include "thr_class.hh"

// -- IMPLEMENTATION OF free list for block

LinkedQueueBlock*
LinkedQueueBlock::freelist[LinkedQueueBlock::FreeListSize];

// -- IMPLEMENTATION OF LinkedQueueImpl
// -- could be moved to its own file e.g. lqueue.cc

// merging is realized by taking the linked list of block from q
// and linking it at the end of the linked list from this.  The
// dequeued prefix of q must be zeroed to cause these entries to
// be skipped as holes.

LinkedQueueImpl *
LinkedQueueImpl::mergeUnsafe(LinkedQueueImpl * q)
{
  if (this==0) return q;
  if (q==0) return this;
  // zero the dequeued prefix of q
  memset(q->tail->array+q->tail_index,0,
	 sizeof(void*)*(q->tail->size - q->tail_index));
  head = head->next = q->tail;
  head_index = q->head_index;
  size += q->size;
  q->zeroAll();
  return this;
}

void** LinkedQueueImpl::find(void* x)
{
  LinkedQueueIteratorImpl iter(this);
  void** y;
  while ((y=iter.getPointerToNext())) if (x==*y) return y;
  return 0;
}

void LinkedQueueImpl::remove(void* x)
{
  void** y = find(x);
  if (y) { *y=0; size--; }
}

// -- IMPLEMENTATION OF ThreadQueue

// template class LinkedQueue<Thread,THREAD_QUEUE_SIZE>;
// template class LinkedQueueIterator<Thread,THREAD_QUEUE_SIZE>;
// template class LinkedQueue<Propagator,PROPAGATOR_QUEUE_SIZE>;
// template class LinkedQueueIterator<Propagator,PROPAGATOR_QUEUE_SIZE>;

int ThreadQueue::getRunnableNumber()
{
  ThreadQueueIterator iter(this);
  int ret=0;
  Thread*ptr;
  while ((ptr=iter.getNext())) ret+=ptr->getRunnableNumber();
  return ret;
}

void initLinkedQueueFreeList() {
  LinkedQueueBlock::initFreeList();
}
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
