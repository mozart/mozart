/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Contributors:
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

#include "space.hh"
#include "thr_int.hh"
#include "var_base.hh"

OZ_Return oz_installScript(Script & s) {
  OZ_Return ret = PROCEED;

  am.setInstallingScript();

  for (int i = 0; i < s.getSize(); i++) {

    int res = oz_unify(s[i].left, s[i].right);

    if (res == PROCEED)
      continue;

    if (res == FAILED) {
      ret = FAILED;
      if (!oz_onToplevel()) {
        break;
      }
    } else {
      // mm2: instead of failing, this should corrupt the space
      (void) am.emptySuspendVarList();
      ret = FAILED;
      if (!oz_onToplevel()) {
        break;
      }
    }
  }

  am.unsetInstallingScript();

  s.dispose();

  return ret;
}

/*
 *
 *  Install every board from the currentBoard to 'n'
 * and move cursor to 'n'
 *
 *  Algorithm:
 *   find common parent board of 'to' and 'currentBoard'
 *   deinstall until common parent (go upward)
 *   install (go downward)
 *
 *  Pre-conditions:
 *  - 'to' ist not deref'd;
 *  - 'to' may be committed, failed or discarded;
 *
 *  Return values and post-conditions:
 *  - INST_OK:
 *      installation successful, currentBoard == 'to';
 *  - INST_FAILED:
 *      installation of *some* board on the "down" path has failed,
 *      'am.currentBoard' points to that board;
 *  - INST_REJECTED:
 *      *some* board on the "down" path is already failed or discarded,
 *      'am.currentBoard' stays unchanged;
 *
 */


static
Board * installOnly(Board * frm, Board * to) {

  if (frm == to)
    return frm;

  Board * r = installOnly(frm, to->getParent());

  if (r != frm)
    return r;

  am.setCurrent(to);
  am.trail.pushMark();

  OZ_Return ret = oz_installScript(to->getScript());

  if (ret != PROCEED) {
    Assert(ret==FAILED);
    return to;
  }

  return r;

}


Bool oz_installPath(Board * to) {
  // Tries to install "to".
  // If installation of a script fails, NO is returned and
  // the highest space for which installation is possible gets installed.
  // Otherwise, OK is returned.

  Board * frm = oz_currentBoard();

  Assert(!frm->isCommitted() && !to->isCommitted());

  if (frm == to)
    return OK;

  Assert(to->isAlive());

  // Step 1: Mark all spaces including root as installed
  {
    Board * s;

    for (s = frm; !s->isRoot(); s=s->getParent()) {
      Assert(!s->hasMarkOne());
      s->setMarkOne();
    }
    Assert(!s->hasMarkOne());
    s->setMarkOne();
  }

  // Step 2: Find ancestor
  Board * ancestor = to;

  while (!ancestor->hasMarkOne())
    ancestor = ancestor->getParent();

  // Step 3: Deinstall from "frm" to "ancestor", also purge marks
  {
    Board * s = frm;

    while (s != ancestor) {
      Assert(s->hasMarkOne());
      s->unsetMarkOne();
      am.trail.unwind();
      s=s->getParent();
      am.setCurrent(s);
    }

    am.setCurrent(ancestor);

    // Purge remaining marks
    for ( ; !s->isRoot() ; s=s->getParent()) {
      Assert(s->hasMarkOne());
      s->unsetMarkOne();
    }
    Assert(s->hasMarkOne());
    s->unsetMarkOne();

  }

  // Step 4: Install from "ancestor" to "to"

  return installOnly(ancestor, to) == ancestor;

}
