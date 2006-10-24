/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1999
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
#include "susplist.hh"
#include "mem.hh"

/*
 * Queue is implemented as cyclic list.
 *
 * Last points to queue's tail
 * Last's cdr points to queues head
 * Queue empty <=> Last == NULL
 *
 */


class SuspStack {
private:
  SuspList * _head;

public:

  SuspStack(void) {}
  ~SuspStack(void) {}

  void init(void) {
    _head = (SuspList *) NULL;
  }

  void reset(void);

  void gCollect(void);
  void sClone(void);


  /*
   * Fast operations on queues
   *
   */

  Bool isEmpty(void) {
    return _head == (SuspList *) NULL;
  }

  int getSize(void);

  void enqueue(Suspendable * s) {
    _head = new SuspList(s, _head);
  }

  Suspendable * dequeue(void) {
    Assert(!isEmpty());
    Suspendable * s = _head->getSuspendable();
    _head = _head->getNext();
    return s;
  }


  /*
   * Slow operations on queues
   *
   */

  int isIn(Suspendable *);

  void remove(Suspendable *);

  void merge(SuspStack &);

  /*
   * Misc stuff
   *
   */

  OZPRINT;

};

//////////////////////////////////////////////////////////////////////

class SuspQueue {
private:
  SuspList * last;

public:

  /*
   * Management operations
   *
   */
 
  SuspQueue(void) {}
  ~SuspQueue(void) {}

  void init(void) {
    last = 0;
  }

  void reset(void);

  void gCollect(void);
  void sClone(void);


  /*
   * Fast operations on queues
   *
   */

  Bool isEmpty(void) { 
    return last == NULL; 
  }

  int getSize(void);

  void enqueue(Suspendable * s) {
    if (isEmpty()) {
      last = new SuspList(s);
      last->setNext(last);
    } else {
      SuspList * sl = new SuspList(s,last->getNext());
      last->setNext(sl);
      last = sl;
    }
  }

  Suspendable * dequeue(void) {
    Assert(!isEmpty());
    SuspList * head = last->getNext();
    Suspendable * s = head->getSuspendable();

    if (head == last)
      init();
    else
      last->setNext(head->dispose());

    return s;
  }


  /*
   * Slow operations on queues
   *
   */

  int isIn(Suspendable *);

  void remove(Suspendable *);

  void merge(SuspQueue &);

  /*
   * Misc stuff
   *
   */

  OZPRINT;

};

#endif


