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

#include "suspendable.hh"
#include "thr_stack.hh"


/*  Every thread can be in the following states - 
 * suspended, runnable, running and dead:
 *
 *  Moreover, only the following transactions are possible:
 *
 *                    .------------> dead <-----------.
 *                    |                               |
 *                    |                               |
 *  (create) ---> suspended -----> runnable <----> running
 *                    ^                               |
 *                    |                               |
 *                    `-------------------------------'
 *
 */


class Thread : public Suspendable {
  friend void scheduler(void);
private:
  unsigned int id;              // unique identity for debugging
  PrTabEntry *abstr;            // for profiler
  TaskStack *taskStack;

public:
  NO_DEFAULT_CONSTRUCTORS(Thread);
  USEFREELISTMEMORY;

  Thread(int flags, int prio, Board * bb, int i)
    : Suspendable(flags | (prio << PRIORITY_SHIFT), bb), id(i), abstr(0) {
    taskStack = new TaskStack(ozconf.stackMinSize); 
    ozstat.createdThreads.incf();
  }

  OZPRINTLONG;

  void gCollectRecurse(Thread *);
  void sCloneRecurse(Thread *);

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

  void pushDebug(OzDebug *dbg, Atom * dothis) {
    taskStack->pushDebug(dbg, dothis);
  }

  void popDebug(OzDebug *&dbg, Atom * &dothis) {
    PopFrame(taskStack,pc,y,cap);
    if (pc == C_DEBUG_CONT_Ptr) {
      dbg = (OzDebug *) y;
      dothis = (Atom *) cap;
    } else {
      taskStack->restoreFrame();
      dbg = (OzDebug *) NULL;
      dothis = DBG_EXIT_ATOM;
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

  void setTaskStack(TaskStack * ts) {
    taskStack = ts;
  }

  void disposeStack(void) {
    taskStack->dispose();
  }

};

#endif
