/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

// implementation of threads

#if defined(INTERFACE)
#pragma implementation "thr_class.hh"
#endif

/* some random comments on threads:

   threads are 'switch'ed, if
     - 'timed out' by alarm clock
     - 'finish'ed
     - 'displace'd by higher priority
     - 'freeze'd

   need for thread switching is signaled by 'ThreadSwitch' S-Flag

   "current thread"
     - the currently running thread

   */

#include "thr_class.hh"

int Thread::getRunnableNumber()
{
  switch (getThrType()) {
  case S_RTHREAD:
    {
      TaskStack * taskstack = getTaskStackRef();
      if (taskstack->isEmpty()) return 0;
      Frame *tos = taskstack->getTop();
      GetFrame(tos,PC,Y,G);
      if (PC!=C_LPQ_Ptr)
        return 1;

#ifdef COUNT_PROPAGATORS
      SolveActor *sa = (SolveActor *) Y;
      LocalPropagatorQueue * lpq = sa->getLocalPropagatorQueue();
      return lpq->getSize();
#else
      return 0;
#endif
    }
  case S_WAKEUP:
    return 0;
  default:
    Assert(0);
    return 0;
  }
}
