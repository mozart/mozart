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

#ifndef __FSGENVAR_HH__
#define __FSGENVAR_HH__

#if defined(INTERFACE)
#pragma interface
#endif


#include "var_base.hh"
#include "fset.hh"
#include "mozart_cpi.hh"

class OzFSVariable: public OzVariable {

friend class OzVariable;
friend void addSuspFSetVar(OZ_Term, Suspension, OZ_FSetPropState);

private:
  OZ_FSetConstraint _fset;
  SuspList * fsSuspList[fs_prop_any];

public:
  OzFSVariable(DummyClass *) : OzVariable(OZ_VAR_FS,(DummyClass*)0) {}
  OzFSVariable(Board *bb) : OzVariable(OZ_VAR_FS,bb) {
    _fset.init();
    for (int i = fs_prop_any; i--; )
      fsSuspList[i] = NULL;
  }
  OzFSVariable(OZ_FSetConstraint &fs,Board *bb)
    : OzVariable(OZ_VAR_FS,bb) {
    _fset = fs;
    for (int i = fs_prop_any; i--; )
      fsSuspList[i] = NULL;
  }

  void gc(OzFSVariable *);
  void dispose(void);

  OZ_Return bind(OZ_Term *, OZ_Term, ByteCode *);
  OZ_Return unify(OZ_Term *, OZ_Term*, ByteCode *);
  OZ_FSetConstraint &getSet(void) { return _fset; }
  void setSet(OZ_FSetConstraint fs) { _fset = fs; }

  Bool valid(OZ_Term val);

  int getSuspListLength(void) {
    int len = suspList->length();
    for (int i = fs_prop_any; i--; )
      len += fsSuspList[i]->length();
    return len;
  }

  SuspList * getSuspList(int i) { return fsSuspList[i]; }

  void relinkSuspListTo(OzFSVariable * lv, Bool reset_local = FALSE);

  void propagate(OZ_FSetPropState state,
                 PropCaller prop_eq = pc_propagator);

  void propagateUnify() {
    propagate(fs_prop_val, pc_cv_unif);
  }

  void becomesFSetValueAndPropagate(OZ_Term *);

  void installPropagators(OzFSVariable *);

  OZ_FSetConstraint * getReifiedPatch(void) {
    return (OZ_FSetConstraint *) (u.var_type & ~u_mask);
  }
  void patchReified(OZ_FSetConstraint * s) {
    u.patchFSet =  (OZ_FSetConstraint *) ToPointer(ToInt32(s) | u_fset);
    setReifiedFlag();
  }
  void unpatchReified(void) {
    setType(OZ_VAR_FS);
    resetReifiedFlag();
  }

  void printStream(ostream &out,int depth = 10) {
    out << getSet().toString();
  }
  void printLongStream(ostream &out,int depth = 10,
                       int offset = 0) {
    printStream(out,depth); out << endl;
  }
};

void addSuspFSetVar(OZ_Term, SuspList *, OZ_FSetPropState = fs_prop_any);
void addSuspFSetVar(OZ_Term, Suspension, OZ_FSetPropState = fs_prop_any);
OZ_Return tellBasicConstraint(OZ_Term, OZ_FSetConstraint *);

#if !defined(OUTLINE)
#include "var_fs.icc"
#else
Bool isGenFSetVar(OZ_Term term);
Bool isGenFSetVar(OZ_Term term, TypeOfTerm tag);
OzFSVariable * tagged2GenFSetVar(OZ_Term term);
OZ_FSetConstraint * unpatchReifiedFSet(OZ_Term t);

#undef inline
#endif

#endif // __FSGENVAR_HH__
