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

private:
  // if this pointer is `NULL', the variable is constraint to the 
  // least constraint of this kind
  OZ_GenConstraint * _constraint;
  OZ_GenDefinition * _definition;
  int _noOfSuspLists;
  SuspList ** _susp_lists;

  Bool unifyCt(OZ_Term *, OZ_Term, 
	       OZ_Term *, OZ_Term, 
	       ByteCode *, Bool = TRUE);
  
  void propagate(OZ_Term, OZ_GenWakeUpDescriptor, PropCaller);
  void propagateUnify(OZ_Term var) { 
    propagate(var, OZ_WAKEUP_ALL, pc_cv_unif); 
  }

  void copyConstraint(OZ_GenConstraint * c) { 
    *_constraint = *c;
  }

  void relinkSuspListTo(GenCtVariable * lv, Bool reset_local = FALSE);

  void installPropagators(GenCtVariable *, Board *);

public:
  USEFREELISTMEMORY;

  GenCtVariable() : GenCVariable(CtVariable) {}
  GenCtVariable(OZ_GenConstraint * c, OZ_GenDefinition * d) 
    : _definition(d), GenCVariable(CtVariable)
  {
    Assert(c);
    Assert(d);

    copyConstraint(c); 
    
    int no_of_wakup_lists = d->getNoOfWakeUpLists();

    _susp_lists = (SuspList **) 
      freeListMalloc(sizeof(SuspList *) * no_of_wakup_lists);

    for (int i = no_of_wakup_lists; i--; )
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


  OZ_Return unifyV(TaggedRef *vPtr,TaggedRef v,TaggedRef *tPtr,TaggedRef t,
		   ByteCode*scp) {
    return unifyCt(vPtr,v,tPtr,t,scp);
  }
  OZ_Return validV(TaggedRef* /* vPtr */, TaggedRef val ) {
    return FALSE; // mm2
  }
  OZ_Return hasFeatureV(TaggedRef val, TaggedRef *) { return SUSPEND; }
  GenCVariable* gcV() { error("not impl"); return 0; }
  void gcRecurseV() { error("not impl"); }
  void addSuspV(Suspension susp, TaggedRef* ptr, int state) {
    // mm2: addSuspFDVar(makeTaggedRef(ptr),susp,state);
  }
  Bool isKindedV() { return true; }
  void disposeV(void) { error("not impl"); }
  int getSuspListLengthV() { return getSuspListLength(); }
  void printV() {}
  void printLongV() {}

};

OZ_Return tellBasicConstraint(OZ_Term, OZ_GenConstraint *, OZ_GenDefinition *);



#ifndef OUTLINE 
#include "ctgenvar.icc"
#else
#undef inline

GenCtVariable * tagged2GenCtVar(OZ_Term);
Bool isGenCtVar(OZ_Term term, TypeOfTerm tag);
OZ_GenConstraint * unpatchReifiedCt(OZ_Term t);
void addSuspCtVar(TaggedRef, Suspension, OZ_GenWakeUpDescriptor);

#endif
 
#endif /*  __CTGENVAR__H__ */

// eof
//-----------------------------------------------------------------------------
