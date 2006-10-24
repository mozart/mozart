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

#ifndef __REL_FILTER__
#define __REL_FILTER__

#define EXPECT(O, F, V, R) if (O.F(V, R)) return R;

template <class RETURN, class EXPECT, class VAR1, class VAR2>
RETURN make_lessEqOffset(RETURN r, EXPECT &pe, VAR1 x, VAR1 y, VAR2 c)
{
  EXPECT(pe, expectIntVarBounds, x, r);
  EXPECT(pe, expectIntVarBounds, y, r);
  EXPECT(pe, expectInt, c, r);
  //
  return pe.impose(new LessEqOffset(x, y, OZ_intToC(c)));
}


template  <class SERVICE, class FDVAR>
SERVICE &filter_lessEqOffset(SERVICE & s, FDVAR &x, FDVAR &y, int c)
{
  if (x == y) {
    return  (0 <= c) ? s.entail() : s.fail();
  }
  //
  FailOnEmpty(*x <= (y->getMaxElem() + c));
  FailOnEmpty(*y >= (x->getMinElem() - c));
  //
  if (x->getMaxElem() <= y->getMinElem() + c) {
    return s.entail();
  }
  if (x->getMinElem() > y->getMaxElem() + c) {
    return s.fail();
  }
  //
  return s.leave();
 failure:
  return s.fail();
}

#endif
