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
friend OZ_Bool fdDomainConstrain(TaggedRef &var, TaggedRef * &varPtr,
				 FiniteDomain &domain);
friend class GenCVariable;

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
    finiteDomain.init(fu);
    fdSuspList[det] = fdSuspList[bounds] = NULL;
    fdSuspList[size] = fdSuspList[eqvar] = NULL;
  }

  // methods relevant for term copying (gc and solve)
  void gc(void); 
  size_t getSize(void){return sizeof(GenFDVariable);}

  Bool unifyFD(TaggedRef *, TaggedRef, TypeOfTerm, 
	       TaggedRef *, TaggedRef, TypeOfTerm);

  void becomesSmallIntAndPropagate(TaggedRef * trPtr);
   
  ProgramCounter index(ProgramCounter elseLabel, IHashTable * table);

  void setDom(FiniteDomain &fd){finiteDomain = fd;}
  FiniteDomain &getDom(void){return finiteDomain;}

  void relinkSuspList(GenFDVariable * leftVar);

  void addVirtualConstr(SuspList * elem) {
    ::addVirtualConstr(this, elem);
  }
      
  void addVirtualConstr(SuspList * elem, FDState state) {
    DebugCheck(state > any, error("Unexpected state.")); 
    if (state == any)
      addVirtualConstr(elem);
    else
      fdSuspList[state] = ::addVirtualConstr(fdSuspList[state], elem, home);
  }

  void addVirtualConstrLocal(SuspList * elem, FDState state) {
    DebugCheck(state > any, error("Unexpected state.")); 
    if (isLocalVariable() == OK || state == any || state == eqvar)
      addVirtualConstr(elem, state);
    else
      fdSuspList[size] = ::addVirtualConstr(fdSuspList[size], elem, home);
      
  }

  void propagate(TaggedRef var, FDState state, TaggedRef term);  
  void propagate(TaggedRef var, FDState state, TaggedRef * tPtr);

};


//-----------------------------------------------------------------------------
//            Some additional stuff for tagged references
//-----------------------------------------------------------------------------

Bool isGenFDVar(TaggedRef term);
GenFDVariable * tagged2GenFDVar(TaggedRef term);

//-----------------------------------------------------------------------------
//                   Functions to constrain variables
//-----------------------------------------------------------------------------

OZ_Bool fdDomainConstrain(TaggedRef &var, TaggedRef * &varPtr,
			  FiniteDomain &domain);
     


#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdgenvar.icc"
#endif

#endif

