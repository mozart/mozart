#include "mozart_task.hh"
#include <signal.h>
#include <time.h>

// an OzTaskManager

class OzTaskManager {
public:
  int total_max;		// at most these many threads may coexists
  int idle_max;			// at most these many threads may be idle
  int total;			// number of existing threads
  int idle;			// number of idle threads
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  OzTask * queue_first;		// queue of pending tasks
  OzTask * queue_last;
  OzTaskManager(int tmax,int imax,int itime);
  void enq(OzTask*);
  void work(void);
  void finish(void);
};

OzTaskManager::OzTaskManager(int tmax,int imax)
  : total_max(tmax),
    idle_max(imax),
    total(0),
    idle(0),
    queue_first(0),
    queue_last(0)
{
  pthread_mutex_init(&mutex,NULL);
  pthread_cond_init(&cond,NULL);
}

OzTaskManager*
OZ_newTaskManager(int tmax,int imax)
{
  new OzTaskManager(tmax,imax);
}

void
OZ_taskManagerEnq(OzTaskManager*m,OzTask*t)
{
  m->enq(t);
}

static sigset_t empty_mask;
static pthread_attr_t thread_attr;

class OzTaskDie : public OzTask {
public:
  void execute(void);
};

static void* OzTask_execute(void*x)
{
  ((OzTaskManager*)x)->work();
  return NULL;
}

void OzTaskManager::enq(OzTask* t)
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
    pthread_create(&id,&thread_attr,OzTask_execute,(void*)this)
      pthread_sigmask(SIG_SETMASK,&old_mask,NULL);
  }
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

//////////////////////////////////////////////////////////////////////

static OzTask* finish_first = 0;
static OzTask* finish_last  = 0;

void OzTaskManager::work(void)
{
  pthread_mutex_lock(&mutex);
  int redo = 1;
  struct timespec timeout;

  goto inner_loop;

 outer_loop:
  // we use 2 seconds timeout
  timeout.tv_sec = time(NULL) + 2;
  ntimeout.tv_nsec = 0;
  if (pthread_cond_timedwait(&cond,&mutex,&timeout)==ETIMEOUT) redo=0;

 inner_loop:
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

//////////////////////////////////////////////////////////////////////

void OzTaskManager::finish(void)
{
  pthread_mutex_lock(&mutex);
  OzTask* t;
  while (finish_first) {
    t = finish_first;
    finish_first = t->_next;
    t->_next = 0;
    t->finish();
  }
  finish_last = 0;
  pthread_mutex_unlock(&mutex);
}
