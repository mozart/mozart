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


#include "types.hh"

class Thread : public ConstTerm
{
friend void engine();
private:
  static Thread *Head;
  static Thread *Tail;
  static Thread *Current;
  static Thread *Root;
  static int TimeSlice;
  static int UserPriority;
  static int SystemPriority;

public:
  static void Init();
  static void GC();
  static void Print();
  static Bool CheckSwitch();
  static Thread *GetCurrent();
  static Thread *GetHead();
  static Thread *GetRoot();
  static int GetSystemPriority();
  static Thread *GetTail();
  static int GetTimeSlice();
  static int GetUserPriority();
  static Bool QueueIsEmpty();
  static void Start();
  static void MakeTaskStack();
  static void ScheduleCurrent();
  static void FinishCurrent();
  static int GetCurrentPriority();
  static void NewCurrent(int prio);
  static void Schedule(Suspension *s);
  static void ScheduleRoot(ProgramCounter PC,RefsArray y);
  static void Schedule(Board *n);
private:
  static Thread *UnlinkHead();

private:
  Thread *next;
  Thread *prev;
  int flags;
  union {
    TaskStack *taskStack;
    Suspension *suspension;
    Board *board;
  };
  int priority;
public:
  USEFREELISTMEMORY;
  Thread *gc();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;

  int getPriority();
  Bool isNormal();
  Bool isWarm();
  Bool isNervous();
  void schedule();

  Board *popBoard();
  Suspension *popSuspension();
  
private:
  Thread(int p);
  Bool isScheduled();
  void insertFromTail();
  void insertFromHead();
  void insertAfter(Thread *here);
  void insertBefore(Thread *here);
  Thread *unlink();
  void dispose();
};

#endif
