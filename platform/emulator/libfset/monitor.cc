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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "monitor.hh"

OZ_C_proc_begin(fsp_monitorIn, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_STREAM);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectStream);

  return pe.impose(new MonitorInPropagator(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_CFunHeader MonitorInPropagator::header = fsp_monitorIn;

OZ_Return MonitorInPropagator::propagate(void)
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
      FailOnInvalid(*fsetvar += e);
      _in_sofar += e;
    } else {
      goto failure;
    }
  }

  // now the other way around:
  // check if stream is closed, i.e., _fsetvar_ becomes determined
  if (stream.isClosed()) {
    // fsetvar has to become a value
    int known_in = fsetvar->getKnownIn();
    FailOnInvalid(fsetvar->putCard(known_in, known_in));
    vanish = OZ_TRUE;
  } else { // if the stream is _not_ closed ...
    OZ_Term tail = stream.getTail();

#ifdef FSET_HIGH
    for (int i = fsethigh32; i --; ) {
      if (! _in_sofar.isIn(i) && fsetvar->isIn(i)) {
        _in_sofar += i;
        tail = stream.put(tail, OZ_int(i));
      }
    }
#else

    // find out what is new
    OZ_FSetValue new_in = fsetvar->getGlbSet() - _in_sofar;
    // append new elements to tail of the stream
    FSetIterator fsi(& new_in);
    for (int e = fsi.resetToMin(); e > -1; e = fsi.getNextLarger())
      tail = stream.put(tail, OZ_int(e));
    // keep track of what is known so far
    _in_sofar = _in_sofar | new_in;
#endif

    if (fsetvar->isValue()) {
      if (OZ_unify(tail, OZ_nil()) == FAILED)
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
