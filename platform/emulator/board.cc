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
#include "var_base.hh"

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


Board::Board(Board * p)
  : localPropagatorQueue(0), flags(0), suspCount(0), bag(0),
    threads(0), suspList(0), nonMonoSuspList(0)
{
  Assert(p==NULL || !p->isCommitted());
  parent = p;
  if (p) {
    result  = oz_newVar(p);
    rootVar = oz_newVar(this);
  } else {
    rootVar = result = oz_nil();
  }
#ifdef CS_PROFILE
  orig_start  = (int32 *) NULL;
  copy_start  = (int32 *) NULL;
  copy_size   = 0;
#endif
}

/*
 * Routines for checking suspensions
 *
 */

void oz_checkExtSuspension(Suspension susp, Board * varHome) {

  varHome=varHome->derefBoard();

  Board * bb = oz_currentBoard();

  Bool wasFound = NO;

  Assert(!varHome->isCommitted());

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


void Board::clearSuspList(Suspension killSusp) {
  Assert(!isRoot());

  SuspList * fsl = getSuspList();
  SuspList * tsl = (SuspList *) 0;

  while (fsl) {
    // Traverse suspension list and copy all valid suspensions
    Suspension susp = fsl->getSuspension();

    fsl = fsl->dispose();

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

      if (bb == this)
        break;

      bb = bb->getParent();
    }

    if (susp.isPropagator()) {
      Propagator * prop = susp.getPropagator();

      if (isAlive) {

        // if propagator suspends on external variable then keep its
        // thread in the list to avoid stability
        if (extParameters(prop->getPropagator()->getParameters(), this)) {
          tsl = new SuspList(susp, tsl);
        }

      }

    } else {
      Assert(susp.isThread());

      Thread * thr = susp.getThread();

      if (isAlive) {
        tsl = new SuspList(susp, tsl);
      } else {
        oz_disposeThread(thr);
      }

    }
  }

  setSuspList(tsl);

}


/*
 * Implementation of space operations
 */

inline
void Board::inject(TaggedRef proc) {
  // thread creation for {proc root}
  RefsArray args = allocateRefsArray(1, NO);
  args[0] = getRootVar();

  Thread *it = oz_newThreadInject(this);
  it->pushCall(proc, args, 1);
}

inline
TaggedRef Board::merge(Board *bb, Bool sibling) {
  // this is the board that gets merged
  //   --- it is merged with bb

  // First the things that must done even for merging with root board

  // Merge propagators
  bb->setLocalPropagatorQueue(getLocalPropagatorQueue()->
                              merge(bb->getLocalPropagatorQueue()));

  // Mark as merged
  parent = bb;
  setCommitted();

  // Must be before script installation
  bb->incSuspCount(getSuspCount());

  // Merge constraints
  if (!oz_installScript(this->getScriptRef()))
    return makeTaggedNULL();

  Assert(oz_isCurrentBoard(bb));

  if (bb->isRoot()) {
    scheduleNonMono();
  } else {
    // Update counters
    if (sibling)
      bb->incThreads(getThreads());

    // Merge distributors
    bb->setDistBag(bb->getDistBag()->merge(getDistBag()));

    // Merge nonmonotonic propagators
    bb->setNonMono(bb->getNonMono()->merge(getNonMono()));

  }

  return getRootVar();
}

inline
int Board::commit(int left, int right) {
  ozstat.incSolveAlt();

  Assert(bag && bag->getDist()->isAlive());

  return bag->getDist()->commit(this,left,right);

}



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

void Board::scheduleNonMono(void) {

  for (OrderedSuspList * p = getNonMono();
       p != NULL;
       p = p->getNext()) {
    Propagator * prop = p->getPropagator();

    oz_pushToLPQ(GETBOARD(prop),prop);
  }

  setNonMono(NULL);

}



//
// First class spaces (the builtin interface)
//


#define declareSpace                            \
  OZ_Term tagged_space = OZ_in(0);                      \
  DEREF(tagged_space, space_ptr, space_tag);            \
  if (isVariableTag(space_tag))                         \
    oz_suspendOn(makeTaggedRef(space_ptr));             \
  if (!oz_isSpace(tagged_space))                        \
    oz_typeError(0, "Space");                           \
  Space *space = (Space *) tagged2Const(tagged_space);

#define isFailedSpace \
  (space->isMarkedFailed() || \
   (space->isMarkedMerged() ? NO : space->getSpace()->isFailed()))

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
  sb->inject(proc);

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
  declareSpace;

  if (isFailedSpace)
    OZ_RETURN(AtomFailed);

  if (space->isMarkedMerged())
    OZ_RETURN(AtomMerged);

  TaggedRef answer = space->getSpace()->getResult();

  DEREF(answer, answer_ptr, answer_tag);

  if (isVariableTag(answer_tag))
    oz_suspendOn(makeTaggedRef(answer_ptr));

  OZ_RETURN((oz_isSTuple(answer) &&
             oz_eq(tagged2SRecord(answer)->getLabel(),
                   AtomSucceeded))
            ? AtomSucceeded : answer);

} OZ_BI_end


OZ_BI_define(BIaskVerboseSpace, 2,0) {
  declareSpace;
  oz_declareIN(1,out);

  if (isFailedSpace)
    return oz_unify(out, AtomFailed);

  if (space->isMarkedMerged())
    return oz_unify(out, AtomMerged);

  if (space->getSpace()->isBlocked()) {
    SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
    stuple->setArg(0, am.currentUVarPrototype());

    if (oz_unify(out, makeTaggedSRecord(stuple)) == FAILED)
      return FAILED;

    OZ_in(1) = stuple->getArg(0);
  }

  TaggedRef answer = space->getSpace()->getResult();

  DEREF(answer, answer_ptr, answer_tag);

  if (isVariableTag(answer_tag))
    oz_suspendOn(makeTaggedRef(answer_ptr));

  return oz_unify(OZ_in(1), answer);
} OZ_BI_end


OZ_BI_define(BIaskUnsafeSpace, 2,0) {
  declareSpace;
  oz_declareIN(1,out);

  if (isFailedSpace)
    return oz_unify(out, AtomFailed);

  if (space->isMarkedMerged())
    return oz_unify(out, AtomMerged);

  if (space->getSpace()->isBlocked()) {
    SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
    stuple->setArg(0, am.currentUVarPrototype());

    if (oz_unify(out, makeTaggedSRecord(stuple)) == FAILED) // mm2
      return FAILED;

    OZ_in(1) = stuple->getArg(0);
  }

  TaggedRef answer = space->getSpace()->getResult();

  return oz_unify(OZ_in(1), answer);
} OZ_BI_end


OZ_BI_define(BImergeSpace, 1,1) {
  declareSpace;

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  if (isFailedSpace)
    return FAILED;

  Board *CBB = oz_currentBoard();
  Board *SBB = space->getSpace()->derefBoard();
  Board *SBP = SBB->getParent();

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

  Bool isSibling = (!CBB->isRoot() &&
                    CBB->getParent() == SBP &&
                    CBB != SBB);

  if (!isSibling && CBB != SBP)
    return oz_raise(E_ERROR,E_KERNEL,"spaceSuper",1,tagged_space);

  Assert(!oz_isBelow(CBB,SBB));

  TaggedRef result = space->getSpace()->getResult();

  if (result == makeTaggedNULL())
    return FAILED;

  if (OZ_isVariable(result)) {

    if (isSibling) {
      switch (oz_installPath(SBP)) {
      case INST_FAILED: case INST_REJECTED: return FAILED;
      case INST_OK: break;
      }

      if (oz_unify(result, AtomMerged) == FAILED)
        return FAILED;

      switch (oz_installPath(CBB)) {
      case INST_FAILED: case INST_REJECTED: return FAILED;
      case INST_OK: break;
      }

    } else {
      if (oz_unify(result, AtomMerged) == FAILED)
        return FAILED;
    }

  }


  TaggedRef root = space->getSpace()->merge(CBB, isSibling);
  space->markMerged();

  if (root == makeTaggedNULL())
    return FAILED;

  OZ_RETURN(root);
} OZ_BI_end


OZ_BI_define(BIcloneSpace, 1,1) {
  declareSpace;

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  Board* CBB = oz_currentBoard();

  if (isFailedSpace)
    OZ_RETURN(makeTaggedConst(new Space(CBB, (Board *) 0)));

  TaggedRef result = space->getSpace()->getResult();

  DEREF(result, result_ptr, result_tag);

  if (isVariableTag(result_tag))
    oz_suspendOn(makeTaggedRef(result_ptr));

  ozstat.incSolveCloned();

  Board * copy = (Board *) space->getSpace()->clone();

#ifdef CS_PROFILE
  copy->orig_start = cs_orig_start;
  copy->copy_start = cs_copy_start;
  copy->copy_size  = cs_copy_size;
#endif

  OZ_RETURN(makeTaggedConst(new Space(CBB,copy)));
} OZ_BI_end


OZ_BI_define(BIcommitSpace, 2,0) {
  declareSpace;
  oz_declareIN(1,choice);

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  if (isFailedSpace)
    return PROCEED;

  TaggedRef result = space->getSpace()->getResult();

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

  if (!oz_isCurrentBoard(space->getSpace()->getParent()))
    return oz_raise(E_ERROR,E_KERNEL,"spaceParent",1,tagged_space);

  Board * sb = space->getSpace()->derefBoard();

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
  declareSpace;
  oz_declareIN(1,proc);

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  // Check whether space is failed!
  if (isFailedSpace)
    return PROCEED;

  if (!oz_isCurrentBoard(space->getSpace()->getParent()))
    return oz_raise(E_ERROR,E_KERNEL,"spaceParent", 1, tagged_space);

  DEREF(proc, proc_ptr, proc_tag);

  if (isVariableTag(proc_tag))
    oz_suspendOn(makeTaggedRef(proc_ptr));

  if (!oz_isProcedure(proc))
    oz_typeError(1, "Procedure");

  Board *sb = space->getSpace();

  // clear status
  sb->clearResult();

  // inject
  sb->inject(proc);

  return BI_PREEMPT;
} OZ_BI_end


inline
void Board::addToDistBag(Distributor * d) {
  setDistBag(getDistBag()->add(d));
}



OZ_BI_define(BIregisterSpace, 1, 1) {
  oz_declareIntIN(0, i);

  Board * bb = oz_currentBoard();

  if (bb->isRoot()) {
    OZ_out(0) = oz_newVar(bb);
  } else {
    BaseDistributor * bd = new BaseDistributor(bb,i);

    bb->addToDistBag(bd);

    OZ_out(0) = bd->getVar();
  }

  return BI_PREEMPT;
} OZ_BI_end



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

OZ_BI_define(BIgetCloneDiff, 1,1) {
  declareSpace;

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  OZ_RETURN(space->getSpace()->getCloneDiff());
} OZ_BI_end

#endif


// eof
//-----------------------------------------------------------------------------
