/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Copyright:
 *    Ralf Scheidhauer 1999
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

#if defined(INTERFACE)
#pragma implementation "tagged.hh"
#endif

#include "tagged.hh"

//
void *tagged2Addr(TaggedRef t)
{
  TaggedRef *tp = 0;
 repeat:
  switch(tagged2ltag(t)) {
  case LTAG_REF00:
  case LTAG_REF01:
  case LTAG_REF10:
  case LTAG_REF11:
    tp = tagged2Ref(t);
    t = *tp;
    goto repeat;

  case LTAG_VAR0:
  case LTAG_VAR1:
    return (tp);

  case LTAG_CONST0:
  case LTAG_CONST1:
    return (tagged2Const(t));

  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    return (tagged2LTuple(t));

  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    return (tagged2SRecord(t));

  case LTAG_LITERAL:
    return (tagged2Literal(t));

  case LTAG_MARK0:
  case LTAG_MARK1:
  case LTAG_SMALLINT:
    return (0);			// per definition;
  }
  Assert(0);
  return (0);
}

