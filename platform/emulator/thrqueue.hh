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
class ThreadQueue {
private:
  int head, tail, size, maxsize;
  //  maxsize is required to be a power of 2;
  ThreadPtr *queue;

  void resize ();
public:
  //
  ThreadQueue () : maxsize(0), size(0), head(0), tail(-1), queue (NULL)
  {}
  ~ThreadQueue () {
    if (queue) delete queue;
    maxsize = size = head = tail = -1;
  }

  void doGC ();

  void allocate (int initsize) {
    maxsize = initsize;
    head = size = 0;
    tail = initsize - 1;
    queue = ::new ThreadPtr[initsize];
  }

  void enqueue (Thread *th) {
    if (size == maxsize) resize ();
    tail = (tail + 1) & (maxsize - 1);
    queue[tail] = th;
    size++;
  }
  Bool isEmpty () { return (size == 0); }
  Thread *dequeue () {
    Thread *th;
    Assert (!isEmpty ());
    th = queue[head];
    head = (head + 1) & (maxsize - 1);
    size--;
    return (th);
  }

  int getSize () { return (size); }
  Bool isAllocated () { return (maxsize); }

  Bool isScheduled (Thread *thr);
};

#endif
