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
   Warm: thread has no taskStack, but 'suspension' contains a suspension
   Nervous: thread has no taskStack, but 'board' contains the board to visit
   Solve: thread was created inside a search problem;
   */

enum ThreadFlags
{
  T_Normal =    1<<0,
  T_Warm =      1<<1,
  T_Nervous =   1<<2,
  T_Solve =     1<<3
};

static int T_No_State = ~(T_Normal | T_Warm | T_Nervous);

/* class Thread
   static variables
     Head: pointer to the head of the thread queue
     Tail: pointer to the tail of the thread queue
     am.currentThread: pointer to the current thread
     am.rootThread: pointer to the root thread
     conf.timeSlice: the overall time slice of a thread in msec
     conf.defaultPriority: the user priority
     conf.systemPriority: the system priority
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


void Thread::ScheduleSuspension(Suspension *s)
{
  Thread *t=new Thread;
  t->flags = T_Warm;
  t->priority = s->getPriority();
  t->u.suspension = s;
  s->getNode()->addSuspension();
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
  DebugCheck(!isNormal() || !u.taskStack, error("Thread::queueCont"));
  bb->addSuspension();
  u.taskStack->queueCont(bb,PC,y);
  if (this!=am.currentThread && !this->isScheduled()) {
    schedule();
  }
}

// create a new thread after wakeup (nervous)
void Thread::ScheduleWakeup(Board *b)
{
  Thread *t = new Thread;
  t->flags = T_Nervous;
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
  u.taskStack = new TaskStack(conf.taskStackSize);
}

// initialize the thread member data
void Thread::init()
{
  prev=next= (Thread *) NULL;
#ifdef KP
  Threads_Total++;
#endif
  DebugCheckT(priority = -1; u.taskStack = (TaskStack *) -1; flags = -1);
}

Bool Thread::isNormal()
{
  return ((flags & T_Normal) ? OK : NO);
}

Bool Thread::isWarm()
{
  return ((flags & T_Warm) ? OK : NO);
}

Bool Thread::isNervous()
{
  return ((flags & T_Nervous) ? OK : NO);
}

Bool Thread::isSolve ()
{
  return ((flags & T_Solve) ? OK : NO);
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
    DebugCheck(prev!=0 || next!=0,error("Thread::dispose"));
    freeListDispose(this,sizeof(Thread));
  }
}


// unlink the thread from the thread queue
Thread *Thread::unlink() {
  if (prev) {
    DebugCheck(prev->next!=this,error("Thread::unlink"));
    prev->next=next;
  } else {
    DebugCheck(Head!=this,error("Thread::unlink"));
    Head=next;
  }
  if (next) {
    DebugCheck(next->prev!=this,error("Thread::unlink"));
    next->prev=prev;
  } else {
    DebugCheck(Tail!=this,error("Thread::unlink"));
    Tail=prev;
  }
  prev=next=(Thread *) NULL;
  return this;
}


int Thread::getPriority()
{
  DebugCheck(priority < 0 || priority > 100, error("Thread::getPriority"));
  return priority;
}

void Thread::setPriority(int prio)
{
  DebugCheck(prio < 0 || prio > 100, error("Thread::setPriority"));
  priority=prio;
}

// add a thread to the thread queue
void Thread::schedule() {
  DebugCheck(isScheduled(),error("Thread::schedule"));

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
      DebugCheck(next->prev!=here,error("Thread::insertAfter"));
      next->prev=this;
    } else {
      DebugCheck(Tail!=here,error("Thread::insertAfter"));
      Tail=this;
    }
  } else {
    next=Head;
    Head=this;
    if (next) {
      DebugCheck(next->prev!=0,error("Thread::insertAfter"));
      next->prev=this;
    } else {
      DebugCheck(Tail!=0,error("Thread::insertAfter"));
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
      DebugCheck(prev->next!=here,error("Thread::insertBefore"));
      prev->next=this;
    } else {
      DebugCheck(Tail!=here,error("Thread::insertBefore"));
      Head=this;
    }
  } else {
    prev=Tail;
    Tail=this;
    if (prev) {
      DebugCheck(prev->next!=0,error("Thread::insertBefore"));
      prev->next=this;
    } else {
      DebugCheck(Head!=0,error("Thread::insertBefore"));
      Head=this;
    }
  }
}


Bool Thread::QueueIsEmpty() {
  return Head ? NO : OK;
}


/* only usage in emulate */
Thread *Thread::GetFirst() {
  DebugCheck(Head==0,error("Thread::Start"));
  Thread *tt = Head;
  Head=tt->next;
  if (Head) {
    DebugCheck(Head->prev!=tt,error("Thread::Start"));
    Head->prev= (Thread *) NULL;
  } else {
    DebugCheck(Tail!=tt,error("Thread::Start"));
    Tail=Head;
  }
  tt->prev=tt->next=(Thread *) NULL;

  return tt;
}

TaskStack *Thread::makeTaskStack()
{
  DebugCheck(!isNormal() || u.taskStack!=NULL,
             error("Thread::makeTaskStack"));
  u.taskStack = new TaskStack(conf.taskStackSize);
  return u.taskStack;
}

/* only in emulate during process creation */
void Thread::pushTask(Board *bb,ProgramCounter pc,
                      RefsArray y,RefsArray g,
                      RefsArray x,int i)
{
  DebugCheck(!isNormal(),error("Thread::pushTask"));
  bb->addSuspension();
  u.taskStack->pushCont(bb,pc,y,g,x,i);
}

void Thread::pushTask (Board *bb, BIFun f, RefsArray x, int i)
{
  DebugCheck (!isNormal(), error ("Thread::pushTask"));
  bb->addSuspension ();
  u.taskStack->pushCont (bb, f, (Suspension *) NULL, x, i);
}

TaskStack *Thread::getTaskStack() {
  DebugCheck(!isNormal(),error("Thread::getTaskStack"));
  return u.taskStack;
}

Board *Thread::popBoard()
{
  DebugCheck(!isNervous(),error("Thread::popBoard"));
  Board *ret = u.board;
  u.board = (Board *) NULL;
  flags = (T_Normal | (flags & T_No_State));
  return ret;
}

Suspension *Thread::popSuspension()
{
  DebugCheck(!isWarm(),error("Thread::popSuspension"));
  Suspension *ret = u.suspension;
  u.suspension = (Suspension *) NULL;
  flags = (T_Normal | (flags & T_No_State));
  return ret;
}

#ifdef OUTLINE
#define inline
#include "thread.icc"
#undef inline
#endif
