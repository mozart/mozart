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
#include "susp_queue.hh"
#include "thr_class.hh"

class ThreadsPool {
private:
  SuspQueue _q[HI_PRIORITY+1];

  int hi, mid;

public:
  ThreadsPool(void) {};
  ~ThreadsPool(void) {};

  void init(void);

  void gCollect(void);

  void scheduleThread(Thread *th) {
    Assert(!isScheduledSlow(th));
    _q[th->getPriority()].enqueue(th);
  }

  void rescheduleThread(Thread *th);
  void deleteThread(Thread *th);

  Bool isScheduledSlow(Thread *thr);

  int getRunnableNumber();

  Bool isEmpty() {
    return (_q[MID_PRIORITY].isEmpty() &&
	    _q[ HI_PRIORITY].isEmpty() &&
	    _q[LOW_PRIORITY].isEmpty());
  }

  Thread * getNext() {
    
    do {
      if (!_q[HI_PRIORITY].isEmpty() && hi > 0) {
	hi--; 
	return SuspToThread(_q[HI_PRIORITY].dequeue());
      }
      
      hi = ozconf.hiMidRatio;

      if (!_q[MID_PRIORITY].isEmpty() && mid > 0) {
	mid--; 
	return SuspToThread(_q[MID_PRIORITY].dequeue());
      }
      
      mid = ozconf.midLowRatio;
      
      if (!_q[LOW_PRIORITY].isEmpty())
	return SuspToThread(_q[LOW_PRIORITY].dequeue());
      
    } while (!_q[MID_PRIORITY].isEmpty() || 
	     !_q[ HI_PRIORITY].isEmpty());
    
    return (Thread *) NULL;
  }

  // in print.cc; 
  void printThreads ();


};

#endif
