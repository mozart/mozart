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
  DebugCheck(FDcurrentTaskSusp->isResistant() == NO,
	     error("Cannot revive non-resistant suspension."));
  DebugCheck(FDcurrentTaskSusp->isDead() == OK,
	     error("Cannot revive dead suspension."));
  FDcurrentTaskSusp->unmarkPropagated();
  FDcurrentTaskSusp->setNode(am.currentBoard);
  am.currentBoard->incSuspCount();
  FDcurrentTaskSusp = NULL;
}


void killPropagatedCurrentTaskSusp(void) {
  if (FDcurrentTaskSusp == NULL) return;

  DebugCheck(FDcurrentTaskSusp->isResistant() == NO,
	     error("Cannot kill non-resistant suspension."));
  DebugCheck(FDcurrentTaskSusp->isDead() == OK,
	     error("Suspension already dead."));

  if (FDcurrentTaskSusp->isPropagated() == NO) {
    FDcurrentTaskSusp = NULL;
    return;
  }
      
  FDcurrentTaskSusp->markDead();
  (void) am.checkExtSuspension(FDcurrentTaskSusp);
  FDcurrentTaskSusp = NULL;
};


void dismissCurrentTaskSusp(void) {
  DebugCheck(FDcurrentTaskSusp->isResistant() == NO,
	     error("Cannot dismiss non-resistant suspension."));
  DebugCheck(FDcurrentTaskSusp->isDead() == OK,
	     error("Suspension is dead."));
  
  FDcurrentTaskSusp->cContToNode(am.currentBoard);
  FDcurrentTaskSusp = NULL;
}


void undoTrailing(int n) {
  while(n--) {
    TaggedRef * refPtr;
    TaggedRef value;
    am.trail.popRef(refPtr,value);
    *refPtr = value;
  }
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


SuspList * addVirtualConstr(SuspList * list, SuspList * elem, Board * home)
{
  Suspension * susp = elem->getSusp ();
  updateExtSuspension (home->getBoardDeref(), susp);

  elem->setNext(list);
  return elem;
}


void addVirtualConstr(SVariable * var, SuspList * elem)
{
  Suspension * susp = elem->getSusp ();
  updateExtSuspension (var->getHome(), susp);

  elem->setNext(var->getSuspList());
  var->setSuspList(elem);
}
