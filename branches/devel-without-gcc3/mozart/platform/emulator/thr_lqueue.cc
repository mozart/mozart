/*
 *  Authors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Denys Duchier, 1998
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
#pragma implementation "thr_lqueue.hh"
#endif

#include "thr_lqueue.hh"
#include "thr_class.hh"

int ThreadQueue::getRunnableNumber()
{
  ThreadQueueIterator iter(this);
  int ret=0;
  Thread**ptr;
  while (ptr=iter.getNext()) ret=(*ptr)->getRunnableNumber();
  return ret;
}

void initLinkedQueueFreeList() {
  LinkedQueueBlock::initFreeList();
}
