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

#include "monitor.hh"

OZ_BI_define(fsp_monitorIn, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_STREAM);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectStream);

  return pe.impose(new MonitorInPropagator(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

OZ_BI_define(fsp_monitorOut, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_STREAM);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectStream);

  return pe.impose(new MonitorOutPropagator(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

OZ_Return MonitorPropagator::propagate(OZ_Boolean is_inprop)
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_Stream stream(_stream);
  OZ_FSetVar fsetvar(_fsetvar);
  OZ_Boolean vanish = OZ_FALSE; 

  // find elements imposed from outside to stream and constrain
  // _fsetvar_ appropriately
  while (!stream.isEostr()) {
    OZ_Term elem = stream.get();
    if (OZ_isSmallInt(elem)) {
      int e = OZ_intToC(elem);

      if (is_inprop) {
	FailOnInvalid(*fsetvar += e);
      } else {
	FailOnInvalid(*fsetvar -= e);
      }
      _present_sofar += e;
    } else {
      goto failure;
    }
  }
  // now, we are at the end of teh stream

  // now the other way around: 
  // check if stream is closed, i.e., _fsetvar_ becomes determined
  if (stream.isClosed()) {
    static int max_card = OZ_getFSetSup() + OZ_getFSetInf()+ 1;
    // fsetvar has to become a value
    int known_in = (is_inprop 
		    ? fsetvar->getKnownIn() 
		    : max_card - fsetvar->getKnownNotIn());
    FailOnInvalid(fsetvar->putCard(known_in, known_in));
    vanish = OZ_TRUE;
  } else { // if the stream is _not_ closed ...
    OZ_Term tail = stream.getTail();

    // find out what is new
    OZ_FSetValue new_present = 
      (is_inprop ? fsetvar->getGlbSet() : fsetvar->getNotInSet()) 
      - _present_sofar;

    // append new elements to tail of the stream
    FSetIterator fsi(& new_present);
    for (int e = fsi.resetToMin(); e > -1; e = fsi.getNextLarger())
      tail = stream.put(tail, OZ_int(e));
    // keep track of what is known so far
    _present_sofar = _present_sofar | new_present; 

    if (fsetvar->isValue()) {
      if (OZ_unify(tail, OZ_nil()) == FAILED) // mm_u
	goto failure;
      vanish = OZ_TRUE;
    }
  }
 
  OZ_DEBUGPRINTTHIS("out ");

  stream.leave();
  _stream = stream.getTail();
  fsetvar.leave();
  return vanish ? OZ_ENTAILED : SLEEP;

failure:
  OZ_DEBUGPRINTTHIS("fail ");

  stream.fail();
  fsetvar.fail();
  return FAILED;
}


OZ_PropagatorProfile MonitorInPropagator::profile;
OZ_PropagatorProfile MonitorOutPropagator::profile;

