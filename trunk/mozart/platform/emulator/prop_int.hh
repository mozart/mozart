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
#include "prop_class.hh"
#include "thr_int.hh"
#include "am.hh"
#include "debug.hh"
#include "cpi_heap.hh"
#include "var_base.hh"

#include "trail.hh"

inline
Propagator * oz_newPropagator(OZ_Propagator * p)
{
  Board * bb = oz_currentBoard();
  Propagator * prop = new Propagator(p, bb);
  prop->setRunnable();
  prop->setUnify();
  prop->setActive();

  if (! p->isMonotonic())
    prop->setNMO();

  bb->incSuspCount();

  /* checkDebug(th,bb); tmueller ask BENNI */

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
  Assert(!prop->isDead());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  prop->dispose();	// kost@: TODO? optimize;
  prop->setDead();

  Board * cb = oz_currentBoard();
  //
  //  Actually, the current board can be alive or not - 
  // so, in the last case it's redundant;
  cb->decSuspCount();

  //
  //  ... again to the 'SolveActor::checkExtSuspList':
  // there is a limitation in the implementation that no stability
  // can be achieved before a propagator on a global variable(s) 
  // completely disappears. Therefore, we make the check here;
  if (prop->isExternal())
    cb->checkSolveThreads();

  //
  //  An ESSENTIAL invariant:
  //  If entailment/whatever is reached somewhere, that's a bad news.
  //  I don't know how to check this assertion right now;
}

#ifdef DEBUG_PROPAGATORS

#define RETVAL_TEXT(R) 							\
(R == OZ_FAILED 							\
 ? "FAILED" 								\
 : (R == OZ_ENTAILED 							\
    ? "ENTAILED"							\
    : (R == SUSPEND 							\
       ? "SUSPEND"							\
       : (R == SLEEP							\
	  ? "SLEEP"							\
	  : (R == SCHEDULED 						\
	     ? "SCHEDULED"						\
	     : (R == RAISE ? "RAISE" : "UNXEPECTED RETVAL"))))))

#endif

inline
OZ_Return oz_runPropagator(Propagator * p)
{
  ozstat.propagatorsInvoked.incf();

  CpiHeap.reset();

  OZ_Propagator * ozprop = p->getPropagator();

  if (am.profileMode()) {
    OZ_PropagatorProfile * prop = ozprop->getProfile();
    ozstat.enterProp(prop);
    int heapNow = getUsedMemoryBytes();

#ifdef DEBUG_PROPAGATORS
    OZ_PropagatorProfile * profile = prop;
    Assert(profile);
    //
    char * pn = profile->getPropagatorName();
    printf("<%s[%p]", pn, ozprop); fflush(stdout);
#endif

    OZ_Return ret = (p->isActive() ? ozprop->propagate() : OZ_SLEEP);

#ifdef DEBUG_PROPAGATORS
    printf(" -> %s>\n\n", RETVAL_TEXT(ret)); fflush(stdout);
#endif

    int heapUsed = getUsedMemoryBytes() - heapNow;
    prop->incHeap(heapUsed);
    ozstat.leaveProp();
    if (ozstat.currAbstr)
      ozstat.currAbstr->getProfile()->heapUsed -= heapUsed;
    return ret;
  } else {

#ifdef DEBUG_PROPAGATORS
    OZ_PropagatorProfile * profile = ozprop->getProfile();
    OZ_Term params = ozprop->getParameters();
    Assert(profile);
    //    
    char * pn = profile->getPropagatorName();
    printf("<%s[%p] %s", pn, ozprop, OZ_toC(params, 10, 10));
    fflush(stdout);
    //
    OZ_Return ret = (p->isActive() ? ozprop->propagate() : OZ_SLEEP);
    printf(" -> %s (%s)>\n\n", RETVAL_TEXT(ret), OZ_toC(params, 10, 10)); 
    fflush(stdout);
    return ret;
#else
    return (p->isActive() ? ozprop->propagate() : OZ_SLEEP);
#endif
  }
}

inline
void oz_preemptedPropagator(Propagator * prop) {
  Assert(prop);
  Assert(!prop->isDead());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  Assert(prop->isRunnable());

  if (prop->isNMO() && !oz_onToplevel()) {
    oz_currentBoard()->addToNonMono(prop);
  } else {
    oz_currentBoard()->addToLPQ(prop);
  }

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
  Assert(!prop->isDead());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  prop->unsetRunnable();
  prop->unsetUnify();
}

#endif
