/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller

  ------------------------------------------------------------------------
*/

#ifndef __FDHOOK__H__
#define __FDHOOK__H__

#ifdef INTERFACE
#pragma interface
#endif

#include "types.hh"

#include "runtime.hh"

#define DEBUG_STABLE
#ifdef DEBUG_STABLE
extern SuspList * board_constraints;
void printBC(ostream &, Board *);
void printBCDebug(Board * = NULL);
#endif

SuspList * addSuspToList(SuspList * list, SuspList * elem, Board * home);
SuspList * addSuspToList(SuspList * list, Thread * elem, Board * home);

inline
Bool isUnifyCurrentPropagator () {
  Assert (am.currentThread->isPropagator ());
  return (am.currentThread->isUnifyThread ());
}

#endif
