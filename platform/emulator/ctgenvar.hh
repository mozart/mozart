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

#ifndef __CTGENVAR__H__
#define __CTGENVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "genvar.hh"

class GenCtVariable : public GenCVariable {

  friend class GenCVariable;
  friend OZ_Return tellBasicConstraint(OZ_Term, 
				       OZ_GenConstraint *, 
				       OZ_GenDefinition *);
  friend OZ_Boolean OZ_GenCtVar::tell(void);

  friend void addSuspCtVar(OZ_Term , Suspension , OZ_GenWakeUpDescriptor);

private:
  // --------------------
  // private data members

  // if this pointer is `NULL', the variable is constrained to the 
  // least constraint of this kind

  OZ_GenConstraint * _constraint;
  OZ_GenDefinition * _definition;
  SuspList ** _susp_lists;

  // ------------------------
  // private member functions

  void propagate(OZ_GenWakeUpDescriptor, PropCaller);
  void propagateUnify() { 
    propagate(OZ_WAKEUP_ALL, pc_cv_unif); 
  }

  void copyConstraint(OZ_GenConstraint * c) { 
    _constraint = c->copy();
  }

  void relinkSuspListTo(GenCtVariable * lv, Bool reset_local = FALSE);

  void installPropagators(GenCtVariable *, Board *);

public:
  USEFREELISTMEMORY;

  int getNoOfSuspLists(void) {
    return _definition->getNoOfWakeUpLists();
  }

  GenCtVariable(OZ_GenConstraint * c, OZ_GenDefinition * d) 
    : _definition(d), GenCVariable(CtVariable)
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

  OZ_GenConstraint * getConstraint(void) {
    return _constraint;
  }

  OZ_GenDefinition * getDefinition(void) {
    return _definition;
  }

  OZ_GenConstraint * getReifiedPatch(void) { 
    return (OZ_GenConstraint *) (u.var_type & ~u_mask); 
  }

  void patchReified(OZ_GenConstraint * s) { 
    u.patchCt = (OZ_GenConstraint *) ToPointer(ToInt32(s) | u_ct); 
    setReifiedFlag();
  }

  void unpatchReified(void) { 
    setType(CtVariable); 
    resetReifiedFlag();
  }

  OZ_Return unifyV(OZ_Term * vPtr, OZ_Term t, ByteCode * scp);

  OZ_Return validV(OZ_Term * /* vPtr */, OZ_Term val) {    
    return _constraint->unify(val);
  }

  OZ_Return hasFeatureV(OZ_Term val, OZ_Term *) { 
    return SUSPEND; 
  }

  virtual GenCVariable * gcV(void);

  virtual void gcRecurseV(void);
  
  virtual void addSuspV(Suspension susp, OZ_Term * ptr, int state) 
  {
    // covered by `addSuspCVar(OZ_Term, Suspension, int)' [genvar.icc]
  }

  virtual Bool isKindedV(void) { 
    return true; 
  }

  virtual void disposeV(void) {
    // dispose suspension lists
    freeListDispose(_susp_lists, getNoOfSuspLists() * sizeof(SuspList *));

    // dispose constraint
    delete _constraint;        
  }

  virtual int getSuspListLengthV(void) { 
    return getSuspListLength(); 
  }

  virtual void printStreamV(ostream &out, int depth = 10) {
    out << _definition->getName() << ':' << _constraint->toString(depth);
  }

  virtual void printLongStreamV(ostream &out,
				int depth = 10,
				int offset = 0) {
    printStreamV(out,depth); out << endl;
  }
};


OZ_Return tellBasicConstraint(OZ_Term, OZ_GenConstraint *, OZ_GenDefinition *);


#ifndef OUTLINE
#include "ctgenvar.icc"
#else
#undef inline

GenCtVariable * tagged2GenCtVar(OZ_Term);
Bool isGenCtVar(OZ_Term term, TypeOfTerm tag);
OZ_GenConstraint * unpatchReifiedCt(OZ_Term t);
void addSuspCtVar(OZ_Term, Suspension, OZ_GenWakeUpDescriptor);

#endif
 
#endif /*  __CTGENVAR__H__ */

// eof
//-----------------------------------------------------------------------------
