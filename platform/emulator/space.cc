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

inline
void unBind(TaggedRef *p, TaggedRef t)
{
  Assert(oz_isVariable(t));
  *p = t;
}

Bool oz_installScript(Script &script)
{
  Bool ret = OK;
  am.setInstallingScript(); // mm2: special hack ???
  for (int index = 0; index < script.getSize(); index++) {
    int res = oz_unify(script[index].getLeft(),script[index].getRight());
    if (res == PROCEED) continue;
    if (res == FAILED) {
      ret = NO;
      if (!oz_onToplevel()) {
        break;
      }
    } else {
      // mm2: instead of failing, this should corrupt the space
      (void) am.emptySuspendVarList();
      ret = NO;
      if (!oz_onToplevel()) {
        break;
      }
    }
  }
  am.unsetInstallingScript();

  // mm2: why this?
#ifndef DEBUG_CHECK
  script.dealloc();
#else
  if (ret == OK)
    script.dealloc();
#endif
  return ret;
}

Bool oz_isBelow(Board *below, Board *above)
{
  while (1) {
    if (below == above) return OK;
    if (oz_isRootBoard(below)) return NO;
    below = below->getParent();
  }
}

/*
  This function checks if the current board is between "varHome" and "to"
  resp. equal to "to".
  */

oz_BFlag oz_isBetween(Board *to, Board *varHome)
{
  while (1) {

    if (oz_isCurrentBoard(to))
      return B_BETWEEN;

    if (to == varHome)
      return B_NOT_BETWEEN;

    Assert(!oz_isRootBoard(to));

    if (to->isFailed())
      return B_DEAD;

    to = to->getParent();

  }
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
InstType oz_installPath(Board *to)
{
  if (to->isInstalled()) {
    oz_deinstallPath(to);
    return INST_OK;
  }

  Assert(!oz_isRootBoard(to));

  if (to->isFailed())
    return INST_REJECTED;

  InstType ret = oz_installPath(to->getParent());

  if (ret != INST_OK)
    return ret;

  am.setCurrent(to);
  to->setInstalled();

  am.trail.pushMark();

  if (!oz_installScript(to->getScriptRef()))
    return INST_FAILED;

  return INST_OK;
}

// only used in deinstall
// Three cases may occur:
// any global var G -> ground ==> add susp to G
// any global var G -> constrained local var ==> add susp to G
// unconstrained global var G1 -> unconstrained global var G2
//    ==> add susp to G1 and G2

void oz_reduceTrailOnSuspend()
{
  if (!am.trail.isEmptyChunk()) {
    int numbOfCons = am.trail.chunkSize();
    Board * bb = oz_currentBoard();
    bb->newScript(numbOfCons);

    //
    // one single suspended thread for all;
    Thread *thr = oz_newThreadPropagate(bb);

    for (int index = 0; index < numbOfCons; index++) {
      TaggedRef * refPtr, value;

      am.trail.popRef(refPtr, value);

      Assert(oz_isRef(*refPtr) || !oz_isVariable(*refPtr));
      Assert(oz_isVariable(value));

      bb->setScript(index,refPtr,*refPtr);

      TaggedRef vv= *refPtr;
      DEREF(vv,vvPtr,_vvTag);
      if (oz_isVariable(vv)) {
        oz_var_addSusp(vvPtr,thr,NO);  // !!! Makes space *not* unstable !!!
      }

      unBind(refPtr, value);

      // value is always global variable, so add always a thread;
      if (oz_var_addSusp(refPtr,thr)!=SUSPEND) {
        Assert(0);
      }

    } // for
  } // if
  am.trail.popMark();
}

void oz_reduceTrailOnFail()
{
  while(!am.trail.isEmptyChunk()) {
    TaggedRef *refPtr;
    TaggedRef value;
    am.trail.popRef(refPtr,value);
    unBind(refPtr,value);
  }
  am.trail.popMark();
}

void oz_reduceTrailOnEqEq()
{
  am.emptySuspendVarList();

  while(!am.trail.isEmptyChunk()) {
    TaggedRef *refPtr;
    TaggedRef value;
    am.trail.popRef(refPtr,value);

    Assert(oz_isVariable(value));

    TaggedRef oldVal = makeTaggedRef(refPtr);
    DEREF(oldVal,ptrOldVal,_1);

    unBind(refPtr,value);

    if (oz_isVariable(oldVal)) {
      am.addSuspendVarList(ptrOldVal);
    }

    am.addSuspendVarList(refPtr);
  }
  am.trail.popMark();
}

/* -------------------------------------------------------------------------
 * Search
 * -------------------------------------------------------------------------*/

void oz_checkExtSuspension(Suspension susp, Board * varHome) {

  if (!oz_onToplevel()) {

    varHome=varHome->derefBoard();

    Board * bb = oz_currentBoard();

    Bool wasFound = NO;

    Assert (!varHome->isCommitted());

    while (bb != varHome) {
      Assert (!oz_isRootBoard(bb));
      Assert (!bb->isCommitted() && !bb->isFailed());

      bb->addSuspension(susp);
      wasFound = OK;

      bb = bb->getParent();
    }

    if (wasFound)
      susp.setExtSuspension();
  }
}
