/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#include "cpi.hh"

// check stream for validity, init closed and eostr, init nextGet and nextPut
void OZ_Stream::setFlags(void) 
{
  closed = eostr = FALSE;
  valid = TRUE;
  OZ_Term t = tail;

  DEREF(t, tptr);
  Assert(!oz_isRef(t));
  if (oz_isNil(t)) {
    eostr = closed = TRUE;
    return;
  } else if (oz_isFree(t)) {
    eostr = TRUE;
    return;
  } else if (oz_isLTupleOrRef(t)) {
    return;
  }
  valid = FALSE;
  eostr = closed = TRUE;
}

OZ_Term OZ_Stream::get(void)
{
  if (closed || eostr) {
    return 0;
  } 

  OZ_Term deref_tail = oz_deref(tail);
  OZ_Term r = oz_head(deref_tail);
  tail = makeTaggedRef(tagged2LTuple(deref_tail)->getRefTail());
  setFlags();
  return r;
}

OZ_Term OZ_Stream::put(OZ_Term stream, OZ_Term elem)
{
  OZ_Term tail = OZ_newVariable();
  OZ_Term ret = (OZ_unify(stream, OZ_cons(elem, tail)) == PROCEED) ? tail : 0; // mm_u
  setFlags();
  return ret;
}

OZ_Boolean OZ_Stream::leave(void) 
{
  setFlags();

  while (!eostr) 
    get();

  if (closed || !valid) 
    return FALSE;
  
  OZ_Term t = tail;
  DEREF(t, tailptr);

  oz_var_addSusp(tailptr, Propagator::getRunningPropagator());
  return TRUE;
}

void OZ_Stream::fail(void) 
{
}

// End of File
//-----------------------------------------------------------------------------
