/*
 *  Authors:
 *    Author's name (Author's email address)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

protected:
  //  formerly in AM;
  //
  Thread *_currentThread;
  Thread *_rootThread;
  RunnableThreadBody *threadBodyFreeList;
public:
  ThreadsPool () {};
  ~ThreadsPool () {};

  Thread *currentThread()           { return _currentThread; }
  void unsetCurrentThread()         { _currentThread=0; }
  void setCurrentThread(Thread *th) { _currentThread=th; }
  Thread *rootThread()              { return _rootThread; }

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

#endif
