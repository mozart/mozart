/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
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

//#define DEBUG_TELLCONSTRAINTS

//-----------------------------------------------------------------------------
//                       Generic Constrained Variable
//-----------------------------------------------------------------------------


enum TypeOfGenCVariable {
  FDVariable,
  OFSVariable,
  MetaVariable,
  BoolVariable,
  FSetVariable,
  AVAR,
  PerdioVariable,
  LazyVariable,
  NonGenCVariable
};

#define GenVarCheckType(t)                              \
    Assert(t == FDVariable || t == OFSVariable ||       \
           t == MetaVariable || t == BoolVariable ||    \
           t==AVAR || t==PerdioVariable || t == FSetVariable || \
           t ==LazyVariable )

class GenCVariable: public SVariable {

friend class GenFDVariable;
friend class GenFSetVariable;

private:
  union {
    TypeOfGenCVariable var_type;
    OZ_FiniteDomain *patchDomain;
    OZ_FSetConstraint *patchFSet;
  } u;

  enum u_mask_t {u_fd=0, u_bool=1, u_fset=2, u_ri=3, u_mask=3};

protected:

  void propagate(TaggedRef, SuspList * &, PropCaller);

  // takes the suspensionlist of var and  appends it to the
  // suspensionlist of leftVar
  void relinkSuspListTo(GenCVariable * lv, Bool reset_local = FALSE) {
    suspList = suspList->appendToAndUnlink(lv->suspList, reset_local);
  }

public:
  USEFREELISTMEMORY;

  // the constructor creates per default a local variable (wrt curr. node)
  GenCVariable(TypeOfGenCVariable, Board * = NULL);

  TypeOfGenCVariable getType(void){ return u.var_type; }
  void setType(TypeOfGenCVariable t){
    GenVarCheckType(t);
    u.var_type = t;
  }

  // methods relevant for term copying (gc and solve)
  void gc(void);
  size_t getSize(void);

  // unifies a generic variable with another generic variable
  // or a non-variable
  // invariant: left term == *this
  Bool unify(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, ByteCode *);
  Bool unifyOutline(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, ByteCode *);

  int getSuspListLength(void);

  // is X=val still valid
  Bool valid(TaggedRef *varPtr, TaggedRef val);
  int hasFeature(TaggedRef fea,TaggedRef *out);

  void print(ostream &stream, int depth, int offset, TaggedRef v);
  void printLong(ostream &stream, int depth, int offset, TaggedRef v);

  void installPropagators(GenCVariable *);

  void addDetSusp (Thread *thr, TaggedRef *tptr);

  void dispose(void);

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
  OZ_Boolean isRIPatched(void) { return (u.var_type & u_mask) == u_ri; }
  OZ_FiniteDomain * getReifiedPatch(void) {
    return (OZ_FiniteDomain *)  (u.var_type & ~u_mask);
  }
};


// only SVar and their descendants can be exclusive
inline
void setStoreFlag(OZ_Term t)
{
  Assert(!isUVar(t) && isAnyVar(t) && !isRef(t));

  ((SVariable *) tagValueOf(t))->setStoreFlag();
}

inline
void setReifiedFlag(OZ_Term t)
{
  Assert(!isUVar(t) && isAnyVar(t) && !isRef(t));

  ((SVariable *) tagValueOf(t))->setReifiedFlag();
}

inline
OZ_Boolean testReifiedFlag(OZ_Term t)
{
  Assert(!isUVar(t) && isAnyVar(t) && !isRef(t));

  return ((SVariable *) tagValueOf(t))->testReifiedFlag();
}

inline
void patchReified(OZ_FiniteDomain * fd, OZ_Term t, Bool isBool)
{
  ((GenCVariable *) tagValueOf(t))->patchReified(fd, isBool);
}

inline
OZ_Boolean testBoolPatched(OZ_Term t)
{
  Assert(!isUVar(t) && isAnyVar(t) && !isRef(t));

  return ((GenCVariable *) tagValueOf(t))->isBoolPatched();
}

inline
OZ_Boolean testResetStoreFlag(OZ_Term t)
{
  Assert(!isUVar(t) && isAnyVar(t) && !isRef(t));

  return ((SVariable *) tagValueOf(t))->testResetStoreFlag();
}

inline
OZ_Boolean testResetReifiedFlag(OZ_Term t)
{
  Assert(!isUVar(t) && isAnyVar(t) && !isRef(t));

  return ((SVariable *) tagValueOf(t))->testResetReifiedFlag();
}

inline
OZ_FiniteDomain * unpatchReified(OZ_Term t, Bool isBool)
{
  Assert(!isUVar(t) && isAnyVar(t) && !isRef(t));
  GenCVariable * v = ((GenCVariable *) tagValueOf(t));

  v->unpatchReified(isBool);
  return v->getReifiedPatch();
}


#include "fsgenvar.hh"
#include "fdgenvar.hh"
#include "fdbvar.hh"
#include "ofgenvar.hh"
#include "metavar.hh"
#include "avar.hh"
#include "perdiovar.hh"
#include "lazyvar.hh"

#ifdef OUTLINE
void addSuspCVar(TaggedRef v, Thread *el);
#else
#include "genvar.icc"
#endif


#endif
