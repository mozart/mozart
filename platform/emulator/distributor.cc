/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1997
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
#pragma implementation "distributor.hh"
#endif

#include "distributor.hh"


DistBag * DistBag::get(WaitActor ** wa) {
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
