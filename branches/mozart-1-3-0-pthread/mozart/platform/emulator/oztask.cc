#include "mozart.h"
#include "oztask.hh"
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

static int initialized = 0;

class OzTaskQueue {
private:
  OzTask* head;
  OzTask* tail;
  int count;
public:
  OzTaskQueue():head(0),tail(0),count(0){}
  void enq(OzTask*t) {
    t->_next = 0;
    if (tail) { tail->_next=t; tail=t; }
    else { head=tail=t; }
    count += 1;
  }
  OzTask* deq(void) {
    OzTask* t;
    if ((t=head)) { head=t->_next; count-=1; }
    return t;
  }
  int getSize() { return count; }
};

static OzTaskQueue queue_entry;
static OzTaskQueue queue_exit;
static void task_init(void);
static pthread_mutex_t mutex;
static pthread_cond_t cond;
static int threads_idle = 0;

static inline void lock(void) { pthread_mutex_lock(&mutex); }
static inline void unlock(void) { pthread_mutex_unlock(&mutex); }

static inline void thread_spawn(void)
{
  sigset_t old_mask;
  pthread_sigmask(SIG_SETMASK,&empty_mask,&old_mask);
  pthread_t id;
  pthread_create(&id,&thread_attr,thread_worker,NULL);
  pthread_sigmask(SIG_SETMASK,&old_mask,NULL);
}

void
OZ_taskEnq(OzTask* t) {
  if (!initialized) task_init();
  lock();
  queue_entry.enq(t);
  if (threads_idle) pthread_cond_signal(&cond);
  else thread_spawn();
  unlock();
}

static inline void thread_timedwait(void)
{
  struct timespec timeout;
  timeout.tv_sec = time(NULL) + 10;
  timeout.tv_nsec = 0;
  pthread_cond_timedwait(&cond,&mutex,&timeout);
}

void thread_worker(void*)
{
  lock();
  OzTask* t;
  int redo = 1;

 loop:

  while ((t=queue_entry.deq())) {
    redo = 1;
    unlock();
    t->execute();
    lock();
    queue_exit.enq(t);
  }

  if (redo) {
    threads_idle += 1;
    thread_timedwait();
    threads_idle -= 1;
    redo = 0;
    goto loop;
  }

  unlock();
}
