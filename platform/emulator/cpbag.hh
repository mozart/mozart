/*
 * Hydra Project, DFKI Saarbruecken,
 * Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5337
 * Author: schulte
 * Last modified: $Date $ from $Author $
 * Version: $Revision $
 *
 */

#ifndef __CPBAG_H_
#define __CPBAG_H_

#ifdef INTERFACE
#pragma interface
#endif

#include "mem.hh"
#include "types.hh"

class CpBag {
  WaitActor * choice;
  CpBag     * next;
public:
  USEFREELISTMEMORY;

  CpBag(WaitActor *wa, CpBag *ncpb) {
    choice = wa; next = ncpb;
  }
  CpBag(WaitActor *wa) {
    choice = wa; next = (CpBag *) 0;
  }

  void dispose(void) {
    freeListDispose(this, sizeof(CpBag));
  }

  CpBag * get(WaitActor ** wa);
  /* returns as wa a unit choice (if there is any)
     or the first choice otherwise
     if the returned choice is a unit choice, it is removed from the bag
  */

  WaitActor * access(void) {
    Assert(this);
    return this->choice;
  }

  CpBag * getLast(void) {
    Assert(this);
    CpBag * last = this;
    while (last->next) {
      last = last->next;
    }
    return last;
  }

  CpBag * add(WaitActor *wa) {
    return new CpBag(wa,this);
  }

  CpBag * merge(CpBag * mcpb) {
    if (!this) {
      return mcpb;
    } else {
      if (mcpb)
        getLast()->next = mcpb;
      return this;
    }
  }

  CpBag * remove(void) {
    Assert(this);
    CpBag * ret = this->next;;
    dispose();
    return ret;
  }

  CpBag * gc(void);

};


// ------------------------------------------------------------------------

#ifndef OUTLINE
#include "actor.hh"
#endif

#endif // __CPSTACK_H_
