/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
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

#ifndef __SUSPLIST_H__
#define __SUSPLIST_H__

#ifdef INTERFACE
#pragma interface
#endif

#include "suspendable.hh"
#include "prop_class.hh"
#include "thr_class.hh"

#ifdef OUTLINE
#define inline
#endif

/*
 *  class SuspList;
 *
 */
class SuspList {
private:
  Suspendable * _susp;
  SuspList    * _next;

  static SuspList * _gc_sentinel;

public:
  USEFREELISTMEMORY;
  NO_DEFAULT_CONSTRUCTORS(SuspList);

  SuspList(Suspendable * s, SuspList * n)
    : _susp(s), _next(n) {}

  SuspList(Suspendable * s)
    : _susp(s) {}

  static void init(void);


  SuspList * getNext(void)   { 
    return _next; 
  }
  SuspList ** getNextRef(void)   { 
    return &_next; 
  }
  void setNext(SuspList * n) { 
    _next = n; 
  }
  Suspendable * getSuspendable(void) { 
    return _susp; 
  }

  Bool isIn(Suspendable * s) {
    for (SuspList * sl = this; sl; sl = sl->getNext())
      if (s == sl->getSuspendable())
	return OK;
    return NO;
  }
    
  SuspList * appendToAndUnlink(SuspList * &, Bool reset_local);

  SuspList * dispose(void) {
    SuspList * ret = _next;
    oz_freeListDispose(this, sizeof(SuspList));
    return ret;
  }

  Bool hasSuspAt(Board *);

  int length(void);
  int lengthProp(void);

  SuspList * gCollectRecurse(SuspList **);
  SuspList * gCollectLocalRecurse(Board *);

  SuspList * sCloneRecurse(SuspList **);
  SuspList * sCloneLocalRecurse(Board *);

  OZPRINTLONG;

  SuspList * dropPropagator(Propagator * prop);
}; // SuspList


//-----------------------------------------------------------------------------
// class OrderedSuspList

class OrderedSuspList {
private:
  Propagator * _p;
  OrderedSuspList * _n;
public:
  USEFREELISTMEMORY;
  
  OrderedSuspList(Propagator * p, OrderedSuspList * l) :  _p(p), _n(l) {}
  OrderedSuspList * insert(Propagator *);
  OrderedSuspList * merge(OrderedSuspList *);
  OZPRINT;
  OrderedSuspList * getNext(void) const { return _n; }
  Propagator * getPropagator(void) const { return _p; }
};

#ifdef OUTLINE
#undef inline
#else
#include "susplist.icc"
#endif


#endif //__SUSPLIST_H__
