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

#ifdef __GNUC__
#pragma implementation "thread.hh"
#endif

/* some random comments on threads:

   threads are 'switch'ed, if
     - 'timed out' by alarm clock
     - 'finish'ed
     - 'displace'd by higher priority
     - 'freeze'd

   need for thread switching is signaled by 'ThreadSwitch' S-Flag

   "thread queue"
     - has HEAD and TAIL
     - HEAD has always the highest priority and is the next to run

   "current thread"
     - the currently running thread

   "current taskstack"
     - optimized into a register -> am.currentTaskStack
     - is only visible variable to the outside ?


   */

#include "types.hh"

#include "actor.hh"
#include "alarm.hh"
#include "am.hh"
#include "io.hh"
#include "suspension.hh"
#include "taskstack.hh"
#include "thread.hh"

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
     am.conf.timeSlice: the overall time slice of a thread in msec
     am.conf.defaultPriority: the user priority
     am.conf.systemPriority: the system priority
   member data
     next: the next thread in the thread queue
     prev: the prev thread in the thread queue
     priority: the thread priority
               -MININT ... +MAXINT
               low     ... high priority
     flags: see ThreadFlags
     u.taskStack: the stack of elaboration tasks
     u.suspension: a suspension scheduled for execution
     u.board: a board scheduled for visiting
       Note: taskStack,suspension and board are shared (union)
     */


// head and tail of the thread queue
Thread *Thread::Head;
Thread *Thread::Tail;

void Thread::Init()
{
  Head = (Thread *) NULL;
  Tail = (Thread *) NULL;
  am.currentThread = (Thread *) NULL;
  am.rootThread = new Thread(am.conf.defaultPriority);
  am.currentTaskStack = NULL;
}

/* for gdb debugging: cannot access static member data */
Thread *Thread::GetHead()
{
  return Head;
}

/* for gdb debugging: cannot access static member data */
Thread *Thread::GetTail()
{
  return Tail;
}


void Thread::ScheduleSuspCont(SuspContinuation *c, Bool wasExtSusp)
{
  Thread *t=new Thread;
  t->flags = T_SuspCont;
  if (am.currentSolveBoard != (Board *) NULL || wasExtSusp == OK) {
    Board *nb = c->getNode ();
    am.incSolveThreads (nb);
    t->setNotificationBoard (nb);
  }
  t->priority = c->getPriority();
  t->u.suspCont = c;
  t->schedule();
}

void Thread::ScheduleSuspCCont(CFuncContinuation *c, Bool wasExtSusp,
                               Suspension *s)
{
  Thread *t=new Thread;
  t->resSusp = s;
  t->flags = T_SuspCCont;
  if (am.currentSolveBoard != (Board *) NULL || wasExtSusp == OK) {
    Board *nb = c->getNode ();
    am.incSolveThreads (nb);
    t->setNotificationBoard (nb);
  }
  t->priority = c->getPriority();
  t->u.suspCCont = c;
  t->schedule();
}


/* Thread::queueCont
     insert a top level continuation at the bottom of the rootThread
     NOTE: the compiler depends on the order of toplevel queries
     */
void Thread::queueCont(Board *bb,ProgramCounter PC,RefsArray y) {
  Assert(isNormal() && u.taskStack);
  bb->incSuspCount();
  u.taskStack->queueCont(bb,PC,y);
  if (this!=am.currentThread && !this->isScheduled()) {
    schedule();
  }
}

// create a new thread after wakeup (nervous)
void Thread::ScheduleWakeup(Board *b, Bool wasExtSusp)
{
  Thread *t = new Thread;
  t->flags = T_Nervous;
  if (am.currentSolveBoard != (Board *) NULL || wasExtSusp == OK) {
    am.incSolveThreads (b);
    t->setNotificationBoard (b);
  }
  t->priority = b->getActor()->getPriority();
  t->u.board = b;
  b->setNervous();
  t->schedule();
}

// create a new thread to reduce a solve actor;
void Thread::ScheduleSolve (Board *b)
{
  DebugCheck ((b->isCommitted () == OK || b->isSolve () == NO),
              error ("no solve board in Thread::ScheduleSolve ()"));
  // DebugCheckT (message("Thread::ScheduleSolve (@0x%x)\n", (void *) b->getActor ()));
  Thread *t = new Thread;
  t->flags = T_Nervous;
  Board *nb = (b->getParentBoard ())->getSolveBoard ();
  am.incSolveThreads (nb);
  t->setNotificationBoard (nb);
  t->priority = b->getActor()->getPriority();
  t->u.board = b;
  b->setNervous();
  t->schedule();
}


TaskStack *Thread::makeTaskStack()
{
  Assert(isNormal() && !u.taskStack);
  u.taskStack = new TaskStack(am.conf.taskStackSize);
  return u.taskStack;
}
