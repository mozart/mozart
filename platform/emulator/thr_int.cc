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

#include "threadInterface.hh"

/* -------------------------------------------------------------------------
 * Suspension lists
 * ------------------------------------------------------------------------- */

inline
static
Bool wakeup_Thread(Thread * tt, Board *home)
{
  Assert (tt->isSuspended());
  Assert (tt->isRThread());

  switch (oz_isBetween(GETBOARD(tt), home)) {
  case B_BETWEEN:
    oz_wakeupThreadOPT(tt);
    return TRUE;

  case B_NOT_BETWEEN:
    return FALSE;

  case B_DEAD:
    //
    //  The whole thread is eliminated - because of the invariant
    // stated just before 'disposeThread ()' in thread.hh;
    tt->markDeadThread();
    oz_checkExtSuspension(tt);
    am.threadsPool.freeThreadBody(tt);
    return TRUE;

  default:
    Assert(0);
    return FALSE;
  }
}

inline
static
void wakeup_Wakeup(Thread *tt)
{
  Assert(tt->isSuspended());

  tt->markRunnable();
  am.threadsPool.scheduleThread(tt);

  if (am.isBelowSolveBoard() || tt->isExtThread()) {
    Assert (oz_isInSolveDebug(GETBOARD(tt)));
    oz_incSolveThreads(GETBOARD(tt));
    tt->setInSolve();
  } else {
    Assert(!oz_isInSolveDebug(GETBOARD(tt)));
  }
}

inline
static
Bool wakeup_Board(Thread *tt, Board *home)
{
  Assert(tt->isSuspended());
  Assert(tt->getThrType() == S_WAKEUP);

  //
  //  Note:
  //  We use here the dereferenced board pointer, because:
  // - normally, there should be a *single* "wakeup" suspension
  //   per guard (TODO);
  // - when "unit commit" takes place, the rest of (suspended?) threads
  //   from that guard belong to the guard just above
  //   (or toplevel, of course) - we have to update
  //   the threads counter there;
  // - garbage collector moves the pointer anyway.
  //
  //  It's relevant (should be) for unit commits *only*;
  //  Implicitly move the thread upstairs - the threads counter
  // should be already updated before (during unit committing);
  Board *bb=GETBOARD(tt);

  //
  //  Do not propagate to the current board, but discard it;
  //  Do not propagate to the board which has a runnable
  // "wakeup" thread;
  //
  // Note that we don't need to schedule the wakeup for the board
  // because in both cases there is a thread which will check
  // entailment for us;
  if (oz_isCurrentBoard(bb) || bb->isNervous ()) {
#ifdef DEBUG_CHECK
    // because of assertions in decSuspCount and getSuspCount
    if (bb->isFailed()) {
      tt->markDeadThread();
      oz_checkExtSuspension(tt);
      return OK;
    }
#endif
    bb->decSuspCount();

    Assert(bb->getSuspCount() > 0);
    tt->markDeadThread();
    // checkExtThread(); // don't check here !
    return OK;
  }

  //
  //  Don't propagate to the variable's home board (again,
  // this can happen only in the case of unit commit), but we have
  // to schedule a wakeup for the new thread's home board,
  // because it could be the last thread in it - check entailment!
  if (bb == home && bb->getSuspCount() == 1) {
    wakeup_Wakeup(tt);
    return OK;
  }

  //
  //  General case;
  switch (oz_isBetween(bb, home)) {
  case B_BETWEEN:
    Assert(!oz_currentBoard()->isSolve() || am.isBelowSolveBoard());
    wakeup_Wakeup(tt);
    return OK;

  case B_NOT_BETWEEN:
    return NO;

  case B_DEAD:
    tt->markDeadThread();
    oz_checkExtSuspension(tt);
    return OK;

  default:
    Assert(0);
    return NO;
  }
}


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

Bool _wakeup_Propagator(Propagator * prop, Board * home, PropCaller calledBy)
{
  Assert(prop->getBoardInternal() && prop->getPropagator());

  Board *cb_cache = oz_currentBoard();

  if (prop->isNonMonotonicPropagator() && am.isBelowSolveBoard()) {
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

//
//  Generic 'wakeUp';
//  Since this method is used at the only one place, it's inlined;
inline
static
Bool wakeup_Suspension(Suspension susp, Board * home, PropCaller calledBy)
{
  if (susp.isThread()) {
    Thread * tt = susp.getThread();

    switch (tt->getThrType()) {
    case S_RTHREAD:
      return wakeup_Thread(tt,home);
    case S_WAKEUP:
      return wakeup_Board(tt,home);
    default:
      Assert(0);
      return FALSE;
    }
  } else {
    Assert(susp.isPropagator());

    return _wakeup_Propagator(susp.getPropagator(), home, calledBy);
  }
}

static
void wakeup_Suspension_Any(Suspension susp, Board * bb)
{
  if (susp.isThread()) {
    Thread * tt = susp.getThread();

    switch (tt->getThrType()) {
    case S_RTHREAD:
      Assert (tt->isSuspended());
      Assert (tt->isRThread());
      oz_wakeupThread(tt);
      break;
    case S_WAKEUP:
      Assert(tt->isSuspended());
      wakeup_Wakeup(tt);
      break;
    default:
      Assert(0);
    }
  } else {
    Assert(susp.isPropagator());
    // mm2: i'm not sure what i'm doing here
    int ret = _wakeup_Propagator(susp.getPropagator(), bb, pc_std_unif);
    Assert(ret);
  }
}

void oz_wakeupAll(SVariable *sv)
{
  SuspList *sl=sv->getSuspList();
  sv->setSuspList(0);
  while (sl) {
    Suspension susp = sl->getElem();

    if (!susp.isDead() && !susp.isRunnable()) {
      wakeup_Suspension_Any(susp, GETBOARD(sv));
    }
    sl = sl->dispose();
  }
}

SuspList * oz_checkAnySuspensionList(SuspList *suspList,Board *home,
                                     PropCaller calledBy)
{
  if (am.inShallowGuard())
    return suspList;

  SuspList * retSuspList = NULL;

  while (suspList) {
    Suspension susp = suspList->getSuspension();

    if (susp.isDead()) {
      suspList = suspList->dispose();
      continue;
    }

 // already runnable susps remain in suspList
    if (susp.isRunnable()) {
      if (susp.isPropagator()) {
        Propagator * prop = susp.getPropagator();

        if (calledBy && !prop->isUnifyPropagator()) {
          switch (oz_isBetween(GETBOARD(prop), home)) {
          case B_BETWEEN:
            prop->markUnifyPropagator();
            break;
          case B_DEAD:
            //  keep the thread itself alive - it will be discarded
            // *properly* in the emulator;
            suspList = suspList->dispose ();
            continue;
          case B_NOT_BETWEEN:
            break;
          }
        }
      } else {
        //  non-propagator, i.e. it just goes away;
        suspList = suspList->dispose();
        continue;
      }
    } else {
      if (wakeup_Suspension(susp, home, calledBy)) {
        Assert (susp.isDead() || susp.isRunnable());
        suspList = suspList->dispose ();
        continue;
      }
    }

    // susp cannot be woken up therefore relink it
    SuspList * first = suspList;
    suspList = suspList->getNext();
    first->setNext(retSuspList);
    retSuspList = first;
  } // while

  return retSuspList;
}

/* -------------------------------------------------------------------------
 * TODO
 * ------------------------------------------------------------------------- */

//  Make a runnable thread with a single task stack entry <local thread queue>
Thread * oz_mkLPQ(Board *bb, int prio)
{
  Thread * th = new Thread(S_RTHREAD|T_runnable|T_lpq, prio, bb, am.newId());
  th->setBody(am.threadsPool.allocateBody());
  bb->incSuspCount();
  oz_checkDebug(th,bb);
  //Assert(oz_isCurrentBoard(bb));

#ifdef DEBUG_THREADCOUNT
  th->markLPQThread();
#endif

#ifdef DEBUG_THREADCOUNT
  //printf("+");fflush(stdout);
#endif

  if (am.isBelowSolveBoard()) {
#ifdef DEBUG_THREADCOUNT
    //printf("!");fflush(stdout);
#endif
    Assert(oz_isInSolveDebug(bb));
    oz_incSolveThreads(bb);
    th->setInSolve();
  } else {
    Assert(!oz_isInSolveDebug(GETBOARD(th)));
  }

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

#ifdef DEBUG_THREADCOUNT
int existingLTQs = 0;
#endif

void oz_pushToLPQ(Board *bb, Propagator * prop)
{
  LocalPropagatorQueue *lpq = bb->getLocalPropagatorQueue();
  if (lpq) {
    lpq->enqueue(prop);
  } else {
    Thread * lpq_thr = oz_mkLPQ(bb, PROPAGATOR_PRIORITY);
    bb->setLocalPropagatorQueue(new LocalPropagatorQueue(lpq_thr, prop));
    am.threadsPool.scheduleThreadInline(lpq_thr, PROPAGATOR_PRIORITY);
#ifdef DEBUG_THREADCOUNT
    existingLTQs += 1;
    //    printf("+LTQ=%p\n", localPropagatorQueue); fflush(stdout);
#endif
  }
}

void oz_solve_scheduleNonMonoSuspList(SolveActor *sa)
{
#ifdef DEBUG_NONMONOTONIC
  printf("------------------------------------------------------------------"
         "\nSolveActor::scheduleNonMonoSuspList\n"); fflush(stdout);
#endif

  for (OrderedSuspList * p = sa->getNonMonoSuspList();
       p != NULL;
       p = p->getNext()) {
    Propagator * prop = p->getPropagator();

#ifdef DEBUG_NONMONOTONIC
    OZ_PropagatorProfile * header = prop->getPropagator()->getProfile();
    char * pn = header->getPropagatorName();
    printf("<%s %d>\n", pn,
           prop->getPropagator()->getOrder());
#endif

    oz_pushToLPQ(GETBOARD(prop),prop);
  }

  sa->setNonMonoSuspList(NULL);

#ifdef DEBUG_NONMONOTONIC
  printf("Done\n"); fflush(stdout);
#endif
}

static
Bool extParameters(OZ_Term list, Board * solve_board)
{
  while (OZ_isCons(list)) {
    OZ_Term h = OZ_head(list);

    Bool found = FALSE;

    if (OZ_isVariable(h)) {

#ifdef DEBUG_PROP_STABILTY_TEST
      oz_print(h);
#endif

      DEREF(h, hptr, htag);

      Assert(!isUVar(htag));

      Board  * home = GETBOARD(tagged2SVarPlus(h));
      Board * tmp = solve_board;

      // from solve board go up to root; if you step over home
      // then the variable is external otherwise it must be a local one
      do {
        tmp = tmp->getParent();

        Assert (!(tmp->isCommitted()) && !(tmp->isFailed()));

        if (tmp == home) {
          found = TRUE;
          break;
        }
      } while (!tmp->_isRoot());

    } else if (OZ_isCons(h)) {
      found = extParameters(h, solve_board);
    }

    if (found) return TRUE;

    list = OZ_tail(list);
  } // while
  return FALSE;
}

static
void oz_solve_clearSuspList(SolveActor *sa,Suspension killSusp)
{
  SuspList * tmpSuspList = sa->getSuspList();

  sa->setSuspList(NULL);
  while (tmpSuspList) {
    Suspension susp = tmpSuspList->getSuspension();

    /*
     *  kost@
     *  Note that i've preserved here a limitation of stability
     * check: no propagators (i.e. former "resistant" suspensions")
     * might suspend on global variables; otherwise, no stability
     * will be reached (precisely speaking, they must go away -
     * then stability can be reached);
     *  This limitation was introduced with the "resistant"
     * suspensions (Hi, Tobias!).
     *
     */

    if (susp.isDead() ||
        killSusp == susp ||
        (susp.isRunnable() && !susp.isPropagator())) {
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    Board * bb = GETBOARDOBJ(susp);

    // find suspensions, which occured in a failed nested search space
    while (1) {
      bb = bb->getSolveBoard();
      if (bb == sa->getSolveBoard()) break;
      if (bb == 0) break;
      bb = bb->getParentAndTest();
      if (bb == 0) break;
    }

    if (susp.isPropagator()) {
      Propagator * prop = susp.getPropagator();

#ifdef DEBUG_PROP_STABILTY_TEST
      cout << "SolveActor::clearSuspList : Found propagator." << endl
           << prop->toString() << endl
           << "\tbb = " << bb << endl << flush;
#endif

      if (bb) {
        // if propagator suspends on external variable then keep its
        // thread in the list to avoid stability
        if (extParameters(prop->getPropagator()->getParameters(), sa->getSolveBoard())) {
#ifdef DEBUG_PROP_STABILTY_TEST
          cout << "\tExt parameter found!" << endl << flush;
#endif
          SuspList * helpList = tmpSuspList;
          sa->addSuspension (helpList);
        }
#ifdef DEBUG_PROP_STABILTY_TEST
        else {
          cout << "\tNo ext parameter found!" << endl << flush;
        }
#endif

      }
      tmpSuspList = tmpSuspList->getNext ();
    } else {
      Assert(susp.isThread());

      Thread * thr = susp.getThread();

      if (bb == 0) {
        oz_disposeThread(thr);
        tmpSuspList = tmpSuspList->dispose ();
      } else {
        SuspList *helpList = tmpSuspList;
        tmpSuspList = tmpSuspList->getNext();
        sa->addSuspension (helpList);
      }
    }
  }
}

void oz_removeExtThreadOutlined(Thread *tt)
{
  Assert(tt->wasExtThread());

  Board *sb = GETBOARD(tt)->getSolveBoard ();

  while (sb) {
    Assert (sb->isSolve());

    SolveActor *sa = SolveActor::Cast(sb->getActor());
    oz_solve_clearSuspList(sa,tt);
    sb = GETBOARD(sa)->getSolveBoard();
  }
}

// see variable.hh
void oz_checkExtSuspension(Suspension susp, Board * home)
{
  if (am.isBelowSolveBoard()) {
    oz_setExtSuspensionOutlined(susp, home->derefBoard());
  }
}

void oz_setExtSuspensionOutlined(Suspension susp, Board *varHome)
{
  Board * bb = oz_currentBoard();
  Bool wasFound = NO;
  Assert (!varHome->isCommitted());

  while (bb != varHome) {
    Assert (!oz_isRootBoard(bb));
    Assert (!bb->isCommitted() && !bb->isFailed());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      sa->addSuspension(susp);
      wasFound = OK;
    }
    bb = bb->getParent();
  }

  if (wasFound) susp.setExtSuspension();
}

void oz_checkExtSuspensionOutlined(Suspension susp)
{
  Assert(susp.wasExtSuspension());

  Board *sb = GETBOARDOBJ(susp)->getSolveBoard();

  while (sb) {
    Assert(sb->isSolve());

    SolveActor * sa = SolveActor::Cast(sb->getActor());
    if (oz_isStableSolve(sa)) {
      oz_newThreadInject(DEFAULT_PRIORITY, sb); // mm2: maybe OPT
    }
    sb = GETBOARD(sa)->getSolveBoard();
  }
}

Bool oz_solve_checkExtSuspList (SolveActor *sa)
{
  // Kostja: Christian's; (no spaces!);
  oz_solve_clearSuspList(sa,(Thread *) NULL);
  return (sa->getSuspList() == NULL);
}
