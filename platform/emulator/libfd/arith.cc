/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include <math.h>

#include "arith.hh"
#include "rel.hh"
#include "auxcomp.hh"

//-----------------------------------------------------------------------------
// Twice

OZ_C_proc_begin(fdp_twice, 2)
{
  OZ_warning("This foreign function must never be called.");
  return FAILED;
}
OZ_C_proc_end

OZ_Return TwicePropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  if (OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*x &= 0);
    return P.vanish();
  } else {
    FailOnEmpty(*x >= int(ceil(y->getMinElem() / 2.0)));
    FailOnEmpty(*x <= int(floor(y->getMaxElem() / 2.0)));
    FailOnEmpty(*y >= (2 * x->getMinElem()));
    FailOnEmpty(*y <= (2 * x->getMaxElem()));
  }
  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// Square

OZ_C_proc_begin(fdp_square, 2)
{
  OZ_warning("This foreign function must never be called.");
  return FAILED;
}
OZ_C_proc_end

OZ_Return SquarePropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  if (OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*x <= 1);
    return P.vanish();
  } else {
    FailOnEmpty(*x >= int(ceil(sqrt(y->getMinElem()))));
    FailOnEmpty(*x <= int(floor(sqrt(y->getMaxElem()))));
    FailOnEmpty(*y >= square(x->getMinElem()));
    FailOnEmpty(*y <= square(x->getMaxElem()));
  }
  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_plus_rel, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectIntVarMinMax);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);

  return pe.impose(new PlusPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_C_proc_begin(fdp_plus, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new PlusPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return PlusPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(new TwicePropagator(reg_x, reg_z)),
		  replaceByInt(reg_y, 0),
		  replaceByInt(reg_x, 0));

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
  
  if (*x == fd_singl) 
    if (*x == 0) {
      P.vanish();
      return replaceBy(reg_y, reg_z);
    }

  if (*y == fd_singl) 
    if (*y == 0) {
      P.vanish();
      return replaceBy(reg_x, reg_z);
    }

  int txl = z->getMinElem() - y->getMaxElem();
  int txu = z->getMaxElem() - y->getMinElem();
  int tyl = z->getMinElem() - x->getMaxElem();
  int tyu = z->getMaxElem() - x->getMinElem();
  int tzl = x->getMinElem() + y->getMinElem();
  int tzu = x->getMaxElem() + y->getMaxElem();

 loop:
  if (x->getMinElem() < txl) {
    FailOnEmpty(*x >= txl);
    tyu = z->getMaxElem() - x->getMinElem();
    tzl = x->getMinElem() + y->getMinElem();
    goto loop;
  }
  
  if (x->getMaxElem() > txu) {
    FailOnEmpty(*x <= txu);
    tyl = z->getMinElem() - x->getMaxElem();
    tzu = x->getMaxElem() + y->getMaxElem();
    goto loop;
  }
  
  if (y->getMinElem() < tyl) {
    FailOnEmpty(*y >= tyl);
    txu = z->getMaxElem() - y->getMinElem();
    tzl = x->getMinElem() + y->getMinElem();
    goto loop;
  }
  
  if (y->getMaxElem() > tyu) {
    FailOnEmpty(*y <= tyu);
    txl = z->getMinElem() - y->getMaxElem();
    tzu = x->getMaxElem() + y->getMaxElem();
    goto loop;
  }
  
  if (z->getMinElem() < tzl) {
    FailOnEmpty(*z >= tzl);
    txl = z->getMinElem() - y->getMaxElem();
    tyl = z->getMinElem() - x->getMaxElem();
    goto loop;
  }
  
  if (z->getMaxElem() > tzu) {
    FailOnEmpty(*z <= tzu);
    txu = z->getMaxElem() - y->getMinElem();
    tyu = z->getMaxElem() - x->getMinElem();
    goto loop;
  }

  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_minus, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new MinusPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return MinusPropagator::propagate(void)
{
  SimplifyOnUnify(replaceByInt(reg_z, 0),
		  replaceByInt(reg_y, 0),
		  replaceBy(new TwicePropagator(reg_z, reg_x)));

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
  
  int txl = z->getMinElem() + y->getMinElem();
  int txu = z->getMaxElem() + y->getMaxElem();
  int tyl = x->getMinElem() - z->getMaxElem();
  int tyu = x->getMaxElem() - z->getMinElem();
  int tzl = x->getMinElem() - y->getMaxElem();
  int tzu = x->getMaxElem() - y->getMinElem();

 loop:
  if (x->getMinElem() < txl) {
    FailOnEmpty(*x >= txl);
    tyl = x->getMinElem() - z->getMaxElem();
    tzl = x->getMinElem() - y->getMaxElem();
    goto loop; 
  }

  if (x->getMaxElem() > txu) {
    FailOnEmpty(*x <= txu);
    tyu = x->getMaxElem() - z->getMinElem();
    tzu = x->getMaxElem() - y->getMinElem();
    goto loop; 
  }

  if (y->getMinElem() < tyl) {
    FailOnEmpty(*y >= tyl);
    txl = z->getMinElem() + y->getMinElem();
    tzu = x->getMaxElem() - y->getMinElem();
    goto loop; 
  }

  if (y->getMaxElem() > tyu) {
    FailOnEmpty(*y <= tyu);
    txu = z->getMaxElem() + y->getMaxElem();
    tzl = x->getMinElem() - y->getMaxElem();
    goto loop; 
  }

  if (z->getMinElem() < tzl) {
    FailOnEmpty(*z >= tzl);
    txl = z->getMinElem() + y->getMinElem();
    tyu = x->getMaxElem() - z->getMinElem();
    goto loop; 
  }

  if (z->getMaxElem() > tzu) {
    FailOnEmpty(*z <= tzu);
    txu = z->getMaxElem() + y->getMaxElem();
    tyl = x->getMinElem() - z->getMaxElem();
    goto loop; 
  }
  
  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_times, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new TimesPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_C_proc_begin(fdp_times_rel, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectIntVarMinMax);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);

  return pe.impose(new TimesPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return TimesPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(new SquarePropagator(reg_x,reg_z)),
		  replaceByInt(reg_y, 1),
		  replaceByInt(reg_x, 1));

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  int tzl = truncToIntMax(double(x->getMinElem()) * double(y->getMinElem()));
  int tzu = truncToIntMax(double(x->getMaxElem()) * double(y->getMaxElem()));
  int xint=0, yint=0;

loop:
  if (*x == fd_singl && *y == fd_singl) {
    FailOnEmpty(*z &= truncToIntMax(double(x->getSingleElem()) * double(y->getSingleElem())));
    goto loopend;
  }
  
  // z mod x = 0
  if (*x == fd_singl && *z == fd_singl && (xint = x->getSingleElem()) != 0) {
    int zint;
    if ((zint = z->getSingleElem()) % xint != 0) {
      goto failure;
    } else {	
      FailOnEmpty(*y &= (zint / xint));
      goto loopend;
    }
  }
  
  // z mod y = 0
  if (*y == fd_singl && *z == fd_singl && (yint = y->getSingleElem()) != 0) {
    int zint;
    if ((zint = z->getSingleElem()) % yint != 0) {
      goto failure;
    } else {
      FailOnEmpty(*x &= (zint / yint));
      goto loopend;
    }
  }
  
  if (z->getMinElem() < tzl) {
    FailOnEmpty(*z >= tzl);
    goto loop;
  }
  
  if (z->getMaxElem() > tzu) {
    FailOnEmpty(*z <= tzu);
    goto loop;
  }
  
  if (y->getMaxElem() != 0) {
    int txl = int(ceil(double(z->getMinElem()) / double(y->getMaxElem())));
    if (x->getMinElem() < txl) {
      FailOnEmpty(*x >= txl);
      tzl = truncToIntMax(double(x->getMinElem()) * double(y->getMinElem()));
      goto loop;
    }
  }
  if (y->getMinElem() != 0) {
    int txu = int(floor(double(z->getMaxElem()) / double(y->getMinElem())));
    if (x->getMaxElem() > txu) {
      FailOnEmpty(*x <= txu);
      tzu = truncToIntMax(double(x->getMaxElem()) * double(y->getMaxElem()));
      goto loop;
    }
  }
  
  if (x->getMaxElem() != 0) {
    int tyl = int(ceil(double(z->getMinElem()) / double(x->getMaxElem())));
    if (y->getMinElem() < tyl) {
      FailOnEmpty(*y >= tyl);
      tzl = truncToIntMax(double(x->getMinElem()) * double(y->getMinElem()));
      goto loop;
    }
  }
  if (x->getMinElem() != 0) {
    int tyu = int(floor(double(z->getMaxElem()) / double(x->getMinElem())));
    if (y->getMaxElem() > tyu) { 
      FailOnEmpty(*y <= tyu);
      tzl = truncToIntMax(double(x->getMinElem()) * double(y->getMinElem()));
      goto loop;
    }
  }
  
 loopend:
  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_divD, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  if (OZ_intToC(OZ_args[1]) == 0) return pe.fail();

  return pe.impose(new DivPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return DivPropagator::propagate(void)
{
  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_z))
    return replaceByInt(reg_x, 1);
  
  OZ_FDIntVar x(reg_x), z(reg_z);
  PropagatorController_V_V P(x, z);
  int &y = reg_y;
  
  OZ_FiniteDomain x_new(fd_empty), z_new(fd_empty);
  
  FiniteDomainIterator x_it(&*x);
  for (int x_v = x_it.resetToMin(); x_v != -1; x_v = x_it.nextLarger()) {
    int z_v = x_v / y;
    if (z->isIn(z_v)) {
      x_new += x_v;
      z_new += z_v;
    }
  }

  FailOnEmpty(*x &= x_new);
  FailOnEmpty(*z &= z_new);

  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_divI, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  if (OZ_intToC(OZ_args[1]) == 0) return pe.fail();

  return pe.impose(new DivIPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return DivIPropagator::propagate(void)
{
  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_z))
    return replaceByInt(reg_x, 1);

  OZ_FDIntVar x(reg_x), z(reg_z);
  PropagatorController_V_V P(x, z);
  int &y = reg_y;
  OZ_Boolean touched = OZ_FALSE;
  
  int xl = x->getMinElem(), xu = x->getMaxElem();
  int zl = z->getMinElem(), zu = z->getMaxElem();

loop:
  while (xl < truncToIntMax(double(zl) * double(y))) {
    FailOnEmpty(*x >= truncToIntMax(double(zl) * double(y)));
    xl = x->getMinElem();
  }
  while (xu > truncToIntMax(double(zu) * double(y) + y - 1)) {
    FailOnEmpty(*x <= truncToIntMax(double(zu) * double(y) + y - 1));
    xu = x->getMaxElem();
  }
  while (zl < (xl / y)) {
    FailOnEmpty(*z >= (xl / y));
    zl = z->getMinElem();
    touched = OZ_TRUE;
  }
  while (zu > (xu / y)) {
    FailOnEmpty(*z <= (xu / y));
    zu = z->getMaxElem();
    touched = OZ_TRUE;
  }
  if (touched) {
    touched = OZ_FALSE;
    goto loop;
  }
  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_modD, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  if (OZ_intToC(OZ_args[1]) == 0) return pe.fail();

  return pe.impose(new ModPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return ModPropagator::propagate(void)
{
  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_z))
    return replaceBy(new LessEqOffPropagator(reg_x, OZ_int(reg_y), -1));

  OZ_FDIntVar x(reg_x), z(reg_z);
  PropagatorController_V_V P(x, z);
  int &y = reg_y, xu = x->getMaxElem();
  OZ_FiniteDomain x_new(fd_empty), z_new(fd_empty);
  FiniteDomainIterator x_it(&*x);

  FailOnEmpty(*z <= y - 1);
  
  if (xu < y) {
    P.vanish();
    return replaceBy(reg_x, reg_z);
  } else if (*x == fd_singl && x->getSingleElem() == y) {
    P.vanish();
    return replaceByInt(reg_z, 0);
  } else if (xu <= y && *z == fd_singl && z->getSingleElem() == 0) {
    OZ_FiniteDomain u(fd_empty); 
    u += 0;
    if (xu == y) u += y;
    FailOnEmpty(*x &= u);
    return P.vanish();
  } 

  {
    for (int x_v = x_it.resetToMin(); x_v != -1; x_v = x_it.nextLarger()) {
      int z_v = x_v % y;
      if (z->isIn(z_v)) {
	x_new += x_v;
	z_new += z_v;
      }
    }
  }
  
  FailOnEmpty(*x &= x_new);
  FailOnEmpty(*z &= z_new);

  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_modI, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  if (OZ_intToC(OZ_args[1]) == 0) return pe.fail();

  return pe.impose(new ModIPropagator(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_Return ModIPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in: " << *this);

  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_z))
    return replaceBy(new LessEqOffPropagator(reg_x, OZ_int(reg_y), -1));

  OZ_FDIntVar x(reg_x), z(reg_z);
  PropagatorController_V_V P(x, z);
  int &y = reg_y;
  OZ_Boolean touched;

  int xl = x->getMinElem(), xu = x->getMaxElem();
  int zl = z->getMinElem(), zu = z->getMaxElem();
  
  FailOnEmpty(*z <= y - 1);
  
  if (xu < y) {
    P.vanish();
    return replaceBy(reg_x, reg_z);
  } else if (*x == fd_singl && x->getSingleElem() == y) {
    P.vanish();
    return replaceByInt(reg_z, 0);
  } else if (xu <= y && *z == fd_singl && z->getSingleElem() == 0) {
    OZ_FiniteDomain u(fd_empty); 
    u += 0;
    if (xu == y) u += y;
    FailOnEmpty(*x &= u);
    return P.vanish();
  } 

  do{
    touched = OZ_FALSE;
    
    int xl_y = xl % y;
    if ((xl_y < zl) || (zu < xl_y)) {
      FailOnEmpty(*x >= xl+1);
      xl = x->getMinElem();
      touched = OZ_TRUE;
    }
    
    int xu_y = xu % y;
    if ((xu_y < zl) || (zu < xu_y)) {
      FailOnEmpty(*x <= xu-1);
      xu = x->getMaxElem();
      touched = OZ_TRUE;
    }

    if (xu - xl < y - 1) {
      if (ceil((xl-zl)/double(y)) > floor((xu-zl)/double(y))) {
	FailOnEmpty(*z >= zl+1);
	zl = z->getMinElem();
	touched = OZ_TRUE;
      }
      
      if (ceil((xl-zu)/double(y)) > floor((xu-zu)/double(y))) {
	FailOnEmpty(*z <= zu-1);
	zu = z->getMaxElem();
      touched = OZ_TRUE;
      }
    }
  } while (touched);

  return P.leave();
  
failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------
// static members

OZ_CFun TwicePropagator::spawner = fdp_twice;
OZ_CFun SquarePropagator::spawner = fdp_square;
OZ_CFun PlusPropagator::spawner = fdp_plus;
OZ_CFun MinusPropagator::spawner = fdp_minus;
OZ_CFun TimesPropagator::spawner = fdp_times;
OZ_CFun DivPropagator::spawner = fdp_divD;
OZ_CFun DivIPropagator::spawner = fdp_divI;
OZ_CFun ModPropagator::spawner = fdp_modD;
OZ_CFun ModIPropagator::spawner = fdp_modI;

//-----------------------------------------------------------------------------
