/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
  T_dead     = 0x001,  // the thread is dead;
  T_runnable = 0x002,  // the thread is runnable;
  T_catch    = 0x004,  // has or has had an exception handler
  T_ext      = 0x008,  // an external suspension wrt current search problem
  T_tag      = 0x010,  // used to avoid duplication of threads
  T_noblock  = 0x040,  // if this thread blocks, raise an exception
  // debugger
  T_G_trace  = 0x080,   // thread is being traced
  T_G_step   = 0x100,   // step mode turned on
  T_G_stop   = 0x200,   // no permission to run
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
// <stack>

class Thread {
  friend int engine(Bool);
  friend void scheduler();
private:
  //  Sparc, for instance, has a ldsb/stb instructions -
  // so, this is exactly as efficient as just two integers;
  Board *board;
  struct {
    int pri:    sizeof(char) * 8;
    int flags:  (sizeof(int) - sizeof(char)) * sizeof(char) * 8;
  } state;

  unsigned int id;              // unique identity for debugging
  PrTabEntry *abstr;            // for profiler
  TaskStack *taskStack;
public:
  NO_DEFAULT_CONSTRUCTORS(Thread)
  Thread(int flags, int prio, Board *bb, int id)
    : board(bb), id(id), abstr(0)
  {
    state.flags = flags;
    state.pri   = prio;
    ozstat.createdThreads.incf();
    taskStack   = new TaskStack(ozconf.stackMinSize);
  }

  USEHEAPMEMORY;
  OZPRINTLONG;

  Board *getBoardInternal()        { return board; }
  void setBoardInternal(Board *bb) { board = bb; }

  Thread *gcThread();
  Thread *gcDead();
  void gcRecurse();

  int gcIsMarked() { return ((int)board) & 1; }
  void gcMark(Thread * fwd) { board = (Board *)(((int)fwd)|1); }
  Thread * gcGetFwd() {
    Assert(gcIsMarked());
    return (Thread *) (((int)board)&~1);
  }
  void ** gcGetMarkField() { return (void **)&board; };


  Bool isDead() {
    return state.flags & T_dead;
  }
  void setDead() {
    state.flags = state.flags | T_dead;
  }

  int getFlags() { return state.flags; }

  void setTagged() {
    if (isDead()) return;
    state.flags = state.flags | T_tag;
  }
  void unsetTagged() {
    state.flags = state.flags & ~T_tag;
  }
  Bool isTagged() {
    return (state.flags & T_tag);
  }

  int getPriority() {
    return state.pri;
  }
  void setPriority(int p) {
    state.pri = p;
  }

  Bool isRunnable() {
    return state.flags & T_runnable;
  }

  //  non-runnable threads;
  void setRunnable() {
    Assert(isSuspended() && !isDead());
    state.flags = state.flags | T_runnable;
  }
  void unsetRunnable() {
    Assert((isRunnable () && !isDead()) || isStop());
    state.flags &= ~T_runnable;
  }

  Bool isExternal() {
    return state.flags & T_ext;
  }
  void setExternal() {
    state.flags = state.flags | T_ext;
  }
  void unsetExternal() {
    state.flags &= ~T_ext;
  }


  void setNoBlock() {
    state.flags = state.flags | T_noblock;
  }
  void unsetNoBlock() {
    state.flags = state.flags & ~T_noblock;
  }
  Bool getNoBlock() {
    return (state.flags & T_noblock);
  }


  // source level debugger
  // set/delete some bits...
  void setTrace() {
    state.flags = state.flags | T_G_trace;
  }
  void setStep() {
    state.flags = state.flags | T_G_step;
  }
  void setStop() {
    state.flags = state.flags | T_G_stop;
  }
  void unsetTrace() {
    state.flags = state.flags & ~T_G_trace;
  }
  void unsetStep() {
    state.flags = state.flags & ~T_G_step;
  }
  void unsetStop() {
    state.flags = state.flags & ~T_G_stop;
  }

  // ...and check them
  Bool isTrace() { return (state.flags & T_G_trace); }
  Bool isStep()  { return (state.flags & T_G_step); }
  Bool isStop()  { return (state.flags & T_G_stop); }

  Bool isCatch() { return (state.flags & T_catch); }
  void setCatch() { state.flags = state.flags|T_catch; }

  unsigned int getID() {
    return id;
  }
  void setID(unsigned int i) {
    id = i;
  }

  void setAbstr(PrTabEntry * a) {
    abstr = a;
  }
  PrTabEntry *getAbstr(void) {
    return abstr;
  }


  Bool isSuspended() {
    Assert(!isDead());
    return !isRunnable();
  }


  void pushDebug(OzDebug *dbg, OzDebugDoit dothis) {
    taskStack->pushDebug(dbg,dothis);
  }

  void popDebug(OzDebug *&dbg, OzDebugDoit &dothis) {
    PopFrame(taskStack,pc,y,cap);
    if (pc == C_DEBUG_CONT_Ptr) {
      dbg = (OzDebug *) y;
      dothis = (OzDebugDoit) (int) cap;
    } else {
      taskStack->restoreFrame();
      dbg = (OzDebug *) NULL;
      dothis = DBG_EXIT;
    }
  }
  void pushCall(TaggedRef pred, TaggedRef arg0=0, TaggedRef arg1=0,
                TaggedRef arg2=0, TaggedRef arg3=0, TaggedRef arg4=0)
  {
    taskStack->pushCall(pred, arg0,arg1,arg2,arg3,arg4);
  }

  void pushCall(TaggedRef pred, RefsArray  x, int n) {
    taskStack->pushCall(pred, x, n);
  }
  void pushCallNoCopy(TaggedRef pred, RefsArray  x) {
    taskStack->pushCallNoCopy(pred, x);
  }
  void pushCFun(OZ_CFun f, RefsArray  x, int n) {
    taskStack->pushCFun(f, x, n);
  }

  void pushCatch() {
    setCatch();
    taskStack->pushCatch();
  }

  void pushSelf(Object * s) {
    taskStack->pushSelf(s);
  }

  Bool isEmpty() {
    return taskStack->isEmpty();
  }

  void printTaskStack(int);

  TaskStack *getTaskStackRef() {
    return taskStack;
  }

  void disposeStack(void) {
    taskStack->dispose();
  }


  int getRunnableNumber();
};

#endif
