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

#ifndef __SUM_FILTER_HH__
#define __SUM_FILTER_HH__

#define EXPECT(O, F, V, R) if (O.F(V, R)) return R;

template <class RETURN, class EXPECT, class VAR>
RETURN make_lessEqOffsetN(RETURN r, EXPECT &pe, VAR a, VAR x, VAR c)
{
  EXPECT(pe, expectVectorInt, a, r);
  EXPECT(pe, expectVectorIntVarBounds, x, r);
  EXPECT(pe, expectIntVarBounds, c, r);
  //
  return pe.impose(new LessEqOffsetN(a, x, c));
}

template <class SERVICE, class FDVARVECTOR>
SERVICE &filter_lessEqOffsetN(SERVICE &s, int * a, FDVARVECTOR &x, int &c)
{
  int n = x.getHigh();
  DECL_DYN_ARRAY(int, pa, n);
  x.find_equals(pa);
  int i;
  for (i = n; i --; ) {
    if (a[i] == 0) {
      x[i].dropParameter();
    } else if (pa[i] == -1) {
      // found an integer
      c += (a[i] * x[i]->getSingleElem());
      x[i].dropParameter();
    } else if (pa[i] != i) {
      // found next occurence of a parameter
      a[pa[i]] += a[i];
      x[i].dropParameter();
    }
  }
  //
  n = x.compress(a);
  //
  if (n == 0) {
    return (c <= 0) ? s.entail() : s.fail();
  }
  // if possible reduce to ternary propagator 
  if (n == 2) {
    if ((a[0] == 1) && (a[1] == -1)) {
      return s.replace_propagator(new LessEqOffset(x[0], x[1], -c));
    } else if ((a[0] == -1) && (a[1] == 1)) {
      return s.replace_propagator(new LessEqOffset(x[1], x[0], -c));
    }
  }
  //
  DECL_DYN_ARRAY(double, v, n);
  DECL_DYN_ARRAY(double, w, n);
  //
  v[0] = 0;
  for (i = 1; i < n; i += 1) {
    double x_i = a[i-1] > 0 ? x[i-1]->getMinElem() : x[i-1]->getMaxElem();
    v[i] = v[i-1] + a[i-1] * x_i;
  }
  //
  w[n-1] = 0;
  for (i = n; i > 1; i -= 1) {
    double x_i = a[i-1] > 0 ? x[i-1]->getMinElem() : x[i-1]->getMaxElem();
    w[i-2] = w[i-1] + a[i-1] * x_i;
  }
  //
  double sum = c;
  //
  for (i = 0; i < n; i += 1) {
    if (a[i] > 0) {
      int ub = doubleToInt(floor(double(v[i]+w[i]+c)/-a[i]));
      FailOnEmpty(*x[i] <= ub);
      sum += a[i] * x[i]->getMaxElem();
    } else {
      int lb = doubleToInt(ceil(double(v[i]+w[i]+c)/-a[i]));
      FailOnEmpty(*x[i] >= lb);
      sum += a[i] * x[i]->getMinElem();
    }
  }
  //
  return (sum <= 0) ? s.entail() : s;
  //
failure:
  //
  return s.fail();
}

#endif /* __SUM_FILTER_HH__ */
