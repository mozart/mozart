/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "fdbuilti.hh"

// ---------------------------------------------------------------------
//                  Finite Domains Distribution Built-ins
// ---------------------------------------------------------------------

inline
static int getSize(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getSize() : 2;
}

inline
static int getMin(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().minElem() : 0;
}

inline
static int getMax(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().maxElem() : 1;
}

inline
static int getMid(TaggedRef var) {
  if (isGenFDVar(var)) {
    OZ_FiniteDomain &dom = tagged2GenFDVar(var)->getDom();
    return dom.next((dom.minElem() + dom.maxElem()) / 2);
  } else {
    return 0;
  }
}

inline
static int getConstraints(TaggedRef var) {
  Assert(isCVar(var));
  return tagged2CVar(var)->getSuspListLength();
}


OZ_C_proc_begin(BIfdDistribute, 5) { 
  TaggedRef tagged_vector = deref(OZ_getCArg(0));
  TaggedRef order_spec    = deref(OZ_getCArg(1));
  TaggedRef value_spec    = deref(OZ_getCArg(2));
  TaggedRef out_variable  = OZ_getCArg(3); 
  TaggedRef out_value     = OZ_getCArg(4); 
  TaggedRef variable, d_variable, value;

  if (!isSTuple(tagged_vector))
    return OZ_unify(out_value, makeTaggedSmallInt(-1));

  SRecord *vector = tagged2SRecord(tagged_vector);
  int      width  = vector->getWidth();
  int      cur    = 0;

  // Skip all elements which are small ints already
  do {
    variable   = vector->getArg(cur);
    d_variable = deref(variable);
  } while (isSmallInt(d_variable) && (++cur < width));

  // No elements left
  if (cur==width)
    return OZ_unify(out_value, makeTaggedSmallInt(-1));

  Assert(isGenFDVar(d_variable) || isGenBoolVar(d_variable));

  if (cur!=0)
    vector->setArg(0,variable);

  if (literalEq(order_spec, AtomNaive) && (cur>0)) {
    for (int i=cur+1; i<width; i++)
      vector->setArg(i-cur,vector->getArg(i));

  } else  if (literalEq(order_spec, AtomSize)) {
    int minsize = getSize(d_variable);
    int new_cur = 1;

    for (int i=cur+1; i<width; i++) {
      TaggedRef   arg = vector->getArg(i);
      TaggedRef d_arg = deref(arg);

      if (isSmallInt(d_arg))
	continue;

      if (i != new_cur)
	vector->setArg(new_cur, arg);
      new_cur++;

      Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

      int cursize = getSize(d_arg);

      if (cursize < minsize) {
	minsize    = cursize;
	variable   = arg; 
	d_variable = d_arg;
      }
      
    }
  } else  if (literalEq(order_spec, AtomMax)) {
    int maxmax  = getMax(d_variable);
    int new_cur = 1;

    for (int i=cur+1; i<width; i++) {
      TaggedRef   arg = vector->getArg(i);
      TaggedRef d_arg = deref(arg);

      if (isSmallInt(d_arg))
	continue;

      if (i != new_cur)
	vector->setArg(new_cur, arg);
      new_cur++;

      Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

      int curmax = getMax(d_arg);

      if (curmax < maxmax) {
	maxmax     = curmax;
	variable   = arg; 
	d_variable = d_arg;
      }
      
    }
  } else  if (literalEq(order_spec, AtomMin)) {
    int minmin  = getMin(d_variable);
    int new_cur = 1;

    for (int i=cur+1; i<width; i++) {
      TaggedRef   arg = vector->getArg(i);
      TaggedRef d_arg = deref(arg);

      if (isSmallInt(d_arg))
	continue;

      if (i != new_cur)
	vector->setArg(new_cur, arg);
      new_cur++;

      Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

      int curmin = getSize(d_arg);

      if (curmin < minmin) {
	minmin     = curmin;
	variable   = arg; 
	d_variable = d_arg;
      }
      
    }
  } else  if (literalEq(order_spec, AtomConstraints)) {
    int minsize = getSize(d_variable);
    int mincon  = getConstraints(d_variable);
    int new_cur = 1;

    for (int i=cur+1; i<width; i++) {
      TaggedRef   arg = vector->getArg(i);
      TaggedRef d_arg = deref(arg);

      if (isSmallInt(d_arg))
	continue;

      if (i != new_cur)
	vector->setArg(new_cur, arg);
      new_cur++;

      Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

      int curcon  = getConstraints(d_arg);
      
      if (curcon < mincon)
	continue;

      if (curcon==mincon) {
	int cursize = getSize(d_arg);

	if (cursize < minsize) {
	  minsize = cursize;
	} else {
	  continue;
	}
      }

      mincon     = curcon;
      variable   = arg; 
      d_variable = d_arg;
      
    }

    if (new_cur < width)
      vector->downSize(new_cur);
  }

  if (literalEq(value_spec, AtomMin)) {
    value = makeTaggedSmallInt(getMin(d_variable));
  } else if (literalEq(value_spec, AtomMax)) {
    value = makeTaggedSmallInt(getMax(d_variable));
  } else {
    Assert(literalEq(value_spec, AtomMid));
    value = makeTaggedSmallInt(getMid(d_variable));
  } 
 
  return (OZ_unify(out_variable, variable) &&
	  OZ_unify(out_value,    value)) ? PROCEED : FAILED;
} OZ_C_proc_end


// ---------------------------------------------------------------------
//                  Scheduling Distribution 
// ---------------------------------------------------------------------

static inline double min(double a, double b) { return a < b ? a : b; }
static inline double max(double a, double b) { return a > b ? a : b; }

inline
static double pairCosts(int min_left, int max_left, int min_right, 
		     int max_right, int dur_left, int dur_right) {
  double slack_left_right = max_right - min_left - dur_left;
  double slack_right_left = max_left - min_right - dur_right;
  double s = min( slack_left_right,  slack_right_left) / 
    max( slack_left_right,  slack_right_left);
  double square_root = (s < 0) ? sqrt(-s) : sqrt(s); 
  return min(slack_left_right / square_root, slack_right_left / square_root);
}

OZ_C_proc_begin(BIfdDistributeMinPairs, 5) { 
  TaggedRef tagged_pair_vector  = deref(OZ_getCArg(0));
  TaggedRef tagged_start_record = deref(OZ_getCArg(1));
  TaggedRef tagged_dur_record   = deref(OZ_getCArg(2));
  TaggedRef out_pair            = OZ_getCArg(3); 
  TaggedRef out_rest            = OZ_getCArg(4); 
  TaggedRef left, right;

  SRecord *pair_vector = tagged2SRecord(tagged_pair_vector);
  SRecord *start_record = tagged2SRecord(tagged_start_record);
  SRecord *dur_record = tagged2SRecord(tagged_dur_record);
  int     width  = pair_vector->getWidth();
  int     cur    = 0;
  int     new_cur = 0;
  TaggedRef tagged_best_pair;
  SRecord *best_pair = NULL;
  double     best_costs = (double) OZ_getFDSup();
  TaggedRef tagged_pair;
  SRecord *pair;

  if (width == 0) {
    cout << "width of tuple must not be zero";
    return FAILED;
  }

  // Find the minimal pair
  do {
    tagged_pair = deref(pair_vector->getArg(cur));
    pair = tagged2SRecord(tagged_pair);

    left = deref(pair->getArg(0));
    right = deref(pair->getArg(1));

    TaggedRef left_var = deref(start_record->getFeature(left));
    TaggedRef right_var = deref(start_record->getFeature(right));
    
    int min_left = getMin(left_var);
    int max_left = getMax(left_var);
    int min_right = getMin(right_var);
    int max_right = getMax(right_var);
    int dur_left = smallIntValue(deref(dur_record->getFeature(left)));
    int dur_right = smallIntValue(deref(dur_record->getFeature(right)));

    // test if already entailed
    if ( (max_left + dur_left <= min_right) ||
	 (max_right + dur_right <= min_left) ||
	 (min_left + dur_left > max_right) ||
	 (min_right + dur_right > max_left) 
	 ) 
      continue;

    double costs = pairCosts(min_left, max_left, min_right, max_right, dur_left, dur_right);

    if (costs < best_costs) {
      if (best_pair != NULL) {
	pair_vector->setArg(new_cur, tagged_best_pair);
	tagged_best_pair = tagged_pair;
	best_pair = pair;
	best_costs = costs;
	new_cur++;
      }
      else { 
	tagged_best_pair = tagged_pair;
	best_pair = pair;
	best_costs = costs;
      }
    }
    else {
      pair_vector->setArg(new_cur, tagged_pair);
      new_cur++;
    }

  } while (++cur < width);


  if ( (new_cur > 0) && (new_cur < width))
    pair_vector->downSize(new_cur);

  if ( (best_pair == NULL) || (width == 1) || (new_cur == 0)) {
    return (OZ_unify(out_pair, makeTaggedSmallInt(0)) &&
	  OZ_unify(out_rest, tagged_pair_vector   )) ? PROCEED : FAILED; 
  }
  else {
    // build pair depending on order
    TaggedRef left = deref(best_pair->getArg(0));
    TaggedRef right = deref(best_pair->getArg(1));

    TaggedRef left_var = deref(start_record->getFeature(left));
    TaggedRef right_var = deref(start_record->getFeature(right));
    
    int min_left = getMin(left_var);
    int max_left = getMax(left_var);
    int min_right = getMin(right_var);
    int max_right = getMax(right_var);
    int dur_left = smallIntValue(deref(dur_record->getFeature(left)));
    int dur_right = smallIntValue(deref(dur_record->getFeature(right)));

    double slack_left_right = max_right - min_left - dur_left;
    double slack_right_left = max_left - min_right - dur_right;
    double s = min( slack_left_right,  slack_right_left) / 
      max( slack_left_right,  slack_right_left);
    double square_root = (s < 0) ? sqrt(-s) : sqrt(s); 
    if (slack_left_right / square_root >  slack_right_left / square_root) {
      return (OZ_unify(out_pair, tagged_best_pair) &&
	      OZ_unify(out_rest, tagged_pair_vector   )) ? PROCEED : FAILED;
    }
    else {
      SRecord *new_pair = SRecord::newSRecord(makeTaggedAtom("#"), 2);
      new_pair->setArg(0, right);
      new_pair->setArg(1, left);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_rest, tagged_pair_vector   )) ? PROCEED : FAILED;
    }


  }

} OZ_C_proc_end

