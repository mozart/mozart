/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Michael Mehl (1998)
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "promise.hh"
#endif

#include "am.hh"
#include "genvar.hh"
#include "runtime.hh"

void Promise::request()
{
  if (isRef(requested)) {
    oz_bind_global(requested,oz_unit());
  }
  requested=oz_true();
}

OZ_Return Promise::waitRequest(TaggedRef *v)
{
  if (oz_eq(requested,oz_false())) {
    requested=oz_newVar(GETBOARD(this));
    // fall through
  }

  if (isRef(requested)) {
    am.addSuspendVarList(requested);
    return SUSPEND;
  } else {
    return PROCEED;
  }
}

OZ_Return Promise::unifyPromise(TaggedRef* vPtr)
{
  request();

  am.addSuspendVarList(makeTaggedRef(vPtr));
  return SUSPEND;
}

void Promise::addSuspPromise(TaggedRef *vPtr, Thread*th,int unstable)
{
  request();

  addSuspSVar(th,unstable);
}

OZ_BI_define(BIPromiseNew,0,1)
{
  Promise *pr = new Promise();
  OZ_Term var=makeTaggedRef(newTaggedCVar(pr));
  OZ_RETURN(oz_newPromise());
} OZ_BI_end

OZ_Return promiseAssign(OZ_Term var, OZ_Term val)
{
  DEREF(var,varPtr,varTag);
  if (isCVar(var)) {
    GenCVariable *cvar=tagged2CVar(var);
    if (cvar->getType()==PROMISE) {
      Promise *l=(Promise *)cvar;
      CheckLocalBoard(l,"promise");
      if (deref(val)==var) {
        return oz_raise(E_ERROR,E_KERNEL,"promiseAssignToItself",
                        1,makeTaggedRef(varPtr));
      }
      oz_bind_global(makeTaggedRef(varPtr),val);
      l->dispose();
      return PROCEED;
    }
  }
  oz_typeError(1,"Promise");
}

OZ_BI_define(BIPromiseAssign,2,0)
{
  OZ_Term var = OZ_in(0);
  OZ_Term val = OZ_in(1);
  return promiseAssign(var,val);
} OZ_BI_end

OZ_BI_define(BIPromiseWaitRequest,1,0)
{
  OZ_Term var = OZ_in(0);
  DEREF(var,varPtr,varTag);
  if (!isCVar(var)) return PROCEED;
  GenCVariable *cvar=tagged2CVar(var);
  if (cvar->getType() != PROMISE) return PROCEED;
  Promise *l=(Promise *)cvar;

  CheckLocalBoard(l,"promise");

  return l->waitRequest(varPtr);
} OZ_BI_end

OZ_BI_define(BIPromiseIs,1,1)
{
  OZ_declareIN(0,var);
  OZ_RETURN(isPromise(deref(var))?oz_true():oz_false());
} OZ_BI_end
