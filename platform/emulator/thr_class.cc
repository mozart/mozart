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

// --------------------------------------------------------------------------


/* enum ThreadFlags:
   Normal: thread has a taskStack and is scheduled
   SuspCont: thread has no taskStack,
             but 'suspCont' contains a SuspContinuation
   SuspCCont: thread has no taskStack,
             but 'suspCCont' contains a CFuncContinuation
   Nervous: thread has no taskStack, but 'board' contains the board to visit
   */

enum ThreadFlags
{
  T_Normal =      0x01,
  T_SuspCont =    0x02,
  T_SuspCCont =   0x04,
  T_Nervous =     0x08
};

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
  am.rootThread = new Thread(am.conf.systemPriority);
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


// check if the thread is scheduled
Bool Thread::isScheduled() {
  return (prev!=NULL || next!=NULL || Head==this) ? OK : NO;
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

// create a thread with a taskstack
Thread::Thread(int prio)
: ConstTerm(Co_Thread)
{
  init();
  flags = T_Normal;
  priority = prio;
  resSusp = NULL;
  u.taskStack = new TaskStack(am.conf.taskStackSize);
}

// initialize the thread member data
void Thread::init()
{
  prev=next= (Thread *) NULL;
  notificationBoard = (Board *) NULL;
  DebugCheckT(priority = -1; u.taskStack = (TaskStack *) -1; flags = -1);
}

Bool Thread::isNormal()
{
  return ((flags == T_Normal) ? OK : NO);
}

Bool Thread::isSuspCont()
{
  return ((flags == T_SuspCont) ? OK : NO);
}

Bool Thread::isSuspCCont()
{
  return ((flags == T_SuspCCont) ? OK : NO);
}

Bool Thread::isNervous()
{
  return ((flags == T_Nervous) ? OK : NO);
}

// mm2: not yet enabled
// free the memory of the thread (there are no references to it anymore)
void Thread::dispose() {
  if (this != am.rootThread) { // mm2
    if (isNormal()) {
      if (u.taskStack) {
        u.taskStack->dispose();
      }
    }
    Assert(prev==0 && next==0);
    DebugCheckT (flags = 0xff000000|flags);
    freeListDispose(this,sizeof(Thread));
  }
}


// unlink the thread from the thread queue
Thread *Thread::unlink() {
  if (prev) {
    Assert(prev->next==this);
    prev->next=next;
  } else {
    Assert(Head==this);
    Head=next;
  }
  if (next) {
    Assert(next->prev==this);
    next->prev=prev;
  } else {
    Assert(Tail==this);
    Tail=prev;
  }
  prev=next=(Thread *) NULL;
  return this;
}


int Thread::getPriority()
{
  Assert(priority >= 0 && priority <= 100);
  return priority;
}

Suspension *Thread::getResSusp()
{
  return resSusp;
}

void Thread::setPriority(int prio)
{
  Assert(prio >= 0 && prio <= 100);
  priority=prio;
}

// add a thread to the thread queue
void Thread::schedule() {
  Assert(!isScheduled());

  if (this != am.currentThread
      && am.currentThread
      && priority > am.currentThread->getPriority()) {
    insertAfter(0);
    am.setSFlag(ThreadSwitch);
  } else {
    insertFromTail(); // or insertFromHead();
  }
}

// insert into priority queue searching from tail
//  after the last one with the same or a higher priority
void Thread::insertFromTail()
{
  for (Thread *here = Tail; here; here=here->prev) {
    if (here->priority >= priority) {
      break;
    }
  }
  insertAfter(here);
}

// insert into priority queue searching from head
//  before the first one with the same or a lower priority
void Thread::insertFromHead() {
  for (Thread *here = Head; here; here=here->prev) {
    if (here->priority <= priority) {
      break;
    }
  }
  insertBefore(here);
}

// insert 'this' after 'here', if here = 0 then at head
void Thread::insertAfter(Thread *here) {
  prev=here;
  if (here) {
    next=here->next;
    here->next=this;
    if (next) {
      Assert(next->prev==here);
      next->prev=this;
    } else {
      Assert(Tail==here);
      Tail=this;
    }
  } else {
    next=Head;
    Head=this;
    if (next) {
      Assert(next->prev==0);
      next->prev=this;
    } else {
      Assert(Tail==0);
      Tail=this;
    }
  }
}

// insert 'this' before 'here', if here = 0 then at tail
void Thread::insertBefore(Thread *here) {
  next=here;
  if (here) {
    prev=here->prev;
    here->prev=this;
    if (prev) {
      Assert(prev->next==here);
      prev->next=this;
    } else {
      Assert(Tail==here);
      Head=this;
    }
  } else {
    prev=Tail;
    Tail=this;
    if (prev) {
      Assert(prev->next==0);
      prev->next=this;
    } else {
      Assert(Head==0);
      Head=this;
    }
  }
}


Bool Thread::QueueIsEmpty() {
  return Head ? NO : OK;
}


/* only usage in emulate */
Thread *Thread::GetFirst() {
  Assert(Head!=0);
  Thread *tt = Head;
  Head=tt->next;
  if (Head) {
    Assert(Head->prev==tt);
    Head->prev= (Thread *) NULL;
  } else {
    Assert(Tail==tt);
    Tail=Head;
  }
  tt->prev=tt->next=(Thread *) NULL;

  return tt;
}

TaskStack *Thread::makeTaskStack()
{
  Assert(isNormal() && !u.taskStack!=NULL);
  u.taskStack = new TaskStack(am.conf.taskStackSize);
  return u.taskStack;
}

/* only in emulate during process creation */
void Thread::pushTask(Board *bb,ProgramCounter pc,
                      RefsArray y,RefsArray g,
                      RefsArray x,int i)
{
  Assert(isNormal());
  bb->incSuspCount();
  u.taskStack->pushCont(bb,pc,y,g,x,i);
}

void Thread::pushTask (Board *bb, OZ_CFun f, RefsArray x, int i)
{
  Assert(isNormal());
  bb->incSuspCount ();
  u.taskStack->pushCont (bb, f, (Suspension *) NULL, x, i);
}

TaskStack *Thread::getTaskStack() {
  Assert(isNormal());
  return u.taskStack;
}

Board *Thread::popBoard()
{
  Assert(isNervous());
  Board *ret = u.board;
  flags = T_Normal;
  u.taskStack = (TaskStack *) NULL;
  return ret;
}

SuspContinuation *Thread::popSuspCont()
{
  Assert(isSuspCont());
  SuspContinuation *ret = u.suspCont;
  flags = T_Normal;
  u.taskStack = (TaskStack *) NULL;
  return ret;
}

CFuncContinuation *Thread::popSuspCCont()
{
  Assert(isSuspCCont());
  CFuncContinuation *ret = u.suspCCont;
  flags = T_Normal;
  u.taskStack = (TaskStack *) NULL;
  return ret;
}


#ifdef OUTLINE
#define inline
#include "thread.icc"
#undef inline
#endif
