/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  Solver
  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "solve.hh"
#endif

#include "am.hh"

#include "cpbag.hh"
#include "solve.hh"

/*
 * class SolveActor:
 *    solve actor;
 *    member data:
 *      solveBoard: 'home' board of a search problem; 
 *      orActors: all actors that may be distributed; 
 *      solveVar: root variable of a search problem; 
 *      result: reference to the control variable 
 *         (recall that there is single control variable, but not three 
 *           as in kernel definition);
 *      suspList: list of external threads; 
 *      threads: the number of *runnable* threads!
 */

/* ------------------------------------------------------------------------
   class SolveActor
   ------------------------------------------------------------------------ */

TaggedRef SolveActor::genSolved() {
  ozstat.incSolveSolved();
  SRecord *stuple = SRecord::newSRecord(AtomSucceeded, 1);

  Assert(solveBoard->isSolve());
  stuple->setArg(0, AtomEntailed);
  
  return makeTaggedSRecord(stuple);
}

TaggedRef SolveActor::genStuck() {
  SRecord *stuple = SRecord::newSRecord(AtomSucceeded, 1);
  
  Assert(solveBoard->isSolve());
  stuple->setArg(0, AtomSuspended);
  return makeTaggedSRecord(stuple);
}

TaggedRef SolveActor::genChoice(int noOfClauses) {
  SRecord *stuple = SRecord::newSRecord(AtomAlt, 1);

  Assert(!solveBoard->isCommitted());
  stuple->setArg(0, makeTaggedSmallInt(noOfClauses));

  return makeTaggedSRecord(stuple);
}

TaggedRef SolveActor::genFailed() {
  ozstat.incSolveFailed();
  return AtomFailed;
}

TaggedRef SolveActor::genUnstable(TaggedRef arg) {
  SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
  stuple->setArg(0, arg);
  return makeTaggedSRecord(stuple);
}

Bool SolveActor::isBlocked() {
  return ((getThreads()==0) && !am.isStableSolve(this));
}

static
Bool extParameters(OZ_Term list, Board * solve_board)
{
  while (OZ_isCons(list)) {
    OZ_Term h = OZ_head(list);
    
    Bool found = FALSE;

    if (OZ_isVariable(h)) {

#ifdef DEBUG_PROP_STABILTY_TEST
      taggedPrint(h);
#endif

      DEREF(h, hptr, htag);

      Assert(!isUVar(htag));

      Board  * home = (isSVar(htag) 
		       ? tagged2SVar(h) 
		       : taggedCVar2SVar(h))->getBoard();
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
      } while (!tmp->isRoot());
      
    } else if (OZ_isCons(h)) {
      found = extParameters(h, solve_board);
    }

    if (found) return TRUE;

    list = OZ_tail(list);
  } // while
  return FALSE;
}

void SolveActor::clearSuspList(Thread *killThr) {
  SuspList *tmpSuspList = suspList;

  suspList = NULL;
  while (tmpSuspList) {
    Thread *thr = tmpSuspList->getElem ();

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

    if (thr->isDeadThread () ||
	killThr == thr ||
	(thr->isRunnable () && !(thr->isPropagator()))) {
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    Board *bb = thr->getBoard();

    // find threads, which occured in a failed nested search space
    while (1) {
      bb = bb->getSolveBoard();
      if (bb == solveBoard) break;
      if (bb == 0) break;
      bb = bb->getParentAndTest();
      if (bb == 0) break;
    }

    if (thr->isPropagator()) {

#ifdef DEBUG_PROP_STABILTY_TEST
      cout << "SolveActor::clearSuspList : Found propagator." << endl 
	   << *thr->getPropagator() << endl
	   << "\tbb = " << bb << endl << flush;
#endif

      if (bb) {
	// if propagator suspends on external variable then keep its
	// thread in the list to avoid stability
	if (extParameters(thr->getPropagator()->getParameters(), solveBoard)) {
#ifdef DEBUG_PROP_STABILTY_TEST
	  cout << "\tExt parameter found!" << endl << flush;
#endif
	  SuspList * helpList = tmpSuspList;
	  addSuspension (helpList);
	} 
#ifdef DEBUG_PROP_STABILTY_TEST
	else {
	  cout << "\tNo ext parameter found!" << endl << flush;
	}
#endif

      }
      tmpSuspList = tmpSuspList->getNext ();
    } else {
      if (bb == 0) {
	thr->disposeSuspendedThread ();
	tmpSuspList = tmpSuspList->dispose ();
      } else {
	SuspList *helpList = tmpSuspList;
	tmpSuspList = tmpSuspList->getNext();
	addSuspension (helpList);
      }
    }
  }
}

SolveActor::SolveActor(Board *bb)
 : Actor (Ac_Solve, bb), suspList (NULL), threads (0), cpb(NULL), 
   localThreadQueue(NULL), nonMonoSuspList(NULL) {
  result     = makeTaggedRef(newTaggedUVar(bb));
  solveBoard = new Board(this, Bo_Solve);
  solveVar   = makeTaggedRef(newTaggedUVar(solveBoard));
  bb->decSuspCount();         // don't count this actor!
}

// ------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// support for nonmonotonic propagators

void SolveActor::addToNonMonoSuspList(Thread * thr)
{
  nonMonoSuspList = nonMonoSuspList->insert(thr);
}

void SolveActor::mergeNonMonoSuspListWith(OrderedSuspList * p)
{
  for (; p != NULL; p = p->getNext())
    nonMonoSuspList = nonMonoSuspList->insert(p->getThread()); 
}

void SolveActor::scheduleNonMonoSuspList(void)
{
#ifdef DEBUG_NONMONOTONIC
  cout << "SolveActor::scheduleNonMonoSuspList" << endl << flush;
#endif

  for (OrderedSuspList * p = nonMonoSuspList; p != NULL; p = p->getNext()) {
    Thread * thr = p->getThread();

#ifdef DEBUG_NONMONOTONIC
    cout << "   "; thr->printDebug();
#endif
    
    thr->updateSolveBoardPropagatorToRunnable();
    am.scheduleThreadInline(thr, thr->getPriority());
  }  

  nonMonoSuspList = NULL;

#ifdef DEBUG_NONMONOTONIC
  cout << "Done" << endl << flush;
#endif
}

//-----------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "solve.icc"
#undef inline
#endif
