/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    
 *  Contributors:
 *    Joerg Wuertz (wuertz@ps.uni-sb.de)
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

#include <math.h>

#include "base.hh"
#include "arith.hh"
#include "rel.hh"
#include "auxcomp.hh"

//-----------------------------------------------------------------------------
// Twice

OZ_Return TwicePropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  if (OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*x &= 0);
    return P.vanish();
  } else {
  loop:
    FailOnEmpty(*x >= int(ceil(y->getMinElem() / 2.0)));
    FailOnEmpty(*x <= int(floor(y->getMaxElem() / 2.0)));
    int ymin = y->getMinElem();
    int ymax = y->getMaxElem();
    FailOnEmpty(*y >= (2 * x->getMinElem()));
    FailOnEmpty(*y <= (2 * x->getMaxElem()));
    if (ymin < y->getMinElem()) goto loop;
    if (ymax > y->getMaxElem()) goto loop;
  }
  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// Square

OZ_Return SquarePropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  if (OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*x <= 1);
    return P.vanish();
  } else {
  loop:
    FailOnEmpty(*x >= int(ceil(sqrt(y->getMinElem()))));
    FailOnEmpty(*x <= int(floor(sqrt(y->getMaxElem()))));
    int ymin = y->getMinElem();
    int ymax = y->getMaxElem();
    FailOnEmpty(*y >= square(x->getMinElem()));
    FailOnEmpty(*y <= square(x->getMaxElem()));
    if (ymin < y->getMinElem()) goto loop;
    if (ymax > y->getMaxElem()) goto loop;
  }
  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_plus, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new PlusPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

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

OZ_BI_define(fdp_times, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new TimesPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

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

OZ_BI_define(fdp_divD, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT(pe, 1, expectInt);

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarAny, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  if (OZ_intToC(OZ_in(1)) == 0) return pe.fail();

  return pe.impose(new DivPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

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

OZ_BI_define(fdp_divI, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT(pe, 1, expectInt);

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  if (OZ_intToC(OZ_in(1)) == 0) return pe.fail();

  return pe.impose(new DivIPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

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

OZ_BI_define(fdp_modD, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT(pe, 1, expectInt);

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarAny, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  if (OZ_intToC(OZ_in(1)) == 0) return pe.fail();

  return pe.impose(new ModPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

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

OZ_BI_define(fdp_modI, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT(pe, 1, expectInt);

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  if (OZ_intToC(OZ_in(1)) == 0) return pe.fail();

  return pe.impose(new ModIPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_Return ModIPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_z))
    return replaceBy(new LessEqOffPropagator(reg_x, OZ_int(reg_y), -1));

  OZ_FDIntVar x(reg_x), z(reg_z);
  PropagatorController_V_V P(x, z);
  int &y = reg_y;
  OZ_Boolean touched;

  int xl,xu,zl,zu;

  FailOnEmpty(*z <= y - 1);
  
  xl = x->getMinElem(), xu = x->getMaxElem();
  zl = z->getMinElem(), zu = z->getMaxElem();
  
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

OZ_BI_define(fdp_power, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);
  
  PropagatorExpect pe;
  int susp_count = 0;
  
  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1)
    return pe.suspend();

  return pe.impose(new PowerPropagator(OZ_in(0), 
				       OZ_in(2), 
				       OZ_intToC(OZ_in(1))));
}
OZ_BI_end

inline
double power(int x, int y)
{
  return pow(x, y);
}

inline
double root(int x, int y)
{
  double r = pow(x, 1/double(y));
  if (x == pow(floor(r), y))
    return floor(r);
  else if (x == pow(ceil(r), y))
    return ceil(r);
  else
    return r;
}

OZ_Return PowerPropagator::propagate(void)
{
  int y = reg_c;
  OZ_FDIntVar x(reg_x), z(reg_y);
  PropagatorController_V_V P(x, z);
  
  if (y == 0) {
    FailOnEmpty(*z &= 1);
    return P.leave();
  }
  
  int zmin, zmax;
  
  do {
    FailOnEmpty(*x >= int(ceil(root(z->getMinElem(), y))));
    FailOnEmpty(*x <= int(floor(root(z->getMaxElem(), y))));

    zmin = z->getMinElem();
    zmax = z->getMaxElem();
    
    FailOnEmpty(*z >= int(ceil(power(x->getMinElem(), y))));
    FailOnEmpty(*z <= int(floor(power(x->getMaxElem(), y))));

  } while (zmin < z->getMinElem() || zmax > z->getMaxElem());

  return P.leave();

failure:
  return P.fail();
}

//=============================================================================
// domain consistent propagators

//-----------------------------------------------------------------------------
// Twice

OZ_Return TwiceDPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  OZ_FiniteDomain new_x(fd_empty), new_y(fd_empty);
  FiniteDomainIterator x_it(&*x);
  for (int x_v = x_it.resetToMin(); x_v != -1; x_v = x_it.nextLarger()) {
    int y_v = 2 * x_v;
      if (y->isIn(y_v)) {
	new_x += x_v;
	new_y += y_v;
      }
  }
  FailOnEmpty(*x &= new_x);
  FailOnEmpty(*y &= new_y);

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// Square

OZ_Return SquareDPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  OZ_FiniteDomain new_x(fd_empty), new_y(fd_empty);
  FiniteDomainIterator x_it(&*x);
  for (int x_v = x_it.resetToMin(); x_v != -1; x_v = x_it.nextLarger()) {
    int y_v = x_v * x_v;
      if (y->isIn(y_v)) {
	new_x += x_v;
	new_y += y_v;
      }
  }
  FailOnEmpty(*x &= new_x);
  FailOnEmpty(*y &= new_y);

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_plusD, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarAny, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new PlusDPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_Return PlusDPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(new TwiceDPropagator(reg_x, reg_z)),
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

  OZ_FiniteDomain new_x(fd_empty), new_y(fd_empty), new_z(fd_empty);
  FiniteDomainIterator x_it(&*x), y_it(&*y);
  for (int x_v = x_it.resetToMin(); x_v != -1; x_v = x_it.nextLarger()) {
    for (int y_v = y_it.resetToMin(); y_v != -1; y_v = y_it.nextLarger()) {
      int z_v = x_v + y_v;
      if (z->isIn(z_v)) {
	new_x += x_v;
	new_y += y_v;
	new_z += z_v;
      }
    }
  }
  FailOnEmpty(*x &= new_x);
  FailOnEmpty(*y &= new_y);
  FailOnEmpty(*z &= new_z);

  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_timesD, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarAny, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new TimesDPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_Return TimesDPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(new SquareDPropagator(reg_x,reg_z)),
		  replaceByInt(reg_y, 1),
		  replaceByInt(reg_x, 1));

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  OZ_FiniteDomain new_x(fd_empty), new_y(fd_empty), new_z(fd_empty);
  FiniteDomainIterator x_it(&*x), y_it(&*y);
  for (int x_v = x_it.resetToMin(); x_v != -1; x_v = x_it.nextLarger()) {
    for (int y_v = y_it.resetToMin(); y_v != -1; y_v = y_it.nextLarger()) {
      int z_v = x_v * y_v;
      if (z->isIn(z_v)) {
	new_x += x_v;
	new_y += y_v;
	new_z += z_v;
      }
    }
  }
  FailOnEmpty(*x &= new_x);
  FailOnEmpty(*y &= new_y);
  FailOnEmpty(*z &= new_z);

  return P.leave();

failure: 
  return P.fail(); 
}


//-----------------------------------------------------------------------------
// static members

OZ_PropagatorProfile TwicePropagator::profile;
OZ_PropagatorProfile SquarePropagator::profile;
OZ_PropagatorProfile PlusPropagator::profile;
OZ_PropagatorProfile TimesPropagator::profile;
OZ_PropagatorProfile DivPropagator::profile;
OZ_PropagatorProfile DivIPropagator::profile;
OZ_PropagatorProfile ModPropagator::profile;
OZ_PropagatorProfile ModIPropagator::profile;
OZ_PropagatorProfile PowerPropagator::profile;
OZ_PropagatorProfile TwiceDPropagator::profile;
OZ_PropagatorProfile SquareDPropagator::profile;
OZ_PropagatorProfile PlusDPropagator::profile;
OZ_PropagatorProfile TimesDPropagator::profile;

//-----------------------------------------------------------------------------
// eof
