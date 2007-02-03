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
Propagator * oz_newPropagator(OZ_Propagator * p) {
  Board * bb = oz_currentBoard();
  bb->incSuspCount();
  Propagator * prop = new Propagator(p, bb);
  prop->setRunnable();
  prop->setUnify();
  prop->setActive();

  if (!p->isMonotonic())
    prop->setNMO();

  /* checkDebug(th,bb); tmueller ask BENNI */

  return prop;
}

inline
void oz_closeDonePropagator(Propagator * prop) {
  Assert(prop);
  Assert(!prop->isDead());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  GETBOARD(prop)->decSuspCount();
  prop->dispose();
  prop->setDead();
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

#ifdef COUNT_PROP_INVOCS
extern int count_prop_invocs_run;
#endif 

inline
OZ_Return oz_runPropagator(Propagator * p) {

#ifdef COUNT_PROP_INVOCS
  count_prop_invocs_run += 1;
#endif 

  if (!p->isActive())
    return OZ_SLEEP;

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

    OZ_Return ret = ozprop->propagate();

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
    printf("<%s[%p] %s", pn, ozprop, OZ_toC(params, ozconf.errorPrintDepth, ozconf.errorPrintWidth));
    fflush(stdout);
    //
    OZ_Return ret = ozprop->propagate();
    printf(" -> %s (%s)>\n\n", RETVAL_TEXT(ret), OZ_toC(params, ozconf.errorPrintDepth, ozconf.errorPrintWidth)); 
    fflush(stdout);
    return ret;
#else
    return ozprop->propagate();
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

inline
void oz_sleepPropagator(Propagator * prop) {
  Assert(prop);
  Assert(!prop->isDead());
  Assert(oz_isCurrentBoard(GETBOARD(prop)));

  prop->unsetRunnable();
  prop->unsetUnify();
}

#endif
