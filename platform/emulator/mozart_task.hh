#ifndef _MOZART_TASK__HH_
#define _MOZART_TASK__HH_

#include "mozart.h"
#include <pthread.h>

class OzTaskManager;

// An OzTask represents some work to be done that may block
// or require a long time to complete.  Since we don't want
// to block the emulator thread, such tasks must be delegated
// to some other threads.

class OzTask {
private:
  OzTask * _next;
public:
  OzTask():_next(0){}
  virtual ~OzTask();
  virtual void execute(void);
  virtual void finish(void);
  friend class OzTaskManager;
};

extern OzTaskManager* OZ_newTaskManager(int,int);
extern void OZ_taskManagerEnq(OzTaskManager*,OzTask*);

#endif
