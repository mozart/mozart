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
    : maxsize(0), size(0), head(0), tail(-1), queue (NULL)  {}

  ~ThreadQueueImpl (void) {
  }
  
  Bool isEmpty () { return (size == 0); }
  int getSize () { return (size); }
  Bool isAllocated () { return (maxsize); }

  int suggestNewSize(void) {
    return max(min(size * 2,(maxsize + size) >> 1), QUEUEMINSIZE);
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
  void print(void);

  Board * getHighestSolveDebug(void); // TMUELLER
  int getRunnableNumber();
  void deleteThread(Thread *th);

  void disposePool () {
    freeListDispose (queue, (size_t) (maxsize * sizeof (Thread *)));
  }
};

class ThreadQueue : public ThreadQueueImpl {
public:
  ThreadQueue(void) : ThreadQueueImpl() {}
  
  Bool isScheduled (Thread * thr);
  void gc();
  void printThreads(void);
};

class ThreadStackImpl {
private:
  int tos, size, maxsize;
  Thread ** stack;

  static int fromBos;

  void resize(void);
public:
  
  ThreadStackImpl(void) : tos(0), size(0), maxsize(0), stack(NULL) {}
  ~ThreadStackImpl(void) {}

  USEFREELISTMEMORY;
 
  void allocate(int initsize) {
    maxsize = initsize;
    tos = size = 0;
    // in the Oz heap;
    stack = 
      (Thread **) heapMalloc ((size_t) (sizeof(Thread *) * initsize));
  }

  Bool isEmpty () { return (size == 0); }
  int getSize () { return (size); }
  Bool isAllocated () { return (maxsize); }

  // push
  void enqueue (Thread * th) {
    if (size == maxsize) resize();
    stack[tos++] = th;
    size++;
  }

  // pop
  Thread * dequeue(void) {
    Assert(!isEmpty());
    Thread * th = stack[--tos];
    size--;
    return th;
  }

  Thread * readOutFromBottom(void) {
    return (fromBos == tos) ? (Thread *) NULL : stack[fromBos++];
  }
  void initReadOutFromBottom(void) { fromBos = 0; }

  void print(void) {}

  int suggestNewSize(void) {
    return max(min(size * 2,(maxsize + size) >> 1), QUEUEMINSIZE);
  }

  void disposePool () {
    freeListDispose (stack, (size_t) (maxsize * sizeof (Thread *)));
  }
}; // class ThreadStackImpl
 
//-----------------------------------------------------------------------------
//#define LOCAL_THREAD_STACK


#ifdef LOCAL_THREAD_STACK
typedef ThreadStackImpl LocalThreadImpl;
#else
typedef ThreadQueueImpl LocalThreadImpl;
#endif

class LocalThreadQueue : public LocalThreadImpl {
private:
  // needed when merging spaces to unpack threads in local thread queue
  Thread * ltq_thr;
public:
  LocalThreadQueue(Thread * lthr, Thread * thr) 
    : ltq_thr(lthr), LocalThreadImpl() 
  {
    allocate(QUEUEMINSIZE);
    enqueue(thr);
  }
  LocalThreadQueue(int sz) : LocalThreadImpl(){
    allocate(sz);
  }
  ~LocalThreadQueue() { error("never destroy LTQ"); }

  LocalThreadQueue * gc(void);

  void dispose () {
    LocalThreadImpl::disposePool();
    freeListDispose (this, sizeof(LocalThreadQueue));
  }

  Thread * getLTQThread(void) { return ltq_thr; }
};

#endif /* __THREADQUEUEH */


