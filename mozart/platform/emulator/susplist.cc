/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "susplist.hh"
#endif

#include "susplist.hh"
#include "board.hh"

#ifdef OUTLINE
#define inline
#endif

SuspList * SuspList::_gc_sentinel;

void SuspList::init(void) {
  _gc_sentinel = (SuspList *) malloc(sizeof(SuspList));
}

Bool SuspList::hasSuspAt(Board * b) {
  b = b->derefBoard();
  for (SuspList * sl = this; sl ; sl = sl->getNext()) {
    Suspendable * s = sl->getSuspendable();
    if (!s->isDead() && !s->isRunnable() &&
	(b == s->getBoardInternal()->derefBoard()))
      return OK;
  }
  return NO;
}

int SuspList::length(void) {
  int i=0;
  for (SuspList * aux = this; aux != NULL; aux = aux->getNext()) {
    if (!aux->getSuspendable()->isDead() && 
    	!aux->getSuspendable()->isRunnable())
      i++;
  }
  return i;
} 

int SuspList::lengthProp(void) {
  int i=0;
  for (SuspList * aux = this; aux != NULL; aux = aux->getNext()) {
    if (!aux->getSuspendable()->isDead() && 
	aux->getSuspendable()->isRunnable())
      i++;
  }
  return i;
} 

SuspList * SuspList::appendToAndUnlink(SuspList * &to_list, Bool reset_local)
{
  if (this) { 
    SuspList * aux;
    
    if (reset_local) 
      for (aux = this; aux; aux = aux->getNext())
      	aux->getSuspendable()->unsetLocal();
    
    if (to_list) {
      // this != NULL && to_list != NULL
      
      // multiple entries of suspensions have to be removed; therefore ...
      // ... tag this-list and find last entry
      SuspList * last DebugCode(= NULL);
      for (aux = this; 1; ) {
	aux->getSuspendable()->setTagged();
	SuspList * aux_next = aux->getNext(); 
	
	if (aux_next) {
	  aux = aux_next; 
	} else {
	  last = aux;
	  break;
	}
      }

      Assert (last);
      
      // remove tagged entries from to_list 
      SuspList ** aux1 = &to_list;
      while (*aux1) {
	if ((*aux1)->getSuspendable()->isTagged()) {
	  *aux1 = (*aux1)->getNext();
	  continue;
	}
	aux1 = &(*aux1)->_next;
      }
      
      // untag this-list
      for (aux = this; aux; aux = aux->getNext())
      	aux->getSuspendable()->unsetTagged();
      
      last->setNext(to_list);
    } 

    to_list = this;
  }
  return NULL;
}


// drop every list entry referring to `prop'
SuspList *  SuspList::dropPropagator(Propagator * prop) {
  SuspList * head = NULL;
  SuspList * prev = NULL;
  SuspList * curr = this;

  while (curr) {
    Suspendable * susp = curr->getSuspendable();
    if (susp->isPropagator() && (SuspToPropagator(susp) == prop)) {
      curr = curr->getNext();
    } else {
      if (prev == NULL) {
	head = prev = curr;
      } else {
	prev->_next = curr;
	prev = curr;
      }
      curr = curr->getNext();
    }
  }
  return head;
}

//-----------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "susplist.icc"
#undef inline
#endif
