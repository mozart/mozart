/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Christian Schulte (schulte@dfki.de)
 *    Michael Mehl (mehl@dfki.de)
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

// I M P O R T A N T:
// This file defines the interface between the abstract machine
// and generic variables, which provide the basic functionality
// for concrete generic variables. The implementor of subclasses
// of GenCVariable is encouraged to include only this file and
// files related to the constraint system concerned.

#ifndef __GENVAR__H__
#define __GENVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "variable.hh"

#ifdef DEBUG_CHECK
#include "am.hh"
#endif

//#define DEBUG_TELLCONSTRAINTS

//-----------------------------------------------------------------------------
//                       Generic Constrained Variable
//-----------------------------------------------------------------------------


// NOTE:
//   this order is used in the case of CVAR=CVAR unification
//   e.g. SimpleVariable are bound prefered
// partial order required:
//  Simple<<everything
//  Bool<<FD
//  ???

enum TypeOfGenCVariable {
  OZ_VAR_SIMPLE,
  OZ_VAR_FUTURE,
  PerdioVariable,
  BoolVariable,
  FDVariable,
  OFSVariable,
  FSetVariable,
  CtVariable,
  NonGenCVariable,
  OZ_VAR_EXTENTED
};

#ifdef DEBUG_CHECK
#define OZ_VAR_INVALID ((TypeOfGenCVariable) -1)
#endif

class GenCVariable: public SVariable {

friend class GenFDVariable;
friend class GenFSetVariable;
friend class GenCtVariable;

private:
  union {
    TypeOfGenCVariable var_type;
    OZ_FiniteDomain   * patchDomain;
    OZ_FSetConstraint * patchFSet;
    OZ_Ct             * patchCt;
  } u;

  enum u_mask_t {u_fd = 0, u_bool = 1, u_fset = 2, u_ct = 3, u_mask = 3};

protected:

  void propagate(SuspList *& sl, PropCaller unifyVars) {
    sl=oz_checkAnySuspensionList(sl,GETBOARD(this), unifyVars);
  }

public:
  USEFREELISTMEMORY;

  GenCVariable(); // mm2: fake compiler

  TypeOfGenCVariable getType(void) { return u.var_type; }
  void setType(TypeOfGenCVariable t){
    u.var_type = t;
  }

  GenCVariable(TypeOfGenCVariable t, DummyClass *) { setType(t); };
  GenCVariable(TypeOfGenCVariable t, Board *bb) : SVariable(bb) { setType(t); }

  GenCVariable * gcG(void);
  void           gcRecurseG(void);

  void installPropagatorsG(GenCVariable *glob_var) {
    Assert(this->getType() == glob_var->getType() ||
           (this->getType() == BoolVariable &&
            glob_var->getType() == FDVariable));
    Assert(am.inShallowGuard() || am.isLocalSVar(this) &&
           ! am.isLocalSVar(glob_var));
    suspList = oz_installPropagators(suspList,
                                     glob_var->getSuspList(),
                                     GETBOARD(glob_var));
  }

  OZPRINTLONG;

  // needed to catch multiply occuring reified vars in propagators
  void patchReified(OZ_FiniteDomain * d, Bool isBool) {
    u.patchDomain = d;
    if (isBool) {
      u.patchDomain =
        (OZ_FiniteDomain*) ToPointer(ToInt32(u.patchDomain) | u_bool);
    }
    setReifiedFlag();
  }
  void unpatchReified(Bool isBool) {
    setType(isBool ? BoolVariable : FDVariable);
    resetReifiedFlag();
  }
  OZ_Boolean isBoolPatched(void) { return (u.var_type & u_mask) == u_bool; }
  OZ_Boolean isFDPatched(void) { return (u.var_type & u_mask) == u_fd; }
  OZ_Boolean isFSetPatched(void) { return (u.var_type & u_mask) == u_fset; }
  OZ_Boolean isCtPatched(void) { return (u.var_type & u_mask) == u_ct; }

  OZ_FiniteDomain * getReifiedPatch(void) {
    return (OZ_FiniteDomain *)  (u.var_type & ~u_mask);
  }
};

/* -------------------------------------------------------------------------
 * Kinded/Free
 * ------------------------------------------------------------------------- */

inline
Bool oz_cv_isKinded(GenCVariable *cv)
{
  switch (cv->getType()) {
  case FDVariable:
  case BoolVariable:
  case OFSVariable:
  case FSetVariable:
  case CtVariable:
    return true;
    // mm2: handle ExtVar???
  default:
    return false;
  }
}

// isKinded <=> !isFree
inline
int oz_isFree(TaggedRef r)
{
  return oz_isVariable(r) && (!isCVar(r) || !oz_cv_isKinded(tagged2CVar(r)));
}

Bool oz_cv_valid(GenCVariable *,TaggedRef *,TaggedRef);
OZ_Return oz_cv_unify(GenCVariable *,TaggedRef *,TaggedRef, ByteCode *);
OZ_Return oz_cv_bind(GenCVariable *,TaggedRef *,TaggedRef, ByteCode *);
void oz_cv_addSusp(GenCVariable *, TaggedRef *, Suspension, int = TRUE);
void oz_cv_printStream(ostream &, const char *, GenCVariable *, int);
int oz_cv_getSuspListLength(GenCVariable *);

/* -------------------------------------------------------------------------
 *
 * ------------------------------------------------------------------------- */

// only SVar and their descendants can be exclusive
inline
void setStoreFlag(OZ_Term t)
{
  tagged2SVarPlus(t)->setStoreFlag();
}

inline
void setReifiedFlag(OZ_Term t)
{
  tagged2SVarPlus(t)->setReifiedFlag();
}

inline
OZ_Boolean testReifiedFlag(OZ_Term t)
{
  return tagged2CVar(t)->testReifiedFlag();
}

inline
void patchReified(OZ_FiniteDomain * fd, OZ_Term t, Bool isBool)
{
  tagged2CVar(t)->patchReified(fd, isBool);
}

inline
OZ_Boolean testBoolPatched(OZ_Term t)
{
  return tagged2CVar(t)->isBoolPatched();
}

inline
OZ_Boolean testResetStoreFlag(OZ_Term t)
{
  return tagged2SVarPlus(t)->testResetStoreFlag();
}

inline
OZ_Boolean testStoreFlag(OZ_Term t)
{
  return tagged2SVarPlus(t)->testStoreFlag();
}

inline
OZ_Boolean testResetReifiedFlag(OZ_Term t)
{
  return tagged2SVarPlus(t)->testResetReifiedFlag();
}

inline
OZ_FiniteDomain * unpatchReifiedFD(OZ_Term t, Bool isBool)
{
  GenCVariable * v = tagged2CVar(t);

  v->unpatchReified(isBool);
  return v->getReifiedPatch();
}

#endif
