/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

// implementation of threads

#if defined(INTERFACE)
#pragma implementation "thr_class.hh"
#endif

#include "thr_class.hh"

#ifdef DEBUG_CHECK

void Thread::printTaskStack(int depth) {
  if (!isDead()) {
    taskStack->printTaskStack(depth);
  } else {
    message("\tEMPTY\n");
    message("\n");
  }
}

#endif

// given a list of terms which used to be variables
// on which the thread was suspended, the thread removes
// itself from the suspension lists of those which are
// still variables
#include "var_base.hh"
void Thread::removeSuspensions(TaggedRef list)
{
  while (list!=AtomNil) {
    Assert(oz_isCons(list));
    TaggedRef car = oz_head(list);
    DEREF(car,ptr,tag);
    if (isVariableTag(tag))
      tagged2Var(car)->removeFromSuspList(this);
    list = oz_tail(list);
  }
}
