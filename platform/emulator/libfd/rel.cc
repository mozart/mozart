/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "rel.hh"
#include "auxcomp.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_notEqOff, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarSingl);
  OZ_EXPECT(pe, 1, expectIntVarSingl);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.impose(new NotEqOffPropagator(OZ_args[0], OZ_args[1],
                                          OZ_intToC(OZ_args[2])));
}
OZ_C_proc_end

OZ_Return NotEqOffPropagator::propagate(void)
{
  int &c = reg_c;

  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  if (*x == fd_singl) {
    FailOnEmpty(*y -= (x->getSingleElem() - c));
    return P.vanish();
  }

  if (*y == fd_singl) {
    FailOnEmpty(*x -= (y->getSingleElem() + c));
    return P.vanish();
  }

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_lessEqOff, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectIntVarMinMax);
  OZ_EXPECT(pe, 2, expectInt);

  return pe.impose(new LessEqOffPropagator(OZ_args[0], OZ_args[1],
                                           OZ_intToC(OZ_args[2])));
}
OZ_C_proc_end

OZ_Return LessEqOffPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  int &c = reg_c;

  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_y))
    return (0 <= c) ? PROCEED : FAILED;

  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  FailOnEmpty(*x <= (y->getMaxElem() + c));
  FailOnEmpty(*y >= (x->getMinElem() - c));

  if (x->getMaxElem() <= y->getMinElem() + c) return P.vanish();
  if (x->getMinElem() > y->getMaxElem() + c) goto failure;

  OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_minimum, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new MinimumPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return MinimumPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(reg_x, reg_z),
                  replaceBy(new LessEqOffPropagator(reg_x, reg_y, 0)),
                  replaceBy(new LessEqOffPropagator(reg_y, reg_x, 0)));

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
  int lower_min = min(x->getMinElem(), y->getMinElem());
  int upper_min = min(x->getMaxElem(), y->getMaxElem());

  FailOnEmpty(*z >= lower_min);
  FailOnEmpty(*z <= upper_min);
  FailOnEmpty(*x >= z->getMinElem());
  FailOnEmpty(*y >= z->getMinElem());

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_maximum, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new MaximumPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return MaximumPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(reg_x, reg_z),
                  replaceBy(new LessEqOffPropagator(reg_y, reg_x, 0)),
                  replaceBy(new LessEqOffPropagator(reg_x, reg_y, 0)));

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
  int lower_max = max(x->getMinElem(), y->getMinElem());
  int upper_max = max(x->getMaxElem(), y->getMaxElem());

  FailOnEmpty(*z >= lower_max);
  FailOnEmpty(*z <= upper_max);
  FailOnEmpty(*x <= z->getMaxElem());
  FailOnEmpty(*y <= z->getMaxElem());

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_inter, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarAny, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new IntersectionPropagator(OZ_args[0], OZ_args[1],
                                              OZ_args[2]));
}
OZ_C_proc_end

OZ_Return IntersectionPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(new SubSetPropagator(reg_z, reg_x)),
                  replaceBy(new SubSetPropagator(reg_x, reg_y)),
                  replaceBy(new SubSetPropagator(reg_y, reg_x)));

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  FailOnEmpty(*z &= (*x & *y));

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_union, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarAny, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new UnionPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return UnionPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(new SubSetPropagator(reg_z, reg_x)),
                  PROCEED, PROCEED);

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  FailOnEmpty(*z &= (*x | *y)); // TMUELLER

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_subset, 2)
{
  OZ_warning("This foreign function must never be called.");
  return FAILED;
}
OZ_C_proc_end

OZ_Return SubSetPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  if (OZ_isEqualVars(reg_x, reg_y)) {
    return P.vanish();
  } else {
    FailOnEmpty(*x &= *y);
  }

  return P.leave();
failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_distinct, 1)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarSingl);

  return pe.impose(new DistinctPropagator(OZ_args[0]));
}
OZ_C_proc_end

OZ_Return DistinctPropagator::propagate(void)
{
  if (reg_l_sz == 0) return PROCEED;

  int &sz  = reg_l_sz;

  DECL_DYN_ARRAY(OZ_FDIntVar, l, sz);
  PropagatorController_VV P(sz, l);
  OZ_FiniteDomain u(fd_empty);
  int i;

  for (i = sz; i--; )
    l[i].read(reg_l[i]);

  if (hasEqualVars()) goto failure;

  for  (i = sz; i--; )
    if (*l[i] == fd_singl) {
      int s = l[i]->getSingleElem();
      if (u.isIn(s)) {
        goto failure;
      } else {
        u += s;
      }
    }

 loop:
  for (i = sz; i--; ) {
    if (*l[i] != fd_singl) {
      FailOnEmpty(*l[i] -= u);

      if (*l[i] == fd_singl) {
        u += l[i]->getSingleElem();
        goto loop;
      }
    }
  }

  int from, to;
  for (from = 0, to = 0; from < sz; from += 1)
    if (*l[from] != fd_singl)
      reg_l[to++] = reg_l[from];
  sz = to;

  /*
  for (i = to; i--; ) {
    f = f | *l[i];
    if (f.getSize() >= to) goto escape;
  }
  if (f.getSize() < to) goto failure;
  */

 escape:
  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_distinctOffset, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarSingl);
  OZ_EXPECT(pe, 1, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  return pe.impose(new DistinctOffsetPropagator(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_Return DistinctOffsetPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  if (reg_sz == 0) return PROCEED;

  if (mayBeEqualVars()) {
    int * eq_vars = OZ_findEqualVars(reg_sz, reg_l);
    for (int i = reg_sz; i--; )
      if ((eq_vars[i] > -1) &&
          (i != eq_vars[i]) &&
          (reg_offset[i] == reg_offset[eq_vars[i]])
          )
        return FAILED;
  }

  int &sz  = reg_sz;
  int * offset = reg_offset;

  DECL_DYN_ARRAY(OZ_FDIntVar, l, sz);
  PropagatorController_VV P(sz, l);
  OZ_FiniteDomain u(fd_empty);
  int i;

  for (i = sz; i--; )
    l[i].read(reg_l[i]);

loop:
  for (i=sz; i--; )
    if (*l[i] == fd_singl) {
      int s = offset[i]+l[i]->getSingleElem();
      for (int j=sz; j--; ) {
        if ( i != j) {
          if (*l[j] != fd_singl) {
            int tmp = s-offset[j];
            if (tmp >= 0) {
              FailOnEmpty(*l[j] -= tmp);
              if (*l[j] == fd_singl) goto loop;
            }
          }
          else {
            if (s == l[j]->getSingleElem()+ offset[j])
              goto failure;
          }
        }
      }
    }

  int from, to;
  for (from = 0, to = 0; from < sz; from += 1)
    if (*l[from] != fd_singl) {
      reg_l[to] = reg_l[from];
      offset[to++] = offset[from];
    }
  sz = to;


  OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------
// static members

OZ_CFun NotEqOffPropagator::spawner = fdp_notEqOff;
OZ_CFun LessEqOffPropagator::spawner = fdp_lessEqOff;
OZ_CFun MinimumPropagator::spawner = fdp_minimum;
OZ_CFun MaximumPropagator::spawner = fdp_maximum;
OZ_CFun IntersectionPropagator::spawner = fdp_inter;
OZ_CFun UnionPropagator::spawner = fdp_union;
OZ_CFun DistinctPropagator::spawner = fdp_distinct;
OZ_CFun DistinctOffsetPropagator::spawner = fdp_distinctOffset;
OZ_CFun  SubSetPropagator::spawner  = fdp_subset;

//-----------------------------------------------------------------------------
