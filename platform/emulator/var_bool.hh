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
  friend inline void addSuspBoolVar(TaggedRef, Thread *);

private:
  OZ_FiniteDomain * store_patch;

public:
  NO_DEFAULT_CONSTRUCTORS2(GenBoolVariable);
  GenBoolVariable(void) : GenCVariable(BoolVariable) {
    ozstat.fdvarsCreated.incf();
  }

  // methods relevant for term copying (gc and solve)
  void gc(GenBoolVariable *);
  void dispose(void);

  Bool unifyBool(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef,
                 ByteCode *, Bool = TRUE);

  // is X=val still valid, i.e. is val an smallint and either 0 or 1.
  Bool valid(TaggedRef val);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr, OZ_FiniteDomain & fd);
  void becomesSmallIntAndPropagate(TaggedRef * trPtr, int e);

  int getSuspListLength(void) { return suspList->length(); }

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
};

inline Bool isGenBoolVar(TaggedRef term);
inline Bool isGenBoolVar(TaggedRef term, TypeOfTerm tag);
inline GenBoolVariable * tagged2GenBoolVar(TaggedRef term);
inline void addSuspBoolVar(TaggedRef, Thread *);


#ifndef OUTLINE
#include "fdbvar.icc"
#else
#undef inline
#endif

#endif
