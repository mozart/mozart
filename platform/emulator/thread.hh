/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  interface of threads (both runnable and non-runnable);
  ------------------------------------------------------------------------
*/

#ifndef __THREADHH
#define __THREADHH


#ifdef INTERFACE
#pragma interface
#endif

#include "fdinterface.hh"

#ifdef OUTLINE
#define inline
#endif

//
//  On Sparc (v8), most efficient flags are (strictly) between 0x0 and 
// 0x1000. Flags up to 0x1000 and above 0x1000 should not be mixed, 
// because then three instructions are required (for testing, i mean);
enum ThreadFlag {
  T_U_null   = 0x000000, // no flag is set;
  //
  //  Semantics of abbreviations:
  //  T_T_*  - runnable&running states (former threads);
  //  T_S_*  - non-runnable state (former suspensions);
  //  T_G_*  - general (i.e. in all states);
  //

  //
  T_G_dead   = 0x000001,   // thte thread is dead;

  //  Temporary: 'real' propagators by Tobias;
  T_G_new_p_thr = 0x000002,

  //
  T_G_hasjob = 0x000004,   // has more than one job on the stack;
  T_T_stack  = 0x000008,   // it has (allocated) stack;
  T_T_solve  = 0x000010,   // it was created in a search CS 
                           // (former notificationBoard);

  //
  //  Note that at least some 'T_S_' flags should be preserved when a thread 
  // becomes runnable, because in the case of suspension they might be 
  // needed again. This is true, for instance, for the 'T_S_ext', and all
  // FD-specific flags;
  T_G_prop   = 0x000020,   // "propagated", i.e. the thread is runnable;
  T_S_ext    = 0x000040,   // an external suspension wrt current search
			   // problem;

  //  Note that T_G_p_thr is used when a 'proper' runnable thread containing 
  // a propagator is suspended again (see emulate.cc); 
  T_G_p_thr  = 0x000080,   // a 'propagator' thread, i.e. a
                           // non-dead thread remains in a suspension
                           // list at propagation;

  T_S_cfun   = 0x000100,   // the continuation contains a pointer to
			   // a c-function;
  T_S_unif   = 0x000200,   // the thread is due to a (proper) unification 
                           // of fd variables; 
  T_S_loca   = 0x000400,   // all variables of this propagator are local;

  //
  T_S_tag    = 0x000800,   // a special stuff for fd 
			   // (Tobias, please comment?);
  T_S_ofs    = 0x001000,   // the OFS thread (needed for DynamicArity);
  T_S_stable = 0x002000,   // do wake up on stablity of a search actor;

  //
  M_max    = 0x800000      // MAXIMAL FLAG;
};

//  Types of a suspended thread;
//  ('S_RTHREAD' is a (generically) suspended thread);
#define  S_TYPE_MASK  (T_T_stack|T_S_cfun|T_G_p_thr|T_G_new_p_thr)
#define  S_WAKEUP     T_U_null
#define  S_RTHREAD    T_T_stack
#define  S_CFUN       (T_S_cfun)
#define  S_PR_THR     (T_S_cfun|T_G_p_thr)
//  The last corresponds to the 'old' 'propagator' threads;
#define  S_NEW_PR_THR T_G_new_p_thr

//
//  Flags & priority of a thread;
//
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
class ThreadState {
private:
  //  Sparc, for instance, has a ldsb/stb instructions - 
  // so, this is exactly as efficient as just two integers;
  struct {
    int pri:    sizeof(char) * 8;
    int flags:  (sizeof(int) - sizeof(char)) * sizeof(char) * 8;
  } state;

protected:
  //  Note that other flags are removed;
  void markDeadThread () { 
    state.flags = state.flags | T_G_dead;
  }
  //
  //  for 'Thread::print ()';
  int getFlags () { return (state.flags); }

public:
  //
  //  Initialisation;
  //  Note that a thread is made alive and suspended, and with 
  // some undefined priority (needs further initialisation!);
  ThreadState () {
    state.flags = T_U_null;
  }
  ThreadState (int inFlags) {
    state.flags = inFlags;
  }
  ThreadState (int inFlags, int inPri) {
    state.pri = inPri;
    state.flags = inFlags;
  }

  //  priority;
  int getPriority() { 
    Assert (state.pri >= OZMIN_PRIORITY && state.pri <= OZMAX_PRIORITY);
    return (state.pri);
  }
  void setPriority (int newPri) { 
    Assert (state.pri >= OZMIN_PRIORITY && state.pri <= OZMAX_PRIORITY);
    state.pri = newPri;
  }

  //
  void setHasJobs() { 
    Assert (isRunnable ());
    state.flags = state.flags | T_G_hasjob; 
  }
  void unsetHasJobs() {
    state.flags = state.flags & ~T_G_hasjob;
  }
  Bool hasJobs () { return (state.flags & T_G_hasjob); }

  //  
  Bool isDeadThread () { return (state.flags & T_G_dead); }

  //  ... if not dead;
  Bool isSuspended () { 
    Assert (!(isDeadThread ()));
    return (!(state.flags & T_G_prop));
  }
  Bool isPropagated () { 
    Assert (!(isDeadThread ()));
    return (state.flags & T_G_prop);
  }
  //  'isRunnable' is just an alias for the 'isPropagated'; 
  Bool isRunnable () { 
    Assert (!(isDeadThread ()));
    return (state.flags & T_G_prop);
  }

  // kost@ : TODO: Is this really needed?
  //  special for FDs: if a propagator thread is propagated;
  Bool isPropagatedRes () { 
    Assert (!(isDeadThread ()));
    return (state.flags & (T_G_prop|T_G_p_thr) == (T_G_prop|T_G_p_thr));
  }

  //  For reinitialisation; 
  void setRunnable () { 
    state.flags = (state.flags & ~T_G_dead) | T_G_prop;
  }

  Bool isInSolve () { 
    Assert (!(isDeadThread ()));
    return (state.flags & T_T_solve);
  }
  void setInSolve () { 
    Assert (isRunnable ());
    state.flags = state.flags | T_T_solve;
  }

  //  non-runnable threads;
  void markPropagated () {
    Assert (isSuspended () && !(isDeadThread ()));
    state.flags = state.flags | T_G_prop;
  }
  void unmarkPropagated () { 
    Assert (isPropagated () && !(isDeadThread ()));
    state.flags = state.flags & ~T_G_prop;
  }

  void setExtThread () { 
    Assert (isSuspended () && !(isDeadThread ()));
    state.flags = state.flags | T_S_ext;
  }
  Bool isExtThread () { 
    Assert (isRunnable ());
    return (state.flags & T_S_ext);
  }
  Bool wasExtThread () {
    Assert (isDeadThread ());	// already killed!
    return (state.flags & T_S_ext);
  }

  void markPropagatorThread () { 
    Assert (!(isDeadThread ()));
    state.flags = state.flags | T_G_p_thr;
  }
  void unmarkPropagatorThread () { 
    Assert (!(isDeadThread ()));
    state.flags = state.flags & ~T_G_p_thr;
  }
  Bool isPropagator () { 
    Assert (!(isDeadThread ()));
    return (state.flags & T_G_p_thr);
  }

  void markNewPropagatorThread () { 
    Assert (!(isDeadThread ()));
    state.flags = state.flags | T_G_new_p_thr;
  }
  void unmarkNewPropagatorThread () { 
    Assert (!(isDeadThread ()));
    state.flags = state.flags & ~T_G_new_p_thr;
  }
  Bool isNewPropagator () { 
    Assert (!(isDeadThread ()));
    return (state.flags & T_G_new_p_thr);
  }
  
  void headInit (void) {
    state.flags = T_G_p_thr | T_G_prop | T_S_unif | S_CFUN;
  }
  void anyGlobalInit (void) {
    state.flags = T_G_p_thr | T_G_prop;
  }

  int getThrType () { return (state.flags & S_TYPE_MASK); }

  // 
  //  Convert a thread of any type to a 'wakeup' thread (without a stack).
  //  That's used in GC because thread with a dead home board
  // might not dissappear during GC, but moved to a first alive board,
  // and killed in the emulator.
  void setWakeUpTypeGC () {
    state.flags = (state.flags & ~S_TYPE_MASK) | S_WAKEUP;
  }

  void markUnifyThread () { 
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags | T_S_unif;
  }
  void unmarkUnifyThread () { 
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags & ~T_S_unif;
  }
  Bool isUnifyThread () {
    Assert (isPropagator () && !(isDeadThread ()));
    return (state.flags & T_S_unif);
  }

  void markLocalThread () {
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags | T_S_loca;
  }
  void unmarkLocalThread () {
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags & ~T_S_loca;
  }
  Bool isLocalThread () {
    Assert (isPropagator () && !(isDeadThread ()));
    return (state.flags & T_S_loca);
  }

  void setOFSThread () {
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags | T_S_ofs;
  }
  Bool isOFSThread () {
    Assert (!(isDeadThread ()));
    return (state.flags & T_S_ofs);
  }

  void markTagged () { 
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags | T_S_tag;
  }
  void unmarkTagged () { 
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags & ~T_S_tag;
  }
  Bool isTagged () { 
    Assert (isPropagator () && !(isDeadThread ()));
    return (state.flags & T_S_tag);
  }

  void markStable () { 
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags | T_S_stable;
  }
  void unmarkStable () { 
    Assert (isPropagator () && !(isDeadThread ()));
    state.flags = state.flags & ~T_S_stable;
  }
  Bool isStable () { 
    Assert (isPropagator () && !(isDeadThread ()));
    return (state.flags & T_S_stable);
  }

  //  stack;
  Bool hasStack () { 
    Assert (!(isDeadThread ()));
    return (state.flags & T_T_stack);
  }
  Bool hadStack () { 
    Assert (isDeadThread ());
    return (state.flags & T_T_stack);
  }
  void setHasStack () { 
    Assert (isRunnable ());
    Assert (!(isPropagator ()));
    state.flags = (state.flags & ~(S_CFUN)) | T_T_stack; 
  }
};

// 
//  Body of the runnable thread;
class RunnableThreadBody {
friend class Thread;
private:
  TaskStack taskStack;
public:
  void* operator new (size_t size) { 
    error ("'RunnableThreadBody::new ()' is applied!");
    return ((void *) NULL);	// just to keep gcc happy;
  }
  static void operator delete (void *ptr, size_t size) {
    error ("'RunnableThreadBody::delete ()' is applied!");
  }

  // 
  //  Note that 'RunnableThreadBody'"s
  // are allocated in pre-defined regions, and, therefore, 
  // their allocators may not be used.
  //  The same could be done for 'CFuncContinuation'"s, but these
  // are treated specially beacuse of the memory efficiency and 
  // low probability that they become a proper running thread 
  // (i.e. with a taskstack);
  void init (int size);		// size of the stack;
  void reInit ();		// for the root thread only;

  //  gc methods;
  RunnableThreadBody *gcRTBody ();
  void gcRecurse ();
};

//
//  Note that 'CFuncContinuation'"s are allocated in 'USEFREELISTMEMORY'
// fashion. The idea behind it is that such threads are seldom getting a 
// runnable thread, i.e. a thread with a stack. 
// (Hmm, i hope that's true.). And, anyway, 'CFuncContinuation ' can be 
// padded anytime to the 'threadBodySize' (see beneath), and run 
// therefore in constant space.
class CFuncContinuation {
friend class Thread;
private:
  OZ_CFun cFunc;
  RefsArray xRegs;
public:
  USEFREELISTMEMORY;
  void disposeRegs ();
  void dispose ();
  
  CFuncContinuation (OZ_CFun f, RefsArray x, int i);
  void init (OZ_CFun f, RefsArray x, int i) { error(""); };

  OZ_CFun getCFunc () { return (cFunc); }

  //  some special FD stuff;
  void setCFuncRegs(OZ_CFun f, RefsArray x) {
    cFunc = f;
    xRegs = x;
  }

  void setX (RefsArray x, int i);  
  int getXSize ();
  RefsArray getX () { return (xRegs); }
  void getX (RefsArray X);
  OZ_Term &getXReg (int i) {
    Assert (0 <= i && i < getXSize()); 
    return (xRegs[i]);
  }

  //  gc methods;
  CFuncContinuation *gcCFuncCont ();
  void gcRecurse ();

  OZPRINT;
};

//
//  
union ThreadBodyItem {
  CFuncContinuation *ccont;
  OZ_Propagator *propagator;
  RunnableThreadBody *threadBody;
};  

//
//  Allocation policy: 
// RunnableThread stay in their own regions, and 
// CFuncContinuation is allocated in smaller region (there is 
// a free list of them). 
const size_t threadBodySize = sizeof (RunnableThreadBody);

//
//  'Lightweight' thread - for local computation only 
// (aka local propagation, etc.);
class LWThread : public ThreadState {
friend class ThreadsPool;
protected:
  ThreadBodyItem item;		// NULL if it's a deep 'unify' suspension;
public:
  //
  //  Tobias' stub. 
  //  NOT in the sense "a stub FOR Tobias", but "a stub which 
  // CAN BE USED by Tobias" :-)))
  //  So, define here all what your soul ever wants! :-))

  LWThread ()
    : ThreadState () {}
  LWThread (int inFlags)
    : ThreadState (inFlags) {}
  LWThread (int inFlags, int inPri)
    : ThreadState (inFlags, inPri) {}
};

//
//  Generic 'Thread' class;
//  It contains a reference to either a kind of continuation, or
// the body of the 'RunnableThread';
class Thread : public LWThread {		// it's a kind of hanger;
private:
  Board *board;

  //  special allocator for thread's bodies;
  RunnableThreadBody *allocateBody ();
  void freeThreadBody ();

  //
  void disposeThread ();
  
  Bool wakeUpCCont (Board *home, PropCaller calledBy = pc_propagator);
  Bool wakeUpBoard (Board *home);
  Bool wakeUpThread (Board *home);

  // 
  void setExtThreadOutlined (Board *varHome);
  //  it asserts that the suspended thread is 'external' (see beneath);
  void checkExtThreadOutlined ();

  //
  //  Adds the thread to the 'stable' list in the current solve actor;
  void addStableThreadOutlined ();

  //  
  //  ... with furhter assertions;
  DebugCode (void markDeadThread ();)
public:
  USEHEAPMEMORY;
  OZPRINT;
  OZPRINTLONG;
  // 
  Thread *gcThread ();
  void gcRecurse ();

  //  General;
  //  Zeroth: constructors for various cases;
  void reInit (int prio, Board *home);  // for the root thread only;

  //  It makes a runnable thread with a task stack;
  Thread (int prio, Board *b, Bool inSolve = NO);
  //  They make a suspended thread;
  Thread (Board *b);
  Thread (Board *b, int prio, OZ_CFun f, RefsArray x, int i);
  //  an empty suspended sequential thread (with a task stack!);
  Thread (Board *b, int prio);
  //
  Thread (int prio, OZ_Propagator *p);

  //  First - it inherits all the methods of 'ThreadState';
  //  Second - get/set the home board;
  Board *getBoardFast ();
  void setBoard (Board *bp) { board = bp; }

  //  Third - transactions between states;
  //  These methods are specialised because the type of suspended thread
  // is known statically;
  void wakeupToRunnable ();
  void cContToRunnable ();
  void suspThreadToRunnable ();

  // 
  //  Note that the stack is allocated now in "lazy fashion", i.e. 
  // that it is created only when a thread is getting "running"; 
  //  This can be simply switched off when the stack is allocated 
  // in '<something>ToRunnable ()'; 'makeRunning ()' is getting 
  // empty in this case;
  void makeRunning ();

  // 
  //  Note: killing the suspended thread *might not* make any
  // actor reducible OR reducibility must be tested somewhere else !!!
  //
  //  Invariant: 
  // There can be no threads which are suspended not in its 
  // "proper" home, i.e. not in the comp. space where it is started;
  // 
  //  Note that this is true for wakeups, continuations etc. anyway, 
  // *and* this is also true for threads suspended in the sequential 
  // mode! The point is that whenever a thread tries to suspend in a 
  // deep guard, a new (local) thread is created which carries the 
  // rest of the guard (Hi, Michael!);
  //
  //  Note also that these methods don't decrement suspCounters, 
  // thread counters or whatever because their home board might 
  // not exist at all - such things should be done outside!
  void disposeSuspendedThread ();
  //
  //  It marks the thread as dead and disposes it;
  void disposeRunnableThread ();

  //  
  //  Mark a suspended thread as an 'external' one, 
  // and increcement thread counters in solve actor(s) above 
  // the current board;
  void updateExtThread (Board *varHome);
  //  Check all the solve actors above for stabily 
  // (and, of course, wake them up if needed);
  void checkExtThread ();

  //
  //  Runnable (running) threads;
  Bool discardLocalTasks();
  //
  int findExceptionHandler(TaggedRef &pred, TaskStackEntry *&oldTos);
  // 
  //  The iterative procedure which cleans up all the tasks 
  // up to the 'current' board, and sets the 'current' to the 
  // thread's board;
  void cleanUp (Board *current);

  //
  Bool isBelowFailed (Board *top);

  //
  void pushDebug (OzDebug *d);
  void pushCall (TaggedRef pred, RefsArray  x, int n); 
  void pushJob ();
  void pushSolve ();
  void pushLocal ();
  void pushCFunCont (OZ_CFun f, RefsArray  x, int n, Bool copyF);
  void pushCont (ProgramCounter pc, 
		 RefsArray y, RefsArray g, RefsArray x, int n,
		 Bool copyF);
  void pushCont (Continuation *cont);
  //
  Bool isEmpty(); 

  //
  void printTaskStack (ProgramCounter pc, 
		       Bool verbose = NO, int depth = 10000);

  // 
  //  Gets the size of the stack segment with the top-level job, 
  // and *virtually* removes it -- i.e. moves the 'tos' downwards, 
  // while preserving the data. It means in particular, 
  // that it must be followed by the 'getJob ()'!
  int getSeqSize ();
  //  
  //  'getJob' yields a *suspended* thread!
  Thread *getJob ();
  //
  DebugCode (Bool hasJobDebug (););

#ifdef DEBUG_CHECK
  //  redefined from the ThreadState - because of assertion;
  void unsetHasJobs() {
    Assert (!(hasJobDebug ()));
    ThreadState::unsetHasJobs ();
  }
#endif

  void pushExceptionHandler (TaggedRef pred);

  //
  CFuncContinuation *getCCont () {
    Assert (getThrType () == S_CFUN || 
	    getThrType () == S_PR_THR);
    return (item.ccont);
  }

  //
  TaskStack *getTaskStackRef ();
  TaskStackEntry *getTop ();
  void setTop (TaskStackEntry *newTos);
  TaskStackEntry pop ();

  /*
   *  propagators special;
   *
   */
  //
  //  Note that this function *does not* reset the 'propagated' flag; 
  void replacePropagator (OZ_CFun biFun, RefsArray xRegs, int xSize);
  //  Just throws away the biFun (for debugging only);
  DebugCode 
  (void removePropagator () { 
    Assert (isPropagator ());
    item.ccont->cFunc = (OZ_CFun) NULL;
  });

  //  For debugging only - get a reference to the 'cFunc';
  DebugCode 
  (OZ_CFun getPropagator () { 
    Assert (isPropagator ());
    return (item.ccont->cFunc);
  });

  //  
  //  (re-)Suspend a propagator again; (was: 'reviveCurrentTaskSusp');
  //  It does not take into account 'solve threads', i.e. it must 
  // be done externally - if needed;
  void suspendPropagator ();

  //
  //  Terminate a propagator thread which is (still) marked as propagated
  // (was: 'killPropagatedCurrentTaskSusp' with some variations);
  //
  //  This might be used only from the local propagation queue,
  // because it doesn't check for entaiment, stability, etc. 
  // Moreover, such threads are NOT counted in solve actors
  // and are not marked as "inSolve" EVEN in the "running" state!
  //
  //  Philosophy (am i right, Tobias?):
  // When some propagator returns 'PROCEED' and still has the 
  // 'propagated' flag set, then it's done.
  void closeDonePropagator ();

  // 
  //  Takes a propagator thread and makes out of it a usual 
  // runnable thread with an empty task stack;
  //  NOTE: This could be used, but very carefully 'cause there can be 
  // *other* references to the thread!
  // .ps  Right now it's not used;
  // void prop2generic ();

  // wake up cconts and board conts
  Bool wakeUp (Board *home, PropCaller calledBy);
};


#ifndef OUTLINE
#include "thread.icc"
#endif

#endif
