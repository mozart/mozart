/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Christian Schulte (schulte@dfki.de)
 * 
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Copyright:
 *    Konstantin Popov, 1998
 *    Christian Schulte, 1998
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

// Solver

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "solve.hh"
#endif

#include "solve.hh"
#include "space.hh"
#include "thr_int.hh"
#include "board.hh"
#include "var_base.hh"
#include "builtins.hh"

#ifdef OUTLINE
#include "solve.icc"
#endif

// from prop_int.cc
void oz_pushToLPQ(Board *bb, Propagator * prop);

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

SolveActor::SolveActor(Board *bb)
 : Actor (Ac_Solve, bb), cpb(NULL), suspList (NULL), threads (0), 
   nonMonoSuspList(NULL) {
  result     = oz_newVar(bb);
  solveBoard = new Board(this, Bo_Solve);
  solveVar   = oz_newVar(solveBoard);
#ifdef CS_PROFILE
  orig_start  = (int32 *) NULL;
  copy_start  = (int32 *) NULL;
  copy_size   = 0;
#endif
}

#ifdef CS_PROFILE
TaggedRef SolveActor::getCloneDiff(void) {
  TaggedRef l = oz_nil();
  
  if (copy_start && orig_start && (copy_size>0)) {
    int n = 0;

    while (n < copy_size) {
      if (copy_start[n] != orig_start[n]) {
	int d = 0;
	
	while ((n < copy_size) && (copy_start[n] != orig_start[n])) {
	  d++; n++;
	}
      
	LTuple *lt = new LTuple();
	lt->setHead(newSmallInt(d));
	lt->setTail(l);
	l = makeTaggedLTuple(lt);
      } else {
	n++;
      }
      
    }

    free(copy_start);

    TaggedRef ret = OZ_pair2(newSmallInt(copy_size),l);

    copy_start = (int32 *) 0;
    copy_size  = 0;
    orig_start = (int32 *) 0;

    return ret;
    
  } else {
    return OZ_pair2(newSmallInt(0),l);
  }

}
#endif

//-----------------------------------------------------------------------------
// support for nonmonotonic propagators

void SolveActor::addToNonMonoSuspList(Propagator * prop)
{
  nonMonoSuspList = nonMonoSuspList->insert(prop);
}

void SolveActor::mergeNonMonoSuspListWith(OrderedSuspList * p)
{
  for (; p != NULL; p = p->getNext())
    nonMonoSuspList = nonMonoSuspList->insert(p->getPropagator()); 
}

//-----------------------------------------------------------------------------

void oz_solve_scheduleNonMonoSuspList(SolveActor *sa)
{
#ifdef DEBUG_NONMONOTONIC
  printf("------------------------------------------------------------------"
	 "\nSolveActor::scheduleNonMonoSuspList\n"); fflush(stdout);
#endif

  for (OrderedSuspList * p = sa->getNonMonoSuspList();
       p != NULL;
       p = p->getNext()) {
    Propagator * prop = p->getPropagator();

#ifdef DEBUG_NONMONOTONIC
    OZ_PropagatorProfile * header = prop->getPropagator()->getProfile();
    char * pn = header->getPropagatorName();
    printf("<%s %d>\n", pn,
	   prop->getPropagator()->getOrder()); 
#endif
    
    oz_pushToLPQ(GETBOARD(prop),prop);
  }

  sa->setNonMonoSuspList(NULL);

#ifdef DEBUG_NONMONOTONIC
  printf("Done\n"); fflush(stdout);
#endif
}

// ---------------------------------------------------------------------
// Spaces
// ---------------------------------------------------------------------

#define declareSpace()					\
  OZ_Term tagged_space = OZ_in(0);			\
  DEREF(tagged_space, space_ptr, space_tag);		\
  if (isVariableTag(space_tag))				\
    oz_suspendOn(makeTaggedRef(space_ptr));		\
  if (!oz_isSpace(tagged_space))			\
    oz_typeError(0, "Space");				\
  Space *space = (Space *) tagged2Const(tagged_space);



inline
void oz_solve_inject(TaggedRef var, int prio, TaggedRef proc,
		     Board *solveBoard)
{
  // thread creation for {proc root}
  RefsArray args = allocateRefsArray(1, NO);
  args[0] = var;

  Thread *it = oz_newThreadInject(solveBoard,prio);
  it->pushCall(proc, args, 1);
}

inline
OZ_Term oz_solve_merge(SolveActor *solveActor, Board *bb, int sibling)
{
  Board *solveBoard=solveActor->getSolveBoard();
  oz_merge(solveBoard,bb,solveBoard->getSuspCount());

  if (!oz_installScript(solveBoard->getScriptRef())) {
    return makeTaggedNULL();
  }
  
  Assert(oz_isCurrentBoard(bb));
  
  solveActor->mergeCPB(bb,sibling);
  solveActor->mergeNonMono(am.currentSolveBoard());
  return solveActor->getSolveVar();
}

inline
Board *oz_solve_clone(SolveActor *sa, Board *bb) {
  ozstat.incSolveCloned();
  Bool testGround;
  Board *copy = (Board *) am.copyTree(sa->getSolveBoard(), &testGround);
  SolveActor *ca = SolveActor::Cast(copy->getActor());
 
  ca->setBoard(bb);

  if (testGround == OK) {
    ca->setGround();
    sa->setGround();
  }

#ifdef CS_PROFILE
  ca->orig_start = cs_orig_start;
  ca->copy_start = cs_copy_start;
  ca->copy_size  = cs_copy_size;
#endif 

  return copy;
}

OZ_BI_define(BInewSpace, 1,1) {
  OZ_Term proc = OZ_in(0);

  DEREF(proc, proc_ptr, proc_tag);
  if (isVariableTag(proc_tag)) 
    oz_suspendOn(makeTaggedRef(proc_ptr));

  if (!oz_isProcedure(proc))
    oz_typeError(0, "Procedure");

  Board* CBB = oz_currentBoard();

  ozstat.incSolveCreated();
  // creation of solve actor and solve board
  SolveActor *sa = new SolveActor(CBB);

  // thread creation for {proc root}
  oz_solve_inject(sa->getSolveVar(),DEFAULT_PRIORITY, proc,
		  sa->getSolveBoard());
    
  // create space
  OZ_result(makeTaggedConst(new Space(CBB,sa->getSolveBoard())));
  return BI_PREEMPT;

} OZ_BI_end


OZ_BI_define(BIisSpace, 1,1) {
  OZ_Term tagged_space = OZ_in(0);

  DEREF(tagged_space, space_ptr, space_tag);

  if (isVariableTag(space_tag))
    oz_suspendOn(makeTaggedRef(space_ptr));

  OZ_RETURN(oz_bool(oz_isSpace(tagged_space)));
} OZ_BI_end


OZ_BI_define(BIaskSpace, 1,1) {
  declareSpace();

  // mm2: dead code
  if (space->isProxy()) Assert(0);

  if (space->isFailed()) OZ_RETURN(AtomFailed);
  
  if (space->isMerged()) OZ_RETURN(AtomMerged);
  
  TaggedRef answer = space->getSolveActor()->getResult();
  
  DEREF(answer, answer_ptr, answer_tag);

  if (isVariableTag(answer_tag))
    oz_suspendOn(makeTaggedRef(answer_ptr));
  
  OZ_RETURN((oz_isSTuple(answer) && 
	     oz_eq(tagged2SRecord(answer)->getLabel(), 
		   AtomSucceeded))
	    ? AtomSucceeded : answer);
		  
} OZ_BI_end


Bool oz_solve_isBlocked(SolveActor *sa)
{
  return ((sa->getThreads()==0) && !oz_isStableSolve(sa));
}

OZ_BI_define(BIaskVerboseSpace, 2,0) {
  declareSpace();
  oz_declareIN(1,out);

  if (space->isProxy()) Assert(0);

  if (space->isFailed())
    return oz_unify(out, AtomFailed);
  
  if (space->isMerged())
    return oz_unify(out, AtomMerged);

  if (oz_solve_isBlocked(space->getSolveActor())) {
    SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
    stuple->setArg(0, am.currentUVarPrototype());

    if (oz_unify(out, makeTaggedSRecord(stuple)) == FAILED) // mm2
      return FAILED;

    OZ_in(1) = stuple->getArg(0);
  } 
  
  TaggedRef answer = space->getSolveActor()->getResult();
  
  DEREF(answer, answer_ptr, answer_tag);

  if (isVariableTag(answer_tag))
    oz_suspendOn(makeTaggedRef(answer_ptr));

  return oz_unify(OZ_in(1), answer);
} OZ_BI_end


OZ_BI_define(BImergeSpace, 1,1) {
  declareSpace();

  if (space->isProxy()) Assert(0);

  if (space->isMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  if (space->isFailed())
    return FAILED;

  Board *CBB = oz_currentBoard();
  Board *SBB = space->getSolveBoard()->derefBoard();
  Board *SBP = SBB->getParent()->derefBoard();

  /* There can be two different situations during merging:
   *  1) SBB is subordinated to CBB:          CBB  <-+
   *                                           |     |
   *                                          SBB   -+
   *   
   *   
   *  2) SBB is a sibling of CBB:            parent
   *                                          /   \
   *                                        CBB   SBB
   *                                         ^     |
   *                                         +-----+
   */

  Assert(CBB == CBB->derefBoard());

  Bool isSibling = (!oz_isRootBoard(CBB) && 
		    CBB->getParent()->derefBoard() == SBP &&
		    CBB != SBB);

  if (!isSibling && CBB != SBP)
    return oz_raise(E_ERROR,E_KERNEL,"spaceSuper",1,tagged_space);
      
  Assert(!oz_isBelow(CBB,SBB));
  
  TaggedRef result = space->getSolveActor()->getResult();

  if (result == makeTaggedNULL())
    return FAILED;

  if (OZ_isVariable(result)) {

    if (isSibling) {
      switch (oz_installPath(SBP)) {
      case INST_FAILED: case INST_REJECTED: return FAILED;
      case INST_OK: break;
      }

      if (oz_unify(result, AtomMerged) == FAILED) // mm2
	return FAILED;

      switch (oz_installPath(CBB)) {
      case INST_FAILED: case INST_REJECTED: return FAILED;
      case INST_OK: break;
      }

    } else {
      if (oz_unify(result, AtomMerged) == FAILED) // mm2
	return FAILED;
    }
  }


  TaggedRef root = oz_solve_merge(space->getSolveActor(), CBB, isSibling);
  space->merge();

  if (root == makeTaggedNULL())
    return FAILED;

  OZ_RETURN(root);
} OZ_BI_end


OZ_BI_define(BIcloneSpace, 1,1) {
  declareSpace();

  if (space->isProxy()) Assert(0);

  if (space->isMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  Board* CBB = oz_currentBoard();

  if (space->isFailed())
    OZ_RETURN(makeTaggedConst(new Space(CBB, (Board *) 0)));

  TaggedRef result = space->getSolveActor()->getResult();
  
  DEREF(result, result_ptr, result_tag);
  if (isVariableTag(result_tag)) 
    oz_suspendOn(makeTaggedRef(result_ptr));

  OZ_RETURN(makeTaggedConst(new Space(CBB,oz_solve_clone(space->getSolveActor(),CBB))));

} OZ_BI_end


OZ_BI_define(BIcommitSpace, 2,0) {
  declareSpace();
  oz_declareIN(1,choice);

  if (space->isProxy()) Assert(0);

  if (space->isMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  if (space->isFailed())
    return PROCEED;

  TaggedRef result = space->getSolveActor()->getResult();
  
  DEREF(result, result_ptr, result_tag);
  if (isVariableTag(result_tag)) 
    oz_suspendOn(makeTaggedRef(result_ptr));

  DEREF(choice, choice_ptr, choice_tag);

  if (isVariableTag(choice_tag))
    oz_suspendOn(makeTaggedRef(choice_ptr));

  TaggedRef left, right;

  if (isSmallIntTag(choice_tag)) {
    left  = choice;
    right = choice;
  } else if (oz_isSTuple(choice) &&
	     oz_eq(AtomPair,
		   tagged2SRecord(choice)->getLabel()) &&
	     tagged2SRecord(choice)->getWidth() == 2) {
    left  = tagged2SRecord(choice)->getArg(0);
    DEREF(left, left_ptr, left_tag);

    if (isVariableTag(left_tag))
      oz_suspendOn(makeTaggedRef(left_ptr));

    right = tagged2SRecord(choice)->getArg(1);
    
    DEREF(right, right_ptr, right_tag);

    if (isVariableTag(right_tag))
      oz_suspendOn(makeTaggedRef(right_ptr));
  } else {
    oz_typeError(1, "Integer or pair of integers");
  }

  if (!oz_isCurrentBoard(space->getSolveBoard()->getParent()))
    return oz_raise(E_ERROR,E_KERNEL,"spaceParent",1,tagged_space);

  int l = smallIntValue(left) - 1;
  int r = smallIntValue(right) - 1;

  SolveActor *sa = space->getSolveActor();

  WaitActor *wa = sa->select(l,r);
  
  if (!wa)
    return oz_raise(E_ERROR,E_KERNEL,"spaceNoChoice",1,tagged_space);

  int n = wa->getChildCount();

  if (n>1) {
    sa->patchChoiceResult(n);

    return PROCEED;
  }

  if (n==1)
    sa->removeChoice();

  Thread *tt = wa->getThread();

  sa->unsetGround();
  sa->clearResult(GETBOARD(space));

  oz_wakeupThread(tt);

  return BI_PREEMPT;
} OZ_BI_end


OZ_BI_define(BIinjectSpace, 2,0)
{
  declareSpace();
  oz_declareIN(1,proc);

  if (space->isProxy()) Assert(0);

  if (space->isMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  // Check whether space is failed!
  if (space->isFailed())
    return PROCEED;

  if (!oz_isCurrentBoard(space->getSolveBoard()->getParent())) 
    return oz_raise(E_ERROR,E_KERNEL,"spaceParent", 1, tagged_space);

  DEREF(proc, proc_ptr, proc_tag);

  if (isVariableTag(proc_tag)) 
    oz_suspendOn(makeTaggedRef(proc_ptr));

  if (!oz_isProcedure(proc))
    oz_typeError(1, "Procedure");

  Board      *sb = space->getSolveBoard();
  SolveActor *sa = space->getSolveActor();

  // clear status
  sa->unsetGround();
  sa->clearResult(GETBOARD(space));

  // inject
  oz_solve_inject(sa->getSolveVar(),DEFAULT_PRIORITY, proc,
		  sa->getSolveBoard());
    
  return BI_PREEMPT;
} OZ_BI_end


#ifdef CS_PROFILE
OZ_BI_define(BIgetCloneDiff, 1,1) {
  declareSpace();

  if (space->isMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  OZ_RETURN(space->getSolveActor()->getCloneDiff());
} OZ_BI_end

#endif

