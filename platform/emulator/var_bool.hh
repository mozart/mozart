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
  friend inline void addSuspBoolVar(TaggedRef, Suspendable *);

public:
  OzBoolVariable(Board *bb) : OzVariable(OZ_VAR_BOOL,bb)
  {
    ozstat.fdvarsCreated.incf();
  }

  // this one is used by becomesBool()
  OzBoolVariable(Board *bb, SuspList *sl) : OzVariable(OZ_VAR_BOOL, bb) {
    suspList = sl;
  }

  USEFREELISTMEMORY;
  static void *operator new(size_t chunk_size, OzFDVariable *fdv) {
    return fdv;
  }

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

  OZ_Return unify(TaggedRef*, TaggedRef*);
  OZ_Return bind(TaggedRef*, TaggedRef);

  void printStream(ostream &out,int depth = 10) {
    out << "{0#1}";
  }
  void printLongStream(ostream &out,int depth = 10,
                        int offset = 0) {
    printStream(out,depth); out << endl;
  }
  //
  void dropPropagator(Propagator * prop) {
    suspList = suspList->dropPropagator(prop);
  }
  //
  // tagging and untagging constrained variables
  //
  OZ_FDIntVar * getTag(void) {
    return (OZ_FDIntVar *)  (u.var_type & ~u_mask);
  }
  //
  // end of tagging ...
  //
};

inline Bool isGenBoolVar(TaggedRef term);
inline OzBoolVariable * tagged2GenBoolVar(TaggedRef term);
inline void addSuspBoolVar(TaggedRef, Suspendable *);
OZ_Return tellBasicBoolConstraint(OZ_Term);

#ifndef OUTLINE
#include "var_bool.icc"
#else
#undef inline
#endif

#endif
