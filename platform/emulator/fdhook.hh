/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __FDHOOK__H__
#define __FDHOOK__H__

#ifdef INTERFACE
#pragma interface
#endif

#include "types.hh"

#include "am.hh"

#ifdef DEBUG_CHECK
#define DEBUG_STABLE
#endif

#ifdef DEBUG_STABLE
extern SuspList * board_constraints;
void printBC(ostream &, Board *);
void printBCDebug(Board * = NULL);
#endif

SuspList * addSuspToList(SuspList * list, SuspList * elem, Board * home);
SuspList * addSuspToList(SuspList * list, Thread * elem, Board * home);

Thread * createPropagator (OZ_CFun func, int arity, RefsArray xregs);
Thread * createNewPropagator (OZ_Propagator * p,

int prio = PROPAGATOR_PRIORITY);

inline
Bool isUnifyCurrentPropagator () {
  Assert (am.currentThread->isPropagator () ||
          am.currentThread->isNewPropagator ());
  return (am.currentThread->isUnifyThread ());
}

inline
Thread *makeHeadThread (OZ_CFun fun,
                        OZ_Term *args, int arity)
{
  return (new Thread (am.currentBoard,
                      ozconf.defaultPriority,
                      fun, args, arity));
}

inline
Thread *makeHeadThread (OZ_Propagator * p, int prio)
{
  return new Thread (am.currentBoard, prio, p);
}

#endif
