/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of threads
  ------------------------------------------------------------------------
*/

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
