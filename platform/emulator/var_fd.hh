#ifndef __GENFDVAR__H__
#define __GENFDVAR__H__

#ifdef __GNUC__
#pragma interface
#endif

#include "genvar.hh"
#include "fdomn.hh"

//-----------------------------------------------------------------------------
//                           class GenFDVariable
//-----------------------------------------------------------------------------

class GenFDVariable: public GenCVariable{
friend OZ_Bool fdDomainConstrain(TaggedRef &var, TaggedRef* &varPtr,
                                 FiniteDomain &domain);
friend class GenCVariable;
private:
  FiniteDomain finiteDomain;
  SuspList* fdSuspList[any];
public:
  GenFDVariable(FiniteDomain &fd, TaggedRef pn = AtomVoid)
  : GenCVariable(FDVariable, pn) {
#ifdef PROFILE_FD
    FiniteDomain::varsCreated++;
#endif
    finiteDomain = fd;
    fdSuspList[det] = fdSuspList[bounds] = NULL;
    fdSuspList[size] = fdSuspList[eqvar] = NULL;
  }

  // methods relevant for term copying (gc and solve)
  void gc(void);
  size_t getSize(void){return sizeof(GenFDVariable);}

  Bool unifyFD(TaggedRef*, TaggedRef, TypeOfTerm,
               TaggedRef*, TaggedRef, TypeOfTerm);

  inline void becomesSmallIntAndPropagate(TaggedRef* trPtr);

  ProgramCounter index(ProgramCounter elseLabel, IHashTable* table);

  OZPRINT;
  OZPRINTLONG;

  void setDom(FiniteDomain &fd){finiteDomain = fd;}
  FiniteDomain &getDom(void){return finiteDomain;}

  inline void relinkSuspList(GenFDVariable* leftVar);

  void addVirtualConstr(SuspList *elem, FDState state) {
    if (state == any)
      addVirtualConstr(elem);
    else
      fdSuspList[state] =
        ::addVirtualConstr(fdSuspList[state], elem, clusterNode);
  }

  void addVirtualConstrLocal(SuspList *elem, FDState state) {
    if (isLocalVariable() == OK || state == any || state == eqvar)
      addVirtualConstr(elem, state);
    else
      fdSuspList[size] =
        ::addVirtualConstr(fdSuspList[size], elem, clusterNode);

  }

  void addVirtualConstr(SuspList *elem) {
    GenCVariable::addVirtualConstr(elem);
  }

  inline void propagate(TaggedRef var, FDState state, TaggedRef term);
  inline void propagate(TaggedRef var, FDState state, TaggedRef* tPtr);
};


//-----------------------------------------------------------------------------
//            Some additional stuff for tagged references
//-----------------------------------------------------------------------------

Bool isGenFDVar(TaggedRef term);
GenFDVariable* tagged2GenFDVar(TaggedRef term);

//-----------------------------------------------------------------------------
//                   Functions to constrain variables
//-----------------------------------------------------------------------------

OZ_Bool fdDomainConstrain(TaggedRef &var, TaggedRef* &varPtr,
                       FiniteDomain &domain);



#ifndef OUTLINE
#include "fdgenvar.icc"
#endif

#endif
