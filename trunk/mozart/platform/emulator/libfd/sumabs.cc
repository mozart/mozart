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
  
#ifdef DEBUG_SUMAC
  cout << "fd_sumAC=" << (void *) fd_sumAC << endl << flush;
#endif
  
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
  } else if (!strcmp(SUM_OP_GEQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax); 
    return pe.impose(new SumACGreaterEqPropagator(OZ_args[0],
						  OZ_args[1],
						  OZ_args[3]));
  } else if (!strcmp(SUM_OP_NEQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax); 
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

//#define DEBUG_LINEQABS 1

OZ_Return LinEqAbsPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in " << *this);
  
  int summax, summin, axmax, axmin, dmax, dmin, i, j, k, dummy, fail, klausel;
  double bound1, bound2;
  OZ_FiniteDomain d_aux_neg, d_aux_pos, aux;
  OZ_Boolean unified, changed, vars_left;
  
#ifdef DEBUG_LINEQABS
  cout<<endl;
  cout<<"vector size: "<<reg_sz<<endl;
#endif
  
  simplify();
  unified = (dpos>-1 && dpos<reg_sz);
  
#ifdef DEBUG_LINEQABS
  cout<<"vector size: "<<reg_sz<<" (nach simplify)"<<endl;
#endif
  
  OZ_FDIntVar d(reg_d);
  DECL_DYN_ARRAY(OZ_FDIntVar, x, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, x_aux_neg, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, x_aux_pos, reg_sz);
  
  for(j=reg_sz; j--;) {
    x[j].read(reg_x[j]);
    x_aux_neg[j] = x_aux_pos[j] =* x[j];
  }
  
  if (unified) {
    reg_a[dpos] -= 1;
    d_aux_neg.initSingleton(0);
    d_aux_pos.initSingleton(0);
  } else { 
    d_aux_neg = d_aux_pos =* d;
  }

  klausel = 1;
  fail = 0; // 0 no fail, 1 positive clause failed, 2 negative clause failed 
  do {
    changed = false;
    summin = summax = (klausel & 2 ? -reg_c : reg_c);
    
    for(j=reg_sz; j--;) {
      axmax = int(double(reg_a[j]) *
		  (klausel & 2 
		   ? x_aux_neg[j].getMaxElem() 
		   : x_aux_pos[j].getMaxElem())
		  );
      axmin = int(double(reg_a[j])*
		  (klausel & 2 
		   ? x_aux_neg[j].getMinElem() 
		   : x_aux_pos[j].getMinElem())
		  );
      if (reg_a[j]<0) {
	summin += axmax;
	summax += axmin;
      }
      if (reg_a[j]>0) {
	summin += axmin; 
	summax += axmax;
      }
    }
    dummy = (klausel & 2 ? d_aux_neg.getSize() : d_aux_pos.getSize());
    (klausel&2 ? d_aux_neg : d_aux_pos) >= summin;
    (klausel&2 ? d_aux_neg : d_aux_pos) <= summax;
    
#ifdef DEBUG_LINEQABS
    cout<<endl;
    cout<<(klausel&2 ? "negativ:" : "positiv:")<<endl;
    cout<<"summin="<<summin<<" summax="<<summax<<endl;
    cout<<summin<<"<=d<="<<summax<<endl;
    cout<<"d_"<<(klausel&2 ? "neg=" : "pos=")
	<<(klausel&2 ? d_aux_neg : d_aux_pos)<<endl;
#endif
      
    if (!(klausel & 2 ? d_aux_neg.getSize() : d_aux_pos.getSize())) {
      fail|=klausel;
    } else {
      changed |= !(dummy == (klausel & 2 
			     ? d_aux_neg.getSize() 
			     : d_aux_pos.getSize())
		   ); 
      dmax = (klausel & 2 ? d_aux_neg.getMaxElem() : d_aux_pos.getMaxElem());
      dmin = (klausel & 2 ? d_aux_neg.getMinElem() : d_aux_pos.getMinElem());
      
      j = reg_sz;
      while(j-- && !(fail & klausel)) { 
	summin=summax = (klausel & 2 ? -reg_c : reg_c);           
	for(k = reg_sz; k--;)
	  if (j!=k) {
	    axmax = int(double(reg_a[k])*
			(klausel & 2 
			 ? x_aux_neg[k].getMaxElem() 
			 : x_aux_pos[k].getMaxElem())
			);
	    axmin = int(double(reg_a[k])*
			(klausel & 2 
			 ? x_aux_neg[k].getMinElem() 
			 : x_aux_pos[k].getMinElem())
			);
	    if (reg_a[k]<0) {
	      summin += axmax;
	      summax += axmin;
	    }
	    if (reg_a[k]>0) {
		summin += axmin; 
		summax += axmax;
	      }
	  }
	bound1 = (dmin - summax) / double(reg_a[j]);
	bound2 = (dmax-summin) / double(reg_a[j]);
	dummy = (klausel & 2 
		 ? x_aux_neg[j].getSize() 
		 : x_aux_pos[j].getSize()
		 );
	if (reg_a[j] < 0) {
	  (klausel & 2 
	   ? x_aux_neg[j] 
	   : x_aux_pos[j]) >= doubleToInt(ceil(bound2));
	  (klausel & 2 
	   ? x_aux_neg[j] 
	   : x_aux_pos[j]) <= doubleToInt(floor(bound1));
        }  
	if (reg_a[j]>0) {
	  (klausel & 2 ? x_aux_neg[j] : x_aux_pos[j])
	    >= doubleToInt(ceil (bound1));
	  (klausel & 2 ? x_aux_neg[j] : x_aux_pos[j])
	    <= doubleToInt(floor(bound2));
	}
	
#ifdef DEBUG_LINEQABS
	cout<<"summin="<<summin<<" summax="<<summax<<endl;
	if (reg_a[j]<0)
	  cout<<doubleToInt(ceil (bound2))<<"<=x"<<j<<"<="
	      <<doubleToInt(floor(bound1))<<endl;
	if (reg_a[j]>0)
	  cout<<doubleToInt(ceil (bound1))<<"<=x"<<j<<"<="
	      <<doubleToInt(floor(bound2))<<endl;
	cout<<'a'<<j<<'='<<reg_a[j]
	    <<" x"<<j<<(klausel&2 ? "_neg=" : "_pos=")
	    <<(klausel&2 ? x_aux_neg[j] : x_aux_pos[j])<<endl;  
#endif
	
	if (!(klausel & 2 ? x_aux_neg[j].getSize() : x_aux_pos[j].getSize())) {
	  fail |= klausel;
	  changed = false;
	} else {
	  changed |= !(dummy == (klausel & 2 
				 ? x_aux_neg[j].getSize() 
				 : x_aux_pos[j].getSize()));
	}
      }
    }
    
#ifdef DEBUG_LINEQABS
    if (fail&klausel) 
      cout<<(klausel&2 ? "negative" : "positive")
	  <<" clause failed"<<endl;  
#endif
    
    if (!changed && klausel & 1) {
     klausel = 2;
     changed = true;
     if (unified) 
       reg_a[dpos] += 2;   // reg_a[dpos]++, Vorz. umk., reg_a[dpos]--
     for(j = reg_sz; j--;) 
       reg_a[j] = -reg_a[j];
    }   
  } while(changed);
  
  if (unified) 
    reg_a[dpos]-=2;    // Vorz. wieder korrigieren
  for(j = reg_sz; j--;) 
    reg_a[j]=-reg_a[j];
  for(j=reg_sz; j--;) {
    aux.initEmpty();
    if (!(fail & 1)) 
      aux = x_aux_pos[j];
    if (!(fail&2)) 
      aux = aux | x_aux_neg[j];
    FailOnEmpty(*x[j]&=aux);
  }
  if (!unified) {
    aux.initEmpty();
    if (!(fail&1)) 
      aux = d_aux_pos;
    if (!(fail&2)) 
      aux = aux|d_aux_neg;
    *d &= aux;
  }
  FailOnEmpty(d->getSize());
  vars_left=d.leave();
  for(j=reg_sz; j--;) 
    vars_left |= x[j].leave();

  OZ_DEBUGPRINT("out " << *this);

  return (vars_left ? SLEEP : ENTAILED);
  
failure:
  OZ_DEBUGPRINT("failed: " << *this);

  for(j = reg_sz; j--;) 
    x[j].fail(); 
  d.fail();
  return FAILED;
}

//-----------------------------------------------------------------------------

//#define DEBUG_LINLEQABS 1

OZ_Return LinLessEqAbsPropagator::propagate(void)
{
  OZ_DEBUGPRINT("in " << *this);
  
  int summin, dmax, i, j, k, dummy, klausel;
  double bound1, bound2;
  OZ_FiniteDomain d_aux;
  OZ_Boolean unified, changed, vars_left;
  
#ifdef DEBUG_LINLEQABS
  cout<<endl;
  cout<<"vector size: "<<reg_sz<<endl;
#endif
  
  simplify();
  unified = (dpos > -1 && dpos < reg_sz);

#ifdef DEBUG_LINLEQABS
  cout<<"vector size: "<<reg_sz<<" (nach simplify)"<<endl;
#endif

  OZ_FDIntVar d(reg_d);
  DECL_DYN_ARRAY(OZ_FDIntVar,x,reg_sz);
  for(j = reg_sz; j--;)
    x[j].read(reg_x[j]);
  if (unified) {
    reg_a[dpos] -= 1;
    d_aux.initSingleton(0);
  } else {
    d_aux=*d;
  }

  klausel = 1;
  do {
    changed = false;
    summin = (klausel & 2 ? -reg_c : reg_c);
    
    for(j = reg_sz; j--;) {
      if (reg_a[j] < 0) 
	summin += int(double(reg_a[j]) * x[j]->getMaxElem());
      if (reg_a[j] > 0) 
	summin += int(double(reg_a[j]) * x[j]->getMinElem());
    }
    dummy = d_aux.getSize();
    d_aux >= summin;
    
#ifdef DEBUG_LINLEQABS
    cout<<endl;
    cout<<(klausel&2 ? "negativ:" : "positiv:")<<endl;
    cout<<"summin="<<summin<<endl;
    cout<<"d>="<<summin<<endl;
    cout<<"d="<<d_aux<<endl;
#endif
    
    FailOnEmpty(d_aux.getSize());
    changed |= !(dummy == d_aux.getSize()); 
   
   dmax = d_aux.getMaxElem();
   for(j = reg_sz; j--;) { 
     summin = (klausel&2 ? -reg_c : reg_c);
     for(k = reg_sz; k--;)
       if (j!=k) {
	 if (reg_a[k] < 0) 
	   summin += int(double(reg_a[k]) * x[k]->getMaxElem());
	 if (reg_a[k]>0) 
	   summin += int(double(reg_a[k]) * x[k]->getMinElem());
       }
     
     bound1 = (dmax-summin) / double(reg_a[j]);
     dummy = x[j]->getSize();
     if (reg_a[j] < 0)
       *x[j] >= doubleToInt(ceil (bound1));
     if (reg_a[j]>0) 
       *x[j] <= doubleToInt(floor(bound1));
     
#ifdef DEBUG_LINLEQABS
     cout<<"summin="<<summin<<endl;
     if (reg_a[j]<0) cout<<doubleToInt(ceil (bound1))<<"<=x"<<j<<endl;
     if (reg_a[j]>0) cout<<"x"<<j<<"<="<<doubleToInt(ceil (bound1))<<endl;
     cout<<'a'<<j<<'='<<reg_a[j]<<" x"<<j<<"="<<*x[j]<<endl;  
#endif
     
     FailOnEmpty(x[j]->getSize());
     changed|=!(dummy==x[j]->getSize());
   }
   
   if (!changed && klausel & 1) {
     klausel = 2;
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
 if (!unified) *
		 d &= d_aux;
 FailOnEmpty(d->getSize());
 vars_left = d.leave();
 for(j = reg_sz; j--;) 
   vars_left |= x[j].leave();

 OZ_DEBUGPRINT("out " << *this);
 return (vars_left ? SLEEP : ENTAILED);
 
failure:
 OZ_DEBUGPRINT("failed: " << *this);
 
 for(j = reg_sz;j--;) 
   x[j].fail(); 
 d.fail();
 return FAILED;
}

//-----------------------------------------------------------------------------

//#define DEBUG_LINNEQABS 1

OZ_Return LinNotEqAbsPropagator::propagate(void)
{
 OZ_DEBUGPRINT("in " << *this);


 OZ_DEBUGPRINT("out " << *this);
 return ENTAILED;
 
failure:
 OZ_DEBUGPRINT("failed: " << *this);
 return FAILED;
}

//-----------------------------------------------------------------------------

//#define DEBUG_GEQABS 1

OZ_Return LinGreaterEqAbsPropagator::propagate(void)
{
 OZ_DEBUGPRINT("in " << *this);

 OZ_DEBUGPRINT("out " << *this);
 return ENTAILED;
 
 failure:
 OZ_DEBUGPRINT("failed: " << *this);
 return FAILED;
}
