/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl,tmueller,popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(INTERFACE)
#pragma implementation "susplist.hh"
#endif

#include "susplist.hh"
#include "board.hh"

#ifdef OUTLINE
#define inline
#endif

//-----------------------------------------------------------------------------
//                          class SuspList

int SuspList::length(void)
{
  int i=0;
  for(SuspList * aux = this; aux != NULL; aux = aux->next) {
    if (!aux->getElem()->isDeadThread () &&
	!aux->getElem()->isRunnable () &&
	GETBOARD(aux->getElem())) {
      i++;
    }
  }
  return i;
} 

int SuspList::lengthProp(void)
{
  int i=0;
  for(SuspList * aux = this; aux != NULL; aux = aux->next) {
    if (!aux->getElem()->isDeadThread () &&
	aux->getElem()->isRunnable () &&
	GETBOARD(aux->getElem())) {
      i++;
    }
  }
  return i;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

void OrderedSuspList::print(void)
{
  for (OrderedSuspList * p = this; p != NULL; p = p->n) {
    OZ_Propagator * pr = p->t->getPropagator();
    cout << "   " << pr->toString() 
	 << endl << flush;
  }
}

#ifdef OUTLINE
#define inline
#include "susplist.icc"
#undef inline
#endif
