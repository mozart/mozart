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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

#include "var_base.hh"

#ifdef OUTLINE 
#define inline
#endif

//-----------------------------------------------------------------------------
//                           class OzBoolVariable
//-----------------------------------------------------------------------------

class OzBoolVariable : public OzVariable {
  
  friend class OzVariable;
  friend inline void addSuspBoolVar(TaggedRef, Suspension);
  
private:
  OZ_FiniteDomain * store_patch;

public:  
  OzBoolVariable(DummyClass *)
    : OzVariable(OZ_VAR_BOOL,(DummyClass*)0)
  {
  }
  OzBoolVariable(Board *bb) : OzVariable(OZ_VAR_BOOL,bb)
  {
    ozstat.fdvarsCreated.incf();
  }

  // mm2: ???
  OzBoolVariable(SuspList *sl) : OzVariable(OZ_VAR_BOOL,(DummyClass*)0)
  {
    suspList=sl;
  }

  USEFREELISTMEMORY;
  static void *operator new(size_t chunk_size, OzFDVariable *fdv) {
    return fdv;
  }
  // methods relevant for term copying (gc and solve)
  void gc(OzBoolVariable *);
  inline void dispose(void);
  
  // is X=val still valid, i.e. is val an smallint and either 0 or 1.
  Bool valid(TaggedRef val);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr, OZ_FiniteDomain & fd);
  void becomesSmallIntAndPropagate(TaggedRef * trPtr, int e);

  inline int getSuspListLength(void) { return suspList->length(); }

  void installPropagators(OzBoolVariable *);
  void installPropagators(OzFDVariable *);

  void propagate(PropCaller prop_eq = pc_propagator) {
    if (suspList) OzVariable::propagate(suspList, prop_eq);
  }
  void propagateUnify() { propagate(pc_cv_unif); }

  // needed to catch multiply occuring bool vars in propagators
  void patchStoreBool(OZ_FiniteDomain * d) { store_patch = d; }
  OZ_FiniteDomain * getStorePatchBool(void) { return store_patch; }

  OZ_Return unify(TaggedRef*, TaggedRef*, ByteCode*);
  OZ_Return bind(TaggedRef*, TaggedRef, ByteCode*);

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
inline OzBoolVariable * tagged2GenBoolVar(TaggedRef term);
inline void addSuspBoolVar(TaggedRef, Suspension);
OZ_Return tellBasicBoolConstraint(OZ_Term);

#ifndef OUTLINE 
#include "var_bool.icc"
#else
#undef inline
#endif

#endif
