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

#ifndef __FSGENVAR_HH__
#define __FSGENVAR_HH__

#if defined(INTERFACE)
#pragma interface
#endif


#include "genvar.hh"
#include "fset.hh"
#include "fdhook.hh"
#include "oz_cpi.hh"

class GenFSetVariable: public GenCVariable {

friend class GenCVariable;
friend void addSuspFSetVar(OZ_Term, Suspension, OZ_FSetPropState);

private:
  OZ_FSetConstraint _fset;
  SuspList * fsSuspList[fs_prop_any];
  
public:
  GenFSetVariable(DummyClass *) : GenCVariable(FSetVariable,(DummyClass*)0) {}
  GenFSetVariable(void) : GenCVariable(FSetVariable) { 
    _fset.init(); 
    for (int i = fs_prop_any; i--; )
      fsSuspList[i] = NULL;
  }
  GenFSetVariable(OZ_FSetConstraint &fs) : GenCVariable(FSetVariable) { 
    _fset = fs; 
    for (int i = fs_prop_any; i--; )
      fsSuspList[i] = NULL;
  }

  void gc(GenFSetVariable *); 
  void dispose(void);
  
  OZ_Return unifyV(OZ_Term *, OZ_Term, ByteCode *);
  OZ_FSetConstraint &getSet(void) { return _fset; }
  void setSet(OZ_FSetConstraint fs) { _fset = fs; }

  Bool valid(OZ_Term val);

  int getSuspListLength(void) { 
    int len = suspList->length(); 
    for (int i = fs_prop_any; i--; )
      len += fsSuspList[i]->length();
    return len;
  }

  void relinkSuspListTo(GenFSetVariable * lv, Bool reset_local = FALSE);

  void propagate(OZ_FSetPropState state,
		 PropCaller prop_eq = pc_propagator);

  void propagateUnify() {
    propagate(fs_prop_val, pc_cv_unif); 
  }

  void becomesFSetValueAndPropagate(OZ_Term *);

  void installPropagators(GenFSetVariable *, Board *);

  OZ_FSetConstraint * getReifiedPatch(void) { 
    return (OZ_FSetConstraint *) (u.var_type & ~u_mask);
  }
  void patchReified(OZ_FSetConstraint * s) { 
    u.patchFSet =  (OZ_FSetConstraint *) ToPointer(ToInt32(s) | u_fset); 
    setReifiedFlag();
  }
  void unpatchReified(void) { 
    setType(FSetVariable); 
    resetReifiedFlag();
  }


  OZ_Return validV(TaggedRef* /* vPtr */, TaggedRef val ) {
    return valid(val);
  }
  GenCVariable* gcV() { error("not impl"); return 0; }
  void gcRecurseV() { error("not impl"); }
  void addSuspV(Suspension susp, TaggedRef* ptr, int state) {
    error("not impl");
    // mm2: addSuspBoolVar(makeTaggedRef(ptr),susp,state);
  }
  void disposeV(void) { dispose(); }
  int getSuspListLengthV() { return getSuspListLength(); }
  void printStreamV(ostream &out,int depth = 10) {
    out << getSet().toString();
  }
  void printLongStreamV(ostream &out,int depth = 10,
			int offset = 0) {
    printStreamV(out,depth); out << endl;
  }
};

void addSuspFSetVar(OZ_Term, SuspList *, OZ_FSetPropState = fs_prop_any);
void addSuspFSetVar(OZ_Term, Suspension, OZ_FSetPropState = fs_prop_any);
OZ_Return tellBasicConstraint(OZ_Term, OZ_FSetConstraint *);

#if !defined(OUTLINE)
#include "fsgenvar.icc"
#else
Bool isGenFSetVar(OZ_Term term);
Bool isGenFSetVar(OZ_Term term, TypeOfTerm tag);
GenFSetVariable * tagged2GenFSetVar(OZ_Term term);
OZ_FSetConstraint * unpatchReifiedFSet(OZ_Term t);

#undef inline
#endif

#endif // __FSGENVAR_HH__
