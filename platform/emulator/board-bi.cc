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

#include "board.hh"
#include "thr_int.hh"
#include "prop_int.hh"
#include "builtins.hh"
#include "value.hh"
#include "var_base.hh"
#include "var_future.hh"
#include "os.hh"

/*
 * Misc stuff
 *
 */

inline
void telleq(Board * bb, const TaggedRef a, const TaggedRef b) {
  RefsArray args = allocateRefsArray(2, NO);
  args[0] = a;
  args[1] = b;

  Thread * t = oz_newThreadInject(bb);
  t->pushCall(BI_Unify,args,2);
}

inline
void bindfut(Board * bb, const TaggedRef a, const TaggedRef b) {
  RefsArray args = allocateRefsArray(2, NO);
  args[0] = a;
  args[1] = b;

  Thread * t = oz_newThreadInject(bb);
  t->pushCall(BI_bindFuture,args,2);
}


class BaseDistributor : public Distributor {
protected:
  int offset, num;
  TaggedRef var;

public:

  BaseDistributor::BaseDistributor(Board * bb, const int n) {
    offset = 0;
    num    = n;
    var    = oz_newVar(bb);
  }

  TaggedRef getVar(void) {
    return var;
  }

  virtual int getAlternatives(void) {
    return num;
  }

  virtual int BaseDistributor::commit(Board * bb, int l, int r) {

    if (num==1 && l==1 && r==1) {
      int ret = oz_unify(var,makeTaggedSmallInt(1));
      Assert(ret==PROCEED);
      return 0;
    }

    if (l > num+1) {
      num = 0;
    } else {
      offset += l-1;
      num     = min(num,min(r,num+1)-l+1);

      if (num == 1) {
        num = 0;

        telleq(bb,var,makeTaggedSmallInt(offset + 1));
      }
    }
    return num;
  }

  virtual void dispose(void) {
    freeListDispose(this, sizeof(BaseDistributor));
  }

  virtual Distributor * BaseDistributor::gCollect(void) {
    BaseDistributor * t =
      (BaseDistributor *) oz_hrealloc(this,sizeof(BaseDistributor));

    oz_gCollectTerm(var, t->var);

    return (Distributor *) t;
  }

  virtual Distributor * BaseDistributor::sClone(void) {
    BaseDistributor * t =
      (BaseDistributor *) oz_hrealloc(this,sizeof(BaseDistributor));

    OZ_sCloneBlock(&var, &(t->var), 1);

    return (Distributor *) t;
  }

};




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

  TaggedRef answer = space->getSpace()->getStatus();

  DEREF(answer, answer_ptr, answer_tag);

  if (isVariableTag(answer_tag))
    oz_suspendOn(makeTaggedRef(answer_ptr));

  OZ_RETURN((oz_isSTuple(answer) &&
             oz_eq(tagged2SRecord(answer)->getLabel(),
                   AtomSucceeded))
            ? AtomSucceeded : answer);

} OZ_BI_end


OZ_BI_define(BIaskVerboseSpace, 1, 1) {
  declareSpace;

  if (isFailedSpace)
    OZ_RETURN(AtomFailed);

  if (space->isMarkedMerged())
    OZ_RETURN(AtomMerged);

  Board * s = space->getSpace();

  if (s->isBlocked() && !s->isStable()) {
    SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
    stuple->setArg(0, s->getStatus());

    OZ_RETURN(makeTaggedSRecord(stuple));
  }

  OZ_RETURN(s->getStatus());
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

  if (CBB->getDistributor() && SBB->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor",0);

  Bool isSibling = (!CBB->isRoot() &&
                    CBB->getParent() == SBP &&
                    CBB != SBB);

  if (!isSibling && CBB != SBP)
    return oz_raise(E_ERROR,E_KERNEL,"spaceSuper",1,tagged_space);

  Assert(!oz_isBelow(CBB,SBB));

  if (OZ_isVariable(SBB->getStatus())) {

    if (isSibling) {

      // Inject a thread to SBP to make the tell
      bindfut(SBP,SBB->getStatus(),AtomMerged);

    } else {
      SBB->bindStatus(AtomMerged);
    }

  }


  OZ_result(SBB->getRootVar());

  OZ_Return ret = SBB->merge(CBB, isSibling);
  space->markMerged();

  return ret;
} OZ_BI_end


#ifdef CS_PROFILE
extern int32 * cs_copy_start;
extern int32 * cs_orig_start;
extern int     cs_copy_size;
#endif

OZ_BI_define(BIcloneSpace, 1,1) {
  declareSpace;

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  Board* CBB = oz_currentBoard();

  if (isFailedSpace)
    OZ_RETURN(makeTaggedConst(new Space(CBB, (Board *) 0)));

  TaggedRef status = space->getSpace()->getStatus();

  DEREF(status, status_ptr, status_tag);

  if (isVariableTag(status_tag))
    oz_suspendOn(makeTaggedRef(status_ptr));

  ozstat.incSolveCloned();

  Board * copy = (Board *) space->getSpace()->clone();

  copy->setParent(CBB);

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

  TaggedRef status = space->getSpace()->getStatus();

  DEREF(status, status_ptr, status_tag);
  if (isVariableTag(status_tag))
    oz_suspendOn(makeTaggedRef(status_ptr));

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

  if (!sb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceNoChoice",1,tagged_space);

  int n = sb->commit(tagged2SmallInt(left),tagged2SmallInt(right));

  if (n>1) {
    sb->patchAltStatus(n);

    return PROCEED;
  }

  sb->clearStatus();

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
  sb->clearStatus();

  // inject
  sb->inject(proc);

  return BI_PREEMPT;
} OZ_BI_end



OZ_BI_define(BIchooseSpace, 1, 1) {
  oz_declareIntIN(0, i);

  Board * bb = oz_currentBoard();

  if (bb->isRoot()) {
    OZ_out(0) = oz_newVar(bb);
  } else if (bb->getDistributor()) {
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  } else {
    BaseDistributor * bd = new BaseDistributor(bb,i);

    bb->setDistributor(bd);

    OZ_out(0) = bd->getVar();
  }

  RefsArray args = allocateRefsArray(1);
  args[0] = OZ_out(0);

  am.prepareCall(BI_wait, args);

  return BI_REPLACEBICALL;
} OZ_BI_end


OZ_BI_define(BIwaitStableSpace, 0, 0) {
  Board * bb = oz_currentBoard();

  RefsArray args = allocateRefsArray(1);
  args[0] = OZ_out(0);

  if (bb->isRoot()) {
    args[0] = oz_newVar(bb);
  } else if (bb->getDistributor()) {
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  } else {
    BaseDistributor * bd = new BaseDistributor(bb,1);

    bb->setDistributor(bd);

    args[0] = bd->getVar();
  }

  am.prepareCall(BI_wait, args);

  return BI_REPLACEBICALL;
} OZ_BI_end


OZ_BI_define(BIdiscardSpace, 1, 0) {
  declareSpace;

  if (space->isMarkedMerged())
    return PROCEED;

  if (isFailedSpace)
    return PROCEED;

  if (!oz_isCurrentBoard(space->getSpace()->getParent()))
    return oz_raise(E_ERROR,E_KERNEL,"spaceParent", 1, tagged_space);

  Board *sb = space->getSpace();

  // clear status
  sb->clearStatus();

  // inject
  sb->inject(BI_fail, 0);

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
        lt->setHead(makeTaggedSmallInt(d));
        lt->setTail(l);
        l = makeTaggedLTuple(lt);
      } else {
        n++;
      }

    }

    free(copy_start);

    TaggedRef ret = OZ_pair2(makeTaggedSmallInt(copy_size),l);

    copy_start = (int32 *) 0;
    copy_size  = 0;
    orig_start = (int32 *) 0;

    return ret;

  } else {
    return OZ_pair2(makeTaggedSmallInt(0),l);
  }

}

OZ_BI_define(BIgetCloneDiff, 1,1) {
  declareSpace;

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  OZ_RETURN(space->getSpace()->getCloneDiff());
} OZ_BI_end

#endif


/*
 * The builtin table
 */

extern void (*OZ_sCloneBlockDynamic)(OZ_Term *,OZ_Term *,const int);
extern Suspendable * (*suspendableSCloneSuspendableDynamic)(Suspendable *);
extern Suspendable * suspendableSCloneSuspendable(Suspendable *);
extern OZ_Return (*OZ_checkSituatedness)(Board *,TaggedRef *);
extern OZ_Return OZ_checkSituatednessDynamic(Board *,TaggedRef *);

void space_init(void) {
  OZ_sCloneBlockDynamic =
    &OZ_sCloneBlock;
  suspendableSCloneSuspendableDynamic =
    &suspendableSCloneSuspendable;
  OZ_checkSituatedness =
    &OZ_checkSituatednessDynamic;
}

#ifndef MODULES_LINK_STATIC

#include "modSpace-if.cc"

#endif
