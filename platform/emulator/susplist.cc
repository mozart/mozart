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

//-----------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "susplist.icc"
#undef inline
#endif
