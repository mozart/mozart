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

#include "types.hh"

#include "taskstk.hh"
#include "board.hh"

class Toplevel;

const int ALLSEQMODE=3;

class Thread : public ConstTerm
{
friend void engine();
public:
  Thread *next;
  Thread *prev;
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
  Thread *gc();
  void gcRecurse();

  Thread(int size);
  void init(int prio,Board *home);
  int getPriority();
  // isSolve() replace by hasNotificationBoard()
  Bool hasNotificationBoard () { return notificationBoard ? OK : NO; }
  void setNotificationBoard (Board *b) { notificationBoard = b; }
  Board* getNotificationBoard () { return (notificationBoard); }

  void pushDebug(Board *b, OzDebug *d)
  {
#ifndef NEWCOUNTER
    b->incSuspCount();
#endif
    taskStack.pushDebug(b, d);
  }

  void pushCall(Board *b, SRecord *pred, RefsArray  x, int n)
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
		    RefsArray  x=NULL, int n=0, Bool copyF=OK)
  {
#ifndef NEWCOUNTER
    if (copyF) b->incSuspCount();
#endif
    taskStack.pushCFunCont(b,f,s,x,n,copyF);
  }

  void pushCont(Board *b,ProgramCounter pc,
		RefsArray y,RefsArray g=NULL,RefsArray x=NULL,int n=0,
		Bool copyF=OK)
  {
#ifndef NEWCOUNTER
    if (copyF) b->incSuspCount();
#endif
    taskStack.pushCont(b,pc,y,g,x,n,copyF);
  }
  Bool isEmpty()
  {
    return taskStack.isEmpty();
  }
  void printDebug(ProgramCounter pc, Bool verbose, int depth = 10000) {
    taskStack.printDebug(pc,verbose,depth);
  }
  void printSuspension(ostream &out) {
    taskStack.print(out);
  }

  void setPriority(int prio);
  Board *getHome() { return home->getBoardDeref(); }
  void setHome(Board *b) { home=b; }
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
