/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "bool.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_conj, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new ConjunctionPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return ConjunctionPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  FailOnEmpty(x->constrainBool());
  FailOnEmpty(y->constrainBool());
  FailOnEmpty(z->constrainBool());

  if (*x == fd_singl)
    if (x->getSingleElem() == 0) {
      FailOnEmpty(*z &= 0);
      OZ_DEBUGPRINTTHIS("out: x=0 ");
      return P.vanish();
    } else {
      OZ_DEBUGPRINTTHIS("out: x=1 ");
      P.vanish();
      return replaceBy(reg_y, reg_z);
    }

  if (*y == fd_singl)
    if (y->getSingleElem() == 0) {
      FailOnEmpty(*z &= 0);
      OZ_DEBUGPRINTTHIS("out: y=0 ");
      return P.vanish();
    } else {
      OZ_DEBUGPRINTTHIS("out: y=1 ");
      P.vanish();
      return replaceBy(reg_x, reg_z);
    }

  if (*z == fd_singl && z->getSingleElem() == 1) {
    FailOnEmpty(*x &= 1);
    FailOnEmpty(*y &= 1);
    OZ_DEBUGPRINTTHIS("out: z=1 ");
    return P.vanish();
  }

  if (OZ_isEqualVars(reg_x, reg_y)) {
    OZ_DEBUGPRINTTHIS("out: x=y ");
    P.vanish();
    return replaceBy(reg_x, reg_z);
  }

  OZ_DEBUGPRINTTHIS("out: ");
  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_disj, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new DisjunctionPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return DisjunctionPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  FailOnEmpty(x->constrainBool());
  FailOnEmpty(y->constrainBool());
  FailOnEmpty(z->constrainBool());

  if (*x == fd_singl)
    if (x->getSingleElem() == 1) {
      FailOnEmpty(*z &= 1);
      return P.vanish();
    } else {
      P.vanish();
      return replaceBy(reg_y, reg_z);
    }

  if (*y == fd_singl)
    if (y->getSingleElem() == 1) {
      FailOnEmpty(*z &= 1);
      return P.vanish();
    } else {
      P.vanish();
      return replaceBy(reg_x, reg_z);
    }

  if (*z == fd_singl && z->getSingleElem() == 0) {
    FailOnEmpty(*x &= 0);
    FailOnEmpty(*y &= 0);
    return P.vanish();
  }

  if (OZ_isEqualVars(reg_x, reg_y)) {
    P.vanish();
    return replaceBy(reg_x, reg_z);
  }

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_exor, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new XDisjunctionPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return XDisjunctionPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  FailOnEmpty(x->constrainBool());
  FailOnEmpty(y->constrainBool());
  FailOnEmpty(z->constrainBool());

  if (*x == fd_singl && *y == fd_singl) {
    FailOnEmpty(*z &= ((x->getSingleElem() == y->getSingleElem()) ? 0 : 1));
    return P.vanish();
  }

  if (*z == fd_singl) {
    P.vanish();
    if (z->getSingleElem() == 1) {
      return replaceBy(new NegationPropagator(reg_x, reg_y));
    } else {
      return replaceBy(reg_x, reg_y);
    }
  }

  if (OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*z &= 0);
    return P.vanish();
  }

  if (OZ_isEqualVars(reg_x, reg_z)) {
    FailOnEmpty(*y &= 0);
    return P.vanish();

  }
  if (OZ_isEqualVars(reg_y, reg_z)) {
    FailOnEmpty(*x &= 0);
    return P.vanish();
  }

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_impl, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new ImplicationPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return ImplicationPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  FailOnEmpty(x->constrainBool());
  FailOnEmpty(y->constrainBool());
  FailOnEmpty(z->constrainBool());

  if (*x == fd_singl)
    if (x->getSingleElem() == 0) {
      FailOnEmpty(*z &= 1);
      return P.vanish();
    } else {
      P.vanish();
      return replaceBy(reg_y, reg_z);
    }

  if (*y == fd_singl) {
    if (y->getSingleElem() == 1) {
      FailOnEmpty(*z &= 1);
      return P.vanish();
    } else {
      P.vanish();
      return replaceBy(new NegationPropagator(reg_x, reg_z));
    }
  }
  if (*z == fd_singl && z->getSingleElem() == 0) {
    FailOnEmpty(*x &= 1);
    FailOnEmpty(*y &= 0);
    return P.vanish();
  }

  if (OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*z &= 1);
    return P.vanish();
  }

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_equi, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new EquivalencePropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end


OZ_Return EquivalencePropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  FailOnEmpty(x->constrainBool());
  FailOnEmpty(y->constrainBool());
  FailOnEmpty(z->constrainBool());

  if (*x == fd_singl && *y == fd_singl) {
    FailOnEmpty(*z &= ((x->getSingleElem() == y->getSingleElem()) ? 1 : 0));
    return P.vanish();
  }

  if (*z == fd_singl) {
    P.vanish();
    if (z->getSingleElem() == 0) {
      return replaceBy(new NegationPropagator(reg_x, reg_y));
    } else {
      return replaceBy(reg_x, reg_y);
    }
  }
  if (OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*z &= 1);
    return P.vanish();
  }

  if (OZ_isEqualVars(reg_x, reg_z)) {
    FailOnEmpty(*y &= 1);
    return P.vanish();
  }

  if (OZ_isEqualVars(reg_y, reg_z)) {
    FailOnEmpty(*x &= 1);
    return P.vanish();
  }

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_nega, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new NegationPropagator(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_Return NegationPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  FailOnEmpty(x->constrainBool());
  FailOnEmpty(y->constrainBool());

  if (*x == fd_singl) {
    FailOnEmpty(*y &= (1 - x->getSingleElem()));
    return P.vanish();
  }

  if (*y == fd_singl) {
    FailOnEmpty(*x &= (1 - y->getSingleElem()));
    return P.vanish();
  }

  if (OZ_isEqualVars(reg_x, reg_y)) goto failure;

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// static member


OZ_CFun ConjunctionPropagator::spawner = fdp_conj;
OZ_CFun DisjunctionPropagator::spawner = fdp_disj;
OZ_CFun XDisjunctionPropagator::spawner = fdp_exor;
OZ_CFun ImplicationPropagator::spawner = fdp_impl;
OZ_CFun EquivalencePropagator::spawner = fdp_equi;
OZ_CFun NegationPropagator::spawner = fdp_nega;
