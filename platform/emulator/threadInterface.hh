/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __TIHH
#define __TIHH

#include "base.hh"
#include "thread.hh"
#include "am.hh"
#include "space.hh"
#include "debug.hh"
#include "cpi_heap.hh"
#include "variable.hh"
#include "lps.hh"

// exported functions

inline void oz_checkExtSuspension(Suspension susp);
void oz_checkExtSuspension(Suspension susp, Board * home);
void oz_checkExtSuspensionOutlined(Suspension susp);
inline void oz_removeExtThread(Thread *tt);
void oz_removeExtThreadOutlined(Thread *tt);
inline Thread *oz_mkRunnableThread(int prio, Board *bb);
inline Thread *oz_mkRunnableThreadOPT(int prio, Board *bb);
Thread *oz_mkLPQ(Board *bb, int prio);
Propagator * oz_mkPropagator(Board *bb, int prio, OZ_Propagator *pro);
inline Thread *oz_mkSuspendedThread(Board *bb, int prio);
inline Thread *oz_mkWakeupThread(Board *bb);
void oz_pushToLPQ(Board *bb, Propagator * prop);
inline void oz_suspThreadToRunnableOPT(Thread *tt);
inline void oz_suspThreadToRunnable(Thread *tt);
inline void oz_wakeupToRunnable(Thread *tt);
inline void oz_propagatorToRunnable(Thread *tt);
inline void oz_updateSolveBoardPropagatorToRunnable(Thread *tt);
void oz_solve_scheduleNonMonoSuspList(SolveActor *sa);
inline void oz_suspendPropagator(Propagator *);
inline void oz_scheduledPropagator(Propagator *);
inline void oz_closeDonePropagator(Propagator *);
inline void oz_closeDonePropagatorCD(Propagator *);
inline void oz_closeDonePropagatorThreadCD(Propagator *);
inline void oz_disposeSuspendedThread(Thread *tt);
inline void oz_disposeRunnableThread(Thread *tt);
inline OZ_Return oz_runPropagator(Propagator *);
SuspList *oz_checkAnySuspensionList(SuspList *suspList,Board *home,
                                    PropCaller calledBy);
void oz_wakeupAny(Suspension susp, Board * bb);
inline Bool oz_wakeUp(Suspension susp, Board * home, PropCaller calledBy);
inline Bool oz_wakeUpPropagator(Propagator * prop, Board *home,
                        PropCaller calledBy = pc_propagator);
inline Bool oz_wakeUpBoard(Thread *tt, Board *home);
inline Bool oz_wakeUpThread(Thread *tt, Board *home);


/* -------------------------------------------------------------------------
 * inlines
 * ------------------------------------------------------------------------- */

static
inline
Thread *_newThread(int prio, Board *bb) {
  Thread *th = new Thread(S_RTHREAD | T_runnable,prio,bb,am.newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();
  oz_checkDebug(th,bb);
  return th;
}

//
//  Make a runnable thread with a task stack;
inline
Thread *oz_mkRunnableThread(int prio, Board *bb)
{
  Thread *th = _newThread(prio,bb);

  int inSolve = oz_incSolveThreads(bb);
  if (inSolve) {
    th->setInSolve();
  }
  return th;
}

//
//  Make a runnable thread with a task stack;
inline
Thread *oz_mkRunnableThreadOPT(int prio, Board *bb)
{
  Thread *th = _newThread(prio,bb);

  Assert(oz_isCurrentBoard(bb) || oz_isCurrentBoard(bb->getParent()));
  if (am.isBelowSolveBoard()) {
    int inSolve=oz_incSolveThreads(bb);
    Assert(inSolve);
    th->setInSolve();
  } else {
    Assert(!oz_isInSolveDebug(bb));
  }
  return th;
}

//
inline
Propagator * oz_mkPropagator(Board * bb, int prio, OZ_Propagator * p)
{
  Propagator * prop = new Propagator(p, bb);
  prop->markRunnable();
  prop->markUnifyPropagator();

  if (! p->isMonotonic())
    prop->markNonMonotonicPropagator();

  bb->incSuspCount();

  /* checkDebug(th,bb); TMUELLER ask BENNI */

  return prop;
}

//
inline
Thread *oz_mkSuspendedThread(Board *bb, int prio)
{
  Thread *th = new Thread(S_RTHREAD,prio,bb,am.newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();

  oz_checkDebug(th,bb);

  return th;
}

//
//  Constructors for 'suspended' cases:
//    deep 'unify' suspension;
//    suspension with continuation;
//    suspension with a 'C' function;
//    suspended sequential thread (with a task stack);
//
inline
// static
Thread *oz_mkWakeupThread(Board *bb)
{
  Thread *th = new Thread(S_WAKEUP,DEFAULT_PRIORITY,bb,am.newId());
  bb->incSuspCount();
  oz_checkDebug(th,bb);
  return th;
}

inline
void oz_suspThreadToRunnableOPT(Thread *tt)
{
  Assert(tt->isSuspended());

  tt->markRunnable();

  Assert(oz_isCurrentBoard(GETBOARD(tt)) || tt->isExtThread() ||
         (oz_isCurrentBoard(GETBOARD(tt)->getParent())
          && !GETBOARD(tt)->isSolve()));

  if (am.debugmode() && tt->getTrace()) {
    //Thread *t; if ((t = oz_currentThread()) && t->isTraced())
    //  execBreakpoint(t);
    debugStreamReady(tt);
  }

  if (am.isBelowSolveBoard() || tt->isExtThread()) {
    Assert(oz_isInSolveDebug(GETBOARD(tt)));
    oz_incSolveThreads(GETBOARD(tt));
    tt->setInSolve();
    oz_removeExtThread(tt);
    tt->clearExtThread();
  } else {
    Assert(!oz_isInSolveDebug(GETBOARD(tt)));
  }
}

inline
void oz_suspThreadToRunnable(Thread *tt)
{
  Assert(tt->isSuspended());

  tt->markRunnable();

  if (am.debugmode() && tt->getTrace()) {
    //Thread *t; if ((t = oz_currentThread()) && t->isTraced())
    //  execBreakpoint(t);
    debugStreamReady(tt);
  }

  int inSolve = oz_incSolveThreads(GETBOARD(tt));
  if (inSolve) {
    tt->setInSolve();
    oz_removeExtThread(tt);
    tt->clearExtThread();
  }
}

inline
void oz_updateSolveBoardPropagatorToRunnable(Thread *tt)
{
  if (am.isBelowSolveBoard() || tt->isExtThread()) {
    Assert(oz_isInSolveDebug(GETBOARD(tt)));
    oz_incSolveThreads(GETBOARD(tt));
    tt->setInSolve();
  } else {
    Assert(!oz_isInSolveDebug(GETBOARD(tt)));
  }
}

inline
void oz_propagatorToRunnable(Thread *tt)
{
  tt->markRunnable();

  oz_updateSolveBoardPropagatorToRunnable(tt);
}


//  Dispose a thread.
inline
static
void oz_disposeThreadInternal(Thread *tt)
{
  tt->markDeadThread();

  if (am.debugmode() && tt->getTrace())
    debugStreamTerm(tt);

  switch (tt->getThrType()) {
  case S_RTHREAD:
    am.threadsPool.freeThreadBody(tt);
    break;

  case S_WAKEUP:
    break;

  default:
    Assert(0);
  }
}

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
inline
void oz_disposeSuspendedThread(Thread *tt)
{
  Assert(tt->isSuspended());
  Assert(!GETBOARD(tt)->checkAlive());

  oz_disposeThreadInternal(tt);
}

//  It marks the thread as dead and disposes it;
//
//  It marks also the thread as dead;
inline
void oz_disposeRunnableThread(Thread *tt)
{
#ifdef DEBUG_THREADCOUNT
  if (tt->isLPQThread())
    existingLTQs -= 1;
#endif

  Assert(tt->isRunnable());

  oz_disposeThreadInternal(tt);
}

//
//  Terminate a propagator thread which is (still) marked as runnable
// (was: 'killPropagatedCurrentTaskSusp' with some variations);
//
//  This might be used only from the local propagation queue,
// because it doesn't check for entaiment, stability, etc.
// Moreover, such threads are NOT counted in solve actors
// and are not marked as "inSolve" EVEN in the "running" state!
//
//  Philosophy (am i right, Tobias?):
// When some propagator returns 'PROCEED' and still has the
// 'runnable' flag set, then it's done.
//
//  Close up a propagator which is done;
//  Actually, terminating a thread should be done through
// 'LBLkillxxxx' in emulate.cc, but one cannot jump from the local
// propagation queue there;
inline
void oz_closeDonePropagator(Propagator * prop)
{
  Assert(prop);
  Assert(!prop->isDeadPropagator());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  // constructive disjunction ???
  // kost@: i don't know what's going on here - just keep it as it was;
  //if (isSuspended ())
  //return;

  prop->dispose();      // kost@: TODO? optimize;
  prop->markDeadPropagator();

  //
  //  Actually, the current board can be alive or not -
  // so, in the last case it's redundant;
  oz_currentBoard()->decSuspCount();

  //
  //  ... again to the 'SolveActor::checkExtSuspList':
  // there is a limitation in the implementation that no stability
  // can be achieved before a propagator on a global variable(s)
  // completely disappears. Therefore, we make the check here;
  oz_checkExtSuspension(prop);

  //
  //  An ESSENTIAL invariant:
  //  If entailment/whatever is reached somewhere, that's a bad news.
  //  I don't know how to check this assertion right now;
}

inline
void oz_closeDonePropagatorThreadCD(Propagator * prop)
{
  prop->markRunnable();
  oz_currentBoard()->decSuspCount();
  prop->markDeadPropagator();
  oz_checkExtSuspension(prop);
}

inline
void oz_closeDonePropagatorCD(Propagator * prop)
{
  prop->markRunnable();
  prop->dispose();
  prop->markDeadPropagator();
  oz_currentBoard()->decSuspCount();
  oz_checkExtSuspension(prop);
}

inline
OZ_Return oz_runPropagator(Propagator * p)
{
  ozstat.propagatorsInvoked.incf();

  CpiHeap.reset();

  if (am.profileMode()) {
    OZ_PropagatorProfile * prop = p->getPropagator()->getProfile();
    ozstat.enterProp(prop);
    int heapNow = getUsedMemoryBytes();
#ifdef DEBUG_PROPAGATORS
    OZ_PropagatorProfile * profile = prop;
    if (profile) {
      char * pn = profile->getPropagatorName();
      printf("<%s", pn); fflush(stdout);
    } else {
      printf("<CDSuppl"); fflush(stdout);
    }
#endif
    OZ_Return ret = p->getPropagator()->propagate();
#ifdef DEBUG_PROPAGATORS
    printf(">\n"); fflush(stdout);
#endif
    int heapUsed = getUsedMemoryBytes() - heapNow;
    prop->incHeap(heapUsed);
    ozstat.leaveProp();
    if (ozstat.currAbstr)
      ozstat.currAbstr->heapUsed -= heapUsed;
    return ret;
  } else {
#ifdef DEBUG_PROPAGATORS
    OZ_PropagatorProfile * profile = p->getPropagator()->getProfile();
    if (profile) {
      char * pn = profile->getPropagatorName();
      printf("<%s", pn); fflush(stdout);
    } else {
      printf("<CDSuppl"); fflush(stdout);
    }
    OZ_Return ret = p->getPropagator()->propagate();
    printf(">\n"); fflush(stdout);
    return ret;
#else
    return p->getPropagator()->propagate();
#endif
  }
}

inline
void oz_checkSuspensionList(SVariable *var,
                            PropCaller calledBy=pc_propagator)
{
  var->setSuspList(oz_checkAnySuspensionList(var->getSuspList(),
                                             GETBOARD(var),calledBy));
}

// EXT STUFF

inline
void oz_checkExtSuspension(Suspension susp)
{
  if (susp.wasExtSuspension()) {
    oz_checkExtSuspensionOutlined(susp);
  }
}

inline
void oz_removeExtThread(Thread *tt)
{
  if (tt->wasExtThread()) {
    oz_removeExtThreadOutlined(tt);
  }
}

// WAKEUP

#define WAKEUP_PROPAGATOR(CALL_WAKEUP_FUN)      \
{                                               \
  Board * bb = GETBOARD(prop);                  \
  switch (oz_isBetween(bb, home)) {             \
  case B_BETWEEN:                               \
                                                \
    if (calledBy)                               \
      prop->markUnifyPropagator();              \
                                                \
    CALL_WAKEUP_FUN;                            \
    return FALSE;                               \
                                                \
  case B_NOT_BETWEEN:                           \
    return FALSE;                               \
                                                \
  case B_DEAD:                                  \
    prop->markDeadPropagator();                 \
    oz_checkExtSuspension(prop);                \
    prop->dispose();                            \
    return TRUE;                                \
                                                \
  default:                                      \
    Assert(0);                                  \
    return FALSE;                               \
  }                                             \
}

inline
Bool oz_wakeUpPropagator(Propagator * prop, Board * home, PropCaller calledBy)
{
  Assert(prop->getBoardInternal() && prop->getPropagator());

  Board *cb_cache = oz_currentBoard();

  if (prop->isNonMonotonicPropagator() && am.isBelowSolveBoard()) {
#ifdef DEBUG_NONMONOTONIC
    OZ_PropagatorProfile * profile = prop->getPropagator()->getProfile();
    char * pn = profile->getPropagatorName();
    printf("wakeUpPropagator: nonmono prop <%s %d>\n",
           pn,
           prop->getPropagator()->getOrder());
    fflush(stdout);
#endif

    Assert(!prop->getPropagator()->isMonotonic());

    WAKEUP_PROPAGATOR(prop->markRunnable();
                      SolveActor::Cast(am.currentSolveBoard()->getActor())->addToNonMonoSuspList(prop));
  }

  if (localPropStore.isUseIt()) {
    Assert(GETBOARD(prop) == cb_cache);
    prop->markRunnable();
    localPropStore.push(prop);
    return FALSE;
  }

  WAKEUP_PROPAGATOR(prop->markRunnable();
                    oz_pushToLPQ(GETBOARD(prop),prop));
}

inline
void oz_scheduledPropagator(Propagator * prop)
{
  Assert(prop);
  Assert(!prop->isDeadPropagator());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  prop->unmarkRunnable();
  oz_wakeUpPropagator(prop, oz_currentBoard(), pc_propagator);
}

//
//  (re-)Suspend a propagator again; (was: 'reviveCurrentTaskSusp');
//  It does not take into account 'solve threads', i.e. it must
// be done externally - if needed;
//
//  (re-)Suspend the propagator again (former 'reviveCurrentTaskSusp');
inline
void oz_suspendPropagator(Propagator * prop)
{
  Assert(prop);
  Assert(!prop->isDeadPropagator());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  prop->unmarkRunnable();
  prop->unmarkUnifyPropagator();
}

inline
Thread *oz_newSuspendedThread()
{
  return oz_mkSuspendedThread(oz_currentBoard(), DEFAULT_PRIORITY);
}

inline
Thread *oz_newRunnableThread(int prio=DEFAULT_PRIORITY)
{
  Thread *tt = oz_mkRunnableThreadOPT(prio, oz_currentBoard());
  am.threadsPool.scheduleThread(tt);
  return tt;
}

inline
void oz_resetLocalPropagatorQueue(Board *bb) {
  LocalPropagatorQueue *lpq = bb->getLocalPropagatorQueue();
  if (!lpq)
    return;

#ifdef DEBUG_THREADCOUNT
  //  existingLTQs -= 1;
  //  printf("-LTQ=%p\n", localPropagatorQueue); fflush(stdout);

  //  printf("-");fflush(stdout);
  if (am.isBelowSolveBoard()) {
;    //printf("!");fflush(stdout);
  }
#endif

  lpq->getLPQThread()->getTaskStackRef()->makeEmpty();
  lpq->dispose();
  bb->setLocalPropagatorQueue(NULL);
}

#endif
