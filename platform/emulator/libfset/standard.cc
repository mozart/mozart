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

#include "standard.hh"

OZ_BI_define(fsp_intersection, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectFSetVarBounds, susp_count);
  
  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new FSetIntersectionPropagator(OZ_in(0),
						  OZ_in(1),
						  OZ_in(2)));
} 
OZ_BI_end

OZ_BI_define(fsp_union, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectFSetVarBounds, susp_count);
  
  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new FSetUnionPropagator(OZ_in(0),
					   OZ_in(1),
					   OZ_in(2)));
} 
OZ_BI_end


OZ_BI_define(fsp_subsume, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  
  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new FSetSubsumePropagator(OZ_in(0),
					     OZ_in(1)));
} 
OZ_BI_end


OZ_BI_define(fsp_disjoint, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarGlb, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarGlb, susp_count);
  
  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new FSetDisjointPropagator(OZ_in(0),
					      OZ_in(1)));
} 
OZ_BI_end


OZ_BI_define(fsp_distinct, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  
  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new FSetDistinctPropagator(OZ_in(0),
					      OZ_in(1)));
} 
OZ_BI_end

OZ_BI_define(fsp_diff, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET","OZ_EM_FSET","OZ_EM_FSET);

  PropagatorExpect pe;
   int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectFSetVarBounds, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new FSetDiffPropagator(OZ_in(0),
					  OZ_in(1),
					  OZ_in(2)));
}
OZ_BI_end


//*****************************************************************************

OZ_Return FSetIntersectionPropagator::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_FSetVar x(_x), y(_y), z(_z);
  PropagatorController_S_S_S P(x, y, z);
  FSetTouched xt, yt, zt;

  do {
    xt = x;  yt = y;  zt = z;
    
    if (z->isEmpty()) {
      OZ_DEBUGPRINTTHIS("replace: (z empty)");
      P.vanish();
      return replaceBy(new FSetDisjointPropagator(_x, _y));
    }
    if (x->isSubsumedBy(*y)) {
      OZ_DEBUGPRINTTHIS("replace: (x subsumbed by y)");
      P.vanish();
      return OZ_DEBUGRETURNPRINT(replaceBy(_x, _z));
    }
    if (y->isSubsumedBy(*x)) {
      OZ_DEBUGPRINTTHIS("replace: (y subsumbed by xy)");
      P.vanish();
      return OZ_DEBUGRETURNPRINT(replaceBy(_y, _z));
    }
    
    FailOnInvalid(*x <= -(- *z & *y)); // lub
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y <= -(- *z & *x)); // lub
    OZ_DEBUGPRINT(("y=%s",y->toString()));
    
    FailOnInvalid(*z <<= (*x & *y)); // glb
    OZ_DEBUGPRINT(("z=%s",z->toString()));
    FailOnInvalid(*x >= *z); // glb
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y >= *z); // glb
    OZ_DEBUGPRINT(("y=%s",y->toString()));
    
  } while (xt <= x || yt <= y || zt <= z);
    
  OZ_DEBUGPRINTTHIS("out ");

  return OZ_DEBUGRETURNPRINT(P.leave1()); 
  
failure:
  OZ_DEBUGPRINTTHIS("failed ");
  return P.fail();
}

OZ_Return FSetUnionPropagator::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_FSetVar x(_x), y(_y), z(_z);
  PropagatorController_S_S_S P(x, y, z);
  FSetTouched xt, yt, zt;

  do {
    xt = x;  yt = y;  zt = z;
    
    if (z->isEmpty()) {
      OZ_DEBUGPRINT(("z->isEmpty()"));
      OZ_FSetConstraint aux(fs_empty);
      FailOnInvalid(*x <<= aux);
      FailOnInvalid(*y <<= aux);
      P.vanish();
      return OZ_ENTAILED;
    }
    if (x->isSubsumedBy(*y)) {
      OZ_DEBUGPRINT(("x->isSubsumedBy(*y)"));
      P.vanish();
      return replaceBy(_y, _z);
    }
    if (y->isSubsumedBy(*x)) {
      OZ_DEBUGPRINT(("y->isSubsumedBy(*x)"));
      P.vanish();
      return replaceBy(_x, _z);
    }
    
    OZ_DEBUGPRINT(("before"));
    FailOnInvalid(*x >= (*z & - *y)); // glb
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y >= (*z & - *x)); // glb
    OZ_DEBUGPRINT(("y=%s",y->toString()));
    
    FailOnInvalid(*z <<= (*x | *y)); // lub
    OZ_DEBUGPRINT(("z=%s",z->toString()));
    FailOnInvalid(*x <= *z); // lub
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y <= *z); // lub
    OZ_DEBUGPRINT(("y=%s",y->toString()));
    
  } while (xt <= x || yt <= y || zt <= z);
  
  OZ_DEBUGPRINTTHIS("out ");
  return P.leave1();
  
failure:
  OZ_DEBUGPRINTTHIS("failed ");
  return P.fail();
}

OZ_Return FSetSubsumePropagator::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("int ");
  OZ_FSetVar x(_x), y(_y);
  PropagatorController_S_S P(x, y);

  FailOnInvalid(*x <= *y);
  FailOnInvalid(*y >= *x);

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave1(); /* is entailed if only 
			one var is left */
  
failure:
  OZ_DEBUGPRINTTHIS("failed ");
  return P.fail();
}

OZ_Return FSetDisjointPropagator::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in ");
  
  OZ_FSetVar x(_x), y(_y);
  PropagatorController_S_S P(x, y);

  FSetTouched xt, yt;

  do {
    xt = x;  yt = y;

    FailOnInvalid(*x != *y);
    FailOnInvalid(*y != *x);
    
  } while (xt <= x || yt <= y);
  
  OZ_DEBUGPRINTTHIS("out ");
  return  OZ_DEBUGRETURNPRINT(P.leave1()); 
  /* is entailed if only one var is left */

failure:
  OZ_DEBUGPRINTTHIS("failed ");
  return P.fail();
}

OZ_Return FSetDistinctPropagator::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in ");
  OZ_FSetVar x(_x), y(_y);
  PropagatorController_S_S P(x, y);

  // cardinality differs
  if (x->getCardMax() < y->getCardMin() ||
      y->getCardMax() < x->getCardMin()) {
    return P.vanish();
  }

  // glb(x) is_not_subseteq lub(y) -> entailed
  {
    OZ_FSetValue glb_x = x->getGlbSet();
    OZ_FSetValue lub_y = y->getLubSet();
  
    if(!(glb_x <= lub_y)) 
      return P.vanish();
  }

  // glb(y) is_not_subseteq lub(x) -> entailed
  {
    OZ_FSetValue glb_y = y->getGlbSet();
    OZ_FSetValue lub_x = x->getLubSet();
  
    if(!(glb_y <= lub_x)) 
      return P.vanish();
  }

  // x is value and x = glb(y) and glb(y) == union({e}, lub(y))
  if (x->isValue()) {
    OZ_FSetValue x_val = x->getGlbSet();
    OZ_FSetValue glb_y = y->getGlbSet();
    int card_glb_y = y->getGlbCard();
    int card_lub_y = y->getLubCard();

    if ((card_glb_y + 1) == card_lub_y) {
      if (x_val.getCard() == 1 && card_lub_y == 1) {
	FailOnInvalid(y->putCard(0, 0));
	return P.vanish();
      } else if ((x_val == glb_y) && (card_glb_y + 1 == card_lub_y)) {
	FailOnInvalid(y->putCard(card_lub_y, card_lub_y));
	return P.vanish();
      }
    }
  }

  // y is value and y = glb(x) and glb(x) == union({e}, lub(x))
  if (y->isValue()) {
    OZ_FSetValue y_val = y->getGlbSet();
    OZ_FSetValue glb_x = x->getGlbSet();
    int card_glb_x = x->getGlbCard();
    int card_lub_x = x->getLubCard();

    if ((card_glb_x + 1) == card_lub_x) {
      if (y_val.getCard() == 1 && card_lub_x == 1) {
	FailOnInvalid(x->putCard(0, 0));
	return P.vanish();
      } else if (y_val == glb_x) {
	FailOnInvalid(x->putCard(card_lub_x, card_lub_x));
	return P.vanish();
      }
    }
  }

  if (x->isValue() && y->isValue() && *x == *y) {
    goto failure;
  }
  
  return P.leave();
  
 failure:
  return P.fail();
}

//--------------------------------------------------------------------
// Set Difference Propagator (DENYS)

OZ_Return FSetDiffPropagator::propagate(void)
{
  OZ_FSetVar x(_x),y(_y),z(_z);
  PropagatorController_S_S_S P(x,y,z);
  FSetTouched xt,yt,zt;

  do {
    xt=x; yt=y; zt=z;
    // x-y=z
    //	disjoint(y,z)
    //	union(x,z)=union(y,z)
    //
    // disjoint(y,z)
    //
    FailOnInvalid(*y != *z);
    FailOnInvalid(*z != *y);
    //
    // union(x,y)=union(y,z)
    //	x subset union(y,z)
    //	z = x-y
    //	x supset z
    //	y supset x-z
    //
    FailOnInvalid(*x <= (*y | *z));
    FailOnInvalid(*z <<= (*x & - *y));
    FailOnInvalid(*x >= *z);
    FailOnInvalid(*y >= (*x & - *z));
  } while (xt <= x || yt <= y || zt <= z);

  return P.leave1();
failure:
  return P.fail();
}

OZ_PropagatorProfile FSetIntersectionPropagator::profile;
OZ_PropagatorProfile FSetUnionPropagator::profile;
OZ_PropagatorProfile FSetSubsumePropagator::profile;
OZ_PropagatorProfile FSetDisjointPropagator::profile;
OZ_PropagatorProfile FSetDistinctPropagator::profile;
OZ_PropagatorProfile FSetDiffPropagator::profile;

