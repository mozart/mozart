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
#include "var_readonly.hh"
#include "os.hh"

/*
 * Misc stuff
 *
 */

inline
void telleq(Board * bb, const TaggedRef a, const TaggedRef b) {
  oz_newThreadInject(bb)->pushCall(BI_Unify,RefsArray::make(a,b));
}

inline
void bindreadonly(Board * bb, const TaggedRef a, const TaggedRef b) {
  oz_newThreadInject(bb)->pushCall(BI_bindReadOnly,RefsArray::make(a,b));
}


class BaseDistributor : public Distributor {
protected:
  int offset, num;
  TaggedRef var;

public:

  BaseDistributor::BaseDistributor(Board * bb, const int n) {
    offset = 0; 
    num    = n;
    var    = oz_newVariable(bb);
  }
  
  void dispose(void) {
    oz_freeListDispose(this, sizeof(BaseDistributor));
  }
  
  TaggedRef getVar(void) {
    return var;
  }

  virtual int getAlternatives(void) {
    return num;
  }

  virtual int BaseDistributor::commit(Board * bb, int n) {
    if (n > offset+num) {
      return -num;
    }

    if (num==1 && n==1) {
      (void) oz_unify(var,makeTaggedSmallInt(1));
    } else {
      telleq(bb,var,makeTaggedSmallInt(offset + n));
    }
    
    dispose();
    return 0;
  }
  
  virtual int BaseDistributor::commit(Board * bb, int l, int r) {
    if (r > offset+num) {
      return -num;
    }

    offset += l-1;
    num     = r-l+1;
    
    Assert(num>1);
    return num;
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


#define declareSpace					\
  OZ_Term tagged_space = OZ_in(0);			\
  DEREF(tagged_space, space_ptr);		        \
  Assert(!oz_isRef(tagged_space));			\
  if (oz_isVarOrRef(tagged_space))			\
    oz_suspendOn(makeTaggedRef(space_ptr));		\
  if (!oz_isSpace(tagged_space))			\
    oz_typeError(0, "Space");				\
  Space *space = (Space *) tagged2Const(tagged_space);

#define isFailedSpace \
  (space->isMarkedFailed() || \
   (space->isMarkedMerged() ? NO : space->getSpace()->isFailed()))

OZ_BI_define(BInewSpace, 1,1) {
  OZ_Term proc = OZ_in(0);

  DEREF(proc, proc_ptr);
  Assert(!oz_isRef(proc));
  if (oz_isVarOrRef(proc)) 
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

  DEREF(tagged_space, space_ptr);

  Assert(!oz_isRef(tagged_space));
  if (oz_isVarOrRef(tagged_space))
    oz_suspendOn(makeTaggedRef(space_ptr));

  OZ_RETURN(oz_bool(oz_isSpace(tagged_space)));
} OZ_BI_end


OZ_BI_define(BIaskSpace, 1,1) {
  declareSpace;

  if (isFailedSpace) 
    OZ_RETURN(AtomFailed);
  
  if (space->isMarkedMerged()) 
    OZ_RETURN(AtomMerged);
  
  if (!space->getSpace()->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);
    
  TaggedRef answer = space->getSpace()->getStatus();
  
  DEREF(answer, answer_ptr);

  Assert(!oz_isRef(answer));
  if (oz_isVarOrRef(answer))
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

  if (!s->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);
    
  if (s->isBlocked() && !s->isStable()) {
    SRecord *stuple = SRecord::newSRecord(AtomSuspended, 1);
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

  Board * sb = space->getSpace()->derefBoard();

  if (!sb->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);

  Board * sc = oz_currentBoard();
  Board * sp = sb->getParent();

  if (sc->getDistributor() && sb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor",0);

  Bool isUpward = (sc == sp);
  
  if (OZ_isVariable(sb->getStatus())) {
    if (isUpward) {
      sb->bindStatus(AtomMerged);
    } else {
      // Inject a thread to SBP to make the tell
      bindreadonly(sp,sb->getStatus(),AtomMerged);
    }
  }

  OZ_result(sb->getRootVar());

  OZ_Return ret = sb->merge(sc,isUpward);

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

  if (!space->getSpace()->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);

  TaggedRef status = space->getSpace()->getStatus();
  
  DEREF(status, status_ptr);

  Assert(!oz_isRef(status));
  if (oz_isVarOrRef(status)) 
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


OZ_BI_define(BIcommit1Space, 2, 0) {
  declareSpace;
  oz_declareSmallIntIN(1,n);

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  if (isFailedSpace)
    return PROCEED;
  
  Board * sb = space->getSpace();

  if (!sb->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);

  TaggedRef status = space->getSpace()->getStatus();
  
  DEREF(status, status_ptr);
  Assert(!oz_isRef(status));
  if (oz_isVarOrRef(status)) 
    oz_suspendOn(makeTaggedRef(status_ptr));

  if (!sb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceNoChoice",1,tagged_space);

  return sb->commit(tagged_space,n); 
} OZ_BI_end


OZ_BI_define(BIcommit2Space, 3,0) {
  declareSpace;
  oz_declareIntIN(1,l);
  oz_declareIntIN(2,r);

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  if (isFailedSpace)
    return PROCEED;
  
  Board * sb = space->getSpace();

  if (!sb->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);

  TaggedRef status = space->getSpace()->getStatus();
  
  DEREF(status, status_ptr);
  Assert(!oz_isRef(status));
  if (oz_isVarOrRef(status)) 
    oz_suspendOn(makeTaggedRef(status_ptr));

  if (!sb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceNoChoice",1,tagged_space);

  return sb->commit(tagged_space, l, r);
} OZ_BI_end


OZ_BI_define(BIcommitSpace, 2,0) {
  declareSpace;
  oz_declareIN(1,choice);

  if (space->isMarkedMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  if (isFailedSpace)
    return PROCEED;
  
  Board * sb = space->getSpace();

  if (!sb->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);

  TaggedRef status = space->getSpace()->getStatus();
  
  DEREF(status, status_ptr);
  Assert(!oz_isRef(status));
  if (oz_isVarOrRef(status)) 
    oz_suspendOn(makeTaggedRef(status_ptr));

  DEREF(choice, choice_ptr);
  Assert(!oz_isRef(choice));
  if (oz_isVarOrRef(choice))
    oz_suspendOn(makeTaggedRef(choice_ptr));

  TaggedRef left, right;

  if (oz_isSmallInt(choice)) {
    left  = choice;
    right = choice;
  } else if (oz_isSTuple(choice) &&
	     oz_eq(AtomPair,
		   tagged2SRecord(choice)->getLabel()) &&
	     tagged2SRecord(choice)->getWidth() == 2) {
    left  = tagged2SRecord(choice)->getArg(0);
    DEREF(left, left_ptr);

    Assert(!oz_isRef(left));
    if (oz_isVarOrRef(left))
      oz_suspendOn(makeTaggedRef(left_ptr));

    right = tagged2SRecord(choice)->getArg(1);
    
    DEREF(right, right_ptr);

    Assert(!oz_isRef(right));
    if (oz_isVarOrRef(right))
      oz_suspendOn(makeTaggedRef(right_ptr));
  } else {
    oz_typeError(1, "Integer or pair of integers");
  }

  if (!sb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceNoChoice",1,tagged_space);

  return sb->commit(tagged_space, 
		    tagged2SmallInt(left),
		    tagged2SmallInt(right));
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

  Board * sb = space->getSpace();

  if (!sb->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);

  DEREF(proc, proc_ptr);

  Assert(!oz_isRef(proc));
  if (oz_isVarOrRef(proc)) 
    oz_suspendOn(makeTaggedRef(proc_ptr));

  if (!oz_isProcedure(proc))
    oz_typeError(1, "Procedure");

  // clear status
  sb->clearStatus();

  // inject
  sb->inject(proc);
    
  return BI_PREEMPT;
} OZ_BI_end



OZ_BI_define(BIchooseSpace, 1, 1) {
  oz_declareSmallIntIN(0, i);

  Board * bb = oz_currentBoard();

  if (bb->isRoot()) {
    OZ_out(0) = oz_newVariable(bb);
  } else if (bb->getDistributor()) {
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  } else {
    BaseDistributor * bd = new BaseDistributor(bb,i);

    bb->setDistributor(bd);

    OZ_out(0) = bd->getVar();
  }

  RefsArray * args = RefsArray::allocate(1,NO);
  args->setArg(0,OZ_out(0));

  am.prepareCall(BI_wait, args);

  return BI_REPLACEBICALL;
} OZ_BI_end


OZ_BI_define(BIwaitStableSpace, 0, 0) {
  Board * bb = oz_currentBoard();

  RefsArray * args = RefsArray::allocate(1,NO);
  args->setArg(0,OZ_out(0));

  if (bb->isRoot()) {
    args->setArg(0,oz_newVariable(bb));
  } else if (bb->getDistributor()) {
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  } else {
    BaseDistributor * bd = new BaseDistributor(bb,1);

    bb->setDistributor(bd);

    args->setArg(0,bd->getVar());
  }

  am.prepareCall(BI_wait, args);

  return BI_REPLACEBICALL;
} OZ_BI_end


OZ_BI_define(BIkillSpace, 1, 0) {
  declareSpace;

  if (space->isMarkedMerged())
    return PROCEED;

  if (isFailedSpace)
    return PROCEED;

  Board *sb = space->getSpace();

  if (!sb->isAdmissible())
    return oz_raise(E_ERROR,E_KERNEL,"spaceAdmissible",1,tagged_space);

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

extern void (*OZ_sCloneBlockDynamic)(OZ_Term *,OZ_Term *,int);
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



