/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __THRSPOOLH
#define __THRSPOOLH

#ifdef __GNUC__
#pragma interface
#endif

#include "thrqueue.hh"

// == 1 if local propagation integrated as MAX_PRIORITY+1 queue
// (forthcoming???)
#define ADDITIONAL_PRIOS  0

// just the size of a stack and lookup table;
#define TAB_SIZE   (MAX_PRIORITY-MIN_PRIORITY+1+ADDITIONAL_PRIOS)

class ThreadsPool {
private:
  //  A fixed-size stack wich contains the next priority to be taken;
  //  Note that 'char' limits NOW the range of possilbe priorities
  // to 0-255;
  //  Invariant: it contains (always) #'s of all non-empty queues in
  // descending order;
  char nextPrio[TAB_SIZE];
  //  ... top of that stack (i.e. the index of the last element)
  // or '-1' if it's empty;
  int nextPrioInd;

  //  thread queues (one per priority);
  ThreadQueue queues[TAB_SIZE];

  //  Pointer to the current queue;
  //  Invariant: always points at the non-empty queue with the highest
  // priority. HENCE, it must be updated by every 'scheduleThread ()';
  // It's equal to (ThreadQueue *) NULL iff currentPriority == -1;
  ThreadQueue *currentQueue;
  //  Invariant: it can range between MIN_PRIORITY and MAX_PRIORITY,
  // complains with the 'currentQueue', and is equal to '-1' when
  // there is no runnable thread;
  int currentPriority;

  //  Adjust a 'nextPrio' stack for the new thread with the
  // pri != currentPriority;
  void scheduleThreadOutline (Thread *th, int pri);
protected:
  //
public:
  //  formerly in AM;
  Thread *currentThread;
  Thread *rootThread;
  Thread *threadsFreeList;

  ThreadsPool () {};
  ~ThreadsPool () {};
  void initThreads();

  // in print.cc;
  void printThreads ();

  //
  void doGC ();

  Thread *newThread (int p,Board *h,int compMode);
  // ... in emulate.cc;
  void disposeThread (Thread *th);

  void scheduleThread (Thread *th);
  Bool threadQueueIsEmpty ();
  Thread *getFirstThread ();

  //  misc;
  int getNextThPri () { return (currentPriority); }
  Bool isScheduled (Thread *thr) {
    return (queues[thr->getPriority ()].isScheduled (thr));
  }
};

#ifndef OUTLINE
#include "thrspool.icc"
#endif

#endif
