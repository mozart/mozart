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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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

#include "genvar.hh"

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
  GenBoolVariable(Board *bb) : GenCVariable(BoolVariable,bb)
  {
    ozstat.fdvarsCreated.incf();
  }

  // mm2: ???
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
  
  // is X=val still valid, i.e. is val an smallint and either 0 or 1.
  Bool valid(TaggedRef val);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr, OZ_FiniteDomain & fd);
  void becomesSmallIntAndPropagate(TaggedRef * trPtr, int e);

  inline int getSuspListLength(void) { return suspList->length(); }

  void installPropagators(GenBoolVariable *);
  void installPropagators(GenFDVariable *);

  void propagate(PropCaller prop_eq = pc_propagator) {
    if (suspList) GenCVariable::propagate(suspList, prop_eq);
  }
  void propagateUnify() { propagate(pc_cv_unif); }

  // needed to catch multiply occuring bool vars in propagators
  void patchStoreBool(OZ_FiniteDomain * d) { store_patch = d; }
  OZ_FiniteDomain * getStorePatchBool(void) { return store_patch; }

  OZ_Return unify(TaggedRef *, TaggedRef, ByteCode *);

  void printStream(ostream &out,int depth = 10) {
    out << "{0#1}";
  }
  void printLongStream(ostream &out,int depth = 10,
			int offset = 0) {
    printStream(out,depth); out << endl;
  }
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
