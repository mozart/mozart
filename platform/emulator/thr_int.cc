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

#include "thr_int.hh"
#include "var_base.hh"
#include "solve.hh"

void oz_pushToLPQ(Board *bb, Propagator * prop);
void oz_setExtSuspensionOutlined(Suspension susp, Board *varHome);
Bool oz_isStableSolve(SolveActor *sa);


/* -------------------------------------------------------------------------
 * TODO
 * ------------------------------------------------------------------------- */

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
  SuspList * tmpSuspList = sa->unlinkSuspList();

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

// see var_base.hh
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
  return sa->isEmptySuspList();
}
