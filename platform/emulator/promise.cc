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

void Future::request()
{
  if (oz_isRef(requested)) {
    oz_bind_global(requested,oz_unit());
  }
  requested=oz_true();
}

OZ_Return Future::waitRequest(TaggedRef *v)
{
  if (oz_eq(requested,oz_false())) {
    requested=oz_newVar(GETBOARD(this));
    // fall through
  }

  if (oz_isRef(requested)) {
    am.addSuspendVarList(requested);
    return SUSPEND;
  } else {
    return PROCEED;
  }
}   
    
OZ_Return Future::unifyFuture(TaggedRef* vPtr)
{
  request();

  am.addSuspendVarList(makeTaggedRef(vPtr));
  return SUSPEND;
}

void Future::addSuspFuture(TaggedRef *vPtr, Thread*th,int unstable)
{
  request();

  addSuspSVar(th,unstable);
}

OZ_BI_define(BIPromiseNew,0,1)
{
  TaggedRef p = makeTaggedRef(newTaggedCVar(new Future()));
  OZ_RETURN(makeTaggedPromise(new Promise(p)));
} OZ_BI_end

OZ_Return promiseAssign(OZ_Term p, OZ_Term val)
{
  TaggedRef var = tagged2Promise(p)->getFuture();
  DEREF(var,varPtr,_);
  if (isCVar(var)) {
    GenCVariable *cvar=tagged2CVar(var);
    if (cvar->getType()==FUTURE) {
      Future *l=(Future *)cvar;
      CheckLocalBoard(l,"promise");
      if (oz_deref(val)==var) { // mm2
	return oz_raise(E_ERROR,E_KERNEL,"promiseAssignToItself",
			1,makeTaggedRef(varPtr));
      }
      oz_bind_global(makeTaggedRef(varPtr),val);
      return PROCEED;
    }
  }
  return oz_raise(E_ERROR,E_KERNEL,"promiseAssignTwice",
		  1,makeTaggedRef(varPtr));
}

OZ_BI_define(BIPromiseAssign,2,0)
{
  oz_declareNonvarIN(0,p);
  oz_declareIN(1,val);
  if (!oz_isPromise(p)) {
    oz_typeError(0,"Promise");
  }
  return promiseAssign(p,val);
} OZ_BI_end

OZ_BI_define(BIPromiseAccess,1,1)
{
  oz_declareNonvarIN(0,p);
  if (!oz_isPromise(p)) {
    oz_typeError(0,"Promise");
  }
  OZ_RETURN(tagged2Promise(p)->getFuture());
} OZ_BI_end

OZ_BI_define(BIPromiseWaitRequest,1,0)
{
  oz_declareNonvarIN(0,p);
  if (!oz_isPromise(p)) {
    oz_typeError(0,"Promise");
  }

  TaggedRef var = tagged2Promise(p)->getFuture();
  DEREF(var,varPtr,_);
  if (!isCVar(var)) return PROCEED;
  GenCVariable *cvar=tagged2CVar(var);
  if (cvar->getType() != FUTURE) return PROCEED;
  Future *l=(Future *)cvar;

  CheckLocalBoard(l,"promise");

  return l->waitRequest(varPtr);
} OZ_BI_end

OZ_BI_define(BIPromiseIs,1,1)
{
  oz_declareNonvarIN(0,p);
  OZ_RETURN(oz_isPromise(p)?oz_true():oz_false());
} OZ_BI_end
