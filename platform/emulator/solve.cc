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
	(thr->isPropagated () && !(thr->isPropagator()))) {
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    Board *bb = thr->getBoard();

    while (1) {
      bb = bb->getSolveBoard();
      if (bb == solveBoard) break;
      if (bb == 0) break;
      bb = bb->getParentAndTest();
      if (bb == 0) break;
    }

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

SolveActor::SolveActor(Board *bb, int prio)
 : Actor (Ac_Solve, bb, prio), suspList (NULL), threads (0), cpb(NULL), 
   localThreadQueue(NULL) {
  result     = makeTaggedRef(newTaggedUVar(bb));
  solveBoard = new Board(this, Bo_Solve);
  solveVar   = makeTaggedRef(newTaggedUVar(solveBoard));
  bb->decSuspCount();         // don't count this actor!
}

// ------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "solve.icc"
#undef inline
#endif
