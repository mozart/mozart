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
#include "suspensi.hh"
#include "trail.hh"
#include "thread.hh"
#include "fdhook.hh"

Suspension* FDcurrentTaskSusp = NULL;

void reviveCurrentTaskSusp(void) {
  DebugCheck(FDcurrentTaskSusp == NULL,
	     error("FDcurrentTaskSusp is NULL in reviveFDcurrentTaskSusp."));
  DebugCheck(!FDcurrentTaskSusp->isResistant(),
	     error("Cannot revive non-resistant suspension."));
  DebugCheck(FDcurrentTaskSusp->isDead(),
	     error("Cannot revive dead suspension."));
  FDcurrentTaskSusp->unmarkPropagated();
  FDcurrentTaskSusp->unmarkUnifySusp();
  FDcurrentTaskSusp->setBoard(am.currentBoard);
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
  
  FDcurrentTaskSusp->cContToBoard(am.currentBoard);
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
#ifdef DEBUG_STABLE
  static Suspension * board_constraints_susp = NULL;
  if (board_constraints_susp != elem->getSusp()) {
    board_constraints_susp = elem->getSusp();
    board_constraints = new SuspList(board_constraints_susp,board_constraints);
  }
#endif
  
  updateExtSuspension (home->getBoardDeref(), elem->getSusp());
  elem->setNext(list);
  return elem;
}

#ifdef DEBUG_STABLE
SuspList * board_constraints = NULL;

void printBC(ostream &ofile, Board * b)
{
  for (SuspList * sl = board_constraints; sl != NULL; sl = sl->getNext()) {
    Suspension * s = sl->getSusp();
    if (s->isDead())
      continue;
    if (s->getNode()->getBoardDeref() == NULL)
      continue;
    
    if (sl->isCondSusp())
      ofile << ((CondSuspList*)sl)->getCondNum() << " conds";
    else 
      ofile << "true";
    
    s->print(ofile);
    ofile << "  " << (void *) b << endl;
    
  } 
  ofile.flush();
}
#endif
