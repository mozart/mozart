/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __THREADQUEUEH
#define __THREADQUEUEH

#ifdef INTERFACE
#pragma interface
#endif

typedef Thread* ThreadPtr;

//
//  A queue a'la 'LocalPropagationQueue' by Tobias;

#define INC(a)  { a = (a==maxsize ? 0 : a+1); }

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
    : maxsize(0), size(0), head(0), tail(-1), queue (NULL)  {}

  ~ThreadQueueImpl (void) {
  }

  Bool isEmpty () { return (size == 0); }
  int getSize () { return (size); }
  Bool isAllocated () { return (maxsize); }

  int suggestNewSize() {
    return max(min(size * 2,(maxsize + size) >> 1), QUEUEMINSIZE);
  }

  void enqueue (Thread * th) {
    if (size == maxsize) 
      resize();
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

  void print(void);

  Board * getHighestSolveDebug(void); // TMUELLER
  int getRunnableNumber();
  void deleteThread(Thread *th);
};


class ThreadQueue : public ThreadQueueImpl {
public:
  ThreadQueue() : ThreadQueueImpl() {}
  
  Bool isScheduled (Thread * thr);
  void gc(void);
  void printThreads(void);
};


class LocalThreadQueue : public ThreadQueueImpl {
private:
  // needed when merging spaces to unpack threads in local thread queue
  Thread * ltq_thr;
public:
  USEFREELISTMEMORY;

  LocalThreadQueue(Thread * lthr, Thread * thr) 
    : ltq_thr(lthr), ThreadQueueImpl() {
      allocate(QUEUEMINSIZE);
      enqueue(thr);
  }

  LocalThreadQueue(int sz) : ThreadQueueImpl(){
    allocate(sz);
  }

  ~LocalThreadQueue() { error("never destroy LTQ"); }

  LocalThreadQueue * gc(void);

  void dispose () {
    freeListDispose(queue, (size_t) (maxsize * sizeof (Thread *)));
    freeListDispose(this, sizeof(LocalThreadQueue));
  }
  
  Thread * getLTQThread(void) { return ltq_thr; }
};

#endif


