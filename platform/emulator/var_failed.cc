/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    derived from var_future.cc by
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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_failed.hh"
#endif

#include "var_failed.hh"
#include "builtins.hh"
#include "unify.hh"

OZ_Return Failed::bind(TaggedRef *vPtr, TaggedRef t)
{
  // simply raise the exception in the current thread
  return OZ_raiseDebug(exception);
}

OZ_Return Failed::unify(TaggedRef *vPtr, TaggedRef *tPtr)
{
  return bind(vPtr, makeTaggedRef(tPtr));
}

OZ_Return Failed::forceBind(TaggedRef *vPtr, TaggedRef t)
{
  if (*vPtr != oz_deref(t))
    oz_bindVar(this,vPtr,t);
  return PROCEED;
}

OZ_Return Failed::addSusp(TaggedRef *tPtr, Suspendable * susp)
{
  // simply raise the exception in the current thread
  return OZ_raiseDebug(exception);
}

void Failed::printStream(ostream &out,int depth)
{
  out << "<failed value: ";
  oz_printStream(exception,out,depth-1);
  out << ">";
}


/*
 * Builtins
 */

OZ_BI_define(BIfailedValue,1,1)
{
  OZ_Term f = oz_newFailed(oz_currentBoard(), OZ_in(0));
  OZ_RETURN(f);
} OZ_BI_end
