/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: toelgart, tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "sumabs.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_sumAC, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT","OZ_EM_VECT OZ_EM_FD","
		   OZ_EM_LIT","OZ_EM_FD);
  
  PropagatorExpect pe;
  OZ_EXPECT(pe, 0, expectVectorInt);
  OZ_EXPECT(pe, 2, expectLiteral); 
  SAMELENGTH_VECTORS(0,1);
  
  char * op = OZ_atomToC(OZ_args[2]);
  
  if (!strcmp(SUM_OP_EQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax); 
    return pe.impose(new SumACEqPropagator(OZ_args[0],
					   OZ_args[1],
					   OZ_args[3]));
  } else if (!strcmp(SUM_OP_LEQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax); 
    return pe.impose(new SumACLessEqPropagator(OZ_args[0],
					       OZ_args[1],
					       OZ_args[3]));
  } else if (!strcmp(SUM_OP_LT, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax); 
    return pe.impose(new SumACLessPropagator(OZ_args[0],
					     OZ_args[1],
					     OZ_args[3]));
  } else if (!strcmp(SUM_OP_GEQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax); 
    return pe.impose(new SumACGreaterEqPropagator(OZ_args[0],
						  OZ_args[1],
						  OZ_args[3]));
  } else if (!strcmp(SUM_OP_GT, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax); 
    return pe.impose(new SumACGreaterPropagator(OZ_args[0],
						OZ_args[1],
						OZ_args[3]));
  } else if (!strcmp(SUM_OP_NEQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarSingl);
    OZ_EXPECT(pe, 3, expectIntVarSingl); 
    return pe.impose(new SumACNotEqPropagator(OZ_args[0],
					      OZ_args[1],
					      OZ_args[3]));
  }
  
  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_C_proc_end

OZ_CFun LinEqAbsPropagator::header = fdp_sumAC;
OZ_CFun LinLessEqAbsPropagator::header = fdp_sumAC;
OZ_CFun LinGreaterEqAbsPropagator::header = fdp_sumAC;
OZ_CFun LinNotEqAbsPropagator::header = fdp_sumAC;

//-----------------------------------------------------------------------------

#define CLAUSE1 1
#define CLAUSE2 2
#define CLAUSE(A, B) (clause & 2 ? (A) : (B))
       
OZ_Return LinEqAbsPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in " << *this);
  
  int summax, summin, axmax, axmin, dmax, dmin, j, k, d_size, fail, clause;
  double bound1, bound2;
  OZ_FiniteDomain d_aux_neg, d_aux_pos, aux;
  OZ_Boolean unified, changed;
  
  unified = simplify(); 
  
  OZ_FDIntVar d(reg_d);
  DECL_DYN_ARRAY(OZ_FDIntVar, x, reg_sz);
  PropagatorController_VV_V P(reg_sz, x, d);

  DECL_DYN_ARRAY(OZ_FiniteDomain, x_aux_neg, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, x_aux_pos, reg_sz);
  
  for(j=reg_sz; j--;) {
    x[j].read(reg_x[j]);
    x_aux_neg[j] = x_aux_pos[j] = *x[j];
  }
  
  if (unified) {
    reg_a[dpos] -= 1;
    d_aux_neg.initSingleton(0);
    d_aux_pos.initSingleton(0);
  } else { 
    d_aux_neg = d_aux_pos = *d;
  }

  clause = CLAUSE1;
  fail = 0; // 0 no fail, 1 positive clause failed, 2 negative clause failed 
  do {
    changed = false;
    summin = summax = CLAUSE(-reg_c, reg_c);
    
    for(j=reg_sz; j--;) {
      axmax = int(double(reg_a[j]) *
		  CLAUSE(x_aux_neg[j].getMaxElem(), x_aux_pos[j].getMaxElem())
		  );
      axmin = int(double(reg_a[j]) *
		  CLAUSE(x_aux_neg[j].getMinElem(), x_aux_pos[j].getMinElem())
		  );

      if (reg_a[j] < 0) {
	summin += axmax; summax += axmin;
      } 
      if (reg_a[j] > 0) {
	summin += axmin; summax += axmax;
      }
    }

    d_size = CLAUSE(d_aux_neg.getSize(), d_aux_pos.getSize());
    CLAUSE(d_aux_neg, d_aux_pos) >= summin;
    CLAUSE(d_aux_neg, d_aux_pos) <= summax;
    
    if (! CLAUSE(d_aux_neg.getSize(), d_aux_pos.getSize())) {
      fail |= clause;
    } else {
      changed |= !(d_size == CLAUSE(d_aux_neg.getSize(), d_aux_pos.getSize())); 
      dmax = CLAUSE(d_aux_neg.getMaxElem(), d_aux_pos.getMaxElem());
      dmin = CLAUSE(d_aux_neg.getMinElem(), d_aux_pos.getMinElem());
      
      for (j = reg_sz; j-- && !(fail & clause); ) { 
	summin = summax = CLAUSE(-reg_c, reg_c);           

	for(k = reg_sz; k--;) {
	  if (j != k) {
	    axmax = int(double(reg_a[k]) * CLAUSE(x_aux_neg[k].getMaxElem(), 
						  x_aux_pos[k].getMaxElem()));
	    axmin = int(double(reg_a[k]) * CLAUSE(x_aux_neg[k].getMinElem(), 
						  x_aux_pos[k].getMinElem()));
	    
	    if (reg_a[k] < 0) {
	      summin += axmax; summax += axmin;
	    } 
	    if (reg_a[k] > 0) {
	      summin += axmin; summax += axmax;
	    }
	  }
	} // for

	bound1 = (dmin - summax) / double(reg_a[j]);
	bound2 = (dmax - summin) / double(reg_a[j]);
	d_size = CLAUSE(x_aux_neg[j].getSize(), x_aux_pos[j].getSize());
	
	if (reg_a[j] < 0) {
	  CLAUSE(x_aux_neg[j], x_aux_pos[j]) >= doubleToInt(ceil(bound2));
	  CLAUSE(x_aux_neg[j], x_aux_pos[j]) <= doubleToInt(floor(bound1));
        } 
	if (reg_a[j] > 0) {
	  CLAUSE(x_aux_neg[j], x_aux_pos[j]) >= doubleToInt(ceil(bound1));
	  CLAUSE(x_aux_neg[j], x_aux_pos[j]) <= doubleToInt(floor(bound2));
	}
	
	if (!(CLAUSE(x_aux_neg[j].getSize(), x_aux_pos[j].getSize()))) {
	  fail |= clause;
	  changed = false;
	} else {
	  changed |= !
	    (d_size == CLAUSE(x_aux_neg[j].getSize(), x_aux_pos[j].getSize()));
	}
      }
    }
    
    if (!changed && (clause & CLAUSE1)) {
     clause = CLAUSE2;
     changed = true;
     if (unified) 
       reg_a[dpos] += 2;   // reg_a[dpos]++, Vorz. umk., reg_a[dpos]--
     for(j = reg_sz; j--;) 
       reg_a[j] = -reg_a[j];
    }   
  } while(changed);
  
  if (unified) 
    reg_a[dpos] -= 2;    // Vorz. wieder korrigieren

  for(j = reg_sz; j--;) 
    reg_a[j] = -reg_a[j];

  for(j = reg_sz; j--;) {
    aux.initEmpty();
    
    if (!(fail & CLAUSE1)) 
      aux = x_aux_pos[j];
    
    if (!(fail & CLAUSE2)) 
      aux = aux | x_aux_neg[j];
    
    FailOnEmpty(*x[j] &= aux);
  }

  if (!unified) {
    aux.initEmpty();
    if (!(fail & CLAUSE1)) 
      aux = d_aux_pos;
    if (!(fail & CLAUSE2)) 
      aux = aux | d_aux_neg;
    FailOnEmpty(*d &= aux);
  }

  FailOnEmpty(d->getSize());
  
  return P.leave();
  
failure:
  OZ_DEBUGPRINT("failed: " << *this);

  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return LinLessEqAbsPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in " << *this);
  
  int summin, dmax, j, k, d_size, clause;
  double bound1, bound2;
  OZ_FiniteDomain d_aux;
  OZ_Boolean unified, changed;
  
  unified  = simplify();

  OZ_FDIntVar d(reg_d);
  DECL_DYN_ARRAY(OZ_FDIntVar, x, reg_sz);
  PropagatorController_VV_V P(reg_sz, x, d);

  for(j = reg_sz; j--;)
    x[j].read(reg_x[j]);

  if (unified) {
    reg_a[dpos] -= 1;
    d_aux.initSingleton(0);
  } else {
    d_aux = *d;
  }

  clause = CLAUSE1;
  do {
    changed = false;
    summin = CLAUSE(-reg_c,reg_c);
    
    for(j = reg_sz; j--;) {
      summin += int(double(reg_a[j]) * 
		    (reg_a[j] < 0 ? x[j]->getMaxElem() : x[j]->getMinElem()));
    }
    d_size = d_aux.getSize();
    FailOnEmpty(d_aux >= summin);
    
    changed |= !(d_size == d_aux.getSize()); 
   
    dmax = d_aux.getMaxElem();
    
    for(j = reg_sz; j--;) { 
      
      summin = CLAUSE(-reg_c, reg_c);

      for(k = reg_sz; k--; )
	if (j != k) {
	  summin += int(double(reg_a[k]) * 
			(reg_a[k] < 0 ? x[k]->getMaxElem() : x[k]->getMinElem()));
       }
     
     bound1 = (dmax - summin) / double(reg_a[j]);
     d_size = x[j]->getSize();
     
     if (reg_a[j] < 0)
       *x[j] >= doubleToInt(ceil(bound1));
     if (reg_a[j] > 0)
       *x[j] <= doubleToInt(floor(bound1));
     
     FailOnEmpty(x[j]->getSize());
     changed |= !(d_size == x[j]->getSize());
   }
   
   if (!changed && (clause & CLAUSE1)) {
     clause = CLAUSE2;
     changed = true;
     if (unified) 
       reg_a[dpos] += 2;   // reg_a[dpos]++, Vorz. umk., reg_a[dpos]--
     for(j = reg_sz; j--;) 
       reg_a[j] = -reg_a[j];
   }
  } while(changed);
  
  if (unified) 
    reg_a[dpos] -= 2;    // Vorz. wieder korrigieren
  
  for(j = reg_sz; j--;) 
    reg_a[j] = -reg_a[j];
 
  if (!unified) 
    FailOnEmpty(*d &= d_aux);
 
  return P.leave();
 
failure:
 OZ_DEBUGPRINT("failed: " << *this);
 
 return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return LinNotEqAbsPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in " << *this);
  
  int dmax, j, k, d_size, clause;
  double bound1, bound2;
  OZ_FiniteDomain d_aux;
  OZ_Boolean unified, changed;
  
  unified  = simplify();

  OZ_FDIntVar d(reg_d);
  DECL_DYN_ARRAY(OZ_FDIntVar, x, reg_sz);
  PropagatorController_VV_V P(reg_sz, x, d);

  int  num_of_singl = 0, last_nonsingl = 0;
  for(j = reg_sz; j--;) {
    x[j].read(reg_x[j]);
    if (*x[j] == fd_singl) 
      num_of_singl += 1;
    else
      last_nonsingl = j;
  }
  
  if (unified) {
    reg_a[dpos] -= 1;
    d_aux.initSingleton(0);
  } else {
    d_aux = *d;
  }

  if (*d == fd_singl) 
    num_of_singl += 1;
  else
    last_nonsingl = -1;

  if (num_of_singl < reg_sz) // anything to do, not enough singletons found?
    return P.leave();

  int all_singl = (num_of_singl == reg_sz + 1);

  clause = CLAUSE1;
  for (k = 2; k--; ) {
    double sum = CLAUSE(-reg_c,reg_c);
    
    if (all_singl) { // just check consistency
      
      for(j = reg_sz; j--;) {
	sum += int(double(reg_a[j]) * x[j]->getSingleElem());
      }

      if (sum == d->getSingleElem())
	goto failure;

    } else if (last_nonsingl == -1) { // _d_ is last non-singleton
      
      for (j = reg_sz; j--; )
	sum += double(reg_a[j]) * x[j]->getSingleElem();
      
      FailOnEmpty(*d -= int(sum));
    } else { // some _x_ is last singleton
      
      for (j = reg_sz; j--; )
	if (last_nonsingl != j)
	  sum += double(reg_a[j]) * x[j]->getSingleElem();
      sum -= d->getSingleElem();
      
      if ((int(sum) % reg_a[last_nonsingl]) == 0) {
	sum /= -double(reg_a[last_nonsingl]);
	FailOnEmpty(*x[last_nonsingl] -= int(sum));
      }

    }

   if ((clause & CLAUSE1)) {
     clause = CLAUSE2;
     if (unified) 
       reg_a[dpos] += 2;   // reg_a[dpos]++, Vorz. umk., reg_a[dpos]--
     for(j = reg_sz; j--;) 
       reg_a[j] = -reg_a[j];
   }
  } // for
  
  if (unified) 
    reg_a[dpos] -= 2;    // Vorz. wieder korrigieren
  
  for(j = reg_sz; j--;) 
    reg_a[j] = -reg_a[j];
 
  if (!unified) 
    FailOnEmpty(*d &= d_aux);
 
  return P.leave();
 
failure:
 OZ_DEBUGPRINT("failed: " << *this);
 
 return P.fail();
}

//-----------------------------------------------------------------------------

OZ_Return LinGreaterEqAbsPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in " << *this);
  
  int summax, axmax, axmin, dmax, dmin, j, k, d_size, fail, clause;
  double bound1, bound2;
  OZ_FiniteDomain d_aux_neg, d_aux_pos, aux;
  OZ_Boolean unified, changed;
  
  unified = simplify(); 
  
  OZ_FDIntVar d(reg_d);
  DECL_DYN_ARRAY(OZ_FDIntVar, x, reg_sz);
  PropagatorController_VV_V P(reg_sz, x, d);

  DECL_DYN_ARRAY(OZ_FiniteDomain, x_aux_neg, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, x_aux_pos, reg_sz);
  
  for(j=reg_sz; j--;) {
    x[j].read(reg_x[j]);
    x_aux_neg[j] = x_aux_pos[j] = *x[j];
  }
  
  if (unified) {
    reg_a[dpos] -= 1;
    d_aux_neg.initSingleton(0);
    d_aux_pos.initSingleton(0);
  } else { 
    d_aux_neg = d_aux_pos = *d;
  }

  clause = CLAUSE1;
  fail = 0; // 0 no fail, 1 positive clause failed, 2 negative clause failed 
  do {
    changed = false;
    summax = CLAUSE(-reg_c, reg_c);
    
    for(j=reg_sz; j--;) {
      axmax = int(double(reg_a[j]) *
		  CLAUSE(x_aux_neg[j].getMaxElem(), x_aux_pos[j].getMaxElem())
		  );
      axmin = int(double(reg_a[j]) *
		  CLAUSE(x_aux_neg[j].getMinElem(), x_aux_pos[j].getMinElem())
		  );

      if (reg_a[j] < 0) {
	summax += axmin;
      } 
      if (reg_a[j] > 0) {
	summax += axmax;
      }
    }

    d_size = CLAUSE(d_aux_neg.getSize(), d_aux_pos.getSize());
    CLAUSE(d_aux_neg, d_aux_pos) <= summax;
    
    if (! CLAUSE(d_aux_neg.getSize(), d_aux_pos.getSize())) {
      fail |= clause;
    } else {
      changed |= !(d_size == CLAUSE(d_aux_neg.getSize(), d_aux_pos.getSize())); 
      dmin = CLAUSE(d_aux_neg.getMinElem(), d_aux_pos.getMinElem());
      
      for (j = reg_sz; j-- && !(fail & clause); ) { 
	summax = CLAUSE(-reg_c, reg_c);           

	for(k = reg_sz; k--;) {
	  if (j != k) {
	    axmax = int(double(reg_a[k]) * CLAUSE(x_aux_neg[k].getMaxElem(), 
						  x_aux_pos[k].getMaxElem()));
	    axmin = int(double(reg_a[k]) * CLAUSE(x_aux_neg[k].getMinElem(), 
						  x_aux_pos[k].getMinElem()));
	    
	    if (reg_a[k] < 0) {
	      summax += axmin;
	    } 
	    if (reg_a[k] > 0) {
	      summax += axmax;
	    }
	  }
	} // for

	bound1 = (dmin - summax) / double(reg_a[j]);
	d_size = CLAUSE(x_aux_neg[j].getSize(), x_aux_pos[j].getSize());
	
	if (reg_a[j] < 0) {
	  CLAUSE(x_aux_neg[j], x_aux_pos[j]) <= doubleToInt(floor(bound1));
        } 
	if (reg_a[j] > 0) {
	  CLAUSE(x_aux_neg[j], x_aux_pos[j]) >= doubleToInt(ceil(bound1));
	}
	
	if (!(CLAUSE(x_aux_neg[j].getSize(), x_aux_pos[j].getSize()))) {
	  fail |= clause;
	  changed = false;
	} else {
	  changed |= !
	    (d_size == CLAUSE(x_aux_neg[j].getSize(), x_aux_pos[j].getSize()));
	}
      }
    }
    
    if (!changed && (clause & CLAUSE1)) {
     clause = CLAUSE2;
     changed = true;
     if (unified) 
       reg_a[dpos] += 2;   // reg_a[dpos]++, Vorz. umk., reg_a[dpos]--
     for(j = reg_sz; j--;) 
       reg_a[j] = -reg_a[j];
    }   
  } while(changed);
  
  if (unified) 
    reg_a[dpos] -= 2;    // Vorz. wieder korrigieren
  
  for(j = reg_sz; j--;) 
    reg_a[j] = -reg_a[j];

  for(j = reg_sz; j--;) {
    aux.initEmpty();
    
    if (!(fail & CLAUSE1)) 
      aux = x_aux_pos[j];
    
    if (!(fail & CLAUSE2)) 
      aux = aux | x_aux_neg[j];
    
    FailOnEmpty(*x[j] &= aux);
  }

  if (!unified) {
    aux.initEmpty();
    if (!(fail & CLAUSE1)) 
      aux = d_aux_pos;
    if (!(fail & CLAUSE2)) 
      aux = aux | d_aux_neg;
    FailOnEmpty(*d &= aux);
  }

  FailOnEmpty(d->getSize());
  
  return P.leave();
  
failure:
  OZ_DEBUGPRINT("failed: " << *this);

  return P.fail();
}
