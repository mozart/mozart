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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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

#include "types.hh"
#include "mem.hh"

// #include "oz_cpi.hh"

#ifdef OUTLINE
#define inline
#endif

/*
 *  class SuspList;
 *
 */
class SuspList {
private:
  Thread *thr;
  SuspList *next;
public:
  ~SuspList (void); // fake compiler

  USEFREELISTMEMORY;
  SuspList * gc();
  OZPRINT;

  SuspList(Thread *t, SuspList * n = NULL)
    : thr(t), next(n)
  {
    Assert(t);
  }


  SuspList * getNext(void)   { return next; }
  void setNext(SuspList * n) { next = n; }
  Thread *getElem()          { return thr; }

  SuspList * appendToAndUnlink(SuspList * &, Bool reset_local);

  SuspList * dispose(void) {
    SuspList * ret = next;
    freeListDispose(this, sizeof(SuspList));
    return ret;
  }

  void disposeList(void) {
    for (SuspList * l = this; l; l = l->dispose());
  }

  int length(void);
  int lengthProp(void);

}; // SuspList


//-----------------------------------------------------------------------------
// class OrderedSuspList

class OrderedSuspList {
private:
  Thread * t;
  OrderedSuspList * n;
public:
  USEFREELISTMEMORY;

  OrderedSuspList(Thread * thr, OrderedSuspList * l) : t(thr), n(l) {}
  OrderedSuspList * insert(Thread *);
  void print(void);
  OrderedSuspList * gc(void);
  OrderedSuspList * getNext(void) const { return n; }
  Thread * getThread(void) const { return t; }
};

#ifdef OUTLINE
#undef inline
#else
#include "susplist.icc"
#endif


#endif //__SUSPLIST_H__
