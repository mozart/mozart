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
  if (!oz_installScript(to->getScriptRef())) {
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

int oz_incSolveThreads(Board *bb) {

  bb = bb->derefBoard();
      
  int ret = NO;

  while (!bb->isRoot()) {

    ret = OK;

    bb->incThreads();

    Assert(!(oz_isStableSolve(bb)));

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
  while (!bb->isRoot()) {

    Assert(!bb->isCommitted());

    // local optimization - check for threads first;
    if (bb->decThreads()==0) {

      // ... first - notification board below the failed solve board; 
      if (oz_isStableSolve(bb)) {
	oz_newThreadInject(bb);
      }
      
    }
    
    bb = bb->getParent();
  }
  
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

      Board * home = GETBOARD(tagged2SVarPlus(h)); 
      Board * tmp  = solve_board;

      // from solve board go up to root; if you step over home 
      // then the variable is external otherwise it must be a local one
      do {
	tmp = tmp->getParent();

	if (tmp->isFailed())
	  return FALSE;
	
	tmp = tmp->derefBoard();

	if (tmp == home) { 
	  found = TRUE;
	  break;
	}
      } while (!tmp->isRoot());
      
    } else if (OZ_isCons(h)) {
      found = extParameters(h, solve_board);
    }

    if (found) return TRUE;

    list = OZ_tail(list);
  } // while
  return FALSE;
}

static
void solve_clearSuspList(Board *sb, Suspension killSusp) {
  Assert(!sb->isRoot());
  
  SuspList * tmpSuspList = sb->unlinkSuspList();

  while (tmpSuspList) {
    // Traverse suspension list and copy all valid suspensions
    Suspension susp = tmpSuspList->getSuspension();

    tmpSuspList = tmpSuspList->dispose();

    if (susp.isDead() ||
	killSusp == susp ||
	(susp.isRunnable() && !susp.isPropagator())) {
      continue;
    }

    Board * bb = GETBOARDOBJ(susp);

    Bool isAlive = OK;
    
    // find suspensions, which occured in a failed nested search space
    while (1) {
      Assert(!bb->isCommitted() && !bb->isRoot());
      
      if (bb->isFailed()) {
	isAlive = NO;
	break;
      }
		     
      if (bb == sb)
	break;

      bb = bb->getParent();
    }

    if (susp.isPropagator()) {
      Propagator * prop = susp.getPropagator();
      
      if (isAlive) {

	// if propagator suspends on external variable then keep its
	// thread in the list to avoid stability
	if (extParameters(prop->getPropagator()->getParameters(), sb)) {
	  sb->addSuspension(susp);
	} 

      }

    } else {
      Assert(susp.isThread());
      
      Thread * thr = susp.getThread();

      if (isAlive) {
	bb->addSuspension(susp);
      } else {
	oz_disposeThread(thr);
      }
      
    }
  }
}

void oz_removeExtThread(Thread *tt) {
  Assert(tt->wasExtThread());
  
  Board *sb = GETBOARD(tt);
  
  while (!sb->isRoot()) {
    solve_clearSuspList(sb,tt);
    sb = sb->getParent();
  }
  
}

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

void _checkExtSuspension(Suspension susp) {
  Assert(susp.wasExtSuspension());

  Board *sb = GETBOARDOBJ(susp);

  while (!sb->isRoot()) {
    
    if (oz_isStableSolve(sb)) {
      oz_newThreadInject(sb);
    }

    sb = sb->getParent();
  }
}

inline
Bool solve_checkExtSuspList(Board *sb) {
  // Kostja: Christian's; (no spaces!);
  solve_clearSuspList(sb, (Thread *) NULL);
  return sb->isEmptySuspList();
}

inline
Bool solve_areNoExtSuspensions(Board *sb)
{
  if (sb->isEmptySuspList())
    return OK;
  else
    return (solve_checkExtSuspList(sb));
}

Bool oz_isStableSolve(Board *sb) {
  if (sb->getThreads() != 0) 
    return NO;
  
  if (oz_isCurrentBoard(sb) && !am.trail.isEmptyChunk())
    return NO;
  
  return solve_areNoExtSuspensions(sb); 
}

