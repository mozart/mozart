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
    CheckExtSuspension(prop);                   \
    prop->dispose();                            \
    return TRUE;                                \
                                                \
  default:                                      \
    Assert(0);                                  \
    return FALSE;                               \
  }                                             \
}

Bool oz_wakeup_Propagator(Propagator * prop, Board * home, PropCaller calledBy)
{
  Assert(prop->getBoardInternal() && prop->getPropagator());

  Board *cb_cache = oz_currentBoard();

  if (prop->isNonMonotonicPropagator() && !oz_onToplevel()) {
#ifdef DEBUG_NONMONOTONIC
    OZ_PropagatorProfile * profile = prop->getPropagator()->getProfile();
    char * pn = profile->getPropagatorName();
    printf("wakeup_Propagator: nonmono prop <%s %d>\n",
           pn,
           prop->getPropagator()->getOrder());
    fflush(stdout);
#endif

    Assert(!prop->getPropagator()->isMonotonic());

    WAKEUP_PROPAGATOR(prop->markRunnable();
                      am.currentBoard()->addToNonMono(prop));
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

//  Make a runnable thread with a single task stack entry <local thread queue>
Thread * oz_mkLPQ(Board *bb, int prio)
{
  Thread * th = new Thread(S_RTHREAD|T_runnable|T_lpq, prio, bb, oz_newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();
  oz_checkDebug(th,bb);

  if (!bb->isRoot())
    bb->incSolveThreads();

  th->pushLPQ(bb);

  return th;
}


SuspList * oz_installPropagators(SuspList * local_list, SuspList * glob_list,
                                 Board * glob_home)
{
  Assert((local_list && glob_list && (local_list != glob_list)) ||
         !local_list || !glob_list);

  SuspList * aux = local_list, * ret_list = local_list;


  // mark up local suspensions to avoid copying them
  while (aux) {
    aux->getSuspension().markTagged();
    aux = aux->getNext();
  }

  // create references to suspensions of global variable
  aux = glob_list;
  while (aux) {
    Suspension susp = aux->getSuspension();

    /* NOTE: a possible optimization isTaggedAndUntag (TMUELLER) */

    if (!susp.isDead() &&
        susp.isPropagator() &&
        !susp.isTagged() &&
        oz_isBetween(GETBOARDOBJ(susp), glob_home) == B_BETWEEN) {
      ret_list = new SuspList(susp, ret_list);
    }

    aux = aux->getNext();
  }

  // unmark local suspensions
  aux = local_list;
  while (aux) {
    aux->getSuspension().unmarkTagged();
    aux = aux->getNext();
  }

  return ret_list;
}

void oz_pushToLPQ(Board *bb, Propagator * prop)
{
  LocalPropagatorQueue *lpq = bb->getLocalPropagatorQueue();
  if (lpq) {
    lpq->enqueue(prop);
  } else {
    Thread * lpq_thr = oz_mkLPQ(bb, DEFAULT_PRIORITY);
    bb->setLocalPropagatorQueue(new LocalPropagatorQueue(lpq_thr, prop));
    am.threadsPool.scheduleThreadInline(lpq_thr, DEFAULT_PRIORITY);
  }
}
