/*
 *  Authors:
 *    Christian Schulte (schulte@dfki.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */


#if defined(INTERFACE)
#pragma implementation "cpbag.hh"
#endif

#include "cpbag.hh"
#include "actor.hh"

CpBag * CpBag::get(WaitActor ** wa) {
  *wa = (WaitActor *) 0;

  CpBag * cur   = this;
  CpBag * first = this;
  CpBag ** prev = &first;

  while (cur) {
    if (cur->choice && !cur->choice->isAliveUpToSolve()) {
      // Remove choice
      (*prev) = cur->next;
      CpBag * junk = cur;
      cur     = cur->next;
      junk->dispose();
    } else if (cur->choice->getChildCount() == 1) {
      Assert(cur->choice->isChoice());
      // Remove choice
      (*prev) = cur->next;
      // return choice
      *wa = cur->choice;
      cur->dispose();
      return first;
    } else if (!(*wa)) {
      *wa  = cur->choice;
      prev = &(cur->next);
      cur  = cur->next;
    } else {
      prev = &(cur->next);
      cur  = cur->next;
    }

  }


  Assert((first ?
          ((*wa) && ((*wa)->getChildCount()>1) )
          : !(*wa)));

  return first;
}
