/*
 *  Authors:
 *    Fred Spiessens (fsp@info.ucl.ac.be)
 *    Raphael Collet (raph@info.ucl.ac.be)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Fred Spiessens and Raphael Collet (2003)
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

// This class is an adaptation of SimpleVar.

// A QuietVar object models a variable with suspensions that do not
// demand the value of it.  Typical suspensions are by-need suspensions
// and WaitQuiet.

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_quiet.hh"
#endif

#include "var_quiet.hh"
#include "var_readonly_quiet.hh"
#include "var_simple.hh"
#include "builtins.hh"
#include "unify.hh"

OZ_Return QuietVar::bind(TaggedRef* vPtr, TaggedRef t)
{
  oz_bindVar(this,vPtr, t);
  return PROCEED;
}

OZ_Return QuietVar::unify(TaggedRef* vPtr, TaggedRef *tPtr)
{
  OzVariable *tv=tagged2Var(*tPtr);
  if (tv->getType() == OZ_VAR_QUIET
      && oz_isBelow(tv->getBoardInternal(), GETBOARD(this))
#ifdef VAR_BIND_NEWER
      // if both are local, then check heap
      && (!oz_isLocalVar(this) || heapNewer(tPtr,vPtr))
#endif
      ) {

    oz_bindVar(tv,tPtr, makeTaggedRef(vPtr));
  } else {
    oz_bindVar(this,vPtr, makeTaggedRef(tPtr));
  }
  return PROCEED;
}

// this method forces the variable to become needed; it kicks the
// suspension list and turns the variable into a SimpleVar.
OZ_Return QuietVar::becomeNeeded()
{
  // step 1: release the current suspension list
  oz_checkSuspensionList(this, pc_all);
  // step 2: mutate into a SimpleVar
  setType(OZ_VAR_SIMPLE);
  // disposeS();
  return PROCEED;
}

// use this method for adding demanding suspensions only!
OZ_Return QuietVar::addSusp(TaggedRef *tPtr, Suspendable * susp) {
  // release the current suspension list and mutate into a SimpleVar
  becomeNeeded();
  // add susp into the SimpleVar's suspension list
  addSuspSVar(susp);
  return SUSPEND;
}

OzVariable *oz_newQuietVar(Board *bb)
{
  return new QuietVar(bb);
}

// this built-in allows us to create a quiet variable (was for testing)
/*
OZ_BI_define(BInewQuiet,0,1)
{
    Board *bb = oz_currentBoard();
    TaggedRef q = oz_newQuiet(bb);
    OZ_RETURN(q);
} OZ_BI_end
*/

// this built-in turns an OptVar into a QuietVar,
// suspends the current thread on a QuietVar,
// and it continues otherwise (SimpleVar, or more constrained vars).
OZ_BI_define(BIwaitNeeded,1,0)
{
  oz_declareDerefIN(0,v);
  Assert(!oz_isRef(v));

  if (oz_isVar(v)) {
    OzVariable *ov = tagged2Var(v);
    switch (ov->getType()) {
    case OZ_VAR_OPT:
    case OZ_VAR_QUIET:
    case OZ_VAR_READONLY_QUIET:
      return oz_var_addQuietSusp(vPtr, oz_currentThread());
    default:
      return (PROCEED);
    }
  }
  // the former if statement was:
  //   if (oz_isVar(v) && tagged2Var(v)->getType() >= OZ_VAR_QUIET) {
  //     return oz_var_addQuietSusp(vPtr, oz_currentThread());
  //   }

  return PROCEED;
} OZ_BI_end

// this built-in turns an OptVar or QuietVar into a SimpleVar, a
// QuietReadOnly into a ReadOnly, then continues.
OZ_BI_define(BIneed,1,0)
{
  oz_declareDerefIN(0,v);
  Assert(!oz_isRef(v));

  if (oz_isVar(v)) {
    OzVariable *ov = tagged2Var(v);
    switch (ov->getType()) {
    case OZ_VAR_OPT:
      ov = new SimpleVar(ov->getBoardInternal());
      *vPtr = makeTaggedVar(ov);
      return (PROCEED);
    case OZ_VAR_QUIET:
      ((QuietVar*) ov)->becomeNeeded();
      return (PROCEED);
    case OZ_VAR_READONLY_QUIET:
      ((QuietReadOnly*) ov)->becomeNeeded();
      return (PROCEED);
    default:
      return (PROCEED);
    }
  }
    // if (oz_isVar(v) && tagged2Var(v)->getType() >= OZ_VAR_QUIET) {
        // return oz_var_addQuietSusp(vPtr, oz_currentThread());
//            QuietVar* q = (QuietVar*) tagged2Var(v);
//            q->addSuspSVar(oz_currentThread());
//            return (SUSPEND);
    // }
  return PROCEED;
} OZ_BI_end

// this built-in returns true if the argument is needed
OZ_BI_define(BIisNeeded,1,1)
{
  oz_declareDerefIN(0,var);
  Assert(!oz_isRef(var));

  if (oz_isVarOrRef(var)) {
    switch (tagged2Var(var)->getType()) {
    case OZ_VAR_OPT:
    case OZ_VAR_QUIET:
    case OZ_VAR_READONLY_QUIET:
      OZ_RETURN(oz_false());
    default:
      break;
    }
  }
  OZ_RETURN(oz_true());
} OZ_BI_end
