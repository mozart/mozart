/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    derived from var_future.cc by Raphael Collet <raph@info.ucl.ac.be>
 *    and Alfred Spiessens <fsp@info.ucl.ac.be>
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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_readonly.hh"
#endif

#include "var_readonly.hh"
#include "var_failed.hh"
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
  // release the current suspension list
  oz_checkSuspensionList(this, pc_all);
  // mutate into a needed read-only variable
  setType(OZ_VAR_READONLY);
  // disposeS();
  return PROCEED;
}

// use this method for adding demanding suspensions only!
OZ_Return ReadOnly::addSusp(TaggedRef *tPtr, Suspendable * susp) {
  // release the current suspension list and mutate into a needed read-only
  becomeNeeded();
  // add susp into the ReadOnly's suspension list
  addSuspSVar(susp);
  return SUSPEND;
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
// it is used by the definition of '!!' only
OZ_BI_define(BIbindReadOnly,2,0)
{
  // BEWARE: the type of var is not checked.
  oz_declareDerefIN(0, var);
  Assert(oz_isVar(var) &&
	 (tagged2Var(var)->getType() == OZ_VAR_READONLY ||
	  tagged2Var(var)->getType() == OZ_VAR_READONLY_QUIET));

  // oz_declareNonvarIN(1, val);
  OZ_Term val = OZ_in(1);

  oz_bindReadOnly(varPtr,val);
  return PROCEED;
} OZ_BI_end



// this builtin/propagator is only internally available
// It binds a read-only variable to its final value.
OZ_BI_define(BIvarToReadOnly,2,0)
{
  oz_declareDerefIN(0,v);

  if (oz_isVarOrRef(v) && !oz_isFailed(v))
    return oz_var_addQuietSusp(vPtr, oz_currentThread());

  oz_declareDerefIN(1,r);
  oz_bindReadOnly(rPtr,v);

  return PROCEED;
} OZ_BI_end

// this builtin/propagator is only internally available
// It makes a variable needed when it read-only view becomes needed.
OZ_BI_define(BIreadOnlyToVar,2,0)
{
  oz_declareDerefIN(0,r);

  if (!oz_isNeeded(r))
    return oz_var_addQuietSusp(rPtr, oz_currentThread());

  oz_declareDerefIN(1,v);
  if (!oz_isNeeded(v)) oz_var_need(vPtr);

  return PROCEED;
} OZ_BI_end

// returns a read-only view of a variable
OZ_BI_define(BIreadOnly,1,1)
{
  oz_declareSafeDerefIN(0,v);
  // TaggedRef v = OZ_in(0);
  // v = oz_safeDeref(v);

  if (oz_isRef(v)) {
    // create the read-only variable in the same space as v
    OZ_Term *vPtr = tagged2Ref(v);
    OzVariable *ov = tagged2Var(*vPtr);
    Board *bb = GETBOARD(ov);
    TaggedRef r = oz_newReadOnly(bb);

    // create the propagators for data and need
    if (bb != oz_currentBoard()) {
      Thread *datathr = oz_newThreadInject(bb);
      datathr->pushCall(BI_varToReadOnly, RefsArray::make(v,r));

      Thread *needthr = oz_newThreadInject(bb);
      needthr->pushCall(BI_readOnlyToVar, RefsArray::make(r,v));

    } else { // optimization: immediately suspend threads
      Thread *datathr = oz_newThreadSuspended();
      datathr->pushCall(BI_varToReadOnly, RefsArray::make(v,r));
      OZ_Return ret = oz_var_addQuietSusp(vPtr, datathr);
      Assert(ret==SUSPEND);

      Thread *needthr = oz_newThreadSuspended();
      needthr->pushCall(BI_readOnlyToVar, RefsArray::make(r,v));
      ret = oz_var_addQuietSusp(tagged2Ref(r), needthr);
      Assert(ret==SUSPEND);
    }

    // return the read-only view
    OZ_RETURN(r);
  }

  // first argument is already a value
  OZ_RETURN(v);
} OZ_BI_end
