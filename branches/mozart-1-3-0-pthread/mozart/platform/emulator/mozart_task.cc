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

static pthread_mutex_t mutex;

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

static sigset_t empty_mask;

static OzTaskQueue* queue_tiny;
static OzTaskQueue* queue_normal;

static int initialized = 0;
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

static void* task_execute(void*x)
{
  ((OzTaskQueue*)x)->work();
  return NULL;
}

void OzTaskQueue::enq(OzTask* t)
{
  pthread_mutex_lock(&mutex);
  t->_next=0;
  if (queue_last==0) {
    queue_first=queue_last=t;
  } else {
    queue_last->_next = t;
    queue_last = t;
  }
  if (idle==0 && total<total_max) {
    sigset_t old_mask;
    pthread_sigmask(SIG_SETMASK,&empty_mask,&old_mask);
    pthread_t id;
    pthread_create(&id,&thread_attr,task_execute,(void*)this);
    pthread_sigmask(SIG_SETMASK,&old_mask,NULL);
  }
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

static OzTask* finish_first = 0;
static OzTask* finish_last  = 0;

void OzTaskQueue::work(void)
{
  pthread_mutex_lock(&mutex);
  int redo = 1;
  struct timespec timeout;

  goto inner_loop;

 outer_loop:
  timeout.tv_sec = time(NULL) + idle_sec;
  timeout.tv_nsec = 0;
  // atomically: release lock, wait for condition, acquire lock
  if (pthread_cond_timedwait(&cond,&mutex,&timeout)==ETIMEOUT) redo=0;

 inner_loop:
  // check for work to be done
  while (queue_first) {
    redo=1;
    OzTask* t = queue_first;
    queue_first = t->_next;
    t->_next = 0;
    if (!queue_first) queue_last=0;
    idle--;
    pthread_mutex_unlock(&mutex);
    t->execute();
    pthread_mutex_lock(&mutex);
    idle++;
    if (finish_last) {
      finish_last->_next = t;
    } else {
      finish_first=finish_last=t;
    }
  }

  if (redo) goto outer_loop;

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
    pthread_mutex_unlock(&mutex);
    while (q) {
      t = q;
      q = t->_next;
      t->_next = 0;
      t->finish();
      delete t;
    }
  } else pthread_mutex_unlock(&mutex);
}
