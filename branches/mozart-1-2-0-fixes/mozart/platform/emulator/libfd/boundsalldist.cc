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
 * THIS IS EXPERIMENTAL, IN PARTICULAR THE SORTING AND ALL THAT!
 * AND USES O(n^2) RATHER THAN O(n log n)
 *
 * The algorithm is taken from:
 *   Jean-François Puget, A fast algorithm for the bound consistency 
 *   of alldiff constraints, Proceedings of the 15th National Conference 
 *   on Artificial Intelligence (AAAI-98), pages 359--366, 1998.
 *
 */

#ifdef USE_SORT_TEMPLATE
#include "sort.hh"
#endif

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

#ifdef USE_SORT_TEMPLATE
inline
Bool order_by_max_inc(const varinfo &x, const varinfo &y) {
  return x.max < y.max;
}

inline
Bool order_by_min_dec(const varinfo &x, const varinfo &y) {
  return x.min > y.min;
}
#else
inline
void swap_varinfo(varinfo * vi, int i, int j) {
  varinfo v=vi[i]; vi[i]=vi[j]; vi[j]=v;
}

static 
void sort_max(varinfo * x, int n)
{
 next:
  if (n < 3) {
    if ((n == 2) && (x[1].max < x[0].max))
      swap_varinfo(x,0,1);
  } else {
    int y = x[n >> 1].max;
    int i = -1;
    int j = n;
    
    while (1) {
      do j--; while (x[j].max > y);
      do i++; while (y > x[i].max);
      if (i >= j)
	break;
      swap_varinfo(x,i,j);
    };
  
    if (j < n-2) sort_max(x+j+1, n-j-1);
    n = j+1;
    goto next;
  }
}

static 
void sort_min(varinfo * x, int n)
{
 next:
  if (n < 3) {
    if ((n == 2) && (x[1].min > x[0].min))
      swap_varinfo(x,0,1);
  } else {
    int y = x[n >> 1].min;
    int i = -1;
    int j = n;
    
    while (1) {
      do j--; while (x[j].min < y);
      do i++; while (y < x[i].min);
      if (i >= j)
	break;
      swap_varinfo(x,i,j);
    };
  
    if (j < n-2) sort_min(x+j+1, n-j-1);
    n = j+1;
    goto next;
  }
}
#endif

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
#ifdef USE_SORT_TEMPLATE
  fastsort<varinfo,order_by_max_inc>(xi, n);
#else
  sort_max(xi, n);
#endif

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
#ifdef USE_SORT_TEMPLATE
  fastsort<varinfo,order_by_min_dec>(xi, n);
#else
  sort_min(xi, n);
#endif

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

    for (i = 0; i < n; i++) {
      if (xi[i].min == xi[i].max) {
	// This is a singleton
	// Invariant: the mins are sorted descending
	// That is:  j < i: xi[j].min > xi[i].min: no overlap possible
	// And:      j > i: xi[i].min > xi[j].min:
	//   Overlap, iff xi[j].max >= xi[i].min
	// Strictness, since they have been propagated!
	for (int j = i+1; j< n; j++) 
	  if (xi[j].max >= xi[i].min) 
	    goto next;
	elim = 1;
	reg_l[xi[i].pos] = (OZ_Term) NULL;
      }
    next: ;
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

