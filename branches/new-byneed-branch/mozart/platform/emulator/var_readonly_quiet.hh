/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Raphael Collet <raph@info.ucl.ac.be>
 *    Alfred Spiessens <fsp@info.ucl.ac.be>
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

#ifndef __READONLY_QUIET__HH__
#define __READONLY_QUIET__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"
#include "value.hh"

class QuietReadOnly: public OzVariable {
public:
  QuietReadOnly(Board *bb) : OzVariable(OZ_VAR_READONLY_QUIET,bb) {}

  OZ_Return bind(TaggedRef* vPtr,TaggedRef t);
  OZ_Return unify(TaggedRef* vPtr,TaggedRef* tPtr);
  OZ_Return forceBind(TaggedRef* vPtr,TaggedRef v);
  OZ_Return valid(TaggedRef /* val */) {
    return TRUE;
  }

  OZ_Return becomeNeeded();

  // void gCollectRecurse(void);
  // void sCloneRecurse(void);
  OZ_Return addSusp(TaggedRef*, Suspendable *);
  void dispose(void) {
    disposeS();
    // DebugCode(function=0);
    oz_freeListDispose(this, sizeof(QuietReadOnly));
  }
  void printStream(ostream &out,int depth = 10);
  void printLongStream(ostream &out,int depth = 10,
				int offset = 0) {
    printStream(out,depth); out << endl;
  }
  // OZ_Term inspect();
  // OZ_Return kick(TaggedRef *);
  // Bool isFailed();
};

inline
OZ_Term oz_newReadOnly(Board *bb) {
  return makeTaggedRef(newTaggedVar(new QuietReadOnly(bb)));
}

#endif /* __READONLY_QUIET__HH__ */
