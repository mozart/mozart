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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "propagators.hh"
//#include "../reflect/reflect.hh"

//-----------------------------------------------------------------------------

OZ_BI_define(ri_lessEq, 2, 0)
{
  OZ_EXPECTED_TYPE(EM_RI "," EM_RI);

  RIExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectRIVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectRIVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend();

  return pe.impose(new RILessEq(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

//-----------------------------------------------------------------------------

OZ_PropagatorProfile RILessEq::profile;

OZ_Return RILessEq::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  // x <= y

  RIVar x(_x), y(_y);
  PropagatorController_RI_RI P(x, y);
  
  if (x->upperBound() <= y->lowerBound())
    return P.vanish();

  FailOnInvalid(*x <= y->upperBound());

  FailOnInvalid(*y >= x->lowerBound());

  RI_DEBUG_PRINT_THIS(("OUT "));

  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_BI_define(ri_greater, 2, 0)
{
  OZ_EXPECTED_TYPE(EM_RI "," EM_RI);

  RIExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectRIVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectRIVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend();

  return pe.impose(new RIGreater(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

OZ_PropagatorProfile RIGreater::profile;

OZ_Return RIGreater::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  // x > y

  RIVar x(_x), y(_y);
  PropagatorController_RI_RI P(x, y);

  if (x->lowerBound() > y->upperBound())
    return P.vanish();

  FailOnInvalid(*x > y->lowerBound());

  FailOnInvalid(*y < x->upperBound());

  RI_DEBUG_PRINT_THIS(("OUT "));

  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_BI_define(ri_plus, 3, 0)
{
  OZ_EXPECTED_TYPE(EM_RI "," EM_RI "," EM_RI);

  RIExpect pe;

  int susp_count = 0;
  
  OZ_EXPECT_SUSPEND(pe, 0, expectRIVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectRIVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectRIVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend();

  return pe.impose(new RIPlus(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_PropagatorProfile RIPlus::profile;

OZ_Return RIPlus::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  // x + y = z

  RIVar x(_x), y(_y), z(_z);
  ri_float x_w = x->getWidth(), y_w = y->getWidth(), z_w = z->getWidth();
  int redo;
  PropagatorController_RI_RI_RI P(x, y, z);

  do {
    redo = 0;

    // constrain lower bounds -> round downwards
    TOWARDS_MINUS_INF;
    
    FailOnInvalidTouched(*z >= x->lowerBound() + y->lowerBound(), z_w, redo);
    FailOnInvalidTouched(*x >= z->lowerBound() - y->upperBound(), x_w, redo);
    FailOnInvalidTouched(*y >= z->lowerBound() - x->upperBound(), y_w, redo);
    
    // constrain upper bounds -> round upwards
    TOWARDS_PLUS_INF;

    FailOnInvalidTouched(*z <= x->upperBound() + y->upperBound(), z_w, redo);
    FailOnInvalidTouched(*x <= z->upperBound() - y->lowerBound(), x_w, redo);
    FailOnInvalidTouched(*y <= z->upperBound() - x->lowerBound(), y_w, redo);
  } while (redo);

  RI_DEBUG_PRINT_THIS(("OUT "));

  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_BI_define(ri_times, 3, 0)
{
  OZ_EXPECTED_TYPE(EM_RI "," EM_RI "," EM_RI);

  RIExpect pe;

  int susp_count = 0;
  
  OZ_EXPECT_SUSPEND(pe, 0, expectRIVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectRIVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectRIVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend();

  return pe.impose(new RITimes(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_PropagatorProfile RITimes::profile;

OZ_Return RITimes::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  // x * y = z

  RIVar x(_x), y(_y), z(_z);
  ri_float x_w = x->getWidth(), y_w = y->getWidth(), z_w = z->getWidth();
  int redo;
  PropagatorController_RI_RI_RI P(x, y, z);

  do {
    redo = 0;

    // constrain lower bound of `z' -> round downwards
    TOWARDS_MINUS_INF;
    
    ri_float z_min_ri = min_ri(x->lowerBound() * y->lowerBound(),
			       x->lowerBound() * y->upperBound(),
			       x->upperBound() * y->lowerBound(),
			       x->upperBound() * y->upperBound());

    FailOnInvalidTouched(*z >= z_min_ri, z_w, redo);
    
    RI_DEBUG_PRINT(("z_min_ri=%f redo=%d\n", z_min_ri, redo));

    // constrain upper bound of `z' -> round upwards
    TOWARDS_PLUS_INF;

    ri_float z_max_ri = max_ri(x->lowerBound() * y->lowerBound(),
			       x->lowerBound() * y->upperBound(),
			       x->upperBound() * y->lowerBound(),
			       x->upperBound() * y->upperBound());
    FailOnInvalidTouched(*z <= z_max_ri, z_w, redo);

    RI_DEBUG_PRINT(("z_max_ri=%f redo=%d\n", z_max_ri, redo));

    // constraint `x' (if possible, i.e., `y' does not contain `0')
    if (! y->containsZero()) {

      {
	// constrain lower bound of `z' -> round downwards
	TOWARDS_MINUS_INF;
	
	ri_float y_ub_reciprocal = 1.0 / y->upperBound();
	ri_float y_lb_reciprocal = 1.0 / y->lowerBound();
	
	ri_float x_min_ri = min_ri(z->lowerBound() * y_lb_reciprocal,
				   z->lowerBound() * y_ub_reciprocal,
				   z->upperBound() * y_lb_reciprocal,
				   z->upperBound() * y_ub_reciprocal);
	FailOnInvalidTouched(*x >= x_min_ri, x_w, redo);
	
	RI_DEBUG_PRINT(("x_min_ri=%f redo=%d\n", x_min_ri, redo));
      }

      {
	// constrain upper bound of `z' -> round upwards
	TOWARDS_PLUS_INF;
	
	ri_float y_ub_reciprocal = 1.0 / y->upperBound();
	ri_float y_lb_reciprocal = 1.0 / y->lowerBound();
	
	ri_float x_max_ri = max_ri(z->lowerBound() * y_lb_reciprocal,
				   z->lowerBound() * y_ub_reciprocal,
				   z->upperBound() * y_lb_reciprocal,
				   z->upperBound() * y_ub_reciprocal);
	FailOnInvalidTouched(*x <= x_max_ri, x_w, redo);
	
	RI_DEBUG_PRINT(("x_max_ri=%f redo=%d\n", x_max_ri, redo));
      }
    }

    // constraint `y' (if possible, i.e., `x' does not contain `0')
    if (! x->containsZero()) {

      {      
	// constrain lower bound of `z' -> round downwards
	TOWARDS_MINUS_INF;
	
	ri_float x_ub_reciprocal = 1.0 / x->upperBound();
	ri_float x_lb_reciprocal = 1.0 / x->lowerBound();
	
	ri_float y_min_ri = min_ri(z->lowerBound() * x_lb_reciprocal,
				   z->lowerBound() * x_ub_reciprocal,
				   z->upperBound() * x_lb_reciprocal,
				   z->upperBound() * x_ub_reciprocal);
	FailOnInvalidTouched(*y >= y_min_ri, y_w, redo);
	
	RI_DEBUG_PRINT(("y_min_ri=%f redo=%d\n", y_min_ri, redo));
      }
      {
	// constrain upper bound of `z' -> round upwards
	TOWARDS_PLUS_INF;

	ri_float x_ub_reciprocal = 1.0 / x->upperBound();
	ri_float x_lb_reciprocal = 1.0 / x->lowerBound();
	
	ri_float y_max_ri = max_ri(z->lowerBound() * x_lb_reciprocal,
				   z->lowerBound() * x_ub_reciprocal,
				   z->upperBound() * x_lb_reciprocal,
				   z->upperBound() * x_ub_reciprocal);
	
	FailOnInvalidTouched(*y <= y_max_ri, y_w, redo);

	RI_DEBUG_PRINT(("y_max_ri=%f redo=%d\n", y_max_ri, redo));
      }
    }

  } while (redo);

  RI_DEBUG_PRINT_THIS(("OUT "));

  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_BI_define(ri_intBounds, 2, 0)
{
  OZ_EXPECTED_TYPE(EM_RI "," OZ_EM_FD);

  RIExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectRIVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend();

  return pe.impose(new RIIntBounds(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

OZ_PropagatorProfile RIIntBounds::profile;

OZ_Return RIIntBounds::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  RIVar ri(_ri);
  OZ_FDIntVar d(_d);
  PropagatorController_RI_D P(ri, d);

loop:
  {  
    ri_float ri_lb = ri->lowerBound();
    ri_float ri_ub = ri->upperBound();
    
    int d_lb = d->getMinElem();
    int d_ub = d->getMaxElem();
    
    ri_lb = ceil(ri_lb);
    ri_ub = floor(ri_ub);
  
    ri_lb = max_ri(ri_lb, (ri_float) d_lb);
    ri_ub = min_ri(ri_ub, (ri_float) d_ub);

    FailOnEmpty(*d >= int(ri_lb));
    FailOnEmpty(*d <= int(ri_ub));
 
    // iterate if holes in the domain are detected
    if (d->getMinElem() > d_lb || d->getMaxElem() < d_ub)
      goto loop;

    FailOnInvalid(*ri >= ri_lb);
    FailOnInvalid(*ri <= ri_ub);
  }

  RI_DEBUG_PRINT_THIS(("OUT "));
  
  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));
  
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_BI_define(ri_intBoundsSPP, 2, 0)
{
  OZ_EXPECTED_TYPE(EM_RI "," OZ_EM_FD);

  RIExpect pe;

  OZ_EXPECT(pe, 0, expectRIVarMinMax);
  OZ_EXPECT(pe, 1, expectIntVarMinMax);

  return pe.impose(new RIIntBoundsSPP(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

OZ_PropagatorProfile RIIntBoundsSPP::profile;

OZ_Return RIIntBoundsSPP::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  RIVar ri(_ri);
  OZ_FDIntVar d(_d);
  PropagatorController_RI_D P(ri, d);

  if (d->getSize() > 2) {
    printf("Warning (RIIntBoundsSPP::propagate): size of d must be <= 1\n");
    goto failure;
  }

  if (!(0.0 <= ri->lowerBound() && ri->upperBound() <= 1.0)) {
    printf("Warning (RIIntBoundsSPP::propagate): ri must be 0<=ri<=1\n");
    goto failure;
  }

  {  
    ri_float ri_lb = ri->lowerBound();
    ri_float ri_ub = ri->upperBound();
    
    int d_lb = d->getMinElem();
    int d_ub = d->getMaxElem();
    
    ri_lb = ceil(ri_lb);
    ri_ub = floor(ri_ub);
  
    if (ri_lb > 0.0) 
      FailOnEmpty(*d >= 1);

    if (ri_ub < 1.0) 
      FailOnEmpty(*d &= 0);

    if (d_lb > 0) 
      FailOnInvalid(*ri = 1.0);

    if (d_ub == 0)
	FailOnInvalid(*ri = 0.0);
  }

  RI_DEBUG_PRINT_THIS(("OUT "));
  
  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));
  
  return P.fail();
}

// eof
//-----------------------------------------------------------------------------
