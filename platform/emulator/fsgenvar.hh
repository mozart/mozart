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

#ifdef FSETVAR

enum OZ_FSetPropState {fs_glb = 0, fs_lub, fs_val, fs_any};

class GenFSetVariable: public GenCVariable {
private:
  OZ_FSet _fset;
  SuspList * fsSuspList[fs_any];
  
public:
  GenFSetVariable(OZ_FSet &fs) : GenCVariable(FSetVariable) { 
    _fset = fs; 
    for (int i = fs_any; i--; )
      fsSuspList[i] = NULL;
  }

  void gc(void); 
  size_t getSize(void){return sizeof(GenFSetVariable);}
  void dispose(void);
  
  Bool unifyFSet(OZ_Term *, OZ_Term, OZ_Term *, OZ_Term,
		 Bool, Bool = TRUE);
  OZ_FSet &getSet(void) { return _fset; }
  void setSet(OZ_FSet fs) { _fset = fs; }

  Bool valid(OZ_Term val);

  int getSuspListLength(void) { 
    int len = suspList->length(); 
    for (int i = fs_any; i--; )
      len += fsSuspList[i]->length();
    return len;
  }

  void propagate(OZ_Term var, OZ_FSetPropState state,
		 PropCaller prop_eq = pc_propagator);

  void propagateUnify(OZ_Term var);
};

inline
GenFSetVariable * tagged2GenFSetVar(OZ_Term term)
{
  GCDEBUG(term);
  return (GenFSetVariable *) tagged2CVar(term);
}

inline Bool isGenFSetVar(OZ_Term term, TypeOfTerm tag);
inline GenFSetVariable * tagged2GenFSetVar(OZ_Term term);
inline void addSuspFSetVar(OZ_Term, SuspList *);
inline void addSuspFSetVar(OZ_Term, Thread *);

#if !defined(OUTLINE)
#include "fsgenvar.icc"
#else
#undef inline
#endif

#endif /* FSETVAR */

#endif // __FSGENVAR_HH__

