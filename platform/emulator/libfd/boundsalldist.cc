/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 2001
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

/*
 * THIS IS EXPERIMENTAL AND USES O(n^2) RATHER THAN O(n log n)
 *
 * The algorithm is taken from:
 *   Jean-François Puget, A fast algorithm for the bound consistency 
 *   of alldiff constraints, Proceedings of the 15th National Conference 
 *   on Artificial Intelligence (AAAI-98), pages 359--366, 1998.
 *
 */

#include "sort.hh"

#include "boundsalldist.hh"

OZ_BI_define(fdp_distinctB, 1, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD);
  
  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);

  return pe.impose(new BoundsDistinctPropagator(OZ_in(0)));
}
OZ_BI_end

class varinfo {
public:
  int min;
  int max;
  int pos;
};

class Order_VarInfo_By_Max_Inc {
public:
  Bool operator()(const varinfo& x, const varinfo& y) {
    return x.max < y.max;
  }
};

class Order_VarInfo_By_Min_Dec {
public:
  Bool operator()(const varinfo& x, const varinfo& y) {
    return x.min > y.min;
  }
};


OZ_Return BoundsDistinctPropagator::propagate(void)
{
  int &n = reg_l_sz;
  
  if (n < 2)
    return PROCEED;
  
  if (hasEqualVars())
    return FAILED;

  // Do bounds propagation
  static const int no_max = OZ_getFDSup() + 1;
  static const int no_min = OZ_getFDInf() - 1;

  DECL_DYN_ARRAY(OZ_FDIntVar, x, n);
  PropagatorController_VV P(n, x);
  DECL_DYN_ARRAY(varinfo,xi,n);
  DECL_DYN_ARRAY(int,u,n);

  int i;


  // Get access to variables and set up variable infos
  for (i = n; i--; ) {
    x[i].read(reg_l[i]);
    xi[i].min = x[i]->getMinElem();
    xi[i].max = x[i]->getMaxElem();
    xi[i].pos = i;
  }
  
  // Sort variables in ascending order of max
  Order_VarInfo_By_Max_Inc ilt;
  fastsort(xi, n, ilt);

  // Propagate lower bounds
  for (i = 0; i < n; i++) {
    int bmm  = no_max;
    u[i] = xi[i].min;
    for (int j = 0; j < i; j++)
      if (xi[j].min < xi[i].min) {
	u[j]++;
	if (u[j] > xi[i].max)
	  goto failure;
	if ((u[j] == xi[i].max) && (xi[j].min < bmm))
	  bmm = xi[j].min;
      } else {
	u[i]++;
      }
    if (u[i] > xi[i].max)
      goto failure;
    if ((u[i] == xi[i].max) && (xi[i].min < bmm))
      bmm = xi[i].min;
    if (bmm < no_max) {
      int b = xi[i].max+1;
      for (int k = i+1; k < n; k++)
	if (xi[k].min >= bmm) {
	  FailOnEmpty(*(x[xi[k].pos]) >= b);
	  xi[k].min = x[xi[k].pos]->getMinElem();
	}
    }
  }

  // Sort variables in descending order of min
  Order_VarInfo_By_Min_Dec dlt;
  fastsort(xi, n, dlt);

  // Propagate upper bounds
  for (i = 0; i < n; i++) {
    int bmm = no_min;
    u[i] = xi[i].max;
    for (int j = 0; j < i; j++)
      if (xi[j].max > xi[i].max) {
	u[j]--;
	if (u[j] < xi[i].min)
	  goto failure;
	if ((u[j] == xi[i].min) && (xi[j].max > bmm))
	  bmm = xi[j].max;
      } else {
	u[i]--;
      }
    if (u[i] < xi[i].min)
      goto failure;
    if ((u[i] == xi[i].min) && (xi[i].max > bmm))
      bmm = xi[i].max;
    if (bmm > no_min) {
      int b = xi[i].min-1;
      for (int k = i+1; k < n; k++)
	if (xi[k].max <= bmm) {
	  FailOnEmpty(*(x[xi[k].pos]) <= b);
	  xi[k].max = x[xi[k].pos]->getMaxElem();
	}
    }
  }

  // Eliminate singletons
  {
    int elim = 0;
    int max  = xi[n-1].max;
    int pre  = n-1;
    for (i = n-1; i--; ) {
      if (xi[i].min > max) {
	if (pre == i+1) {
	  elim = 1;
	  reg_l[xi[i+1].pos] = (OZ_Term) NULL;
	}
	pre = i;
	max = xi[i].max;
      } else if (xi[i].max > max) {
	max = xi[i].max;
      }
    }
    if (pre == 0) {
      elim = 1;
      reg_l[xi[0].pos] = (OZ_Term) NULL;
    }
    
    if (elim) {
      int f = 0;
      while (reg_l[f]) { f++; }
      int t = f++;
      while (f < n) {
	if (reg_l[f])
	  reg_l[t++] = reg_l[f];
	f++;
      }
      n = t;
    }
    
  }
  
  return P.leave();
  
 failure:
  return P.fail();
}

OZ_PropagatorProfile BoundsDistinctPropagator::profile;

