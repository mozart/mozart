/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller, wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "disjoint.hh" 
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_disjoint, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD "," OZ_EM_INT);
  
  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);

  return pe.impose(new SchedCDPropagator(OZ_args[0], OZ_args[1], 
					OZ_args[2], OZ_args[3]));
}
OZ_C_proc_end

OZ_Return SchedCDPropagator::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_xd, &yd = reg_yd;

  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);
    
  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  
  if (xu + xd <= yl) return P.vanish();
  if (yu + yd <= xl) return P.vanish();
  
  if (xl + xd > yu) {
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
  }

  if (yl + yd > xu) {
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
  }
  
  int lowx = yu-xd+1, lowy = xu-yd+1;
  int upx = yl+yd-1, upy = xl+xd-1;
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
    
  
  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail");
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_disjointC, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);
  OZ_EXPECT(pe, 4, expectIntVarMinMax);

  return pe.impose(new SchedCDBPropagator(OZ_args[0], OZ_args[1], 
					 OZ_args[2], OZ_args[3], OZ_args[4]));
}
OZ_C_proc_end

OZ_Return SchedCDBPropagator::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_xd, &yd = reg_yd;

  OZ_FDIntVar x(reg_x), y(reg_y), b(reg_b);
  PropagatorController_V_V_V P(x, y, b);
  OZ_FiniteDomain la, lb, lc, ld, l1, l2;
    
  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  int lowx, lowy, upx, upy;
  
  if (xu + xd <= yl) {
    FailOnEmpty(*b &= 0);
    return P.vanish();
  }
  if (yu + yd <= xl) {
    FailOnEmpty(*b &= 1);
    return P.vanish();
  }
  
  if (xl + xd > yu){
    FailOnEmpty(*b &= 1);
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
  }
    
  if (yl + yd > xu){
    FailOnEmpty(*b &= 0);
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
  }

  if (*b == fd_singl) {
    P.vanish();
    if (b->getSingleElem() == 0) {
      return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
    } else {
      return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
    }
  }
  lowx = yu-xd+1; lowy = xu-yd+1;
  upx = yl+yd-1; upy = xl+xd-1;
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
    
  OZ_DEBUGPRINTTHIS("out: ");
  return P.leave();
failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}


//-----------------------------------------------------------------------------
// static member

OZ_CFun SchedCDPropagator::spawner = fdp_disjoint;
OZ_CFun SchedCDBPropagator::spawner = fdp_disjointC;
