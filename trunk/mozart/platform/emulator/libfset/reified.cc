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

#include <limits.h>
#include "reified.hh"

#ifdef PROFILE
#define inline
#endif

//-----------------------------------------------------------------------------
OZ_C_proc_begin(fsp_includeR, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectFSetVarBounds);
  OZ_EXPECT(pe, 2, expectIntVarAny);
  
  return pe.impose(new IncludeRPropagator(OZ_args[1],
					  OZ_args[0],
					  OZ_args[2]));
} 
OZ_C_proc_end

OZ_CFunHeader IncludeRPropagator::header = fsp_includeR;

//*****************************************************************************

OZ_Return IncludeRPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FDIntVar r(_r);

  if (*r == fd_singl) {
    r.leave();
    return replaceBy((r->getSingleElem() == 1)
		     ? (OZ_Propagator*) new IncludePropagator(_s, _d)
		     : (OZ_Propagator*) new ExcludePropagator(_s, _d));
  }

  int r_val = 0;
  OZ_FSetVar s;
  OZ_FDIntVar d;

  s.readEncap(_s);
  d.readEncap(_d);
  
  {
    FailOnEmpty(*d <= (32 * fset_high - 1));
    
    if (*d == fd_singl) {
      FailOnInvalid(*s += d->getSingleElem());
    } else {
      
      for (int i = 32 * fset_high; i --; )
	if (s->isNotIn(i))
	  FailOnEmpty(*d -= i);
      
      if (*d == fd_singl) 
	FailOnInvalid(*s += d->getSingleElem());
    }
    if (!s.isTouched() && !d.isTouched()) {
      r_val = 1;
      goto entailment;
    }
  }
    
  r.leave(); s.leave(); d.leave();
  return SLEEP;

failure:
entailment:

  if (0 == (*r &= r_val)) {
    r.fail(); s.fail(); d.fail();
    return FAILED;
  } 

  r.leave(); s.leave(); d.leave();
  return OZ_ENTAILED;
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_isInR, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_INT "," OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectInt);
  OZ_EXPECT(pe, 1, expectFSetVarAny);
  OZ_EXPECT(pe, 2, expectIntVarAny);
  
  return pe.impose(new IsInRPropagator(OZ_args[1],
				       OZ_args[0],
				       OZ_args[2]));
} 
OZ_C_proc_end

OZ_CFunHeader IsInRPropagator::spawner = fsp_isInR;

OZ_Return IsInRPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FDIntVar b(_b);
  OZ_FSetVar v(_v);

  // TMUELLER if is singleton do the right things  
  if (v->isIn(_i)) {
    FailOnEmpty(*b &= 1);
    b.leave();
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }
  if (v->isNotIn(_i)) {
    FailOnEmpty(*b &= 0);
    b.leave();
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }
  if (*b == fd_singl) {
    if (b->getSingleElem() == 0) {
      FailOnInvalid(*v-= _i);
    } else {
      FailOnInvalid(*v += _i);
    }
    b.leave();
    v.leave();
    OZ_DEBUGPRINTTHIS("entailed: ");
    return OZ_ENTAILED;
  }

  OZ_DEBUGPRINTTHIS("sleep: ");
  b.leave();
  v.leave();
  return SLEEP;

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  b.fail();
  v.fail();
  return FAILED;
}


//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_bounds, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSETVAL "," OZ_EM_FSET "," OZ_EM_INT "," 
		   OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  int dummy;
  OZ_EXPECT(pe, 0, expectFSetValue);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, dummy);
  OZ_EXPECT(pe, 2, expectInt);
  OZ_EXPECT_SUSPEND(pe, 3, expectIntVarMinMax, dummy);
  OZ_EXPECT_SUSPEND(pe, 4, expectIntVarMinMax, dummy);
  
  return pe.impose(new BoundsPropagator(OZ_args[0],
					OZ_args[1],
					OZ_args[2],
					OZ_args[3],
					OZ_args[4]));
} 
OZ_C_proc_end

OZ_CFunHeader BoundsPropagator::header = fsp_bounds;

OZ_Return BoundsPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d), r(_r);
  PropagatorController_S_D_D P(s, d, r);
  OZ_Return retval = OZ_SLEEP;

  // the following code is executed once ..
  if (_s_ub) {
    OZ_FSetVar s_ub;
    s_ub.ask(_s_ub);
    _s_ub_card = s_ub->getCardMin();

    OZ_DEBUGPRINT(("once %s %d\n", s_ub->toString(), _s_ub_card));
    FailOnInvalid(*s <= *s_ub);
    FailOnEmpty(*d <= _d_ub);
    FailOnEmpty(r->constrainBool());

    _s_ub = 0; // .. because of that 
  }

  if (d->getMinElem() > 0 || 
      s->getGlbSet().getCard() > 0 ||
      *r == 1) {
    OZ_DEBUGPRINT(("a\n"));
    FailOnEmpty(*d &= _d_ub);
    FailOnEmpty(*r &= 1);
    FailOnInvalid(s->putCard(_s_ub_card, _s_ub_card));
    retval = OZ_ENTAILED;
  } else if (d->getMaxElem() < _d_ub || 
	     s->getLubSet().getCard() < _s_ub_card ||
	     *r == 0) {
    OZ_DEBUGPRINT(("b\n"));
    FailOnEmpty(*d &= 0);
    FailOnEmpty(*r &= 0);
    FailOnInvalid(s->putCard(0,0));
    retval = OZ_ENTAILED;
  } 

  P.leave();
  OZ_DEBUGPRINTTHIS("out");
  return OZ_DEBUGRETURNPRINT(retval);

failure:
  OZ_DEBUGPRINTTHIS("fail: ");
  P.fail();
  return FAILED;
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_boundsN, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FSETVAL "," 
		   OZ_EM_VECT OZ_EM_FSET "," 
		   OZ_EM_VECT OZ_EM_INT "," 
		   OZ_EM_VECT OZ_EM_FD "," 
		   OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  int dummy;
  OZ_EXPECT(pe, 0, expectVectorFSetValue);
  OZ_EXPECT_SUSPEND(pe, 1, expectVectorFSetVarBounds, dummy);
  OZ_EXPECT(pe, 2, expectVectorInt);
  OZ_EXPECT_SUSPEND(pe, 3, expectVectorIntVarMinMax, dummy);
  OZ_EXPECT_SUSPEND(pe, 4, expectVectorIntVarMinMax, dummy);
  
  SAMELENGTH_VECTORS(0, 1);
  SAMELENGTH_VECTORS(0, 2);
  SAMELENGTH_VECTORS(0, 3);
  SAMELENGTH_VECTORS(0, 4);

  return pe.impose(new BoundsNPropagator(OZ_args[0],
					 OZ_args[1],
					 OZ_args[2],
					 OZ_args[3],
					 OZ_args[4]));
} 
OZ_C_proc_end

OZ_CFunHeader BoundsNPropagator::header = fsp_boundsN;

OZ_Return BoundsNPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");
  
  DECL_DYN_ARRAY(OZ_FSetVar, s, _size);
  DECL_DYN_ARRAY(OZ_FDIntVar, d, _size);
  DECL_DYN_ARRAY(OZ_FDIntVar, r, _size);
  PropagatorController_VS_VD_VD P(_size, s, d, r);
  int i, left = _size;
  
  for (i = _size; i--; ) {
    s[i].read(_s[i]);
    d[i].read(_d[i]);
    r[i].read(_r[i]);
  }

  // the following code is executed once ..
  if (first) {
    for (i = _size; i--; ) {
      OZ_FSetVar s_ub_aux;
      s_ub_aux.ask(_s_ub.s_ub[i]);
      _s_ub.s_ub_card[i] = s_ub_aux->getCardMin();
      
      FailOnInvalid(*s[i] <= *s_ub_aux);
      FailOnEmpty(*d[i] <= _d_ub[i]);
      FailOnEmpty(r[i]->constrainBool());
    }

    first = 0;  // .. because of that 
  }

  for (i = _size; i--; ) {
    if (d[i]->getMinElem() > 0 || 
	s[i]->getGlbSet().getCard() > 0 ||
	*r[i] == 1) {
      OZ_DEBUGPRINT(("a\n"));
      FailOnEmpty(*d[i] &= _d_ub[i]);
      FailOnEmpty(*r[i] &= 1);
      FailOnInvalid(s[i]->putCard(_s_ub.s_ub_card[i], _s_ub.s_ub_card[i]));
      left -= 1;
    } else if (d[i]->getMaxElem() < _d_ub[i] || 
	       s[i]->getLubSet().getCard() < _s_ub.s_ub_card[i] ||
	       *r[i] == 0) {
      OZ_DEBUGPRINT(("b\n"));
      FailOnEmpty(*d[i] &= 0);
      FailOnEmpty(*r[i] &= 0);
      FailOnInvalid(s[i]->putCard(0,0));
      left -= 1;
    } 
  }

  P.leave();
  OZ_DEBUGPRINTTHIS("out");
  
  if (left > 0 && left < _size) {
    int j;
    for (j = i = 0; i < _size; i += 1) {
      if (*r[i] == fd_singl)
	continue;
      _d[j]              = _d[i];
      _s[j]              = _s[i];
      _r[j]              = _r[i];
      _d_ub[j]           = _d_ub[i];
      _s_ub.s_ub_card[j] = _s_ub.s_ub_card[i];
      j += 1;
    }
  }
  return left ? OZ_SLEEP : OZ_ENTAILED;

failure:
  P.fail();
  OZ_DEBUGPRINTTHIS("fail: ");
  return FAILED;
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_partitionReified, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FSET "," OZ_EM_FSET "," 
		   OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorFSetValue);
  OZ_EXPECT(pe, 1, expectFSetValue);
  OZ_EXPECT(pe, 2, expectVectorIntVarMinMax);
  
  return pe.impose(new PartitionReifiedPropagator(OZ_args[0],
						  OZ_args[1],
						  OZ_args[2]));
} 
OZ_C_proc_end

OZ_CFunHeader PartitionReifiedPropagator::header = fsp_partitionReified;

OZ_Return PartitionReifiedPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  DECL_DYN_ARRAY(OZ_FDIntVar, vd, _size);
  DECL_DYN_ARRAY(int, stack, _u_max_elem+1);
  ItStack ist(_u_max_elem+1, stack);
  
  PropagatorController_VD P(_size, vd);
  int i;

  int i_high = _i_sets->getHigh();
  DECL_DYN_ARRAY(int, elems, i_high);
  IndexSet u(i_high, elems);
    
  for (i = _size; i--; ) 
    vd[i].read(_vd[i]);
  
  if (_first) {
    for (i = _u_max_elem+1; i--; ) {
      IndexSet &tmp_i = (*_i_sets)[i];
      if (! tmp_i.isIgnore() && tmp_i.getCard() == 1)
	ist.push(i);
    }
    _first = 0;
  }
 
  IndexSet &det_vars = (*_i_sets)[_u_max_elem+1];

  for (i = _size; i--; ) 
    if (!det_vars.isIn(i) && *vd[i] == fd_singl) {
      int s = vd[i]->getSingleElem();
      if (s > 0) {
	if (! _i_sets->resetAllBut(ist, u, i))
	  goto failure;
      } else {
	OZ_ASSERT(s == 0);

	for (int j = _u_max_elem+1; j--; ) {
	  IndexSet &tmp_j = (*_i_sets)[j];
	  if (! tmp_j.isIgnore()) {
	    int card = tmp_j.reset(i);
	    if (card == 0) {
	      goto failure;
	    } else if (card == 1) {
	      int k = tmp_j.smallestElem();
	      if (! _i_sets->resetAllBut(ist, u, k)) 
		goto failure;
	      FailOnEmpty(*vd[k] -= 0);
	    }
	  }
	}
      }
    }

  while (!ist.isEmpty()) {
    int k = (*_i_sets)[ist.pop()].smallestElem();
    if (! _i_sets->resetAllBut(ist, u, k)) 
      goto failure;
  }  
  
  for (i = _u_max_elem+1; i--; ) {
    IndexSet &tmp_i = (*_i_sets)[i];
    if (! tmp_i.isIgnore() && tmp_i.getCard() == 1)
      FailOnEmpty(*vd[tmp_i.smallestElem()] -= 0);
  }

  _i_sets->unionAll(u);
  for (i = _size; i--; ) {
    if (!u.isIn(i)) 
      FailOnEmpty(*vd[i] &= 0);
    if (*vd[i] == fd_singl)
      det_vars.set(i);
  }

#ifdef OZ_DEBUG
  for (i = _u_max_elem+1; i--; )
    if (!(*_i_sets)[i].getCard())
      abort();
#endif
  
  OZ_DEBUGPRINTTHIS("out ");
  {
    OZ_Return r = P.leave();
    return r;
  }
failure:
  OZ_DEBUGPRINTTHIS("failed");
  return P.fail();  
}

PartitionReifiedPropagator::PartitionReifiedPropagator(OZ_Term vs, OZ_Term s, OZ_Term vd) 
{
  _first = 1;
  // init ground set
  OZ_FSetVar aux(s); // ought to be ask
  OZ_FSetValue u = aux->getGlbSet(); 
  _u_max_elem = u.getMaxElem();
  
  int i;
  _size = OZ_vectorSize(vs);
  
  // creating subsets
  _vs = (OZ_FSetValue *) (void *) OZ_hallocChars(_size * sizeof(OZ_FSetValue));
  DECL_DYN_ARRAY(OZ_Term, vs_terms, _size);
  OZ_getOzTermVector(vs, vs_terms);
  
  for (i = _size; i--; ) {
    OZ_FSetVar aux(vs_terms[i]); // ought to ask!
    _vs[i] = aux->getGlbSet();
  }
  
  // creating bools
  _vd = OZ_hallocOzTerms(_size);
  OZ_getOzTermVector(vd, _vd);
  
  // create index sets
  _i_sets = IndexSets::create(_u_max_elem+2, _size);
  //_i_sets->print();
  
  for (i = _u_max_elem+1; i--; ) {
    if (u.isIn(i)) {
      for (int j = _size; j--; ) {
	if (_vs[j].isIn(i)) {
	  (*_i_sets)[i].set(j);
	}
      }
    } else {
	(*_i_sets)[i].setIgnore();
    }
  }
  //  _i_sets->print();
  
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_partitionReified1, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FSET "," OZ_EM_FSET "," 
		   OZ_EM_VECT OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorFSetValue);
  OZ_EXPECT(pe, 1, expectFSetValue);
  OZ_EXPECT(pe, 2, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 3, expectIntVarMinMax);
  
  return pe.impose(new PartitionReified1Propagator(OZ_args[0],
						   OZ_args[1],
						   OZ_args[2],
						   OZ_args[3]));
} 
OZ_C_proc_end

OZ_CFunHeader PartitionReified1Propagator::header = fsp_partitionReified1;

OZ_Return PartitionReified1Propagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  //_i_sets->print();
 
  DECL_DYN_ARRAY(OZ_FDIntVar, vd, _size);
  DECL_DYN_ARRAY(int, stack, _u_max_elem+1);
  ItStack ist(_u_max_elem+1, stack);
  OZ_FDIntVar cost(_cost);

  PropagatorController_VD_D P(_size, vd, cost);
  int i;

  int i_high = _i_sets->getHigh();
  DECL_DYN_ARRAY(int, elems, i_high);
  IndexSet u(i_high, elems);
    
  for (i = _size; i--; ) 
    vd[i].read(_vd[i]);
  
  if (_first) {
    for (i = _u_max_elem+1; i--; ) {
      IndexSet &tmp_i = (*_i_sets)[i];
      if (! tmp_i.isIgnore() && tmp_i.getCard() == 1)
	ist.push(i);
    }
    
    int sum = 0;
    for (i = _u_max_elem+1; i--; ) {
      IndexSet &tmp_i = (*_i_sets)[i];
      
      if (! tmp_i.isIgnore())
	sum += _min_cost_per_elem[i];
    }
#ifdef OZ_DEBUG
    printf("__cost >= %d\n", sum); fflush(stdout);
#endif

    FailOnEmpty(*cost >= sum);    

    _first = 0;
  }
 
  {
    IndexSet &det_vars = (*_i_sets)[_u_max_elem+1];
    
    for (i = _size; i--; ) 
      if (!det_vars.isIn(i) && *vd[i] == fd_singl) {
	int s = vd[i]->getSingleElem();
	if (s > 0) {
	  if (! _i_sets->resetAllBut(ist, u, i))
	    goto failure;
	} else {
	  OZ_ASSERT(s == 0);
	  
	  for (int j = _u_max_elem+1; j--; ) {
	    IndexSet &tmp_j = (*_i_sets)[j];
	    if (! tmp_j.isIgnore()) {
	      int card = tmp_j.reset(i);
	      if (card == 0) {
		goto failure;
	      } else if (card == 1) {
		int k = tmp_j.smallestElem();
		if (! _i_sets->resetAllBut(ist, u, k)) 
		  goto failure;
	      FailOnEmpty(*vd[k] -= 0);
	      }
	    }
	  }
	}
      }
    
    //_i_sets->print();
    
    while (!ist.isEmpty()) {
      int k = (*_i_sets)[ist.pop()].smallestElem();
      if (! _i_sets->resetAllBut(ist, u, k)) 
	goto failure;
    }  
    
    for (i = _u_max_elem+1; i--; ) {
      IndexSet &tmp_i = (*_i_sets)[i];
      if (! tmp_i.isIgnore() && tmp_i.getCard() == 1)
	FailOnEmpty(*vd[tmp_i.smallestElem()] -= 0);
    }
    
    _i_sets->unionAll(u);
    for (i = _size; i--; ) {
      if (!u.isIn(i)) 
	FailOnEmpty(*vd[i] &= 0);
      if (*vd[i] == fd_singl)
	det_vars.set(i);
    }
  }
  
#ifdef OZ_DEBUG
  for (i = _u_max_elem+1; i--; )
    if (!(*_i_sets)[i].getCard())
      abort();
#endif
  
  // check if better solution is still possible
  
  {
    int sum = 0;
    DECL_DYN_ARRAY(int, stack_flags, _u_max_elem+1);
    ItStack ist_flags(_u_max_elem+1, stack_flags);
    
    for (i = _u_max_elem+1; i--; ) {
      IndexSet &tmp_i = (*_i_sets)[i];
      
      if (! tmp_i.isIgnore()) {
	if (tmp_i.getCard() == 1) {
	  int ind = tmp_i.smallestElem();
	  if (!ist_flags.isIn(ind)) {
	    sum += vd[ind]->getSingleElem();
	    ist_flags.push(ind);
	  }
	} else {
	  sum += _min_cost_per_elem[i];
	}
      }
    }
    
    if (sum > cost->getMaxElem()) {
#ifdef OZ_DEBUG
      printf("failing because of sum (%d) > cost_max(%d)\n", 
	     sum, cost->getMaxElem()); 
      fflush(stdout);
#endif
      goto failure;
    }
  }
  
  OZ_DEBUGPRINTTHIS("out ");
  {
    OZ_Return r = P.leave();
    //    if (r == OZ_ENTAILED)
    //  _i_sets->print();
    return r;
  }
  failure:
  OZ_DEBUGPRINTTHIS("failed");
  return P.fail();  
}

PartitionReified1Propagator::PartitionReified1Propagator(OZ_Term vs, 
							 OZ_Term s, 
							 OZ_Term vd, 
							 OZ_Term cost) 
: _cost(cost), PartitionReifiedPropagator(vs, s, vd)
{
  _min_cost_per_elem = OZ_hallocCInts(_u_max_elem+1);
  
  OZ_FSetVar aux;
  aux.ask(s);
  OZ_FSetValue u = aux->getGlbSet(); 
  
  int i;
  
  for (i = _u_max_elem; i-- ; )
    _min_cost_per_elem[i] = INT_MAX;
  
  for (i = _u_max_elem+1; i--; ) {
    if (u.isIn(i)) {    
      for (int j = _size; j--; ) {
	OZ_FDIntVar aux2;
	aux2.ask(_vd[j]);
	
	if (_vs[j].isIn(i)) {
	  
	  _min_cost_per_elem[i] = min(_min_cost_per_elem[i],
				      aux2->getMaxElem() / _vs[j].getCard());
	  
	}
      }
    } 
  }

#ifdef OZ_DEBUG
  for (i = _u_max_elem+1; i--; ) {
    if (u.isIn(i)) {    
      printf("_min_cost_per_elem[%d]=%d\n", i, _min_cost_per_elem[i]);
      fflush(stdout);
    } 
  }
#endif
  
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_partitionProbing, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FSET "," OZ_EM_FSET "," 
		   OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorFSetValue);
  OZ_EXPECT(pe, 1, expectFSetValue);
  OZ_EXPECT(pe, 2, expectVectorIntVarMinMax);
  
  return pe.impose(new PartitionProbingPropagator(OZ_args[0],
						  OZ_args[1],
						  OZ_args[2]));
} 
OZ_C_proc_end

OZ_CFunHeader PartitionProbingPropagator::header = fsp_partitionProbing;

OZ_Return PartitionProbingPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  DECL_DYN_ARRAY(OZ_FDIntVar, vd, _size);
  DECL_DYN_ARRAY(int, stack, _u_max_elem+1);
  ItStack ist(_u_max_elem+1, stack);
  
  PropagatorController_VD P(_size, vd);
  int i;

  int i_high = _i_sets->getHigh();
  DECL_DYN_ARRAY(int, elems, i_high);
  IndexSet u(i_high, elems);
    
  for (i = 0; i < _size; i += 1) 
    vd[i].read(_vd[i]);
  
  DECL_DYN_ARRAY(char, isetsmem, _i_sets->sizeOfMe());

  int removed = 0;

  for (int ii = 0 ; ii < _size; ii += 1) {

    if (*vd[ii] == fd_singl)
      continue;

    IndexSets * i_sets_copy = _i_sets->copy(isetsmem);
    
    if (! i_sets_copy->resetAllBut(ist, u, ii))
      goto reset;

    while (!ist.isEmpty()) {
      int k = (*i_sets_copy)[ist.pop()].smallestElem();
      if (! i_sets_copy->resetAllBut(ist, u, k)) 
	goto reset;
    }  
    
    OZ_ASSERT(ist.isEmpty());

    continue;
  reset:
    FailOnEmpty(*vd[ii] &= 0);
    ist.setEmpty();
    removed += 1;
  }
  
  if (removed)
    printf("Removed %d sets.\n", removed); fflush(stdout);

  P.leave();
  return OZ_ENTAILED;

failure:
  OZ_ASSERT(0);
  return P.fail();
}

PartitionProbingPropagator::PartitionProbingPropagator(OZ_Term vs, OZ_Term s, OZ_Term vd) 
{
  _first = 1;
  // init ground set
  OZ_FSetVar aux(s); // ought to be ask
  OZ_FSetValue u = aux->getGlbSet(); 
  _u_max_elem = u.getMaxElem();
  
  int i;
  _size = OZ_vectorSize(vs);
  
  // creating subsets
  DECL_DYN_ARRAY(OZ_Term, vs_terms, _size);
  OZ_getOzTermVector(vs, vs_terms);
  
  // creating bools
  _vd = OZ_hallocOzTerms(_size);
  OZ_getOzTermVector(vd, _vd);
  
  // create index sets
  _i_sets = IndexSets::create(_u_max_elem+2, _size);
  //_i_sets->print();
  
  for (i = _u_max_elem+1; i--; ) {
    if (u.isIn(i)) {
      for (int j = _size; j--; ) {
	OZ_FSetVar auxset;
	auxset.ask(vs_terms[j]);
	OZ_FDIntVar auxfd;
	auxfd.ask(_vd[j]);
	if (auxfd->getMaxElem() > 0 && auxset->isIn(i)) {
	  (*_i_sets)[i].set(j);
	}
      }
    } else {
	(*_i_sets)[i].setIgnore();
    }
  }
  //  _i_sets->print();
  //  printf("here2\n"); fflush(stdout);

  
}

//-----------------------------------------------------------------------------
// eof
