/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "distance.hh"
#include "rel.hh"
#include "arith.hh"
#include "sum.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_distance, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_LIT "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 2, expectLiteral);

  char * op = OZ_atomToC(OZ_args[2]);

  if (!strcmp(SUM_OP_NEQ, op)) {
    OZ_EXPECT(pe, 0, expectIntVarSingl);
    OZ_EXPECT(pe, 1, expectIntVarSingl);
    OZ_EXPECT(pe, 3, expectIntVarSingl);

    return pe.spawn(new DistancePropagatorNeq(OZ_args[0], OZ_args[1], 
					      OZ_args[3], 0));
  } else {
    OZ_EXPECT(pe, 0, expectIntVarMinMax);
    OZ_EXPECT(pe, 1, expectIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax);

    if (!strcmp(SUM_OP_EQ, op)) {
      return pe.spawn(new DistancePropagatorEq(OZ_args[0], OZ_args[1], 
					       OZ_args[3], 0));
    } else if (!strcmp(SUM_OP_LEQ, op)) {
      return pe.spawn(new DistancePropagatorLeq(OZ_args[0], OZ_args[1], 
						OZ_args[3], 0));
    } else if (!strcmp(SUM_OP_LT, op)) {
      return pe.spawn(new DistancePropagatorLeq(OZ_args[0], OZ_args[1], 
					      OZ_args[3], 1));
    } else if (!strcmp(SUM_OP_GEQ, op)) {
      return pe.spawn(new DistancePropagatorGeq(OZ_args[0], OZ_args[1], 
						OZ_args[3], 0 ));
    } else if (!strcmp(SUM_OP_GT, op)) {
      return pe.spawn(new DistancePropagatorGeq(OZ_args[0], OZ_args[1], 
						OZ_args[3], 1));
    } 
  }

  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_C_proc_end

//=============================================================================

OZ_Return DistancePropagatorGeq::run(void) 
{
  OZ_DEBUGPRINT("in: " << *this);

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
    
  
  OZ_DEBUGPRINT("out: " << *this);

  return P.leave();

failure:
  OZ_DEBUGPRINT("fail" << *this);
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return DistancePropagatorLeq::run(void) 
{
  OZ_DEBUGPRINT("in: " << *this);

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

  OZ_DEBUGPRINT("out: " << *this);

  return P.leave();

failure:
  OZ_DEBUGPRINT("fail");
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return DistancePropagatorEq::run(void) 
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

  OZ_DEBUGPRINT("out: " << *this);

  return P.leave();

failure:
  OZ_DEBUGPRINT("fail");
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return DistancePropagatorNeq::run(void) 
{
  OZ_DEBUGPRINT("in: " << *this);

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

  
  OZ_DEBUGPRINT("out: " << *this);

  return P.leave();

failure:
  OZ_DEBUGPRINT("fail" << *this);
  return P.fail();
}

//-----------------------------------------------------------------------------
// static member

OZ_CFun DistancePropagatorLeq::spawner = fdp_distance;
OZ_CFun DistancePropagatorGeq::spawner = fdp_distance;
OZ_CFun DistancePropagatorEq::spawner = fdp_distance;
OZ_CFun DistancePropagatorNeq::spawner = fdp_distance;
