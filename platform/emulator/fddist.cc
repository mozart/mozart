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
    return dom.midElem();
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
static inline int intMin(int a, int b) { return a < b ? a : b; }
static inline int intMax(int a, int b) { return a > b ? a : b; }

inline
static int getMin1(TaggedRef var) {
  if (isGenFDVar(var)) 
    return tagged2GenFDVar(var)->getDom().minElem();
  else {
    if (isSmallInt(var))
      return OZ_intToC(var);
    else return 0;
  }
}

inline
static int getMax1(TaggedRef var) {
  if (isGenFDVar(var))
    return tagged2GenFDVar(var)->getDom().maxElem();
  else {
    if (isSmallInt(var))
      return OZ_intToC(var);
    else return 1;
  }
} 

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
    
    int min_left = getMin1(left_var);
    int max_left = getMax1(left_var);
    int min_right = getMin1(right_var);
    int max_right = getMax1(right_var);
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
	tagged_best_pair = tagged_pair;
	pair_vector->setArg(new_cur, tagged_best_pair);
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
    
    int min_left = getMin1(left_var);
    int max_left = getMax1(left_var);
    int min_right = getMin1(right_var);
    int max_right = getMax1(right_var);
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

// ---------------------------------------------------------------------
//                  Scheduling Distribution using Task Intervals
// ---------------------------------------------------------------------

#define MAXRESOURCES 20
#define MAXJOBS 20
#define PAR 4
#define MaxSetsSize10 10000

static int constraints[MaxSetsSize10];

inline
static int funcF(int slackS, int deltaS, int upper){
  int dd = slackS - deltaS;
  if (deltaS == 0) return upper;
  if (dd < 0) return 0;
  return (dd * dd) / slackS;
}

inline
static int funcSlackT(int min_a, int max_a) {
  return max_a - min_a;
}

inline
static int funcDeltaTA(int max_a, int max_b, int dur_a) {
  return intMax(0, max_a + dur_a - max_b);
}

inline 
static int funcDeltaTB(int min_a, int dur_a, int min_b) {
  return intMax(0, min_a + dur_a - min_b);
}

inline
static int funcG(int min_a, int max_a, int dur_a, 
		 int min_b, int max_b, int  dur_b, int upper) {
  return intMin( funcF( funcSlackT( min_a, max_a), 
			funcDeltaTA( max_a, max_b, dur_a),
			upper),
		 funcF( funcSlackT( min_b, max_b),
			funcDeltaTB( min_a, dur_a, min_b),
			upper));
}


OZ_C_proc_begin(BIfdDistributeTaskIntervals, 7) { 
  TaggedRef tagged_tasks_vector   = deref(OZ_getCArg(0));
  TaggedRef tagged_start_record   = deref(OZ_getCArg(1));
  TaggedRef tagged_dur_record     = deref(OZ_getCArg(2));
  TaggedRef tagged_ordered_record = deref(OZ_getCArg(3));
  TaggedRef out_pair              = OZ_getCArg(4); 
  TaggedRef out_resource          = OZ_getCArg(5); 
  TaggedRef out_constraints       = OZ_getCArg(6); 

  TaggedRef rel0  = makeTaggedAtom("v=<:");
  TaggedRef rel1  = makeTaggedAtom("v>=:");
  TaggedRef rel2  = makeTaggedAtom("c=<:");
  TaggedRef rel3  = makeTaggedAtom("c>=:");
  TaggedRef sharp = makeTaggedAtom("#");

  /*
  for (int i=0; i<4; i++) 
    taggedPrint(deref(OZ_getCArg(i)));
    */
  
  struct Set {
    int low, up, dur, extSize;
    int ext[MAXJOBS];
  };

  struct min_max_set {
    int min, max;
  };

  // to store FD variables
  struct min_max_set all_vars[MAXRESOURCES][MAXJOBS];
  // to store durations
  int all_durs[MAXRESOURCES][MAXJOBS];

  int upper;

  int i,j,k,l,left,right;

  // copy FDs and durations into the corresponding arrays 
  SRecord *tasks_vector = tagged2SRecord(tagged_tasks_vector);
  SRecord *start_record = tagged2SRecord(tagged_start_record);
  SRecord *dur_record = tagged2SRecord(tagged_dur_record);
  int number_of_resources = tasks_vector->getWidth();
  int number_of_jobs[MAXRESOURCES];

  TaggedRef all_tasks[MAXRESOURCES];

  // to store task intervals
  struct Set taskints[MAXRESOURCES][MAXJOBS][MAXJOBS];
  // to store orders
  int ordered[MAXRESOURCES][MAXJOBS][MAXJOBS];
  
  for (i=0; i < number_of_resources; i++) {
    TaggedRef tagged_tasks = deref(tasks_vector->getArg(i));
    all_tasks[i]           = tagged_tasks;
    SRecord *tasks         = tagged2SRecord(tagged_tasks);
    number_of_jobs[i]      = tasks->getWidth();
    for (j=0; j < number_of_jobs[i]; j++) {
      TaggedRef task1      = deref(tasks->getArg(j));
      TaggedRef fd_var     = deref(start_record->getFeature(task1));
      int current_dur      = OZ_intToC(deref(dur_record->getFeature(task1)));
      all_vars[i][j].min   = getMin1(fd_var);
      all_vars[i][j].max   = getMax1(fd_var);
      all_durs[i][j]       = current_dur;

    }
  }

  upper = getMin1(deref(start_record->getFeature(makeTaggedAtom("pe"))));

  // Initialize task intervals and ordering arrays
  SRecord *ordered_record = tagged2SRecord(tagged_ordered_record);
  for (i=0; i < number_of_resources; i++) {
    TaggedRef tagged_tasks  = deref(tasks_vector->getArg(i));
    SRecord *tasks          = tagged2SRecord(tagged_tasks);
    TaggedRef tagged_orders = deref(ordered_record->getArg(i));
    SRecord *orders         = tagged2SRecord(tagged_orders);
    for (left=0; left < number_of_jobs[i]; left++) 
      for (right=0; right < number_of_jobs[i]; right++) {
	TaggedRef task1 = deref(tasks->getArg(left));
	TaggedRef task2 = deref(tasks->getArg(right));
	
	// set up task interval
	struct Set *cset = &taskints[i][left][right];	
	cset->low        = all_vars[i][left].min;
	cset->up         = all_vars[i][right].max + all_durs[i][right];
	int cdur = 0;
	int csize = 0;
	if ( (cset->low <= all_vars[i][right].min)
	     && (all_vars[i][left].max + all_durs[i][left] <= cset->up) ) {
	  // otherwise the task interval is trivially empty
	  for (l=0; l < number_of_jobs[i]; l++) 
	    if ( (cset->low <= all_vars[i][l].min)
		 && (all_vars[i][l].max + all_durs[i][l] <= cset->up) ) {
	      cdur += all_durs[i][l];
	      cset->ext[csize++] = l;
	    }
	}
	cset->dur = cdur;
	cset->extSize = csize;
	

	if ( (csize > 0) && (cset->up - cset->low < cdur) ) {
//	  cout << "failure occurred\n";
	  return FAILED;
	}


	// set up ordering
	SRecord *row = tagged2SRecord(deref(orders->getFeature(task1)));
	TaggedRef entry = deref(row->getFeature(task2));
	if (isAnyVar(entry))
	  ordered[i][left][right] = 0;
	else 
	  ordered[i][left][right] = 1;
      }
  }


  int best_resource;
  struct Set best_set;
  int best_cost = OZ_getFDSup();
  int best_left;
  int best_right;
  int constraintsSize = 0;

  for (i=0; i < number_of_resources; i++) {
    struct Set *loc_best_set;
    int loc_best_nc;
    int loc_best_slack;
    int loc_best_left;
    int loc_best_right;
    int loc_best_slack_nc  = OZ_getFDSup();
    int loc_resource_slack = OZ_getFDSup();
    for (left=0; left < number_of_jobs[i]; left++) 
      for (right=0; right < number_of_jobs[i]; right++) {
	struct Set *cset = &taskints[i][left][right];	
	if (cset->extSize > 1) {
	  int count_firsts = 0;
	  int count_lasts = 0;
	  int firsts[MAXJOBS];
	  int lasts[MAXJOBS];
	  int cdur = cset->dur;
	  int up = cset->up;
	  int low = cset->low;
	  
	  loc_resource_slack = intMin(loc_resource_slack, up-low-cdur);

	  // compute firsts and lasts
	  for (l = 0; l < cset->extSize; l++) {
	    int task = cset->ext[l];
	    if ( (left != task)
		 && (all_vars[i][task].min <= up-cdur)
		 && (ordered[i][left][task] == 0) )
	      firsts[count_firsts++] = task;
	    if ( (right != task)
		 && (all_vars[i][task].max+all_durs[i][task] >= low + cdur)
		 && (ordered[i][task][right] == 0) )
	      lasts[count_lasts++] = task;
	  }
	  
	  // test whether firsts or lasts are empty
          if (count_firsts == 0) {
	    for (l=0; l < cset->extSize; l++) {
	      int task = cset->ext[l];
	      if ( (task != left) 
		   && (ordered[i][left][task] == 0) ) {
		// task >= left + d(left)
		constraints[constraintsSize] = 1;
		constraints[constraintsSize+1] = i;
		constraints[constraintsSize+2] = task;
		constraints[constraintsSize+3] = left;
		constraintsSize +=4;
	      }
	    }
	    int value = up - cdur;
	    if (all_vars[i][left].max > value) {
	      // left =< up - cdur
	      constraints[constraintsSize] = 2;
	      constraints[constraintsSize+1] = i;
	      constraints[constraintsSize+2] = left;
	      constraints[constraintsSize+3] = value;
	      constraintsSize +=4;
	    }
	  }
	  else {
	    if (count_lasts == 0) {
	      for (l=0; l<cset->extSize; l++) {
		int task = cset->ext[l];
		if ( (task != right) 
		     && (ordered[i][task][right] == 0) ) {
		  // task + d(task) =< right
		  constraints[constraintsSize] = 0;
		  constraints[constraintsSize+1] = i;
		  constraints[constraintsSize+2] = task;
		  constraints[constraintsSize+3] = right;
		  constraintsSize +=4;
		}
	      }
	      // right >= low + cdur - d(right)
  	      int value = low + cdur - all_durs[i][right];
	      if (all_vars[i][right].min < value) {
		constraints[constraintsSize] = 3;
		constraints[constraintsSize+1] = i;
		constraints[constraintsSize+2] = right;
		constraints[constraintsSize+3] = value;
		constraintsSize +=4;
	      }
	    }
	    else {
	      if ( (count_firsts > 0) && (count_lasts > 0) ) {
		int slack = up - low - cdur;
		int nc = intMin( count_firsts+1, count_lasts+1);
		if (slack*nc < loc_best_slack_nc) {
		  loc_best_set = &taskints[i][left][right];	
		  loc_best_nc = nc;
		  loc_best_slack = slack;
		  loc_best_slack_nc = slack*nc;
		  loc_best_left = left;
		  loc_best_right = right;
		}
	      }
	    }
	  }

	}
      } // of for-right loop 

    // have we found a best task interval?
    if (loc_best_slack_nc != OZ_getFDSup()) {
      int total_cost = loc_best_slack * loc_resource_slack * intMin( PAR, loc_best_nc);
      if (total_cost < best_cost) {
	best_resource    = i;
	best_left        = loc_best_left;
	best_right       = loc_best_right;
	best_cost        = total_cost;
	best_set.low     = loc_best_set->low;
	best_set.up      = loc_best_set->up;
	best_set.dur     = loc_best_set->dur;
	best_set.extSize = loc_best_set->extSize;
	for (l=0; l < best_set.extSize; l++)
	  best_set.ext[l] = loc_best_set->ext[l];
      }
    }

  }

  // all done?
  if (best_cost == OZ_getFDSup()) 
    return (OZ_unify(out_pair, makeTaggedSmallInt(0))) ? PROCEED : FAILED; 


  TaggedRef tagged_out_tuple;
  // compute tuple with constraint description
  if (constraintsSize == 0) {
    // do the dummy
    tagged_out_tuple = sharp;
  }
  else {
    int out_rec_size = constraintsSize / 4;
    SRecord *out_tuple;
    out_tuple = SRecord::newSRecord(sharp, out_rec_size);
    for (l=0; l < out_rec_size; l++) {
      int cur = l * 4;
      SRecord *new_tuple = SRecord::newSRecord(sharp, 4);
      TaggedRef left_side = deref(tagged2SRecord(all_tasks[constraints[cur+1]])->getArg(constraints[cur+2]));    
      new_tuple->setArg(0, left_side);
      new_tuple->setArg(3, makeTaggedSmallInt(constraints[cur+1]));
      switch (constraints[cur]) {
      case 0:
	new_tuple->setArg(1, rel0);      
	new_tuple->setArg(2, deref(tagged2SRecord(all_tasks[constraints[cur+1]])->getArg(constraints[cur+3])));    
	break;
      case 1:
	new_tuple->setArg(1, rel1);      
	new_tuple->setArg(2, deref(tagged2SRecord(all_tasks[constraints[cur+1]])->getArg(constraints[cur+3])));    
	break;
      case 2:
	new_tuple->setArg(1, rel2);      
	new_tuple->setArg(2, makeTaggedSmallInt(constraints[cur+3]));
	break;
      case 3:
	new_tuple->setArg(1, rel3);      
	new_tuple->setArg(2, makeTaggedSmallInt(constraints[cur+3]));
	break;
      }
      out_tuple->setArg(l, makeTaggedSRecord(new_tuple));
    }
    tagged_out_tuple = makeTaggedSRecord(out_tuple);
  }
       
  
  // compute firsts and lasts for the best task interval
  int count_firsts = 0;
  int count_lasts = 0;
  int firsts[MAXJOBS];
  int lasts[MAXJOBS];
  int cdur = best_set.dur;
  int up = best_set.up;
  int low = best_set.low;

  for (l=0; l<best_set.extSize; l++) {
    int task = best_set.ext[l];
    if ( (best_left != task)
	 && (all_vars[best_resource][task].min <= up-cdur)
	 && (ordered[best_resource][best_left][task] == 0) )
      firsts[count_firsts++] = task;
    if ( (best_right != task)
	 && (all_vars[best_resource][task].max + all_durs[best_resource][task] >= low + cdur)
	 && (ordered[best_resource][task][best_right] == 0) )
      lasts[count_lasts++] = task;
  }
  
  Assert( (count_firsts > 0) && (count_lasts > 0) );  

  if (count_firsts < count_lasts) {
    
    int deltaS = OZ_getFDSup();
    for (l=0; l < count_firsts; l++) 
      deltaS = intMin(deltaS, all_vars[best_resource][firsts[l]].min);
    deltaS -= low;

    int slackS = up - low - cdur;

    int bonus = funcF(slackS, deltaS, upper);

    int p1, p2;
    int g_costs  = OZ_getFDSup();
    int side, v;
    int min_left = low;
    int max_left = all_vars[best_resource][best_left].max;
    int dur_left = all_durs[best_resource][best_left];
    for (l=0; l<count_firsts; l++) {
      int firsts_l = firsts[l];
      int min_l = all_vars[best_resource][firsts_l].min;
      int max_l = all_vars[best_resource][firsts_l].max;
      int dur_l = all_durs[best_resource][firsts_l];
      int v1 = funcG(min_left, max_left, dur_left, min_l, max_l, dur_l, upper);
      int v2 = intMin( funcG(min_l, max_l, dur_l, min_left, max_left, dur_left, upper), bonus);
      if (v1 >= v2) {
	side = 1; v = v1;
      }
      else {
	side = 0; v = v2;
      }
      if (v < g_costs) {
	if (side == 1) {
	  p1 = best_left;
	  p2 = firsts_l;
	}
	else {
	  p1 = firsts_l;
	  p2 = best_left;
	}
      }
    }

    int min_p1 = all_vars[best_resource][p1].min;
    int max_p1 = all_vars[best_resource][p1].max;
    int dur_p1 = all_durs[best_resource][p1];
    int min_p2 = all_vars[best_resource][p2].min;
    int max_p2 = all_vars[best_resource][p2].max;
    int dur_p2 = all_durs[best_resource][p2];

    TaggedRef tagged_best_tasks = deref(tasks_vector->getArg(best_resource));
    SRecord *best_tasks         = tagged2SRecord(tagged_best_tasks);
    TaggedRef p1_task           = deref(best_tasks->getArg(p1));    
    TaggedRef p2_task           = deref(best_tasks->getArg(p2));    
    
    if ( funcG(min_p1, max_p1, dur_p1, min_p2, max_p2, dur_p2, upper) 
	 >= funcG(min_p2, max_p2, dur_p2, min_p1, max_p1, dur_p1, upper) ) {
      // return p1#p2
      SRecord *new_pair = SRecord::newSRecord(sharp, 2);
      new_pair->setArg(0, p1_task);
      new_pair->setArg(1, p2_task);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_resource, makeTaggedSmallInt(best_resource)) &&
	      OZ_unify(out_constraints, tagged_out_tuple)) 
	? PROCEED :FAILED;
    }
    else {
      // return p2#p1
      SRecord *new_pair = SRecord::newSRecord(sharp, 2);
      new_pair->setArg(0, p2_task);
      new_pair->setArg(1, p1_task);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_resource, makeTaggedSmallInt(best_resource))&&
	      OZ_unify(out_constraints, tagged_out_tuple)) 
	? PROCEED :FAILED;
    }
  }
  else {
    int deltaS = 0;
    for (l=0; l<count_lasts;  l++) 
      deltaS = intMax(deltaS, 
		      all_vars[best_resource][lasts[l]].max + all_durs[best_resource][lasts[l]]);
    deltaS = up - deltaS;

    int slackS = up - low - cdur;

    int bonus = funcF(slackS, deltaS, upper);

    int p1, p2;
    int g_costs  = OZ_getFDSup();
    int side, v;
    int min_right = all_vars[best_resource][best_right].min;
    int max_right = all_vars[best_resource][best_right].max;
    int dur_right = all_durs[best_resource][best_right];
    for (l=0; l < count_lasts; l++) {
      int lasts_l = lasts[l];
      int min_l = all_vars[best_resource][lasts_l].min;
      int max_l = all_vars[best_resource][lasts_l].max;
      int dur_l = all_durs[best_resource][lasts_l];
      int v1 = funcG(min_l, max_l, dur_l, min_right, max_right, dur_right, upper);
      int v2 = intMin( funcG(min_right, max_right, dur_right, min_l, max_l, dur_l, upper), bonus);
      if (v1 >= v2) {
	side = 1; v = v1;
      }
      else {
	side = 0; v = v2;
      }
      if (v < g_costs) {
	if (side == 1) {
	  p1 = lasts_l;
	  p2 = best_right;
	}
	else {
	  p1 = best_right;
	  p2 = lasts_l;
	}
      }
    }

    int min_p1 = all_vars[best_resource][p1].min;
    int max_p1 = all_vars[best_resource][p1].max;
    int dur_p1 = all_durs[best_resource][p1];
    int min_p2 = all_vars[best_resource][p2].min;
    int max_p2 = all_vars[best_resource][p2].max;
    int dur_p2 = all_durs[best_resource][p2];

    TaggedRef tagged_best_tasks = deref(tasks_vector->getArg(best_resource));
    SRecord *best_tasks         = tagged2SRecord(tagged_best_tasks);
    TaggedRef p1_task           = deref(best_tasks->getArg(p1));    
    TaggedRef p2_task           = deref(best_tasks->getArg(p2));    
    
    if ( funcG(min_p1, max_p1, dur_p1, min_p2, max_p2, dur_p2, upper) 
	 >= funcG(min_p2, max_p2, dur_p2, min_p1, max_p1, dur_p1, upper) ) {
      // return p1#p2
      SRecord *new_pair = SRecord::newSRecord(sharp, 2);
      new_pair->setArg(0, p1_task);
      new_pair->setArg(1, p2_task);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_resource, makeTaggedSmallInt(best_resource)) &&
	      OZ_unify(out_constraints, tagged_out_tuple)) 
	? PROCEED :FAILED;
    }
    else {
      // return p2#p1
      SRecord *new_pair = SRecord::newSRecord(sharp, 2);
      new_pair->setArg(0, p2_task);
      new_pair->setArg(1, p1_task);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_resource, makeTaggedSmallInt(best_resource)) &&
	      OZ_unify(out_constraints, tagged_out_tuple)) 
	? PROCEED :FAILED;
    }
  }

  
} OZ_C_proc_end

////////////////////////////////////////////////////////////
// for finding the optimal solution
////////////////////////////////////////////////////////////

inline
static int funcFOpt(int slackS, int deltaS, int upper){
  int dd = slackS - deltaS;
  if (deltaS == 0) return upper;
  if (dd < 0) return 0;
  return (deltaS * deltaS) / slackS;
}

inline
static int funcGOpt(int min_a, int max_a, int dur_a, 
		    int min_b, int max_b, int  dur_b, int upper) {
  return intMax( funcF( funcSlackT( min_a, max_a), 
			funcDeltaTA( max_a, max_b, dur_a),
			upper),
		 funcF( funcSlackT( min_b, max_b),
			funcDeltaTB( min_a, dur_a, min_b),
			upper));
}

// quick and dirty: just copied the code from above and changed some lines!
OZ_C_proc_begin(BIfdDistributeTaskIntervalsOpt, 7) { 
  TaggedRef tagged_tasks_vector   = deref(OZ_getCArg(0));
  TaggedRef tagged_start_record   = deref(OZ_getCArg(1));
  TaggedRef tagged_dur_record     = deref(OZ_getCArg(2));
  TaggedRef tagged_ordered_record = deref(OZ_getCArg(3));
  TaggedRef out_pair              = OZ_getCArg(4); 
  TaggedRef out_resource          = OZ_getCArg(5); 
  TaggedRef out_constraints       = OZ_getCArg(6); 

  TaggedRef rel0  = makeTaggedAtom("v=<:");
  TaggedRef rel1  = makeTaggedAtom("v>=:");
  TaggedRef rel2  = makeTaggedAtom("c=<:");
  TaggedRef rel3  = makeTaggedAtom("c>=:");
  TaggedRef sharp = makeTaggedAtom("#");


  struct Set {
    int low, up, dur, extSize;
    int ext[MAXJOBS];
  };

  struct min_max_set {
    int min, max;
  };

  // to store FD variables
  struct min_max_set all_vars[MAXRESOURCES][MAXJOBS];
  // to store durations
  int all_durs[MAXRESOURCES][MAXJOBS];

  int upper;

  int i,j,k,l,left,right;

  // copy FDs and durations into the corresponding arrays 
  SRecord *tasks_vector = tagged2SRecord(tagged_tasks_vector);
  SRecord *start_record = tagged2SRecord(tagged_start_record);
  SRecord *dur_record = tagged2SRecord(tagged_dur_record);
  int number_of_resources = tasks_vector->getWidth();
  int number_of_jobs[MAXRESOURCES];

  TaggedRef all_tasks[MAXRESOURCES];

  // to store task intervals
  struct Set taskints[MAXRESOURCES][MAXJOBS][MAXJOBS];
  // to store orders
  int ordered[MAXRESOURCES][MAXJOBS][MAXJOBS];
  
  for (i=0; i < number_of_resources; i++) {
    TaggedRef tagged_tasks = deref(tasks_vector->getArg(i));
    all_tasks[i]           = tagged_tasks;
    SRecord *tasks         = tagged2SRecord(tagged_tasks);
    number_of_jobs[i]      = tasks->getWidth();
    for (j=0; j < number_of_jobs[i]; j++) {
      TaggedRef task1      = deref(tasks->getArg(j));
      TaggedRef fd_var     = deref(start_record->getFeature(task1));
      int current_dur      = OZ_intToC(deref(dur_record->getFeature(task1)));
      all_vars[i][j].min   = getMin1(fd_var);
      all_vars[i][j].max   = getMax1(fd_var);
      all_durs[i][j]       = current_dur;

    }
  }

  upper = getMin1(deref(start_record->getFeature(makeTaggedAtom("pe"))));

  // Initialize task intervals and ordering arrays
  SRecord *ordered_record = tagged2SRecord(tagged_ordered_record);
  for (i=0; i < number_of_resources; i++) {
    TaggedRef tagged_tasks  = deref(tasks_vector->getArg(i));
    SRecord *tasks          = tagged2SRecord(tagged_tasks);
    TaggedRef tagged_orders = deref(ordered_record->getArg(i));
    SRecord *orders         = tagged2SRecord(tagged_orders);
    for (left=0; left < number_of_jobs[i]; left++) 
      for (right=0; right < number_of_jobs[i]; right++) {
	TaggedRef task1 = deref(tasks->getArg(left));
	TaggedRef task2 = deref(tasks->getArg(right));
	
	// set up task interval
	struct Set *cset = &taskints[i][left][right];	
	cset->low        = all_vars[i][left].min;
	cset->up         = all_vars[i][right].max + all_durs[i][right];
	int cdur = 0;
	int csize = 0;
	if ( (cset->low <= all_vars[i][right].min)
	     && (all_vars[i][left].max + all_durs[i][left] <= cset->up) ) {
	  // otherwise the task interval is trivially empty
	  for (l=0; l < number_of_jobs[i]; l++) 
	    if ( (cset->low <= all_vars[i][l].min)
		 && (all_vars[i][l].max + all_durs[i][l] <= cset->up) ) {
	      cdur += all_durs[i][l];
	      cset->ext[csize++] = l;
	    }
	}
	cset->dur = cdur;
	cset->extSize = csize;
	

	if ( (csize > 0) && (cset->up - cset->low < cdur) ) {
//	  cout << "failure occurred\n";
	  return FAILED;
	}


	// set up ordering
	SRecord *row = tagged2SRecord(deref(orders->getFeature(task1)));
	TaggedRef entry = deref(row->getFeature(task2));
	if (isAnyVar(entry))
	  ordered[i][left][right] = 0;
	else 
	  ordered[i][left][right] = 1;
      }
  }


  int best_resource;
  struct Set best_set;
  int best_cost = OZ_getFDSup();
  int best_left;
  int best_right;
  int constraintsSize = 0;

  for (i=0; i < number_of_resources; i++) {
    struct Set *loc_best_set;
    int loc_best_nc;
    int loc_best_slack;
    int loc_best_left;
    int loc_best_right;
    int loc_best_slack_nc  = OZ_getFDSup();
    int loc_resource_slack = OZ_getFDSup();
    for (left=0; left < number_of_jobs[i]; left++) 
      for (right=0; right < number_of_jobs[i]; right++) {
	struct Set *cset = &taskints[i][left][right];	
	if (cset->extSize > 1) {
	  int count_firsts = 0;
	  int count_lasts = 0;
	  int firsts[MAXJOBS];
	  int lasts[MAXJOBS];
	  int cdur = cset->dur;
	  int up = cset->up;
	  int low = cset->low;
	  
	  loc_resource_slack = intMin(loc_resource_slack, up-low-cdur);

	  // compute firsts and lasts
	  for (l = 0; l < cset->extSize; l++) {
	    int task = cset->ext[l];
	    if ( (left != task)
		 && (all_vars[i][task].min <= up-cdur)
		 && (ordered[i][left][task] == 0) )
	      firsts[count_firsts++] = task;
	    if ( (right != task)
		 && (all_vars[i][task].max+all_durs[i][task] >= low + cdur)
		 && (ordered[i][task][right] == 0) )
	      lasts[count_lasts++] = task;
	  }
	  
	  // test whether firsts or lasts are empty
          if (count_firsts == 0) {
	    for (l=0; l < cset->extSize; l++) {
	      int task = cset->ext[l];
	      if ( (task != left) 
		   && (ordered[i][left][task] == 0) ) {
		// task >= left + d(left)
		constraints[constraintsSize] = 1;
		constraints[constraintsSize+1] = i;
		constraints[constraintsSize+2] = task;
		constraints[constraintsSize+3] = left;
		constraintsSize +=4;
	      }
	    }
	    int value = up - cdur;
	    if (all_vars[i][left].max > value) {
	      // left =< up - cdur
	      constraints[constraintsSize] = 2;
	      constraints[constraintsSize+1] = i;
	      constraints[constraintsSize+2] = left;
	      constraints[constraintsSize+3] = value;
	      constraintsSize +=4;
	    }
	  }
	  else {
	    if (count_lasts == 0) {
	      for (l=0; l<cset->extSize; l++) {
		int task = cset->ext[l];
		if ( (task != right) 
		     && (ordered[i][task][right] == 0) ) {
		  // task + d(task) =< right
		  constraints[constraintsSize] = 0;
		  constraints[constraintsSize+1] = i;
		  constraints[constraintsSize+2] = task;
		  constraints[constraintsSize+3] = right;
		  constraintsSize +=4;
		}
	      }
	      // right >= low + cdur - d(right)
  	      int value = low + cdur - all_durs[i][right];
	      if (all_vars[i][right].min < value) {
		constraints[constraintsSize] = 3;
		constraints[constraintsSize+1] = i;
		constraints[constraintsSize+2] = right;
		constraints[constraintsSize+3] = value;
		constraintsSize +=4;
	      }
	    }
	    else {
	      if ( (count_firsts > 0) && (count_lasts > 0) ) {
		int slack = up - low - cdur;
		int nc = intMin( count_firsts+1, count_lasts+1);
		if (slack*nc < loc_best_slack_nc) {
		  loc_best_set = &taskints[i][left][right];	
		  loc_best_nc = nc;
		  loc_best_slack = slack;
		  loc_best_slack_nc = slack*nc;
		  loc_best_left = left;
		  loc_best_right = right;
		}
	      }
	    }
	  }

	}
      } // of for-right loop 

    // have we found a best task interval?
    if (loc_best_slack_nc != OZ_getFDSup()) {
      int total_cost = loc_best_slack * loc_resource_slack * intMin( PAR, loc_best_nc);
      if (total_cost < best_cost) {
	best_resource    = i;
	best_left        = loc_best_left;
	best_right       = loc_best_right;
	best_cost        = total_cost;
	best_set.low     = loc_best_set->low;
	best_set.up      = loc_best_set->up;
	best_set.dur     = loc_best_set->dur;
	best_set.extSize = loc_best_set->extSize;
	for (l=0; l < best_set.extSize; l++)
	  best_set.ext[l] = loc_best_set->ext[l];
      }
    }

  }

  // all done?
  if (best_cost == OZ_getFDSup()) 
    return (OZ_unify(out_pair, makeTaggedSmallInt(0))) ? PROCEED : FAILED; 


  TaggedRef tagged_out_tuple;
  // compute tuple with constraint description
  if (constraintsSize == 0) {
    // do the dummy
    tagged_out_tuple = sharp;
  }
  else {
    int out_rec_size = constraintsSize / 4;
    SRecord *out_tuple;
    out_tuple = SRecord::newSRecord(sharp, out_rec_size);
    for (l=0; l < out_rec_size; l++) {
      int cur = l * 4;
      SRecord *new_tuple = SRecord::newSRecord(sharp, 4);
      TaggedRef left_side = deref(tagged2SRecord(all_tasks[constraints[cur+1]])->getArg(constraints[cur+2]));    
      new_tuple->setArg(0, left_side);
      new_tuple->setArg(3, makeTaggedSmallInt(constraints[cur+1]));
      switch (constraints[cur]) {
      case 0:
	new_tuple->setArg(1, rel0);      
	new_tuple->setArg(2, deref(tagged2SRecord(all_tasks[constraints[cur+1]])->getArg(constraints[cur+3])));    
	break;
      case 1:
	new_tuple->setArg(1, rel1);      
	new_tuple->setArg(2, deref(tagged2SRecord(all_tasks[constraints[cur+1]])->getArg(constraints[cur+3])));    
	break;
      case 2:
	new_tuple->setArg(1, rel2);      
	new_tuple->setArg(2, makeTaggedSmallInt(constraints[cur+3]));
	break;
      case 3:
	new_tuple->setArg(1, rel3);      
	new_tuple->setArg(2, makeTaggedSmallInt(constraints[cur+3]));
	break;
      }
      out_tuple->setArg(l, makeTaggedSRecord(new_tuple));
    }
    tagged_out_tuple = makeTaggedSRecord(out_tuple);
  }
       
  
  // compute firsts and lasts for the best task interval
  int count_firsts = 0;
  int count_lasts = 0;
  int firsts[MAXJOBS];
  int lasts[MAXJOBS];
  int cdur = best_set.dur;
  int up = best_set.up;
  int low = best_set.low;

  for (l=0; l<best_set.extSize; l++) {
    int task = best_set.ext[l];
    if ( (best_left != task)
	 && (all_vars[best_resource][task].min <= up-cdur)
	 && (ordered[best_resource][best_left][task] == 0) )
      firsts[count_firsts++] = task;
    if ( (best_right != task)
	 && (all_vars[best_resource][task].max + all_durs[best_resource][task] >= low + cdur)
	 && (ordered[best_resource][task][best_right] == 0) )
      lasts[count_lasts++] = task;
  }
  
  Assert( (count_firsts > 0) && (count_lasts > 0) );  

  if (count_firsts < count_lasts) {
    
    int deltaS = OZ_getFDSup();
    for (l=0; l < count_firsts; l++) 
      deltaS = intMin(deltaS, all_vars[best_resource][firsts[l]].min);
    deltaS -= low;

    int slackS = up - low - cdur;

    int bonus = funcFOpt(slackS, deltaS, upper);

    int p1, p2;
    int g_costs  = OZ_getFDSup();
    int side, v;
    int min_left = low;
    int max_left = all_vars[best_resource][best_left].max;
    int dur_left = all_durs[best_resource][best_left];
    for (l=0; l<count_firsts; l++) {
      int firsts_l = firsts[l];
      int min_l = all_vars[best_resource][firsts_l].min;
      int max_l = all_vars[best_resource][firsts_l].max;
      int dur_l = all_durs[best_resource][firsts_l];
      int v1 = funcGOpt(min_left, max_left, dur_left, min_l, max_l, dur_l, upper);
      int v2 = intMax( funcGOpt(min_l, max_l, dur_l, min_left, max_left, dur_left, upper), bonus);
      if (v1 <= v2) {
	side = 1; v = v1;
      }
      else {
	side = 0; v = v2;
      }
      if (v < g_costs) {
	if (side == 1) {
	  p1 = best_left;
	  p2 = firsts_l;
	}
	else {
	  p1 = firsts_l;
	  p2 = best_left;
	}
      }
    }

    int min_p1 = all_vars[best_resource][p1].min;
    int max_p1 = all_vars[best_resource][p1].max;
    int dur_p1 = all_durs[best_resource][p1];
    int min_p2 = all_vars[best_resource][p2].min;
    int max_p2 = all_vars[best_resource][p2].max;
    int dur_p2 = all_durs[best_resource][p2];

    TaggedRef tagged_best_tasks = deref(tasks_vector->getArg(best_resource));
    SRecord *best_tasks         = tagged2SRecord(tagged_best_tasks);
    TaggedRef p1_task           = deref(best_tasks->getArg(p1));    
    TaggedRef p2_task           = deref(best_tasks->getArg(p2));    
    
    if ( funcGOpt(min_p1, max_p1, dur_p1, min_p2, max_p2, dur_p2, upper) 
	 >= funcGOpt(min_p2, max_p2, dur_p2, min_p1, max_p1, dur_p1, upper) ) {
      // return p1#p2
      SRecord *new_pair = SRecord::newSRecord(sharp, 2);
      new_pair->setArg(0, p1_task);
      new_pair->setArg(1, p2_task);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_resource, makeTaggedSmallInt(best_resource)) &&
	      OZ_unify(out_constraints, tagged_out_tuple)) 
	? PROCEED :FAILED;
    }
    else {
      // return p2#p1
      SRecord *new_pair = SRecord::newSRecord(sharp, 2);
      new_pair->setArg(0, p2_task);
      new_pair->setArg(1, p1_task);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_resource, makeTaggedSmallInt(best_resource))&&
	      OZ_unify(out_constraints, tagged_out_tuple)) 
	? PROCEED :FAILED;
    }
  }
  else {
    int deltaS = 0;
    for (l=0; l<count_lasts;  l++) 
      deltaS = intMax(deltaS, 
		      all_vars[best_resource][lasts[l]].max + all_durs[best_resource][lasts[l]]);
    deltaS = up - deltaS;

    int slackS = up - low - cdur;

    int bonus = funcFOpt(slackS, deltaS, upper);

    int p1, p2;
    int g_costs  = OZ_getFDSup();
    int side, v;
    int min_right = all_vars[best_resource][best_right].min;
    int max_right = all_vars[best_resource][best_right].max;
    int dur_right = all_durs[best_resource][best_right];
    for (l=0; l < count_lasts; l++) {
      int lasts_l = lasts[l];
      int min_l = all_vars[best_resource][lasts_l].min;
      int max_l = all_vars[best_resource][lasts_l].max;
      int dur_l = all_durs[best_resource][lasts_l];
      int v1 = funcGOpt(min_l, max_l, dur_l, min_right, max_right, dur_right, upper);
      int v2 = intMax( funcGOpt(min_right, max_right, dur_right, min_l, max_l, dur_l, upper), bonus);
      if (v1 <= v2) {
	side = 1; v = v1;
      }
      else {
	side = 0; v = v2;
      }
      if (v < g_costs) {
	if (side == 1) {
	  p1 = lasts_l;
	  p2 = best_right;
	}
	else {
	  p1 = best_right;
	  p2 = lasts_l;
	}
      }
    }

    int min_p1 = all_vars[best_resource][p1].min;
    int max_p1 = all_vars[best_resource][p1].max;
    int dur_p1 = all_durs[best_resource][p1];
    int min_p2 = all_vars[best_resource][p2].min;
    int max_p2 = all_vars[best_resource][p2].max;
    int dur_p2 = all_durs[best_resource][p2];

    TaggedRef tagged_best_tasks = deref(tasks_vector->getArg(best_resource));
    SRecord *best_tasks         = tagged2SRecord(tagged_best_tasks);
    TaggedRef p1_task           = deref(best_tasks->getArg(p1));    
    TaggedRef p2_task           = deref(best_tasks->getArg(p2));    
    
    if ( funcGOpt(min_p1, max_p1, dur_p1, min_p2, max_p2, dur_p2, upper) 
	 >= funcGOpt(min_p2, max_p2, dur_p2, min_p1, max_p1, dur_p1, upper) ) {
      // return p1#p2
      SRecord *new_pair = SRecord::newSRecord(sharp, 2);
      new_pair->setArg(0, p1_task);
      new_pair->setArg(1, p2_task);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_resource, makeTaggedSmallInt(best_resource)) &&
	      OZ_unify(out_constraints, tagged_out_tuple)) 
	? PROCEED :FAILED;
    }
    else {
      // return p2#p1
      SRecord *new_pair = SRecord::newSRecord(sharp, 2);
      new_pair->setArg(0, p2_task);
      new_pair->setArg(1, p1_task);
      return (OZ_unify(out_pair, makeTaggedSRecord(new_pair)) &&
	      OZ_unify(out_resource, makeTaggedSmallInt(best_resource)) &&
	      OZ_unify(out_constraints, tagged_out_tuple)) 
	? PROCEED :FAILED;
    }
  }

  
} OZ_C_proc_end
