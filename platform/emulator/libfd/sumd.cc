/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: loeckelt
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "sumd.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_dsum, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD","OZ_EM_LIT","OZ_EM_FD);
  
  PropagatorExpect pe;

  OZ_EXPECT(pe, 1, expectLiteral);

  char *op = OZ_atomToC(OZ_args[1]);
  if (!strcmp(SUM_OP_NEQ, op)) {
    OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 2, expectIntVarMinMax);
    return pe.impose(new isumNEqProp(OZ_args[0], OZ_args[2]));
  } else {
    OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 2, expectIntVarMinMax);

    if (!strcmp(SUM_OP_EQ, op)) {
      return pe.impose(new isumEqProp(OZ_args[0], OZ_args[2]));
    } else if (!strcmp(SUM_OP_LEQ, op)) {
      return pe.impose(new isumLeqProp(OZ_args[0], OZ_args[2]));
    } else if (!strcmp(SUM_OP_LT, op)) {
      return pe.impose(new isumLtProp(OZ_args[0], OZ_args[2]));
    } else if (!strcmp(SUM_OP_GEQ, op)) {
      return pe.impose(new isumGeqProp(OZ_args[0], OZ_args[2]));
    } else if (!strcmp(SUM_OP_GT, op)) {
      return pe.impose(new isumGtProp(OZ_args[0], OZ_args[2]));
    } 
  }

  ERROR_UNEXPECTED_OPERATOR(1);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_dsumC, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT","OZ_EM_VECT OZ_EM_FD","OZ_EM_LIT
		   ","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 2, expectLiteral);
  OZ_EXPECT(pe, 0, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  char *op = OZ_atomToC(OZ_args[2]);

  if (!strcmp(SUM_OP_NEQ, op)) {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax);
    return pe.impose(new isumcNEqProp(OZ_args[0], OZ_args[1], OZ_args[3]));
  } 
  else {
    OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
    OZ_EXPECT(pe, 3, expectIntVarMinMax);

    if (!strcmp(SUM_OP_EQ, op)) {
      return pe.impose(new isumcEqProp(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_LEQ, op)) {
      return pe.impose(new isumcLeqProp(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_LT, op)) {
      return pe.impose(new isumcLtProp(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_GEQ, op)) {
      return pe.impose(new isumcGeqProp(OZ_args[0],OZ_args[1],OZ_args[3]));
    } else if (!strcmp(SUM_OP_GT, op)) {
      return pe.impose(new isumcGtProp(OZ_args[0],OZ_args[1],OZ_args[3]));
    } 
  }

  ERROR_UNEXPECTED_OPERATOR(2);
}
OZ_C_proc_end

//=============================================================================

int isumProp::getNextInterval(OZ_FDIntVar F, int position, 
			      int &lower, int &upper) {
  // returns 0 if there is no allowed interval larger than 'position' in the
  // domain of T; otherwise, returns 1 and assigns the lower and upper bound
  // for it to the respective variables. Position=-1 means get first interval.
    
  // find lower bound:

  if (position == -1) 
    lower = F->getMinElem();
  else { 
    int x = F->getUpperIntervalBd(position); // position in interval ensured
    lower = F->getNextLargerElem(x);  
  }
  if (lower == -1) return 0; // end of domain
  // now find next upper bound
  upper = F->getUpperIntervalBd(lower);
  return 1;  
}

void isumProp::trimRemainders(int factor, int &lo, int &hi) {
  // trims an interval to bounds that actually are divisible by 'factor'.
  // additionally swaps 'hi' and 'lo' to initialize new interval.
   int remainder, newhi, newlo;
    do {
         newhi = -(lo / factor);
         remainder = lo % factor;
         lo++;
    } while(remainder);

    do {
         newlo = -(hi / factor);
         remainder = hi % factor;
         hi--;
    } while(remainder);
    lo = newlo;
    hi = newhi;
}

int isumProp::sum (OZ_FiniteDomain &AuxDom, OZ_FDIntVar var[], int except, 
		    int pos, int losum, int hisum, int typ) {
  // typ = 0: equality, typ = 1: 'less than or equal'

  int currmin = reg_a[except] * var[except]->getMinElem();
  int currmax = reg_a[except] * var[except]->getMaxElem();

  if (pos == reg_sz) { // end of recursion
 
    if (reg_a[except] < 0) { // negative sign -> swap upper and lower bound
      int swap = losum;  
      losum = hisum;  
      hisum = swap;
    }
    // make sure that there are no remainders, and negate losum and
    // hisum, so that the overall sum is zero:
    trimRemainders(reg_a[except], losum, hisum);

    if (losum < 0) losum = 0;
    _OZ_DEBUGPRINT("sum("<<except<<") ["<<losum<<"; "<<hisum<<"]");

            
    OZ_FiniteDomain NewInterval;
    if (typ == 0) 
      NewInterval.initRange(losum, hisum);
    else 
      // positive sign -> all values up to newhi
      // negative sign -> all values >= newlo
      if (reg_a[except] < 0) 
	NewInterval.initRange(losum, OZ_getFDSup());
      else                   
	NewInterval.initRange(0, hisum);

    AuxDom = AuxDom | NewInterval;

    // out of bounds check: maybe we can save a little work
    if (var[except]->getMinElem() > hisum) return 1;
    if (var[except]->getMaxElem() < losum) return -1;
    return 0;
  }

  if (pos == except) { // do nothing and recurse
    return sum(AuxDom, var, except, pos+1, losum, hisum, typ);
  }

  // otherwise call recursively for all intervals
  int interval_pos = -1, ret_val =0, lo, hi;
  while (getNextInterval(var[pos], interval_pos, lo, hi)) {
     // if the factor is negative, the upper interval bound adds to the
     // lower bound of the sum; and vice versa.

    if (reg_a[pos] > 0) {
       ret_val = sum(AuxDom, var, except, pos+1, 
          losum + reg_a[pos] * lo, hisum + reg_a[pos] * hi, typ);
        // sum already too great? then leave it alone
        if ((losum + reg_a[pos]*lo+currmax > 0) && (ret_val==1))
          return 1;
       
    }
    else { // reg_a[pos] < 0
      ret_val = sum(AuxDom, var, except, pos+1, 
          losum + reg_a[pos] * hi, hisum + reg_a[pos] * lo, typ);
      // sum too small? great...
      if ((hisum + reg_a[pos]*hi+currmin < 0 ) && (ret_val==-1)) 
        return -1;
    }
    interval_pos = hi; // align for next interval to be found
  }   
  return 0;
}

OZ_Return iLinEqProp::propagate(void) {
  _OZ_DEBUGPRINT("isumcEqProp: invoked");
  // if vector nullified, return failed if c!=0, else done:  
  long old_domains_size;    // these record the overall domain size sum to
  long new_domains_size =0; // check if something has changed.

  simplify_on_equality();

  int all_ones =1;          // if abs of all factors is 1, simpler calculation.

  for (int i =0; i < reg_sz; i++) 
    if (reg_a[i]*reg_a[i] != 1) 
      all_ones = 0;

  _OZ_DEBUGPRINT("reg_sz="<< reg_sz<<" all_ones="<<all_ones);
  if (reg_sz == 0) return reg_c ? FAILED : OZ_ENTAILED;

  DECL_DYN_ARRAY(OZ_FDIntVar, var, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, buffer, reg_sz);

  for (int i =0; i < reg_sz; i++) { 
    var[i].read(reg_x[i]);
    buffer[i].initEmpty();    
  }
  PropagatorController_VV P(reg_sz, var);

  // loop until nothing is changed anymore
  do {
    // reduce: Variable that is currently being reduced
    old_domains_size = new_domains_size;
    new_domains_size = 0;

    for (int reduce =0; reduce < reg_sz; reduce++) {
      // AuxDom is used for building up a new domain of possible values
      OZ_FiniteDomain AuxDom;
      AuxDom.initEmpty();
      // summation for variable 'reduce', beginning at vector position 0 with
      // initial lower and upper bound 0. Sum is initialised with reg_c.
      
      sum(AuxDom, var, reduce, 0, reg_c, reg_c, 0);

      // now intersect 'AuxDom' with old domain of 'reduce' to get new bounds
      if ((*var[reduce] &= AuxDom) == 0) goto do_fail;
    }    

    // all factors have absolute value 1 : done.
    if (all_ones == 1) goto do_leave;
    
    // remove impossible values: The ugly part.
    // for all values v of all domains d[i]:
    // - if v in buffer[i] (initially empty) then done.
    // - otherwise: if a summing combination giving v exits, write v into
    //   buffer[i].
    // at the end of the day, the elements in the buffer are exactly the
    // possible ones.

    DECL_DYN_ARRAY(int, curr_val, reg_sz);
    for (int reduce =0; reduce < reg_sz; reduce++) {

      for (int k=0; k<reg_sz; k++) 
        curr_val[k]=var[k]->getMinElem(); // init curr_val array
      int elem = -1;

      // do successively for the elements of var[reduce]
      while ((elem=var[reduce]->getNextLargerElem(elem)) != -1) {
        if (!buffer[reduce].isIn(elem)) {
          // get reverse-lexicographically next value
          int to_increase =0;

	  do {
            if (to_increase < reg_sz) {
	      // make sum and test it
	      int csum =reg_c;
              curr_val[reduce] = elem;
              for (int s =0; s < reg_sz; s++) 
		csum += reg_a[s] * curr_val[s];
              if (csum == 0) // add to buffer
                for (int s =0; s < reg_sz; s++) 
		  buffer[s] += curr_val[s];
	    }

            // while 'overflow' and still in bounds: get next vector
            to_increase =0;
	    if (reduce == 0) to_increase++;

            while((to_increase < reg_sz) && 
                   ((curr_val[to_increase] = 
		   var[to_increase]->getNextLargerElem(curr_val[to_increase]))
		   == -1)) {
              curr_val[to_increase] = var[to_increase]->getMinElem();
              to_increase++;
              if (to_increase == reduce) to_increase++;
            } 

	  } while (to_increase < reg_sz);

          if (!buffer[reduce].isIn(elem)) {
            *var[reduce] -= elem;
	    _OZ_DEBUGPRINT("*var["<<reduce<<"-="<<elem);
          }
          _OZ_DEBUGPRINT("Exited.");
        }
      }
    }
    for (int x =0; x < reg_sz; x++) {
      int size=var[x]->getSize();
      if (size == 0) goto do_fail;
      new_domains_size += size;
    }
  } while (new_domains_size != old_domains_size); // something changed, repeat
  goto do_leave;

do_fail:
   _OZ_DEBUGPRINT("failAll: FAILED");
  return P.fail();

do_leave:
  OZ_Return value =P.leave();
  if (value == OZ_ENTAILED) _OZ_DEBUGPRINT("ENTAILED");
  else                      _OZ_DEBUGPRINT("SLEEP");
  return value;
}

//-----------------------------------------------------------------------------

OZ_Return iLinLessEqProp::propagate(void) {
  _OZ_DEBUGPRINT("isumcLeqProp: invoked");
  // if vector nullified, return failed if c!=0, else done:  
  long old_domains_size;    // these record the overall domain size sum to
  long new_domains_size =0; // check if something has changed.

  simplify_on_equality();

  int all_ones =1;          // if abs of all factors is 1, simpler calculation.

  for (int i=0; i<reg_sz; i++) 
    if (reg_a[i]*reg_a[i] != 1) 
      all_ones=0;

  _OZ_DEBUGPRINT("reg_sz="<< reg_sz);
  if (reg_sz == 0) return reg_c ? FAILED : OZ_ENTAILED;

  DECL_DYN_ARRAY(OZ_FDIntVar, var, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, buffer, reg_sz);

  for (int i =0; i < reg_sz; i++) { 
    var[i].read(reg_x[i]);
    buffer[i].initEmpty();
  }
  PropagatorController_VV P(reg_sz, var);

  // loop until nothing is changed anymore
  do {
    // reduce: Variable that is currently being reduced
    old_domains_size = new_domains_size;
    new_domains_size = 0;

    for (int reduce =0; reduce < reg_sz; reduce++) {
      // AuxDom is used for building up a new domain of possible values
      OZ_FiniteDomain AuxDom;
      AuxDom.initEmpty();

      // summation for variable 'reduce', beginning at vector position 0 with
      // initial lower and upper bound 0. Reg_c gets added at recursion end.

      sum(AuxDom, var, reduce, 0, reg_c, reg_c, 1);

      // now intersect 'AuxDom' with old domain of 'reduce' to get new bounds
      if ((*var[reduce] &= AuxDom) == 0) goto do_fail;
    }

    // all factors have absolute value 1 : done.
    if (all_ones == 1) goto do_leave;

    // remove impossible values: The ugly part.
    // for all values v of all domains d[i]:
    // - if v in buffer[i] (initially empty) then done.
    // - otherwise: if a summing combination giving v exits, write v into
    //   buffer[i].
    // at the end of the day, the elements in the buffer are exactly the
    // possible ones.
    DECL_DYN_ARRAY(int, curr_val, reg_sz);
    for (int reduce =0; reduce < reg_sz; reduce++) {
      for (int k=0; k<reg_sz; k++) 
        curr_val[k]=var[k]->getMinElem(); // init curr_val array
      int elem = -1;

      // do successively for the elements of var[reduce]
      while ((elem = var[reduce]->getNextLargerElem(elem)) != -1) {
        if (!buffer[reduce].isIn(elem)) {
          // get reverse-lexicographically next value
          int to_increase =0;

	  do {
            if (to_increase < reg_sz) {
	      // make sum and test it
	      int csum =reg_c;
              curr_val[reduce] = elem;
              for (int s =0; s < reg_sz; s++) csum += reg_a[s]*curr_val[s];
              if (csum <= 0) // sum still negative -> constraint fulfilled
                for (int s =0; s < reg_sz; s++) buffer[s] += curr_val[s];
	    }

            // while 'overflow' and still in bounds: get next vector
            to_increase = 0;
	    if (reduce == 0) to_increase++;

            while((to_increase < reg_sz) && 
                   ((curr_val[to_increase] = 
		   var[to_increase]->getNextLargerElem(curr_val[to_increase]))
		   == -1)) {
             
              curr_val[to_increase] = var[to_increase]->getMinElem();
              to_increase++;
              if (to_increase == reduce) to_increase++;
            } 
	  } while (to_increase < reg_sz);

          if (!buffer[reduce].isIn(elem)) {
            *var[reduce] -= elem;
	    _OZ_DEBUGPRINT("*var["<<reduce<<"-="<<elem);
          }
          _OZ_DEBUGPRINT("Exited.");
        }
      }
    }
    for (int x =0; x < reg_sz; x++) {
      int size =var[x]->getSize();
      if (size == 0) goto do_fail;
      new_domains_size += size;
    }
  } while (new_domains_size != old_domains_size); // something changed, repeat

  goto do_leave;

do_fail:
   _OZ_DEBUGPRINT("failAll: FAILED");
  return P.fail();

do_leave:
  // bugfix : not recognizing 'entailed'
  // check if sum over all max/mins is smaller than upper bound
  _OZ_DEBUGPRINT("do_leave");

  int upsum = reg_c;
  for (int i =0; i < reg_sz; i++) {
    if (reg_a[i] > 0) upsum += reg_a[i]*var[i]->getMaxElem();
    else              upsum += reg_a[i]*var[i]->getMinElem();
  } 
  _OZ_DEBUGPRINT("upsum="<<upsum<<" reg_c="<<reg_c);

  int already_entailed =(upsum <= 0);

  OZ_Return value =P.leave();

  if (value == OZ_ENTAILED) _OZ_DEBUGPRINT("ENTAILED");
  else                      _OZ_DEBUGPRINT("SLEEP");
  if (already_entailed) {
    _OZ_DEBUGPRINT("ALREADY_ENTAILED");
    return OZ_ENTAILED; 
  }
  else return value;
}


//-----------------------------------------------------------------------------

OZ_Return iLinNEqProp::propagate(void) {
  _OZ_DEBUGPRINT("isumcNEqProp: invoked");
  // if vector nullified, return failed if c!=0, else done:  
  long old_domains_size;    // these record the overall domain size sum to
  long new_domains_size =0; // check if something has changed.

  simplify_on_equality();

  int all_ones =1;          // if abs of all factors is 1, simpler calculation.
  int sum_sizes=0;
  for (int i =0; i < reg_sz; i++) 
    if (reg_a[i]*reg_a[i] != 1) 
      all_ones = 0;

  _OZ_DEBUGPRINT("reg_sz="<< reg_sz<<" all_ones="<<all_ones);

  // all vars gone and still a remainder? good...
  if (reg_sz == 0) {
    _OZ_DEBUGPRINT("reg_sz==0)");
    return reg_c ? OZ_ENTAILED : FAILED;
  }

  DECL_DYN_ARRAY(OZ_FDIntVar, var, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, buffer, reg_sz);

  for (int i =0; i < reg_sz; i++) { 
    var[i].readEncap(reg_x[i]);
    sum_sizes += var[i]->getSize();
    buffer[i].initEmpty();    
  }

  PropagatorController_VV P(reg_sz, var);

  // loop until nothing is changed anymore
  do {
    // reduce: Variable that is currently being reduced
    old_domains_size = new_domains_size;
    new_domains_size = 0;

    for (int reduce =0; reduce < reg_sz; reduce++) {
      // AuxDom is used for building up a new domain of possible values
      OZ_FiniteDomain AuxDom;
      AuxDom.initEmpty();

      // summation for variable 'reduce', beginning at vector position 0 with
      // initial lower and upper bound 0. Sum is initialised with reg_c.
      
      sum(AuxDom, var, reduce, 0, reg_c, reg_c, 0);

      // now intersect 'AuxDom' with old domain of 'reduce' to get new bounds
      if ((*var[reduce] &= AuxDom) == 0) goto do_fail;
    }    
    // all factors have absolute value 1 : done.
    if (all_ones == 1) goto do_leave;
    
    // remove impossible values: The ugly part.
    // for all values v of all domains d[i]:
    // - if v in buffer[i] (initially empty) then done.
    // - otherwise: if a summing combination giving v exits, write v into
    //   buffer[i].
    // at the end of the day, the elements in the buffer are exactly the
    // possible ones.

    DECL_DYN_ARRAY(int, curr_val, reg_sz);
    for (int reduce =0; reduce < reg_sz; reduce++) {

      for (int k=0; k<reg_sz; k++) 
        curr_val[k]=var[k]->getMinElem(); // init curr_val array
      int elem = -1;

      // do successively for the elements of var[reduce]
      while ((elem=var[reduce]->getNextLargerElem(elem)) != -1) {
        if (!buffer[reduce].isIn(elem)) {
          // get reverse-lexicographically next value
          int to_increase =0;
	  do {
            if (to_increase < reg_sz) {
	      // make sum and test it
	      int csum =reg_c;
              curr_val[reduce] = elem;
              for (int s =0; s < reg_sz; s++) csum += reg_a[s]*curr_val[s];
              if (csum == 0) // add to buffer
                for (int s =0; s < reg_sz; s++) 
		  buffer[s] += curr_val[s];
	    }
            // while 'overflow' and still in bounds: get next vector
            to_increase =0;
	    if (reduce == 0) to_increase++;

            while((to_increase < reg_sz) && 
                   ((curr_val[to_increase] = 
		   var[to_increase]->getNextLargerElem(curr_val[to_increase]))
		   == -1)) {

              curr_val[to_increase] = var[to_increase]->getMinElem();
              to_increase++;
              if (to_increase == reduce) to_increase++;
            } 

	  } while (to_increase < reg_sz);
          if (!buffer[reduce].isIn(elem)) {
            *var[reduce] -= elem;
	    _OZ_DEBUGPRINT("*var["<<reduce<<"-="<<elem);
          }
          _OZ_DEBUGPRINT("Exited.");
        }
      }
    }
    for (int x =0; x < reg_sz; x++) {
      int size=var[x]->getSize();
      if (size == 0) goto do_fail;
      new_domains_size += size;
    }
  } while (new_domains_size != old_domains_size); // something changed, repeat
  goto do_leave;

do_fail:
  // Equality has failed, so Inequality is entailed
   _OZ_DEBUGPRINT("Equality FAILED -> Inequality ENTAILED");
  P.fail();
  return OZ_ENTAILED;

do_leave:
  // if Equality is entailed, fail; otherwise, sleep
  OZ_Return value =P.leave();
  if (value == OZ_ENTAILED) {
    
    // all variables determined? then fail
    if (sum_sizes == reg_sz) {
      _OZ_DEBUGPRINT("Equality ENTAILED,vars determined -> Inequality FAILED");
      return FAILED; 
    }
    else {
      _OZ_DEBUGPRINT("SLEEP (_may_ be equal)");
      return SLEEP;
    }
  }
  else {
    _OZ_DEBUGPRINT("SLEEP");
    return SLEEP;
  }
}

//-----------------------------------------------------------------------------
OZ_CFun isumEqProp::spawner = fdp_dsum;
OZ_CFun isumNEqProp::spawner = fdp_dsum;
OZ_CFun isumLeqProp::spawner = fdp_dsum;
OZ_CFun isumLtProp::spawner = fdp_dsum;
OZ_CFun isumGeqProp::spawner = fdp_dsum;
OZ_CFun isumGtProp::spawner = fdp_dsum;

OZ_CFun isumcEqProp::spawner = fdp_dsumC;
OZ_CFun isumcNEqProp::spawner = fdp_dsumC;
OZ_CFun isumcLeqProp::spawner = fdp_dsumC;
OZ_CFun isumcLtProp::spawner = fdp_dsumC;
OZ_CFun isumcGeqProp::spawner = fdp_dsumC;
OZ_CFun isumcGtProp::spawner = fdp_dsumC;

//-----------------------------------------------------------------------------
// eof
