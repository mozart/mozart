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
    top -= 3;
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
  OzVariable * v = tagged2CVar(*varPtr);

  if (v->isTrailed())
    return;
  
  ensureFree(3);
  Stack::push((StackEntry) varPtr,                 NO);
  Stack::push((StackEntry) oz_var_copyForTrail(v), NO);
  Stack::push((StackEntry) Te_Variable,            NO);

  v->setTrailed();
  
}

void Trail::pushCast(TaggedRef * var) {
  ensureFree(3);
  Stack::push((StackEntry) Te_Cast, NO);
}


void Trail::pushMark(void) {
  // All variables marked as trailed must be unmarked!
  
  StackEntry * top = tos-1;
  
  do {
    switch ((TeType) (int) *top) {
    case Te_Mark:
      goto exit;
    case Te_Variable: {
      TaggedRef * varPtr = (TaggedRef *) *(top-1);
      Assert(isCVar(*varPtr));
      OzVariable * v = tagged2CVar(*varPtr);
      Assert(v->isTrailed());
      v->unsetTrailed();
      break;
    }
    default:
      break;
    }
    top -= 3;
  } while (OK);
    
 exit:
  Stack::push((StackEntry) Te_Mark); 
  
}



/*
 * Popping
 *
 */

void Trail::popBind(TaggedRef *&val, TaggedRef &old) {
  Assert(getTeType() == Te_Bind);
  (void) Stack::pop();
  old = (TaggedRef)  ToInt32(Stack::pop());
  val = (TaggedRef*) Stack::pop();
}

void Trail::popVariable(TaggedRef *&varPtr, OzVariable *&orig) {
  Assert(getTeType() == Te_Variable);
  (void) Stack::pop();
  orig   = (OzVariable *) Stack::pop();
  varPtr = (TaggedRef *)  Stack::pop();
}

void Trail::popCast(void) {
}

void Trail::popMark(void) {
  Assert(isEmptyChunk());
  (void) Stack::pop();

  StackEntry * top = tos-1;
  
  do {
    switch ((TeType) (int) *top) {
    case Te_Mark:
      return;
    case Te_Variable: {
      TaggedRef * varPtr = (TaggedRef *) *(top-1);
      Assert(isCVar(*varPtr));
      OzVariable * v = tagged2CVar(*varPtr);
      Assert(!v->isTrailed());
      v->setTrailed();
      break;
    }
    default:
      break;
    }
    top -= 3;
  } while (OK);
  
}

