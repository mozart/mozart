/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of threads and queues of threads with priorities
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

#include "tagged.hh"
#include "constter.hh"
#include "actor.hh"
#include "board.hh"
#include "stack.hh"
#include "taskstk.hh"
#include "thread.hh"

#include "suspensi.hh"

#ifdef OUTLINE
#define inline
#include "thread.icc"
#undef inline
#endif

// --------------------------------------------------------------------------


/* class Thread
   static variables
     Head: pointer to the head of the thread queue
     Tail: pointer to the tail of the thread queue
     am.currentThread: pointer to the current thread
     am.rootThread: pointer to the root thread
   member data
     priority: the thread priority
               -MININT ... +MAXINT
	       low     ... high priority
     taskStack: the stack of elaboration tasks
     */

/*
 * check if a thread's board is below a failed board
 */
Bool Thread::isBelowFailed(Board *top)
{
  Board *bb=getBoardFast();
  while (bb!=top) {
    if (bb->isFailed()) {
      return TRUE;
    }
    bb=bb->getParentFast();
  }
  return FALSE;
}

/*
 * remove local tasks
 * return OK, if done
 * return NO, if no C_LOCAL/C_SOLVE found
 */
Bool Thread::discardLocalTasks()
{
  TaskStackEntry *tos = TaskStack::getTop();
  while (TRUE) {
    TaskStackEntry entry=*(--tos);
    if (TaskStack::isEmpty(entry)) {
      TaskStack::setTop(tos+1);
      return NO;
    }

    ContFlag cFlag = getContFlag(ToInt32(entry));

    switch (cFlag){
    case C_LOCAL:
    case C_SOLVE:
      TaskStack::setTop(tos);
      return OK;
    default:
      tos = tos - TaskStack::frameSize(cFlag) + 1;
      break;
    }
  }
}

