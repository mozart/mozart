/*
 *  Authors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Denys Duchier, 1998
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

#ifndef __THR_LQUEUE_HH
#define __THR_LQUEUE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "lqueue.hh"
#include "base.hh"

#ifndef THREAD_QUEUE_SIZE
#define THREAD_QUEUE_SIZE 100
#endif

class ThreadQueue
  : public LinkedQueue<Thread,THREAD_QUEUE_SIZE>
{
public:
  USEFREELISTMEMORY;
  OZPRINT;
  Bool isScheduledSlow(Thread *th) {
    return find(th)?OK:NO;
  }
  void gc();
  void printThreads(void);
  void deleteThread(Thread*th) { remove(th); }
  int getRunnableNumber();
};

class ThreadQueueIterator
  : public LinkedQueueIterator<Thread,THREAD_QUEUE_SIZE>
{
public:
  ThreadQueueIterator()
    : LinkedQueueIterator<Thread,THREAD_QUEUE_SIZE>( ){}
  ThreadQueueIterator(ThreadQueue*q)
    : LinkedQueueIterator<Thread,THREAD_QUEUE_SIZE>(q){}
  ThreadQueueIterator(ThreadQueue&q)
    : LinkedQueueIterator<Thread,THREAD_QUEUE_SIZE>(q){}
};

#ifndef PROPAGATOR_QUEUE_SIZE
#define PROPAGATOR_QUEUE_SIZE 100
#endif

class LocalPropagatorQueue
  : public LinkedQueue<Propagator,PROPAGATOR_QUEUE_SIZE>
{
private:
  Thread* lpq_thr;
public:
  USEFREELISTMEMORY;
  LocalPropagatorQueue(Thread * lthr, Propagator * p)
    : lpq_thr(lthr), LinkedQueue<Propagator,PROPAGATOR_QUEUE_SIZE>()
    {
      enqueue(p);
    }
  LocalPropagatorQueue()
    : lpq_thr(0), LinkedQueue<Propagator,PROPAGATOR_QUEUE_SIZE>() {}
  ~LocalPropagatorQueue();
  Thread * getLPQThread(void) { return lpq_thr; }
  LocalPropagatorQueue * gc();
  LocalPropagatorQueue * merge(LocalPropagatorQueue*q) {
    return (LocalPropagatorQueue*)
      LinkedQueue<Propagator,PROPAGATOR_QUEUE_SIZE>::merge(q);
  }
};

class LocalPropagatorQueueIterator
  : public LinkedQueueIterator<Propagator,PROPAGATOR_QUEUE_SIZE>
{
public:
  LocalPropagatorQueueIterator()
    : LinkedQueueIterator<Propagator,PROPAGATOR_QUEUE_SIZE>( ){}
  LocalPropagatorQueueIterator(LocalPropagatorQueue*q)
    : LinkedQueueIterator<Propagator,PROPAGATOR_QUEUE_SIZE>(q){}
  LocalPropagatorQueueIterator(LocalPropagatorQueue&q)
    : LinkedQueueIterator<Propagator,PROPAGATOR_QUEUE_SIZE>(q){}
};

#endif /* __THR_LQUEUE_HH */
