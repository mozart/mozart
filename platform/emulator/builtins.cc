/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte (schulte@dfki.de)
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

#include "perdio.hh"
#include "os.hh"
#include "codearea.hh"
#include "threadInterface.hh"
#include "debug.hh"
#include "iso-ctype.hh"
#include "genvar.hh"
#include "ofgenvar.hh"
#include "fdbuilti.hh"
#include "fdhook.hh"
#include "solve.hh"
#include "oz_cpi.hh"
#include "dictionary.hh"

#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

/********************************************************************
 * `builtin`
 ******************************************************************** */

OZ_BI_define(BIbuiltin,2,1)
{
  OZ_declareVirtualStringIN(0,name);
  OZ_declareIntIN(1,arity);

  Builtin *found = string2Builtin(name);

  if (!found) {
    return oz_raise(E_ERROR,E_SYSTEM,"builtinUndefined",1,
                    oz_atom(name));
  }

  if (arity!=-1 && arity != found->getArity()) {
    return oz_raise(E_ERROR,E_SYSTEM,"builtinArity",3,
                    oz_atom(name),oz_int(arity),
                    makeTaggedSmallInt(found->getArity()));
  }

  OZ_RETURN(makeTaggedConst(found));
} OZ_BI_end


/********************************************************************
 * Type tests
 ******************************************************************** */

OZ_Return isValueInline(TaggedRef val)
{
  NONVAR( val, _1);
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEREL1(BIisValue,isValueInline)


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


OZ_Return isLiteralInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
        {
          GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
          if (ofsvar->getWidth()>0) return FAILED;
          return SUSPEND;
        }
      default:
          return FAILED;
      }
  }
  return isLiteralTag(tag) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisLiteralB,isLiteralInline)


OZ_Return isAtomInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
        {
          GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
          TaggedRef lbl=ofsvar->getLabel();
          DEREF(lbl,_1,lblTag);
          if (isLiteralTag(lblTag) && !oz_isAtom(lbl)) return FAILED;
          if (ofsvar->getWidth()>0) return FAILED;
          return SUSPEND;
        }
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
        // return tagged2CVar(term)->isAtomV();
          return SUSPEND;
      }
  }
  return oz_isAtom(term) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisAtomB,isAtomInline)

OZ_Return isLockInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR( t, term, tag );
  return oz_isLock(term) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisLockB,isLockInline)

inline
OZ_Return isFreeRelInline(TaggedRef term) {
  DEREF(term, _1, tag);
  if (oz_isFree(term)) return PROCEED;
  return FAILED;
}

OZ_DECLAREBOOLFUN1(BIisFree,isFreeRelInline)

inline
OZ_Return isKindedRelInline(TaggedRef term) {
  DEREF(term, _1, tag);
  if (oz_isFree(term)) return FAILED;
  return PROCEED;
}

OZ_DECLAREBOOLFUN1(BIisKinded,isKindedRelInline)

OZ_Return isDetRelInline(TaggedRef term) {
  DEREF(term, _1, _2);
  return oz_isVariable(term) ? FAILED : PROCEED;
}

OZ_DECLAREBOOLFUN1(BIisDet,isDetRelInline)



OZ_Return isNameInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
        {
          GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
          TaggedRef lbl=ofsvar->getLabel();
          Assert(lbl!=makeTaggedNULL());
          DEREF(lbl,_1,lblTag);
          if (oz_isAtom(lbl)) return FAILED;
          if (ofsvar->getWidth()>0) return FAILED;
          return SUSPEND;
        }
      default:
          return FAILED;
      }
  }
  if (!isLiteralTag(tag)) return FAILED;
  return oz_isAtom(term) ? FAILED: PROCEED;
}

OZ_DECLAREBOOLFUN1(BIisNameB,isNameInline)


OZ_Return isTupleInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  return oz_isTuple(term) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisTupleB,isTupleInline)


OZ_Return isRecordInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  return oz_isRecord(term) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisRecordB,isRecordInline)


OZ_Return isProcedureInline(TaggedRef t)
{
  NONVAR( t, term);
  return oz_isProcedure(term) ? PROCEED : FAILED;
}

OZ_DECLAREBOOLFUN1(BIisProcedureB,isProcedureInline)

OZ_Return isChunkInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  return oz_isChunk(term) ? PROCEED : FAILED;
}
OZ_DECLAREBOOLFUN1(BIisChunkB,isChunkInline)

OZ_Return procedureArityInline(TaggedRef procedure, TaggedRef &out)
{
  NONVAR( procedure, pterm );

  if (oz_isProcedure(pterm)) {
    int arity;
    ConstTerm *rec = tagged2Const(pterm);

    switch (rec->getType()) {
    case Co_Abstraction:
      arity = ((Abstraction *) rec)->getArity();
      break;
    case Co_Builtin:
      arity = ((Builtin *) rec)->getArity();
      break;
    default:
      goto typeError;
    }
    out = newSmallInt(arity);
    return PROCEED;
  }
  goto typeError;

typeError:
  out = oz_nil();
  oz_typeError(0,"Procedure");
}

OZ_DECLAREBI_USEINLINEFUN1(BIprocedureArity,procedureArityInline)

OZ_Return isCellInline(TaggedRef cell)
{
  NONVAR( cell, term);
  return oz_isCell(term) ? PROCEED : FAILED;
}
OZ_DECLAREBOOLFUN1(BIisCellB,isCellInline)

OZ_Return isPortInline(TaggedRef port)
{
  NONVAR( port, term );
  return oz_isPort(term) ? PROCEED : FAILED;
}
OZ_DECLAREBOOLFUN1(BIisPortB,isPortInline)

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

  Thread *it = oz_newThreadInject(prio, solveBoard);
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
  ca->orig_start = sa->orig_start;
  ca->copy_start = sa->copy_start;
  ca->copy_size  = sa->copy_size;
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

  OZ_RETURN(oz_isSpace(tagged_space) ? NameTrue : NameFalse);
} OZ_BI_end


OZ_BI_define(BIaskSpace, 1,1) {
  declareSpace();

  if (space->isProxy()) {
    OZ_out(0) = oz_newVariable();
    return remoteSend(space,"Space.ask",OZ_out(0));
  }

  if (space->isFailed()) OZ_RETURN(AtomFailed);

  if (space->isMerged()) OZ_RETURN(AtomMerged);

  TaggedRef answer = space->getSolveActor()->getResult();

  DEREF(answer, answer_ptr, answer_tag);

  if (isVariableTag(answer_tag))
    oz_suspendOn(makeTaggedRef(answer_ptr));

  OZ_RETURN((oz_isSTuple(answer) &&
             literalEq(tagged2SRecord(answer)->getLabel(),
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

  if (space->isProxy()) {
    return remoteSend(space,"Space.askVerbose",out);
  }

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

  if (space->isProxy()) {
    OZ_out(0) = oz_newVariable();
    return remoteSend(space,"Space.merge",OZ_out(0));
  }

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

  if (space->isProxy()) {
    OZ_out(0) = oz_newVariable();
    return remoteSend(space,"Space.clone",OZ_out(0));
  }

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

  if (space->isProxy()) {
    return remoteSend(space,"Space.commit",choice);
  }

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
             literalEq(AtomPair,
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

  if (space->isProxy()) {
    return remoteSend(space,"Space.inject",proc);
  }

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


#ifdef MISC_BUILTINS

#ifdef CS_PROFILE
OZ_BI_define(BIgetCloneDiff, 1,1) {
  declareSpace();

  if (space->isMerged())
    return oz_raise(E_ERROR,E_KERNEL,"spaceMerged",1,tagged_space);

  OZ_RETURN(space->getSolveActor()->getCloneDiff());
} OZ_BI_end

#endif

#endif

// ---------------------------------------------------------------------
// Tuple
// ---------------------------------------------------------------------

OZ_Return tupleInline(TaggedRef label, TaggedRef argno, TaggedRef &out)
{
  DEREF(argno,_1,argnoTag);
  DEREF(label,_2,labelTag);

  if (isSmallIntTag(argnoTag)) {
    if (isLiteralTag(labelTag)) {
      int i = smallIntValue(argno);

      if (i < 0) {
        goto typeError1;
      }

      // literals
      if (i == 0) {
        out = label;
        return PROCEED;
      }

      {
        SRecord *s = SRecord::newSRecord(label,i);

        for (int j = 0; j < i; j++) {
          s->setArg(j,am.currentUVarPrototype());
        }

        out = s->normalize();
        return PROCEED;
      }
    }
    if (isVariableTag(labelTag)) {
      return SUSPEND;
    }
    goto typeError0;
  }
  if (isVariableTag(argnoTag)) {
    if (isVariableTag(labelTag) || isLiteralTag(labelTag)) {
      return SUSPEND;
    }
    goto typeError0;
  }
  goto typeError1;

 typeError0:
  oz_typeError(0,"Literal");
 typeError1:
  oz_typeError(1,"(non-negative small) Int");
}

OZ_DECLAREBI_USEINLINEFUN2(BItuple,tupleInline)


// ---------------------------------------------------------------------
// Tuple & Record
// ---------------------------------------------------------------------


OZ_Return labelInline(TaggedRef term, TaggedRef &out)
{
  // Wait for term to be a record with determined label:
  // Get the term's label, if it exists
  DEREF(term,_1,tag);
  switch (tag) {
  case LTUPLE:
    out=AtomCons;
    return PROCEED;
  case LITERAL:
    out=term;
    return PROCEED;
  case SRECORD:
  record:
    out=tagged2SRecord(term)->getLabel();
    return PROCEED;
  case UVAR:
  case SVAR:
    return SUSPEND;
  case CVAR:
    switch (tagged2CVar(term)->getType()) {
    case OFSVariable:
      {
        TaggedRef thelabel=tagged2GenOFSVar(term)->getLabel();
        DEREF(thelabel,_1,_2);
        if (oz_isVariable(thelabel)) return SUSPEND;
        out=thelabel;
        return PROCEED;
      }
    case FDVariable:
    case BoolVariable:
        oz_typeError(0,"Record");
    default:
        return SUSPEND;
    }
  default:
    oz_typeError(0,"Record");
  }
}

OZ_DECLAREBI_USEINLINEFUN1(BIlabel,labelInline)

OZ_Return hasLabelInline(TaggedRef term, TaggedRef &out)
{
  // Wait for term to be a record with determined label:
  // Get the term's label, if it exists
  DEREF(term,_1,tag);
  switch (tag) {
  case LTUPLE:
    out=NameTrue;
    return PROCEED;
  case LITERAL:
    out=NameTrue;
    return PROCEED;
  case SRECORD:
  record:
    out=NameTrue;
    return PROCEED;
  case UVAR:
  case SVAR:
    out=NameFalse;
    return PROCEED;
  case CVAR:
    switch (tagged2CVar(term)->getType()) {
    case OFSVariable:
      {
        TaggedRef thelabel=tagged2GenOFSVar(term)->getLabel();
        DEREF(thelabel,_1,_2);
        out = oz_isVariable(thelabel) ? NameFalse : NameTrue;
        return PROCEED;
      }
    case FDVariable:
    case BoolVariable:
      oz_typeError(0,"Record");
    default:
      out=NameFalse;
      return PROCEED;
    }
  default:
    oz_typeError(0,"Record");
  }
}

OZ_DECLAREBI_USEINLINEFUN1(BIhasLabel,hasLabelInline)

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
    case SVAR:
    case UVAR:
      return SUSPEND;
    case CVAR:
      switch (tagged2CVar(term)->getType()) {
      case FDVariable:
      case BoolVariable:
          goto typeError0;
      default:
          return SUSPEND;
      }
      // if (tagged2CVar(term)->getType() == OFSVariable) return SUSPEND;
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
  case SVAR:
    if (!oz_isFeature(fea)) {
      oz_typeError(1,"Feature");
    }
    return SUSPEND;

  case CVAR:
    switch (tagged2CVar(term)->getType()) {
    case OFSVariable:
      {
        int ret = tagged2GenOFSVar(term)->hasFeature(fea,out);
        if (ret == FAILED) goto typeError0;
        return ret;
      }
    case FDVariable:
    case BoolVariable:
    case FSetVariable:
      goto typeError0;
    default:
      return SUSPEND;
    }

  case LITERAL:
    if (dot) goto raise; else return FAILED;

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

OZ_BI_define(BIhasFeatureB,2,1)
{
  OZ_Return r = hasFeatureInline(OZ_in(0),OZ_in(1));
  switch (r) {
  case PROCEED: OZ_RETURN(NameTrue);
  case FAILED : OZ_RETURN(NameFalse);
  case SUSPEND: oz_suspendOn2(OZ_in(0),OZ_in(1));
  default     : return r;
  }
} OZ_BI_end

OZ_Return hasFeatureBInline(TaggedRef val1, TaggedRef val2, TaggedRef &out)
{
  OZ_Return r = hasFeatureInline(val1,val2);
  switch (r) {
  case PROCEED: out=NameTrue; return PROCEED;
  case FAILED : out=NameFalse; return PROCEED;
  default: return r;
  }
}

inline
OZ_Return subtreeInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
  return genericDot(term,fea,&out,FALSE);
}

/*
 * fun {matchDefault Term Attr Defau}
 *    if X in Term.Attr = X then X else Defau fi
 * end
 */

OZ_Return matchDefaultInline(TaggedRef term, TaggedRef attr, TaggedRef defau,
                             TaggedRef &out)
{
  switch(subtreeInline(term,attr,out)) {
  case PROCEED:
    return PROCEED;
  case SUSPEND:
    return SUSPEND;
  case FAILED:
  default:
    out = defau;
    return PROCEED;
  }
}

OZ_DECLAREBI_USEINLINEFUN3(BImatchDefault,matchDefaultInline)


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
  case SVAR:
    return SUSPEND;
  case CVAR:
    switch (tagged2CVar(term)->getType()) {
    case OFSVariable:
        return SUSPEND;
    case FDVariable:
    case BoolVariable:
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

OZ_Return isUnitInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR( t, term, tag);
  if (isCVar(tag)) {
    switch (tagged2CVar(term)->getType()) {
    case OFSVariable:
      {
        GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
        if (ofsvar->getWidth()>0) return FAILED;
        TaggedRef lbl=ofsvar->getLabel();
        DEREF(lbl,_1,lblTag);
        if (isLiteralTag(lblTag)) {
          if (oz_isAtom(lbl)) {
            if (literalEq(term,NameUnit))
              return SUSPEND;
            else
              return FAILED;
          } else { // isName
            return FAILED;
          }
        }
        return SUSPEND;
      }
    case FDVariable:
    case BoolVariable:
      return FAILED;
    default:
      return SUSPEND;
    }
  }
  if (literalEq(term,NameUnit))
    return PROCEED;
  else
    return FAILED;
}

OZ_DECLAREBOOLFUN1(BIisUnitB,isUnitInline)

// ---------------------------------------------------------------------
// Bool things
// ---------------------------------------------------------------------

OZ_Return isBoolInline(TaggedRef t)
{
  SUSPEND_ON_FREE_VAR(t, term, tag);
  if (isCVar(tag)) {
    switch (tagged2CVar(term)->getType()) {
    case OFSVariable:
      {
        GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
        if (ofsvar->getWidth()>0) return FAILED;
        TaggedRef lbl=ofsvar->getLabel();
        DEREF(lbl,_1,lblTag);
        if (isLiteralTag(lblTag)) {
          if (oz_isAtom(lbl)) {
            if (literalEq(term,NameTrue) ||
                literalEq(term,NameFalse))
              return SUSPEND;
            else
              return FAILED;
          } else { // isName
            return FAILED;
          }
        }
        return SUSPEND;
      }
    case FDVariable:
    case BoolVariable:
      return FAILED;
    default:
      return SUSPEND;
    }
  }
  if (literalEq(term,NameTrue) || literalEq(term,NameFalse))
    return PROCEED;
  else
    return FAILED;
}

OZ_DECLAREBOOLFUN1(BIisBoolB,isBoolInline)

OZ_Return notInline(TaggedRef A, TaggedRef &out)
{
  NONVAR(A,term);

  if (literalEq(term,NameTrue)) {
    out = NameFalse;
    return PROCEED;
  } else {
    if (literalEq(term,NameFalse)) {
      out = NameTrue;
      return PROCEED;
    }
  }

  oz_typeError(0,"Bool");
}

OZ_DECLAREBI_USEINLINEFUN1(BInot,notInline)


OZ_Return andInline(TaggedRef A, TaggedRef B, TaggedRef &out) {
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (literalEq(A,NameTrue)) {
    if (oz_isVariable(B)) {
      return SUSPEND;
    } else if (literalEq(B,NameTrue) || literalEq(B,NameFalse)) {
      out = B;
      return PROCEED;
    } else {
      oz_typeError(1,"Bool");
    }
  } else if (literalEq(A,NameFalse)) {
    if (oz_isVariable(B)) {
      return SUSPEND;
    } else if (literalEq(B,NameTrue) || literalEq(B,NameFalse)) {
      out = NameFalse;
      return PROCEED;
    } else {
      oz_typeError(1,"Bool");
    }
  } else if (oz_isVariable(A)) {
    return SUSPEND;
  } else {
    oz_typeError(0,"Bool");
  }
}

OZ_DECLAREBI_USEINLINEFUN2(BIand,andInline)


OZ_Return orInline(TaggedRef A, TaggedRef B, TaggedRef &out) {
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (literalEq(A,NameTrue)) {
    if (oz_isVariable(B)) {
      return SUSPEND;
    } else if (literalEq(B,NameTrue) || literalEq(B,NameFalse)) {
      out = NameTrue;
      return PROCEED;
    } else {
      oz_typeError(1,"Bool");
    }
  } else if (literalEq(A,NameFalse)) {
    if (oz_isVariable(B)) {
      return SUSPEND;
    } else if (literalEq(B,NameTrue) || literalEq(B,NameFalse)) {
      out = B;
      return PROCEED;
    } else {
      oz_typeError(1,"Bool");
    }
  } else if (oz_isVariable(A)) {
    return SUSPEND;
  } else {
    oz_typeError(0,"Bool");
  }
}

OZ_DECLAREBI_USEINLINEFUN2(BIor,orInline)



// ---------------------------------------------------------------------
// Atom
// ---------------------------------------------------------------------

OZ_Return atomToStringInline(TaggedRef t, TaggedRef &out)
{
  DEREF(t,_1,_2);
  if (oz_isVariable(t)) return SUSPEND;

  if (!oz_isAtom(t)) {
    oz_typeError(-1,"atom");
  }

  out = OZ_string(tagged2Literal(t)->getPrintName());
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN1(BIatomToString,atomToStringInline)


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
             literalEq(tagged2SRecord(vs)->getLabel(),AtomPair)) {
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
    if (literalEq(vs,AtomPair) ||
        literalEq(vs,AtomNil))
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
             literalEq(tagged2SRecord(vs)->getLabel(),AtomPair)) {
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
  OZ_RETURN((status == PROCEED) ? NameTrue : NameFalse);
} OZ_BI_end


// ---------------------------------------------------------------------
// Chunk
// ---------------------------------------------------------------------

OZ_BI_define(BInewChunk,1,1)
{
  oz_declareNonvarIN(0,val);

  if (!oz_isRecord(val)) oz_typeError(0,"Record");

  OZ_RETURN(oz_newChunk(val));
} OZ_BI_end

/* ---------------------------------------------------------------------
 * Threads
 * --------------------------------------------------------------------- */

OZ_BI_define(BIthreadThis,0,1)
{
  OZ_RETURN(makeTaggedConst(oz_currentThread()));
} OZ_BI_end

/*
 * change priority of a thread
 *  if my priority is lowered, then preempt me
 *  if priority of other thread become higher than mine, then preempt me
 */
OZ_BI_define(BIthreadSetPriority,2,0)
{
  oz_declareThreadIN(0,th);
  oz_declareNonvarIN(1,atom_prio);

  int prio;

  if (!oz_isAtom(atom_prio))
    goto type_goof;

  if (literalEq(atom_prio, AtomLow)) {
    prio = LOW_PRIORITY;
  } else if (literalEq(atom_prio, AtomMedium)) {
    prio = MID_PRIORITY;
  } else if (literalEq(atom_prio, AtomHigh)) {
    prio = HI_PRIORITY;
  } else {
  type_goof:
    oz_typeError(1,"Atom [low medium high]");
  }

  if (th->isProxy()) {
    return remoteSend(th,"Thread.setPriority",atom_prio);
  }

  if (th->isDeadThread()) return PROCEED;

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
  oz_declareThreadIN(0,th);

  if (th->isProxy()) {
    OZ_out(0) = oz_newVariable();
    return remoteSend(th,"Thread.getPriority",OZ_out(0));
  }

  OZ_RETURN(threadGetPriority(th));
} OZ_BI_end

OZ_BI_define(BIthreadIs,1,1)
{
  oz_declareNonvarIN(0,th);

  OZ_RETURN(oz_isThread(th)?NameTrue:NameFalse);
} OZ_BI_end

/*
 * raise exception on thread
 */
OZ_C_proc_proto(BIraise);
OZ_C_proc_proto(BIraiseDebug);

void threadRaise(Thread *th,OZ_Term E,int debug) {
  Assert(oz_currentThread() != th);

  if (th->isDeadThread()) return;

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
  oz_declareThreadIN(0,th);
  oz_declareNonvarIN(1,E);

  if (th->isProxy()) {
    return remoteSend(th,"Thread.raise",E);
  }

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
  oz_declareThreadIN(0,th);

  if (th->isProxy()) {
    return remoteSend(th,"Thread.suspend",oz_nil());
  }

  th->setStop(OK);
  if (th == oz_currentThread()) {
    return BI_PREEMPT;
  }
  return PROCEED;
} OZ_BI_end

void threadResume(Thread *th) {
  th->setStop(NO);

  if (th->isDeadThread()) return;

  if (th->isRunnable() && !am.threadsPool.isScheduledSlow(th)) {
    am.threadsPool.scheduleThread(th);
  }
}

OZ_BI_define(BIthreadResume,1,0)
{
  oz_declareThreadIN(0,th);

  if (th->isProxy()) {
    return remoteSend(th,"Thread.resume",oz_nil());
  }


  threadResume(th);

  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIthreadIsSuspended,1,1)
{
  oz_declareThreadIN(0,th);

  if (th->isProxy()) {
    OZ_out(0) = oz_newVariable();
    return remoteSend(th,"Thread.isSuspended",OZ_out(0));
  }

  OZ_RETURN(th->getStop() ? NameTrue : NameFalse);
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

  if (th->isProxy()) {
    OZ_out(0) = oz_newVariable();
    return remoteSend(th,"Thread.state",OZ_out(0));
  }

  OZ_RETURN(threadState(th));
} OZ_BI_end

OZ_BI_define(BIthreadPreempt,1,0)
{
  oz_declareThreadIN(0,th);

  if (th->isProxy())
    return oz_raise(E_ERROR,E_SYSTEM,"threadPreempt Proxy not impl",0);

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
  OZ_RETURN(getUniqueName(name));
} OZ_BI_end

// ---------------------------------------------------------------------
// term type
// ---------------------------------------------------------------------

OZ_Return BItermTypeInline(TaggedRef term, TaggedRef &out)
{
  out = OZ_termType(term);
  if (oz_eq(out,oz_atom("variable"))) {
    return SUSPEND;
  }
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN1(BItermType,BItermTypeInline)

OZ_BI_define(BIstatus,1,1)
{
  oz_declareIN(0,term);

  DEREF(term, _1, tag);

  switch (tag) {
  case UVAR:
  case SVAR:
    OZ_RETURN(AtomFree);
  case CVAR:
    if (oz_isFree(term)) {
      OZ_RETURN(AtomFree);
    } else {
      SRecord *t = SRecord::newSRecord(AtomKinded, 1);
      switch (tagged2CVar(term)->getType()) {
      case FDVariable:
      case BoolVariable:
        t->setArg(0, AtomInt); break;
      case FSetVariable:
        t->setArg(0, AtomFSet); break;
      case OFSVariable:
        t->setArg(0, AtomRecord); break;
      default:
        t->setArg(0, AtomOther); break;
      }
      OZ_RETURN(makeTaggedSRecord(t));
    }
  default:
    {
      SRecord *t = SRecord::newSRecord(AtomDet, 1);
      t->setArg(0, OZ_termType(term));
      OZ_RETURN(makeTaggedSRecord(t));
    }
  }
  return PROCEED;
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

  if (isLiteralTag(tagA))  return literalEq(A,B)  ? PROCEED : FAILED;

  if (A == B && !oz_isVariable(A)) return PROCEED;

  if (isConstTag(tagA)) {
    if (oz_isBigInt(A)) return bigIntEq(A,B) ? PROCEED : FAILED;
    return FAILED;
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
    out = NameTrue;
    return PROCEED;
  case FAILED:
    out = NameFalse;
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
    out = NameFalse;
    return PROCEED;
  case FAILED:
    out = NameTrue;
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
    if (var == 0) OZ_RETURN(NameFalse);
    oz_suspendOn(var);
  }
  OZ_RETURN(NameTrue);
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
  OZ_RETURN(TEST ((unsigned char) i) ? NameTrue : NameFalse);

OZ_BI_define(BIcharIs,1,1) {
 oz_declareNonvarIN(0,c);
 c = oz_deref(c);
 if (!oz_isSmallInt(c)) OZ_RETURN(NameFalse);
 int i = smallIntValue(c);
 OZ_RETURN((i >=0 && i <= 255) ? NameTrue : NameFalse);
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


OZ_Return BIadjoinInline(TaggedRef t0, TaggedRef t1, TaggedRef &out)
{
  DEREF(t0,_0,tag0);
  DEREF(t1,_1,tag1);

  switch (tag0) {
  case LITERAL:
    switch (tag1) {
    case SRECORD:
    case LITERAL:
    case LTUPLE:
      out = t1;
      return PROCEED;
    case UVAR:
    case SVAR:
      return SUSPEND;
    case CVAR:
      switch (tagged2CVar(t1)->getType()) {
      case FDVariable:
      case BoolVariable:
          oz_typeError(1,"Record");
      default:
          return SUSPEND;
      }
    default:
      oz_typeError(1,"Record");
    }
  case LTUPLE:
  case SRECORD:
    {
      SRecord *rec= makeRecord(t0);
      switch (tag1) {
      case LITERAL:
        {
          SRecord *newrec = SRecord::newSRecord(rec);
          newrec->setLabelForAdjoinOpt(t1);
          out = newrec->normalize();
          return PROCEED;
        }
      case SRECORD:
      case LTUPLE:
        {
          out = oz_adjoin(rec,makeRecord(t1));
          return PROCEED;
        }
      case UVAR:
      case SVAR:
        return SUSPEND;
      case CVAR:
        switch (tagged2CVar(t1)->getType()) {
        case FDVariable:
        case BoolVariable:
            oz_typeError(1,"Record");
        default:
            return SUSPEND;
        }
      default:
        oz_typeError(1,"Record");
      }
    }
  case UVAR:
  case SVAR:
  case CVAR:
    if (tag0==CVAR) {
        switch (tagged2CVar(t0)->getType()) {
        case FDVariable:
        case BoolVariable:
          oz_typeError(0,"Record");
        default:
          break;
        }
    }
    switch (tag1) {
    case UVAR:
    case SVAR:
    case SRECORD:
    case LTUPLE:
    case LITERAL:
      return SUSPEND;
    case CVAR:
      switch (tagged2CVar(t1)->getType()) {
      case FDVariable:
      case BoolVariable:
          oz_typeError(1,"Record");
      default:
        return SUSPEND;
      }
    default:
      oz_typeError(1,"Record");
    }
  default:
    oz_typeError(0,"Record");
  }
}

OZ_DECLAREBI_USEINLINEFUN2(BIadjoin,BIadjoinInline)

OZ_BI_define(BIadjoinAt,3,1)
{
  OZ_Term rec = OZ_in(0);
  OZ_Term fea = OZ_in(1);
  OZ_Term value = OZ_in(2);

  DEREF(rec,recPtr,recTag);
  DEREF(fea,feaPtr,feaTag);

  switch (recTag) {
  case LITERAL:
    if (oz_isFeature(fea)) {
      SRecord *newrec
        = SRecord::newSRecord(rec,aritytable.find(oz_cons(fea,oz_nil())));
      newrec->setArg(0,value);
      OZ_RETURN(makeTaggedSRecord(newrec));
    }
    if (oz_isFree(fea)) {
      oz_suspendOnPtr(feaPtr);
    }
    if (isCVar(fea)) {
      if (tagged2CVar(fea)->getType()!=OFSVariable ||
          tagged2GenOFSVar(fea)->getWidth()>0)
        oz_typeError(1,"Feature");
      oz_suspendOnPtr(feaPtr);;
    }
    oz_typeError(1,"Feature");

  case LTUPLE:
  case SRECORD:
    {
      if (isVariableTag(feaTag)) {
        oz_suspendOnPtr(feaPtr);
      }
      if (!oz_isFeature(fea)) {
        oz_typeError(1,"Feature");
      }
      OZ_RETURN(oz_adjoinAt(makeRecord(rec),fea,value));
    }

  case UVAR:
  case SVAR:
  case CVAR:
    if (!oz_isFree(rec) && tagged2CVar(rec)->getType()!=OFSVariable) {
      oz_typeError(0,"Record");
    } else if (oz_isFeature(fea) || oz_isFree(fea)) {
      oz_suspendOnPtr(recPtr);
    } else if (isCVar(fea)) {
      if (tagged2CVar(fea)->getType()!=OFSVariable ||
          tagged2GenOFSVar(fea)->getWidth()>0) {
        oz_typeError(1,"Feature");
      } else {
        oz_suspendOnPtr(recPtr);
      }
    } else {
      oz_typeError(1,"Feature");
    }

  default:
    oz_typeError(0,"Record");
  }
} OZ_BI_end

TaggedRef getArityFromPairList(TaggedRef list)
{
  TaggedRef arity;
  TaggedRef *next=&arity;
  Bool updateFlag=NO;
  DerefReturnVar(list);
  TaggedRef old = list;
loop:
  if (oz_isLTuple(list)) {
    TaggedRef pair = oz_head(list);
    DerefReturnVar(pair);
    if (!oz_isPair2(pair)) return 0;

    TaggedRef fea = tagged2SRecord(pair)->getArg(0);
    DerefReturnVar(fea);

    if (!oz_isFeature(fea)) return 0;

    LTuple *lt=new LTuple();
    *next=makeTaggedLTuple(lt);
    lt->setHead(fea);
    next=lt->getRefTail();

    list = oz_tail(list);
    DerefReturnVar(list);
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
    case SVAR:
    case LITERAL:
      return SUSPEND;
    case SRECORD:
    case LTUPLE:
      if (recordFlag) {
        return SUSPEND;
      }
      goto typeError0;
    case CVAR:
      if (!oz_isFree(t0) && tagged2CVar(t0)->getType()!=OFSVariable)
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
    case SVAR:
      out=makeTaggedRef(t0Ptr);
      return SUSPEND;
    case CVAR:
      if (!oz_isFree(t0) && tagged2CVar(t0)->getType()!=OFSVariable)
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
  case SVAR:
    out=makeTaggedRef(t0Ptr);
    return SUSPEND;
  case CVAR:
    if (!oz_isFree(t0) && tagged2CVar(t0)->getType()!=OFSVariable)
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

  out = getArityList(term);
  if (out) return PROCEED;
  if (oz_isFree(term)) return SUSPEND;
  if (isCVar(tag)) {
    if (tagged2CVar(term)->getType()!=OFSVariable)
      oz_typeError(0,"Record");
    return SUSPEND;
  }
  oz_typeError(0,"Record");
}

OZ_DECLAREBI_USEINLINEFUN1(BIarity,BIarityInline)


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

  switch(tagA) {

  case OZFLOAT:
    out = oz_float(-floatValue(A));
    return PROCEED;

  case SMALLINT:
    out = newSmallInt(-smallIntValue(A));
    return PROCEED;

  case UVAR:
  case SVAR:
    return SUSPEND;

  case OZCONST:
    if (oz_isBigInt(A)) {
      out = tagged2BigInt(A)->neg();
      return PROCEED;
    }
    // fall through
  default:
    oz_typeError(0,"Int");
  }
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
  case PROCEED: out = NameTrue;  return PROCEED;
  case FAILED:  out = NameFalse; return PROCEED;
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
  case PROCEED: out = NameTrue;  return PROCEED;
  case FAILED:  out = NameFalse; return PROCEED;
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

OZ_BI_define(BInewPort,1,1)
{
  oz_declareIN(0,val);

  OZ_RETURN(oz_newPort(val));
} OZ_BI_end

OZ_Return sendPort(OZ_Term prt, OZ_Term val)
{
  Assert(oz_isPort(prt));

  Port *port  = tagged2Port(prt);

  CheckLocalBoard(port,"port");

  if(port->isProxy()) {
    return portSend(port,val);
  }
  LTuple *lt = new LTuple(val,am.currentUVarPrototype());

  OZ_Term old = ((PortWithStream*)port)->exchangeStream(lt->getTail());

  OZ_unifyInThread(old,makeTaggedLTuple(lt));
  return PROCEED;
}


OZ_BI_define(BIsendPort,2,0)
{
  oz_declareNonvarIN(0,prt);
  oz_declareIN(1,val);

  if (!oz_isPort(prt)) {
    oz_typeError(0,"Port");
  }

  return sendPort(prt,val);
} OZ_BI_end

// ---------------------------------------------------------------------
// Locks
// ---------------------------------------------------------------------

OZ_BI_define(BInewLock,0,1)
{

  OZ_RETURN(makeTaggedConst(new LockLocal(oz_currentBoard())));
} OZ_BI_end


OZ_BI_define(BIlockLock,1,0)
{
  oz_declareNonvarIN(0,lock);

  if (!oz_isLock(lock)) {
    oz_typeError(0,"Lock");
  }

  Tertiary *t=tagged2Tert(lock);
  if(t->isLocal()){
    LockLocal *ll=(LockLocal*)t;
    if (!oz_onToplevel()) {
      if (!oz_isCurrentBoard(GETBOARD(ll))) {
        return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("lock"));
      }}
    ll->lock(oz_currentThread());
    return PROCEED;}
  if(!oz_onToplevel()){
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("lock"));}

  switch(t->getTertType()){
  case Te_Manager:{
    ((LockManager *)t)->lock(oz_currentThread());
    return PROCEED;}
  case Te_Proxy:{
    ((LockProxy*)t)->lock(oz_currentThread());
    return PROCEED;}
  case Te_Frame:{
    ((LockFrame*)t)->lock(oz_currentThread());
    return PROCEED;}
  default:
    Assert(0);}

  Assert(0);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIunlockLock,1,0)
{
  oz_declareNonvarIN(0,lock);

  if (!oz_isLock(lock)) {
    oz_typeError(0,"Lock");
  }
  Tertiary *t=tagged2Tert(lock);
  switch(t->getTertType()){
  case Te_Local:{
    ((LockLocal*)t)->unlock();
    return PROCEED;}
  case Te_Manager:{
    ((LockManager*)t)->unlock(oz_currentThread());
    return PROCEED;}
  case Te_Proxy:{
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("lock"));}
  case Te_Frame:{
    ((LockFrame*)t)->unlock(oz_currentThread());
    return PROCEED;}
  }
  Assert(0);
  return PROCEED;
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
    return cellDoAccess(tert,out);
  }
  out = ((CellLocal*)tert)->getValue();
  return PROCEED;
}

OZ_Return exchangeCell(OZ_Term cell, OZ_Term newVal, OZ_Term &oldVal)
{
  Tertiary *tert = tagged2Tert(cell);
  if(tert->isLocal()){
    CellLocal *cellLocal=(CellLocal*)tert;
    CheckLocalBoard(cellLocal,"cell");
    oldVal = cellLocal->exchangeValue(newVal);
    return PROCEED;
  } else {
    if(!tert->isProxy()){
      CellSec* sec;
      if(tert->getTertType()==Te_Frame){
        sec=((CellFrame*)tert)->getSec();}
      else{
        sec=((CellManager*)tert)->getSec();}
      if(sec->getState()==Cell_Lock_Valid){
        TaggedRef old=sec->getContents();
        sec->setContents(newVal);
        oldVal = old;
        return PROCEED;}}
    oldVal = oz_newVariable();
    return cellDoExchange(tert,oldVal,newVal);
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

// mm2: should be function with switched args old-new!
OZ_BI_define(BIexchangeCell,3,0)
{
  oz_declareNonvarIN(0,cell);
  if (!oz_isCell(cell)) { oz_typeError(0,"Cell"); }
  oz_declareIN(1,oldVal);
  oz_declareIN(2,newVal);
  // SaveDeref(newVal);
  OZ_Term old;
  int ret = exchangeCell(cell,newVal,old);
  int ret2 = oz_unify(old,oldVal);
  // mm2: hack!
  if (ret != PROCEED) return ret;
  return ret2;
} OZ_BI_end

/********************************************************************
 * Arrays
 ******************************************************************** */

OZ_BI_define(BIarrayNew,3,1)
{
  oz_declareIntIN(0,ilow);
  oz_declareIntIN(1,ihigh);
  oz_declareIN(2,initValue);

  OZ_RETURN(makeTaggedConst(new OzArray(oz_currentBoard(),
                                        ilow,ihigh,initValue)));
} OZ_BI_end


OZ_Return isArrayInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term );
  out = oz_isArray(term) ? NameTrue : NameFalse;
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
    if (oz_isLiteral(car) && literalEq(car,NameUnit))
      return PROCEED;

    if (!oz_isSTuple(car))
      goto bomb;

    SRecord *tpl = tagged2SRecord(car);
    TaggedRef label = tpl->getLabel();

    if (literalEq(label,AtomUnify)) {
      Assert(OZ_width(car)==2);
      return oz_unify(tpl->getArg(0),tpl->getArg(1));
    }

    if (literalEq(label,AtomException)) {
      Assert(OZ_width(car)==1);
      return OZ_raise(tpl->getArg(0));
    }

    if (literalEq(label,AtomApply)) {
      Assert(OZ_width(car)==2);
      return applyProc(tpl->getArg(0),tpl->getArg(1));
    }

    if (literalEq(label,AtomApplyList)) {
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
  out = oz_isDictionary(term) ? NameTrue : NameFalse;
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
   Bit Arrays
   ----------------------------------------------------------------- */

#define oz_declareBitArrayIN(ARG,VAR)           \
BitArray *VAR;                                  \
{                                               \
  oz_declareNonvarIN(ARG,_VAR);                 \
  if (!oz_isBitArray(oz_deref(_VAR))) {         \
    oz_typeError(ARG,"BitArray");               \
  } else {                                      \
    VAR = tagged2BitArray(oz_deref(_VAR));      \
  }                                             \
}

OZ_BI_define(BIbitArray_new,2,1)
{
  oz_declareIntIN(0,l);
  oz_declareIntIN(1,h);
  if (l <= h)
    OZ_RETURN(makeTaggedConst(new BitArray(l, h)));
  else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.new",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_is,1,1)
{
  oz_declareNonvarIN(0,x);
  OZ_RETURN(oz_isBitArray(oz_deref(x))? OZ_true(): OZ_false());
} OZ_BI_end

OZ_BI_define(BIbitArray_set,2,0)
{
  oz_declareBitArrayIN(0,b);
  oz_declareIntIN(1,i);
  if (b->checkBounds(i)) {
    b->set(i);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_clear,2,0)
{
  oz_declareBitArrayIN(0,b);
  oz_declareIntIN(1,i);
  if (b->checkBounds(i)) {
    b->clear(i);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_test,2,1)
{
  oz_declareBitArrayIN(0,b);
  oz_declareIntIN(1,i);
  if (b->checkBounds(i))
    OZ_RETURN(b->test(i)? OZ_true(): OZ_false());
  else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_low,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN_INT(b->getLower());
} OZ_BI_end

OZ_BI_define(BIbitArray_high,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN_INT(b->getUpper());
} OZ_BI_end

OZ_BI_define(BIbitArray_clone,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN(makeTaggedConst(new BitArray(*b)));
} OZ_BI_end

OZ_BI_define(BIbitArray_or,2,0)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    b1->or(b2);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_and,2,0)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    b1->and(b2);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_nimpl,2,0)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    b1->nimpl(b2);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_disjoint,2,1)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    OZ_RETURN(b1->disjoint(b2) ? NameTrue : NameFalse);
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end


OZ_BI_define(BIbitArray_card,1,1)
{
  oz_declareBitArrayIN(0,b1);
  OZ_RETURN_INT(b1->card());
} OZ_BI_end



OZ_BI_define(BIbitArray_toList,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN(b->toList());
} OZ_BI_end

OZ_BI_define(BIbitArray_complementToList,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN(b->complementToList());
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

  SRecord * sr = tagged2SRecord(builtinRecord);

  for (int i = sr->getWidth(); i--; ) {
    Builtin * abit = tagged2Builtin(sr->getArg(i));

    sum += abit->getCounter();
    if (abit->getCounter()!=0) {
      printf("%010lu x %s\n",abit->getCounter(),abit->getPrintName());
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
  if (literalEq(oz_deref(onoff),NameTrue)) {
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
  OZ_RETURN(OZ_isForeignPointer(p)? NameTrue: NameFalse);
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
  OZ_declareForeignPointerIN(0,handle);
  OZ_RETURN_INT((long)handle);
} OZ_BI_end


/* ------------------------------------------------------------
 * Shutdown
 * ------------------------------------------------------------ */

OZ_BI_define(BIshutdown,1,0)
{
  OZ_declareIntIN(0,status);
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
  oz_declareIN(0,a);
  oz_declareIN(1,b);
  OZ_RETURN(oz_eq(a,b) ? NameTrue : NameFalse);
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

int oz_cv_getSuspListLength(GenCVariable *cv);

OZ_BI_define(BIconstraints,1,1)
{
  oz_declareDerefIN(0,in);

  int len = 0;
  if (isCVar(inTag)) {
    len=oz_cv_getSuspListLength(tagged2CVar(in));
  } else if (isSVar(inTag)) {
    len = tagged2SVar(in)->getSuspList()->length();
  }
  OZ_RETURN_INT(len);
} OZ_BI_end


/* ---------------------------------------------------------------------
 * System
 * --------------------------------------------------------------------- */

/* print and show are inline,
   because it prevents the compiler from generating different code
   */
OZ_Return printInline(TaggedRef term)
{
  oz_printStream(term,cout,ozconf.printDepth,ozconf.printWidth);
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEREL1(BIprint,printInline)

OZ_BI_define(BIprintInfo,1,0)
{
  oz_declareIN(0,t);
  OZ_printVirtualString(t);
  fflush(stdout);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIshowInfo,1,0)
{
  oz_declareIN(0,t);
  char * s = OZ_virtualStringToC(t);
  fprintf(stdout, "%s\n", s);
  fflush(stdout);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIprintError,1,0)
{
  oz_declareIN(0,t);
  prefixError(); // print popup code for opi
  char * s = OZ_virtualStringToC(t);
  fprintf(stderr, "%s", s);
  fflush(stderr);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIshowError,1,0)
{
  oz_declareIN(0,t);
  prefixError(); // print popup code for opi
  char * s = OZ_virtualStringToC(t);
  fprintf(stderr, "%s\n", s);
  fflush(stderr);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BItermToVS,3,1)
{
  oz_declareIN(0,t);
  oz_declareIntIN(1,depth);
  oz_declareIntIN(2,width);
  OZ_RETURN(OZ_string(OZ_toC(t,depth,width)));
} OZ_BI_end

OZ_Return showInline(TaggedRef term)
{
  printInline(term);
  printf("\n");
  return (PROCEED);
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
  case UVAR: case SVAR: case CVAR:
    OZ_RETURN_ATOM(VariableNamer::getName(OZ_in(0)));
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

  OZ_RETURN(OZ_onToplevel() ? NameTrue : NameFalse);
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
    CellSec* sec;
    if(t->getTertType()==Te_Frame) {
      sec=((CellFrame*)t)->getSec();
    } else {
      sec=((CellManager*)t)->getSec();
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
      EmCode = cellAssignExchange(t,fea,val);
    } else {
      if(newVar) val = oz_newVariable();
      EmCode = cellAtExchange(t,fea,val);
    }
  } else {
    if(!isAssign) val = oz_newVariable();
    EmCode = cellAtAccess(t,fea,val);
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
    if (oz_isCell(fea)) {
      OZ_Term out;
      int ret = accessCell(fea,out);
      OZ_result(out);
      return ret;
    }
    // mm2
    oz_typeError(0,"Feature|Cell");
  }

  RecOrCell state = am.getSelf()->getState();
  if (!stateIsCell(state)) {
    SRecord *rec = getRecord(state);
    Assert(rec!=NULL);

    TaggedRef t = rec->getFeature(fea);
    if (t) {
      OZ_RETURN(t);
    }
    return oz_raise(E_ERROR,E_OBJECT,"@",2,state,fea);
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
    if (oz_isCell(fea)) {
      OZ_Term oldIgnored;
      return exchangeCell(fea,value,oldIgnored);
    }
    // mm2
    oz_typeError(0,"Feature|Cell"); // mm2: new name: assignable
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
      return oz_raise(E_ERROR,E_OBJECT,"<-",3,state,fea,value);
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
    return oz_raise(E_ERROR,E_OBJECT,"<-",3,state,fea,value);
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
    if (oz_isCell(fea)) {
      OZ_Term oldVal;
      OZ_Return ret = exchangeCell(fea,newVal,oldVal);
      OZ_result(oldVal);
      return ret;
    }
    // mm2
    oz_typeError(1,"Feature|Cell");
  }

  RecOrCell state = am.getSelf()->getState();
  SRecord *rec;
  if (stateIsCell(state)) {
    rec = getRecordFromState(state);
    if (!rec) {
      // mm2: hey men
      return oz_raise(E_ERROR,E_SYSTEM,
                      "ooExchOnDistObjectNotImplemented",3,
                      state,fea,newVal);
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

  return oz_raise(E_ERROR,E_OBJECT,"ooExch",3,state,fea,newVal); // mm2: Error
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
                                    literalEq(locking,NameTrue),
                                    literalEq(native,NameTrue),
                                    oz_currentBoard());

  OZ_RETURN(makeTaggedConst(cl));
} OZ_BI_end


OZ_C_proc_begin(BIcomma,2)
{
  oz_declareNonvarArg(0,cl);
  cl = oz_deref(cl);

  if (!oz_isClass(cl)) {
    oz_typeError(0,"Class");
  }

  TaggedRef fb = tagged2ObjectClass(cl)->getFallbackApply();
  Assert(fb);

  am.prepareCall(fb,OZ_args[0],OZ_args[1]);
  am.emptySuspendVarList();
  return BI_REPLACEBICALL;
}
OZ_C_proc_end

OZ_C_proc_begin(BIsend,3)
{
  oz_declareNonvarArg(1,cl);
  oz_declareNonvarArg(2,obj);

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

  am.prepareCall(fb,OZ_args[1],OZ_args[0]);
  am.emptySuspendVarList();
  return BI_REPLACEBICALL;
}
OZ_C_proc_end

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
    if (cloneAll || literalEq(NameOoFreeFlag,oz_deref(arg))) {
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


OZ_C_proc_begin(BINew,3)
{
  oz_declareNonvarArg(0,cl);
  cl = oz_deref(cl);

  if (!oz_isClass(cl)) {
    oz_typeError(0,"Class");
  }

  ObjectClass * oc = tagged2ObjectClass(cl);

  TaggedRef fb = oc->getFallbackNew();

  Assert(fb);

  am.prepareCall(fb,OZ_args[0],OZ_args[1],OZ_args[2]);
  am.emptySuspendVarList();
  return BI_REPLACEBICALL;
}
OZ_C_proc_end


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
  warning("funReturn should never be called");
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIgetReturn,0,1)
{
  warning("getReturn should never be called");
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

  OZ_RETURN((ozconf.errorDebug &&
             oz_isSRecord(exc) &&
             (df = tagged2SRecord(exc)->getFeature(AtomDebug)) &&
             oz_isSRecord(oz_deref(df))) ? NameTrue : NameFalse);

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
  DEREF(x,xPtr,xTag);

  switch (xTag) {
  case UVAR:
  case SVAR:
  case CVAR:
    return 0;
    //  case SMALLINT:
    //  case FSETVALUE:
    //  case LITERAL:
    //  case OZFLOAT:
    //    return 1;
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
      case Co_Thread:
        b = ((Thread*)xp)->getBoardInternal(); break;
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
      case Co_HeapChunk:
        return 1;
      case Co_BitArray:
        return 1;
      case Co_Array:
        b = ((OzArray*)xp)->getBoardInternal(); break;
      case Co_Dictionary:
        b = ((OzDictionary*)xp)->getBoardInternal(); break;
      case Co_Lock:
        b = ((Tertiary*)xp)->getBoardInternal(); break;
      case Co_Class:
        b = ((ObjectClass*)xp)->getBoardInternal(); break;
      case Co_Unused2:
        Assert(0);
        return 1;
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

  am.setException(ret, literalEq(cat,E_ERROR) ? TRUE : ozconf.errorDebug);
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
