/*
 *  Authors:
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

#include "base.hh"
#include "thr_queue.hh"
#include "thr_class.hh"

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
  RunnableThreadBody *threadBodyFreeList;
public:
  ThreadsPool () {};
  ~ThreadsPool () {};

  Thread *currentThread()           { 
    return _currentThread; 
  }
  void unsetCurrentThread()         { 
    _currentThread=0; 
  } 
  void setCurrentThread(Thread *);

  void initThreads();

  // in print.cc; 
  void printThreads ();

  //
  void doGC ();

  void scheduleThread(Thread *th) {
    Assert(!isScheduledSlow(th));
    
    int pri = th->getPriority();

    if (pri == MID_PRIORITY) {
      midQueue.enqueue(th);
    } else if (pri == HI_PRIORITY) {
      hiQueue.enqueue(th);
      if (hiCounter<0) hiCounter=ozconf.hiMidRatio;
    } else {
      lowQueue.enqueue(th);
      if (lowCounter<0) lowCounter=ozconf.midLowRatio;
    }
  }

  void rescheduleThread(Thread *th);
  void deleteThread(Thread *th);

  Bool isScheduledSlow(Thread *thr);

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

  //
  //  An allocator for thread's bodies;
  RunnableThreadBody* allocateBody()
  {
    RunnableThreadBody *body = threadBodyFreeList;
    if (body) {
      threadBodyFreeList = threadBodyFreeList->next;
    } else {
      body = new RunnableThreadBody(ozconf.stackMinSize);
    }
    
    body->taskStack.init();

    return body;
  }

  void freeThreadBody(Thread *tt) {
    RunnableThreadBody *it = tt->getBody();
    it->next = threadBodyFreeList;
    threadBodyFreeList = it;
    tt->freeThreadBodyInternal();
  }

};

#endif
