/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(__GNUC__) && !defined(NOPRAGMA)
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
  Assert(FDcurrentTaskSusp != NULL);
  Assert(FDcurrentTaskSusp->isResistant());
  Assert(!FDcurrentTaskSusp->isDead());
  Assert(am.currentBoard==FDcurrentTaskSusp->getBoardFast());
  FDcurrentTaskSusp->unmarkPropagated();
  FDcurrentTaskSusp->unmarkUnifySusp();
#ifndef NEWCOUNTER
  am.currentBoard->incSuspCount();
#endif
  Assert(am.currentBoard==FDcurrentTaskSusp->getBoardFast());
  FDcurrentTaskSusp->setBoard(am.currentBoard);
  FDcurrentTaskSusp = NULL;
}


void killPropagatedCurrentTaskSusp() {
  if (FDcurrentTaskSusp == NULL) return;

  Assert(am.currentBoard==FDcurrentTaskSusp->getBoardFast());
  Assert(FDcurrentTaskSusp->isResistant());
  Assert(!FDcurrentTaskSusp->isDead());

  // constructive disjunction ???
  if (!FDcurrentTaskSusp->isPropagated()) {
    FDcurrentTaskSusp = NULL;
    return;
  }

  FDcurrentTaskSusp->markDead();
#ifdef NEWCOUNTER
  am.currentBoard->decSuspCount();
#endif
  am.checkExtSuspension(FDcurrentTaskSusp);
  FDcurrentTaskSusp = NULL;
}


SuspList * addSuspToList(SuspList * list, SuspList * elem, Board * hoome)
{
#ifdef DEBUG_STABLE
  static Suspension * board_constraints_susp = NULL;
  if (board_constraints_susp != elem->getSusp()) {
    board_constraints_susp = elem->getSusp();
    board_constraints = new SuspList(board_constraints_susp,board_constraints);
  }
#endif
  
  updateExtSuspension (hoome->getBoardFast(), elem->getSusp());
  elem->setNext(list);
  return elem;
}

Suspension * createResSusp(OZ_CFun func, int arity, RefsArray xregs)
{
  Suspension * s = makeHeadSuspension(func, xregs, arity);

  s->headInit();

  Assert(FDcurrentTaskSusp == NULL);
  
  FDcurrentTaskSusp = s;
  return s;
}

#ifdef DEBUG_STABLE
SuspList * board_constraints = NULL;

void printBCDebug(Board * b) { printBC(cerr, b); }

void printBC(ostream &ofile, Board * b)
{
  for (SuspList * sl = board_constraints; sl != NULL; sl = sl->getNext()) {
    Suspension * s = sl->getSusp();
    if (s->isDead())
      continue;
    if (s->getBoardFast() == NULL)
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
