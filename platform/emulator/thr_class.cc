/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
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

// implementation of threads

#if defined(INTERFACE)
#pragma implementation "thread.hh"
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

#include "thread.hh"

int Thread::getRunnableNumber()
{
  switch (getThrType()) {
  case S_RTHREAD:
    {
      TaskStack * taskstack = getTaskStackRef();
      if (taskstack->isEmpty()) return 0;
      Frame *tos = taskstack->getTop();
      GetFrame(tos,PC,Y,G);
      if (PC!=C_LTQ_Ptr)
        return 1;

#ifdef COUNT_PROPAGATORS
      SolveActor *sa = (SolveActor *) Y;
      ThreadQueueImpl *ltq = sa->getLocalThreadQueue();
      return ltq->getSize();
#else
      return 0;
#endif
    }
  case S_WAKEUP:
    return 0;
  case S_PR_THR:
#ifdef COUNT_PROPAGATORS
    return 1;
#else
    return 0;
#endif
  default:
    Assert(0);
    return 0;
  }
}
