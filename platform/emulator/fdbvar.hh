/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#ifndef __GENBOOLVAR__H__
#define __GENBOOLVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "genvar.hh"
#include "fdomn.hh"
#include "fdhook.hh"

#ifdef OUTLINE 
#define inline
#endif

//-----------------------------------------------------------------------------
//                           class GenBoolVariable
//-----------------------------------------------------------------------------

class GenBoolVariable : public GenCVariable {
  
  friend class GenCVariable;
  friend inline void addSuspBoolVar(TaggedRef, Suspension);
  
private:
  OZ_FiniteDomain * store_patch;

public:  
  GenBoolVariable(DummyClass *)
    : GenCVariable(BoolVariable,(DummyClass*)0)
  {
  }
  GenBoolVariable(void) : GenCVariable(BoolVariable)
  {
    ozstat.fdvarsCreated.incf();
  }
  GenBoolVariable(SuspList *sl) : GenCVariable(BoolVariable,(DummyClass*)0)
  {
    suspList=sl;
  }

  USEFREELISTMEMORY;
  static void *operator new(size_t chunk_size, GenFDVariable *fdv) {
    return fdv;
  }
  // methods relevant for term copying (gc and solve)
  void gc(GenBoolVariable *);
  inline void dispose(void);
  
  Bool unifyBool(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef,
		 ByteCode *, Bool = TRUE);

  // is X=val still valid, i.e. is val an smallint and either 0 or 1.
  Bool valid(TaggedRef val);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr, OZ_FiniteDomain & fd);
  void becomesSmallIntAndPropagate(TaggedRef * trPtr, int e);

  inline int getSuspListLength(void) { return suspList->length(); }

  void installPropagators(GenFDVariable *, Board *);

  void relinkSuspListTo(GenBoolVariable * lv, Bool reset_local = FALSE) {
    GenCVariable::relinkSuspListTo(lv, reset_local);
  }

  void relinkSuspListTo(GenFDVariable * lv, Bool reset_local = FALSE) {
    GenCVariable::relinkSuspListTo(lv, reset_local);
  }

  void propagate(TaggedRef var, PropCaller prop_eq = pc_propagator) {
    if (suspList) GenCVariable::propagate(var, suspList, prop_eq);
  }
  void propagateUnify(TaggedRef var) { propagate(var, pc_cv_unif); }

  // needed to catch multiply occuring bool vars in propagators
  void patchStoreBool(OZ_FiniteDomain * d) { store_patch = d; }
  OZ_FiniteDomain * getStorePatchBool(void) { return store_patch; }


  OZ_Return unifyV(TaggedRef* vPtr,TaggedRef v,TaggedRef *tPtr,TaggedRef t,
		   ByteCode*scp) {
    return unifyBool(vPtr,v,tPtr,t,scp);
  }
  OZ_Return validV(TaggedRef* /* vPtr */, TaggedRef val ) {
    return valid(val);
  }
  OZ_Return hasFeatureV(TaggedRef val, TaggedRef *) { return FAILED; }
  GenCVariable* gcV() { error("not impl"); return 0; }
  void gcRecurseV() { error("not impl"); }
  void addSuspV(Suspension susp, TaggedRef* ptr, int state) {
    error("not impl");
    // mm2: addSuspBoolVar(makeTaggedRef(ptr),susp,state);
  }
  Bool isKindedV() { return true; }
  void disposeV(void) { dispose(); }
  int getSuspListLengthV() { return getSuspListLength(); }
  void printV() {}
  void printLongV() {}
};

inline Bool isGenBoolVar(TaggedRef term);
inline Bool isGenBoolVar(TaggedRef term, TypeOfTerm tag);
inline GenBoolVariable * tagged2GenBoolVar(TaggedRef term);
inline void addSuspBoolVar(TaggedRef, Suspension);


#ifndef OUTLINE 
#include "fdbvar.icc"
#else
#undef inline
#endif

#endif
