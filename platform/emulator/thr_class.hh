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

enum T_Flag
{
  T_Normal	= 0,
  T_Warm	= 1<<0,
  T_Nervous	= 1<<1,
  T_RootTask	= 1<<2,
  T_Freezed	= 1<<9,
};

class Thread : public ConstTerm
{
friend void engine();
private:
  static Thread *Head;
  static Thread *Tail;

public:
  static Thread *Current;
  static int TimeSlice;
  static int UserPriority;
  static int SystemPriority;
  static Bool CheckSwitch();
  static Thread *GetCurrentDBG();
  static Thread *GetHeadDBG();
  static Thread *GetTailDBG();
  static Bool QueueIsEmpty();
  static void Start();
  static void GC();
  static void MakeTaskStack();

private:
  static Thread *UnlinkHead();

private:
  Thread *next;
  Thread *prev;
  int flags;
  union {
    TaskStack *taskStack;
    Suspension *suspension;
    Continuation *continuation;
    Board *board;
  };
  int priority;
public:
  USEFREELISTMEMORY;
  Thread *gc();
  OZPRINT;
  OZPRINTLONG;

  Thread(Suspension *s);
  Thread(Continuation *c,int p);
  Thread(int p);
  Thread(Board *n);

  void activate();
  void finished();
  int getPriority();
  TaskStack *getTaskStack();
  State getValue(TaggedRef feature,TaggedRef out); // see ../builtins/debug.C
  void setPriority(int i);
  State setValue(TaggedRef feature,TaggedRef value);
  void schedule();
private:
  Bool isFreezed();
  Bool isScheduled();
  void insertFromTail();
  void insertFromHead();
  void insertAfter(Thread *here);
  void insertBefore(Thread *here);
  Thread *unlink();
  void dispose();
};

#endif
