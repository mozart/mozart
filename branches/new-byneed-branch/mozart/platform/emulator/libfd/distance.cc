/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
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

#include "distance.hh"
#include "rel.hh"
#include "arith.hh"
#include "sum.hh"

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_distance, 4, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_LIT "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 2, expectLiteral);
  sum_ops op = getSumOps(OZ_in(2));

  if (op == sum_ops_neq) {
    OZ_EXPECT(pe, 0, expectIntVarSingl);
    OZ_EXPECT(pe, 1, expectIntVarSingl);
    OZ_EXPECT(pe, 3, expectIntVarSingl);

    return pe.impose(new DistancePropagatorNeq(OZ_in(0), OZ_in(1), 
					      OZ_in(3), 0));
  } else {
    OZ_EXPECT(pe, 0, expectIntVarMinMax);
    OZ_EXPECT(pe, 1, expectIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax);

    switch (op) {
    case sum_ops_eq:
      return pe.impose(new DistancePropagatorEq(OZ_in(0), OZ_in(1), 
						OZ_in(3), 0));
    case sum_ops_leq:
      return pe.impose(new DistancePropagatorLeq(OZ_in(0), OZ_in(1), 
						 OZ_in(3), 0));
    case sum_ops_lt:
      return pe.impose(new DistancePropagatorLeq(OZ_in(0), OZ_in(1), 
						 OZ_in(3), 1));
    case sum_ops_geq:
      return pe.impose(new DistancePropagatorGeq(OZ_in(0), OZ_in(1), 
						 OZ_in(3), 0 ));
    case sum_ops_gt:
      return pe.impose(new DistancePropagatorGeq(OZ_in(0), OZ_in(1), 
						 OZ_in(3), 1));
    default: ;
    } 
  }
  
  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_BI_end

//=============================================================================

OZ_Return DistancePropagatorGeq::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_c;


  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
    

  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  int zl = z->getMinElem(), zu = z->getMaxElem();
  
  int lowx, lowy, upx, upy, upz2, upz1;

  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_y)) {
    if (xd==0) {
      FailOnEmpty( *z &= 0); 
      return P.vanish();
    }
    else return P.fail();
  }

  if (xu + xd + zu <= yl) return P.vanish();
  if (yu + xd + zu <= xl) return P.vanish();

  if (xu < zl + yl + xd) {
    P.vanish();
    return replaceBy(new LinLessEqPropagator(1, reg_z, 1, reg_x, -1, reg_y, xd));
  }

  if (yu < zl + xl + xd) {
    P.vanish();
    return replaceBy(new LinLessEqPropagator(1, reg_z, 1, reg_y, -1, reg_x, xd));
  }

  lowx = yu-xd-zl+1; lowy = xu-xd-zl+1; upz1 = xu-xd-yl;
  upx = yl+xd+zl-1; upy = xl+xd+zl-1; upz2 = xl+xd+yl;
  if (lowx <= upx) {
    OZ_FiniteDomain la;
    la.initRange(lowx,upx);
    FailOnEmpty(*x -= la);
  }
  if (lowy <= upy) {
    OZ_FiniteDomain la;
    la.initRange(lowy,upy);
    FailOnEmpty(*y -= la);
  }
  int maxi;
  if (upz1 > upz2) maxi=upz1;
  else maxi=upz2;

  if (zu > maxi) {
    FailOnEmpty(*z <= maxi);
  }
    
  
  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail");
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return DistancePropagatorLeq::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_c;

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
    
  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  int zl = z->getMinElem(), zu = z->getMaxElem();
  

  if ((xu + xd <= zl + yl) && (yu + xd <= zl + xl)) return P.vanish();

  int txu = zu+yu-xd, txl = yl+xd-zu;
  int tyu = zu+xu-xd, tyl = xl+xd-zu;
  int tzl1 = xl+xd-yu, tzl2 = yl+xd-xu;

  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_y)) {
    if (xd == 0) return P.vanish();
    else {
      FailOnEmpty(*z >= 1);
      return P.vanish();
    }
  }

loop:
  if (x->getMinElem() < txl) {
    FailOnEmpty(*x >= txl);
    tyl = x->getMinElem()+xd-z->getMaxElem();
    tzl1 = x->getMinElem()+xd-y->getMaxElem();
    goto loop;
  }
  if (x->getMaxElem() > txu) {
    FailOnEmpty(*x <= txu);
    tyu = z->getMaxElem()+x->getMaxElem()-xd;
    tzl2 = y->getMinElem()+xd-x->getMaxElem();
    goto loop;
  }
  if (y->getMinElem() < tyl) {
    FailOnEmpty(*y >= tyl);
    txl = y->getMinElem()+xd-z->getMaxElem();
    tzl2 = y->getMinElem()+xd-x->getMaxElem();
    goto loop;
  }
  if (y->getMaxElem() > tyu) {
    FailOnEmpty(*y <= tyu);
    txu = z->getMaxElem()+y->getMaxElem()-xd;
    tzl1 = x->getMinElem()+xd-y->getMaxElem();
    goto loop;
  }
  if ((z->getMinElem() < tzl1) || (z->getMinElem() < tzl2)){
    FailOnEmpty(*z >= tzl1);
    FailOnEmpty(*z >= tzl2);
    int zv = z->getMinElem();
    txu = zv+y->getMaxElem()-xd;
    txl = y->getMinElem()+xd-zv;
    tyu = zv+x->getMaxElem()-xd;
    tyl = x->getMinElem()+xd-zv;
    goto loop;
  }

  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return DistancePropagatorEq::propagate(void) 
{
  // y + xd =: x  or  x + xd =: y

  int &xd = reg_c;

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
    

  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  int zl = z->getMinElem(), zu = z->getMaxElem();

  OZ_FiniteDomain lx(*x), ly(*y), lz(*z);
  // copy x and y to lx and ly
     
  int lxl = ly.getMinElem()+lz.getMinElem(), 
       lxu = ly.getMaxElem()+lz.getMaxElem();
  int lyl = lx.getMinElem()-lz.getMaxElem(), 
    lyu = lx.getMaxElem()-lz.getMinElem();
  int lzl = lx.getMinElem()-ly.getMaxElem(), 
    lzu = lx.getMaxElem()-ly.getMinElem();

  OZ_FiniteDomain rx(*x), ry(*y), rz(*z);
  // copy x and y to rx and ry
     
  int rxl = ry.getMinElem()-rz.getMaxElem(), 
       rxu = ry.getMaxElem()-rz.getMinElem();
  int ryl = rx.getMinElem()+rz.getMinElem(), 
    ryu = rx.getMaxElem()+rz.getMaxElem();
  int rzl = ry.getMinElem()-rx.getMaxElem(), 
    rzu = ry.getMaxElem()-rx.getMinElem();

  OZ_FiniteDomain l1, l2, l3;

  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*z &= 0);
    return P.vanish();
  }
    
  if ((*x == fd_singl) && (*y == fd_singl) && (*z == fd_singl)){  
    if (xu + zl == yl) return P.vanish();
    if (yu + zl == xl) return P.vanish();
    return FAILED;
  }
  

leftloop:
  // for y + z =: x
  if (ly.getMinElem() < lyl) {
    if (ly >= lyl) {
      lxl = ly.getMinElem()+lz.getMinElem();
      lzu = lx.getMaxElem()-ly.getMinElem();
      goto leftloop;
    }
    else goto imposeRight;
  }

  if (ly.getMaxElem() > lyu) {
    if (ly <= lyu) {
      lxu = ly.getMaxElem()+lz.getMaxElem();
      lzl = lx.getMinElem()-ly.getMaxElem();
      goto leftloop;
    }
    else goto imposeRight;
  }
  
  if (lx.getMinElem() < lxl) {
    if (lx >= lxl) {
      lyl = lx.getMinElem()-lz.getMaxElem();
      lzl = lx.getMinElem()-ly.getMaxElem();
      goto leftloop;
    }
    else goto imposeRight;
  }

  if (lx.getMaxElem() > lxu) {
    if (lx <= lxu) {
      lyu = lx.getMaxElem()-lz.getMinElem();
      lzu = lx.getMaxElem()-ly.getMinElem();
      goto leftloop;
    }
    else goto imposeRight;
  }
  
  if (lz.getMinElem() < lzl) {
    if (lz >= lzl) {
      lxl = ly.getMinElem()+lz.getMinElem();
      lyu = lx.getMaxElem()-lz.getMinElem();
      goto leftloop;
    }
    else goto imposeRight;
  }
  
  if (lz.getMaxElem() > lzu) {
    if (lz <= lzu) {
      lyl = lx.getMinElem()-lz.getMaxElem();
      lxu = ly.getMaxElem()+lz.getMaxElem();
      goto leftloop;
    }
    else goto imposeRight;
  }

  goto rightloop;
    

imposeRight: 
      P.vanish();
      return replaceBy(new PlusPropagator(reg_x, reg_z, reg_y));

rightloop:
      // for y =: z + x
  if (ry.getMinElem() < ryl) {
    if (ry >= ryl) {
      rxl = ry.getMinElem()-rz.getMaxElem();
      rzl = ry.getMinElem()-rx.getMaxElem();
      goto rightloop;
    }
    else goto imposeLeft;
  }

  if (ry.getMaxElem() > ryu) {
    if (ry <= ryu) {
      rxu = ry.getMaxElem()-rz.getMinElem();
      rzu = ry.getMaxElem()-rx.getMinElem();
      goto rightloop;
    }
    else goto imposeLeft;
  }
  
  if (rx.getMinElem() < rxl) {
    if (rx >= rxl) {
      ryl = rx.getMinElem()+rz.getMinElem();
      rzu = ry.getMaxElem()-rx.getMinElem();
      goto rightloop;
    }
    else goto imposeLeft;
  }
  
  if (rx.getMaxElem() > rxu) {
    if (rx <= rxu) {
      ryu = rx.getMaxElem()+rz.getMaxElem();
      rzl = ry.getMinElem()-rx.getMaxElem();
      goto rightloop;
    }
    else goto imposeLeft;
  }
      
  if (rz.getMinElem() < rzl) {
    if (rz >= rzl) {
      rxu = ry.getMaxElem()-rz.getMinElem();
      ryl = rx.getMinElem()+rz.getMinElem();
      goto rightloop;
    }
    else goto imposeLeft;
  }
  
  if (rz.getMaxElem() > rzu) {
    if (rz <= rzu) {
      rxl = ry.getMinElem()-rz.getMaxElem();
      ryu = rx.getMaxElem()+rz.getMaxElem();
      goto rightloop;
    }
    else goto imposeLeft;
  }
      
  goto finish;

imposeLeft: 
      P.vanish();
      return replaceBy(new PlusPropagator(reg_y, reg_z, reg_x));
  
finish:

  l1 = (lx | rx);
  l2 = (ly | ry);
  l3 = (lz | rz);
  FailOnEmpty(*x &= l1);
  FailOnEmpty(*y &= l2);
  FailOnEmpty(*z &= l3);

  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return DistancePropagatorNeq::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_c;

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
    
  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  int zl = z->getMinElem(), zu = z->getMaxElem();
  
  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_y)) {
    FailOnEmpty(*z >= 1);
    return P.vanish();
  }

  if ( (*x == fd_singl) && (*y == fd_singl) && (*z == fd_singl)){
    if ((xl-yl != zl) && (yl-xl != zl)) 
      return PROCEED;
    else return FAILED;
  }
  
  if ( (*x == fd_singl) && (*y == fd_singl)) {
    FailOnEmpty(*z -= x->getSingleElem() - y->getSingleElem());
    FailOnEmpty(*z -= y->getSingleElem() - x->getSingleElem());
    return P.vanish();
  }
  if ( (*x == fd_singl) && (*z == fd_singl)) {
    FailOnEmpty(*y -= x->getSingleElem() - z->getSingleElem());
    FailOnEmpty(*y -= z->getSingleElem() + x->getSingleElem());
    return P.vanish();
  }
  if ( (*z == fd_singl) && (*y == fd_singl)) {
    FailOnEmpty(*x -= z->getSingleElem() + y->getSingleElem());
    FailOnEmpty(*x -= y->getSingleElem() - z->getSingleElem());
    return P.vanish();
  }

  
  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail");
  return P.fail();
}

//-----------------------------------------------------------------------------
// static member

OZ_PropagatorProfile DistancePropagatorLeq::profile;
OZ_PropagatorProfile DistancePropagatorGeq::profile;
OZ_PropagatorProfile DistancePropagatorEq::profile;
OZ_PropagatorProfile DistancePropagatorNeq::profile;
