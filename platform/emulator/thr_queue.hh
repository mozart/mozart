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

class ThreadQueueImpl {
protected:
  int head, tail, size, maxsize;
  //  maxsize is required to be a power of 2;
  Thread ** queue;

  void resize (void);

public:
  //
  ThreadQueueImpl(void) 
    : maxsize(0), size(0), head(0), tail(-1), queue (NULL)  {}

  ~ThreadQueueImpl (void) {
    if (queue) delete queue;
    maxsize = size = head = tail = -1;
  }
  
  Bool isEmpty () { return (size == 0); }
  int getSize () { return (size); }
  Bool isAllocated () { return (maxsize); }

  void allocate (int initsize) {
    maxsize = initsize;
    head = size = 0;
    tail = initsize - 1;
    queue = ::new Thread*[initsize];
  }

  void enqueue (Thread * th) {
    if (size == maxsize) resize ();
    tail = (tail + 1) & (maxsize - 1);
    queue[tail] = th;
    size++;
  }

  Thread * dequeue (void) {
    Assert (!isEmpty ());
    Thread * th = queue[head];
    head = (head + 1) & (maxsize - 1);
    size--;
    return (th);
  }
  void print(void);

  Board * getHighestSolveDebug(void); // TMUELLER
};

class ThreadQueue : public ThreadQueueImpl {
public:
  ThreadQueue(void) : ThreadQueueImpl() {}
  
  Bool isScheduled (Thread * thr);
  void doGC ();
};

class LocalThreadQueue : public ThreadQueueImpl {
private:
  // needed when merging spaces to unpack threads in local thread queue
  Thread * ltq_thr;
public:
  USEFREELISTMEMORY;

  void allocate (int initsize) {
    maxsize = initsize;
    head = size = 0;
    tail = initsize - 1;
    // in the Oz heap;
    queue = 
      (Thread **) heapMalloc ((size_t) (sizeof(Thread *) * initsize));
  }

  LocalThreadQueue(Thread * lthr, Thread * thr) 
    : ltq_thr(lthr), ThreadQueueImpl() 
  {
    allocate(0x20);
    enqueue(thr);
  }
  LocalThreadQueue(int sz) : ThreadQueueImpl(){
    allocate(0x20);
  }
  ~LocalThreadQueue() { error("never destroy LTQ"); }

  LocalThreadQueue * gc(void);
  void dispose () {
    freeListDispose (queue, (size_t) (maxsize * sizeof (Thread *)));
    freeListDispose (this, sizeof(LocalThreadQueue));
  }

  void resize();

  // because of resize;
  void enqueue (Thread * th) {
    if (size == maxsize) resize ();
    tail = (tail + 1) & (maxsize - 1);
    queue[tail] = th;
    size++;
  }

  Thread * getLTQThread(void) { return ltq_thr; }
};

#endif


