#ifndef _MOZART_TASK__HH_
#define _MOZART_TASK__HH_

class OzTaskQueue;

// An OzTask represents some work to be done that may block
// or require a long time to complete.  Since we don't want
// to block the emulator thread, such tasks must be delegated
// to some other threads.

class OzTask {
private:
  OzTask * _prev;
  OzTask * _next;
  friend class OzTaskQueue;
  static inline void gc_linked(OzTask*);
public:
  OzTask() : _prev(0),_next(0) {};
  virtual ~OzTask() {}
  virtual void execute(void) = 0;
  virtual void finish(void) = 0;
  virtual void gc(void) = 0;
};

extern void OZ_taskEnq(OzTask*);
extern void OZ_taskEnqTiny(OzTask*);

#endif
