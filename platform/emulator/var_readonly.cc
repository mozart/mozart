/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Raphael Collet <raph@info.ucl.ac.be>
 *    Alfred Spiessens <fsp@info.ucl.ac.be>
 *
 *  Copyright:
 *    Denys Duchier (1998)
 *    Michael Mehl (1998)
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

// derived from var_future.cc by Raph & Fred

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_readonly.hh"
#endif

#include "var_readonly.hh"
#include "dpInterface.hh"
#include "builtins.hh"
#include "unify.hh"
#include "thr_int.hh"
#include "value.hh"
#include "atoms.hh"


OZ_Return ReadOnly::bind(TaggedRef *vPtr, TaggedRef t)
{
  if (oz_isLocalVar(this)) {
    return am.addSuspendVarListInline(vPtr);
  } else {
    oz_bindVar(this,vPtr, t);
    return PROCEED;
  }
}

OZ_Return ReadOnly::forceBind(TaggedRef *vPtr, TaggedRef t)
{
  if (*vPtr != oz_deref(t))
    oz_bindVar(this,vPtr,t);
  return PROCEED;
}

OZ_Return ReadOnly::unify(TaggedRef *vPtr, TaggedRef *tPtr)
{
  return bind(vPtr,makeTaggedRef(tPtr));
}

// this method forces the read-only to become needed; it kicks the
// suspension list and change the variable's type.
OZ_Return ReadOnly::becomeNeeded()
{
  // become a needed read-only variable, and waken the suspension list
  setType(OZ_VAR_READONLY);
  if (am.inEqEq()) {
    am.escapeEqEqMode();
    oz_forceWakeUp(getSuspListRef());
    am.restoreEqEqMode();
  } else {
    oz_forceWakeUp(getSuspListRef());
  }
  return PROCEED;
}


/*
 * Builtins
 */

// this built-in allows us to create a read-only variable
OZ_BI_define(BInewReadOnly,0,1)
{
  Board *bb = oz_currentBoard();
  TaggedRef r = oz_newReadOnly(bb);
  OZ_RETURN(r);
} OZ_BI_end

// this built-in binds a ReadOnly
OZ_BI_define(BIbindReadOnly,2,0)
{
  // BEWARE: the type of var is not checked.
  oz_declareDerefIN(0, var);
  Assert(oz_isVar(var) &&
         (tagged2Var(var)->getType() == OZ_VAR_READONLY ||
          tagged2Var(var)->getType() == OZ_VAR_READONLY_QUIET ||
          tagged2Var(var)->getType() == OZ_VAR_EXT));

  oz_declareIN(1, val);

  oz_bindReadOnly(varPtr,val);
  return PROCEED;
} OZ_BI_end



// this builtin/propagator is only internally available
// It binds a read-only variable to its final value, AND
// propagates the need from the read-only to the variable.
OZ_BI_define(BIvarToReadOnly,2,0)
{
  oz_declareDerefIN(0,v);
  oz_declareDerefIN(1,r);

  if (oz_isVarOrRef(v)) {
    if (oz_isFailed(v)) {
      // The failed value must be bound to the read-only.
      // (v cannot be used directly to bind the read-only)
      v = makeTaggedRef(vPtr);
      oz_bindReadOnly(rPtr,v);
      return PROCEED;
    }
    if (oz_isNeeded(r)) { // propagate need
      oz_var_makeNeeded(vPtr);
    } else { // r not needed yet: suspend again on r
      OZ_Return ret = oz_var_addQuietSusp(rPtr, oz_currentThread());
      Assert(ret == SUSPEND);
    }
    // suspend on v
    return oz_var_addQuietSusp(vPtr, oz_currentThread());
  }

  // bind the read-only to its value v
  oz_bindReadOnly(rPtr,v);
  return PROCEED;
} OZ_BI_end

// returns a read-only view of a variable
OZ_BI_define(BIreadOnly,1,1)
{
  oz_declareSafeDerefIN(0,v);

  if (oz_isRef(v)) {
    OZ_Term *vPtr = tagged2Ref(v);

    // special case: failed values immediately return
    if (oz_isFailed(*vPtr)) OZ_RETURN(v);

    // create the read-only variable in the same space as v
    OzVariable *ov = tagged2Var(*vPtr);
    Board *bb = GETBOARD(ov);
    TaggedRef r = oz_newReadOnly(bb);

    // create the propagator for data and need
    if (bb != oz_currentBoard()) {
      Thread *thr = oz_newThreadInject(bb);
      thr->pushCall(BI_varToReadOnly, RefsArray::make(v,r));

    } else { // optimization: immediately suspend thread
      Thread *thr = oz_newThreadSuspended();
      thr->pushCall(BI_varToReadOnly, RefsArray::make(v,r));

      // suspend on both v and r
      OZ_Return ret = oz_var_addQuietSusp(vPtr, thr);
      Assert(ret==SUSPEND);
      ret = oz_var_addQuietSusp(tagged2Ref(r), thr);
      Assert(ret==SUSPEND);
    }

    // return the read-only view
    OZ_RETURN(r);
  }

  // first argument is already a value
  OZ_RETURN(v);
} OZ_BI_end
