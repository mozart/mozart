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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __UNIFY_HH
#define __UNIFY_HH

#include "base.hh"
#include "am.hh"
#include "var_base.hh"

/* -----------------------------------------------------------------------
 * Var Locality Tests
 * -----------------------------------------------------------------------*/

#define ShallowCheckLocal(ptr)                                  \
   if (am.inShallowGuard())                                     \
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

/* -----------------------------------------------------------------------
 * Unification
 * -----------------------------------------------------------------------*/

OZ_Return oz_unify(OZ_Term t1, OZ_Term t2, ByteCode *scp=0);

// mm2: interface (see also oz_bind)
void doBindAndTrail(TaggedRef * vp, TaggedRef t);

#define DoBindAndTrailAndIP(vp,t,lv,gv) {       \
  lv->installPropagators(gv);                   \
  doBindAndTrail(vp,t);                         \
  }

void oz_bind(OZ_Term *varPtr, OZ_Term term);
void oz_bind_global(OZ_Term var, OZ_Term term);

/* -------------------------------------------------------------------------
 * Suspension lists
 * ------------------------------------------------------------------------- */

void oz_wakeupAll(OzVariable *sv);

SuspList * oz_checkAnySuspensionList(SuspList *suspList,Board *home,
                          PropCaller calledBy);

#define oz_checkSuspensionList(var,calledBy)                            \
  (var)->setSuspList(oz_checkAnySuspensionList((var)->getSuspList(),    \
                                               GETBOARD(var),calledBy))

#define oz_checkSuspensionListProp(var)         \
  oz_checkSuspensionList(var,pc_propagator)

#endif
