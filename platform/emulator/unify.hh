/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (scheidhr@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Kostja Popow (popow@ps.uni-sb.de)
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

#ifndef __UNIFY_HH
#define __UNIFY_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "am.hh"
#include "var_base.hh"

/* -----------------------------------------------------------------------
 * Locality test for variables
 * -----------------------------------------------------------------------*/

#define ShallowCheckLocal(ptr)					\
   if (am.inShallowGuard())					\
      return reallyHeapNever(ptr,am.getShallowHeapTop());

Bool _isLocalUVar(TaggedRef *varPtr);

inline
Bool oz_isLocalUVar(TaggedRef *varPtr)
{
  // variables are usually bound 
  // in the node where they are created
  ShallowCheckLocal(varPtr);
  if (am.currentUVarPrototypeEq(*varPtr)) return OK;
  return _isLocalUVar(varPtr);
}


Bool _isLocalVar(OzVariable *var);

inline
Bool oz_isLocalVar(OzVariable *var) {
  ShallowCheckLocal(var);
  return (oz_isCurrentBoard(var->getHome1())) || _isLocalVar(var);
}

#undef ShallowCheckLocal

inline
Bool oz_isLocalVariable(TaggedRef *varPtr)
{
  CHECK_ISVAR(*varPtr);
  return isUVar(*varPtr)
    ? oz_isLocalUVar(varPtr)
    : oz_isLocalVar(tagged2CVar(*varPtr));
}

/* -------------------------------------------------------------------------
 * Suspension lists
 * ------------------------------------------------------------------------- */

SuspList * oz_checkAnySuspensionList(SuspList *suspList,Board *home,
			  PropCaller calledBy);

#define oz_checkSuspensionList(var,calledBy)				\
  (var)->setSuspList(oz_checkAnySuspensionList((var)->unlinkSuspList(),	\
					       GETBOARD(var),calledBy))

#define oz_checkSuspensionListProp(var)		\
  oz_checkSuspensionList(var,pc_propagator)

/* -----------------------------------------------------------------------
 * Binding
 * -----------------------------------------------------------------------*/

inline
void doBind(TaggedRef *p, TaggedRef t)
{
  ProfileCode(if (oz_isVariable(oz_deref(t))) 
	      {COUNT(varVarUnify);} else {COUNT(varNonvarUnify)});
  CHECK_NONVAR(t);
  Assert(p!=_derefPtr(t));
  *p = t;
}

// mm2: interface for constraints
#define DoBind(vp,t) \
  doBind(vp,t)

#define DoBindAndTrail(vp,t) {			\
  am.trail.pushRef(vp, *vp);			\
  DoBind(vp,t);					\
}

#define DoBindAndTrailAndIP(vp,t,lv,gv) {	\
  lv->installPropagators(gv);			\
  DoBindAndTrail(vp,t);				\
}

#define DoBindAndTrailAndP(vp,t,lv) {		\
  lv->propagate();				\
  DoBindAndTrail(vp,t);				\
}

void oz_bindLocalVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term);
void oz_bindGlobalVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term);

inline
void oz_bindVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term)
{
  Assert(tagged2CVar(*varPtr)==ov);
  if (oz_isLocalVar(ov)) {
    oz_bindLocalVar(ov,varPtr,term);
  } else {
    oz_bindGlobalVar(ov,varPtr,term);
  }
}
void oz_bind_global(TaggedRef var, TaggedRef term);

#endif
