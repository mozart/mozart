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
#include "suspension.hh"
#include "taskstack.hh"
#include "thread.hh"

// mm2 --> config.m4
#define DEFAULT_TIME_SLICE 50 // msec
#define DEFAULT_USER_PRIORITY 0
#define DEFAULT_SYSTEM_PRIORITY 100


// --------------------------------------------------------------------------


/* enum T_Flag:
   Normal: thread has a taskStack and is scheduled
   Warm: thread has no taskStack, but 'suspension' contains a suspension
   Nervous: thread has no taskStack, but 'board' contains the board to visit
   OneTask: thread has only one Task
   Freezed: thread is freezed and not in the thread queue
   */

/* class Thread
   static variables
     Head: pointer to the head of the thread queue
     Tail: pointer to the tail of the thread queue
     Current: pointer to the current thread
     TimeSlice: the overall time slice of a thread in msec
     UserPriority: the user priority
     SystemPriority: the system priority
   member data
     next: the next thread in the thread queue
     prev: the prev thread in the thread queue
     priority: the thread priority
               -MININT ... +MAXINT
               low     ... high priority
     flags: see above T_Flag
     taskStack: the stack of elaboration tasks
     suspension: a suspension scheduled for execution
     continuation: a continuation scheduled for execution
     board: a board scheduled for visiting
       Note: taskStack,suspension and board are shared
     */


// head and tail of the thread queue
Thread *Thread::Head = (Thread *) NULL;
Thread *Thread::Tail = (Thread *) NULL;

// the currently running thread
Thread *Thread::Current = (Thread *) NULL;

// the time slice for threads in ms
int Thread::TimeSlice = DEFAULT_TIME_SLICE;

// the user and system priorities
int Thread::UserPriority = DEFAULT_USER_PRIORITY;
int Thread::SystemPriority = DEFAULT_SYSTEM_PRIORITY;

Thread *Thread::GetCurrentDBG()
{
  return Current;
}
Thread *Thread::GetHeadDBG()
{
  return Head;
}
Thread *Thread::GetTailDBG()
{
  return Tail;
}

// check if the thread is scheduled
#ifdef DEBUG_CHECK
Bool Thread::isScheduled() {
  return (prev!=NULL || next!=NULL || Head==this) ? OK : NO;
}
#endif


void Thread::Schedule(Suspension *s)
{
  Thread *t=new Thread(T_Warm);
  t->priority = s->getPriority();
  t->suspension = s;
  t->schedule();
}

// create a new thread on root
void Thread::ScheduleRoot(Continuation *c)
{
  Thread *t=new Thread(T_RootTask);
  Board::GetRoot()->addSuspension();
  t->priority = UserPriority;
  t->continuation = c;
  t->schedule();
}

// create a new thread after wakeup (nervous)
void Thread::Schedule(Board *b)
{
  Thread *t=new Thread(T_Nervous);
  t->priority = b->getActor()->getPriority();
  t->board = b;
  t->schedule();
}

// create a new thread without any value
Thread::Thread(int f)
: flags(f) , ConstTerm(Co_Thread)
{
  prev=next= (Thread *) NULL;
  DebugCheckT(priority = -1; taskStack = (TaskStack *) -1);
}


#ifdef GC
// gc the static variables, e.g. queue
void Thread::GC() {
  Head=Head->gc();
  Tail=Tail->gc();
  Current=Current->gc();
}

// gc a Thread
Thread *Thread::gc() {
  if (!this) {
    return this;
  }

  INFROMSPACE(this);

  CHECKCOLLECTED(*(int *)this, Thread *);

  Thread *ret = (Thread *) gcRealloc(this, sizeof(Thread));

  setHeapCell((int *)this, GCMARK(ret));
  ptrStack.push(ret,PTR_NODE);
  // mm2 ....
}
#endif

// free the memory of the thread (there are no references to it anymore)
void Thread::dispose() {
  if (flags == T_Normal) {
    if (taskStack) {
      taskStack->dispose();
    }
  }
  DebugCheck(prev!=0 || next!=0,error("Thread::dispose"));
  freeListDispose(this,sizeof(Thread));
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


Bool Thread::isFreezed() { return (flags&T_Freezed) ? OK : NO; }

int Thread::getPriority()
{
  DebugCheck(priority < 0,error("Thread::getPriority"));
  return priority;
}
void Thread::setPriority(int i)
{
  priority = i;
}

TaskStack *Thread::getTaskStack()
{
  DebugCheck(flags!=T_Normal,error("Thread::getTaskStack"));
  return taskStack;
}

// add a thread to the thread queue
void Thread::schedule() {
  DebugCheck(prev!=0 || next!=0,error("Thread::schedule"));

  // n maybe currentProcessNode !!!
  if (isFreezed()) {
    return;
  }

  if (this != Current
      && Current
      && priority > Current->priority) {
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

// check if a thread switch is necessary
// mm2 inline for emulate.C
Bool Thread::CheckSwitch() {
  Bool ret;
  if (!am.isSetSFlag(ThreadSwitch)) {
    ret = NO;
  } else if (Current->isFreezed()) {
    ret = OK;
  } else if (QueueIsEmpty()
             ||
             Head->priority < Current->priority) {
    ret = NO;
    Alarm::RestartProcess();
  } else {
    ret = OK;
  }
  return ret;
}

Thread *Thread::UnlinkHead() {
  DebugCheck(Head==0,error("Thread::UnlinkHead"));
  Thread *ret = Head;
  Head=ret->next;
  if (Head) {
    DebugCheck(Head->prev!=ret,error("Thread::UnlinkHead"));
    Head->prev= (Thread *) NULL;
  } else {
    DebugCheck(Tail!=ret,error("Thread::UnlinkHead"));
    Tail=Head;
  }
  ret->prev=ret->next=(Thread *) NULL;
  return ret;
}

void Thread::Start() {
  if (QueueIsEmpty()) {
    am.suspendEngine();
  }

  Current=UnlinkHead();
  Current->activate();

  Alarm::RestartProcess();
}

/* activate the thread means: generate a taskstack if necessary
   and cache the taskStack
   */
void Thread::activate()
{
  if (flags==T_Normal) {
    am.currentTaskStack = taskStack;
  } else {
    am.currentTaskStack = (TaskStack *) NULL;
  }
}

void Thread::MakeTaskStack()
{
  DebugCheck(Current->flags!=T_Normal || Current->taskStack,
             error("Thread::pushTask"));
  am.currentTaskStack= Current->taskStack = new TaskStack();

}

void Thread::ScheduleCurrent()
{
  Current->schedule();
  Current=(Thread *) NULL;
}


Thread *Thread::GetCurrent()
{
  return Current;
}

void Thread::FinishCurrent()
{
  Current->dispose();
  Current=(Thread *) NULL;
}


int Thread::GetCurrentPriority() {
  return Current->priority;
}

TaskStack *Thread::GetCurrentTaskStack() {
  return Current->taskStack;
}


void Thread::NewCurrent(int prio) {
  Current = new Thread(T_Normal);
  Current->priority = prio;
  Current->taskStack = new TaskStack();

}
