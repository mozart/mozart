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

  int hi;
  int mid;

protected:
  RunnableThreadBody *threadBodyFreeList;

public:
  ThreadsPool() {};
  ~ThreadsPool() {};

  void initThreads();

  //
  void doGC ();

  void scheduleThread(Thread *th) {
    Assert(!isScheduledSlow(th));

    switch (th->getPriority()) {
    case MID_PRIORITY:
      midQueue.enqueue(th); break;
    case HI_PRIORITY:
      hiQueue.enqueue(th);  break;
    default:
      lowQueue.enqueue(th);
    }

  }

  void rescheduleThread(Thread *th);
  void deleteThread(Thread *th);

  Bool isScheduledSlow(Thread *thr);

  int getRunnableNumber();

  Bool isEmpty() {
    return (midQueue.isEmpty() &&
            hiQueue.isEmpty() &&
            lowQueue.isEmpty());
  }

  Thread * getNext() {

    Bool hiIsMt, midIsMt;

    do {

      hiIsMt = hiQueue.isEmpty();

      if (!hiIsMt && hi > 0) {
        hi--;
        return hiQueue.dequeue();
      }

      hi = ozconf.hiMidRatio;

      midIsMt = midQueue.isEmpty();

      if (!midIsMt && mid > 0) {
        mid--;
        return midQueue.dequeue();
      }

      mid = ozconf.midLowRatio;

      if (!lowQueue.isEmpty())
        return lowQueue.dequeue();

    } while (!hiIsMt || !midIsMt);

    return (Thread *) NULL;
  }

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


  // in print.cc;
  void printThreads ();


};

#endif
