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

#if defined(__GNUC__) && !defined(NOPRAGMA)
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
 *      suspList: list of external suspensions; 
 *      threads: the number of _active_ threads
 *         (recall that all threads (in emulator's sense) are 'active' ones
 *          in kernel definition's sense);
 */

/* ------------------------------------------------------------------------
   class SolveActor
   ------------------------------------------------------------------------ */

BuiltinTabEntry *solveContBITabEntry = NULL;
BuiltinTabEntry *solvedBITabEntry    = NULL;

Arity *SolveContArity                = NULL;

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

  TaggedRef solveContFList = cons(makeTaggedAtom(SEARCH_STATUS),nil());
  SolveContArity = aritytable.find(solveContFList);

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

TaggedRef SolveActor::genSolved ()
{
  RefsArray contGRegs = allocateRefsArray (1);
  STuple *stuple = STuple::newSTuple (solvedAtom, 1);

  Assert(solveBoard->isSolve());
  contGRegs[0] = makeTaggedConst (solveBoard);
  stuple->setArg (0, makeTaggedConst
		  (new SolvedBuiltin (solvedBITabEntry, contGRegs,
				      SolveContArity, entailedAtom)));
  return (makeTaggedSTuple (stuple));
}

TaggedRef SolveActor::genStuck ()
{
  RefsArray contGRegs = allocateRefsArray (1);
  STuple *stuple = STuple::newSTuple (solvedAtom, 1);

  Assert(solveBoard->isSolve());
  contGRegs[0] = makeTaggedConst (solveBoard);
  stuple->setArg (0, makeTaggedConst
		  (new SolvedBuiltin (solvedBITabEntry, contGRegs,
				      SolveContArity, stableAtom)));
  return (makeTaggedSTuple (stuple));
}

TaggedRef SolveActor::genChoice (int noOfClauses)
{
  STuple *stuple = STuple::newSTuple (choiceAtom, 2);
  RefsArray contGRegs;

  // left side: 
  contGRegs = allocateRefsArray (1);
  contGRegs[0] = makeTaggedConst (solveBoard);
  stuple->setArg (0, makeTaggedConst
		  (new OneCallBuiltin (solveContBITabEntry, contGRegs)));

  stuple->setArg (1, makeTaggedSmallInt(noOfClauses));

  return (makeTaggedSTuple (stuple));
}

TaggedRef SolveActor::genFailed ()
{
  return (failedAtom);
}

TaggedRef SolveActor::genUnstable (TaggedRef arg)
{
  STuple *stuple = STuple::newSTuple (unstableAtom, 1);
  stuple->setArg (0, arg);
  return (makeTaggedSTuple (stuple));
}

// private members; 
WaitActor* SolveActor::getTopWaitActor ()
{
  return ((WaitActor *) orActors.getTop ());
}

WaitActor* SolveActor::getNextWaitActor ()
{
  return ((WaitActor *) orActors.getNext ());
}

void SolveActor::unlinkLastWaitActor ()
{
  orActors.unlinkLast ();
}

Bool SolveActor::checkExtSuspList ()
{
  SuspList *tmpSuspList = suspList;

  suspList = NULL;
  while (tmpSuspList) {
    Suspension *susp = tmpSuspList->getElem();

    if (susp->isDead ()) {
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    Board *bb = susp->getBoardFast();

    while (1) {
      bb = bb->getSolveBoard();
      if (bb == solveBoard) break;
      if (bb == 0) break;
      bb = bb->getParentAndTest();
      if (bb == 0) break;
    }

    if (bb == 0) {
      susp->markDead ();
      tmpSuspList = tmpSuspList->dispose ();
    } else {
      SuspList *helpList = tmpSuspList;
      tmpSuspList = tmpSuspList->getNext ();
      addSuspension (helpList);
    }
  }

  return (suspList == NULL);
}

// Note that there is one thread ALREADY AT THE CREATION TIME!

SolveActor::SolveActor (Board *bb, int prio, int compMode,
			TaggedRef resTR, TaggedRef guiTR)
 : Actor (Ac_Solve, bb, prio, compMode), result (resTR), guidance (guiTR), 
   suspList (NULL), threads (1), stable_sl(NULL)
{
  solveBoard = NULL;
  solveVar= makeTaggedNULL();
}

void SolveActor::setSolveBoard(Board *bb) {
  solveBoard = bb;
  solveVar = makeTaggedRef(newTaggedUVar (solveBoard));
}

SolveActor::~SolveActor()
{
  solveBoard = (Board *) NULL;
  orActors.clear (); 
  solveVar = (TaggedRef) NULL;
  result = (TaggedRef) NULL;
  guidance = (TaggedRef) NULL;
  suspList = (SuspList *) NULL;
  threads = 0; 
}

void SolveActor::printDebugKP(void)
{
  cout << endl << "solveActor @" << this << endl;
  cout << "solveVar="; taggedPrint(solveVar); cout << endl;
  cout << "result="; taggedPrint(result); cout << endl;
  cout << "threads=" << threads << endl;
  suspList->print(cout);
  cout.flush();
}


Bool SolveActor::stable_wake(void) {
  if (stable_sl) {
    stable_sl = stable_sl->stable_wake();
    return TRUE;
  }
  return FALSE;
}


void SolveActor::add_stable_susp(Suspension * s) {
  stable_sl = new SuspList(s, stable_sl);
}

// ------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "solve.icc"
#undef inline
#endif
