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

#include "rel.hh"
#include "auxcomp.hh"
#include "rel_filter.hh"

//-----------------------------------------------------------------------------

OZ_Return NotEqOffPropagator::propagate(void)
{
  int &c = reg_c;

  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_y)) {
    return (0 == c) ? FAILED : PROCEED;
  }

  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);
  

  if (*x == fd_singl) {
    FailOnEmpty(*y -= (x->getSingleElem() - c));
    return P.vanish();
  }
  
  if (*y == fd_singl) {
    FailOnEmpty(*x -= (y->getSingleElem() + c));
    return P.vanish();
  }

  if ((x->getMaxElem() < y->getMinElem() + c) || 
      (y->getMaxElem() < x->getMinElem() - c)) {
    return P.vanish();
  }

  return P.leave();
  
failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

//#define TMUELLER

#ifdef TMUELLER
OZ_BI_define(fdp_lessEqOff, 3, 0)
{
  OZ_Expect pe;
  OZ_Return r;
  return make_lessEqOffset(r, pe, OZ_in(0), OZ_in(1), OZ_in(2));
}
OZ_BI_end

OZ_Return LessEqOffPropagator::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in ");
  //
  int &c = reg_c;
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);
  OZ_Filter<OZ_Propagator> s(this, &P);
  //
  return filter_lessEqOffset(s, x, y, c)();
}
#else
OZ_BI_define(fdp_lessEqOff, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_INT);
  //
  OZ_Expect pe;
  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectIntVarMinMax);
  OZ_EXPECT(pe, 2, expectInt);
  //
  return pe.impose(new LessEqOffset(OZ_in(0), OZ_in(1), OZ_intToC(OZ_in(2))));
}
OZ_BI_end

OZ_Return LessEqOffPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");
  
  int &c = reg_c;

  if (mayBeEqualVars() && OZ_isEqualVars(reg_x, reg_y)) {
    return (0 <= c) ? PROCEED : FAILED;
  }

  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);
  
  FailOnEmpty(*x <= (y->getMaxElem() + c));
  FailOnEmpty(*y >= (x->getMinElem() - c));

  if (x->getMaxElem() <= y->getMinElem() + c) return P.vanish();
  if (x->getMinElem() > y->getMaxElem() + c) goto failure;

  OZ_DEBUGPRINTTHIS("out ");
  
  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));
  
  return P.fail();
}
#endif

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_minimum, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);
  
  PropagatorExpect pe;
  int susp_count = 0;
  
  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new MinimumPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_Return MinimumPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(reg_x, reg_z),
		  replaceBy(new LessEqOffPropagator(reg_x, reg_y, 0)),
		  replaceBy(new LessEqOffPropagator(reg_y, reg_x, 0)));
  
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
  int lower_min = min(x->getMinElem(), y->getMinElem());
  int upper_min = min(x->getMaxElem(), y->getMaxElem());

  FailOnEmpty(*z >= lower_min);
  FailOnEmpty(*z <= upper_min);
  FailOnEmpty(*x >= z->getMinElem());
  FailOnEmpty(*y >= z->getMinElem());

  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_maximum, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);
  
  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarMinMax, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new MaximumPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_Return MaximumPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(reg_x, reg_z),
		  replaceBy(new LessEqOffPropagator(reg_y, reg_x, 0)),
		  replaceBy(new LessEqOffPropagator(reg_x, reg_y, 0)));
  
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
  int lower_max = max(x->getMinElem(), y->getMinElem());
  int upper_max = max(x->getMaxElem(), y->getMaxElem());

  FailOnEmpty(*z >= lower_max);
  FailOnEmpty(*z <= upper_max);
  FailOnEmpty(*x <= z->getMaxElem());
  FailOnEmpty(*y <= z->getMaxElem());

  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_inter, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);
  
  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarAny, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new IntersectionPropagator(OZ_in(0), OZ_in(1), 
					      OZ_in(2)));
}
OZ_BI_end

OZ_Return IntersectionPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(new SubSetPropagator(reg_z, reg_x)),
		  replaceBy(new SubSetPropagator(reg_x, reg_y)),
		  replaceBy(new SubSetPropagator(reg_y, reg_x)));
  
  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);

  FailOnEmpty(*z &= (*x & *y));

  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_union, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);
  
  PropagatorExpect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarAny, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectIntVarAny, susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new UnionPropagator(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end

OZ_Return UnionPropagator::propagate(void)
{
  SimplifyOnUnify(replaceBy(new SubSetPropagator(reg_z, reg_x)), 
		  PROCEED, PROCEED);

  OZ_FDIntVar x(reg_x), y(reg_y), z(reg_z);
  PropagatorController_V_V_V P(x, y, z);
  
  FailOnEmpty(*z &= (*x | *y)); // TMUELLER

  return P.leave();

failure: 
  return P.fail(); 
}

//-----------------------------------------------------------------------------

OZ_Return SubSetPropagator::propagate(void)
{
  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);
  
  if (OZ_isEqualVars(reg_x, reg_y)) {
    return P.vanish();
  } else {
    FailOnEmpty(*x &= *y);
  }
  
  return P.leave();
failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_distinct, 1, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD);
  
  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarSingl);

  return pe.impose(new DistinctPropagator(OZ_in(0)));
}
OZ_BI_end
 
OZ_Return DistinctPropagator::propagate(void)
{
  if (reg_l_sz < 2) 
    return PROCEED;

  int &sz  = reg_l_sz;

  DECL_DYN_ARRAY(OZ_FDIntVar, l, sz);
  PropagatorController_VV P(sz, l);
  OZ_FiniteDomain u(fd_empty); 
  int i;

  for (i = sz; i--; )
    l[i].read(reg_l[i]);

  if (hasEqualVars()) goto failure;
  
  for  (i = sz; i--; )
    if (*l[i] == fd_singl) {
      int s = l[i]->getSingleElem();
      if (u.isIn(s)) {
	goto failure;
      } else {
	u += s;
      }
    }

 loop:
  for (i = sz; i--; ) {
    if (*l[i] != fd_singl) {
      FailOnEmpty(*l[i] -= u);
      
      if (*l[i] == fd_singl) {
	u += l[i]->getSingleElem();
	goto loop;
      }
    }
  }

  int from, to;
  for (from = 0, to = 0; from < sz; from += 1)
    if (*l[from] != fd_singl)
      reg_l[to++] = reg_l[from];
  sz = to;

  /*
  for (i = to; i--; ) {
    f = f | *l[i];
    if (f.getSize() >= to) goto escape;
  }
  if (f.getSize() < to) goto failure;
  */

 escape:
  return P.leave();

failure:
  return P.fail();
}

/*

class DistinctStack {
private:
  int * _stack, _size, _top;
public: 
  DistinctStack(int s, int * st) : _stack(st), _size(s), _top(0) {}
  void push(int i) { _stack[_top++] = i; }
  int pop(void) {return _stack[--_top]; }
  int nonEmpty(void) { return _top; }
};

OZ_Return DistinctPropagator::propagate(void)
{
  if (reg_l_sz == 0) return PROCEED;

  int &sz  = reg_l_sz;

  DECL_DYN_ARRAY(OZ_FDIntVar, l, sz);
  PropagatorController_VV P(sz, l);
  OZ_FiniteDomain u(fd_empty);
  DECL_DYN_ARRAY(int, _stack, sz);
  DECL_DYN_ARRAY(int, init_size, sz);
  DistinctStack stack(sz, _stack);
  int i;

  for (i = sz; i--; ) {
    if ((init_size[i] = l[i].read(reg_l[i])) == 1)
      stack.push(i);
  }
  
  while (stack.nonEmpty()) {
    int skip = stack.pop();
    int remove = l[skip]->getSingleElem();

    for  (i = sz; i--; ) {
      if (i == skip) 
	continue;
      
      FailOnEmpty(*l[i] -= remove);

      if (*l[i] == fd_singl && init_size[i] > 1) {
	init_size[i] = 1;
	stack.push(i);
      }
    }
  }

  // remove singletons from list
  int from, to;
  for (from = 0, to = 0; from < sz; from += 1)
    if (*l[from] != fd_singl)
      reg_l[to++] = reg_l[from];
  sz = to;

  return P.leave();

failure:
  return P.fail();
}
 */

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_distinctOffset, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT);
  
  PropagatorExpect pe;
  
  OZ_EXPECT(pe, 0, expectVectorIntVarSingl);
  OZ_EXPECT(pe, 1, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  return pe.impose(new DistinctOffsetPropagator(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

OZ_Return DistinctOffsetPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  if (reg_sz == 0) return PROCEED;

  if (mayBeEqualVars()) {
    int * eq_vars = OZ_findEqualVars(reg_sz, reg_l);
    for (int i = reg_sz; i--; )
      if ((eq_vars[i] > -1) && 
	  (i != eq_vars[i]) && 
	  (reg_offset[i] == reg_offset[eq_vars[i]])
	  )
	return FAILED;
  }

  int &sz  = reg_sz;
  int * offset = reg_offset;

  DECL_DYN_ARRAY(OZ_FDIntVar, l, sz);
  PropagatorController_VV P(sz, l);
  OZ_FiniteDomain u(fd_empty);
  int i;

  for (i = sz; i--; )
    l[i].read(reg_l[i]);

loop:
  for (i=sz; i--; )
    if (*l[i] == fd_singl) {
      int s = offset[i]+l[i]->getSingleElem();
      for (int j=sz; j--; ) {
	if ( i != j) {
	  if (*l[j] != fd_singl) {
	    int tmp = s-offset[j];
	    if (tmp >= 0) { 
	      FailOnEmpty(*l[j] -= tmp);
	      if (*l[j] == fd_singl) goto loop;
	    }
	  }
	  else {
	    if (s == l[j]->getSingleElem()+ offset[j])
	      goto failure;
	  }
	}
      }
    }

  int from, to;
  for (from = 0, to = 0; from < sz; from += 1)
    if (*l[from] != fd_singl) {
      reg_l[to] = reg_l[from];
      offset[to++] = offset[from];
    }
  sz = to;
  

  OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------
// static members

OZ_PropagatorProfile NotEqOffPropagator::profile;
OZ_PropagatorProfile LessEqOffPropagator::profile;
OZ_PropagatorProfile MinimumPropagator::profile;
OZ_PropagatorProfile MaximumPropagator::profile;
OZ_PropagatorProfile IntersectionPropagator::profile;
OZ_PropagatorProfile UnionPropagator::profile;
OZ_PropagatorProfile DistinctPropagator::profile;
OZ_PropagatorProfile DistinctOffsetPropagator::profile;
OZ_PropagatorProfile SubSetPropagator::profile;

//-----------------------------------------------------------------------------
