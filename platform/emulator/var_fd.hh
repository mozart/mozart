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
friend inline void addSuspFDVar(TaggedRef, SuspList *, FDPropState);
  
private:
  OZ_FiniteDomain finiteDomain;
  SuspList * fdSuspList[fd_any];
  
  void relinkSuspListToItself(Bool reset_local = FALSE);

  GenBoolVariable * becomesBool(void) { 
    relinkSuspListToItself();
    setType(BoolVariable); 
    
    finiteDomain.dispose();
    // sizeof(GenCVariable) == sizeof(GenBoolVariable) !!!
    freeListDispose(((char *)this) + sizeof(GenCVariable), 
		    sizeof(GenFDVariable) - sizeof(GenCVariable));
    return (GenBoolVariable *) this;
  }
public:  
  GenFDVariable(OZ_FiniteDomain &fd) : GenCVariable(FDVariable) {
    finiteDomain = fd;
    fdSuspList[fd_det] = fdSuspList[fd_bounds] = NULL;
  }

  GenFDVariable() : GenCVariable(FDVariable) {
    finiteDomain.setFull();
    fdSuspList[fd_det] = fdSuspList[fd_bounds] = NULL;
  }

  // methods relevant for term copying (gc and solve)
  void gc(void); 
  size_t getSize(void){return sizeof(GenFDVariable);}
  void dispose(void);
  
  Bool unifyFD(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef,
	       Bool, Bool = TRUE);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr);
  void becomesBoolVarAndPropagate(TaggedRef * trPtr);

  int intersectWithBool(void) {return finiteDomain.intersectWithBool();}

  // is X=val still valid, i.e. is val a smallint and is it still in the domain
  Bool valid(TaggedRef val);


  void setDom(OZ_FiniteDomain &fd) {
    Assert(fd != fd_bool);
    finiteDomain = fd;
  }
  OZ_FiniteDomain &getDom(void) {return finiteDomain;}

  void relinkSuspListTo(GenFDVariable * lv, Bool reset_local = FALSE);
  void relinkSuspListTo(GenBoolVariable * lv, Bool reset_local = FALSE);

  void propagate(TaggedRef var, FDPropState state,
		 PropCaller prop_eq = pc_propagator);  

  void propagateUnify(TaggedRef var);  

  int getSuspListLength(void) {
    return suspList->length() +
      fdSuspList[fd_det]->length() + fdSuspList[fd_bounds]->length();
  }

  void installPropagators(GenFDVariable *, Board *);

  void addDetSusp(Thread *susp) {
    fdSuspList[fd_det] = addSuspToList(fdSuspList[fd_det], 
				       new SuspList(susp,NULL), home);
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



