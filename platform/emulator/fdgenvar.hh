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


//-----------------------------------------------------------------------------
//                           class GenFDVariable
//-----------------------------------------------------------------------------

class GenFDVariable: public GenCVariable {

friend class GenCVariable;
friend void addSuspFDVar(TaggedRef, SuspList *, FDState);
friend void addSuspFDVar(TaggedRef, SuspList *);

private:
  FiniteDomain finiteDomain;
  SuspList * fdSuspList[any];

public:
  GenFDVariable(FiniteDomain &fd, TaggedRef pn = AtomVoid)
  : GenCVariable(FDVariable, pn) {
    finiteDomain = fd;
    fdSuspList[det] = fdSuspList[bounds] = NULL;
    fdSuspList[size] = fdSuspList[eqvar] = NULL;
  }

  GenFDVariable(TaggedRef pn = AtomVoid)
  : GenCVariable(FDVariable, pn) {
    finiteDomain.setFull();
    fdSuspList[det] = fdSuspList[bounds] = NULL;
    fdSuspList[size] = fdSuspList[eqvar] = NULL;
  }

  // methods relevant for term copying (gc and solve)
  void gc(void);
  size_t getSize(void){return sizeof(GenFDVariable);}

  Bool unifyFD(TaggedRef *, TaggedRef, TypeOfTerm,
               TaggedRef *, TaggedRef, TypeOfTerm);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr);

  // is X=val still valid, i.e. is val an smallint and is it still in the domain
  Bool valid(TaggedRef val);


  void setDom(FiniteDomain &fd) {finiteDomain = fd;}
  FiniteDomain &getDom(void) {return finiteDomain;}

  void relinkSuspList(GenFDVariable * leftVar);

  void propagate(TaggedRef var, FDState state,
                 TaggedRef term, Bool prop_eq = FALSE);
  void propagate(TaggedRef var, FDState state,
                 TaggedRef * tPtr, Bool prop_eq = FALSE);

};


Bool isGenFDVar(TaggedRef term);
GenFDVariable * tagged2GenFDVar(TaggedRef term);


void addSuspFDVar(TaggedRef v, SuspList * el, FDState l)
{
  DebugCheck(l > eqvar, error("list index out of range."));

  GenFDVariable * fv = tagged2GenFDVar(v);
  fv->fdSuspList[l] = addSuspToList(fv->fdSuspList[l], el, fv->home);
}

void addSuspFDVar(TaggedRef v, SuspList * el)
{
  GenFDVariable * fv = tagged2GenFDVar(v);
  fv->suspList = addSuspToList(fv->suspList, el, fv->home);
}


#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdgenvar.icc"
#endif

#endif
