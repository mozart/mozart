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
 * The variable copying stack: VarFix
 *
 * When during garbage collection or during copying a variable V is
 * encountered that has not been collected so far and V is not direct, 
 * that is, V has been reached by a reference chain, V cannot be copied
 * directly.
 *
 * So, push the location of the reference on VarFix and replace its content
 * by a reference to the old variable, as to shorten the ref-chain.
 *
 * Later V might be reached directly, that fixes V's location. After 
 * collection has finished, VarFix tracks this new location to
 * and fixes the occurence on VarFix.
 *
 */

class VarFix: public FastStack {
public:
  VarFix() : FastStack() {}
  ~VarFix() {}

  void defer(TaggedRef * var, TaggedRef * ref) {
    Assert(var);
    FastStack::push1((StackEntry) ref);
    *ref = makeTaggedRef(var); 
  }

  void gCollectFix(void);
  void sCloneFix(void);

};

extern VarFix vf;


/*
 * The garbage collector uses an explicit recursion stack. The stack
 * items are references where garbage collection must continue.
 *
 * The items are tagged pointers, the tag gives which routine must 
 * continue, whereas the pointer itself says with which entity.
 *
 */

enum TypeOfPtr {
  PTR_LTUPLE         =  0,  // 0000
  PTR_SRECORD        =  1,  // 0001
  PTR_BOARD          =  2,  // 0010
  PTR_CVAR           =  3,  // 0011
  PTR_CONSTTERM      =  4,  // 0100
  PTR_EXTENSION      =  5,  // 0101
  PTR_SUSPLIST       =  6,  // 0110
  PTR_UNUSED2        =  7,  // 0111
  PTR_LOCAL_SUSPLIST =  8,  // 1000
  // The remaining entries are reserved:
  //  the lower three bits encode, how many suspenlists are to
  //  be collected.
};


typedef TaggedRef TypedPtr;

class CacStack: public FastStack {
public:
  CacStack() : FastStack() {}
  ~CacStack() {}
  
  void push(void * ptr, TypeOfPtr type) {
    FastStack::push1((StackEntry) makeTaggedRef2p((TypeOfTerm) type, ptr));
  }

  void pushLocalSuspList(Board * bb, SuspList ** sl, int n) {
    Assert(n<8);
    FastStack::push2((StackEntry) bb, 
		     (StackEntry) 
		     makeTaggedRef2p((TypeOfTerm) (PTR_LOCAL_SUSPLIST | n), 
				     (void *) sl));
  }

  void gCollectRecurse(void);
  void sCloneRecurse(void);

};

extern CacStack cacStack;

