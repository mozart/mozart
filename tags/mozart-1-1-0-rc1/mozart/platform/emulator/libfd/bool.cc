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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "bool.hh"

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_conj, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FDBOOL "," OZ_EM_FDBOOL "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectBoolVar, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new ConjunctionPropagator(OZ_in(0), 
					     OZ_in(1), 
					     OZ_in(2)));
}
OZ_BI_end

OZ_Return ConjunctionPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

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

OZ_BI_define(fdp_disj, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FDBOOL "," OZ_EM_FDBOOL "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectBoolVar, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new DisjunctionPropagator(OZ_in(0), 
					     OZ_in(1), 
					     OZ_in(2)));
}
OZ_BI_end

OZ_Return DisjunctionPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

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

OZ_BI_define(fdp_exor, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FDBOOL "," OZ_EM_FDBOOL "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectBoolVar, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new XDisjunctionPropagator(OZ_in(0), 
					      OZ_in(1), 
					      OZ_in(2)));
}
OZ_BI_end

OZ_Return XDisjunctionPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

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

OZ_BI_define(fdp_impl, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FDBOOL "," OZ_EM_FDBOOL "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectBoolVar, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new ImplicationPropagator(OZ_in(0), 
					     OZ_in(1), 
					     OZ_in(2)));
}
OZ_BI_end

OZ_Return ImplicationPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

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

OZ_BI_define(fdp_equi, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FDBOOL "," OZ_EM_FDBOOL "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectBoolVar, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new EquivalencePropagator(OZ_in(0), 
					     OZ_in(1), 
					     OZ_in(2)));
}
OZ_BI_end


OZ_Return EquivalencePropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

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

OZ_BI_define(fdp_nega, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FDBOOL "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectBoolVar, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectBoolVar, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new NegationPropagator(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

OZ_Return NegationPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

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


OZ_PropagatorProfile ConjunctionPropagator::profile;
OZ_PropagatorProfile DisjunctionPropagator::profile;
OZ_PropagatorProfile XDisjunctionPropagator::profile;
OZ_PropagatorProfile ImplicationPropagator::profile;
OZ_PropagatorProfile EquivalencePropagator::profile;
OZ_PropagatorProfile NegationPropagator::profile;
