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

/* ------------------------------------------------------------------------
   class WaitActor
   ------------------------------------------------------------------------ */

void WaitActor::addChildInternal(Board *bb)
{
  if (!childs) {
    childs=(Board **) freeListMalloc(3*sizeof(Board *));
    *childs++ = (Board *) 2;
    childs[0] = bb;
    childs[1] = NULL;
    return;
  }
  int maxx=(int) childs[-1];
  for (int i = 0; i < maxx; i++) {
    if (!childs[i]) {
      childs[i] = bb;
      return;
    }
  }
  int size = 2*maxx;
  Board **cc = (Board **) freeListMalloc((size+1)*sizeof(Board *));
  *cc++ = (Board *) size;
  for (i = 0; i < maxx; i++) {
    cc[i] = childs[i];
  }
  freeListDispose(childs-1,(((int) childs[-1])+1)*sizeof(Board *));
  childs = cc;
  childs[maxx] = bb;
  for (i = maxx+1; i < size; i++) {
    childs[i] = NULL;
  }
}

void WaitActor::failChildInternal(Board *bb)
{
  int maxx=(int) childs[-1];
  for (int i = 0; i < maxx; i++) {
    if (childs[i] == bb) {
      for (; i < maxx-1; i++) {    // the order must be preserved (for solve);
        childs[i] = childs[i+1];
      }
      childs[maxx-1] = NULL;
      return;
    }
  }
  error("WaitActor::failChildInternal");
}

Board *WaitActor::getChild()
{
  int maxx=(int) childs[-1];
  for (int i = 0; i < maxx; i++) {
    if (childs[i]) {
      Board *wb = childs[i];
      for (; i < maxx-1; i++) {    // the order must be preserved (for solve);
        childs[i] = childs[i+1];
      }
      childs[maxx-1] = NULL;
      return (wb);
    }
  }
  error("WaitActor::getChild");
  return NULL;
}

Board *WaitActor::getChildRef ()
{
  int maxx=(int) childs[-1];
  for (int i = 0; i < maxx; i++) {
    if (childs[i]) {
      return (childs[i]);
    }
  }
  error("WaitActor::getChild");
  return NULL;
}

/* ------------------------------------------------------------------------
   class SolveActor
   ------------------------------------------------------------------------ */

BuiltinTabEntry *solveContBITabEntry = NULL;
BuiltinTabEntry *solvedBITabEntry    = NULL;
Arity *SolveContArity                = NULL;

TaggedRef solvedAtom;
TaggedRef enumedAtom;
TaggedRef lastAtom;
TaggedRef moreAtom;
TaggedRef entailedAtom;
TaggedRef stableAtom;
TaggedRef failedAtom;

void SolveActor::Init()
{
  solveContBITabEntry
    = new BuiltinTabEntry("*solveCont*", 1, BIsolveCont); // local Entry;
  solvedBITabEntry
    = new BuiltinTabEntry("*solved*", 1, BIsolved);       // local Entry;

  TaggedRef solveContFList = cons(makeTaggedAtom(SEARCH_STATUS),nil());
  SolveContArity = SRecord::aritytable.find(solveContFList);

  solvedAtom     = makeTaggedAtom (SEARCH_SOLVED);
  enumedAtom     = makeTaggedAtom (SEARCH_ENUMED);
  lastAtom       = makeTaggedAtom (SEARCH_LAST);
  moreAtom       = makeTaggedAtom (SEARCH_MORE);
  entailedAtom   = makeTaggedAtom (SEARCH_ENTAILED);
  stableAtom     = makeTaggedAtom (SEARCH_STABLE);
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
  RefsArray status = allocateRefsArray (1);
  STuple *stuple = STuple::newSTuple (solvedAtom, 1);

  // statistic
  am.stat.solveSolved++;

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
  am.stat.solveDistributed++;

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
    status[0] = moreAtom;
  } else {
    status[0] = lastAtom;
  }
  contGRegs[0] = makeTaggedConst (solveBoard);
  stuple->setArg (1, makeTaggedSRecord
                  (new OneCallBuiltin (solveContBITabEntry, contGRegs,
                                       SolveContArity, status)));

  return (makeTaggedSTuple (stuple));
}

TaggedRef SolveActor::genEnumedFail ()
{
  STuple *stuple = STuple::newSTuple (enumedAtom, 2);
  RefsArray status;
  RefsArray contGRegs;

  // statistics
  am.stat.solveDistributed++;

  // left side:
  status = allocateRefsArray (1);
  contGRegs = allocateRefsArray (1);
  status[0] = lastAtom;
  contGRegs[0] = makeTaggedConst (solveBoard);
  stuple->setArg (0, makeTaggedSRecord
                  (new OneCallBuiltin (solveContBITabEntry, contGRegs,
                                       SolveContArity, status)));

  // right side - the rest:
  status = allocateRefsArray (1);
  contGRegs = allocateRefsArray (1);
  status[0] = lastAtom;
  contGRegs[0] = makeTaggedConst (solveBoard);  // but it has no impact;
  OneCallBuiltin *bi = new OneCallBuiltin (solveContBITabEntry, contGRegs,
                                           SolveContArity, status);
  bi->hasSeen ();
  stuple->setArg (1, makeTaggedSRecord (bi));

  return (makeTaggedSTuple (stuple));
}

TaggedRef SolveActor::genFailed ()
{
  // for statistic purposes
  am.stat.solveFailed++;
  return (failedAtom);
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

    if (susp->isDead () == OK) {
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    Board *b = (susp->getNode ())->getBoardDeref ();

    if (b == (Board *) NULL) {
      susp->markDead ();
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    Board *sb = b->getSolveBoard ();
    while (sb != (Board *) NULL && sb != solveBoard)
      sb = (sb->getParentBoard ())->getSolveBoard ();
    if (sb == (Board *) NULL) {
      // note that the board *b could be discarded; therefore
      // it is needed to try to find its solve board;
      susp->markDead ();
      tmpSuspList = tmpSuspList->dispose ();
      continue;
    }

    SuspList *helpList;
    helpList = tmpSuspList;
    tmpSuspList = tmpSuspList->getNext ();
    addSuspension (helpList);
  }

  return ((suspList == NULL) ? OK : NO);
}

//  'OZ_CFun' solveActorWaker;
// No arguments actually, but the type 'OZ_CFun' is fixed;
OZ_Bool SolveActor::Waker (int n, TaggedRef *args)
{
  DebugCheck ((n != 0), error ("arguments in SolveActor::Waker?"));
  Board *bb = am.currentBoard;
  DebugCheck ((bb == NULL || bb->isSolve () == NO
               || bb->isCommitted () == OK ||
               bb->isDiscarded () == OK || bb->isFailed () == OK),
              error ("the blackboard the solveActor is applied to is gone?"));
  SolveActor *sa = SolveActor::Cast(bb->getActor ());
  // DebugCheckT (message ("SolveActor::Waker (@0x%x)\n", (void *) sa));

  DebugCheck ((bb->isReflected () == OK),
              error ("already reflected board in SolveActor::Waker"));
  sa->decThreads ();      // get rid of threads - '1' in creator;
  // after return we are going to the "reduce" state,
  // so reduce the actor if possible;
  return (PROCEED);    // always;
}


// Note that there is one thread ALREADY AT THE CREATION TIME!

SolveActor::SolveActor (Board *bb, int prio, TaggedRef resTR)
 : Actor (Ac_Solve, bb, prio), result (resTR),
   boardToInstall(NULL), suspList (NULL), threads (1)
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
  boardToInstall = (Board *) NULL;
  suspList = (SuspList *) NULL;
  threads = 0;
}

Bool SolveActor::isStable ()
{
  if (threads != 0)
    return (NO);
  if (solveBoard == am.currentBoard && am.trail.isEmptyChunk () == NO)
    return (NO);
  // simply "don't worry" if in all other cases it is too weak;
  return (areNoExtSuspensions ());
}

// ------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "actor.icc"
#undef inline
#endif
