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


#ifdef __GNUC__
#pragma interface
#endif

const int ALLSEQMODE=3;

class Thread : public ConstTerm
{
friend void engine();
friend class ThreadsPool;
private:
  int priority;
  Board *home;
  Board *notificationBoard; // for search capabilities;
  int compMode;
  TaskStack taskStack;

public:
  USEFREELISTMEMORY;
  OZPRINT;
  OZPRINTLONG;
  Thread *gcThread();
  void gcThreadRecurse();

  Thread(int size);
  void init(int prio,Board *home);
  int getPriority();
  // isSolve() replace by hasNotificationBoard()
  // kost@ : 'notificationBoard' is not used actually;
  Bool hasNotificationBoard () { return notificationBoard!=NULL; }
  void setNotificationBoard (Board *b) { notificationBoard = b; }
  void pushDebug(Board *b, OzDebug *d)
  {
#ifndef NEWCOUNTER
    b->incSuspCount();
#endif
    taskStack.pushDebug(b, d);
  }

  void pushCall(Board *b, Chunk *pred, RefsArray  x, int n)
  {
#ifndef NEWCOUNTER
    b->incSuspCount();
#endif
    taskStack.pushCall(b,pred,x,n);
  }

  void pushNervous(Board *b)
  {
    taskStack.pushNervous(b);
  }

  void pushCFunCont(Board *b, OZ_CFun f, Suspension* s,
                    RefsArray  x, int n, Bool copyF)
  {
#ifndef NEWCOUNTER
    if (copyF) b->incSuspCount();
#endif
    taskStack.pushCFunCont(b,f,s,x,n,copyF);
  }

  void pushCont(Board *b,ProgramCounter pc,
                RefsArray y,RefsArray g,RefsArray x,int n,
                Bool copyF)
  {
    Assert(pc!=0)
#ifndef NEWCOUNTER
    if (copyF) b->incSuspCount();
#endif
    taskStack.pushCont(b,pc,y,g,x,n,copyF);
  }
  Bool isEmpty()
  {
    return taskStack.isEmpty();
  }
  void printDebug(ProgramCounter pc, Bool verbose=NO, int depth = 10000) {
    taskStack.printDebug(pc,verbose,depth);
  }
  void setPriority(int prio);
  Board *getBoardFast() { return home->getBoardFast(); }
  int getCompMode() { return compMode; }
  void checkCompMode(int newMode);
  void setCompMode(int newMode);
  void switchCompMode();
  void Thread::getSeqFrom(Thread *th);
};


#ifndef OUTLINE
#include "thread.icc"
#endif

#endif
