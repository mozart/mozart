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

#if defined(INTERFACE)
#pragma implementation "trail.hh"
#endif

#include "trail.hh"
#include "var_base.hh"


/*
 * Tests
 *
 */

int Trail::chunkSize(void) {
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

void Trail::pushBind(TaggedRef *varPtr) {
  ensureFree(3);
  Stack::push((StackEntry) varPtr,             NO);
  Stack::push((StackEntry) ToPointer(*varPtr), NO);
  Stack::push((StackEntry) Te_Bind,            NO);
}

void Trail::pushVariable(TaggedRef * varPtr) {
  // Make copy of variable
  Assert(isCVar(*varPtr));

  OzVariable * ov = tagged2CVar(*varPtr);

  Assert((ov->getType() == OZ_VAR_FD) ||
         (ov->getType() == OZ_VAR_OF) ||
         (ov->getType() == OZ_VAR_FS) ||
         (ov->getType() == OZ_VAR_CT));


  ensureFree(3);
  Stack::push((StackEntry) Te_Variable, NO);
}

void Trail::pushCast(TaggedRef * var) {
  ensureFree(3);
  Stack::push((StackEntry) Te_Cast, NO);
}


/*
 * Popping
 *
 */

void Trail::popVariable(void) {
}

void Trail::popCast(void) {
}
