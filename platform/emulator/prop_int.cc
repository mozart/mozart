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

#include "prop_int.hh"
#include "os.hh"
#include "value.hh"
#include "suspendable.hh"

inline
void oz_resetLocalPropagatorQueue(Board * bb) {
  SuspQueue *lpq = bb->getLocalPropagatorQueue();
  if (!lpq)
    return;

  oz_currentThread()->getTaskStackRef()->makeEmpty();
  lpq->dispose();
  bb->setLocalPropagatorQueue(NULL);
}

SuspList * oz_installPropagators(SuspList * local_list, SuspList * glob_list,
                                 Board * glob_home)
{
  Assert((local_list && glob_list && (local_list != glob_list)) ||
         !local_list || !glob_list);

  SuspList * aux = local_list, * ret_list = local_list;


  // mark up local suspensions to avoid copying them
  while (aux) {
    aux->getSuspendable()->setTagged();
    aux = aux->getNext();
  }

  glob_home = glob_home->derefBoard();

  // create references to suspensions of global variable
  aux = glob_list;
  while (aux) {
    Suspendable * susp = aux->getSuspendable();

    /* NOTE: a possible optimization isTaggedAndUntag (TMUELLER) */

    if (!susp->isDead() &&
        susp->isPropagator() &&
        !susp->isTagged() &&
        oz_isBetween(susp->getBoardInternal(), glob_home) == B_BETWEEN) {
      ret_list = new SuspList(susp, ret_list);
    }

    aux = aux->getNext();
  }

  // unmark local suspensions
  aux = local_list;
  while (aux) {
    aux->getSuspendable()->unsetTagged();
    aux = aux->getNext();
  }

  return ret_list;
}




// Builtin that runs the propagators

OZ_BI_define(BI_prop_lpq, 0, 0) {

  Board * bb = oz_currentBoard();

  SuspQueue * lpq = bb->getLocalPropagatorQueue();

  if (lpq == NULL)
    return PROCEED;

  unsigned int starttime = 0;

  if (ozconf.timeDetailed)
    starttime = osUserTime();

  while (!lpq->isEmpty() && !am.isSetSFlag()) {
    Propagator * prop = SuspToPropagator(lpq->dequeue());
    Propagator::setRunningPropagator(prop);
    Assert(!prop->isDead());

    OZ_Return r = oz_runPropagator(prop);

    if (r == SLEEP) {
      oz_sleepPropagator(prop);
    } else if (r == PROCEED) {
      oz_closeDonePropagator(prop);
    } else if (r == FAILED) {

#ifdef NAME_PROPAGATORS
      // this is experimental: a top-level failure with set
      // property 'internal.propLocation',
      if (am.isPropagatorLocation()) {
        if (!am.hf_raise_failure()) {
          if (ozconf.errorDebug)
            am.setExceptionInfo(OZ_mkTupleC("apply",2,
                                            OZ_atom((prop->getPropagator()->getProfile()->getPropagatorName())),
                                            prop->getPropagator()->getParameters()));
          oz_sleepPropagator(prop);
          prop->setFailed();
          oz_resetLocalPropagatorQueue(bb);
          return RAISE;
        }
      }
#endif

      if (ozconf.timeDetailed)
        ozstat.timeForPropagation.incf(osUserTime()-starttime);

      // check for top-level and if not, prepare raising of an
      // exception (`hf_raise_failure()')
      if (am.hf_raise_failure()) {
        oz_closeDonePropagator(prop);
        return FAILED;
      }

      if (ozconf.errorDebug)
        am.setExceptionInfo(OZ_mkTupleC("apply",2,
                                        OZ_atom((prop->getPropagator()->getProfile()->getPropagatorName())),
                                        prop->getPropagator()->getParameters()));

      oz_closeDonePropagator(prop);

      oz_resetLocalPropagatorQueue(bb);

      return RAISE;

    } else {
      Assert(r == SCHEDULED);
      oz_preemptedPropagator(prop);
    }
    Assert(prop->isDead() || !prop->isRunnable());
  }

  if (ozconf.timeDetailed)
    ozstat.timeForPropagation.incf(osUserTime()-starttime);

  if (lpq->isEmpty()) {
    oz_resetLocalPropagatorQueue(bb);
    return PROCEED;
  } else {
    am.prepareCall(BI_PROP_LPQ, (RefsArray) NULL);
    return BI_REPLACEBICALL;
  }

} OZ_BI_end

SuspQueue * oz_pushToLPQ(Propagator * prop) {

  Board * bb = prop->getBoardInternal()->derefBoard();

  SuspQueue * lpq = bb->getLocalPropagatorQueue();

  if (!lpq) {
    // Create new thread
    Thread * thr = oz_newThreadInject(bb);

    // Push run lpq builtin
    thr->pushCall(BI_PROP_LPQ, 0, 0);

    lpq = new SuspQueue();

    bb->setLocalPropagatorQueue(lpq);
  }

  lpq->enqueue(prop);

  return lpq;

}
