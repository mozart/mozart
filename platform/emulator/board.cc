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

#if defined(INTERFACE)
#pragma implementation "board.hh"
#endif

#include "board.hh"
#include "thr_int.hh"
#include "space.hh"
#include "builtins.hh"
#include "value.hh"

#ifdef OUTLINE
#include "board.icc"
#endif

// from prop_int.cc
void oz_pushToLPQ(Board *bb, Propagator * prop);

Equation *ScriptAllocate(int size)
{
  return (Equation *) freeListMalloc(size * sizeof(Equation));
}

void ScriptDealloc(Equation *ptr, int size)
{
  if (ptr == (Equation *)0)
    return;
  freeListDispose(ptr,size * sizeof(Equation));
}

Script::Script(int sizeInit)
{
  first = ScriptAllocate(sizeInit);
  numbOfCons = sizeInit;
}

Script::~Script()
{
  ScriptDealloc(first,numbOfCons);
}

void Script::allocate(int sizeInit)
{
  if (sizeInit != 0)
    first = ScriptAllocate(sizeInit);
  else
    first = (Equation *)NULL;
  numbOfCons = sizeInit;
}

void Script::dealloc()
{
  if (numbOfCons != 0) {
    ScriptDealloc(first,numbOfCons);
    first = (Equation *)NULL;
    numbOfCons = 0;
  }
}



/*
 * return solve board of this
 *   0, if no solve board found
 *      or discard or fail detected below a solve board
 *
Board* Board::getSolveBoard ()
{
  Assert(!isCommitted());
  Board *bb;
  for (bb=this;
       bb!=0 && bb->isRoot();
       bb=bb->getParentAndTest()) {}
  return bb;
}

*/

Board::Board(Board * p) 
  : localPropagatorQueue(0), gcField(0), flags(0), suspCount(0), bag(0),
    threads(0), suspList(0), nonMonoSuspList(0)
{
  Assert(p==NULL || !p->isCommitted());
  parent = p;
  if (p) {
    result     = oz_newVar(p);
    solveVar   = oz_newVar(this);
  } else {
    solveVar = result = oz_nil();
  }
#ifdef CS_PROFILE
  orig_start  = (int32 *) NULL;
  copy_start  = (int32 *) NULL;
  copy_size   = 0;
#endif
}


/*
 * Before copying all spaces but the space to be copied get marked.
 */
void Board::setGlobalMarks(void) {
  Assert(!isRoot());

  Board * b = this;

  do {
    b = b->getParent(); b->setGlobalMark();
  } while (!b->isRoot());
  
}

/*
 * Purge marks after copying
 */
void Board::unsetGlobalMarks(void) {
  Assert(!isRoot());

  Board * b = this;

  do {
    b = b->getParent(); b->unsetGlobalMark();
  } while (!b->isRoot());

}



#ifdef DEBUG_CHECK
/*
 * Check if a board is alive.
 * NOTE: this test can be very expensive !!!
 */
Bool Board::checkAlive() {

  /*
  Board *bb=this;

  while (!bb->isRoot()) {
loop:
  Assert(!bb->isCommitted());
  if (bb->isFailed()) return NO;
  if (bb->isRoot()) return OK;
  Actor *aa=bb->getActor();
  if (aa->isCommitted()) return NO;
  bb=GETBOARD(aa);
  goto loop;

  */

  return OK;
}
#endif


#ifdef CS_PROFILE
TaggedRef Board::getCloneDiff(void) {
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

//
// Status variable
//

TaggedRef Board::genSolved() {
  ozstat.incSolveSolved();
  SRecord *stuple = SRecord::newSRecord(AtomSucceeded, 1);

  stuple->setArg(0, AtomEntailed);
  
  return makeTaggedSRecord(stuple);
}

TaggedRef Board::genStuck() {
  SRecord *stuple = SRecord::newSRecord(AtomSucceeded, 1);
  
  stuple->setArg(0, AtomSuspended);
  return makeTaggedSRecord(stuple);
}

TaggedRef Board::genChoice(int noOfClauses) {
  SRecord *stuple = SRecord::newSRecord(AtomAlt, 1);

  Assert(!isCommitted());
  stuple->setArg(0, makeTaggedSmallInt(noOfClauses));

  return makeTaggedSRecord(stuple);
}

TaggedRef Board::genFailed() {
  ozstat.incSolveFailed();
  return AtomFailed;
}

TaggedRef Board::genUnstable(TaggedRef arg) {
  SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
  stuple->setArg(0, arg);
  return makeTaggedSRecord(stuple);
}

//-----------------------------------------------------------------------------
// support for nonmonotonic propagators

void Board::addToNonMonoSuspList(Propagator * prop)
{
  nonMonoSuspList = nonMonoSuspList->insert(prop);
}

void Board::mergeNonMonoSuspListWith(OrderedSuspList * p)
{
  for (; p != NULL; p = p->getNext())
    nonMonoSuspList = nonMonoSuspList->insert(p->getPropagator()); 
}

//-----------------------------------------------------------------------------

void oz_solve_scheduleNonMonoSuspList(Board * b) {

  for (OrderedSuspList * p = b->getNonMonoSuspList();
       p != NULL;
       p = p->getNext()) {
    Propagator * prop = p->getPropagator();

    oz_pushToLPQ(GETBOARD(prop),prop);
  }
  
  b->setNonMonoSuspList(NULL);

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
OZ_Term oz_solve_merge(Board *solveBoard, Board *bb, int sibling) {
  
  bb->setLocalPropagatorQueue(solveBoard->
			      getLocalPropagatorQueue()->
			      merge(bb->getLocalPropagatorQueue()));
  solveBoard->setCommittedBoard(bb);
  bb->incSuspCount(solveBoard->getSuspCount());
  
  if (!oz_installScript(solveBoard->getScriptRef())) {
    return makeTaggedNULL();
  }
  
  Assert(oz_isCurrentBoard(bb));
  
  if (!bb->isRoot()) {
    bb->mergeDistributors(solveBoard->getBag());

    if (sibling)
      bb->incThreads(solveBoard->getThreads());

    solveBoard->mergeNonMono(bb);
  }


  return solveBoard->getSolveVar();
}

inline
Board *oz_solve_clone(Board *sb, Board *bb) {
  ozstat.incSolveCloned();
  Board *copy = (Board *) am.copyTree(sb);

#ifdef CS_PROFILE
  copy->orig_start = cs_orig_start;
  copy->copy_start = cs_copy_start;
  copy->copy_size  = cs_copy_size;
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
  // creation of solve board
  Board *sb = new Board(CBB);

  // thread creation for {proc root}
  oz_solve_inject(sb->getSolveVar(),DEFAULT_PRIORITY, proc, sb);
    
  // create space
  OZ_result(makeTaggedConst(new Space(CBB,sb)));
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
  
  TaggedRef answer = space->getSolveBoard()->getResult();
  
  DEREF(answer, answer_ptr, answer_tag);

  if (isVariableTag(answer_tag))
    oz_suspendOn(makeTaggedRef(answer_ptr));
  
  OZ_RETURN((oz_isSTuple(answer) && 
	     oz_eq(tagged2SRecord(answer)->getLabel(), 
		   AtomSucceeded))
	    ? AtomSucceeded : answer);
		  
} OZ_BI_end


Bool oz_solve_isBlocked(Board * b) {
  return ((b->getThreads()==0) && !oz_isStableSolve(b));
}

OZ_BI_define(BIaskVerboseSpace, 2,0) {
  declareSpace();
  oz_declareIN(1,out);
  
  if (space->isFailed())
    return oz_unify(out, AtomFailed);
  
  if (space->isMerged())
    return oz_unify(out, AtomMerged);

  if (oz_solve_isBlocked(space->getSolveBoard())) {
    SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
    stuple->setArg(0, am.currentUVarPrototype());

    if (oz_unify(out, makeTaggedSRecord(stuple)) == FAILED)
      return FAILED;

    OZ_in(1) = stuple->getArg(0);
  } 
  
  TaggedRef answer = space->getSolveBoard()->getResult();
  
  DEREF(answer, answer_ptr, answer_tag);

  if (isVariableTag(answer_tag))
    oz_suspendOn(makeTaggedRef(answer_ptr));

  return oz_unify(OZ_in(1), answer);
} OZ_BI_end


OZ_BI_define(BIaskUnsafeSpace, 2,0) {
  declareSpace();
  oz_declareIN(1,out);

  if (space->isProxy()) Assert(0);

  if (space->isFailed())
    return oz_unify(out, AtomFailed);
  
  if (space->isMerged())
    return oz_unify(out, AtomMerged);

  if (oz_solve_isBlocked(space->getSolveBoard())) {
    SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
    stuple->setArg(0, am.currentUVarPrototype());

    if (oz_unify(out, makeTaggedSRecord(stuple)) == FAILED) // mm2
      return FAILED;

    OZ_in(1) = stuple->getArg(0);
  } 
  
  TaggedRef answer = space->getSolveBoard()->getResult();
  
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
  
  TaggedRef result = space->getSolveBoard()->getResult();

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


  TaggedRef root = oz_solve_merge(space->getSolveBoard(), CBB, isSibling);
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

  TaggedRef result = space->getSolveBoard()->getResult();
  
  DEREF(result, result_ptr, result_tag);
  if (isVariableTag(result_tag)) 
    oz_suspendOn(makeTaggedRef(result_ptr));

  OZ_RETURN(makeTaggedConst(new Space(CBB,oz_solve_clone(space->getSolveBoard(),CBB))));

} OZ_BI_end


OZ_BI_define(BIcommitSpace, 2,0) {
  declareSpace();
  oz_declareIN(1,choice);

  if (space->isProxy()) Assert(0);

  if (space->isMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  if (space->isFailed())
    return PROCEED;

  TaggedRef result = space->getSolveBoard()->getResult();
  
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

  Board * sb = space->getSolveBoard()->derefBoard();

  Distributor * d = sb->getDistributor();

  if (!d)
    return oz_raise(E_ERROR,E_KERNEL,"spaceNoChoice",1,tagged_space);

  int n = d->commit(sb,
		    smallIntValue(left),
		    smallIntValue(right));
    
  if (n>1) {
    sb->patchChoiceResult(n);

    return PROCEED;
  }
    
  sb->clearResult();

  return BI_PREEMPT;
} OZ_BI_end


OZ_BI_define(BIinjectSpace, 2,0)
{
  declareSpace();
  oz_declareIN(1,proc);

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

  Board *sb = space->getSolveBoard();

  // clear status
  sb->clearResult();

  // inject
  oz_solve_inject(sb->getSolveVar(),DEFAULT_PRIORITY, proc, sb);
    
  return BI_PREEMPT;
} OZ_BI_end


OZ_BI_define(BIregisterSpace, 1, 1) {
  oz_declareIntIN(0, i);

  Board * bb = oz_currentBoard();

  if (bb->isRoot()) {
    OZ_out(0) = oz_newVar(bb);
  } else {
    BaseDistributor * bd = new BaseDistributor(bb,i);

    bb->addDistributor(bd);

    OZ_out(0) = bd->getVar();
  }

  return BI_PREEMPT;
} OZ_BI_end


#ifdef CS_PROFILE
OZ_BI_define(BIgetCloneDiff, 1,1) {
  declareSpace();

  if (space->isMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  OZ_RETURN(space->getSolveBoard()->getCloneDiff());
} OZ_BI_end

#endif


// eof
//-----------------------------------------------------------------------------


