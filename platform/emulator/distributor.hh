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

#ifndef __DISTRIBUTOR_H_
#define __DISTRIBUTOR_H_

#ifdef INTERFACE
#pragma interface
#endif

#include "mem.hh"

class Distributor {
public:
  USEFREELISTMEMORY;

  virtual int isAlive(void) = 0;
  virtual int getAlternatives(void) = 0;
  virtual int commit(Board *, int, int) = 0;

  virtual Distributor * gc(void) = 0;

  virtual void dispose(void) = 0;
};

class DistBag {
  Distributor * dist;
  DistBag     * next;
public:
  USEFREELISTMEMORY;

  DistBag(Distributor *d, DistBag *b) {
    dist = d; next = b;
  }
  DistBag(Distributor *d) {
    dist = d; next = (DistBag *) 0;
  }

  Distributor * getDist(void) {
    return dist;
  }
  DistBag * getNext(void) {
    return next;
  }
  void setNext(DistBag * d) {
    next = d;
  }

  DistBag * clean(void);

  void dispose(void) {
    freeListDispose(this, sizeof(DistBag));
  }

  DistBag * getLast(void) {
    Assert(this);
    DistBag * last = this;
    while (last->next) {
      last = last->next;
    }
    return last;
  }

  DistBag * add(Distributor *d) {
    return new DistBag(d,this);
  }

  DistBag * merge(DistBag * b) {
    if (!this) {
      return b;
    } else {
      if (b)
        getLast()->next = b;
      return this;
    }
  }

  DistBag * remove(void) {
    Assert(this);
    DistBag * ret = this->next;
    dispose();
    return ret;
  }

  DistBag * gc(void);

};


#endif
