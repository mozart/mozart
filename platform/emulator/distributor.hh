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

#include "base.hh"
#include "mem.hh"
#include "unify.hh"
#include "thr_int.hh"
#include "value.hh"
#include "builtins.hh"

class Distributor {
public:
  USEFREELISTMEMORY;

  virtual int getAlternatives(void);

  virtual void commit(int, int);

  void dispose(void) {
    freeListDispose(this, sizeOf());
  }

  virtual int sizeOf(void);

  virtual Distributor * gc(void);

};

inline
void telleq(Board * bb, const TaggedRef a, const TaggedRef b) {
  RefsArray args = allocateRefsArray(2, NO);
  args[0] = a;
  args[1] = b;

  Thread * t = oz_newThreadInject(bb);
  t->pushCall(BI_Unify,args,2);
}

class BaseDistributor : virtual Distributor {
  int offset, num;
  TaggedRef var;
public:

  BaseDistributor(const int n, const TaggedRef x) {
    offset = 0; num = n; var = x;
  }

  int getAlternatives(void) {
    return num;
  }

  void commit(Board * bb, const int l, const int r) {
    int tl = max(l,offset+1);
    int tr = min(r,offset+num);
    offset = tl-1;
    num    = tr-tl+1;

    if (num == 1) {
      num = 0;

      telleq(bb,var,makeTaggedSmallInt(offset + 1));
    }

  }

  int sizeOf(void) {
    return sizeof(BaseDistributor);
  }

  Distributor * gc(void);
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

  Distributor * access(void) {
    return dist;
  }

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
