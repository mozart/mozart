/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Konstantin Popov (2000)
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

// fred+raph: An OptVar object models a variable with no suspension.
// In particular, the value of that variable is not demanded.
// When a suspension is added, the OptVar object should be "converted"
// into either a SimpleVar.

#ifndef __OPTVAR__H__
#define __OPTVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"
#include "unify.hh"

//
class OptVar: public OzVariable {
public:
  OptVar(Board *bb) : OzVariable(OZ_VAR_OPT, bb) {}

  OZ_Return bind(TaggedRef* vPtr, TaggedRef t) {
    Assert(suspList == (SuspList *) 0);
    oz_bindVar(this, vPtr, t);
    return (PROCEED);
  }

  // getType(vPtr) == OZ_VAR_OPT
  // getBoard(lvp) != getBoard(rvp) || getType(vPtr) >= getType(tPtr)
  OZ_Return unify(TaggedRef* vPtr, TaggedRef *tPtr) {
    oz_bindVar(this, vPtr, makeTaggedRef(tPtr));
    return (PROCEED);
  }


  OZ_Return valid(TaggedRef /* val */) { return OK; }

  // disposing of opt var"s is done only when its space is gone.
  void dispose(void) {}

  void printStream(ostream &out, int depth = 10) {
    out << "<optimized>";
  }
  void printLongStream(ostream &out, int depth = 10, int offset = 0);
};

#endif /* __SIMPLEVAR__H__ */
