/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  interface of threads and queues of threads with priorities
  ------------------------------------------------------------------------
*/



#ifndef __THREADHH
#define __THREADHH


#ifdef INTERFACE
#pragma interface
#endif

enum TFlags {
  T_Suspended   =0x0001,
  T_HasJobs     =0x0010,
};

class Thread : public ConstTerm, public TaskStack
{
friend void engine();
friend class ThreadsPool;
private:
  int priority;
  Board *board;
  int flags;

public:
  USEFREELISTMEMORY;
  OZPRINT;
  OZPRINTLONG;
  Thread *gcThread();
  void gcThreadRecurse();

  Thread(int size);
  void init(int prio,Board *home);
  int getPriority();
  void setBoard(Board *bb) {
    board=bb;
  }
  void setSuspended() { flags |= T_Suspended; }
  void unsetSuspended() { flags &= ~T_Suspended; }
  int  isSuspended() { return (flags & T_Suspended); }
  Bool isBelowFailed(Board *top);

  /*
   * Optimization:
   * If thread has more than one job the flag T_HasJobs is set.
   * Limitation: not unset when all JOB-tasks are discarded.
   */
  Bool hasJobs() {
    Assert((flags & T_HasJobs) || !TaskStack::hasJob());
    return flags & T_HasJobs;
  }
  void setHasJobs() { flags |= T_HasJobs; }
  Bool unsetHasJobs() {
    Assert(!TaskStack::hasJob());
    return flags &= ~T_HasJobs;
  }
  void pushJob()
  {
    if (!isEmpty()) {
      TaskStack::pushJob(hasJobs());
      setHasJobs();
    }
  }

  void setPriority(int prio);
  Board *getBoardFast() { return board->getBoardFast(); }

  Bool discardLocalTasks();
  int findExceptionHandler(Chunk *&pred, TaskStackEntry *&oldTos);
};


#ifndef OUTLINE
#include "thread.icc"
#endif

#endif
