/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of proper actors
  ------------------------------------------------------------------------
*/


#ifdef __GNUC__
#pragma implementation "actor.hh"
#endif

#include "actor.hh"
#include "am.hh"
#include "board.hh"
#include "tagged.hh"
#include "builtins.hh"
#include "../include/config.h"

/*
 * class Actor:
 *    may be any 'proper' actor;
 *    member data:
 *      flags: see enum ActorFlags
 *      board: board in which actor is installed
 *      priority: if a new thread must be created;
 *
 * class AWActor:
 *    may be conditional or disjunction
 *    member data:
 *      next: continuation for next clause
 *      childCount:
 *
 * class AskActor:
 *    member data
 *      elsePC: programm counter of else
 *
 * class WaitActor
 *    member data
 *      childs: list of childs
 *
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
 *
 */

enum ActorFlags {
  Ac_None       = 0,
  Ac_Ask        = 1<<0,
  Ac_Wait       = 1<<1,
  Ac_Solve      = 1<<2,
  Ac_Committed  = 1<<3,
  Ac_WaitTop    = 1<<4,
  Ac_DisWait    = 1<<5,
};

// ------------------------------------------------------------------------

void AWActor::failChild(Board *n) {
  childCount--;
  if (isWait()) {
    CastWaitActor(this)->failChildInternal(n);
  }
}

void WaitActor::addChildInternal(Board *bb)
{
  if (!childs) {
    childs=(Board **) freeListMalloc(3*sizeof(Board *));
    *childs++ = (Board *) 2;
    childs[0] = bb;
    childs[1] = NULL;
    return;
  }
  int max=(int) childs[-1];
  for (int i = 0; i < max; i++) {
    if (!childs[i]) {
      childs[i] = bb;
      return;
    }
  }
  int size = 2*max;
  Board **cc = (Board **) freeListMalloc((size+1)*sizeof(Board *));
  *cc++ = (Board *) size;
  freeListDispose(childs-1,(((int) childs[-1])+1)*sizeof(Board *));
  childs = cc;
  childs[max] = bb;
  for (int j = max+1; j < size; j++) {
    childs[j] = NULL;
  }
}

void WaitActor::failChildInternal(Board *bb)
{
  int max=(int) childs[-1];
  for (int i = 0; i < max; i++) {
    if (childs[i] == bb) {
      childs[i] = NULL;
      return;
    }
  }
  error("WaitActor::failChildInternal");
}

Board *WaitActor::getChild()
{
  int max=(int) childs[-1];
  for (int i = 0; i < max; i++) {
    if (childs[i]) {
      Board *wb = childs[i];
      childs[i] = (Board *) NULL;
      return (wb);
    }
  }
  error("WaitActor::getChild");
}

// ------------------------------------------------------------------------
//

BuiltinTabEntry *solveContBITabEntry = new BuiltinTabEntry
("*solveCont*", 1, BIsolveCont); // local Entry;
BuiltinTabEntry *solvedBITabEntry = new BuiltinTabEntry
("*solved*", 1, BIsolved);       // local Entry;
TaggedRef SolveContFList = makeTaggedLTuple
(new LTuple(makeTaggedAtom(StatusAtom),makeTaggedAtom(NameOfNil)));
Arity *SolveContArity = SRecord::aritytable.find(SolveContFList);

TaggedRef solvedAtom = makeTaggedAtom (SolvedAtom);
TaggedRef enumedAtom = makeTaggedAtom (EnumedAtom);
TaggedRef lastAtom = makeTaggedAtom (LastAtom);
TaggedRef moreAtom = makeTaggedAtom (MoreAtom);
TaggedRef entailedAtom = makeTaggedAtom (EntailedAtom);
TaggedRef stableAtom = makeTaggedAtom (StableAtom);
TaggedRef failedAtom = makeTaggedAtom (FailedAtom);

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
    if (wa->isCommitted () == OK) {
      unlinkLastWaitActor ();
      wa = getNextWaitActor ();
      continue;
    }
    Board *bb = (wa->getBoard ())->getBoardDeref ();
    if (bb == (Board *) NULL) {
      unlinkLastWaitActor ();
      wa = getNextWaitActor ();
      continue;
    }
    // NB:
    //   We don't test completely whether the actor belongs to the cancelled
    // computation space (since this information is not currently propagated
    // in the computation tree; thus, a board can be marked as non-discarded,
    // though it's actually discarded). There are currently no serious reasons
    // to implement this properly; it can be considered as an optimization.

    if (bb == this) {
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
  RefsArray status = allocateRefsArray (1);
  STuple *stuple = STuple::newSTuple (solvedAtom, 1);

  // statistic
  // am.stat.solveSolved++;

  contGRegs[0] = makeTaggedConst (solveBoard);
  status[0] = entailedAtom;
  stuple->setArg (0, makeTaggedSRecord
                  (new SolvedBuiltin (solvedBITabEntry, contGRegs,
                                      SolveContArity, status)));
  return (makeTaggedSTuple (stuple));
}

TaggedRef SolveActor::genStuck ()
{
  RefsArray contGRegs = allocateRefsArray (1);
  RefsArray status = allocateRefsArray (1);
  STuple *stuple = STuple::newSTuple (solvedAtom, 1);

  contGRegs[0] = makeTaggedConst (solveBoard);
  status[0] = stableAtom;
  stuple->setArg (0, makeTaggedSRecord
                  (new SolvedBuiltin (solvedBITabEntry, contGRegs,
                                      SolveContArity, status)));
  return (makeTaggedSTuple (stuple));
}

TaggedRef SolveActor::genEnumed (Board *newSolveBB)
{
  STuple *stuple = STuple::newSTuple (enumedAtom, 2);
  RefsArray status;
  RefsArray contGRegs;

  // statistics
  // am.stat.solveDistributed++;

  // left side:
  status = allocateRefsArray (1);
  contGRegs = allocateRefsArray (1);
  status[0] = lastAtom;
  contGRegs[0] = makeTaggedConst (newSolveBB);
  stuple->setArg (0, makeTaggedSRecord
                  (new OneCallBuiltin (solveContBITabEntry, contGRegs,
                                       SolveContArity, status)));

  // right side - the rest:
  status = allocateRefsArray (1);
  contGRegs = allocateRefsArray (1);
  if (boardToInstall == (Board *) NULL) {
    status[0] = lastAtom;
  } else {
    status[0] = moreAtom;
  }
  contGRegs[0] = makeTaggedConst (solveBoard);
  stuple->setArg (1, makeTaggedSRecord
                  (new OneCallBuiltin (solveContBITabEntry, contGRegs,
                                       SolveContArity, status)));

  return (makeTaggedSTuple (stuple));
}

TaggedRef SolveActor::genFailed ()
{
  // for statistic purposes
  // am.stat.solveFailed++;
  return (failedAtom);
}

// private members;
WaitActor* SolveActor::getTopWaitActor ()
{
  return (CastWaitActor ((Actor *) orActors.getTop ()));
}

WaitActor* SolveActor::getNextWaitActor ()
{
  return (CastWaitActor ((Actor *) orActors.getNext ()));
}

void SolveActor::unlinkLastWaitActor ()
{
  orActors.unlinkLast ();
}

Bool SolveActor::areNoExtSuspensions ()
{
  SuspList *tmpSuspList = suspList;

  suspList = NULL;
  while (tmpSuspList) {
    Suspension *susp = tmpSuspList->getElem();
    Board *n = (susp->getNode ())->getBoardDeref ();

    SuspList *helpList;

    if (n == (Board *) NULL) {
      susp->markDead ();
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    if (susp->isDead () == OK) {
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    helpList = tmpSuspList;
    tmpSuspList = tmpSuspList->getNext ();
    addSuspension (helpList);
  }

  return ((suspList == NULL) ? OK : NO);
}

//  'BIFun' solveActorWaker;
// No arguments actually, but the type 'BIFun' is fixed;
OZ_Bool solveActorWaker (int n, TaggedRef *args)
{
  DebugCheck ((n != 0), error ("arguments in solveActorWaker?"));
  Board *bb = am.currentBoard;
  DebugCheck ((bb == NULL || bb->isSolve () == NO || bb->isCommitted () == OK ||
               bb->isDiscarded () == OK || bb->isFailed () == OK),
              error ("the blackboard the solveActor is applied to is gone?"));
  SolveActor *sa = CastSolveActor (bb->getActor ());

  sa->decThreads ();      // get rid of threads - '1' in creator;
  if (sa->isStable () == OK) {
    am.pushNervous (bb);  // inderectly - can't say 'goto LBLreduce';
  }
  return (PROCEED);    // always;
}

// ------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "actor.icc"
#undef inline
#endif
