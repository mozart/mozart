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
  friend OZ_Return tellBasicConstraint(OZ_Term, 
				       OZ_Ct *, 
				       OZ_CtDefinition *);
  friend OZ_Boolean OZ_CtVar::tell(void);

  friend void addSuspCtVar(OZ_Term , Suspension , OZ_CtWakeUp);

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
    return _definition->getNoOfWakeUpLists();
  }

  SuspList * getSuspList(int i) {
    return (0 <= i && i < getNoOfSuspLists() 
	    ?  _susp_lists[i] 
	    : (SuspList *) NULL);
  }

  OzCtVariable(OZ_Ct * c, OZ_CtDefinition * d,Board *bb) 
    : _definition(d), OzVariable(OZ_VAR_CT,bb)
  {
    Assert(c);
    Assert(d);

    copyConstraint(c); 
    
    int noOfSuspLists = getNoOfSuspLists();

    _susp_lists = (SuspList **) 
      freeListMalloc(sizeof(SuspList *) * noOfSuspLists);

    for (int i = noOfSuspLists; i--; )
      _susp_lists[i] = (SuspList *) NULL;
  }

  OZ_Ct * getConstraint(void) {
    return _constraint;
  }

  OZ_CtDefinition * getDefinition(void) {
    return _definition;
  }

  OZ_Ct * getReifiedPatch(void) { 
    return (OZ_Ct *) (u.var_type & ~u_mask); 
  }

  void patchReified(OZ_Ct * s) { 
    u.patchCt = (OZ_Ct *) ToPointer(ToInt32(s) | u_ct); 
    setReifiedFlag();
  }

  void unpatchReified(void) { 
    setType(OZ_VAR_CT); 
    resetReifiedFlag();
  }

  OZ_Return bind(OZ_Term * vPtr, OZ_Term t, ByteCode * scp);
  OZ_Return unify(OZ_Term *, OZ_Term *, ByteCode * scp);

  OZ_Return valid(OZ_Term val) {    
    return _constraint->unify(val);
  }

  OzVariable * gc(void);

  void gcRecurse(void);
  
  void dispose(void) {
    // dispose suspension lists
    freeListDispose(_susp_lists, getNoOfSuspLists() * sizeof(SuspList *));

    // dispose constraint
    delete _constraint;        
  }

  void printStream(ostream &out, int depth = 10) {
    out << _definition->getName() << ':' << _constraint->toString(depth);
  }

  void printLongStream(ostream &out,
		       int depth = 10,
		       int offset = 0) {
    printStream(out,depth); out << endl;
  }
};


OZ_Return tellBasicConstraint(OZ_Term, OZ_Ct *, OZ_CtDefinition *);


#ifndef OUTLINE
#include "var_ct.icc"
#else
#undef inline

OzCtVariable * tagged2GenCtVar(OZ_Term);
Bool isGenCtVar(OZ_Term term, TypeOfTerm tag);
OZ_Ct * unpatchReifiedCt(OZ_Term t);
void addSuspCtVar(OZ_Term, Suspension, OZ_CtWakeUp);

#endif
 
#endif /*  __CTGENVAR__H__ */

// eof
//-----------------------------------------------------------------------------
