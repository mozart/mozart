/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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

#include "taskstk.hh"
#include "value.hh"

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
  T_solve    = 0x000010,  // it was created in a search CS
  			  // (former notificationBoard);
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
friend class AM;
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

  void reInit() {		// for the root thread only;
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
// <Secondary Tags>               from class ConstTerm
// <board|index> | <Tertiary Tag> from class Tertiary
// <prio> | <flags>
// <id>    (debugger)
// <abstr> (profiler)
// <stack>

class Thread : public Tertiary {
  friend int engine(Bool);
  friend void scheduler();
  friend void ConstTerm::gcConstRecurse(void);
private:
  //  Sparc, for instance, has a ldsb/stb instructions - 
  // so, this is exactly as efficient as just two integers;
  Object *self;
  struct {
    int pri:    sizeof(char) * 8;
    int flags:  (sizeof(int) - sizeof(char)) * sizeof(char) * 8;
  } state;

  unsigned int id;              // unique identity for debugging
  PrTabEntry *abstr;            // for profiler
  ThreadBodyItem item;		// NULL if it's a deep 'unify' suspension;
public:
  NO_DEFAULT_CONSTRUCTORS(Thread);

  Thread(int flags, int prio, Board *bb, int id1)
    : Tertiary(bb,Co_Thread,Te_Local), id(id1)
  {
    state.flags = flags;
    state.pri = prio;

    item.threadBody = 0;

    setAbstr(NULL);
    setSelf(NULL);

    if (flags & T_stack)
      ozstat.createdThreads.incf();
  }

  Thread(int i, TertType tertType)
    : Tertiary(0,Co_Thread,tertType)
  {
    setIndex(i);
  }

  USEHEAPMEMORY;
  OZPRINTLONG;

  Thread *gcThread();
  Thread *gcThreadInline();
  Thread *gcDeadThread();
  void gcRecurse();

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

  Bool isWakeup() { 
    Assert(!isDeadThread());
    return getThrType() == S_WAKEUP;
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
    Assert(state.pri >= OZMIN_PRIORITY && state.pri <= OZMAX_PRIORITY);
    return state.pri;
  }
  void setPriority(int newPri) { 
    Assert(state.pri >= OZMIN_PRIORITY && state.pri <= OZMAX_PRIORITY);
    state.pri = newPri;
  }

  /* check if thread has a stack */
  Bool isRThread() { return (state.flags & S_TYPE_MASK) == S_RTHREAD; }

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

  Bool isInSolve() { 
    Assert (!isDeadThread());
    return state.flags & T_solve;
  }
  void setInSolve() { 
    Assert(isRunnable());
    state.flags =  state.flags | T_solve;
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

  
  int getThrType() { return (state.flags & S_TYPE_MASK); }

  // 
  //  Convert a thread of any type to a 'wakeup' thread (without a stack).
  //  That's used in GC because thread with a dead home board
  // might not dissappear during GC, but moved to a first alive board,
  // and killed in the emulator.
  void setWakeUpTypeGC() {
    state.flags = (state.flags & ~S_TYPE_MASK) | S_WAKEUP;
  }

  Bool hasStack() { 
    Assert(!isDeadThread());
    return (state.flags & T_stack);
  }
  Bool hadStack() { 
    Assert(isDeadThread ());
    return (state.flags & T_stack);
  }
  void setHasStack() { 
    Assert(isRunnable());
    state.flags = state.flags | T_stack; 
  }

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
  void pushCFun(OZ_CFun f, RefsArray  x, int n, Bool copyF) {
    item.threadBody->taskStack.pushCFun(f, x, n, copyF);
  }

  Bool hasCatchFlag() { return (state.flags & T_catch); }
  void setCatchFlag() { state.flags = state.flags|T_catch; }
  void pushCatch() {
    setCatchFlag();
    item.threadBody->taskStack.pushCatch();
  }

  Bool isEmpty() {
    return hasStack() ? item.threadBody->taskStack.isEmpty() : NO;
  }

  void printTaskStack(int depth) {
    if (!isDeadThread() && hasStack()) {
      item.threadBody->taskStack.printTaskStack(depth);
    } else {
      message("\tEMPTY\n");
      message("\n");
    }
  }


  TaskStack *getTaskStackRef() {
    Assert(hasStack());
    return &(item.threadBody->taskStack);
  }


  int getRunnableNumber();
};

inline 
Bool oz_isThread(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Thread;
}

inline
Thread *tagged2Thread(TaggedRef term)
{
  Assert(oz_isThread(term));
  return (Thread *) tagged2Const(term);
}


//-----------------------------------------------------------------------------
// class Propagator

enum PropagatorFlag {
  P_null     = 0x000000,
  P_gcmark   = 0x000001, // you must not change that
  P_dead     = 0x000002,
  P_tag      = 0x000004,
  P_nmo      = 0x000008,
  P_local    = 0x000010,
  P_runnable = 0x000020,
  P_ofs      = 0x000040,
  P_ext      = 0x000080,
  P_unify    = 0x000100,
  P_max      = 0x800000
};


#define MARKFLAG(F)        (_flags |= (F))
#define UNMARKFLAG(F)      (_flags &= ~(F))
#define UNMARKFLAGTO(T, F) ((T) (_flags & ~(F)))
#define ISMARKEDFLAG(F)    (_flags & (F))

class Propagator {
private:
  unsigned _flags;
  OZ_Propagator * _p;
  Board * _b;
  static Propagator * _runningPropagator;
public:
  Propagator(OZ_Propagator * p, Board * b) 
    : _p(p), _b(b), _flags(p->isMonotonic() ? P_null : P_nmo) 
  {
    Assert(_p);
    Assert(_b);
  }

  OZ_Propagator * getPropagator(void) { return _p; }

  void setPropagator(OZ_Propagator * p) { 
    Assert (p);
    Assert (_p);
    _p = p; 
    if (! _p->isMonotonic())
      MARKFLAG(P_nmo);
  }

  USEHEAPMEMORY;

  OZPRINTLONG;

  static void setRunningPropagator(Propagator * p) { _runningPropagator = p;} 
  static Propagator * getRunningPropagator(void) { return _runningPropagator;} 

  void dispose(void) {
    delete _p;
    
    DebugCode(
	      _p = (OZ_Propagator *) NULL;
	      _b = (Board * ) NULL;
	      );
  }

  Propagator * gcPropagator(void);
  Propagator * gcPropagatorOutlined(void);
  void gcRecurse(void);
  Bool gcIsMarked(void);
  void gcMark(Propagator *);
  void ** gcGetMarkField(void);
  Propagator * gcGetFwd(void);
  
  Board * getBoardInternal(void) {
    return _b;
  }
  
  Bool isDeadPropagator(void) {
    return ISMARKEDFLAG(P_dead);
  }
  void markDeadPropagator(void) {
    MARKFLAG(P_dead);
  }
  
  Bool isRunnable(void) {
    return ISMARKEDFLAG(P_runnable);
  }
  void unmarkRunnable(void) {
    UNMARKFLAG(P_runnable);
  }
  void markRunnable(void) {
    MARKFLAG(P_runnable);
  }
    
  Bool isLocalPropagator(void) {
    return ISMARKEDFLAG(P_local);
  }
  void unmarkLocalPropagator(void) {
    UNMARKFLAG(P_local);
  }
  void markLocalPropagator(void) {
    MARKFLAG(P_local);
  }
  
  Bool isUnifyPropagator(void) {
    return ISMARKEDFLAG(P_unify);
  }
  void unmarkUnifyPropagator(void) {
    UNMARKFLAG(P_unify);
  }
  void markUnifyPropagator(void) {
    MARKFLAG(P_unify);
  }
  
  Bool isTagged(void) {
    return ISMARKEDFLAG(P_tag);
  }
  void markTagged(void) {
    MARKFLAG(P_tag);
  }
  void unmarkTagged(void) {
    UNMARKFLAG(P_tag);
  }

  Bool isOFSPropagator(void) {
    return ISMARKEDFLAG(P_ofs); 
  }
  void markOFSPropagator(void) {
    MARKFLAG(P_ofs);
  }
  
  Bool wasExtPropagator(void) {
    return ISMARKEDFLAG(P_ext);
  }
  void setExtPropagator(void) {
    MARKFLAG(P_ext);
  }
    
  OZ_NonMonotonic::order_t getOrder(void) {
    return _p->getOrder();
  }

  Bool isNonMonotonicPropagator(void) {
    return ISMARKEDFLAG(P_nmo);
  }

  void markNonMonotonicPropagator(void) {
    MARKFLAG(P_nmo);
  }

  OZ_Propagator * swapPropagator(OZ_Propagator * prop) {
    OZ_Propagator * p = _p; 
    setPropagator(prop);
    return p;
  }
};


//-----------------------------------------------------------------------------
// class Suspension

// A suspension is a tagged pointer either to a propagator or a thread.

#define THREADTAG 0x1

class Suspension {

friend Bool operator == (Suspension, Suspension);

private:
  union _susp_t {
    Thread * _t;
    Propagator * _p;
    _susp_t(void) {
      DebugCode(_t = (Thread *) 0x5e5e5e5e);
    }
    _susp_t(Thread * t) : _t(t) {}
    _susp_t(Propagator * p) : _p(p) {}
  } _s;
  Bool _isThread(void) {  
    return ((_ToInt32(_s._t)) & THREADTAG); 
  }
  Thread * _getThread(void) { 
    return (Thread *) ((_ToInt32(_s._t)) & ~THREADTAG);
  }
  Propagator * _getPropagator(void) { 
    return _s._p;
  }
  void _markAsThread(Thread * t) { 
    _s._t = (Thread *) ((_ToInt32(t)) | THREADTAG);
  }
public:
  Suspension(void);

  Suspension(Thread * t) {
    _markAsThread(t);
  }
  Suspension(Propagator * p) : _s(p) {}

  static void *operator new(size_t);
  static void operator delete(void *, size_t);


  OZPRINTLONG;

  Bool isPropagator(void) { return !_isThread(); }
  Bool isThread(void) { return _isThread(); }
  Bool isNull(void) { return ((void *) _getThread()) == NULL;}

  void setPropagator(OZ_Propagator * p) {
    _s._p->setPropagator(p);
  }
  Propagator * getPropagator(void) {
    Assert(isPropagator());
    return _s._p;
  }
  Thread * getThread(void) {
    Assert(_isThread());
    return _getThread();
  }
  void setThread(Thread * t) {
    _markAsThread(t);
  }

  Bool isDead(void) {
    return (_isThread() 
	    ? _getThread()->isDeadThread() 
	    : _getPropagator()->isDeadPropagator());
  }
  Bool isRunnable(void) {
    return (_isThread() 
	    ? _getThread()->isRunnable() 
	    : _getPropagator()->isRunnable());
  }
  Board * getBoardInternal(void) {
    return (_isThread() 
	    ? _getThread()->getBoardInternal() 
	    : _getPropagator()->getBoardInternal());
  }
  
  void unmarkLocalPropagator(void) {
    if (isPropagator())
      _getPropagator()->unmarkLocalPropagator();
  }

  void markLocalPropagator(void) {
    if (isPropagator())
      _getPropagator()->markLocalPropagator();
  }
  Bool isLocalPropagator(void) {
    if (isPropagator())
      return _getPropagator()->isLocalPropagator();
    return FALSE;
  }

  void unmarkUnifyPropagator(void) {
    if (isPropagator())
      _getPropagator()->unmarkUnifyPropagator();
  }

  void markUnifyPropagator(void) {
    if (isPropagator())
      _getPropagator()->markUnifyPropagator();
  }
  Bool isUnifyPropagator(void) {
    if (isPropagator())
      return _getPropagator()->isUnifyPropagator();
    return FALSE;
  }

  void markTagged(void) { 
    if (_isThread())  
      _getThread()->markTagged();
    else
      _getPropagator()->markTagged();
  }
  void unmarkTagged(void) { 
    if (_isThread())  
      _getThread()->unmarkTagged();
    else
      _getPropagator()->unmarkTagged();
  }
  Bool isTagged(void) { 
    return (_isThread() 
	    ? _getThread()->isTagged()
	    : _getPropagator()->isTagged());
  }

  void setExtSuspension(void) {
    if (_isThread())
      _getThread()->setExtThread();
    else
      _getPropagator()->setExtPropagator();
  }

  Bool wasExtSuspension(void) {
    return (_isThread()
	    ? _getThread()->wasExtThread()
	    : _getPropagator()->wasExtPropagator());    
  }

  Bool isOFSPropagator(void) {
    if (isPropagator())
      return _getPropagator()->isOFSPropagator();
    return FALSE;
  }

  Suspension gcSuspension(void);
}; // class Suspension 

inline
Bool operator == (Suspension a, Suspension b)
{
  return (a._getPropagator() == b._getPropagator());
}

#define GETSUSPPTR(S)				\
(S.isThread()					\
 ? (void *) S.getThread()			\
 : (void *) S.getPropagator())

#endif

// eof
//-----------------------------------------------------------------------------
