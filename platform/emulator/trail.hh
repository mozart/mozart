/*
 *  Authors:
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

/*
   Trail class.
*/

#ifndef __TRAILH
#define __TRAILH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "tagged.hh"
#include "stack.hh"

enum TeType {
  Te_Mark     = 1,
  Te_Bind     = 2,
  Te_Variable = 3,
  Te_Cast     = 4,
  Te_Mask     = 7
};

class  Trail: public Stack {
public:
  Trail(): Stack(DEFAULT_TRAIL_SIZE,Stack_WithMalloc) {}
  Trail(int sizeInit): Stack(sizeInit,Stack_WithMalloc) {}

  /*
   * Tests
   *
   */

  TeType getTeType(void) {
    return (TeType) (((int) *(tos-1)) & Te_Mask);
  }

  Bool isEmptyChunk() { 
    return getTeType() == Te_Mark;
  }

  int chunkSize(void) { 
    int ret = 0;

    StackEntry * top = tos-1;

    while (((TeType) ((int) *top)) != Te_Mark) {
      top = top-3;
      ret++;
      Assert(top>=array);  /* there MUST be a mark on the trail! */
    }

    return ret;
  }

  /*
   * Pushing
   *
   */

  void pushMark(void) {
    Stack::push((StackEntry) Te_Mark); 
  }

  void pushBind(TaggedRef *val, TaggedRef old) {
    ensureFree(3);
    Stack::push((StackEntry) val,            NO);
    Stack::push((StackEntry) ToPointer(old), NO);
    Stack::push((StackEntry) Te_Bind,        NO);
  }

  void pushVariable(TaggedRef * var) {
    ensureFree(3);
    Stack::push((StackEntry) Te_Variable, NO);
  }

  void pushCast(TaggedRef var) {
    ensureFree(3);
    Stack::push((StackEntry) Te_Cast, NO);
  }


  /*
   * Popping
   *
   */

  void popMark(void) {
    Assert(isEmptyChunk());
    (void) Stack::pop();
  }

  void popBind(TaggedRef *&val, TaggedRef &old) {
    Assert(getTeType() == Te_Bind);
    (void) Stack::pop();
    old = (TaggedRef)  ToInt32(Stack::pop());
    val = (TaggedRef*) Stack::pop();
  }

  void popVariable(void) {}

  void popCast(void) {}

};

#endif
