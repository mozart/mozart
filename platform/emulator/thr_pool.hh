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

#ifdef INTERFACE
#pragma interface
#endif

#include "thrqueue.hh"

// == 1 if local propagation integrated as OZMAX_PRIORITY+1 queue
// (forthcoming???)
#define ADDITIONAL_PRIOS  0

// just the size of a stack and lookup table;
#define TAB_SIZE   (OZMAX_PRIORITY-OZMIN_PRIORITY+1+ADDITIONAL_PRIOS)

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
  //  Invariant: it can range between OZMIN_PRIORITY and OZMAX_PRIORITY,
  // complains with the 'currentQueue', and is equal to '-1' when
  // there is no runnable thread;
  int currentPriority;

protected:
  //
public:
  //  formerly in AM;
  Thread *currentThread;
  Thread *rootThread;
  //
  //  used by thread constructors;
  RunnableThreadBody *threadBodyFreeList;

  ThreadsPool () {};
  ~ThreadsPool () {};
  void initThreads();

  // in print.cc;
  void printThreads ();

  //
  void doGC ();

  void scheduleThread (Thread *th, int pri) {
    if (pri == currentPriority) {
      currentQueue->enqueue (th);
    } else {
      scheduleThreadOutline (th, pri);
    }
  }
  void updateCurrentQueue();
  void scheduleThread (Thread *th);
  void rescheduleThread(Thread *th);
  void deleteThread(Thread *th);
  void scheduleThreadOutline (Thread *th, int pri);

  Bool threadQueueIsEmpty ();
  Thread *getFirstThread ();

  //  misc;
  int getNextThPri () { return (currentPriority); }
  Bool isScheduled (Thread *thr);

  Board * getHighestSolveDebug(void); // TMUELLER
};

#ifndef OUTLINE
#include "thrspool.icc"
#endif

#endif
