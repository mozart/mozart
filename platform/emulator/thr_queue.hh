/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __THREADQUEUEH
#define __THREADQUEUEH

#ifdef LINKED_QUEUES
#include "thr_lqueue.hh"
#else

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "mem.hh"

//
//  A queue a'la 'LocalPropagationQueue' by Tobias;

#define INC(a)  { (a) = ((a) + 1); if ((a)==maxsize) (a)=0; }

class ThreadQueueImpl {
protected:
  int head, tail, size, maxsize;
  Thread ** queue;

  void resize();

public:
  USEFREELISTMEMORY;

  void allocate(int initsize) {
    maxsize = initsize;
    head    = size = 0;
    tail    = initsize - 1;
    // in the Oz heap;
    queue =
      (Thread **) heapMalloc ((size_t) (sizeof(Thread *) * initsize));
  }

  ThreadQueueImpl(void)
    : head(0), tail(-1), size(0), maxsize(0), queue(NULL)  {}

  ~ThreadQueueImpl(void) {
  }

  Bool isEmpty () { return (size == 0); }
  int getSize () { return (size); }
  Bool isAllocated () { return (maxsize); }

  int suggestNewSize(void) {
    return max(min(size * 2,(maxsize + size + 1) >> 1), QUEUEMINSIZE);
  }

  void enqueue (Thread * th) {
    if (size == maxsize) resize ();
    INC(tail);
    queue[tail] = th;
    size++;
  }

  Thread * dequeue(void) {
    Assert(!isEmpty());
    Thread * th = queue[head];
    INC(head);
    size--;
    return (th);
  }
  OZPRINT;

#ifdef PROP_MERGING
  void merge(ThreadQueueImpl * tq) {
    while (!tq->isEmpty())
      enqueue(tq->dequeue());
  }
#endif

  int getRunnableNumber();
  void deleteThread(Thread *th);

  void disposePool () {
    freeListDispose (queue, (size_t) (maxsize * sizeof (Thread *)));
  }
};

class ThreadQueue : public ThreadQueueImpl {
public:
  ThreadQueue(void) : ThreadQueueImpl() {}

  Bool isScheduledSlow(Thread * thr);
  void gc();
  void printThreads(void);
};

class LocalPropagatorQueue : public ThreadQueueImpl {
private:
  // needed when merging spaces to unpack threads in local thread queue
  Thread * lpq_thr;
public:
  LocalPropagatorQueue(Thread * lthr, Propagator * p)
    : lpq_thr(lthr), ThreadQueueImpl()
  {
    allocate(QUEUEMINSIZE);
    enqueue(p);
  }
  LocalPropagatorQueue(int sz) : ThreadQueueImpl(){
    allocate(sz);
  }
  ~LocalPropagatorQueue();

  LocalPropagatorQueue * gc(void);

  Propagator * dequeue(void) {
    return (Propagator *) ThreadQueueImpl::dequeue();
  }

  void enqueue(Propagator * p) {
    ThreadQueueImpl::enqueue((Thread *) p);
  }

  void dispose () {
    ThreadQueueImpl::disposePool();
    freeListDispose (this, sizeof(LocalPropagatorQueue));
  }

  Thread * getLPQThread(void) { return lpq_thr; }

  LocalPropagatorQueue * merge(LocalPropagatorQueue * tq) {
    if (this == NULL) {
      // this == NULL and tq != NULL OR this == NULL  and tq == NULL
      return tq;
    } else if (tq != NULL) {
      // this != NULL and tq != NULL
      while (!isEmpty())
        tq->enqueue(dequeue());
      return tq;
    }
    // tq == NULL && this != NULL
    return this;
  }
};

#endif /* !LINKED_QUEUE */
#endif /* __THREADQUEUEH */
