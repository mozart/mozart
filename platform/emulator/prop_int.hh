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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __PROPAGATORHH
#define __PROPAGATORHH

#include "base.hh"
#include "thr_class.hh"
#include "thr_int.hh"
#include "am.hh"
#include "space.hh"
#include "debug.hh"
#include "cpi_heap.hh"
#include "var_base.hh"
#include "lps.hh"

// exports
Thread * oz_mkLPQ(Board *bb, int prio);
Propagator * oz_mkPropagator(Board *bb, OZ_Propagator *pro);
void oz_pushToLPQ(Board *bb, Propagator * prop);
Bool oz_wakeup_Propagator(Propagator * prop, Board * home, PropCaller calledBy);


inline
Propagator * oz_newPropagator(OZ_Propagator * p)
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
  CheckExtSuspension(prop);

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
  CheckExtSuspension(prop);
}

inline
void oz_closeDonePropagatorCD(Propagator * prop)
{
  prop->markRunnable();
  prop->dispose();
  prop->markDeadPropagator();
  oz_currentBoard()->decSuspCount();
  CheckExtSuspension(prop);
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
void oz_preemptedPropagator(Propagator * prop)
{
  Assert(prop);
  Assert(!prop->isDeadPropagator());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  prop->unmarkRunnable();
  oz_wakeup_Propagator(prop, oz_currentBoard(), pc_propagator);
}

//
//  (re-)Suspend a propagator again; (was: 'reviveCurrentTaskSusp');
//  It does not take into account 'solve threads', i.e. it must
// be done externally - if needed;
//
//  (re-)Suspend the propagator again (former 'reviveCurrentTaskSusp');
inline
void oz_sleepPropagator(Propagator * prop)
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

  lpq->getLPQThread()->getTaskStackRef()->makeEmpty();
  lpq->dispose();
  bb->setLocalPropagatorQueue(NULL);
}

#endif
