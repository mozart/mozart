/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Per Brand (perbrand@sics.se)
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


#include "stack.hh"
#include "base.hh"
#include "tagged.hh"

/*
 * The variable copying stack: VarFixStack
 *
 * When during garbage collection or during copying a variable V is
 * encountered that has not been collected so far and V is not direct, 
 * that is, V has been reached by a reference chain, V cannot be copied
 * directly.
 *
 * So, push the location of the reference on VarFixStack and replace its content
 * by a reference to the old variable, as to shorten the ref-chain.
 *
 * Later V might be reached directly, that fixes V's location. After 
 * collection has finished, VarFixStack tracks this new location to
 * and fixes the occurence on VarFixStack.
 *
 */

class VarFixStack: public FastStack {
public:
  VarFixStack() : FastStack() {}
  ~VarFixStack() {}

  void defer(TaggedRef * var, TaggedRef * ref) {
    Assert(var);
    FastStack::push1((StackEntry) ref);
    *ref = makeTaggedRef(var); 
  }

  void gCollectFix(void);
  void sCloneFix(void);

};

extern VarFixStack vf;


/*
 * The garbage collector uses an explicit recursion stack. The stack
 * items are references where garbage collection must continue.
 *
 * The items are tagged pointers, the tag gives which routine must 
 * continue, whereas the pointer itself says with which entity.
 *
 */

enum ptrtag_t {
  PTR_LTUPLE         =  0,  // 000
  PTR_SRECORD        =  1,  // 001
  PTR_BOARD          =  2,  // 010
  PTR_SUSPLIST0      =  3,  // 011
  PTR_VAR            =  4,  // 100
  PTR_CONSTTERM      =  5,  // 101
  PTR_EXTENSION      =  6,  // 110
  PTR_SUSPLIST1      =  7,  // 111
};

#define PTR_SUSPLIST PTR_SUSPLIST0
#define PTR_MASK     7

class CacStack: public FastStack {
public:
  CacStack() : FastStack() {}
  ~CacStack() {}
  
  void push(void * ptr, ptrtag_t tag) {
    Assert(isSTAligned(ptr));
    FastStack::push1((StackEntry) (((unsigned int) ptr) | tag));
  }

  void pushSuspList(SuspList ** sl) {
    FastStack::push2((StackEntry) NULL, 
		     (StackEntry) (((unsigned int) sl) | PTR_SUSPLIST));
  }

  void pushLocalSuspList(Board * bb, SuspList ** sl, int n) {
    Assert(isSTAligned(bb) && n<8);
    FastStack::push2((StackEntry) (((unsigned int) bb) | n), 
		     (StackEntry) (((unsigned int) sl) | PTR_SUSPLIST));
  }

  void gCollectRecurse(void);
  void sCloneRecurse(void);

};

extern CacStack cacStack;

//
// isGCMarkedTerm(t) returns true iff
//	t is a marked name, extension, const, var
// the logic is adapted from gcTagged(TaggedRef&,TaggedRef&)
// and simplified according to a suggestion by Christian.
Bool isGCMarkedTerm(OZ_Term t);
