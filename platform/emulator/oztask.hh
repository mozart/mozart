#ifndef _OZTASK__HH__
#define _OZTASK__HH__

class OzTaskQueue;

// ===================================================================
// an OzTask represents some work to be done that may block or take a
// a long time to complete.  Since we don't want to block the main
// emulator thread, such a task must be delegated to another thread.
//
// It is intended that the programmer should define a new task by
// subclassing OzTask.  A task's data members should consist of an Oz
// part and a C part.  In the Oz part must go all data that resides
// on the Oz heap or refers to data that resides on the Oz heap.  In
// the C part goes any ordinary C datatypes.
//
// The asynchronous work to be done by the task is specified by its
// execute() method.  The execute() method must only access the C part
// of the task's data members.  If it needs information from the Oz
// part, this info must be obtained and installed into the C part by
// the task's contructor or whoever creates the task.  The execute()
// method must not touch anything that has to do with memory owned by
// the main emulator thread (primarily, this means the Oz heap - but
// also statically allocated storage).  It should not even _look_ at
// it as the view it would get cannot be guaranteed to be consistent
// without explicit synchronization (which would be prohibitively
// expensive).
//
// If the task needs to communicate some information back to the
// emulator, it should do it with the finish() method which will be
// invoked synchronously by the main emulator thread just before
// it is deleted (and it's destructor invoked).
//
// Since a task may hold on to Oz data, the programmer must provide a
// gc() method.  This method is invoked synchronously by the main
// emulator thread during garbage collection: therefore it cannot be
// invoked during execution of a task's finish() method nor of its
// destructor on the main emulator thread; for this reason, no special
// precaution is taken for synchronization.
// ===================================================================

class OzTask {
private:
  OzTask * _next;
  friend class OzTaskQueue;
public:
  OzTask() : _next(0) {}
  virtual ~OzTask() {}
  virtual void execute(void) = 0;
  virtual void finish(void) = 0;
  virtual void gc(void) = 0;
};

#endif
