/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#ifdef __GNUC__
#pragma implementation "fdhook.hh"
#endif

#include "types.hh"

#include "am.hh"
#include "board.hh"
#include "suspension.hh"
#include "trail.hh"
#include "thread.hh"

Suspension* FDcurrentTaskSusp = NULL;

void reviveCurrentTaskSusp(void) {
  DebugCheck(FDcurrentTaskSusp == NULL,
	     error("FDcurrentTaskSusp is NULL in reviveFDcurrentTaskSusp."));
  DebugCheck(!FDcurrentTaskSusp->isResistant(),
	     error("Cannot revive non-resistant suspension."));
  DebugCheck(FDcurrentTaskSusp->isDead(),
	     error("Cannot revive dead suspension."));
  FDcurrentTaskSusp->unmarkPropagated();
  FDcurrentTaskSusp->setNode(am.currentBoard);
  am.currentBoard->incSuspCount();
  FDcurrentTaskSusp = NULL;
}


void killPropagatedCurrentTaskSusp(void) {
  if (FDcurrentTaskSusp == NULL) return;

  DebugCheck(!FDcurrentTaskSusp->isResistant(),
	     error("Cannot kill non-resistant suspension."));
  DebugCheck(FDcurrentTaskSusp->isDead(),
	     error("Suspension already dead."));

  if (!FDcurrentTaskSusp->isPropagated()) {
    FDcurrentTaskSusp = NULL;
    return;
  }
      
  FDcurrentTaskSusp->markDead();
  am.checkExtSuspension(FDcurrentTaskSusp);
  FDcurrentTaskSusp = NULL;
};


void dismissCurrentTaskSusp(void) {
  DebugCheck(!FDcurrentTaskSusp->isResistant(),
	     error("Cannot dismiss non-resistant suspension."));
  DebugCheck(FDcurrentTaskSusp->isDead(),
	     error("Suspension is dead."));
  
  FDcurrentTaskSusp->cContToNode(am.currentBoard);
  FDcurrentTaskSusp = NULL;
}


Suspension * makeHeadSuspension(OZ_Bool (*fun)(int,OZ_Term[]),
				OZ_Term * args, int arity)
{
  CFuncContinuation * c =
    new CFuncContinuation(am.currentBoard,
			  am.currentThread->getPriority(),
			  fun, args, arity);
  return new Suspension(c);
}


SuspList * addSuspToList(SuspList * list, SuspList * elem, Board * home)
{
  updateExtSuspension (home->getBoardDeref(), elem->getSusp());
  elem->setNext(list);
  return elem;
}

