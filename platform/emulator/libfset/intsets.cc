/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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

#include "intsets.hh"

// for seq/propagation, we can do better than to propagate the
// known information: we can propagate necessary bounds even on
// the unknown stuff.  For example, if a set var has cardinality
// at least N and its LUB is {i1 i2 ... iN ...}, then its maximum
// element (when N>0) is at least iN, which means that the next
// set in the sequence must be above that limit.  Similarly in
// the other direction.  For this reason, we define the auxiliary
// functions FSetGetLowerBoundOfMax and FSetGetUpperBoundOfMin.

// Here we approximate the max element in a set.
// It must be at least as large as the largest element
// in the GLB.  However, if the cardinality is at least
// N, then it must be at least as large as the Nth smallest
// element in the LUB, which could be larger than the
// largest element currently in the GLB.
//
// Note: if the set _could_ be empty, then this _must_ return -1

int FSetGetLowerBoundOfMax(OZ_FSetConstraint& c)
{
  // just in case we _know_ that there are no elements
  if (c.getCardMax()<=0) return -1;
  // else: the set must contain at least i elements
  int i = c.getCardMin();
  // if i==0: we don't know anything about cardinality.
  // the set could be empty.  we expect GLB to still be
  // empty, but just in case it's not, we guess its
  // max element (-1 if empty).
  if (i==0) return c.getGlbMaxElem();
  // else, we start counting upwards from the min element
  // of LUB
  int n = c.getLubMinElem();
  while (--i) {
    n=c.getLubNextLargerElem(n);
    // in case we run out of elements (should not happen)
    if (n<0) break;
  }
  return max(n,c.getGlbMaxElem());
}

// Similarly for the min element

int FSetGetUpperBoundOfMin(OZ_FSetConstraint& c)
{
  // just in case there can be no elements
  if (c.getCardMax()<=0) return -1;
  // else the set must contain at least i elements
  int i = c.getCardMin();
  // if i==0: we know nothing about cardinality. the set
  // could be empty.  we expect GLB to still be empty,
  // but, just in case it's not, we guess its min element
  // (-1 if empty)
  if (i==0) return c.getGlbMinElem();
  // else we start counting downward from the max element
  // of LUB
  int n = c.getLubMaxElem();
  while (--i) {
    n=c.getLubNextSmallerElem(n);
    // in case we run out of elements (should not happen)
    if (n<0) break;
  }
  int m = c.getGlbMinElem();
  return (m<0)?n:((n<0)?m:min(n,m));
}

// FSP_MIN WAS WRITTEN BY TOBIAS AS AN EXAMPLE

OZ_BI_define(fsp_min, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);

  int susp_count_dummy;
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count_dummy);

  return pe.impose(new FSetsMinPropagator(OZ_in(0),
					  OZ_in(1)));
}
OZ_BI_end


OZ_Return FSetsMinPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  // DENYS: d < 32*fset_high
  FailOnEmpty(*d <= (fsethigh32 - 1));

  // card(s) > 0
  FailOnInvalid(s->putCard(1, fsethigh32));

  {
    // i < min(d) ==> i not in s
    // remove all elements from `s' that are less than the minimal
    // element of `d'
    FailOnInvalid(s->ge(d->getMinElem()));

    // i in s ==> min(d) <= i
    // `d' is less or equal to the smallest element of `glb(s)'
    {
      int i = s->getGlbMinElem();
      // i==-1 if glb(s) is still empty
      if (i>=0) { FailOnEmpty(*d <= i); }
    }

    // DENYS: i not in s ==> d=/=i
    // all elements being _not_ in `s' are not in `d'
    OZ_FiniteDomain not_in(s->getNotInSet());
    FailOnEmpty(*d -= not_in);
  }

  {
    int i = d->getSingleElem();
    if (i != -1)
      {
	// d is in s
	FailOnInvalid(*s += i);
      }
    else
      {
	// d <= upper_bound_of_min(s)
	// s is not empty, thus upper_bound_of_min(s) is not -1
	FailOnEmpty(*d <= FSetGetUpperBoundOfMin(*s));
      }
  }

  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave(1);

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  return P.fail();
}

// -------------------------------------------------------------------
// Max Propagator

OZ_BI_define(fsp_max, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);

  int susp_count_dummy;
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count_dummy);

  return pe.impose(new FSetsMaxPropagator(OZ_in(0),
					  OZ_in(1)));
}
OZ_BI_end


OZ_Return FSetsMaxPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  // d < 32*fset_high
  FailOnEmpty(*d <= (fsethigh32 - 1));

  // card(s) > 0
  FailOnInvalid(s->putCard(1, fsethigh32));

  {
    // i > max(d) ==> i not in s
    // remove all elements from `s' that are greater than the maximal
    // element of `d'
    FailOnInvalid(s->le(d->getMaxElem()));

    // i in s ==> max(d) >= i
    // `d' is greater or equal to the largest element of `glb(s)'
    FailOnEmpty(*d >= s->getGlbMaxElem());

    // i not in s ==> d=/=i
    // all elements being _not_ in `s' are not in `d'
    OZ_FiniteDomain not_in(s->getNotInSet());
    FailOnEmpty(*d -= not_in);
  }

  {
    int i = d->getSingleElem();
    if (i != -1)
      {
	// d is in s
	FailOnInvalid(*s += i);
      }
    else
      {
	// d >= lower_bound_of_max(s)
	// s is not empty, thus lower_bound_of_max(s) is not -1
	FailOnEmpty(*d >= FSetGetLowerBoundOfMax(*s));
      }
  }

  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave(1);

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  return P.fail();
}

//--------------------------------------------------------------------
// Convex Propagator

OZ_BI_define(fsp_convex, 1, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET);

  PropagatorExpect pe;
  OZ_EXPECT(pe,0,expectFSetVarAny);
  return pe.impose(new FSetsConvexPropagator(OZ_in(0)));
}
OZ_BI_end


OZ_Return FSetsConvexPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  OZ_FSetVar s(_s);

  // an empty set is convex (per definition)
  if (!s->isEmpty()) {

    OZ_DEBUGPRINT(("a"));

    // find minimal and maximal element in glb and fill it up
    int min_in = s->getGlbMinElem();
    // lower bound is not empty

    OZ_DEBUGPRINT(("min_in=%d", min_in));

    if (min_in != -1) {
      int max_in = s->getGlbMaxElem();
      OZ_ASSERT(max_in != -1);

      OZ_FSetValue fillup(min_in, max_in);
      FailOnInvalid(*s <<= *s | fillup);

      OZ_DEBUGPRINT(("b"));

      // find next smaller element to minimal element in glb not contained
      // and remove all elements starting from this element to `inf'
      {
	int next_smaller = s->getNotInNextSmallerElem(min_in);
	if (next_smaller>=0) FailOnInvalid(s->ge(next_smaller));
      }

      OZ_DEBUGPRINT(("c"));

      // find next larger element to maximal element in glb not contained
      // and remove all elements starting from this element to `sup'
      {
	int next_larger = s->getNotInNextLargerElem(max_in);
	if (next_larger>=0) FailOnInvalid(s->le(next_larger));
      }

      // the largest element is at most getCardMax away from the
      // smallest known element
      FailOnInvalid(s->le(min_in+s->getCardMax()-1));
      // similarly for the smallest element
      {
	int k = max_in-s->getCardMax()+1;
	if (k>0) { FailOnInvalid(s->ge(k)); }
      }
    }

    OZ_DEBUGPRINT(("d"));
  }

  OZ_DEBUGPRINTTHIS("out: ");

  return s.leave() ? OZ_SLEEP : OZ_ENTAILED;

failure:
  OZ_DEBUGPRINTTHIS("fail: ");

  s.fail();
  return FAILED;
}

//-----------------------------------------------------------------------------
// match propagator

OZ_BI_define(fsp_match, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);

  return pe.impose(new FSetMatchPropagator(OZ_in(0),
					   OZ_in(1)));
}
OZ_BI_end


// effects match, minN, and maxN
#define MATCH_NOLOOP

OZ_Return FSetMatchPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("\nin ");

  OZ_FSetVar s(_s);
  DECL_DYN_ARRAY(OZ_FDIntVar, vd, _vd_size);
  PropagatorController_S_VD P(s, _vd_size, vd);
  int max_fd = OZ_getFDInf();
  int min_fd = OZ_getFDSup();
  int i;

  for (i = _vd_size; i--; ) {
    vd[i].read(_vd[i]);
    min_fd = min(min_fd, vd[i]->getMinElem());
    max_fd = max(max_fd, vd[i]->getMaxElem());
  }

  if (_firsttime) {
    OZ_DEBUGPRINT(("firsttime==1"));

    _firsttime = 0; // do it only once

    _k = 0; _l = _vd_size - 1;
    _last_min = s->getLubMinElem() - 1;
    _last_max = s->getLubMaxElem() + 1;

    // (1)
    //
    FailOnInvalid(s->putCard(_vd_size, _vd_size));
    OZ_DEBUGPRINTTHIS("(1) ");
  }

#ifndef  MATCH_NOLOOP
  int old_size, new_size;
  FSetTouched st;

  for (old_size = 0, i = _k; i <= _l; i += 1)
    old_size += vd[i]->getSize();
#endif

loop:
  OZ_DEBUGPRINT(("_k=%d _l=%d _last_min=%d _last_max=%d min_fd=%d max_fd=%d",
		  _k, _l , _last_min,  _last_max, min_fd,  max_fd));

#ifndef MATCH_NOLOOP
  st = s;
#endif

  {
    // (2)
    FailOnEmpty(*vd[_k] >= _last_min + 1);
    for (i = _k; i < _l; i += 1) {
      FailOnEmpty(*vd[i + 1] >= vd[i]->getMinElem() + 1);
    }
    FailOnEmpty(*vd[_l] <= _last_max - 1);
    for (i = _l; i > _k; i -= 1) {
      FailOnEmpty(*vd[i - 1] <= vd[i]->getMaxElem() - 1);
    }
    OZ_DEBUGPRINTTHIS("(2) ");
  }
  {
    // (3)
    OZ_DEBUGPRINT(("_k=%d _l=%d",_k, _l));

    if (_k == 0) {
      OZ_FSetValue remove_elems(OZ_getFSetInf(), vd[0]->getMinElem() - 1);
      FailOnInvalid(*s <<= (*s - remove_elems));
    } else {
      OZ_FSetValue remove_elems(vd[_k - 1]->getMaxElem() + 1,
				vd[_k]->getMinElem() - 1);
      FailOnInvalid(*s <<= (*s - remove_elems));
    }

    if (_l == _vd_size - 1) {
      OZ_FSetValue remove_elems(vd[_l]->getMaxElem() + 1, OZ_getFSetSup());
      FailOnInvalid(*s <<= (*s - remove_elems));
    } else {
      OZ_FSetValue remove_elems(vd[_l]->getMaxElem() + 1,
				vd[_l + 1]->getMinElem() - 1);
      FailOnInvalid(*s <<= (*s - remove_elems));
    }

    OZ_DEBUGPRINTTHIS("(3) ");
  }


  {
    // (4)
    for (i = _k; i <= _l; i += 1)
      if (*vd[i] == fd_singl)
	FailOnInvalid(*s += vd[i]->getMinElem());

    OZ_DEBUGPRINTTHIS("(4) ");
  }

  {
    // (5)
    OZ_FSetValue glb_s = s->getGlbSet(), lub_s = s->getLubSet();
    FSetIterator glb_it(&glb_s, _last_min), lub_it(&lub_s, _last_min);

    int min_lub = lub_it.getNextLarger(), min_glb = glb_it.getNextLarger();
    for ( ; min_lub == min_glb && min_lub != -1;
	  min_lub = lub_it.getNextLarger(),
	    min_glb = glb_it.getNextLarger(), _k += 1 ) {
      FailOnEmpty(*vd[_k] &= min_glb);
      _last_min = min_lub;

      OZ_DEBUGPRINTTHIS("(5) ");
    }

    // (6)
    if (_k != _l) {
      lub_it.init(_last_max);
      glb_it.init(_last_max);
      int max_lub = lub_it.getNextSmaller(), max_glb = glb_it.getNextSmaller();
      for ( ; max_lub == max_glb && max_lub != -1;
	    max_lub = lub_it.getNextSmaller(),
	      max_glb = glb_it.getNextSmaller(), _l -= 1) {
	FailOnEmpty(*vd[_l] &= max_glb);
	_last_max = max_lub;
      }

      OZ_DEBUGPRINTTHIS("(6) ");
    }
  }

#ifndef MATCH_NOLOOP
  for (new_size = 0, i = _k; i <= _l; i += 1)
    new_size += vd[i]->getSize();

  if (((old_size != new_size && new_size > _vd_size) || st <= s) &&
      _k < _vd_size && _l > -1) {
    old_size = new_size;
    goto loop;
  }
#endif

  OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail: ");

  return P.fail();
}

//-----------------------------------------------------------------------------
// minN propagator

OZ_BI_define(fsp_minN, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);

  return pe.impose(new FSetMinNPropagator(OZ_in(0),
					  OZ_in(1)));
}
OZ_BI_end


OZ_Return FSetMinNPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("\nin ");

  OZ_FSetVar s(_s);
  DECL_DYN_ARRAY(OZ_FDIntVar, vd, _vd_size);
  PropagatorController_S_VD P(s, _vd_size, vd);
  int max_fd = OZ_getFDInf();
  int min_fd = OZ_getFDSup();
  int i;

  for (i = _vd_size; i--; ) {
    vd[i].read(_vd[i]);
    min_fd = min(min_fd, vd[i]->getMinElem());
    max_fd = max(max_fd, vd[i]->getMaxElem());
  }

  if (_firsttime) {
    OZ_DEBUGPRINT(("firsttime==1"));

    _firsttime = 0; // do it only once

    _k = 0; _l = _vd_size - 1;
    _last_min = s->getLubMinElem() - 1;
    _last_max = s->getLubMaxElem() + 1;

    // (1)
    //
    FailOnInvalid(s->putCard(_vd_size, fsethigh32));
    OZ_DEBUGPRINTTHIS("(1) ");
  }

#ifndef  MATCH_NOLOOP
  int old_size, new_size;
  FSetTouched st;

  for (old_size = 0, i = _k; i <= _l; i += 1)
    old_size += vd[i]->getSize();
#endif

loop:
  OZ_DEBUGPRINT(("_k=%d _l=%d _last_min=%d _last_max=%d min_fd=%d max_fd=%d",
		  _k, _l , _last_min,  _last_max, min_fd,  max_fd));

#ifndef MATCH_NOLOOP
  st = s;
#endif

  {
    // (2)
    FailOnEmpty(*vd[_k] >= _last_min + 1);
    for (i = _k; i < _l; i += 1) {
      FailOnEmpty(*vd[i + 1] >= vd[i]->getMinElem() + 1);
    }

    FailOnEmpty(*vd[_l] <= _last_max - 1);
    for (i = _l; i > _k; i -= 1) {
      FailOnEmpty(*vd[i - 1] <= vd[i]->getMaxElem() - 1);
    }

    OZ_DEBUGPRINTTHIS("(2) ");
  }
  {
    // (3)
    OZ_DEBUGPRINT(("_k=%d _l=%d",_k, _l));

    /*
    if (_k == 0) { // TMUELLER
      for (i = OZ_getFSetInf(); i < vd[0]->getMinElem(); i += 1)
	FailOnInvalid(*s -= i);
    } else {
      for (i = vd[_k - 1]->getMaxElem() + 1; i < vd[_k]->getMinElem(); i += 1)
	FailOnInvalid(*s -= i);
    }
    */

    if (_k == 0) {
      OZ_FSetValue remove_elems(OZ_getFSetInf(), vd[0]->getMinElem() - 1);
      FailOnInvalid(*s <<= *s - remove_elems);
    } else {
      OZ_FSetValue remove_elems(vd[_k - 1]->getMaxElem() + 1,
				vd[_k]->getMinElem() - 1);
      FailOnInvalid(*s <<= *s - remove_elems);
    }

    OZ_DEBUGPRINTTHIS("(3) ");
  }

  {
    // (4)
    for (i = _k; i <= _l; i += 1)
      if (*vd[i] == fd_singl)
	FailOnInvalid(*s += vd[i]->getMinElem());

    OZ_DEBUGPRINTTHIS("(4) ");
  }

  {
    // (5)
    OZ_FSetValue glb_s = s->getGlbSet(), lub_s = s->getLubSet();
    FSetIterator glb_it(&glb_s, _last_min), lub_it(&lub_s, _last_min);

    int min_lub = lub_it.getNextLarger(), min_glb = glb_it.getNextLarger();
    for ( ; min_lub == min_glb && min_lub != -1;
	  min_lub = lub_it.getNextLarger(),
	    min_glb = glb_it.getNextLarger(), _k += 1 ) {
      FailOnEmpty(*vd[_k] &= min_glb);
      _last_min = min_lub;

      OZ_DEBUGPRINTTHIS("(5) ");
    }
  }

#ifndef MATCH_NOLOOP
  for (new_size = 0, i = _k; i <= _l; i += 1)
    new_size += vd[i]->getSize();

  if (((old_size != new_size && new_size > _vd_size) || st <= s) &&
      _k < _vd_size && _l > -1) {
    old_size = new_size;
    goto loop;
  }
#endif

  OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail: ");

  return P.fail();
}

//-----------------------------------------------------------------------------
// maxN propagator

OZ_BI_define(fsp_maxN, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);

  return pe.impose(new FSetMaxNPropagator(OZ_in(0),
					  OZ_in(1)));
}
OZ_BI_end


#define MATCH_NOLOOP

OZ_Return FSetMaxNPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("\nin ");

  OZ_FSetVar s(_s);
  DECL_DYN_ARRAY(OZ_FDIntVar, vd, _vd_size);
  PropagatorController_S_VD P(s, _vd_size, vd);
  int max_fd = OZ_getFDInf();
  int min_fd = OZ_getFDSup();
  int i;

  for (i = _vd_size; i--; ) {
    vd[i].read(_vd[i]);
    min_fd = min(min_fd, vd[i]->getMinElem());
    max_fd = max(max_fd, vd[i]->getMaxElem());
  }

  if (_firsttime) {
    OZ_DEBUGPRINT(("firsttime==1"));

    _firsttime = 0; // do it only once

    _k = 0; _l = _vd_size - 1;
    _last_min = s->getLubMinElem() - 1;
    _last_max = s->getLubMaxElem() + 1;

    // (1)
    //
    FailOnInvalid(s->putCard(_vd_size, fsethigh32));
    OZ_DEBUGPRINTTHIS("(1) ");
  }

#ifndef  MATCH_NOLOOP
  int old_size, new_size;
  FSetTouched st;

  for (old_size = 0, i = _k; i <= _l; i += 1)
    old_size += vd[i]->getSize();
#endif

loop:
  OZ_DEBUGPRINT(("_k=%d _l=%d _last_min=%d _last_max=%d min_fd=%d max_fd=%d",
		  _k, _l , _last_min,  _last_max, min_fd,  max_fd));

#ifndef MATCH_NOLOOP
  st = s;
#endif

  {
    // (2)
    FailOnEmpty(*vd[_k] >= _last_min + 1);
    for (i = _k; i < _l; i += 1) {
      FailOnEmpty(*vd[i + 1] >= vd[i]->getMinElem() + 1);
    }
    FailOnEmpty(*vd[_l] <= _last_max - 1);
    for (i = _l; i > _k; i -= 1) {
      FailOnEmpty(*vd[i - 1] <= vd[i]->getMaxElem() - 1);
    }
    OZ_DEBUGPRINTTHIS("(2) ");
  }
  {
    // (3)
    OZ_DEBUGPRINT(("_k=%d _l=%d",_k, _l));

    if (_l == _vd_size - 1) {
      OZ_FSetValue remove_elems(vd[_l]->getMaxElem() + 1, OZ_getFSetSup());
      FailOnInvalid(*s <<= *s - remove_elems);
    } else {
      OZ_FSetValue remove_elems(vd[_l]->getMaxElem() + 1,
				vd[_l + 1]->getMinElem() - 1);
      FailOnInvalid(*s <<= *s = remove_elems);
    }

    OZ_DEBUGPRINTTHIS("(3) ");
  }

  {
    // (4)
    for (i = _k; i <= _l; i += 1)
      if (*vd[i] == fd_singl)
	FailOnInvalid(*s += vd[i]->getMinElem());

    OZ_DEBUGPRINTTHIS("(4) ");
  }

  {
    OZ_FSetValue glb_s = s->getGlbSet(), lub_s = s->getLubSet();
    FSetIterator glb_it(&glb_s, _last_min), lub_it(&lub_s, _last_min);

    // (6)
    if (_k != _l) {
      lub_it.init(_last_max);
      glb_it.init(_last_max);
      int max_lub = lub_it.getNextSmaller(), max_glb = glb_it.getNextSmaller();
      for ( ; max_lub == max_glb && max_lub != -1;
	    max_lub = lub_it.getNextSmaller(),
	      max_glb = glb_it.getNextSmaller(), _l -= 1) {
	FailOnEmpty(*vd[_l] &= max_glb);
	_last_max = max_lub;
      }

      OZ_DEBUGPRINTTHIS("(6) ");
    }
  }

#ifndef MATCH_NOLOOP
  for (new_size = 0, i = _k; i <= _l; i += 1)
    new_size += vd[i]->getSize();

  if (((old_size != new_size && new_size > _vd_size) || st <= s) &&
      _k < _vd_size && _l > -1) {
    old_size = new_size;
    goto loop;
  }
#endif

  OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail: ");

  return P.fail();
}


//-----------------------------------------------------------------------------
// seq propagator

OZ_BI_define(fsp_seq, 1, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FSET);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorFSetVarBounds);

  return pe.impose(new FSetSeqPropagator(OZ_in(0)));
}
OZ_BI_end

OZ_Return FSetSeqPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  DECL_DYN_ARRAY(OZ_FSetVar, vs, _vs_size);
  PropagatorController_VS P(_vs_size, vs);
  int i;

  for (i = _vs_size; i--; )
    vs[i].read(_vs[i]);

  {
    int lb_max = -1;

    for (i = 0; i < _vs_size - 1; i += 1) {
      lb_max = max(lb_max,FSetGetLowerBoundOfMax(*vs[i]));

      OZ_DEBUGPRINT(("%d", lb_max));

      // there is no maximal element in the lower bound so far
      if (lb_max == -1)
	continue;

      FailOnInvalid(vs[i+1]->ge(lb_max + 1));

      OZ_DEBUGPRINT(("%d %s > %d\n", i, vs[i]->toString(), lb_max));
    }

    OZ_DEBUGPRINTTHIS("after #1 ");
  }

  {
    int sup1 = OZ_getFSetSup() + 1;
    int lb_min = sup1;

    for (i = _vs_size - 1; i > 0; i -= 1) {
      int lb_min_tmp = FSetGetUpperBoundOfMin(*vs[i]);
      if (lb_min_tmp>=0) lb_min=min(lb_min,lb_min_tmp);

      OZ_DEBUGPRINT(("%d", lb_min));

      // there is no minimal element in the lower bound so far
      if (lb_min == sup1)
	continue;

      FailOnInvalid(vs[i-1]->le(lb_min - 1));

     OZ_DEBUGPRINT(("#2 %d %s < %d\n", i, vs[i]->toString(), lb_min));
    }
  }

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("failed");
  return P.fail();
}

OZ_PropagatorProfile FSetsMinPropagator::profile;
OZ_PropagatorProfile FSetsMaxPropagator::profile;
OZ_PropagatorProfile FSetsConvexPropagator::profile;
OZ_PropagatorProfile FSetMatchPropagator::profile;
OZ_PropagatorProfile FSetMinNPropagator::profile;
OZ_PropagatorProfile FSetMaxNPropagator::profile;
OZ_PropagatorProfile FSetSeqPropagator::profile;

//-----------------------------------------------------------------------------
// eof
