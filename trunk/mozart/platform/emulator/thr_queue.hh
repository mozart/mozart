/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __THREADQUEUEH
#define __THREADQUEUEH

#ifdef INTERFACE
#pragma interface
#endif

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

class LocalThreadQueue : public ThreadQueueImpl {
private:
  // needed when merging spaces to unpack threads in local thread queue
  Thread * ltq_thr;
public:
  LocalThreadQueue(Thread * lthr, Thread * thr) 
    : ltq_thr(lthr), ThreadQueueImpl() 
  {
    allocate(QUEUEMINSIZE);
    enqueue(thr);
  }
  LocalThreadQueue(int sz) : ThreadQueueImpl(){
    allocate(sz);
  }
  ~LocalThreadQueue();

  LocalThreadQueue * gc(void);

  void dispose () {
    ThreadQueueImpl::disposePool();
    freeListDispose (this, sizeof(LocalThreadQueue));
  }

  Thread * getLTQThread(void) { return ltq_thr; }
};

#endif /* __THREADQUEUEH */


