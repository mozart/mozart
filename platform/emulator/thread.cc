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
#include "suspension.hh"
#include "taskstack.hh"
#include "thread.hh"

// mm2 --> config.m4
#define DEFAULT_TIME_SLICE 50 // msec
#define DEFAULT_USER_PRIORITY 0
#define DEFAULT_SYSTEM_PRIORITY 100


// --------------------------------------------------------------------------


/* enum ThreadFlags:
   Normal: thread has a taskStack and is scheduled
   Warm: thread has no taskStack, but 'suspension' contains a suspension
   Nervous: thread has no taskStack, but 'board' contains the board to visit
   OneTask: thread has only one Task
   */

enum ThreadFlags
{
  T_Normal,
  T_Warm,
  T_Nervous
};


/* class Thread
   static variables
     Head: pointer to the head of the thread queue
     Tail: pointer to the tail of the thread queue
     Current: pointer to the current thread
     Root: pointer to the root thread
     TimeSlice: the overall time slice of a thread in msec
     UserPriority: the user priority
     SystemPriority: the system priority
   member data
     next: the next thread in the thread queue
     prev: the prev thread in the thread queue
     priority: the thread priority
               -MININT ... +MAXINT
	       low     ... high priority
     flags: see above ThreadFlags
     taskStack: the stack of elaboration tasks
     suspension: a suspension scheduled for execution
     board: a board scheduled for visiting
       Note: taskStack,suspension and board are shared
     */


// head and tail of the thread queue
Thread *Thread::Head;
Thread *Thread::Tail;

// the currently running thread
Thread *Thread::Current;

Thread *Thread::Root;

// the time slice for threads in ms
int Thread::TimeSlice;

// the user and system priorities
int Thread::UserPriority;
int Thread::SystemPriority;

void Thread::Init()
{
  TimeSlice = DEFAULT_TIME_SLICE;
  UserPriority = DEFAULT_USER_PRIORITY;
  SystemPriority = DEFAULT_SYSTEM_PRIORITY;

  Head = (Thread *) NULL;
  Tail = (Thread *) NULL;
  Current = (Thread *) NULL;
  Root = new Thread;
  Root->flags = T_Normal;
  Root->priority = SystemPriority;
  Root->u.taskStack = new TaskStack();
}

Thread *Thread::GetCurrent()
{
  return Current;
}
Thread *Thread::GetHead()
{
  return Head;
}
Thread *Thread::GetRoot()
{
  return Root;
}
Thread *Thread::GetTail()
{
  return Tail;
}
int Thread::GetUserPriority()
{
  return UserPriority;
}
int Thread::GetSystemPriority()
{
  return SystemPriority;
}
int Thread::GetTimeSlice()
{
  return TimeSlice;
}

// check if the thread is scheduled
Bool Thread::isScheduled() {
  return (prev!=NULL || next!=NULL || Head==this) ? OK : NO;
}


void Thread::Schedule(Suspension *s)
{
  Thread *t=new Thread;
  t->flags = T_Warm;
  t->priority = s->getPriority();
  t->u.suspension = s;
  s->getNode()->addSuspension();
  t->schedule();
}

// create a new thread on root
void Thread::ScheduleRoot(ProgramCounter PC,RefsArray y)
{
  Board *bb=Board::GetRoot();
  bb->addSuspension();
  Root->u.taskStack->queueCont(bb,PC,y);
  if (Root!=Current && !Root->isScheduled()) {
    Root->schedule();
  }
}

// create a new thread after wakeup (nervous)
void Thread::Schedule(Board *b)
{
  Thread *t = new Thread;
  t->flags = T_Nervous;
  t->priority = b->getActor()->getPriority();
  t->u.board = b;
  b->setNervous();
  t->schedule();
}

Thread::Thread(int prio)
: ConstTerm(Co_Thread)
{
  Thread(*this);
  flags = T_Normal;
  priority = prio;
  u.taskStack = new TaskStack;
}
// create a new thread without any value
Thread::Thread()
: ConstTerm(Co_Thread)
{
  prev=next= (Thread *) NULL;
  DebugCheckT(priority = -1; u.taskStack = (TaskStack *) -1; flags = -1);
}


Bool Thread::isNormal()
{
  return flags == T_Normal ? OK : NO;
}

Bool Thread::isWarm()
{
  return flags == T_Warm ? OK : NO;
}

Bool Thread::isNervous()
{
  return flags == T_Nervous ? OK : NO;
}

// free the memory of the thread (there are no references to it anymore)
void Thread::dispose() {
  if (this != Root) {
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
  DebugCheck(priority < 0,error("Thread::getPriority"));
  return priority;
}

// add a thread to the thread queue
void Thread::schedule() {
  DebugCheck(prev!=0 || next!=0,error("Thread::schedule"));

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

  if (Current->isNormal()) {
    am.currentTaskStack = Current->u.taskStack;
  } else {
    am.currentTaskStack = (TaskStack *) NULL;
  }

  Alarm::RestartProcess();
}

void Thread::MakeTaskStack()
{
  DebugCheck(!Current->isNormal() || Current->u.taskStack!=NULL,
	     error("Thread::pushTask"));
  am.currentTaskStack = Current->u.taskStack = new TaskStack();

}

void Thread::pushTask(Board *bb,ProgramCounter pc,
		      RefsArray y,RefsArray g,
		      RefsArray x,int i)
{
  DebugCheck(!isNormal(),error("Thread::pushTask"));
  bb->addSuspension();
  u.taskStack->pushCont(bb,pc,y,g,x,i);
}

void Thread::ScheduleCurrent()
{
  Current->schedule();
  Current=(Thread *) NULL;
}

void Thread::FinishCurrent()
{
  Current->dispose();
  Current=(Thread *) NULL;
}


int Thread::GetCurrentPriority() {
  return Current->priority;
}

Board *Thread::popBoard()
{
  DebugCheck(!isNervous(),error("Thread::popBoard"));
  Board *ret = u.board;
  u.board = (Board *) NULL;
  flags = T_Normal;
  return ret;
}

Suspension *Thread::popSuspension()
{
  DebugCheck(!isWarm(),error("Thread::popSuspension"));
  Suspension *ret = u.suspension;
  u.suspension = (Suspension *) NULL;
  flags = T_Normal;
  return ret;
}

void Thread::NewCurrent(int prio) {
  Current = new Thread();
  Current->flags = T_Normal;
  Current->priority = prio;
  am.currentTaskStack = Current->u.taskStack = new TaskStack();
}
