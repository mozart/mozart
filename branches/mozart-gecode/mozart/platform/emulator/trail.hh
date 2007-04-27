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
#include "stack.hh"


enum TeType {
  Te_Mark        = 0,
  Te_Bind        = 1,
  Te_Variable    = 2,
  Te_GeVariable  = 3
};

class Trail: public Stack {

public:
  Trail(void): Stack(DEFAULT_TRAIL_SIZE, Stack_WithMalloc) {
    Stack::push((StackEntry) Te_Mark);
  }
  

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


  /*
   * Pushing
   *
   */
  void test(void);

  void pushMark(void);

  void pushBind(TaggedRef *);

  void pushVariable(TaggedRef *);

  void pushGeVariable(TaggedRef *);

  /*
   * Popping
   *
   */

  void popMark(void);

  void popBind(TaggedRef *&, TaggedRef &);

  void popVariable(TaggedRef *&, OzVariable *&);
  
  void popGeVariable(TaggedRef *&, TaggedRef &);

  /*
   * Unwinding
   *
   */

  TaggedRef unwind(Board *);

  void unwindFailed(void);

  void unwindEqEq(void);

  /**
     \brief Moves all constraint variables representation from trail 
     to scritp. This function only runs when speculation is not taking
     place.
  */
  TaggedRef unwindGeVar(void);


  /** 
      \brief Test whether the space has speculations. Speculation
      is taking place if:
      1) There are elements different to constraint variables in the trail.
         Those elements must be between top and the next trail mark.
      2) The generic space associated with the board is not stable.
      3) At least one constraint variable has runnable propagators.
      4) At least one of the constraint variables in the trail has a 
         different domain compared to its global representation.
  */
  bool isSpeculating(void);



};

extern Trail trail;


#endif
