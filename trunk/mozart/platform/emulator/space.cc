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

#include "space.hh"
#include "threadInterface.hh"

inline
// static
Bool oz_installScriptOPT(Script &script)
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

Bool oz_installScript(Script &script)
{
  return oz_installScriptOPT(script);
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
    if (oz_isCurrentBoard(to)) return B_BETWEEN;
    if (to == varHome) return B_NOT_BETWEEN;
    Assert(!oz_isRootBoard(to));
    to = to->getParentAndTest();
    if (!to) return B_DEAD;
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

  Board *par=to->getParentAndTest();
  if (!par) {
    return INST_REJECTED;
  }

  InstType ret = oz_installPath(par);
  if (ret != INST_OK) {
    return ret;
  }

  am.setCurrent(to);
  to->setInstalled();

  am.trail.pushMark();
  if (!oz_installScriptOPT(to->getScriptRef())) {
    return INST_FAILED;
  }
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
    Thread *thr = oz_mkWakeupThread(bb);
  
    for (int index = 0; index < numbOfCons; index++) {
      TaggedRef * refPtr, value;

      am.trail.popRef(refPtr, value);

      Assert(oz_isRef(*refPtr) || !oz_isVariable(*refPtr));
      Assert(oz_isVariable(value));

      bb->setScript(index,refPtr,*refPtr);

      TaggedRef vv= *refPtr;
      DEREF(vv,vvPtr,_vvTag);
      if (oz_isVariable(vv)) {
	addSuspAnyVar(vvPtr,thr,NO);  // !!! Makes space *not* unstable !!!
      }

      unBind(refPtr, value);

      // value is always global variable, so add always a thread;
      addSuspAnyVar(refPtr,thr);

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

void oz_reduceTrailOnShallow()
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

    /*
     * shallow guards don't bind variables always.
     *  in INLINEREL/FUNs they are only pushed (pushIfVar) onto the trail
     */
    if (refPtr!=ptrOldVal) {
      if (oz_isVariable(oldVal)) {
	addSuspAnyVar(ptrOldVal,oz_currentThread());
      }
    }

    addSuspAnyVar(refPtr,oz_currentThread());
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

/*
 * increment/decrement the thread counter
 * in every solve board above
 * if "stable" generate a new thread "solve waker"
 * NOTE:
 *   there may be failed board in between which must be simply ignored
 *
 * kost@ to mm2:
 *   Michael, i don't think that it was a good idea to merge 
 * these things together - this leads to efficiency penalties, 
 * and less possibilities to make an assertion.
 *
 * RETURNS: OK if solveSpace found, else NO
 */

int oz_incSolveThreads(Board *bb)
{
  int ret = NO;
  while (!oz_isRootBoard(bb)) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      ret = OK;
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      //
      Assert (!sa->isCommitted());

      //  
      sa->incThreads ();

      //
      Assert (!(oz_isStableSolve (sa)));
    }
    bb = bb->getParent();
  }
  return ret;
}

#ifdef DEBUG_THREADCOUNT
void oz_decSolveThreads(Board *bb, char * s)
{
  //printf("AM::decSolveThreads: %s.\n", s); fflush(stdout);
#else
void oz_decSolveThreads(Board *bb)
{
#endif
  while (!oz_isRootBoard(bb)) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());

      //
      // local optimization - check for threads first;
      if (sa->decThreads () == 0) {
	//
	// ... first - notification board below the failed solve board; 
	if (!(sa->isCommitted ()) && oz_isStableSolve (sa)) {
	  am.threadsPool.scheduleThread(oz_mkRunnableThread(DEFAULT_PRIORITY,
							    bb));
	}
      } else {
	Assert (sa->getThreads () > 0);
      }
    }
    bb = bb->getParent();
  }
#ifdef DEBUG_THREADCOUNT
  //printf("(AM::decSolveThreads LTQs=%d) ", existingLTQs); fflush(stdout);
#endif
}

#ifdef DEBUG_CHECK
/*
 *  Just check whether the 'bb' is located beneath some (possibly dead) 
 * solve board;
 */
Bool oz_isInSolveDebug (Board *bb)
{
  while (!oz_isRootBoard(bb)) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      if (!sa->isCommitted()) {
	return (OK);
      }
    }
    bb = bb->getParent();
  }

  return (NO);
}
#endif

inline
Bool oz_solve_areNoExtSuspensions(SolveActor *sa)
{
  if (sa->getSuspList() == NULL)
    return (OK);
  else
    return (oz_solve_checkExtSuspList(sa));
}

Bool oz_isStableSolve(SolveActor *sa)
{
  if (sa->getThreads() != 0) 
    return NO;
  if (oz_isCurrentBoard(sa->getSolveBoard()) &&
      !am.trail.isEmptyChunk())
    return NO;
  // simply "don't worry" if in all other cases it is too weak;
  return oz_solve_areNoExtSuspensions(sa); 
}


int oz_commit(Board *bb, Thread *tt)
{
  Assert(!oz_currentBoard()->isCommitted());
  Assert(oz_isCurrentBoard(bb->getParent()));

  AWActor *aw = AWActor::Cast(bb->getActor());

  Assert(!tt || tt==aw->getThread());

  Continuation *cont=bb->getBodyPtr();

  oz_merge(bb,oz_currentBoard(),bb->getSuspCount()-1);

  if (bb->isWait()) {
    Assert(bb->isWaiting());

    WaitActor *wa = WaitActor::Cast(aw);

    if (oz_currentBoard()->isWait()) {
      WaitActor::Cast(oz_currentBoard()->getActor())->mergeChoices(wa->getCpb());
    } else if (oz_currentBoard()->isSolve()) {
      SolveActor::Cast(oz_currentBoard()->getActor())->mergeChoices(wa->getCpb());
    } else {
      // forget the choice stack when committing to a conditional
    }

    if (!oz_installScript(bb->getScriptRef())) {
      return 0;
    }
  }

  if (!tt) {
    tt=aw->getThread();
    Assert(tt->isSuspended());
    oz_suspThreadToRunnableOPT(tt);
    am.threadsPool.scheduleThread(tt);
    DebugCheckT(aw->setThread(0));
  }

  TaskStack *ts = tt->getTaskStackRef();
  ts->discardActor();
  if (aw->isAsk()) {
    AskActor::Cast(aw)->disposeAsk();
  } else {
    WaitActor::Cast(aw)->disposeWait();
  }

  ts->pushCont(cont->getPC(),cont->getY(),cont->getCAP());
  if (cont->getX()) ts->pushX(cont->getX());

  return 1;
}


/*
 * when failure occurs
 *  mark the actor
 *  clean the trail
 *  update the current board
 */
void oz_failBoard()
{
  Assert(!oz_onToplevel());
  Board *bb=oz_currentBoard();
  Assert(bb->isInstalled());

  Actor *aa=bb->getActor();
  if (aa->isAsk()) {
    (AskActor::Cast(aa))->failAskChild();
  } else if (aa->isWait()) {
    (WaitActor::Cast(aa))->failWaitChild(bb);
  }

  Assert(!bb->isFailed());
  bb->setFailed();

  oz_reduceTrailOnFail();
  bb->unsetInstalled();
  am.setCurrent(GETBOARD(aa));
}

void oz_merge(Board *bb, Board *to,int inc)
{
  to->setLocalPropagatorQueue(bb->getLocalPropagatorQueue()->merge(to->getLocalPropagatorQueue()));
  bb->getActor()->setCommittedActor();
  bb->setCommittedBoard(to);
  to->incSuspCount(inc);
}
