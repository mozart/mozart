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

class Toplevel;

const int ALLSEQMODE=3;

class Thread : public ConstTerm
{
friend void engine();
private:
  static Thread *Head;
  static Thread *Tail;
  static Toplevel *ToplevelQueue;
  static Thread *FreeList;

public:
  static void Init();
  static void GC();
  static void Print();
  static Bool CheckSwitch();
  static Thread *GetHead();
  static Thread *GetTail();
  static Bool QueueIsEmpty();
  static Thread *GetFirst();
  static void ScheduleSuspCont(SuspContinuation *c, Bool wasExtSusp);
  static void ScheduleSuspCCont(CFuncContinuation *c, Bool wasExtSusp,
				Suspension *s = NULL);
  static void ScheduleWakeup(Board *n, Bool wasExtSusp);
  static void ScheduleSolve (Board *b); 

private:
  Thread *next;
  Thread *prev;
  int priority;
  Board *home;
  Board *notificationBoard; // for search capabilities;
  int compMode;
  Thread();
public:
  TaskStack taskStack;

public:
  static Thread *newThread(int prio,Board *home);

  USEFREELISTMEMORY;
  OZPRINT;
  OZPRINTLONG;
  Thread *gc();
  void gcRecurse();

  int getPriority();
  // isSolve() replace by hasNotificationBoard()
  Bool hasNotificationBoard () { return notificationBoard ? OK : NO; }
  void setNotificationBoard (Board *b) { notificationBoard = b; }
  Board* getNotificationBoard () { return (notificationBoard); }
  void pushTask(Board *n,ProgramCounter pc,
		RefsArray y,RefsArray g,RefsArray x=NULL,int i=0);
  void pushTask(Board *n, OZ_CFun f, RefsArray x=NULL, int i=0);
  void schedule();
  void setPriority(int prio);
  void checkToplevel();
  void addToplevel(ProgramCounter pc);
  void pushToplevel(ProgramCounter pc);
  Board *getHome() { return home->getBoardDeref(); }
  void setHome(Board *b) { home=b; }
  int getCompMode() { return compMode; }
  void checkCompMode(int newMode);
  void setCompMode(int newMode);
  void switchCompMode();
private:
  void init(int prio,Board *home);
  Bool isScheduled();
  void insertFromTail();
  void insertFromHead();
  void insertAfter(Thread *here);
  void insertBefore(Thread *here);
  Thread *unlink();
  void dispose();
};


#ifndef OUTLINE
#include "thread.icc"
#endif

#endif
