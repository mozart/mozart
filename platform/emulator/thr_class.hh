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

enum TFlags {
  T_Suspended=0x0001
};

class Thread : public ConstTerm
{
friend void engine();
friend class ThreadsPool;
private:
  int priority;
  Board *board;
  short compMode;
  short flags;
  TaskStack taskStack;

public:
  USEFREELISTMEMORY;
  OZPRINT;
  OZPRINTLONG;
  Thread *gcThread();
  void gcThreadRecurse();

  Thread(int size);
  void init(int prio,Board *home,int compMode);
  int getPriority();
  void pushDebug(OzDebug *d)
  {
    taskStack.pushDebug(d);
  }

  void setSuspended() { flags |= T_Suspended; }
  void unsetSuspended() { flags &= ~T_Suspended; }
  int  isSuspended() { return (flags & T_Suspended); }
  void pushCall(Chunk *pred, RefsArray  x, int n)
  {
    taskStack.pushCall(pred,x,n);
  }

  void pushNervous()
  {
    taskStack.pushNervous();
  }
  void pushSolve()
  {
    taskStack.pushSolve();
  }
  void pushLocal()
  {
    taskStack.pushLocal();
  }

  void pushCFunCont(OZ_CFun f, Suspension* s,
		    RefsArray  x, int n, Bool copyF)
  {
    taskStack.pushCFunCont(f,s,x,n,copyF);
  }

  void pushCont(ProgramCounter pc,
		RefsArray y,RefsArray g,RefsArray x,int n,
		Bool copyF)
  {
    Assert(pc!=0)
    taskStack.pushCont(pc,y,g,x,n,copyF);
  }
  Bool isEmpty()
  {
    return taskStack.isEmpty();
  }
  void printDebug(ProgramCounter pc, Bool verbose=NO, int depth = 10000) {
    taskStack.printDebug(pc,verbose,depth);
  }
  void setPriority(int prio);
  Board *getBoardFast() { return board->getBoardFast(); }
  int getCompMode() { return compMode; }
  void checkCompMode(int newMode);
  void setCompMode(int newMode);
  void switchCompMode();
  void getSeqFrom(Thread *th);
};


#ifndef OUTLINE
#include "thread.icc"
#endif

#endif
