/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *
 *  Contributors:
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

// interface of threads
// including propagators and LTQ

#ifndef __THREADHH
#define __THREADHH


#ifdef INTERFACE
#pragma interface
#endif

#include "thr_stack.hh"

//
// (kp) On Sparc (v8), most efficient flags are (strictly) between 0x0 and
// 0x1000. Flags up to 0x1000 and above 0x1000 should not be mixed,
// because then three instructions are required (for testing, i mean);
enum ThreadFlag {
  T_null     = 0x000000,  // no flag is set;
  T_dead     = 0x000001,  // the thread is dead;
  T_runnable = 0x000002,  // the thread is runnable;
  T_stack    = 0x000004,  // it has an (allocated) stack;
  T_catch    = 0x000008,  // has or has had an exception handler
  T_ext      = 0x000020,  // an external suspension wrt current search problem
  T_tag      = 0x000040,  // used to avoid duplication of threads
  T_lpq      = 0x000080,  // designates local thread queue

  T_noblock  = 0x000100,  // if this thread blocks, raise an exception

  // debugger
  T_G_trace  = 0x010000,   // thread is being traced
  T_G_step   = 0x020000,   // step mode turned on
  T_G_stop   = 0x040000,   // no permission to run

  T_max      = 0x800000    // MAXIMAL FLAG;
};


#define  S_TYPE_MASK  T_stack
#define  S_WAKEUP     T_null
#define  S_RTHREAD    T_stack


class RunnableThreadBody {
friend class ThreadsPool;
friend class Thread;
private:
  TaskStack taskStack;
  RunnableThreadBody *next;  /* for linking in the freelist */

  RunnableThreadBody(int sz) : taskStack(sz) {}
public:
  //  gc methods;
  RunnableThreadBody *gcRTBody();
  USEHEAPMEMORY;
  NO_DEFAULT_CONSTRUCTORS(RunnableThreadBody);

  void reInit() {               // for the root thread only;
    taskStack.init();
  }
};

union ThreadBodyItem {
  RunnableThreadBody *threadBody;
};


//  Every thread can be in the following states -
// suspended, runnable, running and dead:
//
//  Moreover, only the following transactions are possible:
//
//                    .------------> dead <-----------.
//                    |                               |
//                    |                               |
//  (create) ---> suspended -----> runnable <----> running
//                    ^                               |
//                    |                               |
//                    `-------------------------------'
//
// memory layout
// <board>
// <prio> | <flags>
// <id>    (debugger)
// <abstr> (profiler)
// <stack>

class Thread {
  friend int engine(Bool);
  friend void scheduler();
private:
  //  Sparc, for instance, has a ldsb/stb instructions -
  // so, this is exactly as efficient as just two integers;
  Board *board;
  Object *self;
  struct {
    int pri:    sizeof(char) * 8;
    int flags:  (sizeof(int) - sizeof(char)) * sizeof(char) * 8;
  } state;

  unsigned int id;              // unique identity for debugging
  PrTabEntry *abstr;            // for profiler
  ThreadBodyItem item;          // NULL if it's a deep 'unify' suspension;
public:
  NO_DEFAULT_CONSTRUCTORS(Thread)
  Thread(int flags, int prio, Board *bb, int id)
    : board(bb), id(id)
  {
    state.flags = flags;
    state.pri = prio;

    item.threadBody = 0;

    setAbstr(NULL);
    setSelf(NULL);

    if (flags & T_stack)
      ozstat.createdThreads.incf();
  }

  USEHEAPMEMORY;
  OZPRINTLONG;

  Board *getBoardInternal()        { return board; }
  void setBoardInternal(Board *bb) { board = bb; }

  Thread *gcThread();
  Thread *gcThreadInline();
  Thread *gcDeadThread();
  void gcRecurse();

  int gcIsMarked() { return ((int)board) & 1; }
  void gcMark(Thread * fwd) { board = (Board *)(((int)fwd)|1); }
  Thread * gcGetFwd() {
    Assert(gcIsMarked());
    return (Thread *) (((int)board)&~1);
  }
  void ** gcGetMarkField() { return (void **)&board; };

  void freeThreadBodyInternal() {
    Assert(isDeadThread());
    item.threadBody = 0;
  }

  void markDeadThread() {
    state.flags = state.flags | T_dead;
  }

  int getFlags() { return state.flags; }

  void markLPQThread(void) {
    state.flags = state.flags | T_lpq;
  }
  Bool isLPQThread(void) {
    return state.flags & T_lpq;
  }

  void markTagged() {
    if (isDeadThread ()) return;
    state.flags = state.flags | T_tag;
  }
  void unmarkTagged() {
    state.flags = state.flags & ~T_tag;
  }
  Bool isTagged() {
    return (state.flags & T_tag);
  }

  void setBody(RunnableThreadBody *rb) { item.threadBody=rb; }
  RunnableThreadBody *getBody()        { return item.threadBody; }

  unsigned int getID() { return id; }
  void setID(unsigned int newId) { id = newId; }

  void setAbstr(PrTabEntry *a) { abstr = a; }
  PrTabEntry *getAbstr()       { return abstr; }

  void setSelf(Object *o) { self = o; }
  Object *getSelf()       { return self; }

  int getPriority() {
    Assert(state.pri >= LOW_PRIORITY && state.pri <= HI_PRIORITY);
    return state.pri;
  }
  void setPriority(int newPri) {
    Assert(state.pri >= LOW_PRIORITY && state.pri <= HI_PRIORITY);
    state.pri = newPri;
  }

  Bool isDeadThread() { return state.flags & T_dead; }

  Bool isSuspended() {
    Assert(!isDeadThread());
    return !(state.flags & T_runnable);
  }
  Bool isRunnable() {
    /* Assert(!isDeadThread()); */ // TMUELLER
    return state.flags & T_runnable;
  }

  //  For reinitialisation;
  void setRunnable() {
    state.flags = (state.flags & ~T_dead) | T_runnable;
  }

  //  non-runnable threads;
  void markRunnable() {
    Assert(isSuspended() && !isDeadThread());
    state.flags = state.flags | T_runnable;
  }
  void unmarkRunnable() {
    Assert((isRunnable () && !isDeadThread ()) || getStop());
    state.flags &= ~T_runnable;
  }

  void setExtThread() {
    Assert (!isDeadThread());
    state.flags = state.flags | T_ext;
  }
  Bool isExtThread() {
    Assert(isRunnable());
    return state.flags & T_ext;
  }
  void clearExtThread() {
    state.flags &= ~T_ext;
  }

  Bool wasExtThread() {
    return state.flags & T_ext;
  }

  void setNoBlock(Bool yesno) {
    state.flags = yesno ? state.flags | T_noblock : state.flags & ~T_noblock;
  }
  Bool getNoBlock() {
    return (state.flags & T_noblock);
  }

  // source level debugger
  // set/delete some bits...
  void setTrace(Bool yesno) {
    state.flags = yesno ? state.flags | T_G_trace : state.flags & ~T_G_trace;
  }
  void setStep(Bool yesno) {
    state.flags = yesno ? state.flags | T_G_step  : state.flags & ~T_G_step;
  }
  void setStop(Bool yesno) {
    state.flags = yesno ? state.flags | T_G_stop  : state.flags & ~T_G_stop;
  }

  // ...and check them
  Bool getTrace() { return (state.flags & T_G_trace); }
  Bool getStep()  { return (state.flags & T_G_step); }
  Bool getStop()  { return (state.flags & T_G_stop); }


  void reInit() {  // for the root thread only;
    setRunnable();
    item.threadBody->reInit();
  }

  TaggedRef getStreamTail();
  void setStreamTail(TaggedRef v);

  void pushLPQ(Board * sb) {
    item.threadBody->taskStack.pushLPQ(sb);
  }
  void pushDebug(OzDebug *dbg, OzDebugDoit dothis) {
    item.threadBody->taskStack.pushDebug(dbg,dothis);
  }
  void popDebug(OzDebug *&dbg, OzDebugDoit &dothis) {
    PopFrame(&item.threadBody->taskStack,pc,y,cap);
    if (pc == C_DEBUG_CONT_Ptr) {
      dbg = (OzDebug *) y;
      dothis = (OzDebugDoit) (int) cap;
    } else {
      item.threadBody->taskStack.restoreFrame();
      dbg = (OzDebug *) NULL;
      dothis = DBG_EXIT;
    }
  }
  void pushCall(TaggedRef pred, TaggedRef arg0=0, TaggedRef arg1=0,
                TaggedRef arg2=0, TaggedRef arg3=0, TaggedRef arg4=0)
  {
    item.threadBody->taskStack.pushCall(pred, arg0,arg1,arg2,arg3,arg4);
  }

  void pushCall(TaggedRef pred, RefsArray  x, int n) {
    item.threadBody->taskStack.pushCall(pred, x, n);
  }
  void pushCallNoCopy(TaggedRef pred, RefsArray  x) {
    item.threadBody->taskStack.pushCallNoCopy(pred, x);
  }
  void pushCFun(OZ_CFun f, RefsArray  x, int n) {
    item.threadBody->taskStack.pushCFun(f, x, n);
  }

  Bool hasCatchFlag() { return (state.flags & T_catch); }
  void setCatchFlag() { state.flags = state.flags|T_catch; }
  void pushCatch() {
    setCatchFlag();
    item.threadBody->taskStack.pushCatch();
  }

  Bool isEmpty() {
    return item.threadBody->taskStack.isEmpty();
  }

  void printTaskStack(int depth) {
    if (!isDeadThread()) {
      item.threadBody->taskStack.printTaskStack(depth);
    } else {
      message("\tEMPTY\n");
      message("\n");
    }
  }


  TaskStack *getTaskStackRef() {
    return &(item.threadBody->taskStack);
  }


  int getRunnableNumber();
};

#endif
