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

#include "dllstack.hh"
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

BuiltinTabEntry *solveContBITabEntry = NULL;
BuiltinTabEntry *solvedBITabEntry    = NULL;

TaggedRef solvedAtom;
TaggedRef choiceAtom;
TaggedRef entailedAtom;
TaggedRef stableAtom;
TaggedRef unstableAtom;
TaggedRef failedAtom;

void SolveActor::Init()
{
  solveContBITabEntry
    = new BuiltinTabEntry("*once-only*", 1, BIsolveCont); // local Entry;
  solvedBITabEntry
    = new BuiltinTabEntry("*reflected*", 1, BIsolved);    // local Entry;

  solvedAtom     = makeTaggedAtom (SEARCH_SOLVED);
  choiceAtom     = makeTaggedAtom (SEARCH_CHOICE);
  entailedAtom   = makeTaggedAtom (SEARCH_ENTAILED);
  stableAtom     = makeTaggedAtom (SEARCH_STABLE);
  unstableAtom   = makeTaggedAtom (SEARCH_UNSTABLE);
  failedAtom     = makeTaggedAtom (SEARCH_FAILED);

}

void SolveActor::pushWaitActor (WaitActor *a)
{
  orActors.push ((DLLStackEntry *) a);
}

void SolveActor::pushWaitActorsStackOf (SolveActor *sa)
{
  orActors.pushStack (&(sa->orActors));
}

WaitActor* SolveActor::getDisWaitActor ()
{
  WaitActor *wa = getTopWaitActor ();
  while (wa != (WaitActor *) NULL) {
    if (wa->isCommitted()) {
      unlinkLastWaitActor();
      wa = getNextWaitActor();
      continue;
    }
    Board *bb = wa->getBoardFast();

    if (bb == solveBoard) {
      unlinkLastWaitActor ();
      return (wa);
    } else {
      wa = getNextWaitActor ();
    }
  }
  return ((WaitActor *) NULL);
}

TaggedRef SolveActor::genSolved()
{
  RefsArray contGRegs = allocateRefsArray(1);
  STuple *stuple = STuple::newSTuple(solvedAtom, 2);

  Assert(solveBoard->isSolve());
  contGRegs[0] = makeTaggedConst(solveBoard);
  stuple->setArg(0, makeTaggedConst
                 (new SolvedBuiltin(solvedBITabEntry, contGRegs)));
  stuple->setArg(1, entailedAtom);

  return makeTaggedSTuple(stuple);
}

TaggedRef SolveActor::genStuck()
{
  RefsArray contGRegs = allocateRefsArray(1);
  STuple *stuple = STuple::newSTuple(solvedAtom, 2);

  Assert(solveBoard->isSolve());
  contGRegs[0] = makeTaggedConst(solveBoard);
  stuple->setArg(0, makeTaggedConst
                 (new SolvedBuiltin(solvedBITabEntry, contGRegs)));
  stuple->setArg(1, stableAtom);
  return makeTaggedSTuple(stuple);
}

TaggedRef SolveActor::genChoice(int noOfClauses)
{
  STuple *stuple = STuple::newSTuple(choiceAtom, 2);
  RefsArray contGRegs;

  contGRegs    = allocateRefsArray(1);
  contGRegs[0] = makeTaggedConst(solveBoard);
  Assert(!solveBoard->isCommitted());
  stuple->setArg(0, makeTaggedConst
                 (new OneCallBuiltin(solveContBITabEntry, contGRegs)));
  stuple->setArg(1, makeTaggedSmallInt(noOfClauses));

  return makeTaggedSTuple(stuple);
}

TaggedRef SolveActor::genFailed ()
{
  return failedAtom;
}

TaggedRef SolveActor::genUnstable (TaggedRef arg)
{
  STuple *stuple = STuple::newSTuple(unstableAtom, 1);
  stuple->setArg(0, arg);
  return makeTaggedSTuple (stuple);
}

// private members;
WaitActor* SolveActor::getTopWaitActor()
{
  return ((WaitActor *) orActors.getTop());
}

WaitActor* SolveActor::getNextWaitActor()
{
  return ((WaitActor *) orActors.getNext());
}

void SolveActor::unlinkLastWaitActor ()
{
  orActors.unlinkLast();
}

Bool SolveActor::checkExtSuspList()
{
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
        (thr->isPropagated () && !(thr->isPropagator ()))) {
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    Board *bb = thr->getBoardFast ();

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

  return (suspList == NULL);
}

// Note that there is one thread ALREADY AT THE CREATION TIME!

SolveActor::SolveActor (Board *bb, int prio,
                        TaggedRef resTR, TaggedRef guiTR)
 : Actor (Ac_Solve, bb, prio), result (resTR), guidance (guiTR),
   suspList (NULL), threads (1), stable_sl(NULL)
{
  solveBoard = NULL;
  solveVar   = makeTaggedNULL();
}

void SolveActor::setSolveBoard(Board *bb) {
  solveBoard = bb;
  solveVar   = makeTaggedRef(newTaggedUVar(solveBoard));
}

void SolveActor::add_stable_susp (Thread *thr) {
  stable_sl = new SuspList (thr, stable_sl);
}

// ------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "solve.icc"
#undef inline
#endif
