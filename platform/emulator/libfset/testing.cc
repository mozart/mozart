/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "testing.hh"


OZ_C_proc_begin(fsp_isIn, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_INT "," OZ_EM_FSET "," OZ_EM_TNAME);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectInt);
  OZ_EXPECT(pe, 1, expectFSetVarAny);

  if (!OZ_isVariable(OZ_args[2]) &&
      !(OZ_isTrue(OZ_args[2]) || OZ_isFalse(OZ_args[2]))) {
    pe.fail();
    return OZ_typeError(expectedType, 2, "");
  }

  return pe.impose(new IsInPropagator(OZ_args[1],
                                      OZ_args[0],
                                      OZ_args[2]));
}
OZ_C_proc_end

OZ_CFun IsInPropagator::spawner = fsp_isIn;

OZ_Return IsInPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  OZ_FSetVar v(_v);

  if (v->isIn(_i)) {
    if (OZ_unify(_b, OZ_true()) == OZ_FAILED)
      goto failure;
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }
  if (v->isNotIn(_i)) {
    if (OZ_unify(_b, OZ_false())  == OZ_FAILED)
      goto failure;
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }
  OZ_DEBUGPRINTTHIS("sleep: ");
  v.leave();
  return SLEEP;

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  v.fail();
  return FAILED;
}
