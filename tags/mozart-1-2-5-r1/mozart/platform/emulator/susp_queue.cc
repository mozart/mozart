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

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "susp_queue.hh"
#endif

#include "susp_queue.hh"

// SuspStack
void SuspStack::reset(void) {
  if (isEmpty())
    return;

  SuspList * sl = _head;

  while (sl != (SuspList *) NULL) {
    sl = sl->dispose();
  }

  init();
}

int SuspStack::getSize(void) {
  int n = 0;
  SuspList * sl = _head;
  while (sl != (SuspList *) NULL) {
    n += 1;
    sl = sl->getNext();
  }
  return n;
}

Bool SuspStack::isIn(Suspendable * s) {
  SuspList * sl = _head;
  while (sl != (SuspList *) NULL) {
    if (sl->getSuspendable() == s)
      return OK;
    sl = sl->getNext();
  };
  return NO;
}

void SuspStack::remove(Suspendable * s) {
  SuspList * tmp = (SuspList *) NULL;
  SuspList * sl = _head;
  //
  while (sl != (SuspList *) NULL) {
    Suspendable * susp = sl->getSuspendable();
    if (susp == s) {
      sl = sl->dispose();
    } else {
      SuspList * next =  sl->getNext();
      sl->setNext(tmp);
      tmp = sl;
      sl = next;
    }
  }
  //
  while (tmp != (SuspList *) NULL) {
    SuspList * next = tmp->getNext();
    tmp->setNext(_head);
    _head = tmp;
    tmp = next;
  }
}

void SuspStack::merge(SuspStack & sq) {
  // Merge entries from sq to this
  if (sq.isEmpty())
    return;

  if (isEmpty()) {
    _head = sq._head;
  } else {
#ifdef DEBUG_CHECK
    int n1 = getSize(), n2 = sq.getSize();
#endif
    //
    SuspList * last = _head;
    while (last->getNext() != (SuspList *) NULL) {
      last = last->getNext();
    }
    last->setNext(sq._head);
    //
#ifdef DEBUG_CHECK
    Assert(n1 + n2 == getSize());
#endif
  }
  sq._head = (SuspList *) NULL;
};

// SuspQueue

void SuspQueue::reset(void) {
  if (isEmpty())
    return;

  SuspList * sl = last;

  do {
    sl = sl->dispose();
  } while (sl != last);

  init();
}

int SuspQueue::getSize(void) {
  if (isEmpty())
    return 0;

  int n = 0;

  SuspList * sl = last;

  do {
    n++; sl = sl->getNext();
  } while (sl != last);

  return n;
}

Bool SuspQueue::isIn(Suspendable * s) {
  if (isEmpty())
    return NO;

  SuspList * sl = last;

  do {
    if (sl->getSuspendable() == s)
      return OK;
    sl = sl->getNext();
  } while (sl != last);

  return NO;
}

void SuspQueue::remove(Suspendable * s) {
  if (isEmpty())
    return;

  SuspList * sl = last;

  do {
    SuspList * nl = sl->getNext();

    if (nl->getSuspendable() == s) {
      if (nl == sl)
	init();
      else
	sl->setNext(nl->dispose());
      return;
    }

    sl = nl;
  } while (sl != last);

}

void SuspQueue::merge(SuspQueue & sq) {
  // Merge entries from sq to this
  if (sq.isEmpty())
    return;

  if (isEmpty()) {
    last = sq.last;
  } else {
#ifdef DEBUG_CHECK
    int n1 = getSize(), n2 = sq.getSize();
#endif
    SuspList * head = last->getNext();
    last->setNext((sq.last)->getNext());
    (sq.last)->setNext(head);
#ifdef DEBUG_CHECK
    Assert(n1 + n2 == getSize());
#endif
  }

};
