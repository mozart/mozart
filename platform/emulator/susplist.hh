/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl,tmueller,popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __SUSPLIST_H__
#define __SUSPLIST_H__

#ifdef INTERFACE
#pragma interface
#endif

#include "oz_cpi.hh"

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


  TaggedRef DBGmakeList(void);

  int length(void);
  int lengthProp(void);

}; // SuspList


SuspList * installPropagators(SuspList * local_list, SuspList * glob_list,
                              Board * glob_home);


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
