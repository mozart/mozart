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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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
#include "pointer-marks.hh"

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
  SimpleVarType,
  ComplexVarType,
  LazyVariable,
  PerdioVariable,
  BoolVariable,
  FDVariable,
  OFSVariable,
  FSetVariable,
  CtVariable,
  NonGenCVariable
};

#define GenVarCheckType(t) Assert(t >= SimpleVarType && t < NonGenCVariable)

class GenCVariable: public SVariable {

friend class GenFDVariable;
friend class GenFSetVariable;
friend class GenCtVariable;

private:
  union {
    TypeOfGenCVariable var_type;
    OZ_FiniteDomain   * patchDomain;
    OZ_FSetConstraint * patchFSet;
    OZ_GenConstraint  * patchCt;
  } u;

  enum u_mask_t {u_fd = 0, u_bool = 1, u_fset = 2, u_ct = 3, u_mask = 3};
   
protected:
  
  void propagate(SuspList * &, PropCaller);

public:
  USEFREELISTMEMORY;

  GenCVariable(); // mm2: fake compiler

  TypeOfGenCVariable getType(void){ return u.var_type; }
  void setType(TypeOfGenCVariable t){
    GenVarCheckType(t);
    u.var_type = t;
  }  

  // mm2: default for board is not nice
  // the constructor creates per default a local variable (wrt curr. node)
  GenCVariable(TypeOfGenCVariable t, DummyClass *) { setType(t); };
  GenCVariable(TypeOfGenCVariable, Board * = (Board*)0);

  virtual GenCVariable* gcV() = 0;
  virtual void          gcRecurseV() = 0;
  virtual OZ_Return     unifyV(TaggedRef *, TaggedRef, ByteCode *) = 0;

  virtual int           isKindedV() = 0;
  virtual OZ_Return     validV(TaggedRef *, TaggedRef) = 0;
  virtual OZ_Return     hasFeatureV(TaggedRef, TaggedRef *) = 0;

  virtual void          addSuspV(Suspension susp, TaggedRef *vPtr,
				 int unstable = TRUE) = 0;

  virtual void          disposeV() = 0;
  virtual void          printStreamV(ostream &out,int depth = 10) = 0;
  virtual void          printLongStreamV(ostream &out,int depth = 10,
					 int offset = 0) = 0;
  virtual int           getSuspListLengthV() = 0;

  void printV(void) { printStreamV(cerr); cerr << endl; cerr.flush(); }
  void printLongV(void) { printLongStreamV(cerr); cerr.flush(); }

  // methods relevant for term copying (gc and solve)  
  GenCVariable * gc(void);
  void gcRecurse(void);

  // unifies a generic variable with another generic variable
  // or a non-variable
  // invariant: left term == *this
  OZ_Return unify(TaggedRef *, TaggedRef, ByteCode *);

  inline int getSuspListLength(void);

  // is X=val still valid
  Bool valid(TaggedRef *varPtr, TaggedRef val);
  int hasFeature(TaggedRef fea,TaggedRef *out);

  OZPRINTLONG;

  void installPropagators(GenCVariable *);

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
  OZ_Boolean isCtPatched(void) { return (u.var_type & u_mask) == u_ct; }

  OZ_FiniteDomain * getReifiedPatch(void) { 
    return (OZ_FiniteDomain *)  (u.var_type & ~u_mask); 
  }
  Bool isKinded() {
    switch (getType()) {
    case FDVariable:
    case BoolVariable:
    case OFSVariable:
      return true;
    default:
      return isKindedV();
    }
  }
};

// isKinded = !isFree
inline
int oz_isFree(TaggedRef r)
{
  return oz_isVariable(r) && (!isCVar(r) || !tagged2CVar(r)->isKinded());
}
  
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

void addSuspCVarOutline(TaggedRef * v, Suspension susp, int unstable = TRUE);

#include "fsgenvar.hh"
#include "fdgenvar.hh"
#include "fdbvar.hh"
#include "ofgenvar.hh"
#include "ctgenvar.hh"
#include "perdiovar.hh"
#include "promise.hh"

#ifdef OUTLINE
void addSuspCVar(TaggedRef * v, Suspension susp, int unstable = TRUE);
#else
#include "genvar.icc"
#endif


#endif
