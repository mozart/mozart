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
  USEFREELISTMEMORY;

  SuspList(Thread *t, SuspList * n = NULL) : thr(t), next(n) {
    DebugCheck(ToInt32 (t) == ToInt32 (NULL),
               error ("suspension elem missing."););
  }

  SuspList * getNext(void) {return next;}

  void setNext(SuspList * n) {next = n;}

  Thread *getElem () {
    return ((Thread *) (ToPointer (ToInt32 (thr))));
  }

  SuspList * appendTo(SuspList *);
  SuspList * appendToAndUnlink(SuspList * &);
  SuspList * appendToAndUnlink(SuspList * &, Bool reset_local);

  Bool checkCondition(TaggedRef taggedVar, TaggedRef term);

  ~SuspList (void) {}
  SuspList * dispose(void);
  void disposeList(void);

  TaggedRef DBGmakeList(void);

  int length(void);
  int lengthProp(void);

  SuspList * gc();

  int isContained(SuspList * e) {
    for (SuspList *aux = this; aux; aux = aux->getNext())
      if (aux == e)
        return 1;
    return 0;
  }
  OZPRINT;
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
