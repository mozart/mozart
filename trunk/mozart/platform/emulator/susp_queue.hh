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

#ifndef __SUSP_QUEUE_HH__
#define __SUSP_QUEUE_HH__

#ifdef INTERFACE
#pragma interface
#endif

#include "suspendable.hh"
#include "mem.hh"

#define INC(a)  { (a) = ((a) + 1); if ((a)==maxsize) (a)=0; }

class SuspQueue {
protected:
  int head, tail, size, maxsize;
  Suspendable ** queue;

  void resize();

public:
  USEFREELISTMEMORY;

  /*
   * Management operations
   *
   */
 
  SuspQueue(int n = QUEUEMINSIZE) 
    : head(0), size(0), maxsize(n), tail(n-1) {
    queue = 
      (Suspendable **) freeListMalloc(sizeof(Suspendable *) * n);
  }

  ~SuspQueue(void) {}

  int suggestNewSize(void) {
    return max(min(size * 2,(maxsize + size + 1) >> 1), QUEUEMINSIZE);
  }

  void dispose(void) {
    freeListDispose(queue, maxsize * sizeof(Suspendable *));
    freeListDispose(this,  sizeof(SuspQueue));
  }

  SuspQueue * gc(void);

  /*
   * Fast operations on queues
   *
   */

  Bool isEmpty(void) { 
    return size == 0; 
  }

  int getSize(void) { 
    return (size); 
  }

  void enqueue(Suspendable * s) {
    if (size == maxsize) resize();
    INC(tail);
    queue[tail] = s;
    size++;
  }

  Suspendable * dequeue(void) {
    Assert(!isEmpty());
    Suspendable * s = queue[head];
    INC(head);
    size--;
    return s;
  }


  /*
   * Slow operations on queues
   *
   */

  int isIn(Suspendable *);

  void remove(Suspendable *);

  SuspQueue * merge(SuspQueue *);

  /*
   * Misc stuff
   *
   */

  OZPRINT;

};

#endif


