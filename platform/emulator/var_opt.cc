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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_opt.hh"
#endif

#include "var_opt.hh"
#include "unify.hh"

/*
OZ_Return OptVar::bind(TaggedRef* vPtr, TaggedRef t)
{
  Assert(suspList == (SuspList *) 0);
  oz_bindVar(this, vPtr, t);
  return (PROCEED);
}

OZ_Return OptVar::unify(TaggedRef* vPtr, TaggedRef *tPtr)
{
//    Assert(suspList == (SuspList *) 0);
//    OzVariable *tv = tagged2Var(*tPtr);
//    if (tv->getType() == OZ_VAR_OPT
//        && oz_isBelow(tv->getBoardInternal(), GETBOARD(this))
//  #ifdef VAR_BIND_NEWER
//        // if both are local, then check heap
//        && (!oz_isLocalVar(this) || heapNewer(tPtr, vPtr))
//  #endif
//        ) {
//      oz_bindVar(tv, tPtr, makeTaggedRef(vPtr));
//    } else {
  oz_bindVar(this, vPtr, makeTaggedRef(tPtr));
//    }
  return (PROCEED);
}
*/

void OptVar::printLongStream(ostream &out, int depth, int offset)
{
  printStream(out, depth); 
#ifdef DEBUG_PRINT
  getBoardInternal()->printStream(out, -1);
#endif
  out << endl;
}
