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

#include "telling.hh"

OZ_BI_define(fsp_include, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FSET);

  PropagatorExpect pe;

  int dummy = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, dummy);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, dummy);
  
  return pe.impose(new IncludePropagator(OZ_in(1),
					 OZ_in(0)));
} 
OZ_BI_end


OZ_BI_define(fsp_exclude, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FSET );

  PropagatorExpect pe;

  int dummy = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, dummy);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, dummy);
  
  return pe.impose(new ExcludePropagator(OZ_in(1),
					 OZ_in(0)));
} 
OZ_BI_end


OZ_BI_define(fsp_card, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  int dummy = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarAny, dummy);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, dummy);
  
  return pe.impose(new FSetCardPropagator(OZ_in(0),
					  OZ_in(1)));
} 
OZ_BI_end


//*****************************************************************************

OZ_Return IncludePropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  // the set must have at least 1 element
  // I don't know if the code below is sufficient:
  // CardRange does all kinds of weird stuff that I don't
  // understand and I don't know if I have to do this here too.
  if (!s->putCard(1,s->getCardMax())) goto failure;

  FailOnEmpty(*d <= (fsethigh32 - 1));
  
  if (*d == fd_singl) {
    FailOnInvalid(*s += d->getSingleElem());
  } else {

    // all elements which are known _not_ to be in `s' are not in `d' 
    OZ_FiniteDomain not_in(s->getNotInSet());
    FailOnEmpty(*d -= not_in);

    if (*d == fd_singl) 
      FailOnInvalid(*s += d->getSingleElem());

    // if `s' has at most 1 element, then it must range at most over
    // the domain of `d'
    if (s->getCardMax()<2) {
      OZ_FSetValue ds(*d);
      FailOnInvalid(*s <= ds);
    }
  }
  
  {
    OZ_FSetValue d_set(*d);
    OZ_Boolean ent = (d_set <= s->getGlbSet());
    
    OZ_DEBUGPRINTTHIS("out: ");
    
    return ent ? P.vanish() : P.leave(1);
  }
failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  return P.fail();
}

OZ_Return ExcludePropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);
  
  if (*d == fd_singl) {
    FailOnInvalid(*s -= d->getSingleElem());
  } else {

    // all elements which are known to be in `s' are not in `d' 
    OZ_FiniteDomain lb(s->getGlbSet());
    FailOnEmpty(*d -= lb);

    if (*d == fd_singl) 
      FailOnInvalid(*s -= d->getSingleElem());
  }
  { 
    OZ_FSetValue d_set(*d);
    OZ_Boolean ent = (d_set <= s->getNotInSet()); 
    
    OZ_DEBUGPRINTTHIS("out: ");
    
    return ent ? P.vanish() : P.leave(1);
  }
failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  return P.fail();
}

OZ_Return FSetCardPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  
  FailOnEmpty(*d >= s->getCardMin());
  FailOnEmpty(*d <= s->getCardMax());

  FailOnInvalid(s->putCard(d->getMinElem(), d->getMaxElem()));
  
  if (*d == fd_singl) {
    OZ_DEBUGPRINT(("entailed: %s %s",d->toString(),this->toString()));
    return P.vanish();
  }
    
  OZ_DEBUGPRINTTHIS("out: ");
  
  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  return P.fail();
}

OZ_PropagatorProfile IncludePropagator::profile;
OZ_PropagatorProfile ExcludePropagator::profile;
OZ_PropagatorProfile FSetCardPropagator::profile;
