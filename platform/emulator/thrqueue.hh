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
};

class ThreadQueue : public ThreadQueueImpl {
public:
  ThreadQueue(void) : ThreadQueueImpl() {}

  Bool isScheduled (Thread * thr);
  void doGC ();
};

class LocalThreadQueue : public ThreadQueueImpl {
public:
  LocalThreadQueue(void) : ThreadQueueImpl() {}
  LocalThreadQueue(Thread * thr) : ThreadQueueImpl() {
    allocate(0x20);
    enqueue(thr);
  }
  LocalThreadQueue(int sz) : ThreadQueueImpl(){
    allocate(0x20);
  }
  LocalThreadQueue * gc(void);
};

#endif
