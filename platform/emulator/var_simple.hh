/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Alfred Spiessens (fsp@info.ucl.ac.be)
 * 
 *  Copyright:
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

#ifndef __SIMPLEVAR__H__
#define __SIMPLEVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"
#include "unify.hh"

/*
 * fred+raph:
 * A SimpleVar object implements a free variable whose value can be
 * demanded or not.
 * A non-demanded, or quiet, variable has the type OZ_VAR_SIMPLE_QUIET.
 * A demanded, or needed, variable has the type OZ_VAR_SIMPLE.
 */

class SimpleVar: public OzVariable {
public:
  // raph: a new SimpleVar is quiet by default
  SimpleVar(Board *bb) : OzVariable(OZ_VAR_SIMPLE_QUIET,bb) {}

  OZ_Return bind(TaggedRef* vPtr, TaggedRef t);
  OZ_Return unify(TaggedRef* vPtr, TaggedRef *tPtr);

  // raph: the variable must be quiet; makes it needed
  OZ_Return becomeNeeded();

  Bool valid(TaggedRef /* val */) { return TRUE; }

  void dispose(void) {
    disposeS();
    oz_freeListDispose(this, sizeof(SimpleVar));
  }

  void printStream(ostream &out,int depth = 10) {
    if (getType() == OZ_VAR_SIMPLE_QUIET) {
      out << "<simple quiet>";
    } else {
      out << "<simple>";
    }
  }
  void printLongStream(ostream &out,int depth = 10,
			int offset = 0) {
    printStream(out,depth); out << endl;
  }
};

#endif /* __SIMPLEVAR__H__ */
