/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

  exported variables/classes: class Trail;

  exported procedures: no

  ------------------------------------------------------------------------

  internal static variables: no

  internal procedures: no

  ------------------------------------------------------------------------
*/

/*
   Trail class.
*/

#ifndef __TRAILH
#define __TRAILH

#ifdef __GNUC__
#pragma interface
#endif

#include "stack.hh"
#include "tagged.hh"

/* ***common trail;                         */

class  Trail: public Stack {
private:
  StackEntry* lastMark;

public:
  void gc();

  Trail (int sizeInit = 10): Stack(sizeInit) { lastMark = tos; }

  void pushRef(TaggedRef *val, TaggedRef old)
  {
    ensureFree(2);
    Stack::push((StackEntry) val,NO);
    Stack::push((StackEntry) old,NO);
  }

  void popRef(TaggedRef *&val, TaggedRef &old)
  {
    old = (TaggedRef)  Stack::pop();
    val = (TaggedRef*) Stack::pop();
  }

  void pushIfVar(TaggedRef A)
  {
    DEREF(A,Aptr,_1);
    if (isAnyVar(A)) { pushRef(Aptr,A); }
  }

  void pushMark()
  {
    Stack::push((StackEntry)(tos-lastMark));
    lastMark = tos-1;
  }

  void popMark() {
    Assert(lastMark == tos-1);
    lastMark -= (unsigned long) Stack::pop();
  }

  int chunkSize()     { return (tos-1-lastMark)/2; }
  Bool isEmptyChunk() { return lastMark == tos-1 ? OK : NO; }
  virtual void resize(int newSize);
};




class RebindTrail: public Trail {
public:
  RebindTrail(int sizeInit = 5000): Trail(sizeInit) {};

  void gc();

  void pushCouple(TaggedRef *reference, TaggedRef oldValue)
  {
    Trail::pushRef(reference,oldValue);
  }

  void popCouple(TaggedRef * &reference, TaggedRef &value)
  {
    Trail::popRef(reference,value);
  }
};


#endif
