/*
 *  Authors:
 *    Konstantin Popov (popow@ps.uni-sb.de)
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Konstantin Popov, 1999
 *    Tobias Müller, 1999
 *    Christian Schulte, 1999
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

#include "suspendable.hh"
#include "thr_class.hh"
#include "prop_class.hh"
#include "thr_int.hh"
#include "prop_int.hh"
#include "am.hh"

inline
Bool Suspendable::_wakeup(Board * home, PropCaller calledBy) {
  // Returns OK, if suspension can go away
  // This can happen, if either the suspension is dead
  // or gets woken

  if (isDead())
    return OK;

  Board * sb = getBoardInternal()->derefBoard();

  oz_BFlag between = oz_isBetween(sb, home);

  if (isRunnable()) {

    if (isThread()) {
      return OK;
    } else {
      if (calledBy && !isUnify()) {
        switch (between) {
        case B_BETWEEN:
          setUnify();
          return NO;
        case B_DEAD:
          return OK;
        case B_NOT_BETWEEN:
          return NO;
        }
      }
    }

    return NO;

  } else {

    if (isThread()) {

      Thread * t = SuspToThread(this);

      switch (between) {
      case B_BETWEEN:
        oz_wakeupThread(t);
        DEBUG_CONSTRAIN_VAR(("Suspendable::_wakeup_thread [t:%p s:%p c:%p]\n",
                              t, sb, oz_currentBoard()));
        return OK;

      case B_NOT_BETWEEN:
        if (calledBy==pc_all) {
          // raph: this happens only when making a global variable needed
          oz_wakeupThread(t);
          return OK;
        }
        return NO;
      case B_DEAD:
        setDead();
        t->disposeStack();
        return OK;
      }

    } else {

      switch (between) {
      case B_BETWEEN:
        if (calledBy)
          setUnify();
        setRunnable();

        DEBUG_CONSTRAIN_VAR(("Suspendable::_wakeup_prop [p:%p s:%p c:%p]\n",
                              SuspToPropagator(this)->getPropagator(),
                              sb, oz_currentBoard()));

        if (isNMO() && !oz_onToplevel()) {
          Assert(!SuspToPropagator(this)->getPropagator()->isMonotonic());

          sb->addToNonMono(SuspToPropagator(this));
        } else {
          sb->addToLPQ(SuspToPropagator(this));
        }
        return NO;
      case B_NOT_BETWEEN:
        return NO;
      case B_DEAD:
        setDead();
        SuspToPropagator(this)->dispose();
        return OK;
      }

    }

  }

  return NO;

}

Bool Suspendable::_wakeup_outline(Board * home, PropCaller calledBy) {
  return _wakeup(home, calledBy);
}

void oz_checkAnySuspensionList(SuspList ** suspList,
                               Board * home,
                               PropCaller calledBy) {

  // raph: Do not return when calledBy == pc_all, otherwise by-need
  // futures aren't kicked properly by '=='.
  if ((calledBy != pc_all && am.inEqEq()) || Board::mustIgnoreWakeUp()) {
    return;
  }

  home = home->derefBoard();

  SuspList ** p  = suspList;

  SuspList * sl = *suspList;

#ifdef COUNT_PROP_INVOCS
  int _len = 0;
#endif

  while (sl) {

#ifdef COUNT_PROP_INVOCS
    _len += 1;
#endif

    SuspList ** n = sl->getNextRef();

    if (sl->getSuspendable()->_wakeup(home,calledBy)) {
      *p = *n;
      sl->dispose();
      sl = *p;
    } else {
      sl = *n;
      p  = n;
    }


  }

#ifdef COUNT_PROP_INVOCS
  if (_len) {
    extern int count_prop_invocs_max_len_sl;
    count_prop_invocs_max_len_sl = max(count_prop_invocs_max_len_sl,
                                       _len);
    extern int count_prop_invocs_min_len_sl;
    count_prop_invocs_min_len_sl = min(count_prop_invocs_min_len_sl,
                                       _len);
    extern int count_prop_invocs_sum_len_sl;
    count_prop_invocs_sum_len_sl += _len;
    extern int count_prop_invocs_nb_nonempty_sl;
    count_prop_invocs_nb_nonempty_sl += 1;
  }
#endif

}



inline
Bool Suspendable::_wakeupAll(void) {
  // Returns OK, if suspension can go away
  // This can happen, if either the suspension is dead
  // or gets woken

  if (isDead())
    return OK;

  if (!isRunnable()) {

    if (isThread()) {

      oz_wakeupThread(SuspToThread(this));

    } else {

      setRunnable();

      Board * sb = getBoardInternal()->derefBoard();

      if (isNMO() && !oz_onToplevel()) {
        sb->addToNonMono(SuspToPropagator(this));
      } else {
        sb->addToLPQ(SuspToPropagator(this));
      }

    }
  }

  return isThread();

}


void oz_forceWakeUp(SuspList ** suspList) {

  if (am.inEqEq())
    return;

  SuspList ** p  = suspList;

  SuspList * sl = *suspList;

  while (sl) {

    SuspList ** n = sl->getNextRef();

    if (sl->getSuspendable()->_wakeupAll()) {
      *p = *n;
      sl->dispose();
      sl = *p;
    } else {
      sl = *n;
      p  = n;
    }


  }

}

inline
Bool Suspendable::_wakeupLocal(Board * sb, PropCaller calledBy) {
  if (isDead())
    return OK;

  if (calledBy)
    setUnify();

  if (!isRunnable()) {
    setRunnable();

    DEBUG_CONSTRAIN_VAR(("Suspendable::_wakeupLocal [%p]\n",
                          SuspToPropagator(this)->getPropagator()));

    if (isNMO() && !oz_onToplevel()) {
      Assert(!SuspToPropagator(this)->getPropagator()->isMonotonic());

      sb->addToNonMono(SuspToPropagator(this));
    } else {
      sb->addToLPQ(SuspToPropagator(this));
    }

  }

  return NO;

}

void oz_checkLocalSuspensionList(SuspList ** suspList,
                                 PropCaller calledBy)
{

  if (am.inEqEq() || Board::mustIgnoreWakeUp()) {
    return;
  }

  SuspList ** p = suspList;

  SuspList * sl = *p;

  if (!sl)
    return;

  Board * sb = sl->getSuspendable()->getBoardInternal()->derefBoard();

  if (sb != am.currentBoard())
    return;

#ifdef COUNT_PROP_INVOCS
  int _len = 0;
#endif

  do {

#ifdef COUNT_PROP_INVOCS
    _len += 1;
#endif

    SuspList ** n = sl->getNextRef();

    Assert(sb == sl->getSuspendable()->getBoardInternal()->derefBoard());

    if (sl->getSuspendable()->_wakeupLocal(sb,calledBy)) {
      *p = *n;
      sl->dispose();
      sl = *p;
    } else {
      sl = *n;
      p  = n;
    }


  } while (sl);

#ifdef COUNT_PROP_INVOCS
  if (_len) {
    extern int count_prop_invocs_max_len_el;
    count_prop_invocs_max_len_el = max(count_prop_invocs_max_len_el,
                                       _len);
    extern int count_prop_invocs_min_len_el;
    count_prop_invocs_min_len_el = min(count_prop_invocs_min_len_el,
                                       _len);
    extern int count_prop_invocs_sum_len_el;
    count_prop_invocs_sum_len_el += _len;
    extern int count_prop_invocs_nb_nonempty_el;
    count_prop_invocs_nb_nonempty_el += 1;
  }
#endif

}
