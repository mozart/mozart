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

#if defined(INTERFACE)
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
friend class GenBoolVariable;
friend inline void addSuspFDVar(TaggedRef, SuspList *, OZ_FDPropState);
friend inline void addSuspFDVar(TaggedRef, Thread *, OZ_FDPropState);

private:
  OZ_FiniteDomain finiteDomain;
  SuspList * fdSuspList[fd_prop_any];

  void relinkSuspListToItself(Bool reset_local = FALSE);

  GenBoolVariable * becomesBool(void);
public:
  GenFDVariable(OZ_FiniteDomain &fd) : GenCVariable(FDVariable) {
    ozstat.fdvarsCreated.incf();
    finiteDomain = fd;
    fdSuspList[fd_prop_singl] = fdSuspList[fd_prop_bounds] = NULL;
  }

  GenFDVariable() : GenCVariable(FDVariable) {
    ozstat.fdvarsCreated.incf();
    finiteDomain.initFull();
    fdSuspList[fd_prop_singl] = fdSuspList[fd_prop_bounds] = NULL;
  }

  // methods relevant for term copying (gc and solve)
  void gc(void);
  size_t getSize(void){return sizeof(GenFDVariable);}
  void dispose(void);

  Bool unifyFD(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef,
               Bool, Bool = TRUE);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr);
  void becomesBoolVarAndPropagate(TaggedRef * trPtr);

  int intersectWithBool(void);

  // is X=val still valid, i.e. is val a smallint and is it still in the domain
  Bool valid(TaggedRef val);

  void setDom(OZ_FiniteDomain &fd) {
    Assert(fd != fd_bool);
    finiteDomain = fd;
  }
  OZ_FiniteDomain &getDom(void) {return finiteDomain;}

  void relinkSuspListTo(GenFDVariable * lv, Bool reset_local = FALSE);
  void relinkSuspListTo(GenBoolVariable * lv, Bool reset_local = FALSE);

  void propagate(TaggedRef var, OZ_FDPropState state,
                 PropCaller prop_eq = pc_propagator);

  void propagateUnify(TaggedRef var);

  int getSuspListLength(void) {
    int len = suspList->length();
    for (int i = fd_prop_any; i--; )
      len += fdSuspList[i]->length();
    return len;
  }

  void installPropagators(GenFDVariable *, Board *);

  void addDetSusp(Thread *susp) {
    fdSuspList[fd_prop_singl] = addSuspToList(fdSuspList[fd_prop_singl],
                                       new SuspList(susp,NULL), home);
  }
};

Bool isGenFDVar(TaggedRef term);
Bool isGenFDVar(TaggedRef term, TypeOfTerm tag);
GenFDVariable * tagged2GenFDVar(TaggedRef term);
void addSuspFDVar(TaggedRef, SuspList *, OZ_FDPropState = fd_prop_any);
void addSuspFDVar(TaggedRef, Thread *, OZ_FDPropState = fd_prop_any);
OZ_Return tellBasicConstraint(OZ_Term, OZ_FiniteDomain *);

#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdgenvar.icc"
#else
#undef inline
#endif

#endif
