/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte (schulte@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *
 *  Copyright:
 *    Michael Mehl, 1997,1998
 *    Kostja Popow, 1997
 *    Ralf Scheidhauer, 1997
 *    Christian Schulte, 1997
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

#include "wsock.hh"

#include "builtins.hh"

#include "os.hh"
#include "codearea.hh"
#include "thr_int.hh"
#include "space.hh"
#include "debug.hh"
#include "iso-ctype.hh"
#include "var_base.hh"
#include "var_ext.hh"
#include "var_of.hh"
#include "var_future.hh"
#include "solve.hh"
#include "mozart_cpi.hh"
#include "dictionary.hh"
#include "dpInterface.hh"
#include "bytedata.hh"

#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

/********************************************************************
 * Type tests
 ******************************************************************** */

OZ_BI_define(BIwait,1,0)
{
  oz_declareNonvarIN(0, val);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIwaitOr,2,0)
{
  oz_declareDerefIN(0,a);

  if (!oz_isVariable(a)) return PROCEED;

  oz_declareDerefIN(1,b);

  if (!oz_isVariable(b)) return PROCEED;

  Assert(oz_isVariable(a) && oz_isVariable(b));

  am.addSuspendVarList(aPtr);
  am.addSuspendVarList(bPtr);
  return SUSPEND;
} OZ_BI_end

OZ_BI_define(BIwaitOrF,1,1)
{
  oz_declareNonvarIN(0,a);

  if (!oz_isRecord(a)) oz_typeError(0,"Record");
  if (oz_isLiteral(a)) oz_typeError(0,"ProperRecord");

  TaggedRef arity=OZ_arityList(a);
  while (!OZ_isNil(arity)) {
    TaggedRef v=OZ_subtree(a,OZ_head(arity));
    DEREF(v,vPtr,_);
    if (!oz_isVariable(v)) {
      am.emptySuspendVarList();
      OZ_RETURN(OZ_head(arity));
    }
    am.addSuspendVarList(vPtr);
    arity=OZ_tail(arity);
  }

  return SUSPEND;
} OZ_BI_end


OZ_BI_define(BIisLiteral, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isLiteral(term)));
} OZ_BI_end

OZ_BI_define(BIisAtom, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isAtom(term)));
} OZ_BI_end

OZ_BI_define(BIwaitStatus,2,1)
{
  oz_declareNonvarIN(0,status);
  oz_declareNonvarIN(1,what);

  if (oz_isSRecord(status)) {
    status = tagged2SRecord(status)->getLabel();
  }
  OZ_RETURN(oz_bool(oz_isLiteral(status)
                    && oz_isLiteral(what)
                    && oz_eq(status,what)));
} OZ_BI_end


#define CheckStatus(var,varstatus,statusAtom)                                           \
  OzVariable *cv   = tagged2CVar(var);                                                  \
  VarStatus status = oz_check_var_status(cv);                                           \
  switch (status) {                                                                     \
  case varstatus:                                                                       \
    OZ_RETURN(oz_true());                                                               \
  case EVAR_STATUS_UNKNOWN:                                                             \
    {                                                                                   \
      OZ_Term status = _var_status(cv);                                                 \
      OZ_Term out = oz_newVariable();                                                   \
      OZ_out(0) = out;                                                                  \
      OZ_Term wait = makeTaggedConst(new Builtin("waitStatus", 2,1,BIwaitStatus,OK));   \
      am.prepareCall(wait,status,statusAtom,out);                                       \
      return BI_REPLACEBICALL;                                                          \
    }                                                                                   \
  default:                                                                              \
    OZ_RETURN(oz_false());                                                              \
  }

OZ_BI_define(BIisFree, 1,1)
{
  oz_declareDerefIN(0,var);
  if (isUVar(var))
    OZ_RETURN(oz_true());

  if (!isCVar(var))
    OZ_RETURN(oz_false());

  CheckStatus(var,EVAR_STATUS_FREE,AtomFree);
} OZ_BI_end

OZ_BI_define(BIisKinded, 1,1)
{
  oz_declareDerefIN(0,var);
  if (!isCVar(var))
    OZ_RETURN(oz_false());

  CheckStatus(var,EVAR_STATUS_KINDED,AtomKinded);
} OZ_BI_end

OZ_BI_define(BIisFuture, 1,1)
{
  oz_declareDerefIN(0,var);
  if (!isCVar(var))
    OZ_RETURN(oz_false());

  CheckStatus(var,EVAR_STATUS_FUTURE,AtomFuture);
} OZ_BI_end

OZ_BI_define(BIisDet,1,1)
{
  oz_declareDerefIN(0,var);
  if (isUVar(var))
    OZ_RETURN(oz_false());

  if (!isCVar(var))
    OZ_RETURN(oz_true());

  CheckStatus(var,EVAR_STATUS_DET,AtomDet);
} OZ_BI_end

OZ_BI_define(BIisName, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isName(term)));
} OZ_BI_end

OZ_BI_define(BIisTuple, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isTuple(term)));
} OZ_BI_end

OZ_BI_define(BIisRecord, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isRecord(term)));
} OZ_BI_end

OZ_BI_define(BIisProcedure, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isProcedure(term)));
} OZ_BI_end

OZ_BI_define(BIisChunk, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isChunk(term)));
} OZ_BI_end

OZ_BI_define(BIisExtension, 1, 1)
{
  oz_declareNonvarIN(0,t);
  OZ_RETURN(oz_bool(oz_isExtensionPlus(t)));
} OZ_BI_end

OZ_BI_define(BIprocedureArity, 1,1)
{
  oz_declareNonvarIN(0, pterm);

  if (oz_isProcedure(pterm)) {
    OZ_RETURN(oz_int(oz_procedureArity(pterm)));
  } else {
    oz_typeError(0,"Procedure");
  }
} OZ_BI_end

OZ_BI_define(BIisCell, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isCell(term)));
} OZ_BI_end

// ---------------------------------------------------------------------
// Spaces
// ---------------------------------------------------------------------

#define declareSpace()                                  \
  OZ_Term tagged_space = OZ_in(0);                      \
  DEREF(tagged_space, space_ptr, space_tag);            \
  if (isVariableTag(space_tag))                         \
    oz_suspendOn(makeTaggedRef(space_ptr));             \
  if (!oz_isSpace(tagged_space))                        \
    oz_typeError(0, "Space");                           \
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

// ---------------------------------------------------------------------
// Tuple
// ---------------------------------------------------------------------

OZ_BI_define(BItuple, 2, 1)
{
  oz_declareNonvarIN(0, label);
  oz_declareIntIN(1, i);

  if (!oz_isLiteral(label)) oz_typeError(0, "Literal");
  if (i < 0) oz_typeError(1, "(non-negative small) Int");
  if (i == 0) OZ_RETURN(label);

  SRecord *sr = SRecord::newSRecord(label, i);

  for (int j = 0; j < i; j++) {
    sr->setArg(j, am.currentUVarPrototype());
  }

  OZ_RETURN(sr->normalize());
} OZ_BI_end

// ---------------------------------------------------------------------
// Tuple & Record
// ---------------------------------------------------------------------

OZ_BI_define(BIlabel, 1, 1)
{
 oz_declareNonKindedIN(0,rec);
 if (oz_isCons(rec)) OZ_RETURN(AtomCons);
 if (oz_isLiteral(rec)) OZ_RETURN(rec);
 if (oz_isSRecord(rec)) OZ_RETURN(tagged2SRecord(rec)->getLabel());
 if (isGenOFSVar(rec)) {
   TaggedRef thelabel=tagged2GenOFSVar(rec)->getLabel();
   DEREF(thelabel,lPtr,_2);
   if (oz_isVariable(thelabel)) oz_suspendOnPtr(lPtr);
   OZ_RETURN(thelabel);
 }
 oz_typeError(0,"Record");
} OZ_BI_end

// mm2: who needs this?
OZ_BI_define(BIhasLabel, 1, 1)
{
  oz_declareDerefIN(0,rec);
  // Wait for term to be a record with determined label:
  // Get the term's label, if it exists
  if (oz_isVariable(rec)) {
    if (isGenOFSVar(rec)) {
      TaggedRef thelabel=tagged2GenOFSVar(rec)->getLabel();
      DEREF(thelabel,lPtr,_2);
      OZ_RETURN(oz_bool(!oz_isVariable(thelabel)));
    }
    OZ_RETURN(oz_false());
  }
  if (oz_isRecord(rec)) OZ_RETURN(oz_true());
  oz_typeError(0,"Record");
} OZ_BI_end

/*
 * NOTE: similar functions are dot, genericSet, uparrow
 */
OZ_Return genericDot(TaggedRef term, TaggedRef fea, TaggedRef *out, Bool dot)
{
  DEREF(fea, _1,feaTag);
LBLagain:
  DEREF(term, _2, termTag);

  if (isVariableTag(feaTag)) {
    switch (termTag) {
    case LTUPLE:
    case SRECORD:
      // FUT
    case UVAR:
      return SUSPEND;
    case CVAR:
      switch (tagged2CVar(term)->getType()) {
      case OZ_VAR_FD:
      case OZ_VAR_BOOL:
          goto typeError0;
      default:
          return SUSPEND;
      }
      // if (tagged2CVar(term)->getType() == OZ_VAR_OF) return SUSPEND;
      // goto typeError0;
    case LITERAL:
      goto typeError0;
    default:
      if (oz_isChunk(term)) return SUSPEND;
      goto typeError0;
    }
  }

  if (!oz_isFeature(fea)) goto typeError1;

  switch (termTag) {
  case LTUPLE:
    {
      if (!oz_isSmallInt(fea)) {
        if (dot) goto raise; else return FAILED;
      }
      int i2 = smallIntValue(fea);

      if (i2 == 1) {
        if (out) *out = tagged2LTuple(term)->getHead();
        return PROCEED;
      }
      if (i2 == 2) {
        if (out) *out = tagged2LTuple(term)->getTail();
        return PROCEED;
      }

      if (dot) goto raise; else return FAILED;
    }

  case SRECORD:
    {
      TaggedRef t = tagged2SRecord(term)->getFeature(fea);
      if (t == makeTaggedNULL()) {
        if (dot) goto raise; else return FAILED;
      }
      if (out) *out = t;
      return PROCEED;
    }

  case UVAR:
    // FUT
    if (!oz_isFeature(fea)) {
      oz_typeError(1,"Feature");
    }
    return SUSPEND;

  case CVAR:
    switch (tagged2CVar(term)->getType()) {
    case OZ_VAR_OF:
      {
        int ret = tagged2GenOFSVar(term)->hasFeature(fea,out);
        if (ret == FAILED) goto typeError0;
        return ret;
      }
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
    case OZ_VAR_FS:
      goto typeError0;
    default:
      return SUSPEND;
    }

  case LITERAL:
    if (dot) goto raise; else return FAILED;

  case EXT:
    {
      TaggedRef t = oz_tagged2Extension(term)->getFeatureV(fea);
      if (t == makeTaggedNULL()) {
        if (dot) goto raise; else return FAILED;
      }
      if (out) *out = t;
      return PROCEED;
    }
  default:
    if (oz_isChunk(term)) {
      TaggedRef t;
      switch (tagged2Const(term)->getType()) {
      case Co_Chunk:
        t = tagged2SChunk(term)->getFeature(fea);
        break;
      case Co_Object:
        t = tagged2Object(term)->getFeature(fea);
        break;
      case Co_Class:
        t = tagged2ObjectClass(term)->classGetFeature(fea);
        if (!t) {
          TaggedRef cfs;
          cfs = oz_deref(tagged2ObjectClass(term)->classGetFeature(NameOoUnFreeFeat));
          if (oz_isSRecord(cfs)) {
            t = tagged2SRecord(cfs)->getFeature(fea);
            TaggedRef dt = oz_deref(t);

            if (oz_isName(dt) && oz_eq(dt,NameOoFreeFlag))
              t = makeTaggedNULL();
          }
        }
        break;
      case Co_Array:
      case Co_Dictionary:
      default:
        // no public known features
        t = makeTaggedNULL();
        break;
      }
      if (t == makeTaggedNULL()) {
        if (dot) goto raise; else return FAILED;
      }
      if (out) *out = t;
      return PROCEED;
    }

    goto typeError0;
  }
typeError0:
  oz_typeError(0,"Record or Chunk");
typeError1:
  oz_typeError(1,"Feature");
raise:
  return oz_raise(E_ERROR,E_KERNEL,".",2,term,fea);
}

// extern
OZ_Return dotInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
  return genericDot(term,fea,&out,TRUE);
}
OZ_DECLAREBI_USEINLINEFUN2(BIdot,dotInline)

inline
OZ_Return hasFeatureInline(TaggedRef term, TaggedRef fea)
{
  return genericDot(term,fea,0,FALSE);
}

OZ_BI_define(BIhasFeature,2,1)
{
  OZ_Return r = hasFeatureInline(OZ_in(0),OZ_in(1));
  switch (r) {
  case PROCEED: OZ_RETURN(oz_true());
  case FAILED : OZ_RETURN(oz_false());
  case SUSPEND: oz_suspendOn2(OZ_in(0),OZ_in(1));
  default     : return r;
  }
} OZ_BI_end

/*
 * fun {matchDefault Term Attr Defau}
 *    if X in Term.Attr = X then X else Defau fi
 * end
 */
inline
OZ_Return subtreeInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
  return genericDot(term,fea,&out,FALSE);
}

OZ_BI_define(BImatchDefault,3,1)
{
  OZ_Term aux=0;
  oz_declareIN(0,rec);
  oz_declareIN(1,fea);
  OZ_Return ret=subtreeInline(rec,fea,aux);
  if (ret==SUSPEND) {
    oz_suspendOn2(rec,fea);
  } else if (ret==PROCEED) {
    OZ_RETURN(aux);
  } else {
    oz_declareIN(2,out);
    OZ_RETURN(out);
  }
} OZ_BI_end

OZ_Return widthInline(TaggedRef term, TaggedRef &out)
{
  DEREF(term,_,tag);

  switch (tag) {
  case LTUPLE:
    out = makeTaggedSmallInt(2);
    return PROCEED;
  case SRECORD:
  record:
    out = makeTaggedSmallInt(tagged2SRecord(term)->getWidth());
    return PROCEED;
  case LITERAL:
    out = makeTaggedSmallInt(0);
    return PROCEED;
  case UVAR:
    // FUT
    return SUSPEND;
  case CVAR:
    switch (tagged2CVar(term)->getType()) {
    case OZ_VAR_OF:
        return SUSPEND;
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
        break;
    default:
        return SUSPEND;
    }
    break;
  default:
    break;
  }

  oz_typeError(0,"Record");
}

OZ_DECLAREBI_USEINLINEFUN1(BIwidth,widthInline)


// ---------------------------------------------------------------------
// Unit
// ---------------------------------------------------------------------

OZ_BI_define(BIisUnit, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isLiteral(term) && oz_eq(term,NameUnit)));
} OZ_BI_end

// ---------------------------------------------------------------------
// Bool things
// ---------------------------------------------------------------------

OZ_BI_define(BIisBool, 1, 1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_bool(oz_isBool(term)));
} OZ_BI_end

OZ_BI_define(BInot, 1, 1)
{
  oz_declareBoolIN(0,b);

  OZ_RETURN(oz_bool(!b));
} OZ_BI_end

OZ_BI_define(BIand, 2, 1)
{
  oz_declareBoolIN(0,A);
  oz_declareBoolIN(1,B);

  if (A) {
    OZ_RETURN(oz_bool(B));
  } else {
    OZ_RETURN(oz_false());
  }
} OZ_BI_end

OZ_BI_define(BIor, 2, 1)
{
  oz_declareBoolIN(0,A);
  oz_declareBoolIN(1,B);

  if (A) {
    OZ_RETURN(oz_true());
  } else {
    OZ_RETURN(oz_bool(B));
  }
} OZ_BI_end



// ---------------------------------------------------------------------
// Atom
// ---------------------------------------------------------------------

OZ_BI_define(BIatomToString, 1, 1)
{
  oz_declareNonvarIN(0,t);

  if (!oz_isAtom(t)) oz_typeError(0,"atom");

  OZ_RETURN(OZ_string(tagged2Literal(t)->getPrintName()));
} OZ_BI_end

OZ_BI_define(BIstringToAtom,1,1)
{
  oz_declareProperStringIN(0,str);

  OZ_RETURN(oz_atom(str));
} OZ_BI_end

// ---------------------------------------------------------------------
// Virtual Strings
// ---------------------------------------------------------------------

inline
TaggedRef vs_suspend(SRecord *vs, int i, TaggedRef arg_rest) {
  if (i == vs->getWidth()-1) {
    return arg_rest;
  } else {
    SRecord *stuple = SRecord::newSRecord(AtomPair, vs->getWidth() - i);
    stuple->setArg(0, arg_rest);
    i++;
    for (int j=1 ; i < vs->getWidth() ; (j++, i++))
      stuple->setArg(j, vs->getArg(i));
    return makeTaggedSRecord(stuple);
  }
}

static OZ_Return vs_check(OZ_Term vs, OZ_Term *rest) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isVariableTag(vs_tag)) {
    *rest = makeTaggedRef(vs_ptr);
    oz_suspendOn(*rest);
  } else if (isSmallIntTag(vs_tag)) {
    return PROCEED;
  } else if (oz_isBigInt(vs)) {
    return PROCEED;
  } else if (isFloatTag(vs_tag)) {
    return PROCEED;
  } else if (isLiteralTag(vs_tag) && tagged2Literal(vs)->isAtom()) {
    return PROCEED;
  } else if (isLTupleTag(vs_tag)) {
    TaggedRef cdr  = vs;
    TaggedRef prev = vs;

    while (1) {
      DEREF(cdr, cdr_ptr, cdr_tag);

      if (oz_isNil(cdr))
        return PROCEED;

      if (isVariableTag(cdr_tag)) {
        *rest = prev;
        oz_suspendOn(makeTaggedRef(cdr_ptr));
      }

      if (!isLTupleTag(cdr_tag))
        return FAILED;

      TaggedRef car = tagged2LTuple(cdr)->getHead();
      DEREF(car, car_ptr, car_tag);

      if (isVariableTag(car_tag)) {
        *rest = cdr;
        oz_suspendOn(makeTaggedRef(car_ptr));
      } else if (!isSmallIntTag(car_tag) ||
                 (smallIntValue(car) < 0) ||
                 (smallIntValue(car) > 255)) {
        return FAILED;
      } else {
        prev = cdr;
        cdr  = tagged2LTuple(cdr)->getTail();
      }
    };

    return FAILED;

  } else if (oz_isSTuple(vs) &&
             oz_eq(tagged2SRecord(vs)->getLabel(),AtomPair)) {
    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++) {
      TaggedRef arg_rest;
      OZ_Return status = vs_check(tagged2SRecord(vs)->getArg(i), &arg_rest);

      if (status == SUSPEND) {
        *rest = vs_suspend(tagged2SRecord(vs), i, arg_rest);

        return SUSPEND;
      } else if (status==FAILED) {
        return FAILED;
      }
    }
    return PROCEED;
  } else if (oz_isByteString(vs)) {
    return PROCEED;
  } else {
    return FAILED;
  }
}


// mm2: cycle check needed
static OZ_Return vs_length(OZ_Term vs, OZ_Term *rest, int *len) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isVariableTag(vs_tag)) {
    *rest = makeTaggedRef(vs_ptr);
    oz_suspendOn(*rest);
  } else if (isSmallIntTag(vs_tag) || oz_isBigInt(vs)) {
    *len = *len + strlen(toC(vs));
    return PROCEED;
  } else if (isFloatTag(vs_tag)) {
    *len = *len + strlen(toC(vs));
    return PROCEED;
  } else if (isLiteralTag(vs_tag) && tagged2Literal(vs)->isAtom()) {
    if (oz_eq(vs,AtomPair) ||
        oz_eq(vs,AtomNil))
      return PROCEED;
    *len = *len + ((Atom*)tagged2Literal(vs))->getSize();
    return PROCEED;
  } else if (isLTupleTag(vs_tag)) {
    TaggedRef cdr  = vs;
    TaggedRef prev = vs;

    while (1) {
      DEREF(cdr, cdr_ptr, cdr_tag);

      if (oz_isNil(cdr))
        return PROCEED;

      if (isVariableTag(cdr_tag)) {
        *rest = prev;
        Assert((*len)>0);
        *len = *len - 1;
        oz_suspendOn(makeTaggedRef(cdr_ptr));
      }

      if (!isLTupleTag(cdr_tag))
        return FAILED;

      TaggedRef car = tagged2LTuple(cdr)->getHead();
      DEREF(car, car_ptr, car_tag);

      if (isVariableTag(car_tag)) {
        *rest = cdr;
        oz_suspendOn(makeTaggedRef(car_ptr));
      } else if (!isSmallIntTag(car_tag) ||
                 (smallIntValue(car) < 0) ||
                 (smallIntValue(car) > 255)) {
        return FAILED;
      } else {
        prev = cdr;
        cdr  = tagged2LTuple(cdr)->getTail();
        *len = *len + 1;
      }
    };

    return FAILED;

  } else if (oz_isSTuple(vs) &&
             oz_eq(tagged2SRecord(vs)->getLabel(),AtomPair)) {
    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++) {
      TaggedRef arg_rest;
      OZ_Return status =
        vs_length(tagged2SRecord(vs)->getArg(i), &arg_rest, len);

      if (status == SUSPEND) {
        *rest = vs_suspend(tagged2SRecord(vs), i, arg_rest);
        return SUSPEND;
      } else if (status==FAILED) {
        return FAILED;
      }
    }
    return PROCEED;
  } else if (oz_isByteString(vs)) {
    *len = *len + tagged2ByteString(vs)->getWidth();
    return PROCEED;
  } else {
    return FAILED;
  }
}


OZ_BI_define(BIvsLength,2,1) {
  TaggedRef rest = makeTaggedNULL();
  int len = smallIntValue(oz_deref(OZ_in(1)));
  OZ_Return status = vs_length(OZ_in(0), &rest, &len);
  if (status == SUSPEND) {
    OZ_in(0) = rest;
    OZ_in(1) = makeTaggedSmallInt(len);
    return SUSPEND;
  } else if (status == FAILED) {
    oz_typeError(0, "Virtual String");
  } else {
    OZ_RETURN(makeTaggedSmallInt(len));
  }
} OZ_BI_end

OZ_BI_define(BIvsIs,1,1) {
  TaggedRef rest = makeTaggedNULL();
  OZ_Return status = vs_check(OZ_in(0), &rest);
  if (status == SUSPEND) {
    OZ_in(0) = rest;
    return SUSPEND;
  }
  OZ_RETURN(oz_bool(status == PROCEED));
} OZ_BI_end

OZ_BI_define(BIvsToBs,3,1)
{
  // OZ_in(0) is initially the argument vs
  // OZ_in(1) is initially 0
  // OZ_in(2) is always the initial argument vs
  //
  // OZ_in(0) is side effected to hold what remains so far undetermined
  // packed up as a new virtual string
  TaggedRef rest = makeTaggedNULL();
  int len = smallIntValue(oz_deref(OZ_in(1)));
  OZ_Return status = vs_length(OZ_in(0), &rest, &len);
  if (status == SUSPEND) {
    OZ_in(0) = rest;
    OZ_in(1) = makeTaggedSmallInt(len);
    return SUSPEND;
  } else if (status == FAILED) {
    oz_typeError(0, "Virtual String");
  } else {
    // the initial argument vs is in OZ_in(2)
    // it is known to be fully determined and its
    // size is len
    ByteString* bs = new ByteString(len);
    ostrstream *out = new ostrstream;
    extern void virtualString2buffer(ostream &,OZ_Term);
    virtualString2buffer(*out,OZ_in(2));
    bs->copy(out->str(),len);
    delete out;
    OZ_RETURN(oz_makeTaggedExtension(bs));
  }
} OZ_BI_end

// ---------------------------------------------------------------------
// Chunk
// ---------------------------------------------------------------------

OZ_BI_define(BInewChunk,1,1)
{
  oz_declareNonvarIN(0,val);

  if (!oz_isRecord(val)) oz_typeError(0,"Record");

  OZ_RETURN(oz_newChunk(oz_currentBoard(),val));
} OZ_BI_end

/* ---------------------------------------------------------------------
 * Threads
 * --------------------------------------------------------------------- */

OZ_BI_define(BIthreadThis,0,1)
{
  OZ_RETURN(oz_thread(oz_currentThread()));
} OZ_BI_end

/*
 * change priority of a thread
 *  if my priority is lowered, then preempt me
 *  if priority of other thread become higher than mine, then preempt me
 */
OZ_BI_define(BIthreadSetPriority,2,0)
{
  oz_declareThread(0,th);
  oz_declareNonvarIN(1,atom_prio);

  int prio;

  if (!oz_isAtom(atom_prio))
    goto type_goof;

  if (oz_eq(atom_prio, AtomLow)) {
    prio = LOW_PRIORITY;
  } else if (oz_eq(atom_prio, AtomMedium)) {
    prio = MID_PRIORITY;
  } else if (oz_eq(atom_prio, AtomHigh)) {
    prio = HI_PRIORITY;
  } else {
  type_goof:
    oz_typeError(1,"Atom [low medium high]");
  }

  int oldPrio = th->getPriority();
  th->setPriority(prio);

  if (oz_currentThread() == th) {
    if (prio <= oldPrio) {
      am.setSFlag(ThreadSwitch);
      return BI_PREEMPT;
    }
  } else {
    if (th->isRunnable()) {
      am.threadsPool.rescheduleThread(th);
    }
    if (prio > oz_currentThread()->getPriority()) {
      return BI_PREEMPT;
    }
  }

  return PROCEED;
} OZ_BI_end

OZ_Term threadGetPriority(Thread *th) {
  switch (th->getPriority()) {
  case LOW_PRIORITY: return AtomLow;
  case MID_PRIORITY: return AtomMedium;
  case HI_PRIORITY:  return AtomHigh;
  default: Assert(0); return AtomHigh;
  }
}

OZ_BI_define(BIthreadGetPriority,1,1)
{
  oz_declareThread(0,th);

  OZ_RETURN(threadGetPriority(th));
} OZ_BI_end

OZ_BI_define(BIthreadIs,1,1)
{
  oz_declareNonvarIN(0,th);

  OZ_RETURN(oz_bool(oz_isThread(th)));
} OZ_BI_end

/*
 * raise exception on thread
 */
OZ_BI_proto(BIraise);
OZ_BI_proto(BIraiseDebug);

void threadRaise(Thread *th,OZ_Term E,int debug) {
  Assert(oz_currentThread() != th);

  RefsArray args=allocateRefsArray(1, NO);
  args[0]=E;

  th->pushCFun(debug?BIraiseDebug:BIraise, args, 1);

  th->setStop(NO);

  if (th->isSuspended()) {
    oz_wakeupThread(th);
    return;
  }

  if (!am.threadsPool.isScheduledSlow(th))
    am.threadsPool.scheduleThread(th);
}

OZ_BI_define(BIthreadRaise,2,0)
{
  oz_declareThread(0,th);
  oz_declareNonvarIN(1,E);

  if (oz_currentThread() == th) {
    return OZ_raise(E);
  }

  threadRaise(th,E);
  return PROCEED;
} OZ_BI_end

/*
 * suspend a thread
 *   is done lazy: when the thread becomes running the stop flag is tested
 */
OZ_BI_define(BIthreadSuspend,1,0)
{
  oz_declareThread(0,th);

  th->setStop(OK);
  if (th == oz_currentThread()) {
    return BI_PREEMPT;
  }
  return PROCEED;
} OZ_BI_end

void threadResume(Thread *th) {
  th->setStop(NO);

  /* mm2: I don't understand this, but let's try to give some explanation.
   *  1. resuming the current thread should be a NOOP.
   *  2. only runnable threads need to be rescheduled.
   */
  if (th!=oz_currentThread() &&
      th->isRunnable() &&
      !am.threadsPool.isScheduledSlow(th)) {
    am.threadsPool.scheduleThread(th);
  }
}

OZ_BI_define(BIthreadResume,1,0)
{
  oz_declareThread(0,th);

  threadResume(th);

  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIthreadIsSuspended,1,1)
{
  oz_declareThread(0,th);

  OZ_RETURN(oz_bool(th->getStop()));
} OZ_BI_end

OZ_Term threadState(Thread *th) {
  if (th->isDeadThread()) {
    return oz_atom("terminated");
  }
  if (th->isRunnable()) {
    return oz_atom("runnable");
  }
  return oz_atom("blocked");
}

OZ_BI_define(BIthreadState,1,1)
{
  oz_declareThreadIN(0,th);

  OZ_RETURN(threadState(th));
} OZ_BI_end

OZ_BI_define(BIthreadPreempt,1,0)
{
  oz_declareThread(0,th);

  if (th == oz_currentThread()) {
    return BI_PREEMPT;
  }
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIthreadCreate,1,0)
{
  oz_declareNonvarIN(0,p);

  if (!oz_isAbstraction(p)) {
    oz_typeError(0,"Abstraction");
  }

  Abstraction *a = tagged2Abstraction(p);
  if (a->getArity() != 0) {
    oz_typeError(0,"Nullary Abstraction");
  }

  int prio   = min(oz_currentThread()->getPriority(),DEFAULT_PRIORITY);
  Thread *tt = oz_newThread(prio);

  tt->getTaskStackRef()->pushCont(a->getPC(),NULL,a);
  tt->setAbstr(a->getPred());

  return PROCEED;
} OZ_BI_end

// ------------------ explore a thread's taskstack ---------------------------

OZ_BI_define(BIexceptionTaskStackError,0,1)
{
  Thread * thread = oz_currentThread();

  TaskStack *taskstack = thread->getTaskStackRef();
  OZ_RETURN(taskstack->getTaskStack(thread,NO,ozconf.errorThreadDepth));
} OZ_BI_end

OZ_BI_define(BIexceptionLocation,0,1)
{
  OZ_RETURN(oz_getLocation(GETBOARD(oz_currentThread())));

} OZ_BI_end

// ---------------------------------------------------------------------
// NAMES
// ---------------------------------------------------------------------

OZ_BI_define(BInewName,0,1)
{
  OZ_RETURN(oz_newName());
} OZ_BI_end

OZ_BI_define(BInewUniqueName,1,1)
{
  oz_declareAtomIN(0,name);
  OZ_RETURN(oz_uniqueName(name));
} OZ_BI_end

// ---------------------------------------------------------------------
// term type
// ---------------------------------------------------------------------

OZ_BI_define(BItermType,1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(OZ_termType(term));
} OZ_BI_end


OZ_Term oz_status(OZ_Term term)
{
  DEREF(term, _1, tag);

  switch (tag) {
  case UVAR:
    // FUT
    return AtomFree;
  case CVAR:
    {
      OzVariable *cv = tagged2CVar(term);
      VarStatus status = oz_check_var_status(cv);

      switch (status) {
      case EVAR_STATUS_FREE:
        return AtomFree;
      case EVAR_STATUS_FUTURE:
        return AtomFuture;
      case EVAR_STATUS_DET:
      case EVAR_STATUS_UNKNOWN:
        return _var_status(cv);
      case EVAR_STATUS_KINDED:
        break;
      default:
        Assert(0);
      }

      SRecord *t = SRecord::newSRecord(AtomKinded, 1);
      switch (cv->getType()) {
      case OZ_VAR_FD:
      case OZ_VAR_BOOL:
        t->setArg(0, AtomInt); break;
      case OZ_VAR_FS:
        t->setArg(0, AtomFSet); break;
      case OZ_VAR_OF:
        t->setArg(0, AtomRecord); break;
      default:
        t->setArg(0, AtomOther); break;
      }
      return makeTaggedSRecord(t);
    }
  default:
    {
      SRecord *t = SRecord::newSRecord(AtomDet, 1);
      t->setArg(0, OZ_termType(term));
      return makeTaggedSRecord(t);
    }
  }
  Assert(0);
}

OZ_BI_define(BIstatus,1,1)
{
  oz_declareIN(0,term);
  OZ_RETURN(oz_status(term));
} OZ_BI_end

// ---------------------------------------------------------------------
// Builtins ==, \=, ==B and \=B
// ---------------------------------------------------------------------

inline
OZ_Return oz_eqeq(TaggedRef Ain,TaggedRef Bin)
{
  // simulate a shallow guard
  am.trail.pushMark();
  am.setShallowHeapTop(heapTop);
  OZ_Return ret = oz_unify(Ain,Bin,(ByteCode*)1);
  am.setShallowHeapTop(NULL);

  if (ret == PROCEED) {
    if (am.trail.isEmptyChunk()) {
      am.trail.popMark();
      return PROCEED;
    }

    oz_reduceTrailOnEqEq();
    return SUSPEND;
  }

  oz_reduceTrailOnFail();
  return ret;
}

inline
OZ_Return eqeqWrapper(TaggedRef Ain, TaggedRef Bin)
{
  TaggedRef A = Ain, B = Bin;
  DEREF(A,aPtr,tagA); DEREF(B,bPtr,tagB);

  /* Really fast test for equality */
  if (tagA != tagB) {
    if (oz_isVariable(A) || oz_isVariable(B)) goto dontknow;
    return FAILED;
  }

  if (isSmallIntTag(tagA)) return smallIntEq(A,B) ? PROCEED : FAILED;
  if (isFloatTag(tagA))    return floatEq(A,B)    ? PROCEED : FAILED;

  if (isLiteralTag(tagA))  return oz_eq(A,B)  ? PROCEED : FAILED;

  if (A == B && !oz_isVariable(A)) return PROCEED;

  if (oz_isExtension(A))
    return oz_tagged2Extension(A)->eqV(B);

  if (isConstTag(tagA)) {
    switch (tagged2Const(A)->getType()) {
    case Co_BigInt:
      return bigIntEq(A,B) ? PROCEED : FAILED;
    default:
      return FAILED;
    }
  }

 dontknow:
  return oz_eqeq(Ain,Bin);
}


OZ_Return neqInline(TaggedRef A, TaggedRef B, TaggedRef &out);
OZ_Return eqeqInline(TaggedRef A, TaggedRef B, TaggedRef &out);


OZ_BI_define(BIneqB,2,1)
{
  return neqInline(OZ_in(0),OZ_in(1),OZ_out(0));
    //  OZ_Term help;
    //  OZ_Return ret=neqInline(OZ_getCArg(0),OZ_getCArg(1),help);
    //  return ret==PROCEED ? oz_unify(help,OZ_getCArg(2)) : ret;
} OZ_BI_end

OZ_BI_define(BIeqB,2,1)
{
  return eqeqInline(OZ_in(0),OZ_in(1),OZ_out(0));
  //  OZ_Term help;
  //  OZ_Return ret=eqeqInline(OZ_getCArg(0),OZ_getCArg(1),help);
  //  return ret==PROCEED ? oz_unify(help,OZ_getCArg(2)): ret;
} OZ_BI_end


OZ_Return eqeqInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch(eqeqWrapper(A,B)) {
  case PROCEED:
    out = oz_true();
    return PROCEED;
  case FAILED:
    out = oz_false();
    return PROCEED;
  case SUSPEND:
  default:
    return SUSPEND;
  }
}


OZ_Return neqInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch(eqeqWrapper(A,B)) {
  case PROCEED:
    out = oz_false();
    return PROCEED;
  case FAILED:
    out = oz_true();
    return PROCEED;
  case SUSPEND:
  default:
    return SUSPEND;
  }
}

// ---------------------------------------------------------------------
// String
// ---------------------------------------------------------------------

OZ_BI_define(BIisString,1,1)
{
  OZ_Term in=OZ_in(0);

  OZ_Term var;
  if (!OZ_isString(in,&var)) {
    if (var == 0) OZ_RETURN(oz_false());
    oz_suspendOn(var);
  }
  OZ_RETURN(oz_true());
} OZ_BI_end


// ---------------------------------------------------------------------
// Char
// ---------------------------------------------------------------------

#define OZ_FirstCharArg \
 TaggedRef tc = OZ_in(0);           \
 int i;                             \
 { DEREF(tc, tc_ptr, tc_tag);       \
 if (isVariableTag(tc_tag)) {            \
   am.addSuspendVarList(tc_ptr);    \
   return SUSPEND;                  \
 }                                  \
 if (!oz_isSmallInt(tc)) {             \
   oz_typeError(1,"Char");          \
 } else {                           \
   i = smallIntValue(tc);           \
   if ((i < 0) || (i > 255)) {      \
     oz_typeError(1,"Char");        \
   }                                \
 } }

#define OZ_TestChar(TEST)                                            \
  OZ_FirstCharArg;                                                   \
  OZ_RETURN(oz_bool(TEST ((unsigned char) i)));

OZ_BI_define(BIcharIs,1,1) {
 oz_declareNonvarIN(0,c);
 c = oz_deref(c);
 if (!oz_isSmallInt(c)) OZ_RETURN(oz_false());
 int i = smallIntValue(c);
 OZ_RETURN(oz_bool(i >=0 && i <= 255));
} OZ_BI_end

#define BI_TESTCHAR(Name,Arg) \
OZ_BI_define(Name,1,1) { OZ_TestChar(Arg); } OZ_BI_end

BI_TESTCHAR(BIcharIsAlNum,iso_isalnum)
BI_TESTCHAR(BIcharIsAlpha,iso_isalpha)
BI_TESTCHAR(BIcharIsCntrl,iso_iscntrl)
BI_TESTCHAR(BIcharIsDigit,iso_isdigit)
BI_TESTCHAR(BIcharIsGraph,iso_isgraph)
BI_TESTCHAR(BIcharIsLower,iso_islower)
BI_TESTCHAR(BIcharIsPrint,iso_isprint)
BI_TESTCHAR(BIcharIsPunct,iso_ispunct)
BI_TESTCHAR(BIcharIsSpace,iso_isspace)
BI_TESTCHAR(BIcharIsUpper,iso_isupper)
BI_TESTCHAR(BIcharIsXDigit,iso_isxdigit)


OZ_BI_define(BIcharToLower,1,1) {
  OZ_FirstCharArg;
  OZ_RETURN_INT(iso_tolower((unsigned char) i));
} OZ_BI_end

// mm2: should use iso_tolower directly
OZ_Return INLINE__BIcharToLower(TaggedRef arg1,TaggedRef& out)
{
  OZ_Term X[2];
  X[0]=arg1;
  int ret= BIcharToLower(X,OZ_ID_MAP);
  out = X[1];
  return ret;
}

OZ_BI_define(BIcharToUpper,1,1) {
  OZ_FirstCharArg;
  OZ_RETURN_INT(iso_toupper((unsigned char) i));
} OZ_BI_end

OZ_BI_define(BIcharToAtom,1,1) {
  OZ_FirstCharArg;
  if (i) {
     char s[2]; s[0]= (char) i; s[1]='\0';
     OZ_RETURN(oz_atom(s));
  }
  OZ_RETURN(AtomEmpty);
} OZ_BI_end

#define FirstCharIN                 \
 TaggedRef tc = OZ_in(0);           \
 int i;                             \
 { DEREF(tc, tc_ptr, tc_tag);       \
 if (isVariableTag(tc_tag)) {            \
   am.addSuspendVarList(tc_ptr);    \
   return SUSPEND;                  \
 }                                  \
 if (!oz_isSmallInt(tc)) {             \
   oz_typeError(1,"Char");          \
 } else {                           \
   i = smallIntValue(tc);           \
   if ((i < 0) || (i > 255)) {      \
     oz_typeError(1,"Char");        \
   }                                \
 } }

OZ_BI_define(BIcharType,1,1) {
  OZ_FirstCharArg;
  TaggedRef type;
  if (iso_isupper(i))      type = AtomUpper;
  else if (iso_islower(i)) type = AtomLower;
  else if (iso_isdigit(i)) type = AtomDigit;
  else if (iso_isspace(i)) type = AtomCharSpace;
  else if (iso_ispunct(i)) type = AtomPunct;
  else                     type = AtomOther;
  OZ_RETURN(type);
} OZ_BI_end


/********************************************************************
 * Records
 ******************************************************************** */


OZ_BI_define(BIadjoin,2,1)
{
  oz_declareNonvarIN(0,t0);
  oz_declareNonvarIN(1,t1);

  if (oz_isLiteral(t0)) {
    if (oz_isRecord(t1)) OZ_RETURN(t1);
    oz_typeError(1,"Record");
  }
  if (oz_isRecord(t0)) {
    SRecord *rec= makeRecord(t0);
    if (oz_isLiteral(t1)) {
      SRecord *newrec = SRecord::newSRecord(rec);
      newrec->setLabelForAdjoinOpt(t1);
      OZ_RETURN(newrec->normalize());
    }
    if (oz_isRecord(t1)) {
      OZ_RETURN(oz_adjoin(rec,makeRecord(t1)));
    }
    oz_typeError(1,"Record");
  }
  oz_typeError(0,"Record");
} OZ_BI_end

OZ_BI_define(BIadjoinAt,3,1)
{
  oz_declareNonvarIN(0,rec);
  oz_declareNonvarIN(1,fea);
  oz_declareIN(2,value);

  if (!oz_isFeature(fea)) oz_typeError(1,"Feature");

  if (oz_isLiteral(rec)) {
    SRecord *newrec
      = SRecord::newSRecord(rec,aritytable.find(oz_cons(fea,oz_nil())));
    newrec->setArg(0,value);
    OZ_RETURN(makeTaggedSRecord(newrec));
  }
  if (oz_isRecord(rec)) {
    OZ_RETURN(oz_adjoinAt(makeRecord(rec),fea,value));
  }
  oz_typeError(0,"Record");
} OZ_BI_end

static
TaggedRef getArityFromPairList(TaggedRef list)
{
  TaggedRef arity;
  TaggedRef *next=&arity;
  Bool updateFlag=NO;
  DerefIfVarReturnIt(list);
  TaggedRef old = list;
loop:
  if (oz_isLTuple(list)) {
    TaggedRef pair = oz_head(list);
    DerefIfVarReturnIt(pair);
    if (!oz_isPair2(pair)) return 0;

    TaggedRef fea = tagged2SRecord(pair)->getArg(0);
    DerefIfVarReturnIt(fea);

    if (!oz_isFeature(fea)) return 0;

    LTuple *lt=new LTuple();
    *next=makeTaggedLTuple(lt);
    lt->setHead(fea);
    next=lt->getRefTail();

    list = oz_tail(list);
    DerefIfVarReturnIt(list);
    if (list==old) return 0;
    if (updateFlag) {
      old=oz_deref(oz_tail(old));
    }
    updateFlag=1-updateFlag;
    goto loop;
  }

  if (oz_isNil(list)) {
    *next=oz_nil();
    return arity;
  }

  return 0;
}

/* common subroutine for builtins adjoinList and record:
   recordFlag=OK: 'adjoinList' allows records as first arg
              N0: 'makeRecord' only allows literals */
static
OZ_Return adjoinPropListInline(TaggedRef t0, TaggedRef list, TaggedRef &out,
                           Bool recordFlag)
{
  TaggedRef arity=getArityFromPairList(list);
  if (!arity) {
    oz_typeError(1,"finite list(Feature#Value)");
  }
  DEREF(t0,t0Ptr,tag0);
  if (oz_isRef(arity)) { // must suspend
    out=arity;
    switch (tag0) {
    case UVAR:
      // FUT
    case LITERAL:
      return SUSPEND;
    case SRECORD:
    case LTUPLE:
      if (recordFlag) {
        return SUSPEND;
      }
      goto typeError0;
    case CVAR:
      if (oz_isKinded(t0) && tagged2CVar(t0)->getType()!=OZ_VAR_OF)
        goto typeError0;
      if (recordFlag) {
        return SUSPEND;
      }
      goto typeError0;
    default:
      goto typeError0;
    }
  }

  if (oz_isNil(arity)) { // adjoin nothing
    switch (tag0) {
    case SRECORD:
    case LTUPLE:
      if (recordFlag) {
        out = t0;
        return PROCEED;
      }
      goto typeError0;
    case LITERAL:
      out = t0;
      return PROCEED;
    case UVAR:
      // FUT
      out=makeTaggedRef(t0Ptr);
      return SUSPEND;
    case CVAR:
      if (oz_isKinded(t0) && tagged2CVar(t0)->getType()!=OZ_VAR_OF)
        goto typeError0;
      out=makeTaggedRef(t0Ptr);
      return SUSPEND;
    default:
      goto typeError0;
    }
  }

  switch (tag0) {
  case LITERAL:
    {
      int len1 = oz_fastlength(arity);
      arity = sortlist(arity,len1);
      int len = oz_fastlength(arity); // NOTE: duplicates may be removed
      if (!recordFlag && len!=len1) {  // handles case f(a:_ a:_)
        return oz_raise(E_ERROR,E_KERNEL,"recordConstruction",2,
                        t0,list
                        );
      }
      SRecord *newrec = SRecord::newSRecord(t0,aritytable.find(arity));
      newrec->setFeatures(list);
      out = newrec->normalize();
      return PROCEED;
    }
  case SRECORD:
  case LTUPLE:
    if (recordFlag) {
      out = oz_adjoinList(makeRecord(t0),arity,list);
      return PROCEED;
    }
    goto typeError0;
  case UVAR:
    // FUT
    out=makeTaggedRef(t0Ptr);
    return SUSPEND;
  case CVAR:
    if (oz_isKinded(t0) && tagged2CVar(t0)->getType()!=OZ_VAR_OF)
        goto typeError0;
    out=makeTaggedRef(t0Ptr);
    return SUSPEND;
  default:
    goto typeError0;
  }

 typeError0:
  if (recordFlag) {
    oz_typeError(0,"Record");
  } else {
    oz_typeError(0,"Literal");
  }
}

// extern
OZ_Return adjoinPropList(TaggedRef t0, TaggedRef list, TaggedRef &out,
                     Bool recordFlag)
{
  return adjoinPropListInline(t0,list,out,recordFlag);
}


OZ_BI_define(BIadjoinList,2,1)
{
  //OZ_Term help;

  OZ_Return state = adjoinPropListInline(OZ_in(0),OZ_in(1),OZ_out(0),OK);
  switch (state) {
  case SUSPEND:
    oz_suspendOn(OZ_out(0));
    //case PROCEED:
    //OZ_RETURN(help);
  default:
    return state;
  }
} OZ_BI_end


OZ_BI_define(BImakeRecord,2,1)
{
  //OZ_Term help;

  OZ_Return state = adjoinPropListInline(OZ_in(0),OZ_in(1),OZ_out(0),NO);
  switch (state) {
  case SUSPEND:
    oz_suspendOn(OZ_out(0));
    return PROCEED;
    //case PROCEED:
    //OZ_RETURN(help);
  default:
    return state;
  }
} OZ_BI_end


OZ_Return BIarityInline(TaggedRef term, TaggedRef &out)
{
  DEREF(term,termPtr,tag);

  if (oz_isVariable(term)) {
    if (oz_isKinded(term) && !isGenOFSVar(term)) {
      oz_typeError(0,"Record");
    }
    return SUSPEND;
  }
  out = getArityList(term);
  if (out) return PROCEED;
  oz_typeError(0,"Record");
}

OZ_DECLAREBI_USEINLINEFUN1(BIarity,BIarityInline)


// Builtins for Record Pattern-Matching

OZ_BI_define(BItestRecord,3,1)
{
  // type-check the inputs:
  oz_declareNonKindedIN(0,val);
  oz_declareNonvarIN(1,patLabel);
  oz_declareNonvarIN(2,patArityList);

  if (!oz_isLiteral(patLabel)) oz_typeError(1,"Literal");

  OZ_Term ret=oz_checkList(patArityList,OZ_CHECK_FEATURE);
  if (oz_isRef(ret))   oz_suspendOn(ret);
  if (oz_isFalse(ret)) oz_typeError(2,"non-empty finite list(Feature)");
  int len = smallIntValue(ret);
  if (len==0) oz_typeError(2,"non-empty finite list(Feature)");

  // compute the pattern's arity:
  TaggedRef sortedPatArityList = sortlist(duplist(patArityList,len),len);
  if (oz_fastlength(sortedPatArityList) != len) {
    // duplicate features are not allowed
    return oz_raise(E_ERROR,E_KERNEL,"recordPattern",2,patLabel,patArityList);
  }
  Arity *patArity = aritytable.find(sortedPatArityList);

  // is the input a proper record (or can it still become one)?
  if (oz_isVariable(val) && oz_isKinded(val) && isGenOFSVar(val)) {
    OzOFVariable *ofsvar = tagged2GenOFSVar(val);
    if (patArity->isTuple()) {
      if (ofsvar->disentailed(tagged2Literal(patLabel),patArity->getWidth())) {
        OZ_RETURN(oz_false());
      }
    } else {
      if (ofsvar->disentailed(tagged2Literal(patLabel),patArity)) {
        OZ_RETURN(oz_false());
      }
    }
    oz_suspendOnPtr(valPtr);
  }
  if (oz_isLiteral(val) || !oz_isRecord(val)) {
    // literals never match since the arity is always a non-empty list
    OZ_RETURN(oz_false());
  }
  // from here on we deal with a determined proper record

  // get the value's label and SRecordArity:
  TaggedRef valLabel;
  SRecordArity valSRA;
  if (oz_isSRecord(val)) {
    SRecord *rec = tagged2SRecord(val);
    valLabel = rec->getLabel();
    valSRA = rec->getSRecordArity();
  } else {
    Assert(oz_isCons(val));
    valLabel = AtomCons;
    valSRA = mkTupleWidth(2);
  }

  // do the records match?
  SRecordArity patSRA = (patArity->isTuple())?
    mkTupleWidth(patArity->getWidth()): mkRecordArity(patArity);
  if (oz_eq(valLabel,patLabel) && sameSRecordArity(valSRA,patSRA)) {
    OZ_RETURN(oz_true());
  } else {
    OZ_RETURN(oz_false());
  }
} OZ_BI_end

OZ_BI_define(BItestRecordLabel,2,1)
{
  oz_declareNonKindedIN(0,val);
  oz_declareNonvarIN(1,patLabel);
  if (!oz_isLiteral(patLabel)) {
    oz_typeError(1,"Literal");
  }

  // get value's label:
  TaggedRef valLabel;
  if (isGenOFSVar(val)) {
    valLabel = oz_safeDeref(tagged2GenOFSVar(val)->getLabel());
    if (oz_isRef(valLabel)) {
      oz_suspendOnPtr(valPtr);
    }
  } else if (oz_isLiteral(val)) {
    valLabel = val;
  } else if (!oz_isRecord(val)) {
    OZ_RETURN(oz_false());
  } else if (oz_isSRecord(val)) {
    valLabel = tagged2SRecord(val)->getLabel();
  } else {
    Assert(oz_isCons(val));
    valLabel = AtomCons;
  }

  // do the labels match?
  OZ_RETURN(oz_bool(oz_eq(patLabel,valLabel)));
} OZ_BI_end

OZ_BI_define(BItestRecordFeature,2,2)
{
  oz_declareIN(0,val);
  oz_declareIN(1,patFeature);
  TaggedRef out;
  OZ_Return ret = genericDot(val,patFeature,&out,FALSE);
  switch (ret) {
  case SUSPEND:
    oz_suspendOn2(val,patFeature);
  case FAILED:
    OZ_out(1) = oz_unit();
    OZ_RETURN(oz_false());
  case PROCEED:
    OZ_out(1) = out;
    OZ_RETURN(oz_true());
  default:
    return ret;
  }
} OZ_BI_end

/* -----------------------------------------------------------------------
   Numbers
   ----------------------------------------------------------------------- */

static OZ_Return bombArith(char *type)
{
  oz_typeError(-1,type);
}

#define suspendTest(A,B,test,type)                      \
  if (oz_isVariable(A)) {                                       \
    if (oz_isVariable(B) || test(B)) { return SUSPEND; }        \
    return bombArith(type);                             \
  }                                                     \
  if (oz_isVariable(B)) {                                       \
    if (oz_isNumber(A)) { return SUSPEND; }             \
  }                                                     \
  return bombArith(type);


static OZ_Return suspendOnNumbers(TaggedRef A, TaggedRef B)
{
  suspendTest(A,B,oz_isNumber,"int or float\nuniformly for all arguments");
}

inline Bool isNumOrAtom(TaggedRef t)
{
  return oz_isNumber(t) || oz_isAtom(t);
}

static OZ_Return suspendOnNumbersAndAtoms(TaggedRef A, TaggedRef B)
{
  suspendTest(A,B,isNumOrAtom,"int, float or atom\nuniformly for all arguments");
}

static OZ_Return suspendOnFloats(TaggedRef A, TaggedRef B)
{
  suspendTest(A,B,oz_isFloat,"Float");
}


static OZ_Return suspendOnInts(TaggedRef A, TaggedRef B)
{
  suspendTest(A,B,oz_isInt,"Int");
}

#undef suspendTest





/* -----------------------------------
   Z = X op Y
   ----------------------------------- */

// Float x Float -> Float
OZ_Return BIfdivInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);
  if (isFloatTag(tagA) && isFloatTag(tagB)) {
    out = oz_float(floatValue(A) / floatValue(B));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}


// Int x Int -> Int
#define BIGOP(op)                                       \
  if (oz_isBigInt(A)) {                                 \
    if (oz_isBigInt(B)) {                                       \
      out = tagged2BigInt(A)->op(tagged2BigInt(B));     \
      return PROCEED;                                   \
    }                                                   \
    if (tagB == SMALLINT) {                             \
      BigInt *b = new BigInt(smallIntValue(B));         \
      out = tagged2BigInt(A)->op(b);                    \
      b->dispose();                                     \
      return PROCEED;                                   \
    }                                                   \
  }                                                     \
  if (oz_isBigInt(B)) {                                 \
    if (tagA == SMALLINT) {                             \
      BigInt *a = new BigInt(smallIntValue(A));         \
      out = a->op(tagged2BigInt(B));                    \
      a->dispose();                                     \
      return PROCEED;                                   \
    }                                                   \
  }

// Integer x Integer -> Integer
OZ_Return BIdivInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagB == SMALLINT && smallIntValue(B) == 0) {
    if (tagA == SMALLINT || oz_isBigInt(A)) {
      return oz_raise(E_ERROR,E_KERNEL,"div0",1,A);
    } else {
      return bombArith("Int");
    }
  }

  if ( (tagA == SMALLINT) && (tagB == SMALLINT)) {
    out = newSmallInt(smallIntValue(A) / smallIntValue(B));
    return PROCEED;
  }
  BIGOP(div);
  return suspendOnInts(A,B);
}

// Integer x Integer -> Integer
OZ_Return BImodInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if ((tagB == SMALLINT && smallIntValue(B) == 0)) {
    if (tagA == SMALLINT || oz_isBigInt(A)) {
      return oz_raise(E_ERROR,E_KERNEL,"mod0",1,A);
    } else {
      return bombArith("Int");
    }
  }

  if ( (tagA == SMALLINT) && (tagB == SMALLINT)) {
    out = newSmallInt(smallIntValue(A) % smallIntValue(B));
    return PROCEED;
  }

  BIGOP(mod);
  return suspendOnInts(A,B);
}


/* Division is slow on RISC (at least SPARC)
 *  --> first make a simpler test for no overflow
 */

inline
int multOverflow(int a, int b)
{
  int absa = ozabs(a);
  int absb = ozabs(b);
  const int bits = (sizeof(TaggedRef)*8-tagSize)/2 - 1;

  if (!((absa|absb)>>bits)) /* if none of the 13 MSB in neither a nor b are set */
    return NO;
  return ((b!=0) && (absa >= OzMaxInt / absb));
}

OZ_Return BImultInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == SMALLINT && tagB == SMALLINT) {
    int valA = smallIntValue(A);
    int valB = smallIntValue(B);
    if ( multOverflow(valA,valB) ) {
      BigInt *a = new BigInt(valA);
      BigInt *b = new BigInt(valB);
      out = a->mul(b);
      a->dispose();
      b->dispose();
      return PROCEED;
    } else {
      out = newSmallInt(valA*valB);
      return PROCEED;
    }
  }

  if (isFloatTag(tagA) && isFloatTag(tagB)) {
    out = oz_float(floatValue(A) * floatValue(B));
    return PROCEED;
  }

  BIGOP(mul);
  return suspendOnNumbers(A,B);
}

OZ_Return BIminusInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,_11);
  DEREF(B,_2,_22);

  if ( oz_isSmallInt(A) && oz_isSmallInt(B) ) {
    out = oz_int(smallIntValue(A) - smallIntValue(B));
    return PROCEED;
  }

  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(floatValue(A) - floatValue(B));
    return PROCEED;
  }


  TypeOfTerm tagA = tagTypeOf(A);
  TypeOfTerm tagB = tagTypeOf(B);
  BIGOP(sub);
  return suspendOnNumbers(A,B);
}

OZ_Return BIplusInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,_11);
  DEREF(B,_2,_22);

  if ( oz_isSmallInt(A) && oz_isSmallInt(B) ) {
    out = oz_int(smallIntValue(A) + smallIntValue(B));
    return PROCEED;
  }

  if (oz_isFloat(A) && oz_isFloat(B)) {
    out = oz_float(floatValue(A) + floatValue(B));
    return PROCEED;
  }

  TypeOfTerm tagA = tagTypeOf(A);
  TypeOfTerm tagB = tagTypeOf(B);
  BIGOP(add);
  return suspendOnNumbers(A,B);
}


OZ_Return BIminusOrPlus(Bool callPlus,TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return callPlus ?  BIplusInline(A,B,out) : BIminusInline(A,B,out);
}


#undef BIGOP

/* -----------------------------------
   Z = op X
   ----------------------------------- */

// unary minus: Number -> Number
OZ_Return BIuminusInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  if (isSmallIntTag(tagA)) {
    out = newSmallInt(-smallIntValue(A));
    return PROCEED;
  }

  if (isFloatTag(tagA)) {
    out = oz_float(-floatValue(A));
    return PROCEED;
  }

  if (oz_isBigInt(A)) {
    out = tagged2BigInt(A)->neg();
    return PROCEED;
  }

  if (isVariableTag(tagA)){
    return SUSPEND;
  }

  oz_typeError(0,"Number");

}

OZ_Return BIabsInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  if (isSmallIntTag(tagA)) {
    int i = smallIntValue(A);
    out = (i >= 0) ? A : newSmallInt(-i);
    return PROCEED;
  }

  if (isFloatTag(tagA)) {
    double f = floatValue(A);
    out = (f >= 0.0) ? A : oz_float(fabs(f));
    return PROCEED;
  }

  if (oz_isBigInt(A)) {
    BigInt *b = tagged2BigInt(A);
    out = (b->cmp(0l) >= 0) ? A : b->neg();
    return PROCEED;
  }

  if (isVariableTag(tagA)){
    return SUSPEND;
  }

  oz_typeError(0,"Number");
}

// add1(X) --> X+1
OZ_Return BIadd1Inline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  if (isSmallIntTag(tagA)) {
    /* INTDEP */
    int res = (int)A + (1<<tagSize);
    if ((int)A < res) {
      out = res;
      return PROCEED;
    }
  }

  return BIplusInline(A,makeTaggedSmallInt(1),out);
}

// sub1(X) --> X-1
OZ_Return BIsub1Inline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  if (isSmallIntTag(tagA)) {
    /* INTDEP */
    int res = (int)A - (1<<tagSize);
    if ((int)A > res) {
      out = res;
      return PROCEED;
    }
  }

  return BIminusInline(A,makeTaggedSmallInt(1),out);
}


/* -----------------------------------
   X test Y
   ----------------------------------- */

OZ_Return bigintLess(BigInt *A, BigInt *B)
{
  return (A->cmp(B) < 0 ? PROCEED : FAILED);
}


OZ_Return bigintLe(BigInt *A, BigInt *B)
{
  return (A->cmp(B) <= 0 ? PROCEED : FAILED);
}


OZ_Return bigtest(TaggedRef A, TaggedRef B,
                  OZ_Return (*test)(BigInt*, BigInt*))
{
  if (oz_isBigInt(A)) {
    if (oz_isBigInt(B)) {
      return test(tagged2BigInt(A),tagged2BigInt(B));
    }
    if (oz_isSmallInt(B)) {
      BigInt *b = new BigInt(smallIntValue(B));
      OZ_Return res = test(tagged2BigInt(A),b);
      b->dispose();
      return res;
    }
  }
  if (oz_isBigInt(B)) {
    if (oz_isSmallInt(A)) {
      BigInt *a = new BigInt(smallIntValue(A));
      OZ_Return res = test(a,tagged2BigInt(B));
      a->dispose();
      return res;
    }
  }
  if (oz_isVariable(A) || oz_isVariable(B))
    return SUSPEND;

  oz_typeError(-1,"int, float or atom\nuniformly for all arguments");
}




OZ_Return BIminInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    switch(tagA) {
    case SMALLINT: out = (smallIntLess(A,B) ? A : B);             return PROCEED;
    case OZFLOAT:  out = (floatValue(A) < floatValue(B)) ? A : B; return PROCEED;
    case LITERAL:
      if (oz_isAtom(A) && oz_isAtom(B)) {
        out = (strcmp(tagged2Literal(A)->getPrintName(),
                      tagged2Literal(B)->getPrintName()) < 0)
          ? A : B;
        return PROCEED;
      }
      oz_typeError(-1,"Comparable");

    default: break;
    }
  }

  OZ_Return ret = bigtest(A,B,bigintLess);
  switch (ret) {
  case PROCEED: out = A; return PROCEED;
  case FAILED:  out = B; return PROCEED;
  case RAISE:   return RAISE;
  default:      break;
  }

  return suspendOnNumbersAndAtoms(A,B);
}


/* code adapted from min */
OZ_Return BImaxInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    switch(tagA) {
    case SMALLINT:
      out = (smallIntLess(A,B) ? B : A);
      return PROCEED;
    case OZFLOAT:
      out = (floatValue(A) < floatValue(B)) ? B : A;
      return PROCEED;
    case LITERAL:
      if (oz_isAtom(A) && oz_isAtom(B)) {
        out = (strcmp(tagged2Literal(A)->getPrintName(),
                      tagged2Literal(B)->getPrintName()) < 0)
          ? B : A;
        return PROCEED;
      }
      oz_typeError(-1,"Comparable");

    default: break;
    }
  }

  OZ_Return ret = bigtest(A,B,bigintLess);
  switch (ret) {
  case PROCEED: out = B; return PROCEED;
  case FAILED:  out = A; return PROCEED;
  case RAISE:   return RAISE;
  default:      break;
  }

  return suspendOnNumbersAndAtoms(A,B);
}


OZ_Return BIlessInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    if (tagA == SMALLINT) {
      return (smallIntLess(A,B) ? PROCEED : FAILED);
    }

    if (isFloatTag(tagA)) {
      return (floatValue(A) < floatValue(B)) ? PROCEED : FAILED;
    }

    if (tagA == LITERAL) {
      if (oz_isAtom(A) && oz_isAtom(B)) {
        return (strcmp(tagged2Literal(A)->getPrintName(),
                       tagged2Literal(B)->getPrintName()) < 0)
          ? PROCEED : FAILED;
      }
      oz_typeError(-1,"Comparable");
    }
  }

  OZ_Return ret = bigtest(A,B,bigintLess);
  if (ret!=SUSPEND)
    return ret;

  return suspendOnNumbersAndAtoms(A,B);
}


OZ_Return BIlessInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  OZ_Return ret = BIlessInline(A,B);
  switch (ret) {
  case PROCEED: out = oz_true();  return PROCEED;
  case FAILED:  out = oz_false(); return PROCEED;
  default:      return ret;
  }
}

OZ_Return BIgreatInline(TaggedRef A, TaggedRef B)
{
  return BIlessInline(B,A);
}

OZ_Return BIgreatInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return BIlessInlineFun(B,A,out);
}


OZ_Return BIleInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {

    if (tagA == SMALLINT) {
      return (smallIntLE(A,B) ? PROCEED : FAILED);
    }

    if (isFloatTag(tagA)) {
      return (floatValue(A) <= floatValue(B)) ? PROCEED : FAILED;
    }

    if (tagA == LITERAL) {
      if (oz_isAtom(A) && oz_isAtom(B)) {
        return (strcmp(tagged2Literal(A)->getPrintName(),
                       tagged2Literal(B)->getPrintName()) <= 0)
          ? PROCEED : FAILED;
      }
      oz_typeError(-1,"Comparable");
    }

  }

  OZ_Return ret = bigtest(A,B,bigintLe);
  if (ret!=SUSPEND)
    return ret;

  return suspendOnNumbersAndAtoms(A,B);
}


OZ_Return BIleInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  OZ_Return ret = BIleInline(A,B);
  switch (ret) {
  case PROCEED: out = oz_true();  return PROCEED;
  case FAILED:  out = oz_false(); return PROCEED;
  default:      return ret;
  }
}



OZ_Return BIgeInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return BIleInlineFun(B,A,out);
}


OZ_Return BIgeInline(TaggedRef A, TaggedRef B)
{
  return BIleInline(B,A);
}

OZ_Return BILessOrLessEq(Bool callLess, TaggedRef A, TaggedRef B)
{
  return callLess ? BIlessInline(A,B) : BIleInline(A,B);
}


/* -----------------------------------
   X = conv(Y)
   ----------------------------------- */


OZ_Return BIintToFloatInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,_2);
  if (oz_isSmallInt(A)) {
    out = oz_float((double)smallIntValue(A));
    return PROCEED;
  }
  if (oz_isBigInt(A)) {
    char *s = toC(A);
    out = OZ_CStringToFloat(s);
    return PROCEED;
  }

  if (oz_isVariable(A)) {
    return SUSPEND;
  }

  oz_typeError(0,"Int");
}

/* mm2: I don't know if this is efficient, but it's only called,
   when rounding "xx.5" */
inline
Bool ozisodd(double ff)
{
  double m = ff/2;
  return m != floor(m);
}

/* ozround -- round float to int as required by IEEE Standard */
inline
double ozround(double in) {
  double ff = floor(in);
  double diff = in-ff;
  if (diff > 0.5 || (diff == 0.5 && ozisodd(ff))) {
    ff += 1;
  }
  return ff;
}

OZ_Return BIfloatToIntInline(TaggedRef A, TaggedRef &out) {
  A=oz_deref(A);

  if (oz_isVariable(A))
    return SUSPEND;

  if (oz_isFloat(A)) {
    double ff = ozround(floatValue(A));
    if (ff > INT_MAX || ff < INT_MIN) {
      OZ_warning("float to int: truncated to signed 32 Bit\n");
    }
    out = oz_int((int) ff);
    return PROCEED;
  }

  oz_typeError(-1,"Float");
}

OZ_BI_define(BIfloatToString, 1,1)
{
  oz_declareNonvarIN(0,in);

  if (oz_isFloat(in)) {
    char *s = OZ_toC(in,100,100); // mm2
    OZ_RETURN(OZ_string(s));
  }
  oz_typeError(0,"Float");
} OZ_BI_end

OZ_BI_define(BIstringToFloat, 1,1)
{
  oz_declareProperStringIN(0,str);

  char *end = OZ_parseFloat(str);
  if (!end || *end != 0) {
    return oz_raise(E_ERROR,E_KERNEL,"stringNoFloat",1,OZ_in(0));
  }
  OZ_RETURN(OZ_CStringToFloat(str));
} OZ_BI_end

OZ_BI_define(BIstringToInt, 1,1)
{
  oz_declareProperStringIN(0,str);

  if (!str) return oz_raise(E_ERROR,E_KERNEL,"stringNoInt",1,OZ_in(0));


  OZ_Term res = OZ_CStringToInt(str);
  if (res == 0)
    return oz_raise(E_ERROR,E_KERNEL,"stringNoInt",1,OZ_in(0));
  else
    OZ_RETURN(res);
} OZ_BI_end

OZ_BI_define(BIintToString, 1,1)
{
  oz_declareNonvarIN(0,in);

  if (oz_isInt(in)) {
    OZ_RETURN(OZ_string(OZ_toC(in,100,100))); //mm2
  }
  oz_typeError(0,"Int");
} OZ_BI_end

/* -----------------------------------
   type X
   ----------------------------------- */

OZ_Return BIisFloatInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isVariableTag(tag)) {
    return SUSPEND;
  }

  return isFloatTag(tag) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisFloatB,BIisFloatInline)

OZ_Return BIisIntInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isVariableTag(tag)) {
    return SUSPEND;
  }

  return oz_isInt(num) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisIntB,BIisIntInline)



OZ_Return BIisNumberInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isVariableTag(tag)) {
    return SUSPEND;
  }

  return oz_isNumber(num) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisNumberB,BIisNumberInline)


/* -----------------------------------------------------------------------
   misc. floating point functions
   ----------------------------------------------------------------------- */


#define FLOATFUN(Fun,BIName,InlineName)                 \
OZ_Return InlineName(TaggedRef AA, TaggedRef &out)      \
{                                                       \
  DEREF(AA,_,tag);                                      \
                                                        \
  if (isVariableTag(tag)) {                                     \
    return SUSPEND;                                     \
  }                                                     \
                                                        \
  if (isFloatTag(tag)) {                                        \
    out = oz_float(Fun(floatValue(AA)));                \
    return PROCEED;                                     \
  }                                                     \
  oz_typeError(0,"Float");                              \
}                                                       \
OZ_DECLAREBI_USEINLINEFUN1(BIName,InlineName)


FLOATFUN(exp, BIexp, BIinlineExp)
FLOATFUN(log, BIlog, BIinlineLog)
FLOATFUN(sqrt,BIsqrt,BIinlineSqrt)
FLOATFUN(sin, BIsin, BIinlineSin)
FLOATFUN(asin,BIasin,BIinlineAsin)
FLOATFUN(cos, BIcos, BIinlineCos)
FLOATFUN(acos,BIacos,BIinlineAcos)
FLOATFUN(tan, BItan, BIinlineTan)
FLOATFUN(atan,BIatan,BIinlineAtan)
FLOATFUN(ceil,BIceil,BIinlineCeil)
FLOATFUN(floor,BIfloor,BIinlineFloor)
FLOATFUN(ozround, BIround, BIinlineRound)
#undef FLOATFUN


OZ_Return BIfPowInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (isFloatTag(tagA) && isFloatTag(tagB)) {
    out = oz_float(pow(floatValue(A),floatValue(B)));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}

OZ_Return BIatan2Inline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (isFloatTag(tagA) && isFloatTag(tagB)) {
    out = oz_float(atan2(floatValue(A),floatValue(B)));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}


/* -----------------------------------
   make non inline versions
   ----------------------------------- */

OZ_DECLAREBI_USEINLINEFUN2(BIplus,BIplusInline)
OZ_DECLAREBI_USEINLINEFUN2(BIminus,BIminusInline)

OZ_DECLAREBI_USEINLINEFUN2(BImult,BImultInline)
OZ_DECLAREBI_USEINLINEFUN2(BIdiv,BIdivInline)
OZ_DECLAREBI_USEINLINEFUN2(BIfdiv,BIfdivInline)
OZ_DECLAREBI_USEINLINEFUN2(BImod,BImodInline)

OZ_DECLAREBI_USEINLINEFUN2(BIfPow,BIfPowInline)
OZ_DECLAREBI_USEINLINEFUN2(BIatan2,BIatan2Inline)

OZ_DECLAREBI_USEINLINEFUN2(BImax,BImaxInline)
OZ_DECLAREBI_USEINLINEFUN2(BImin,BIminInline)

OZ_DECLAREBI_USEINLINEFUN2(BIlessFun,BIlessInlineFun)
OZ_DECLAREBI_USEINLINEFUN2(BIleFun,BIleInlineFun)
OZ_DECLAREBI_USEINLINEFUN2(BIgreatFun,BIgreatInlineFun)
OZ_DECLAREBI_USEINLINEFUN2(BIgeFun,BIgeInlineFun)

OZ_DECLAREBI_USEINLINEFUN1(BIintToFloat,BIintToFloatInline)
OZ_DECLAREBI_USEINLINEFUN1(BIfloatToInt,BIfloatToIntInline)
OZ_DECLAREBI_USEINLINEFUN1(BIuminus,BIuminusInline)
OZ_DECLAREBI_USEINLINEFUN1(BIabs,BIabsInline)
OZ_DECLAREBI_USEINLINEFUN1(BIadd1,BIadd1Inline)
OZ_DECLAREBI_USEINLINEFUN1(BIsub1,BIsub1Inline)

// ---------------------------------------------------------------------
// Ports
// ---------------------------------------------------------------------

OZ_BI_define(BIisPort, 1, 1)
{
  oz_declareNonvarIN(0,t);
  OZ_RETURN(oz_isPort(t) ? oz_true() : oz_false());
} OZ_BI_end

// use VARS or FUTURES for ports
// #define VAR_PORT
#ifdef VAR_PORT

OZ_BI_define(BInewPort,1,1)
{
  oz_declareIN(0,val);

  OZ_RETURN(oz_newPort(val));
} OZ_BI_end

void doPortSend(PortWithStream *port,TaggedRef val)
{
  LTuple *lt = new LTuple(am.currentUVarPrototype(),am.currentUVarPrototype());

  OZ_Term old = ((PortWithStream*)port)->exchangeStream(lt->getTail());

  OZ_unifyInThread(old,makeTaggedLTuple(lt));
  OZ_unifyInThread(val,lt->getHead()); // might raise exception if val is non exportable
}

OZ_Return oz_sendPort(OZ_Term prt, OZ_Term val)
{
  Assert(oz_isPort(prt));

  Port *port  = tagged2Port(prt);

  CheckLocalBoard(port,"port");

  if(port->isProxy()) {
    return (*portSend)(port,val);
  }
  doPortSend((PortWithStream*)port,val);

  return PROCEED;
}

OZ_BI_define(BIsendPort,2,0)
{
  oz_declareNonvarIN(0,prt);
  oz_declareIN(1,val);

  if (!oz_isPort(prt)) {
    oz_typeError(0,"Port");
  }

  return oz_sendPort(prt,val);
} OZ_BI_end

#else

// PORTS with Futures

OZ_BI_define(BInewPort,1,1)
{
  OZ_Term fut = oz_newFuture(oz_currentBoard());
  OZ_Term port = oz_newPort(fut);

  OZ_out(0)= port;
  return oz_unify(OZ_in(0),fut);
} OZ_BI_end

// export
void doPortSend(PortWithStream *port,TaggedRef val)
{
  OZ_Term newFut = oz_newFuture(oz_currentBoard());
  OZ_Term lt  = oz_cons(am.currentUVarPrototype(),newFut);
  OZ_Term oldFut = port->exchangeStream(newFut);

  DEREF(oldFut,ptr,_);
  oz_bindFuture(ptr,lt);
  OZ_unifyInThread(val,oz_head(lt)); // might raise exception if val is non exportable
}

OZ_Return oz_sendPort(OZ_Term prt, OZ_Term val)
{
  Assert(oz_isPort(prt));

  Port *port  = tagged2Port(prt);

  CheckLocalBoard(port,"port");

  if(port->isProxy()) {
    return (*portSend)(port,val);
  }
  doPortSend((PortWithStream*)port,val);
  return PROCEED;
}

OZ_BI_define(BIsendPort,2,0)
{
  oz_declareNonvarIN(0,prt);
  oz_declareIN(1,val);

  if (!oz_isPort(prt)) {
    oz_typeError(0,"Port");
  }

  return oz_sendPort(prt,val);
} OZ_BI_end
#endif

// ---------------------------------------------------------------------
// Locks
// ---------------------------------------------------------------------

OZ_BI_define(BInewLock,0,1)
{
  OZ_RETURN(makeTaggedConst(new LockLocal(oz_currentBoard())));
} OZ_BI_end

OZ_BI_define(BIisLock, 1,1)
{
  oz_declareNonvarIN(0,term);
  OZ_RETURN(oz_isLock(term) ? oz_true() : oz_false());
} OZ_BI_end

// ---------------------------------------------------------------------
// Cell
// ---------------------------------------------------------------------

OZ_BI_define(BInewCell,1,1)
{
  OZ_Term val = OZ_in(0);

  OZ_RETURN(oz_newCell(val));
} OZ_BI_end


OZ_Return accessCell(OZ_Term cell,OZ_Term &out)
{
  Tertiary *tert=tagged2Tert(cell);
  if(!tert->isLocal()){
    out = oz_newVariable(); /* ATTENTION - clumsy */
    return (*cellDoAccess)(tert,out);
  }
  out = ((CellLocal*)tert)->getValue();
  return PROCEED;
}

OZ_Return exchangeCell(OZ_Term cell, OZ_Term newVal, OZ_Term &oldVal)
{
  CHECK_NONVAR(newVal);
  Tertiary *tert = tagged2Tert(cell);
  if(tert->isLocal()){
    CellLocal *cellLocal=(CellLocal*)tert;
    CheckLocalBoard(cellLocal,"cell");
    oldVal = cellLocal->exchangeValue(newVal);
    return PROCEED;
  } else {
    if(!tert->isProxy()){
      CellSecEmul* sec;
      if(tert->getTertType()==Te_Frame){
        sec=((CellFrameEmul*)tert)->getSec();}
      else{
        sec=((CellManagerEmul*)tert)->getSec();}
      if(sec->getState()==Cell_Lock_Valid){
        TaggedRef old=sec->getContents();
        sec->setContents(newVal);
        oldVal = old;
        return PROCEED;}}
    oldVal = oz_newVariable();
    return (*cellDoExchange)(tert,oldVal,newVal);
  }
}

OZ_BI_define(BIaccessCell,1,1)
{
  oz_declareNonvarIN(0,cell);
  if (!oz_isCell(cell)) { oz_typeError(0,"Cell"); }

  OZ_Term out;
  int ret = accessCell(cell,out);
  OZ_result(out);
  return ret;
} OZ_BI_end

OZ_BI_define(BIassignCell,2,0)
{
  oz_declareNonvarIN(0,cell);
  if (!oz_isCell(cell)) { oz_typeError(0,"Cell"); }
  oz_declareIN(1,newVal);
  // SaveDeref(newVal);
  OZ_Term oldIgnored;
  return exchangeCell(cell,newVal,oldIgnored);
} OZ_BI_end


OZ_BI_define(BIexchangeCellFun,2,1)
{
  oz_declareNonvarIN(0,cell);
  if (!oz_isCell(cell)) { oz_typeError(0,"Cell"); }
  oz_declareIN(1,newVal);
  OZ_Term old;
  int ret = exchangeCell(cell,newVal,old);
  OZ_out(0) = old;
  return ret;
} OZ_BI_end

/********************************************************************
 * Arrays
 ******************************************************************** */

OZ_BI_define(BIarrayNew,3,1)
{
  oz_declareIntIN(0,ilow);
  oz_declareIntIN(1,ihigh);
  oz_declareIN(2,initValue);

  if (!oz_isSmallInt(OZ_deref(OZ_in(0)))) { oz_typeError(0,"smallInteger"); }
  if (!oz_isSmallInt(OZ_deref(OZ_in(1)))) { oz_typeError(1,"smallInteger"); }

  OzArray *array = new OzArray(oz_currentBoard(),ilow,ihigh,initValue);
  if (array==NULL || array->getWidth()==-1) {
    return oz_raise(E_SYSTEM,E_SYSTEM,"limitExternal",1,OZ_atom("not enough memory"));
  }

  OZ_RETURN(makeTaggedConst(array));
} OZ_BI_end


OZ_Return isArrayInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term );
  out = oz_bool(oz_isArray(term));
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN1(BIisArray,isArrayInline)

OZ_Return arrayLowInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term );
  if (!oz_isArray(term)) {
    oz_typeError(0,"Array");
  }
  out = oz_int(tagged2Array(term)->getLow());
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN1(BIarrayLow,arrayLowInline)

OZ_Return arrayHighInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term );
  if (!oz_isArray(term)) {
    oz_typeError(0,"Array");
  }
  out = oz_int(tagged2Array(term)->getHigh());
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN1(BIarrayHigh,arrayHighInline)

OZ_Return arrayGetInline(TaggedRef t, TaggedRef i, TaggedRef &out)
{
  NONVAR( t, array );
  NONVAR( i, index );

  if (!oz_isArray(array)) {
    oz_typeError(0,"Array");
  }

  if (!oz_isSmallInt(index)) {
    oz_typeError(1,"smallInteger");
  }

  OzArray *ar = tagged2Array(array);
  out = ar->getArg(smallIntValue(index));
  if (out) return PROCEED;
  return oz_raise(E_ERROR,E_KERNEL,"array",2,array,index);
}
OZ_DECLAREBI_USEINLINEFUN2(BIarrayGet,arrayGetInline)

OZ_Return arrayPutInline(TaggedRef t, TaggedRef i, TaggedRef value)
{
  NONVAR( t, array );
  NONVAR( i, index );

  if (!oz_isArray(array)) {
    oz_typeError(0,"Array");
  }

  if (!oz_isSmallInt(index)) {
    oz_typeError(1,"smallInteger");
  }

  OzArray *ar = tagged2Array(array);
  CheckLocalBoard(ar,"array");
  if (ar->setArg(smallIntValue(index),value)) return PROCEED;

  return oz_raise(E_ERROR,E_KERNEL,"array",2,array,index);
}

OZ_DECLAREBI_USEINLINEREL3(BIarrayPut,arrayPutInline);

/*
 * Control Vars
 */

OZ_Return applyProc(TaggedRef proc, TaggedRef args)
{
  OZ_Term var;
  if (!OZ_isList(args,&var)) {
    if (var == 0) oz_typeError(1,"finite List");
    oz_suspendOn(var);
  }

  int len = OZ_length(args);
  RefsArray argsArray = allocateRefsArray(len);
  for (int i=0; i < len; i++) {
    argsArray[i] = OZ_head(args);
    args=OZ_tail(args);
  }
  Assert(OZ_isNil(args));

  if (!oz_isProcedure(proc) && !oz_isObject(proc)) {
    oz_typeError(0,"Procedure or Object");
  }

  am.prepareCall(proc,argsArray);
  return BI_REPLACEBICALL;
}


OZ_BI_define(BIcontrolVarHandler,1,0)
{
  OZ_Term varlist = oz_deref(OZ_in(0));

  {
    TaggedRef aux = varlist;
    while (oz_isCons(aux)) {
      TaggedRef car = oz_head(aux);
      if (oz_isVariable(oz_deref(car))) {
        am.addSuspendVarList(car);
        aux = oz_tail(aux);
      } else {
        am.emptySuspendVarList();
        goto no_suspend;
      }
    }
    /* only unbound variables found */
    return SUSPEND;
  }

no_suspend:
  for ( ; oz_isCons(varlist); varlist = oz_deref(oz_tail(varlist))) {
    TaggedRef car = oz_deref(oz_head(varlist));
    if (oz_isVariable(car))
      continue;
    if (oz_isLiteral(car) && oz_eq(car,NameUnit))
      return PROCEED;

    if (!oz_isSTuple(car))
      goto bomb;

    SRecord *tpl = tagged2SRecord(car);
    TaggedRef label = tpl->getLabel();

    if (oz_eq(label,AtomUnify)) {
      Assert(OZ_width(car)==2);
      return oz_unify(tpl->getArg(0),tpl->getArg(1));
    }

    if (oz_eq(label,AtomException)) {
      Assert(OZ_width(car)==1);
      return OZ_raise(tpl->getArg(0));
    }

    if (oz_eq(label,AtomApply)) {
      Assert(OZ_width(car)==2);
      return applyProc(tpl->getArg(0),tpl->getArg(1));
    }

    if (oz_eq(label,AtomApplyList)) {
      Assert(OZ_width(car)==1);
      TaggedRef list = reverseC(oz_deref(tpl->getArg(0)));
      while(oz_isCons(list)) {
        TaggedRef car = oz_head(list);
        if (!OZ_isPair(car))
          return oz_raise(E_ERROR,E_SYSTEM,"applyList: pair expected",1,car);
        OZ_Return aux = applyProc(OZ_getArg(car,0),OZ_getArg(car,1));
        if (aux != BI_REPLACEBICALL)
          return aux;
        list = oz_deref(oz_tail(list));
      }
      return BI_REPLACEBICALL;
    }

    goto bomb;
  }

bomb:
  return oz_raise(E_ERROR,E_SYSTEM,"controlVarHandler: no action found",1,OZ_in(0));
} OZ_BI_end



/********************************************************************
 *   Dictionaries
 ******************************************************************** */

OZ_BI_define(BIdictionaryNew,0,1)
{
  OZ_RETURN(makeTaggedConst(new OzDictionary(oz_currentBoard())));
} OZ_BI_end

OZ_BI_define(BIdictionaryKeys,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->keys());
} OZ_BI_end


OZ_BI_define(BIdictionaryMarkSafe,1,0)
{
  oz_declareDictionaryIN(0,dict);
  dict->markSafe();
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIdictionaryMarkCache,1,0)
{
  oz_declareDictionaryIN(0,dict);
  dict->markCache();
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIdictionaryEntries,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->pairs());
} OZ_BI_end


OZ_BI_define(BIdictionaryItems,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->items());
} OZ_BI_end


OZ_BI_define(BIdictionaryClone,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->clone(oz_currentBoard()));
} OZ_BI_end


OZ_Return isDictionaryInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term);
  out = oz_bool(oz_isDictionary(term));
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN1(BIisDictionary,isDictionaryInline)


#define GetDictAndKey(d,k,dict,key,checkboard)                  \
  NONVAR(d,dictaux);                                            \
  NONVAR(k,key);                                                \
  if (!oz_isDictionary(dictaux)) { oz_typeError(0,"Dictionary"); }      \
  if (!oz_isFeature(key))        { oz_typeError(1,"feature"); } \
  OzDictionary *dict = tagged2Dictionary(dictaux);              \
  if (checkboard) { CheckLocalBoard(dict,"dict"); }


OZ_Return dictionaryMemberInline(TaggedRef d, TaggedRef k, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  out = dict->member(key);
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN2(BIdictionaryMember,dictionaryMemberInline)


OZ_Return dictionaryGetInline(TaggedRef d, TaggedRef k, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  if (dict->getArg(key,out) != PROCEED) {
    return oz_raise(E_SYSTEM,E_KERNEL,"dict",2,d,k);
  }
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN2(BIdictionaryGet,dictionaryGetInline)


OZ_Return dictionaryCondGetInline(TaggedRef d, TaggedRef k, TaggedRef deflt, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  if (dict->getArg(key,out) != PROCEED) {
    out = deflt;
  }
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN3(BIdictionaryCondGet,dictionaryCondGetInline)

OZ_Return dictionaryPutInline(TaggedRef d, TaggedRef k, TaggedRef value)
{
  GetDictAndKey(d,k,dict,key,OK);
  dict->setArg(key,value);
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEREL3(BIdictionaryPut,dictionaryPutInline)

OZ_Return dictionaryRemoveInline(TaggedRef d, TaggedRef k)
{
  GetDictAndKey(d,k,dict,key,OK);
  dict->remove(key);
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEREL2(BIdictionaryRemove,dictionaryRemoveInline)


OZ_BI_define(BIdictionaryRemoveAll,1,0)
{
  oz_declareNonvarIN(0,dict);
  if (!oz_isDictionary(dict)) {
    oz_typeError(0,"Dictionary");
  }

  tagged2Dictionary(dict)->removeAll();
  return PROCEED;
} OZ_BI_end

/* -----------------------------------------------------------------
   Statistics
   ----------------------------------------------------------------- */


OZ_BI_define(BIstatisticsReset, 0,0)
{
  ozstat.initCount();
  return PROCEED;
} OZ_BI_end


#ifdef MISC_BUILTINS

OZ_BI_define(BIstatisticsPrint, 1,0)
{
  oz_declareVirtualStringIN(0,file);
  ProfileCode(ozstat.printCount(file));
  return PROCEED;
} OZ_BI_end

#ifdef PROFILE_INSTR
OZ_BI_define(BIinstructionsPrint, 0,0)
{
  ozstat.printInstr();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIinstructionsPrintCollapsable, 0,0)
{
  ozstat.printInstrCollapsable();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIinstructionsPrintReset, 0,0)
{
  ozstat.printInstrReset();
  return PROCEED;
} OZ_BI_end
#endif

#ifdef PROFILE_BI
OZ_BI_define(BIbiPrint, 0,0)
{
  unsigned long sum = 0;

  extern TaggedRef dictionary_of_builtins;
  OzDictionary * d = tagged2Dictionary(dictionary_of_builtins);
  for (int i=d->getFirst(); i>=0; i=d->getNext(i)) {
    TaggedRef v = d->getValue(i);
    if (v && oz_isBuiltin(v)) {
      Builtin * abit = tagged2Builtin(v);
      sum += abit->getCounter();
      if (abit->getCounter()!=0) {
        printf("%010lu x %s\n",abit->getCounter(),abit->getPrintName());
      }
    }
  }
  printf("----------\n%010lu\n",sum);
  return PROCEED;
} OZ_BI_end
#endif


OZ_BI_define(BIstatisticsPrintProcs, 0,0)
{
  PrTabEntry::printPrTabEntries();
  return PROCEED;
} OZ_BI_end

#endif

OZ_BI_define(BIstatisticsGetProcs, 0,1)
{
  OZ_RETURN(PrTabEntry::getProfileStats());
} OZ_BI_end

OZ_BI_define(BIsetProfileMode, 1,0)
{
  oz_declareIN(0,onoff);
  if (oz_isTrue(oz_deref(onoff))) {
    am.setProfileMode();
  } else {
    am.unsetProfileMode();
  }
  return PROCEED;
} OZ_BI_end

/* -----------------------------------------------------------------
   dynamic link objects files
   ----------------------------------------------------------------- */

OZ_BI_define(BIisForeignPointer,1,1)
{
  oz_declareNonvarIN(0,p);
  OZ_RETURN(oz_bool(OZ_isForeignPointer(p)));
} OZ_BI_end

// currently the ozm format has lines mentionning
// foreign pointers with hexadecimal addresses.  these
// are use solely for identification purposes, but unfortunately
// vary from one computation to the next which means that
// the ozm file is always different even if abstractly it truly
// hasn't changed.  To fix this problem, foreign pointers
// should be assigned integers to serve as these identification
// labels, in the order in which they are encountered (which will
// be the same from one execution to the next - unless something
// major has changed).  To support this we need to keep a mapping
// from foreign pointers to integers: we must use a dictionary.
// however foreign pointers cannot be keys.  so we provide the
// function below to use the integer value of its address as the
// key instead.

OZ_BI_define(BIForeignPointerToInt,1,1)
{
  OZ_declareForeignPointer(0,handle);
  OZ_RETURN_INT((long)handle);
} OZ_BI_end


/* ------------------------------------------------------------
 * Shutdown
 * ------------------------------------------------------------ */

OZ_BI_define(BIshutdown,1,0)
{
  oz_declareIntIN(0,status);
  am.exitOz(status);
  return(PROCEED); /* not reached but anyway */
} OZ_BI_end

/* ------------------------------------------------------------
 * Alarm und Delay
 * ------------------------------------------------------------ */

OZ_BI_define(BIalarm,2,0) {
  oz_declareIntIN(0,t);
  oz_declareIN(1,out);

  if (!oz_onToplevel()) {
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("io"));
  }

  if (t <= 0)
    return oz_unify(NameUnit,out);

  am.insertUser(t,oz_cons(NameUnit,out));
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIdelay,1,0) {
  oz_declareIntIN(0,t);

  if (!oz_onToplevel()) {
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("io"));
  }

  if (t <= 0)
    return PROCEED;

  TaggedRef var = oz_newVariable();

  am.insertUser(t,oz_cons(NameUnit,var));
  DEREF(var, var_ptr, var_tag);

  if (isVariableTag(var_tag)) {
    am.addSuspendVarList(var_ptr);
    OZ_in(0) = makeTaggedSmallInt(-1);
    return SUSPEND;
  }
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BItimeTime,0,1) {
  time_t ttt;

  time(&ttt);

  struct tm * t = gmtime(&ttt);

  int tt = (t->tm_yday * 86400 +
            t->tm_hour * 3600  +
            t->tm_min  * 60 +
            t->tm_sec);

  OZ_RETURN(makeTaggedSmallInt(tt));
} OZ_BI_end

/* ------------------------------------------------------------
 * Garbage Collection
 * ------------------------------------------------------------ */

OZ_BI_define(BIgarbageCollection,0,0)
{
  am.setSFlag(StartGC);

  return BI_PREEMPT;
} OZ_BI_end

/* ------------------------------------------------------------
 * System specials
 * ------------------------------------------------------------ */

OZ_BI_define(BIsystemEq,2,1) {
  oz_declareSafeDerefIN(0,a);
  oz_declareSafeDerefIN(1,b);
  OZ_RETURN(oz_bool(oz_eq(a,b)));
} OZ_BI_end


OZ_BI_define(BIunify,2,0)
{
  oz_declareIN(0,a);
  oz_declareIN(1,b);

  return oz_unify(a,b);
} OZ_BI_end

OZ_BI_define(BIfail,0,0)
{
  return FAILED;
} OZ_BI_end


// ------------------------------------------------------------------------
// --- Apply
// ------------------------------------------------------------------------

OZ_BI_define(BIapply,2,0)
{
  oz_declareNonvarIN(0,proc);
  oz_declareIN(1,args);

  return applyProc(proc, args);
} OZ_BI_end


/* ---------------------------------------------------------------------
 * ???
 * --------------------------------------------------------------------- */

int oz_var_getSuspListLength(OzVariable *cv);

OZ_BI_define(BIconstraints,1,1)
{
  oz_declareDerefIN(0,in);

  int len = 0;
  if (isCVar(inTag)) {
    len=oz_var_getSuspListLength(tagged2CVar(in));
  }
  OZ_RETURN_INT(len);
} OZ_BI_end


/* ---------------------------------------------------------------------
 * System
 * --------------------------------------------------------------------- */

static
OZ_Return printVS(char*s,int n, int fd, Bool newline)
{
  char c = '\n';
  if ((ossafewrite(fd,s,n) < 0) ||
      (newline && (ossafewrite(fd,&c,1) < 0))) {
    if (isDeadSTDOUT())
      am.exitOz(1);
    else
      return oz_raise(E_ERROR,E_KERNEL,"writeFailed",1,OZ_string(OZ_unixError(ossockerrno())));
  }
  return PROCEED;
}

OZ_BI_define(BIprintInfo,1,0)
{
  OZ_declareVS(0,s,n);
  return printVS(s,n,STDOUT_FILENO,NO);
} OZ_BI_end


OZ_BI_define(BIshowInfo,1,0)
{
  OZ_declareVS(0,s,n);
  return printVS(s,n,STDOUT_FILENO,OK);
} OZ_BI_end

OZ_BI_define(BIprintError,1,0)
{
  OZ_declareVS(0,s,n);
  prefixError(); // print popup code for opi
  return printVS(s,n,STDERR_FILENO,NO);
} OZ_BI_end

OZ_BI_define(BIshowError,1,0)
{
  OZ_declareVS(0,s,n);
  prefixError(); // print popup code for opi
  return printVS(s,n,STDERR_FILENO,OK);
} OZ_BI_end

OZ_BI_define(BItermToVS,3,1)
{
  oz_declareIN(0,t);
  oz_declareIntIN(1,depth);
  oz_declareIntIN(2,width);
  OZ_RETURN(OZ_string(OZ_toC(t,depth,width)));
} OZ_BI_end


/*
 * print and show are inline,
 * because it prevents the compiler from generating different code
 */

OZ_Return printInline(TaggedRef term, Bool newline = NO)
{
  char *s = OZ_toC(term,ozconf.printDepth,ozconf.printWidth);
  return printVS(s,strlen(s),STDOUT_FILENO,newline);
}

OZ_DECLAREBI_USEINLINEREL1(BIprint,printInline)


OZ_Return showInline(TaggedRef term)
{
  return printInline(term,OK);
}

OZ_DECLAREBI_USEINLINEREL1(BIshow,showInline)

// ---------------------------------------------------------------------------
// ???
// ---------------------------------------------------------------------------

TaggedRef Abstraction::DBGgetGlobals() {
  int n = getPred()->getGSize();
  OZ_Term t = OZ_tuple(oz_atom("globals"),n);
  for (int i = 0; i < n; i++) {
    OZ_putArg(t,i,getG(i));
  }
  return t;
}

OZ_BI_define(BIgetPrintName,1,1)
{
  oz_declareDerefIN(0,t);
  switch (tTag) {
  case OZCONST:
    {
      ConstTerm *rec = tagged2Const(t);
      switch (rec->getType()) {
      case Co_Builtin:
        OZ_RETURN(((Builtin *) rec)->getName());
      case Co_Abstraction:
        OZ_RETURN(((Abstraction *) rec)->getName());
      case Co_Class:
        OZ_RETURN_ATOM(((ObjectClass *) rec)->getPrintName());
      default:
        break;
      }
      break;
    }
  case UVAR: case CVAR: // FUT
    OZ_RETURN_ATOM(oz_varGetName(OZ_in(0)));
  case LITERAL:
    {
      const char *s = tagged2Literal(t)->getPrintName();
      OZ_RETURN(s? oz_atom(s): AtomEmpty);
    }
  default:
    break;
  }
  OZ_RETURN(AtomEmpty);
} OZ_BI_end

// ---------------------------------------------------------------------------

OZ_BI_define(BIonToplevel,0,1)
{

  OZ_RETURN(oz_bool(OZ_onToplevel()));
} OZ_BI_end


// ---------------------------------------------------------------------
// OO Stuff
// ---------------------------------------------------------------------

/*
 *      Construct a new SRecord to be a copy of old.
 *      This is the functionality of adjoin(old,newlabel).
 */
OZ_BI_define(BIcopyRecord,1,1)
{
  oz_declareNonvarIN(0,rec);

  switch (recTag) {
  case SRECORD:
    {
      SRecord *rec0 = tagged2SRecord(rec);
      SRecord *rec1 = SRecord::newSRecord(rec0);
      OZ_RETURN(makeTaggedSRecord(rec1));
    }
  case LITERAL:
    OZ_RETURN(rec);

  default:
    oz_typeError(0,"Determined Record");
  }
} OZ_BI_end


// perdio
inline
SRecord *getRecordFromState(RecOrCell state)
{
  if (!stateIsCell(state))
    return getRecord(state);

  Tertiary *t=getCell(state);          // shortcut
  if(t->isLocal()) { // can happen if globalized object becomes localized again
    return tagged2SRecord(oz_deref(((CellLocal*)t)->getValue()));
  }

  if(!t->isProxy()) {
    CellSecEmul* sec;
    if(t->getTertType()==Te_Frame) {
      sec=((CellFrameEmul*)t)->getSec();
    } else {
      sec=((CellManagerEmul*)t)->getSec();
    }
    if(sec->getState()==Cell_Lock_Valid) {
      TaggedRef old=oz_deref(sec->getContents());
      if (!oz_isVariable(old))
        return tagged2SRecord(old);
    }
  }
  return NULL;
}



// perdio
inline
SRecord *getStateInline(RecOrCell state, Bool isAssign, Bool newVar,
                        OZ_Term fea, OZ_Term &val, int &EmCode)
{
  SRecord *aux = getRecordFromState(state);
  if (aux)
    return aux;

  Tertiary *t=getCell(state);          // shortcut
  DEREF(fea, _1, feaTag);
  if (oz_isVariable(fea)) {
    EmCode = SUSPEND;
    return NULL;}

  if (oz_onToplevel()) {
    if(isAssign) {
      EmCode = (*cellAssignExchange)(t,fea,val);
    } else {
      if(newVar) val = oz_newVariable();
      EmCode = (*cellAtExchange)(t,fea,val);
    }
  } else {
    if(!isAssign) val = oz_newVariable();
    EmCode = (*cellAtAccess)(t,fea,val);
  }

  return NULL;
}

//perdio
SRecord *getState(RecOrCell state, Bool isAssign, OZ_Term fea,OZ_Term &val)
{
  int EmCode;
  return getStateInline(state,isAssign,TRUE,fea,val,EmCode);
}


//perdio
inline
OZ_Return doAt(SRecord *rec, TaggedRef fea, TaggedRef &out)
{
  Assert(rec!=NULL);

  DEREF(fea, _1, feaTag);
  if (!oz_isFeature(fea)) {
    if (oz_isVariable(fea)) {
      return SUSPEND;
    }
  } else {
    TaggedRef t = rec->getFeature(fea);
    if (t) {
      out = t;
      return PROCEED;
    }
  }

  oz_typeError(0,"(valid) Feature");
}

//perdio
OZ_Return atInlineRedo(TaggedRef fea, TaggedRef out)
{
  RecOrCell state = am.getSelf()->getState();
  int emC;
  SRecord *rec = getStateInline(state,NO,FALSE,fea,out,emC);
  if (rec==NULL) {
    return emC;
  }
  return doAt(rec,fea,out);
}
OZ_DECLAREBI_USEINLINEREL2(BIatRedo,atInlineRedo)

OZ_BI_define(BIat,1,1)
{
  oz_declareIN(0,fea);

  DEREF(fea, feaPtr, feaTag);
  if (!oz_isFeature(fea)) {
    if (oz_isVariable(fea)) {
      oz_suspendOnPtr(feaPtr);
    }
    oz_typeError(0,"Feature");
  }

  RecOrCell state = am.getSelf()->getState();
  if (!stateIsCell(state)) {
    SRecord *rec = getRecord(state);
    Assert(rec!=NULL);

    TaggedRef t = rec->getFeature(fea);
    if (t) {
      OZ_RETURN(t);
    }
    return oz_raise(E_ERROR,E_OBJECT,"@",2,makeTaggedConst(am.getSelf()),fea);
  } else { // perdio
    int ret;
    OZ_Term out;
    SRecord *rec = getStateInline(state,NO,TRUE,fea,out,ret);
    if (!rec) {
      if (ret==SUSPEND) {
        oz_suspendOn(fea);
      }
    } else {
      ret = doAt(rec,fea,out);
      OZ_result(out);
    }
    return ret;
  }
} OZ_BI_end

OZ_BI_define(BIassign,2,0)
{
  oz_declareIN(0,fea);
  oz_declareIN(1,value);

  DEREF(fea, feaPtr, feaTag);
  if (!oz_isFeature(fea)) {
    if (oz_isVariable(fea)) {
      oz_suspendOnPtr(feaPtr);
    }
    oz_typeError(0,"Feature");
  }

  Object *self = am.getSelf();
  CheckLocalBoard(self,"object");

  RecOrCell state = self->getState();
  if (!stateIsCell(state)) {
    SRecord *rec = getRecord(state);
    Assert(rec!=NULL);
    TaggedRef t = rec->replaceFeature(fea,value);
    if (t) {
      return PROCEED;
    } else {
      return oz_raise(E_ERROR,E_OBJECT,"<-",3,makeTaggedConst(self),fea,value);
    }
  }

  // perdio
  int emC;
  OZ_Term val=value;
  SRecord *rec = getStateInline(state,OK,TRUE,fea,val,emC);
  if (rec==NULL) {
    if (state == SUSPEND) {
      oz_suspendOn(fea);
    } else {
      return state;
    }
    return emC;
  }
  Assert(rec!=NULL);

  if (rec->replaceFeature(fea,value) == makeTaggedNULL()) {
    return oz_raise(E_ERROR,E_OBJECT,"<-",3,makeTaggedConst(self),fea,value);
  }
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIexchange,2,1)
{
  oz_declareIN(0,fea);
  oz_declareIN(1,newVal);

  DEREF(fea, feaPtr, feaTag);

  if (!oz_isFeature(fea)) {
    if (oz_isVariable(fea)) {
      oz_suspendOnPtr(feaPtr);
      return SUSPEND;
    }
    oz_typeError(1,"Feature");
  }

  RecOrCell state = am.getSelf()->getState();
  SRecord *rec;
  if (stateIsCell(state)) {
    rec = getRecordFromState(state);
    if (!rec) {
      Tertiary* t=getCell(state);
      if(!oz_onToplevel())
        return oz_raise(E_ERROR,E_OBJECT,
                      "exchange",3,
                      makeTaggedConst(am.getSelf()),fea,newVal);
      OZ_Term old;
      old=oz_newVariable();
      int ret=(*objectExchange)(t,fea,old,newVal);
      OZ_out(0) = old;
      return ret;
    }
  } else {
    rec = getRecord(state);
  }
  Assert(rec!=NULL);

  // mm2: why twice? getFea and replaceFea
  TaggedRef aux = rec->getFeature(fea);
  if (aux) {
    OZ_Term tmp = rec->replaceFeature(fea,newVal);
    Assert(tmp);
    OZ_RETURN(aux);
  }

  return oz_raise(E_ERROR,E_OBJECT,"ooExch",3,makeTaggedConst(am.getSelf()),fea,newVal); // mm2: Error
} OZ_BI_end



inline int sizeOf(SRecord *sr)
{
  return sr ? sr->sizeOf() : 0;
}

Object *newObject(SRecord *feat, SRecord *st, ObjectClass *cla, Board *b)
{
  COUNT1(sizeObjects,sizeof(Object)+sizeOf(feat)+sizeOf(st));
  COUNT1(sizeRecords,-sizeOf(feat)-sizeOf(st));
  OzLock *lck=NULL;
  if (cla->supportsLocking()) {
    lck = new LockLocal(oz_currentBoard());
    COUNT1(sizeObjects,sizeof(LockLocal));
  }
  return new Object(b,st,cla,feat,lck);
}


OZ_BI_define(BImakeClass,6,1)
{
  OZ_Term fastmeth   = OZ_in(0); { DEREF(fastmeth,_1,_2); }
  OZ_Term features   = OZ_in(1); { DEREF(features,_1,_2); }
  OZ_Term ufeatures  = OZ_in(2); { DEREF(ufeatures,_1,_2); }
  OZ_Term defmethods = OZ_in(3); { DEREF(defmethods,_1,_2); }
  OZ_Term locking    = OZ_in(4); { DEREF(locking,_1,_2); }
  OZ_Term native     = OZ_in(5); { DEREF(native,_1,_2); }

  if (!oz_isDictionary(fastmeth))   { oz_typeError(0,"dictionary"); }
  if (!oz_isRecord(features))       { oz_typeError(1,"record"); }
  if (!oz_isRecord(ufeatures))      { oz_typeError(2,"record"); }
  if (!oz_isDictionary(defmethods)) { oz_typeError(3,"dictionary"); }

  SRecord *uf = oz_isSRecord(ufeatures) ? tagged2SRecord(ufeatures) : (SRecord*)NULL;

  ObjectClass *cl = new ObjectClass(tagged2SRecord(features),
                                    tagged2Dictionary(fastmeth),
                                    uf,
                                    tagged2Dictionary(defmethods),
                                    oz_isTrue(locking),
                                    oz_isTrue(native),
                                    oz_currentBoard());

  OZ_RETURN(makeTaggedConst(cl));
} OZ_BI_end


OZ_BI_define(BIcomma,2,0)
{
  oz_declareNonvarIN(0,cl);
  cl = oz_deref(cl);

  if (!oz_isClass(cl)) {
    oz_typeError(0,"Class");
  }

  TaggedRef fb = tagged2ObjectClass(cl)->getFallbackApply();
  Assert(fb);

  am.prepareCall(fb,OZ_in(0),OZ_in(1));
  am.emptySuspendVarList();
  return BI_REPLACEBICALL;
} OZ_BI_end

OZ_BI_define(BIsend,3,0)
{
  oz_declareNonvarIN(1,cl);
  oz_declareNonvarIN(2,obj);

  cl = oz_deref(cl);
  if (!oz_isClass(cl)) {
    oz_typeError(1,"Class");
  }

  obj = oz_deref(obj);
  if (!oz_isObject(obj)) {
    oz_typeError(2,"Object");
  }

  TaggedRef fb = tagged2ObjectClass(cl)->getFallbackApply();
  Assert(fb);

  am.changeSelf(tagged2Object(obj));

  am.prepareCall(fb,OZ_in(1),OZ_in(0));
  am.emptySuspendVarList();
  return BI_REPLACEBICALL;
} OZ_BI_end

OZ_Return BIisObjectInline(TaggedRef t)
{
  DEREF(t,_1,_2);
  if (oz_isVariable(t)) return SUSPEND;
  return oz_isObject(t) ?  PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisObjectB,BIisObjectInline)


OZ_Return getClassInline(TaggedRef t, TaggedRef &out)
{
  DEREF(t,_,tag);
  if (isVariableTag(tag)) return SUSPEND;
  if (!oz_isObject(t)) {
    oz_typeError(0,"Object");
  }
  out = makeTaggedConst(tagged2Object(t)->getClass());
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN1(BIgetClass,getClassInline)




inline
TaggedRef cloneObjectRecord(TaggedRef record, Bool cloneAll)
{
  if (oz_isLiteral(record))
    return record;

  Assert(oz_isSRecord(record));

  SRecord *in  = tagged2SRecord(record);
  SRecord *rec = SRecord::newSRecord(in);

  for(int i=0; i < in->getWidth(); i++) {
    OZ_Term arg = in->getArg(i);
    if (cloneAll || oz_eq(NameOoFreeFlag,oz_deref(arg))) {
      arg = oz_newVariable();
    }
    rec->setArg(i,arg);
  }

  return makeTaggedSRecord(rec);
}

inline
OZ_Term makeObject(OZ_Term initState, OZ_Term ffeatures, ObjectClass *clas)
{
  Assert(oz_isRecord(initState) && oz_isRecord(ffeatures));

  /* state is _allways_ a record, this makes life somewhat easier */
  if (!oz_isSRecord(initState)) {
    static TaggedRef dummyRecord = 0;
    if (dummyRecord==0) {
      dummyRecord = OZ_recordInitC("noattributes",
                                   oz_list(OZ_pair2(OZ_newName(),OZ_atom("novalue")),0));
      OZ_protect(&dummyRecord);
    }
    initState = dummyRecord;
  }

  Object *out =
    newObject(oz_isSRecord(ffeatures) ? tagged2SRecord(ffeatures) : (SRecord*) NULL,
              tagged2SRecord(initState),
              clas,
              oz_currentBoard());

  return makeTaggedConst(out);
}


OZ_Return newObjectInline(TaggedRef cla, TaggedRef &out)
{
  { DEREF(cla,_1,_2); }
  if (oz_isVariable(cla)) return SUSPEND;
  if (!oz_isClass(cla)) {
    oz_typeError(0,"Class");
  }

  ObjectClass *realclass = tagged2ObjectClass(cla);
  TaggedRef attr = realclass->classGetFeature(NameOoAttr);
  { DEREF(attr,_1,_2); }
  if (oz_isVariable(attr)) return SUSPEND;

  TaggedRef attrclone = cloneObjectRecord(attr,NO);

  TaggedRef freefeat = realclass->classGetFeature(NameOoFreeFeatR);
  { DEREF(freefeat,_1,_2); }
  Assert(!oz_isVariable(freefeat));
  TaggedRef freefeatclone = cloneObjectRecord(freefeat,OK);

  out = makeObject(attrclone, freefeatclone, realclass);

  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN1(BInewObject,newObjectInline)


OZ_BI_define(BINew,3,0)
{
  oz_declareNonvarIN(0,cl);
  cl = oz_deref(cl);

  if (!oz_isClass(cl)) {
    oz_typeError(0,"Class");
  }

  ObjectClass * oc = tagged2ObjectClass(cl);

  TaggedRef fb = oc->getFallbackNew();

  Assert(fb);

  am.prepareCall(fb,OZ_in(0),OZ_in(1),OZ_in(2));
  am.emptySuspendVarList();
  return BI_REPLACEBICALL;
} OZ_BI_end


OZ_Return ooGetLockInline(TaggedRef val)
{
  OzLock *lock = am.getSelf()->getLock();
  if (lock==NULL)
    return oz_raise(E_ERROR,E_OBJECT,"locking",1,
                    makeTaggedConst(am.getSelf()));

  return oz_unify(val,makeTaggedConst(lock));
}
OZ_DECLAREBI_USEINLINEREL1(BIooGetLock,ooGetLockInline)


#ifdef MISC_BUILTINS

/********************************************************************
 * Functions
 ******************************************************************** */

OZ_BI_define(BIfunReturn,1,0)
{
  OZ_warning("funReturn should never be called");
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIgetReturn,0,1)
{
  OZ_warning("getReturn should never be called");
  return PROCEED;
} OZ_BI_end

#endif


/********************************************************************
 * Exceptions
 ******************************************************************** */
OZ_BI_define(BIraise,1,0)
{
  oz_declareIN(0,exc);

  return OZ_raise(exc);
} OZ_BI_end

OZ_BI_define(BIraiseError,1,0)
{
  oz_declareIN(0,exc);

  return OZ_raiseError(exc);
} OZ_BI_end

OZ_BI_define(BIraiseDebug,1,0)
{
  oz_declareIN(0,exc);

  return OZ_raiseDebug(exc);
} OZ_BI_end

OZ_BI_define(BIraiseDebugCheck, 1, 1) {
  oz_declareIN(0,exc);

  exc = oz_deref(exc);

  TaggedRef df;

  OZ_RETURN(oz_bool(ozconf.errorDebug &&
                    oz_isSRecord(exc) &&
                    (df = tagged2SRecord(exc)->getFeature(AtomDebug)) &&
                    oz_isSRecord(oz_deref(df))));

} OZ_BI_end


/********************************************************************
 * Finalization
 ******************************************************************** */

// Copyright © by Denys Duchier, Nov 1997, Universität des Saarlandes
//
// {Finalize.register OBJECT HANDLER}
//
//      When OBJECT becomes inaccessible {HANDLER OBJECT} is
//      eventually executed.  Both data must be at the top-level.
//      In practice, (OBJECT|HANDLER) is simply entered into the
//      `GUARDIAN' list.  Only OZCONST values can be registered.
//      Finalization makes no sense for replicable data.
//
// !!! For the moment I am ignoring distributed stuff
//
// {Finalize.setHandler HANDLER}
//
//      HANDLER is a procedure of 1 argument which is a list
//      of conses (OBJECT|HANDLER).  Its mandate is to execute
//      each {HANDLER OBJECT} safely (i.e. catching all exceptions)
//
// WHAT HAPPENS DURING GC?
//
// The GUARDIAN list is a list of conses (OBJECT|HANDLER).  During
// GC, after all other data has been collected (i.e. towards the end)
// this list is specially gc'ed as follows:
//
// *    each cons where OBJECT has already been copied (i.e. is
//      otherwise reachable through live data) is entered into the
//      guardian list in the new half-space.
//
// *    each cons where OBJECT has not already been copied (i.e.
//      is not otherwise reachable through live data) is entered
//      into the FINALIZE list.
//
// Both the new GUARDIAN and the FINALIZE lists are then processed
// to normally gc the element conses (i.e. OBJECT|HANDLER).
//
// If the FINALIZE list is not empty, a top-level thread is created
// to evaluate {FINALIZE_HANDLER FINALIZE_LIST} where FINALIZE_HANDLER
// is the handler specified by Finalize.setHandler and FINALIZE_LIST
// is the FINALIZE list.

OZ_Term guardian_list   = 0;
OZ_Term finalize_list   = 0;
OZ_Term finalize_handler= 0;

// returns 0 if non determined
//         1 if determined and (literal or top-level)
//         2 otherwise
static int finalizable(OZ_Term& x)
{
  x=oz_safeDeref(x);
  if (oz_isRef(x)) return 0;

  DEREF(x,xPtr,xTag);

  switch (xTag) {
  case UVAR:
    // FUT
  case CVAR:
    Assert(0);

    //  case SMALLINT:
    //  case FSETVALUE:
    //  case LITERAL:
    //  case OZFLOAT:
    //    return 1;
  case EXT:
    {
      Board *b = (Board *)(oz_tagged2Extension(x)->__getSpaceInternal());
      return (!b || oz_isRootBoard(b))?1:2;
    }
  case OZCONST:
    {
      ConstTerm* xp = tagged2Const(x);
      Board*b;
      switch (xp->getType()) {
      case Co_BigInt:
        return 2;
      case Co_Foreign_Pointer:
        return 1;
        // Tertiary Consts
      case Co_Abstraction:
        b = ((Abstraction*)xp)->getBoardInternal(); break;
      case Co_Builtin:
        return 1;
      case Co_Cell:
        b = ((Tertiary*)xp)->getBoardInternal(); break;
      case Co_Space:
        b = ((Space*)xp)->getBoardInternal(); break;
      case Co_Object:
        b = ((Object*)xp)->getBoardInternal(); break;
      case Co_Port:
        b = ((Port*)xp)->getBoardInternal(); break;
      case Co_Chunk:
        b = ((SChunk*)xp)->getBoardInternal(); break;
      case Co_Array:
        b = ((OzArray*)xp)->getBoardInternal(); break;
      case Co_Dictionary:
        b = ((OzDictionary*)xp)->getBoardInternal(); break;
      case Co_Lock:
        b = ((Tertiary*)xp)->getBoardInternal(); break;
      case Co_Class:
        b = ((ObjectClass*)xp)->getBoardInternal(); break;
      case Co_Resource:
        return 2; break;
      }
      return oz_isRootBoard(b)?1:2;
    }
  default:
    return 2;
  }
}

// Builtin {Finalize.register OBJECT HANDLER}

OZ_BI_define(BIfinalize_register,2,0)
{
  OZ_Term obj = OZ_in(0);
  OZ_Term hdl = OZ_in(1);
  switch (finalizable(obj)) {
  case 0: oz_suspendOn(obj);
  case 1: break;
  case 2:
    return oz_raise(E_ERROR,E_KERNEL,"nonGlobal",3,
                    oz_atom("Finalize.register"),
                    oz_atom("object"),obj);
  }
  switch (finalizable(hdl)) {
  case 0: oz_suspendOn(hdl);
  case 1: break;
  case 2:
    return oz_raise(E_ERROR,E_KERNEL,"nonGlobal",3,
                    oz_atom("Finalize.register"),
                    oz_atom("handler"),hdl);
  }
  if (guardian_list==0) guardian_list=oz_nil();
  guardian_list = oz_cons(oz_cons(obj,hdl),guardian_list);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIfinalize_setHandler,1,0)
{
  OZ_Term hdl = OZ_in(0);
  switch (finalizable(hdl)) {
  case 0: oz_suspendOn(hdl);
  case 1: break;
  case 2: return oz_raise(E_ERROR,E_KERNEL,"nonGlobal",2,
                          oz_atom("Finalize.setHandler"),
                          hdl);
  }
  if (!(oz_isProcedure(hdl)||oz_isObject(hdl)))
    oz_typeError(0,"Procedure|Object");
  finalize_handler = hdl;
  return PROCEED;
} OZ_BI_end

/*
 * Exceptions
 */

int oz_raise(OZ_Term cat, OZ_Term key, char *label, int arity, ...)
{
  Assert(!oz_isRef(cat));
  OZ_Term exc=OZ_tuple(key,arity+1);
  OZ_putArg(exc,0,OZ_atom(label));

  va_list ap;
  va_start(ap,arity);

  for (int i = 0; i < arity; i++) {
    OZ_putArg(exc,i+1,va_arg(ap,OZ_Term));
  }

  va_end(ap);


  OZ_Term ret = OZ_record(cat,
                          oz_cons(OZ_int(1),
                                  oz_cons(AtomDebug,oz_nil())));
  OZ_putSubtree(ret,OZ_int(1),exc);
  OZ_putSubtree(ret,AtomDebug,NameUnit);

  am.setException(ret, oz_eq(cat,E_ERROR) ? TRUE : ozconf.errorDebug);
  return RAISE;
}

OZ_Term oz_getLocation(Board *bb)
{
  OZ_Term out = oz_nil();
  while (!oz_isRootBoard(bb)) {
    if (bb->isSolve()) {
      out = oz_cons(OZ_atom("space"),out);
    } else if (bb->isAsk()) {
      out = oz_cons(OZ_atom("cond"),out);
    } else if (bb->isWait()) {
      out = oz_cons(OZ_atom("dis"),out);
    } else {
      out = oz_cons(OZ_atom("???"),out);
    }
    bb=bb->getParent();
  }
  return out;
}

/*===================================================================
 * type errors
 *=================================================================== */

static
char *getTypeOfPos(char * t, int p)
{
  static char buffer[100];
  int i, bi, comma;

  for (i = 0, comma = 0; t[i] != '\0' && comma < p; i += 1) {
    if (t[i] == ',') comma += 1;
    if (t[i] == '\\' && t[i+1] == ',') i += 1;
  }

  for (bi = 0; t[i] != '\0' && t[i] != ','; i += 1, bi += 1) {
    if (t[i] == '\\' && t[i+1] == ',') i += 1;
    buffer[bi] = t[i];
  }

  buffer[bi] = '\0';

  return buffer;
}

OZ_Return typeError(int Pos, char *Comment, char *TypeString)
{
  (void) oz_raise(E_ERROR,E_KERNEL,
                  "type",5,NameUnit,NameUnit,
                  OZ_atom(getTypeOfPos(TypeString, Pos)),
                  OZ_int(Pos+1),
                  OZ_string(Comment));
  return BI_TYPE_ERROR;
}
