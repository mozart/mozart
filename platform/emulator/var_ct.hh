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

#ifndef __CTGENVAR__H__
#define __CTGENVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"

class OzCtVariable : public OzVariable {

  friend class OzVariable;
  friend void constrainGlobalVar(OZ_Term *, OZ_Ct *);
  friend OZ_Return tellBasicConstraint(OZ_Term, OZ_Ct *, OZ_CtDefinition *);
  friend class OZ_CtVar;
  friend void addSuspCtVar(OZ_Term , Suspendable * , OZ_CtWakeUp);

private:
  // --------------------
  // private data members

  // if this pointer is `NULL', the variable is constrained to the
  // least constraint of this kind

  OZ_Ct * _constraint;
  OZ_CtDefinition * _definition;
  SuspList ** _susp_lists;

  // ------------------------
  // private member functions

  void propagate(OZ_CtWakeUp, PropCaller);
  void propagateUnify() {
    propagate(OZ_WAKEUP_ALL, pc_cv_unif);
  }

  void copyConstraint(OZ_Ct * c) {
    _constraint = c->copy();
  }

  void relinkSuspListTo(OzCtVariable * lv, Bool reset_local = FALSE);

public:
  USEFREELISTMEMORY;

  void installPropagators(OzCtVariable *);

  int getNoOfSuspLists(void) {
    return _definition->getNoEvents();
  }

  SuspList * getSuspList(int i) {
    return (0 <= i && i < getNoOfSuspLists()
	    ?  _susp_lists[i]
	    : (SuspList *) NULL);
  }

  int getSuspListLength(void) {
    int len = suspList->length();
    for (int i = getNoOfSuspLists(); i--; )
      len += _susp_lists[i]->length();
    return len;
  }

  OzCtVariable(OZ_Ct * c, OZ_CtDefinition * d,Board *bb)
    : _definition(d), OzVariable(OZ_VAR_CT,bb)
  {
    Assert(c);
    Assert(d);

    copyConstraint(c);

    int noOfSuspLists = getNoOfSuspLists();

    _susp_lists = (SuspList **)
      oz_freeListMalloc(sizeof(SuspList *) * noOfSuspLists);

    for (int i = noOfSuspLists; i--; )
      _susp_lists[i] = (SuspList *) NULL;
  }

  OZ_Ct * getConstraint(void) {
    return _constraint;
  }

  OZ_CtDefinition * getDefinition(void) {
    return _definition;
  }

  OZ_Return bind(OZ_Term * vPtr, OZ_Term t);
  OZ_Return unify(OZ_Term *, OZ_Term *);

  Bool valid(OZ_Term val) {
    return _constraint->isInDomain(val);
  }

  void gCollect(Board *);
  void gCollectRecurse(void);

  void sClone(Board *);
  void sCloneRecurse(void);

  // methods for trailing
  OzVariable * copyForTrail(void);
  void restoreFromCopy(OzCtVariable *);


  void dispose(void) {
    // dispose suspension lists
    oz_freeListDisposeUnsafe(_susp_lists, getNoOfSuspLists() * sizeof(SuspList *));

    // dispose constraint
    delete _constraint;
  }

  void printStream(ostream &out, int depth = 10) {
    out << _definition->getName() << ':' << _constraint->toString(depth);
  }

  void printLongStream(ostream &out,
		       int depth = 10,
		       int offset = 0) {
    out << this << ' ';
    printStream(out, depth);
    // getSuspListLength() is intended as consistency check
    out << getSuspListLength() << endl;
  }
  //
  void dropPropagator(Propagator * prop) {
    for (int i = getNoOfSuspLists(); i--; ) {
      _susp_lists[i]= _susp_lists[i]->dropPropagator(prop);
    }
    suspList = suspList->dropPropagator(prop);
  }
  //
  // tagging and untagging constrained variables
  //
  OZ_CtVar * getTag(void) {
    return (OZ_CtVar *)  (u.var_type & ~u_mask);
  }
  //
  // end of tagging ...
  //
};


OZ_Return tellBasicConstraint(OZ_Term, OZ_Ct *, OZ_CtDefinition *);


#ifndef OUTLINE
#include "var_ct.icc"
#else
#undef inline

OzCtVariable * tagged2GenCtVar(OZ_Term);
Bool isGenCtVar(OZ_Term term);
void addSuspCtVar(OZ_Term, Suspendable *, OZ_CtWakeUp);

#endif

#endif /*  __CTGENVAR__H__ */

// eof
//-----------------------------------------------------------------------------
