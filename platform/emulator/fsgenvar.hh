/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
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
friend void addSuspFSetVar(OZ_Term, Thread *, OZ_FSetPropState);

private:
  OZ_FSetConstraint _fset;
  SuspList * fsSuspList[fs_prop_any];

public:

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

  void gc(void);
  size_t getSize(void){return sizeof(GenFSetVariable);}
  void dispose(void);

  Bool unifyFSet(OZ_Term *, OZ_Term, OZ_Term *, OZ_Term,
                 ByteCode *, Bool = TRUE);
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

  void propagate(OZ_Term var, OZ_FSetPropState state,
                 PropCaller prop_eq = pc_propagator);

  void propagateUnify(OZ_Term var);

  void becomesFSetValueAndPropagate(OZ_Term *);

  void installPropagators(GenFSetVariable *, Board *);

  OZ_FSetConstraint * getReifiedPatch(void) {
    return (OZ_FSetConstraint *)  (u.var_type & ~1);
  }
  void patchReified(OZ_FSetConstraint * s) {
    u.patchFSet =  (OZ_FSetConstraint *) ToPointer(ToInt32(s) | u_fset);
    setReifiedFlag();
  }
  void unpatchReified(void) {
    setType(FSetVariable);
    resetReifiedFlag();
  }
};

void addSuspFSetVar(OZ_Term, SuspList *, OZ_FSetPropState = fs_prop_any);
void addSuspFSetVar(OZ_Term, Thread *, OZ_FSetPropState = fs_prop_any);
OZ_Return tellBasicConstraint(OZ_Term, OZ_FSetConstraint *);

#if !defined(OUTLINE)
#include "fsgenvar.icc"
#else
Bool isGenFSetVar(OZ_Term term);
Bool isGenFSetVar(OZ_Term term, TypeOfTerm tag);
GenFSetVariable * tagged2GenFSetVar(OZ_Term term);
OZ_FSetConstraint * unpatchReified(OZ_Term t);

#undef inline
#endif

#endif // __FSGENVAR_HH__
