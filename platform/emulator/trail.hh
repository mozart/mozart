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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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

/* ***common trail;                         */

const StackEntry trailMark = (StackEntry) -1l;

class  Trail: public Stack {
public:
  void gc();

  Trail (int sizeInit = 200): Stack(sizeInit,Stack_WithMalloc) { }

  void pushRef(TaggedRef *val, TaggedRef old)
  {
    ensureFree(2);
    Assert(trailMark != val && trailMark != ToPointer(old));
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

  void pushMark() { Stack::push(trailMark); }

  int chunkSize()
  {
    int ret = 0;
    StackEntry *top = tos-1;
    while(*top != trailMark) {
      top = top-2;
      ret++;
      Assert(top>=array);  /* there MUST be a mark on the trail! */
    }
    return ret;
  }
  Bool isEmptyChunk() { return (trailMark == *(tos-1)); }

  void popMark()
  {
    Assert(isEmptyChunk());
    (void)Stack::pop();
  }

};

#endif
