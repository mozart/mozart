/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Kostja Popow, 1997-1999
 *    Michael Mehl, 1997-1999
 *    Christian Schulte, 1997-1999
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

#if defined(INTERFACE)
#pragma implementation "board.hh"
#endif

#include "board.hh"
#include "thr_int.hh"
#include "prop_int.hh"
#include "space.hh"
#include "builtins.hh"
#include "value.hh"
#include "var_base.hh"
#include "var_future.hh"
#include "os.hh"

#ifdef OUTLINE
#include "board.icc"
#endif


/*
 * Generic operations
 *
 */

Board::Board() 
  : suspCount(0), bag(0),
    threads(0), suspList(0), nonMonoSuspList(0),
    status(taggedVoidValue), rootVar(taggedVoidValue)
{
  parentAndFlags.set((void *) 0, (int) BoTag_Root);
  lpq.init();
}


Board::Board(Board * p) 
  : suspCount(0), bag(0),
    threads(0), suspList(0), nonMonoSuspList(0)
{
  Assert(!p->isCommitted());
  status  = oz_newFuture(p);
  rootVar = oz_newVar(this);
  parentAndFlags.set((void *) p, 0);
  lpq.init();
#ifdef CS_PROFILE
  orig_start  = (int32 *) NULL;
  copy_start  = (int32 *) NULL;
  copy_size   = 0;
#endif
}

TaggedRef Board::genBlocked(TaggedRef arg) {
  SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
  stuple->setArg(0, arg);
  return makeTaggedSRecord(stuple);
}

void Board::bindStatus(TaggedRef t) {
  TaggedRef s = getStatus();
  DEREF(s, sPtr, _);

  // STRANGE
  if (oz_isFuture(s))
    oz_bindFuture(sPtr, t);
}

void Board::clearStatus() {
  if (oz_isFuture(oz_deref(getStatus())))
    return;

  status = oz_newFuture(getParent());
}


/*
 * Propagator queue
 *
 */

void Board::wakeServeLPQ(void) {
  Assert(lpq.isEmpty());

  if (board_served == this)
    return;
  
  Thread * thr = oz_newThreadInject(this);

  // Push run lpq builtin
  thr->pushCall(BI_PROP_LPQ, 0, 0);

}

inline
void Board::killServeLPQ(void) {
  board_served = NULL;
  lpq.reset();
}

Board * Board::board_served = NULL;

OZ_Return Board::scheduleLPQ(void) {
  Assert(!board_served);

  board_served = this;

  unsigned int starttime = 0;
	 
  if (ozconf.timeDetailed)
    starttime = osUserTime();
	
  while (!lpq.isEmpty() && !am.isSetSFlag()) {
    Propagator * prop = SuspToPropagator(lpq.dequeue());
    Propagator::setRunningPropagator(prop);
    Assert(!prop->isDead());
	   
    switch (oz_runPropagator(prop)) {
    case SLEEP:
      oz_sleepPropagator(prop);
      break;
    case PROCEED:
      oz_closeDonePropagator(prop);
      break;
    case SCHEDULED:
      oz_preemptedPropagator(prop);
      break;
    case FAILED:
      
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
	  killServeLPQ();
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
	killServeLPQ();
	return FAILED;
      }
      
      if (ozconf.errorDebug) 
	am.setExceptionInfo(OZ_mkTupleC("apply",2,
					OZ_atom((prop->getPropagator()->getProfile()->getPropagatorName())),
					prop->getPropagator()->getParameters()));	
      
      oz_closeDonePropagator(prop);
      killServeLPQ();
      return RAISE;
    }

    Assert(prop->isDead() || !prop->isRunnable());
    
  }
  
  if (ozconf.timeDetailed)
    ozstat.timeForPropagation.incf(osUserTime()-starttime);

  if (lpq.isEmpty()) {
    killServeLPQ();
    return PROCEED;
  } else {
    board_served = NULL;
    am.prepareCall(BI_PROP_LPQ, (RefsArray) NULL);
    return BI_REPLACEBICALL;
  }

}

/*
 * Routines for checking external suspensions
 *
 */

void Board::checkExtSuspension(Suspendable * susp) {

  Board * varHome = derefBoard();

  Board * bb = oz_currentBoard();

  Bool wasFound = NO;
    
  while (bb != varHome) {
    Assert(!bb->isRoot() && !bb->isCommitted() && !bb->isFailed());
    
    bb->addSuspension(susp);
    wasFound = OK;
    
    bb = bb->getParent();
  }
  
  if (wasFound) 
    susp->setExternal();
  
}

static
Bool extParameters(TaggedRef list, Board * solve_board) {

  list = oz_deref(list);
  
  while (oz_isCons(list)) {
    TaggedRef h = oz_head(list);
    
    Bool found = FALSE;

    DEREF(h, hptr, htag);

    if (oz_isVariable(h)) {

      Assert(!isUVar(htag));

      Board * home = GETBOARD(tagged2SVarPlus(h)); 
      Board * tmp  = solve_board;

      // from solve board go up to root; if you step over home 
      // then the variable is external otherwise it must be a local one
      do {
	tmp = tmp->getParent();

	if (tmp->isFailed())
	  return FALSE;
	
	if (tmp == home) { 
	  found = TRUE;
	  break;
	}
      } while (!tmp->isRoot());
      
    } else if (oz_isCons(h)) {
      found = extParameters(h, solve_board);
    }

    if (found) return TRUE;

    list = oz_tail(oz_deref(list));
  } // while
  return FALSE;
}


void Board::clearSuspList(Suspendable * killSusp) {
  Assert(!isRoot());
  
  SuspList * fsl = getSuspList();
  SuspList * tsl = (SuspList *) 0;

  while (fsl) {
    // Traverse suspension list and copy all valid suspensions
    Suspendable * susp = fsl->getSuspendable();

    fsl = fsl->dispose();

    if (susp->isDead() ||
	killSusp == susp ||
	(susp->isRunnable() && !susp->isPropagator())) {
      continue;
    }

    Board * bb = GETBOARD(susp);

    Bool isAlive = OK;
    
    // find suspensions, which occured in a failed nested search space
    while (1) {
      Assert(!bb->isCommitted() && !bb->isRoot());
      
      if (bb->isFailed()) {
	isAlive = NO;
	break;
      }
		     
      if (bb == this)
	break;

      bb = bb->getParent();
    }

    if (susp->isPropagator()) {
      
      if (isAlive) {
	// if propagator suspends on external variable then keep its
	// thread in the list to avoid stability
	if (extParameters(SuspToPropagator(susp)->getPropagator()->getParameters(), this)) {
	  tsl = new SuspList(susp, tsl);
	} 

      }

    } else {
      Assert(susp->isThread());
      
      if (isAlive) {
	tsl = new SuspList(susp, tsl);
      } else {
	oz_disposeThread(SuspToThread(susp));
      }
      
    }
  }

  setSuspList(tsl);

}


/*
 * Stability checking
 *
 */

void Board::checkStability(void) {
  Assert(!isRoot());

  Assert(!isFailed() && !isCommitted());

  Board * pb = getParent();

  if (decThreads() != 0) {
    pb->decSolveThreads();
    return;
  }
    
  if (isStable()) {
    Assert(am.trail.isEmptyChunk());

    // check for nonmonotonic propagators
    scheduleNonMono();
    if (!isStable())
      goto exit;
    
    // Check whether there are registered distributors
    Distributor * d = getDistributor();
    
    if (d) {
      
      int n = d->getAlternatives();
      
      if (n == 1) {
	// Is the distributor unary?
	d->commit(this,1,1);
	goto exit;
      } else {
	// don't decrement counter of parent board!
	am.trail.popMark();
	am.setCurrent(getParent());
      
	bindStatus(genAlt(n));

	goto exit;
      }
      
    }
    
    // succeeded
    am.trail.popMark();
    am.setCurrent(getParent());
    
    bindStatus(genSucceeded(getSuspCount() == 0));

    goto exit;
  }

  if (getThreads() == 0) {
    // There are some external suspensions: blocked

    oz_deinstallCurrent();

    TaggedRef newVar = oz_newFuture(oz_currentBoard());

    bindStatus(genBlocked(newVar));

    setStatus(newVar);

    goto exit;
  }

  oz_deinstallCurrent();

 exit:
  
  pb->decSolveThreads();
} 

void Board::fail(Thread * ct) {
  // Note that ``this'' might be different from the thread's home:
  // this can happen while trying to install the space!
  Assert(ct->isRunnable());
      
  Board * pb = getParent();

  Assert(!isRoot());
      
  setFailed();
      
  am.trail.unwindFailed();
      
  am.setCurrent(pb);
      
  bindStatus(genFailed());
     
  pb->decSolveThreads();

  oz_disposeThread(ct);
  
}


