/*
 *  Authors:
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


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "lps.hh"
#endif

#include "lps.hh"
#include "am.hh"
#include "prop_class.hh"
#include "prop_int.hh"

#ifdef OUTLINE
#define inline
#endif

//-----------------------------------------------------------------------------

LocalPropagationStore localPropStore;

void LocalPropagationQueue::resize () {
  int new_maxsize = maxsize * 2;
  queue_t * new_queue = ::new queue_t[new_maxsize];
  int index = size - 1;
  int old_size = size;

  while (size) {
    Propagator * prop = dequeue ();
    new_queue[index].prop = prop;
    index -= 1;
  }

  Assert(index == -1);
  delete queue;
  queue = new_queue;
  head = 0;
  tail = (size = old_size) - 1;
  maxsize = new_maxsize;
}
