/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
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

OZ_CFun MonitorInPropagator::header = fsp_monitorIn;

OZ_Return MonitorInPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in " << *this);

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

  if (stream.isClosed()) {
    // fsetvar has to become a value
    int known_in = fsetvar->getKnownIn();
    FailOnInvalid(fsetvar->putCard(known_in, known_in));
    vanish = OZ_TRUE;
  } else {
    OZ_Term tail = stream.getTail();
    for (int i = 32 * fset_high; i --; ) {
      if (! _in_sofar.isIn(i) && fsetvar->isIn(i)) {
        _in_sofar += i;
        tail = stream.put(tail, OZ_int(i));
      }
    }
    if (fsetvar->isValue()) {
      if (OZ_unify(tail, OZ_nil()) == FAILED)
        goto failure;
      vanish = OZ_TRUE;
    }
  }

  OZ_DEBUGPRINT("out " << *this);

  stream.leave();
  _stream = stream.getTail();
  fsetvar.leave();
  return vanish ? ENTAILED : SLEEP;

failure:
  OZ_DEBUGPRINT("fail " << *this);

  stream.fail();
  fsetvar.fail();
  return FAILED;
}
