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

class GenFSetVariable: public GenCVariable {
private:
  OZ_FSet _fset;

public:
  GenFSetVariable(OZ_FSet &fs) : GenCVariable(FSetVariable) { _fset = fs; }

  void gc(void);
  size_t getSize(void){return sizeof(GenFSetVariable);}
  void dispose(void);

  Bool unifyFSet(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef,
                 Bool, Bool = TRUE);
  OZ_FSet &getSet(void) { return _fset; }
  void setSet(OZ_FSet fs) { _fset = fs; }

  Bool valid(TaggedRef val);

  int getSuspListLength(void) { return suspList->length(); }

  void propagate(TaggedRef var, PropCaller prop_eq = pc_propagator) {
    if (suspList) GenCVariable::propagate(var, suspList, prop_eq);
  }
  void propagateUnify(TaggedRef var) { propagate(var, pc_cv_unif); }
};

inline
GenFSetVariable * tagged2GenFSetVar(TaggedRef term)
{
  GCDEBUG(term);
  return (GenFSetVariable *) tagged2CVar(term);
}

inline Bool isGenFSetVar(TaggedRef term, TypeOfTerm tag);
inline GenFSetVariable * tagged2GenFSetVar(TaggedRef term);
inline void addSuspFSetVar(TaggedRef, SuspList *);
inline void addSuspFSetVar(TaggedRef, Thread *);

#endif // __FSGENVAR_HH__
