/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Alfred Spiessens (fsp@info.ucl.ac.be)
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

#ifndef __FAILED__HH__
#define __FAILED__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"
#include "value.hh"

/*
 * fred+raph:
 * A Failed object implements a failed value, i.e., a value that
 * encapsulates an exception.  The exception is raised in any thread
 * that synchronizes on that value.
 *
 * It is implemented as a variable object so as to catch binding
 * attempts and synchronization on it.
 */

class Failed: public OzVariable {
private:
  // encapsulated exception
  OZ_Term exception;

public:
  Failed(Board *bb, OZ_Term exc)
    : OzVariable(OZ_VAR_FAILED,bb), exception(exc) {}

  OZ_Return bind(TaggedRef* vPtr,TaggedRef t);
  OZ_Return unify(TaggedRef* vPtr,TaggedRef* tPtr);
  OZ_Return forceBind(TaggedRef* vPtr,TaggedRef v);

  // raph: use this method for adding demanding suspensions only
  OZ_Return addSusp(TaggedRef*, Suspendable *);

  Bool valid(TaggedRef /* val */) {
    return TRUE;
  }
  void gCollectRecurse(void);
  void sCloneRecurse(void);
  void dispose(void) {
    disposeS();
    DebugCode(exception=0);   // ?
    oz_freeListDispose(this, sizeof(Failed));
  }

  void printStream(ostream &out,int depth = 10);
  void printLongStream(ostream &out, int depth = 10, int offset = 0) {
    printStream(out,depth); out << endl;
  }
};


inline
OZ_Term oz_newFailed(Board *bb, OZ_Term exc) {
  return makeTaggedRef(newTaggedVar(new Failed(bb, exc)));
}

#endif /* __FAILED__HH__ */
