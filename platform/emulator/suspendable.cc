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

  Board * sb = getBoardInternal();

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

      switch (between) {
      case B_BETWEEN:
        oz_wakeupThread(SuspToThread(this));
        return OK;
      case B_NOT_BETWEEN:
        if (calledBy==pc_all) {
          oz_wakeupThread(SuspToThread(this));
          return OK;
        }
        return NO;
      case B_DEAD:
        setDead();

        if (isExternal())
          sb->derefBoard()->checkSolveThreads();

        SuspToThread(this)->disposeStack();
        return OK;
      }

    } else {

      switch (between) {
      case B_BETWEEN:
        if (calledBy)
          setUnify();
        setRunnable();
        if (isNMO() && !oz_onToplevel()) {
          Assert(!SuspToPropagator(this)->getPropagator()->isMonotonic());

          am.currentBoard()->addToNonMono(SuspToPropagator(this));
        } else {
          oz_pushToLPQ(SuspToPropagator(this));
        }
        return NO;
      case B_NOT_BETWEEN:
        return NO;
      case B_DEAD:
        setDead();
        if (isExternal())
          sb->derefBoard()->checkSolveThreads();
        SuspToPropagator(this)->dispose();
        return OK;
      }

    }

  }

}

Bool Suspendable::wakeup(Board * bb, PropCaller calledBy) {
  return _wakeup(bb, calledBy);
}

SuspList * oz_checkAnySuspensionList(SuspList *suspList,Board *home,
                                     PropCaller calledBy) {
  if (am.inEqEq())
    return suspList;

  SuspList * retSuspList = NULL;

  while (suspList) {

    if (suspList->getSuspendable()->_wakeup(home,calledBy)) {
      suspList = suspList->dispose();
      continue;
    }

    SuspList * first = suspList;
    suspList = suspList->getNext();
    first->setNext(retSuspList);
    retSuspList = first;

  }

  return retSuspList;
}
