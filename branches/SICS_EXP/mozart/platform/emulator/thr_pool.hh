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

class ThreadsPool {
private:
  ThreadQueue hiQueue;
  ThreadQueue midQueue;
  ThreadQueue lowQueue;

  int hiCounter;
  int lowCounter;

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

  void scheduleThread(Thread *th,int pri=-1);
  void scheduleThreadInline(Thread *th,int pri)
  {
#ifdef DEBUG_CHECK
    scheduleThread(th,pri);
#else
    if (pri == MID_PRIORITY) {
      midQueue.enqueue(th);
    } else {
      scheduleThread(th,pri);
    }
#endif
  }

  void rescheduleThread(Thread *th);
  void deleteThread(Thread *th);

  Bool isScheduledSlow(Thread *thr);

  Board * getHighestSolveDebug(void); // TMUELLER

  Thread *getFirstThreadOutline();
  Thread *getFirstThread()
  {
    if (hiCounter < 0 && lowCounter < 0) {
      Assert(hiQueue.isEmpty() && lowQueue.isEmpty());
      return midQueue.dequeue();
    }

    return getFirstThreadOutline();
  }

  Bool threadQueuesAreEmptyOutline();
  Bool threadQueuesAreEmpty()
  {
    if (hiCounter < 0 && lowCounter < 0) {
      Assert(hiQueue.isEmpty() && lowQueue.isEmpty());
      return midQueue.isEmpty();
    }
    return threadQueuesAreEmptyOutline();
  }
  int getRunnableNumber();
};

#ifndef OUTLINE
#include "thrspool.icc"
#endif

#endif
