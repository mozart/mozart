/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Kostja Popov, 1999
 *    Christian Schulte, 1999
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

#ifndef __TRAILH
#define __TRAILH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "tagged.hh"
#include "stack.hh"
#include "mem.hh"


class Equation {
public:
  TaggedRef left, right;
};


class Script {
public:
  Equation * eqs;
  int      size;

public:

  Script(void) : size(0) {}
  ~Script() {}

  void gc(void);

  void allocate(int n) {
    Assert(n > 0);
    size = n;
    eqs  = (Equation *) freeListMalloc(n * sizeof(Equation)); 
  }

  void setEmpty(void) {
    size = 0;
    Assert(!(eqs = 0));
  }

  void dispose(void) {
    if (size > 0) {
      freeListDispose(eqs, size * sizeof(Equation));
      size = 0;
    }
    Assert(!(eqs = 0));
  }

  int getSize(void) { 
    return size; 
  }

  Equation & operator[] (int i) { 
    return eqs[i]; 
  }

  OZPRINT;

};



enum TeType {
  Te_Mark     = 0,
  Te_Bind     = 1,
  Te_Variable = 2,
  Te_Cast     = 3
};

class Trail: public Stack {

public:
  Trail(void): Stack(DEFAULT_TRAIL_SIZE, Stack_WithMalloc) {
    Stack::push((StackEntry) Te_Mark);
  }
  
  void init(void);


  /*
   * Tests
   *
   */

  TeType getTeType(void) {
    return (TeType) (int) Stack::topElem();
  }

  Bool isEmptyChunk() { 
    return getTeType() == Te_Mark;
  }

  int chunkSize(void);


  /*
   * Pushing
   *
   */

  void pushMark(void);

  void pushBind(TaggedRef *);

  void pushVariable(TaggedRef *);

  void pushCast(TaggedRef *);


  /*
   * Popping
   *
   */

  void popMark(void);

  void popBind(TaggedRef *&val, TaggedRef &old);

  void popVariable(TaggedRef *&varPtr, OzVariable *&orig);
  
  void popCast(void);


  /*
   * Unwinding
   *
   */

  void unwind(void);

  void unwindFailed(void);

  void unwindEqEq(void);

};

#endif
