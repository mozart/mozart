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
friend void addSuspFSetVar(OZ_Term, Suspendable *, OZ_FSetPropState);

private:
  OZ_FSetConstraint _fset;
  SuspList * fsSuspList[fs_prop_any];
  
public:
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

  void gCollect(Board *); 
  void sClone(Board *); 

  void dispose(void);
  
  // methods for trailing
  OzVariable * copyForTrail(void);
  void restoreFromCopy(OzFSVariable *);
  
  OZ_Return bind(OZ_Term *, OZ_Term);
  OZ_Return unify(OZ_Term *, OZ_Term*);
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

  void printStream(ostream &out,int depth = 10) {
    out << getSet().toString();
  }
  void printLongStream(ostream &out,int depth = 10,
		       int offset = 0) {
    printStream(out,depth); out << endl;
  }
  //
  void dropPropagator(Propagator * prop) {
    for (int i = fs_prop_any; i--; ) {
      fsSuspList[i] = fsSuspList[i]->dropPropagator(prop);
    }
    suspList = suspList->dropPropagator(prop);
  }
  //
  // tagging and untagging constrained variables
  //
  OZ_FSetVar * getTag(void) {
    return (OZ_FSetVar *)  (u.var_type & ~u_mask);
  }
  //
  // end of tagging ...
  //
};

void addSuspFSetVar(OZ_Term, SuspList *, OZ_FSetPropState = fs_prop_any);
void addSuspFSetVar(OZ_Term, Suspendable *, OZ_FSetPropState = fs_prop_any);
OZ_Return tellBasicConstraint(OZ_Term, OZ_FSetConstraint *);

#if !defined(OUTLINE)
#include "var_fs.icc"
#else
Bool isGenFSetVar(OZ_Term term);
OzFSVariable * tagged2GenFSetVar(OZ_Term term);

#undef inline
#endif

#endif // __FSGENVAR_HH__
