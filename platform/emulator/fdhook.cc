/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller

  ------------------------------------------------------------------------
*/


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "fdhook.hh"
#endif

#include "am.hh"

#include "fdhook.hh"

Thread *board_constraints_thr = NULL;

SuspList *addSuspToList(SuspList *list, Thread *elem, Board *hoome)
{
  if (list && (list->getElem() == elem)) 
    return list;
  
  SuspList * sl_elem = new SuspList(elem, list);

#ifdef DEBUG_STABLE
  if (board_constraints_thr != elem) {
    board_constraints_thr = elem;
    board_constraints = new SuspList(board_constraints_thr, board_constraints);
  }
#endif
  
  elem->updateExtThread(hoome->derefBoard());
  return sl_elem;
}

#ifdef DEBUG_STABLE
SuspList * board_constraints = NULL;

void printBCDebug(Board * b) { printBC(cerr, b); }

void printBC(ostream &ofile, Board * b)
{
  SuspList *sl;
  Board *hb;

  sl = board_constraints; 
  board_constraints = (SuspList *) NULL;

  while (sl != NULL) {
    Thread *thr = sl->getElem ();
    if (thr->isDeadThread () ||
        (hb = thr->getBoard()) == NULL ||
        hb->isFailed ()) {
      sl = sl->dispose ();
      continue;
    }

    thr->print (ofile);
    ofile << endl;
    if (b) { 
      ofile << "    ---> " << (void *) b << endl; 
    }

    sl = sl->getNext();
    board_constraints = new SuspList (thr, board_constraints);
  }

  ofile.flush();
}

#endif
