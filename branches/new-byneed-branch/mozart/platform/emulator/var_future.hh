/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Denys Duchier (1998)
 *    Michael Mehl (1998)
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

#ifndef __FUTURE__HH__
#define __FUTURE__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"
#include "value.hh"

class Future: public OzVariable {
private:
  OZ_Term function;
public:
  Future(Board *bb) : OzVariable(OZ_VAR_FUTURE,bb), function(0) {}
  Future(Board *bb,OZ_Term function)
    : OzVariable(OZ_VAR_FUTURE,bb), function(function) {}
  OZ_Return bind(TaggedRef* vPtr,TaggedRef t);
  OZ_Return unify(TaggedRef* vPtr,TaggedRef* tPtr);
  OZ_Return forceBind(TaggedRef* vPtr,TaggedRef v);
  Bool valid(TaggedRef /* val */) {
    return TRUE;
  }
  void gCollectRecurse(void);
  void sCloneRecurse(void);
  OZ_Return addSusp(TaggedRef*, Suspendable *);
  void dispose(void) {
    disposeS();
    DebugCode(function=0);
    oz_freeListDispose(this, sizeof(Future));
  }
  void printStream(ostream &out,int depth = 10);
  void printLongStream(ostream &out,int depth = 10,
				int offset = 0) {
    printStream(out,depth); out << endl;
  }
  OZ_Term inspect();
  OZ_Return kick(TaggedRef *);
  Bool isFailed();
};

inline
OZ_Term oz_newFuture(Board *bb) {
  return makeTaggedRef(newTaggedVar(new Future(bb)));
}

// bind a future, don't care about the variable, e.g. for byNeedFuture
inline
void oz_bindFuture(OZ_Term *vPtr,OZ_Term val)
{
  oz_var_forceBind(tagged2Var(*vPtr),vPtr,val);
}

#endif /* __BYNEED__HH__ */
