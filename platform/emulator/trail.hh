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

  Trail (int sizeInit = 200): Stack(sizeInit) { lastMark = tos; }

  void pushRef(TaggedRef *val, TaggedRef old)
  {
    ensureFree(2);
    Stack::push((StackEntry) val,NO);
    Stack::push((StackEntry) ToPointer(old),NO);
  }

  void popRef(TaggedRef *&val, TaggedRef &old)
  {
    old = (TaggedRef)  ToInt32(Stack::pop());
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
    lastMark -= (unsigned intlong) Stack::pop();
  }

  int chunkSize()     { return (tos-1-lastMark)/2; }
  Bool isEmptyChunk() { return (lastMark == tos-1); }
  virtual void resize(int newSize);
};




class RebindTrail: public Trail {
private:
  Bool toplevel; /* rebinds must not be trailed on toplevel */
public:
  RebindTrail(int sizeInit = 5000): Trail(sizeInit) {};

  void gc();

  void init(Bool tl) { toplevel = tl; }

  void pushCouple(TaggedRef *reference, TaggedRef oldValue)
  {
    if (!toplevel) Trail::pushRef(reference,oldValue);
  }

  void popCouple(TaggedRef * &reference, TaggedRef &value)
  {
    Trail::popRef(reference,value);
  }
};


#endif
