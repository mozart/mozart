/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
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
#pragma implementation "distributor.hh"
#endif

#include "distributor.hh"
#include "tagged.hh"
#include "thr_int.hh"
#include "thr_class.hh"
#include "value.hh"

void Distributor::dispose(void) {
  freeListDispose(this, sizeOf());
}

// PERFORMANCE PROBLEMS
DistBag * DistBag::clean(void) {

  if (!this)
    return this;
  
  if (getDist()->isAlive()) {
    setNext(getNext()->clean());
    return this;
  } else {
    return getNext()->clean();
  }
  
}

inline
void telleq(Board * bb, const TaggedRef a, const TaggedRef b) {
  RefsArray args = allocateRefsArray(2, NO);
  args[0] = a;
  args[1] = b;

  Thread * t = oz_newThreadInject(bb);
  t->pushCall(BI_Unify,args,2);
}


BaseDistributor::BaseDistributor(Board * bb, const int n) {
  offset = 0; 
  num    = n;
  var    = oz_newVar(bb);
}


int BaseDistributor::commit(Board * bb, int l, int r) {
  if (l > num+1) {
    num = 0;
  } else {
    offset += l-1;
    num     = min(num,min(r,num+1)-l+1);
    
    if (num == 1) {
      num = 0;
      
      telleq(bb,var,makeTaggedSmallInt(offset + 1));
      return 1;
    }
  }
  return num;
}
  
