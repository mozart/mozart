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

#ifdef __GNUC__
#pragma interface
#endif

#include "types.hh"

#include "am.hh"
#include "suspensi.hh"

extern Suspension * FDcurrentTaskSusp;

#ifdef DEBUG_CHECK
//#define DEBUG_STABLE
#endif

#ifdef DEBUG_STABLE
extern SuspList * board_constraints;
void printBC(ostream &, Board *);
void printBCDebug(Board * b = NULL) { printBC(cerr, b); }
#endif

void reviveCurrentTaskSusp(void);
void killPropagatedCurrentTaskSusp(void);
void dismissCurrentTaskSusp(void);

inline 
Bool isUnifyCurrentTaskSusp(void) {
  return FDcurrentTaskSusp->isUnifySusp();
}

SuspList * addSuspToList(SuspList * list, SuspList * elem, Board * home);

Suspension * makeHeadSuspension(OZ_Bool (* fun)(int, OZ_Term[]),
				OZ_Term * args, int arity);


#endif
