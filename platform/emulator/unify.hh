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

#ifndef __UNIFY_HH
#define __UNIFY_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "am.hh"
#include "var_base.hh"
#include "board.hh"
#include "trail.hh"

/* -----------------------------------------------------------------------
 * Locality test for variables
 * -----------------------------------------------------------------------*/

//
// kost@ : faking non-locality for '==' forces trailing, which is then
// used for restoring changes back;
inline
Bool oz_isLocalVar(OzVariable *var) {
  if (am.inEqEq())
    return FALSE;

  Board * bb = var->getBoardInternal();

  Board * c  = oz_currentBoard();

  if (bb == c)
    return OK;

  while (bb->isCommitted()) {
    bb = bb->getParentInternal();

    if (bb == c)
      return OK;
  }

  return NO;
}

#include "var_ext.hh"
inline
Bool oz_isLocalVar(ExtVar*v) {
  return oz_isLocalVar(extVar2Var(v));
}

inline
Bool oz_isLocalVariable(TaggedRef var)
{
  return (oz_isOptVar(var) ? !am.inEqEq() : oz_isLocalVar(tagged2Var(var)));
}

/* -------------------------------------------------------------------------
 * Suspension lists
 * ------------------------------------------------------------------------- */

void oz_checkAnySuspensionList(SuspList ** suspList, Board *home,
                               PropCaller calledBy);

void oz_checkLocalSuspensionList(SuspList ** suspList,
                                 PropCaller calledBy);

#define oz_checkSuspensionList(var, calledBy) \
  oz_checkAnySuspensionList((var)->getSuspListRef(),   \
                            (var)->getBoardInternal(), \
                            calledBy)

#define oz_checkSuspensionListProp(var)         \
  oz_checkSuspensionList(var,pc_propagator)

/* -----------------------------------------------------------------------
 * Binding
 * -----------------------------------------------------------------------*/

inline
void doBind(TaggedRef *p, TaggedRef t)
{
  Assert(!oz_isVar(t));
#ifdef DEBUG_CHECK
  {
    TaggedRef tt = t;
    DEREF(tt,ttPtr);
    Assert(p != ttPtr);
  }
#endif
  *p = t;
}

// mm2: interface for constraints
#define DoBind(vp,t) \
  doBind(vp,t)

#define DoBindAndTrail(vp,t) {                  \
  trail.pushBind(vp);                           \
  DoBind(vp,t);                                 \
}

#define DoBindAndTrailAndIP(vp,t,lv,gv) {       \
  lv->installPropagators(gv);                   \
  DoBindAndTrail(vp,t);                         \
}

#define DoBindAndTrailAndP(vp,t,lv) {           \
  lv->propagate();                              \
  DoBindAndTrail(vp,t);                         \
}

void oz_bindLocalVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term);
void oz_bindGlobalVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term);

inline
void oz_bindVar(OzVariable *ov, TaggedRef *varPtr, TaggedRef term)
{
  Assert(tagged2Var(*varPtr)==ov);
  if (oz_isLocalVar(ov)) {
    oz_bindLocalVar(ov,varPtr,term);
  } else {
    oz_bindGlobalVar(ov,varPtr,term);
  }
}
void oz_bind_global(TaggedRef var, TaggedRef term);

#endif
