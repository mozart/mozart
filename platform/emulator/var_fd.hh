/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __GENFDVAR__H__
#define __GENFDVAR__H__

#if defined(__GNUC__)
#pragma interface
#endif

#include "genvar.hh"
#include "fdomn.hh"
#include "fdhook.hh"

#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline
#endif

//-----------------------------------------------------------------------------
//                           class GenFDVariable
//-----------------------------------------------------------------------------

class GenFDVariable: public GenCVariable {

friend class GenCVariable;
friend inline void addSuspFDVar(TaggedRef, SuspList *, FDPropState);

private:
  FiniteDomain finiteDomain;
  SuspList * fdSuspList[fd_any];

public:
  GenFDVariable(FiniteDomain &fd, TaggedRef pn = AtomVoid)
  : GenCVariable(FDVariable, pn) {
    finiteDomain = fd;
    fdSuspList[fd_det] = fdSuspList[fd_bounds] = NULL;
  }

  GenFDVariable(TaggedRef pn = AtomVoid)
  : GenCVariable(FDVariable, pn) {
    finiteDomain.setFull();
    fdSuspList[fd_det] = fdSuspList[fd_bounds] = NULL;
  }

  // methods relevant for term copying (gc and solve)
  void gc(void);
  size_t getSize(void){return sizeof(GenFDVariable);}
  void dispose(void);

  Bool unifyFD(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, Bool);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr);

  // is X=val still valid, i.e. is val an smallint and is it still in the domain
  Bool valid(TaggedRef val);


  void setDom(FiniteDomain &fd) {finiteDomain = fd;}
  FiniteDomain &getDom(void) {return finiteDomain;}

  void relinkSuspListTo(GenFDVariable * lv, Bool reset_local = FALSE);

  void propagate(TaggedRef var, FDPropState state,
                 TaggedRef term, PropCaller prop_eq = pc_propagator);

  int getSuspListLength(void) {
    return suspList->length() +
      fdSuspList[fd_det]->length() + fdSuspList[fd_bounds]->length();
  }
};

inline Bool isGenFDVar(TaggedRef term);
inline Bool isGenFDVar(TaggedRef term, TypeOfTerm tag);
inline GenFDVariable * tagged2GenFDVar(TaggedRef term);
inline void addSuspFDVar(TaggedRef, SuspList *, FDPropState = fd_any);

#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdgenvar.icc"
#else
#undef inline
#endif

#endif
