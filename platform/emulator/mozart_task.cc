#include "mozart_cpi.hh"
#include "mozart_task.hh"
#include <signal.h>
#include <time.h>

// ===================================================================
// OzTaskQueue
//
// provides the method enq(OzTask*) to enqueue a task for execution by
// a system thread.  This class-based design makes it possible to have
// several queues with possibly different characteristics: we use this
// to provide a `tiny' queue as well as a `normal' queue.  The tiny
// threads have a very small stack, which, hopefully makes them more
// economical.
// ===================================================================

static int initialized = 0;
static pthread_mutex_t mutex;

static int number_of_tasks = 0;
static int number_of_tasks_finished = 0;

static sigset_t empty_mask;

class OzTaskQueue {
public:
  const int total_max = 100;	// at most these many threads may coexists
  const int idle_sec = 2;	// stays idle at most 2s
  int total;			// number of existing threads
  int idle;			// number of idle threads
  pthread_cond_t cond;
  pthread_attr_t attr;
  OzTask * queue_first;		// queue of pending tasks
  OzTask * queue_last;
  OzTaskQueue(void);
  void enq(OzTask*);
  void work(void);
  void finish(void);
};

// ===================================================================
// exported API
// ===================================================================

static OzTaskQueue* queue_tiny;
static OzTaskQueue* queue_normal;

void
OZ_taskEnqTiny(OzTask*t) {
  if (!initialized) task_queues_init();
  queue_tiny->enq(t);
}

void
OZ_taskEnq(OzTask*t) {
  if (!initialized) task_queues_init();
  queue_normal->enq(t);
}

// ===================================================================
// creation and initialization of task queues and related data
// ===================================================================

static void task_queues_init()
{
  initialized=1;
  sigemptyset(&empty_mask);
  pthread_mutex_init(&mutex,NULL);
  queue_tiny   = new OzTaskQueue;
  queue_normal = new OzTaskQueue;
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
#ifndef OZ_TINY_STACK
#define OZ_TINY_STACK PTHREAD_STACK_MIN*2
#endif
  pthread_attr_setstacksize(&queue_tiny->attr,OZ_TINY_STACK);
#endif
}

OzTaskQueue::OzTaskQueue(void)
  : total(0),
    idle(0),
    queue_first(0),
    queue_last(0)
{
  pthread_cond_init(&cond,NULL);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
}


// ===================================================================
// Enqueing a new task
// ===================================================================

static void* task_execute(void*x)
{
  ((OzTaskQueue*)x)->work();
  return NULL;
}

void OzTaskQueue::enq(OzTask* t)
{
  pthread_mutex_lock(&mutex);
  t->_next=0;
  t->_prev=queue_last;
  if (queue_last==0) {
    queue_first=queue_last=t;
  } else {
    queue_last->_next = t;
    queue_last = t;
  }
  number_of_tasks++;
  // maybe create a new pthread
  if (idle==0 && total<total_max) {
    // block all signals, the newly created thread will inherit
    // this mask.  We need this in order to ensure that all
    // signals are delivered to the main "emulator" thread.
    sigset_t old_mask;
    pthread_sigmask(SIG_SETMASK,&empty_mask,&old_mask);
    pthread_t id;
    pthread_create(&id,&thread_attr,task_execute,(void*)this);
    pthread_sigmask(SIG_SETMASK,&old_mask,NULL);
    // the new thread is created in the idle state
    idle++;
    total++;
  }
  // wake up one thread. either this actually wakes up an idle
  // thread, or there is at least one active thread and it will
  // check whether there is some task to be done before going
  // idle
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

// ===================================================================
// perform asynchronous step on a concurrent system thread
// ===================================================================

static OzTask* finish_first = 0;
static OzTask* finish_last  = 0;

void OzTaskQueue::work(void)
{
  // on entry, the thread is in the idle state

  int redo = 1;
  struct timespec timeout;

  pthread_mutex_lock(&mutex);

 loop:
  // check for work to be done
  while (queue_first) {
    redo=1;
    OzTask* t = queue_first;
    queue_first = t->_next;
    t->_next = 0;
    if (!queue_first) queue_last=0;
    idle--;
    pthread_mutex_unlock(&mutex);
    // perform asynchronous 1st step
    t->execute();
    pthread_mutex_lock(&mutex);
    idle++;
    // enqueue task for synchronous 2nd step
    if (finish_last) {
      finish_last->_next = t;
    } else {
      finish_first=finish_last=t;
    }
  }

  // queue is empty: maybe go idle
  if (redo) {
    timeout.tv_sec = time(NULL) + idle_sec;
    timeout.tv_nsec = 0;
    // atomically: release lock, wait for condition, acquire lock
    if (pthread_cond_timedwait(&cond,&mutex,&timeout)==ETIMEOUT) redo=0;
    goto loop;
  }

  // we get here only if (1) we timedout and (2) the queue is empty
  // in this case, let the thread die

  total--;
  idle--;
  pthread_mutex_unlock(&mutex);
}

// ===================================================================
// perform task's synchronous fast 2nd step
// ===================================================================

void OzTaskQueue::finish(void)
{
  pthread_mutex_lock(&mutex);
  if (finish_first) {
    OzTask* q = finish_first;
    OzTask* t;
    finish_first=finish_last=0;
    number_of_tasks -= number_of_tasks_finished;
    number_of_tasks_finished = 0;
    pthread_mutex_unlock(&mutex);
    while (q) {
      t = q;
      q = t->_next;
      t->finish();
      delete t;
    }
  } else pthread_mutex_unlock(&mutex);
}

// ===================================================================
// GC
// ===================================================================

inline void OzTask::gc_linked(OzTask* t)
{
  while (t) {
    OZ_gCollectTerm(t->_var);
    t->gc();
    t=t->_next;
  }
}

void gc_tasks(void)
{
  if (number_of_tasks) {
    // if there are tasks, then this module was initialized
    pthread_mutex_lock(&mutex);
    gc_linked_tasks(finish_first);
    gc_linked_tasks(queue_tiny->queue_first);
    gc_linked_tasks(queue_normal->queue_first);
    pthread_mutex_unlock(&mutex);
  }
}
