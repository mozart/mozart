/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

inline
void oz_checkExtSuspension(Suspension susp);
void oz_checkExtSuspension(Suspension susp, Board * home);
void oz_checkExtSuspensionOutlined(Suspension susp);
void oz_removeExtThreadOutlined(Thread *tt);
Thread * oz_mkLPQ(Board *bb, int prio);
Propagator * oz_mkPropagator(Board *bb, int prio, OZ_Propagator *pro);
void oz_pushToLPQ(Board *bb, Propagator * prop);
void oz_solve_scheduleNonMonoSuspList(SolveActor *sa);

/* -------------------------------------------------------------------------
 * Suspension lists
 * ------------------------------------------------------------------------- */

void oz_wakeupAll(SVariable *sv);

SuspList * oz_checkAnySuspensionList(SuspList *suspList,Board *home,
			  PropCaller calledBy);

// mm2: maybe #define this???
inline
void oz_checkSuspensionList(SVariable *var,
		       PropCaller calledBy=pc_propagator)
{
  var->setSuspList(oz_checkAnySuspensionList(var->getSuspList(),
					     GETBOARD(var),calledBy));
}

/* -------------------------------------------------------------------------
 * Threads
 * ------------------------------------------------------------------------- */

static
inline
Thread * _newThread(int prio, Board *bb) {
  Thread *th = new Thread(S_RTHREAD | T_runnable,prio,bb,am.newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();
  oz_checkDebug(th,bb);
  return th;
}

inline
Thread * oz_newThread(int prio=DEFAULT_PRIORITY)
{
  Board *bb=oz_currentBoard();
  Thread *tt = _newThread(prio,bb);

  if (am.isBelowSolveBoard()) {
    int inSolve=oz_incSolveThreads(bb);
    Assert(inSolve);
    tt->setInSolve();
  } else {
    Assert(!oz_isInSolveDebug(bb));
  }
  am.threadsPool.scheduleThread(tt);
  return tt;
}


inline
Thread * oz_newThreadToplevel(int prio=DEFAULT_PRIORITY)
{
  Board *bb=oz_rootBoard();
  Thread *tt = _newThread(prio,bb);

  am.threadsPool.scheduleThread(tt);
  return tt;
}

inline
Thread * oz_newThreadInject(int prio,Board *bb)
{
  Thread *tt = _newThread(prio,bb);

  int inSolve = oz_incSolveThreads(bb);
  if (inSolve) {
    tt->setInSolve();
  }
  am.threadsPool.scheduleThread(tt);
  return tt;
}


inline
Thread * oz_newThreadSuspended(int prio=DEFAULT_PRIORITY)
{
  Board *bb = oz_currentBoard();
  Thread *th = new Thread(S_RTHREAD,prio,bb,am.newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();

  oz_checkDebug(th,bb);

  return th;
}

inline
Thread * oz_newThreadPropagate(Board *bb) 
{
  Thread *th = new Thread(S_WAKEUP,DEFAULT_PRIORITY,bb,am.newId());
  bb->incSuspCount();
  oz_checkDebug(th,bb);
  return th;
}


//  Dispose a thread.
inline
void oz_disposeThread(Thread *tt)
{
#ifdef DEBUG_THREADCOUNT
  if (tt->isRunnable() && tt->isLPQThread())
    existingLTQs -= 1;
#endif
  Assert(!tt->isSuspended() || !GETBOARD(tt)->checkAlive());

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

inline
static
void _wakeupThread(Thread *tt)
{
  Assert(tt->isSuspended());

  tt->markRunnable();

  if (am.debugmode() && tt->getTrace()) {
    //Thread *t; if ((t = oz_currentThread()) && t->isTraced())
    //  execBreakpoint(t);
    debugStreamReady(tt);
  }
  am.threadsPool.scheduleThread(tt);
}

inline
void oz_removeExtThread(Thread *tt) 
{
  if (tt->wasExtThread()) {
    oz_removeExtThreadOutlined(tt);
  }
}

inline 
void oz_wakeupThreadOPT(Thread *tt)
{
  _wakeupThread(tt);

  Assert(oz_isCurrentBoard(GETBOARD(tt)) || tt->isExtThread() ||
	 (oz_isCurrentBoard(GETBOARD(tt)->getParent())
	  && !GETBOARD(tt)->isSolve()));

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
void oz_wakeupThread(Thread *tt)
{
  _wakeupThread(tt);
  
  int inSolve = oz_incSolveThreads(GETBOARD(tt));
  if (inSolve) {
    tt->setInSolve();
    oz_removeExtThread(tt);
    tt->clearExtThread();
  }
}

/* -------------------------------------------------------------------------
 * Propagators
 * ------------------------------------------------------------------------- */

inline
Propagator * oz_newPropagator(int prio, OZ_Propagator * p)
{
  Board * bb = oz_currentBoard();
  Propagator * prop = new Propagator(p, bb);
  prop->markRunnable();
  prop->markUnifyPropagator();

  if (! p->isMonotonic()) 
    prop->markNonMonotonicPropagator();

  bb->incSuspCount();

  /* checkDebug(th,bb); TMUELLER ask BENNI */

  return prop;
}


inline
void oz_checkExtSuspension(Suspension susp) 
{
  if (susp.wasExtSuspension()) {
    oz_checkExtSuspensionOutlined(susp);
  }
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

  prop->dispose();	// kost@: TODO? optimize;
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


/* -------------------------------------------------------------------------
 * TODO
 * ------------------------------------------------------------------------- */

// EXT STUFF

// WAKEUP

// mm2: outlined
Bool _wakeup_Propagator(Propagator * prop, Board * home, PropCaller calledBy);

inline
void oz_scheduledPropagator(Propagator * prop)
{
  Assert(prop);
  Assert(!prop->isDeadPropagator());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  prop->unmarkRunnable();
  _wakeup_Propagator(prop, oz_currentBoard(), pc_propagator);
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
