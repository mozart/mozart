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
  SuspList * new_list = (SuspList *) NULL; // head of new list
  SuspList * last_new_list = new_list;     // last of new list
  SuspList * old_list = this;              // list to be scanned

  while (old_list) {
    Suspendable * susp = old_list->getSuspendable();
    if (susp->isPropagator() && (SuspToPropagator(susp) == prop)) {
      //
      // skip entry
      //
      old_list = old_list->getNext();
      //
    } else {
      //
      // keep entry
      //
      SuspList * kept_entry = old_list;
      //
      // move on
      //
      old_list = old_list->getNext();
      //
      // close new list
      //
      kept_entry->_next = (SuspList *) NULL;
      //
      // append entry to new list
      //
      if (NULL != last_new_list) {
        last_new_list->_next = kept_entry;
      }
      //
      // move last entry
      last_new_list = kept_entry;
      //
      // init first element of new list
      //
      if (NULL == new_list) {
        new_list = kept_entry;
      }
      //
    }
  }
  return new_list;
}

//-----------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "susplist.icc"
#undef inline
#endif
