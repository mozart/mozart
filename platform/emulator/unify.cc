/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (scheidhr@ps.uni-sb.de)
 *
 *  Contributors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#ifdef INTERFACE
#pragma implementation
#endif

#include "unify.hh"
#include "var_all.hh"
#include "space.hh"
#include "thr_int.hh"

// imports
Bool oz_wakeup_Propagator(Propagator * prop, Board * home, PropCaller calledBy);

/* -------------------------------------------------------------------------
 * Suspension lists
 * ------------------------------------------------------------------------- */

inline
static
Bool wakeup_Thread(Thread * tt, Board *home, PropCaller calledBy)
{
  Assert (tt->isSuspended());

  switch (oz_isBetween(tt->getBoardInternal(), home)) {
  case B_BETWEEN:
    oz_wakeupThread(tt);
    return TRUE;

  case B_NOT_BETWEEN:
    if (calledBy==pc_all) {
      oz_wakeupThread(tt);
      return TRUE;
    }
    return FALSE;

  case B_DEAD:
    //
    //  The whole thread is eliminated - because of the invariant
    // stated just before 'disposeThread ()' in thread.hh;
    tt->markDeadThread();
    CheckExtSuspension(tt);
    am.threadsPool.freeThreadBody(tt);
    return TRUE;

  default:
    Assert(0);
    return FALSE;
  }
}

//
//  Generic 'wakeUp';
//  Since this method is used at the only one place, it's inlined;
inline
static
Bool wakeup_Suspension(Suspension susp, Board * home, PropCaller calledBy)
{
  if (susp.isThread()) {
    Thread * tt = susp.getThread();

    return wakeup_Thread(tt,home,calledBy);

  } else {
    Assert(susp.isPropagator());

    return oz_wakeup_Propagator(susp.getPropagator(), home, calledBy);
  }
}

SuspList * oz_checkAnySuspensionList(SuspList *suspList,Board *home,
                                     PropCaller calledBy)
{
  if (am.inEqEq())
    return suspList;

  SuspList * retSuspList = NULL;

  while (suspList) {
    Suspension susp = suspList->getSuspension();

    if (susp.isDead()) {
      suspList = suspList->dispose();
      continue;
    }

 // already runnable susps remain in suspList
    if (susp.isRunnable()) {
      if (susp.isPropagator()) {
        Propagator * prop = susp.getPropagator();

        if (calledBy && !prop->isUnifyPropagator()) {
          switch (oz_isBetween(prop->getBoardInternal(), home)) {
          case B_BETWEEN:
            prop->markUnifyPropagator();
            break;
          case B_DEAD:
            //  keep the thread itself alive - it will be discarded
            // *properly* in the emulator;
            suspList = suspList->dispose ();
            continue;
          case B_NOT_BETWEEN:
            break;
          }
        }
      } else {
        //  non-propagator, i.e. it just goes away;
        suspList = suspList->dispose();
        continue;
      }
    } else {
      if (wakeup_Suspension(susp, home, calledBy)) {
        Assert (susp.isDead() || susp.isRunnable());
        suspList = suspList->dispose ();
        continue;
      }
    }

    // susp cannot be woken up therefore relink it
    SuspList * first = suspList;
    suspList = suspList->getNext();
    first->setNext(retSuspList);
    retSuspList = first;
  } // while

  return retSuspList;
}

/* -------------------------------------------------------------------------
 * Binding
 * -------------------------------------------------------------------------*/

#ifdef DEBUG_CHECK
static
Board *varHome(TaggedRef val) {
  if (isUVar(val)) {
    return tagged2VarHome(val);
  } else {
    return GETBOARD(tagged2SVarPlus(val));
  }
}
static
Bool checkHome(TaggedRef *vPtr) {
  TaggedRef val = oz_deref(*vPtr);

  return !oz_isVariable(val) ||
    oz_isBelow(oz_currentBoard(),varHome(val));
}
#endif

/*
 * oz_bindUVar: bind an opt. variable to a value
 *   - trail global vars
 *   - redirect REF
 */
inline
void oz_bindUVar(TaggedRef *varPtr, TaggedRef term)
{
  Assert(isUVar(*varPtr));
  if (!oz_isLocalUVar(varPtr)) {
    Assert(am.inEqEq() || checkHome(varPtr));
    am.trail.pushRef(varPtr,*varPtr);
  }
  doBind(varPtr,term);
}

/*
 * oz_bindVar: bind a variable to a value
 *   - trail global vars
 *   - wakeup suspensions
 *   - relink suspensions of local vars
 *   - dispose local vars
 *   - redirect REF
 */
void oz_bindGlobalVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term)
{
  Assert(tagged2CVar(*varPtr)==ov);
  Assert(!oz_isLocalVar(ov));
  Assert(am.inEqEq() || checkHome(varPtr));
  oz_checkSuspensionList(ov, pc_std_unif);
  am.trail.pushRef(varPtr,*varPtr);
  doBind(varPtr,term);
}

void oz_bindLocalVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term)
{
  Assert(tagged2CVar(*varPtr)==ov);
  Assert(oz_isLocalVar(ov));
  Assert(!am.inEqEq());
  oz_checkSuspensionList(ov, pc_std_unif);
  DEREF(term,termPtr,_);
  if (isCVar(term)) {
    OzVariable *sv=tagged2CVar(term);
    Assert(sv!=ov);
    ov->relinkSuspListTo(sv);
    term=makeTaggedRef(termPtr);
  } else {
    // mm2: problems with fsp_monitorIn, monitorArity,...
    // Assert(ov->isEmptySuspList());
  }
  oz_var_dispose(ov);
  doBind(varPtr,term);
}

void oz_bind_global(TaggedRef var, TaggedRef term)
{
  DEREF(var,varPtr,_);
  if (isCVar(var)) {
    OzVariable *ov=tagged2CVar(var);
    oz_checkSuspensionList(ov, pc_all);
    DEREF(term,termPtr,_);
    if (isCVar(term)) {
      OzVariable *sv=tagged2CVar(term);
      Assert(sv!=ov);
      ov->relinkSuspListTo(sv);
      term=makeTaggedRef(termPtr);
    } else {
      Assert(ov->isEmptySuspList());
    }
    oz_var_dispose(ov);
  } else {
    Assert(isUVar(var));
  }
  doBind(varPtr,term);
}

/* -------------------------------------------------------------------------
 * Unification
 * -------------------------------------------------------------------------*/

inline
Board *getVarBoard(TaggedRef var)
{
  CHECK_ISVAR(var);

  if (isUVar(var))
    return tagged2VarHome(var);
  return tagged2SVarPlus(var)->getHome1();
}

inline
Bool isMoreLocal(TaggedRef var1, TaggedRef var2)
{
  Board *board1 = getVarBoard(var1);
  Board *board2 = getVarBoard(var2)->derefBoard();
  return oz_isBelow(board1,board2);
}

inline
int cmpCVar(OzVariable *v1, OzVariable *v2)
{
  TypeOfVariable t1 = v1->getType();
  TypeOfVariable t2 = v2->getType();
  return t1-t2;
}

const StackEntry mark=(StackEntry)-1;
class UnifyStack : public Stack {
public:
  UnifyStack() : Stack(100,Stack_WithMalloc) {}
  void pushMark() { push(mark); }
  Bool isMark() { return topElem()==mark; }
};

// global vars!!!
static UnifyStack unifyStack;
static Stack rebindTrail(100,Stack_WithMalloc);

inline
void rebind(TaggedRef *refPtr, TaggedRef term2)
{
  rebindTrail.ensureFree(2);
  rebindTrail.push(refPtr,NO);
  rebindTrail.push(ToPointer(*refPtr),NO);
  doBind(refPtr,term2);
}

#define PopRebindTrail(value,refPtr)                    \
    TaggedRef value   = ToInt32(rebindTrail.pop());     \
    TaggedRef *refPtr = (TaggedRef*) rebindTrail.pop();


OZ_Return oz_unify(TaggedRef t1, TaggedRef t2)
{
  unifyStack.pushMark();
  CHECK_NONVAR(t1); CHECK_NONVAR(t2);

  OZ_Return result = FAILED;

  TaggedRef term1 = t1;
  TaggedRef term2 = t2;
  TaggedRef *termPtr1 = &term1;
  TaggedRef *termPtr2 = &term2;

loop:
  int argSize;

  _DEREF(term1,termPtr1,tag1);
  _DEREF(term2,termPtr2,tag2);

  // identical terms ?
  if (isUVar(term1) ? termPtr1 == termPtr2 : term1 == term2) {
    goto next;
  }

  if (oz_isVariable(term1)) {
    if (oz_isVariable(term2)) {
      goto var_var;
    } else {
      goto var_nonvar;
    }
  } else {
    if (oz_isVariable(term2)) {
      Swap(term1,term2,TaggedRef);
      Swap(termPtr1,termPtr2,TaggedRef*);
      Swap(tag1,tag2,TypeOfTerm);
      goto var_nonvar;
    } else {
      goto nonvar_nonvar;
    }
  }


 /*************/
 var_nonvar:

  if (isCVar(tag1)) {
    int res = oz_var_bindINLINE(tagged2CVar(term1),termPtr1, term2);
    if (res == PROCEED)
      goto next;
    result = res;
    goto fail;
  }

  Assert(isUVar(tag1));
  oz_bindUVar(termPtr1, term2);
  goto next;



 /*************/
 var_var:

  /*
   * The implemented partial order for binding variables to variables is:
   *   local -> global
   *   UVAR -> CVAR (prefer binding nonCVars to CVars)
   *   local newer -> local older
   */
  if (isUVar(tag1)) {
    if (isUVar(tag2) &&
        isMoreLocal(term2,term1) &&
        (!oz_isLocalUVar(termPtr1) ||
         heapNewer(termPtr2,termPtr1))) {
      oz_bindUVar(termPtr2, makeTaggedRef(termPtr1));
    } else {
      oz_bindUVar(termPtr1, makeTaggedRef(termPtr2));
    }
    goto next;
  }

  if (isUVar(tag2)) {
    oz_bindUVar(termPtr2, makeTaggedRef(termPtr1));
    goto next;
  }

  Assert(isCVar(tag1) && isCVar(tag2));
  /* prefered binding of perdio vars */
  if (cmpCVar(tagged2CVar(term1),tagged2CVar(term2))>0) {
    Swap(term1,term2,TaggedRef);
    Swap(termPtr1,termPtr2,TaggedRef*);
  }

  {
    int res = oz_var_unifyINLINE(tagged2CVar(term1),termPtr1, termPtr2);
    if (res == PROCEED)
      goto next;
    result = res;
    goto fail;
  }


 /*************/
 nonvar_nonvar:

  COUNT(nonvarNonvarUnify);

  if (tag1 != tag2)
    goto fail;

  switch ( tag1 ) {

  case FSETVALUE:
    if (((FSetValue *) tagged2FSetValue(term1))->unify(term2))
      goto next;
    goto fail;

  case LTUPLE:
    {
      COUNT(recRecUnify);
      LTuple *lt1 = tagged2LTuple(term1);
      LTuple *lt2 = tagged2LTuple(term2);

      rebind(termPtr2,term1);
      argSize = 2;
      termPtr1 = lt1->getRef();
      termPtr2 = lt2->getRef();
      goto push;
    }

  case SRECORD:
    {
      COUNT(recRecUnify);
      SRecord *sr1 = tagged2SRecord(term1);
      SRecord *sr2 = tagged2SRecord(term2);

      if (! sr1->compareFunctor(sr2))
        goto fail;

      rebind(termPtr2,term1);
      argSize  = sr1->getWidth();
      termPtr1 = sr1->getRef();
      termPtr2 = sr2->getRef();
      goto push;
    }

  case OZFLOAT:
    if (floatEq(term1,term2))
      goto next;
    else
      goto fail;

  case SMALLINT:
    if (smallIntEq(term1,term2))
      goto next;
    goto fail;

  case EXT:
    {
      int res = oz_tagged2Extension(term1)->eqV(term2);
      if (res == PROCEED)
        goto next;
      result = res;
      goto fail;
    }
  case OZCONST:
    switch (tagged2Const(term1)->getType()) {
    case Co_BigInt:
      if (bigIntEq(term1,term2))
        goto next;
      break;
    default:
      break;
    }
    goto fail;

  case LITERAL:
    /* literals and constants unify if their pointers are equal */
  default:
    goto fail;
  }


 /*************/

next:
  if (unifyStack.isMark()) {
    result = PROCEED;
    goto exit;
  }

  termPtr2 = (TaggedRef*) unifyStack.pop();
  termPtr1 = (TaggedRef*) unifyStack.pop();
  argSize  = ToInt32(unifyStack.pop());
  // fall through

push:
  if (argSize>1) {
    unifyStack.ensureFree(3);
    unifyStack.push(ToPointer(argSize-1),NO);
    unifyStack.push(termPtr1+1,NO);
    unifyStack.push(termPtr2+1,NO);
  }
  term1=tagged2NonVariable(termPtr1);
  term2=tagged2NonVariable(termPtr2);
  goto loop;

fail:
  Assert(result!=PROCEED);
  while (!unifyStack.isMark()) {
    unifyStack.pop();
  }
  // fall through

exit:
  Assert(unifyStack.isMark());
  unifyStack.pop(); // pop mark

  while (!rebindTrail.isEmpty ()) {
    PopRebindTrail(value,refPtr);
    doBind(refPtr,value);
  }

  return result;
}
