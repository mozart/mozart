/*
 *  Authors:
 *    Markus Loeckelt (loeckelt@ps.uni-sb.de)
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

#include "sumd.hh"

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_dsum, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD","OZ_EM_LIT","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 1, expectLiteral);
  OZ_EXPECT(pe, 0, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectIntVarAny);

  switch (getSumOps(OZ_in(1))) {
  case sum_ops_eq:
    return pe.impose(new isumEqProp(OZ_in(0), OZ_in(2)));
  case sum_ops_neq:
    return pe.impose(new isumNEqProp(OZ_in(0), OZ_in(2)));
  default: ;
  }
  ERROR_UNEXPECTED_OPERATOR_NOIN(1);
}
OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_dsumC, 4, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT","OZ_EM_VECT OZ_EM_FD","OZ_EM_LIT
                   ","OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 2, expectLiteral);
  OZ_EXPECT(pe, 0, expectVectorInt);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 3, expectIntVarAny);
  SAMELENGTH_VECTORS(0, 1);

  switch (getSumOps(OZ_in(2))) {
  case sum_ops_eq:
    return pe.impose(new isumcEqProp(OZ_in(0),OZ_in(1),OZ_in(3)));
  case sum_ops_neq:
    return pe.impose(new isumcNEqProp(OZ_in(0), OZ_in(1), OZ_in(3)));
  default: ;
  }
  ERROR_UNEXPECTED_OPERATOR_NOIN(2);
}
OZ_BI_end

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

  _OZ_DEBUGPRINT(("trimRemainders(factor=%d, lo=%d, hi=%d)",
                  factor,lo,hi));

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
    _OZ_DEBUGPRINT(("sum(%d) [%d; %d]",except,losum,hisum));


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
  _OZ_DEBUGPRINT((" iLinEqProp::propagate %s", toString()));
  // if vector nullified, return failed if c!=0, else done:
  long old_domains_size;    // these record the overall domain size sum to
  long new_domains_size =0; // check if something has changed.

  simplify_on_equality();

  int all_ones =1;          // if abs of all factors is 1, simpler calculation.

  int i;
  for (i =0; i < reg_sz; i++)
    if (reg_a[i]*reg_a[i] != 1)
      all_ones = 0;

  _OZ_DEBUGPRINT(("reg_sz=%d all_ones=%d",reg_sz,all_ones));
  if (reg_sz == 0) return reg_c ? FAILED : OZ_ENTAILED;

  DECL_DYN_ARRAY(OZ_FDIntVar, var, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, buffer, reg_sz);

  for (i =0; i < reg_sz; i++) {
    var[i].read(reg_x[i]);
    buffer[i].initEmpty();
  }
  PropagatorController_VV P(reg_sz, var);

  // loop until nothing is changed anymore
  do {
    // reduce: Variable that is currently being reduced
    old_domains_size = new_domains_size;
    new_domains_size = 0;

    int reduce;
    for (reduce =0; reduce < reg_sz; reduce++) {
      // AuxDom is used for building up a new domain of possible values
      OZ_FiniteDomain AuxDom;
      AuxDom.initEmpty();
      // summation for variable 'reduce', beginning at vector position 0 with
      // initial lower and upper bound 0. Sum is initialised with reg_c.

      sum(AuxDom, var, reduce, 0, reg_c, reg_c, 0);

      _OZ_DEBUGPRINT(("sum done!"));

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

    _OZ_DEBUGPRINT(("intermediate %s", toString()));

    DECL_DYN_ARRAY(int, curr_val, reg_sz);
    for (reduce =0; reduce < reg_sz; reduce++) {

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

          _OZ_DEBUGPRINT(("after do-while"));

          if (!buffer[reduce].isIn(elem)) {
            *var[reduce] -= elem;
            _OZ_DEBUGPRINT(("*var[%d-=%d",reduce,elem));
          }
          _OZ_DEBUGPRINT(("Exited."));
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
   _OZ_DEBUGPRINT(("failAll: FAILED"));
  return P.fail();

do_leave:
  OZ_Return value =P.leave();
  if (value == OZ_ENTAILED) _OZ_DEBUGPRINT(("ENTAILED"));
  else                      _OZ_DEBUGPRINT(("SLEEP"));
  return value;
}

//-----------------------------------------------------------------------------

OZ_Return iLinNEqProp::propagate(void) {
  _OZ_DEBUGPRINT(("isumcNEqProp: invoked"));
  // if vector nullified, return failed if c!=0, else done:
  long old_domains_size;    // these record the overall domain size sum to
  long new_domains_size =0; // check if something has changed.

  simplify_on_equality();

  int all_ones =1;          // if abs of all factors is 1, simpler calculation.
  int sum_sizes=0;
  int i;
  for (i =0; i < reg_sz; i++)
    if (reg_a[i]*reg_a[i] != 1)
      all_ones = 0;

  _OZ_DEBUGPRINT(("reg_sz=%d all_ones=",reg_sz,all_ones));

  // all vars gone and still a remainder? good...
  if (reg_sz == 0) {
    _OZ_DEBUGPRINT(("reg_sz==0)"));
    return reg_c ? OZ_ENTAILED : FAILED;
  }

  DECL_DYN_ARRAY(OZ_FDIntVar, var, reg_sz);
  DECL_DYN_ARRAY(OZ_FiniteDomain, buffer, reg_sz);

  for (i =0; i < reg_sz; i++) {
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

    int reduce;
    for (reduce =0; reduce < reg_sz; reduce++) {
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
    for (reduce =0; reduce < reg_sz; reduce++) {

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
            _OZ_DEBUGPRINT(("*var[%s-=",reduce,elem));
          }
          _OZ_DEBUGPRINT(("Exited."));
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
   _OZ_DEBUGPRINT(("Equality FAILED -> Inequality ENTAILED"));
  P.fail();
  return OZ_ENTAILED;

do_leave:
  // if Equality is entailed, fail; otherwise, sleep
  OZ_Return value =P.leave();
  if (value == OZ_ENTAILED) {

    // all variables determined? then fail
    if (sum_sizes == reg_sz) {
      _OZ_DEBUGPRINT(("Equality ENTAILED,vars determined -> Inequality FAILED"));
      return FAILED;
    }
    else {
      _OZ_DEBUGPRINT(("SLEEP (_may_ be equal)"));
      return SLEEP;
    }
  }
  else {
    _OZ_DEBUGPRINT(("SLEEP"));
    return SLEEP;
  }
}

//-----------------------------------------------------------------------------
OZ_PropagatorProfile isumEqProp::profile;
OZ_PropagatorProfile isumNEqProp::profile;

OZ_PropagatorProfile isumcEqProp::profile;
OZ_PropagatorProfile isumcNEqProp::profile;

//-----------------------------------------------------------------------------
// eof
