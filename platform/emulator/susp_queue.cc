/*
 *  Authors:
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *
 *  Copyright:
 *    Tobias Müller, 1999
 *    Kostja Popow, 1999
 *    Christian Schulte, 1999
 *    Michael Mehl, 1999
 *    Denys Duchier, 1999
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
#pragma implementation "susp_queue.hh"
#endif

#include "susp_queue.hh"


void SuspQueue::resize(void) {
  int new_maxsize = (maxsize * 3) >> 1;
  Suspendable ** new_queue =
    (Suspendable **) heapMalloc(sizeof(Suspendable *) * new_maxsize);

  int h = head;

  for (int i = 0; i < size; i++) {
    new_queue[i] = queue[h];
    INC(h);
  }

  freeListDispose (queue, maxsize * sizeof(Suspendable *));
  queue = new_queue;
  head = 0;
  tail = size - 1;
  maxsize = new_maxsize;
}


Bool SuspQueue::isIn(Suspendable * s) {
  int h = head;

  for (int i = size; i--; ) {
    if (queue[h] == s)
      return OK;
    INC(h);
  }

  return NO;
}

void SuspQueue::remove(Suspendable * s) {
  int h = head;

  for (int i = size; i--; ) {
    if (queue[h] == s) {
      for (int j = i-1; j-- ; ) {
        int last=h;
        INC(h);
        queue[last] = queue[h];
      }
      size--;
      tail = tail-1; if (tail < 0 && size>0) tail = maxsize-1;
      return;
    }
    INC(h);
  }
}

SuspQueue * SuspQueue::merge(SuspQueue * sq) {
  if (!this)
    return sq;

  if (!sq)
    return this;

  while (!isEmpty())
    sq->enqueue(dequeue());

  return sq;

};
