/*
 *  Authors:
 *    Fred Spiessens (fsp@info.ucl.ac.be)
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Fred Spiessens and Raphael Collet (2003)
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
 
// fred+raph: This class is an adaptation of SimpleVar.
// A QuietVar object models a variable with suspensions that do not
// demand the value of it.
// Typical suspensions are by-need suspensions and WaitQuiet.
// When a "demanding" suspension is added, the suspension list is
// released, then the QuietVar object is converted into a SimpleVar, and
// the new suspension added.

#ifndef __QUIETVAR__H__
#define __QUIETVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"

class QuietVar: public OzVariable {
public:
  QuietVar(Board *bb) : OzVariable(OZ_VAR_QUIET,bb) {}

  OZ_Return bind(TaggedRef* vPtr, TaggedRef t);
  OZ_Return unify(TaggedRef* vPtr, TaggedRef *tPtr);

  OZ_Return becomeNeeded();

  OZ_Return valid(TaggedRef /* val */) { return OK; }

  OZ_Return addSusp(TaggedRef*, Suspendable *);
  void dispose(void) {
    disposeS();
    oz_freeListDispose(this, sizeof(QuietVar));
  }

  void printStream(ostream &out,int depth = 10) {
    out << "<quiet>";
  }
  void printLongStream(ostream &out,int depth = 10,
			int offset = 0) {
    printStream(out,depth); out << endl;
  }
};

inline
OZ_Term oz_newQuiet(Board *bb) {
  return makeTaggedRef(newTaggedVar(new QuietVar(bb)));
}

#endif /* __QUIETVAR__H__ */
