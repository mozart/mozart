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

#include "types.hh"
#include "gc.hh"
#include "term.hh"

class TrailEntry {
private:
  TaggedRef *refPtr;
  TaggedRef value;
public:
  void setRefPtr(TaggedRef *refPtr1) { refPtr = refPtr1; }
  void setValue(TaggedRef value1) { value = value1; }
  TaggedRef *getRefPtr() { return refPtr; }
  TaggedRef getValue() { return value; }
};

class RebindTrail {
public:
  void gc();
  RebindTrail (int sizeInit = 5000);
  ~RebindTrail ();
  void pushCouple(TaggedRef *reference, TaggedRef oldValue);
  void popCouple(TaggedRef * &reference, TaggedRef &value);
  inline Bool isEmpty ()
    { return ( (cursor == lowBound) ? OK : NO ); };
private:
  TrailEntry* lowBound;
  TrailEntry* upperBound;
  TrailEntry* cursor;
  /* points always to the next free cell;   */
  int size;
};

/* ***common trail;                         */

class  Trail {
  friend class AM;
private:
  TrailEntry* lowBound;
  TrailEntry* upperBound;
  TrailEntry* cursor;
  TrailEntry* lastMark;
  /* points always to the next free cell;   */
  int size;
  void push(TaggedRef *val, TaggedRef old)
  {
    if (cursor >= upperBound) {
      error ("Trail::push: space in trail (%d words) exhausted",size);
    } else {
      cursor->setRefPtr(val);
      cursor->setValue(old);
      cursor++;
    }
  }


// !!!!! mm2 : x = pop(); push(a) --> x ist geaendert !
  TrailEntry *pop()
  {
    cursor--;
    if (cursor < lowBound)
      error ("Trail::popRef: bottom in trail is reached");
    return ( cursor );
  }

public:
  void gc();
  inline Trail (int sizeInit = 10000);
  inline ~Trail () { delete [] (char *)lowBound; }
  inline void pushRef(TaggedRef* reference,TaggedRef val)
  {
    push(reference,val);
  }
  inline void pushIfVar(TaggedRef A)
  {
    DEREF(A,Aptr,_1);
    if (isAnyVar(A)) { pushRef(Aptr,A); }
  }
  inline TrailEntry *popRef () { return pop(); }
  inline void pushMark();
  inline void popMark();
  inline int chunkSize() { return cursor - lastMark; }
  inline Bool isEmptyChunk() { return lastMark == cursor ? OK : NO; }
};

inline Trail::Trail (int sizeInit)
{
  lowBound = (TrailEntry *) new char [sizeInit * sizeof (TrailEntry)];
  if ( lowBound == NULL ) {
    error ("Trail::Trail: failed");
    return;
  }
  cursor = lowBound;
  lastMark = cursor;
  upperBound = lowBound + sizeInit;
  size = sizeInit;
}

inline void Trail::pushMark() {
  push((TaggedRef *)lastMark,makeTaggedNULL());
  lastMark = cursor;
}

inline void Trail::popMark() {
  lastMark = (TrailEntry *) (pop()->getRefPtr());
}

#endif
