/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

Cumulative with finite domains for dur and use
  ------------------------------------------------------------------------
*/

#define DUR
#define HOLES

#include "cumulative.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>

static inline int intMin(int a, int b) { return a < b ? a : b; }
static inline int intMax(int a, int b) { return a > b ? a : b; }

class Min_max {
public:
  int min, max, durMin, durMax, useMin, useMax;
};

struct Set {
  int cSi, dSi, mSi, sUp, sLow, extSize, val;
  int min,max;
  int * ext;
};

struct Set2 {
  int dSi, sUp, sLow, extSize;
  int * ext;
};

struct Interval {
  int left, right, use;
};

static int  ozcdecl CompareIntervals(const void *In1, const void *In2) 
{
  Interval *Int1 = (Interval *)In1;
  Interval *Int2 = (Interval *)In2;
  int left1 = Int1->left;
  int left2 = Int2->left;
  if (left1 < left2) return -1;
  else {
    if (left1 == left2) {
      return Int1->right - Int2->right;
    }
    else return 1;
  }
}

static int ozcdecl CompareBounds(const void *Int1, const void *Int2) {
  return *(int*)Int1 - *(int*)Int2;
}


Propagator_Cumulative::Propagator_Cumulative(OZ_Term tasks, OZ_Term starts, 
					     OZ_Term offset,
					     OZ_Term use, OZ_Term surfaces,
					     OZ_Term cap)
{

  reg_sz   = OZ_vectorSize(tasks);
  reg_offset = OZ_hallocOzTerms(reg_sz);
  reg_use    = OZ_hallocOzTerms(reg_sz);
  reg_surfaces = OZ_hallocCInts(reg_sz);
  reg_l      = OZ_hallocOzTerms(reg_sz);

  VectorIterator vi(tasks);
  int i = 0;
		     
  while (vi.anyLeft()) {
    OZ_Term task = vi.getNext();

    reg_l[i] = OZ_subtree(starts, task);
    reg_offset[i] = OZ_subtree(offset, task);
    reg_use[i] = OZ_subtree(use, task);
    reg_surfaces[i] = OZ_intToC(OZ_subtree(surfaces, task));
    i += 1;
  } // while

  OZ_ASSERT(i == reg_sz);

  reg_capacity = OZ_intToC(cap);
}

Propagator_Cumulative::~Propagator_Cumulative(void) 
{
  OZ_hfreeCInts(reg_surfaces, reg_sz);
  OZ_hfreeOzTerms(reg_offset, reg_sz);
  OZ_hfreeOzTerms(reg_use, reg_sz);
  OZ_hfreeOzTerms(reg_l, reg_sz);

}

void Propagator_Cumulative::updateHeapRefs(OZ_Boolean)
{
  int * new_reg_surfaces = OZ_hallocCInts(reg_sz);
  OZ_Term * new_reg_offset = OZ_hallocOzTerms(reg_sz);
  OZ_Term * new_reg_use    = OZ_hallocOzTerms(reg_sz);
  OZ_Term * new_reg_l  = OZ_hallocOzTerms(reg_sz);

  for (int i = reg_sz; i--; ) {
    new_reg_surfaces[i] = reg_surfaces[i];
    new_reg_offset[i] = reg_offset[i];
    new_reg_use[i] = reg_use[i];
    new_reg_l[i] = reg_l[i];
    OZ_updateHeapTerm(new_reg_l[i]);
    OZ_updateHeapTerm(new_reg_use[i]);
    OZ_updateHeapTerm(new_reg_offset[i]);
  }
  reg_surfaces = new_reg_surfaces;
  reg_offset = new_reg_offset;
  reg_use = new_reg_use;
  reg_l = new_reg_l;

}

OZ_Term Propagator_Cumulative::getParameters(void) const
{
  TERMVECTOR2LIST(reg_l, reg_sz, l);
  TERMVECTOR2LIST(reg_offset, reg_sz, offset);
  TERMVECTOR2LIST(reg_use, reg_sz, use);
  RETURN_LIST4(l, offset, use, OZ_int(reg_capacity));
}

OZ_C_proc_begin(sched_cumulative_FD, 6)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_VECT OZ_EM_LIT "," OZ_EM_VECT OZ_EM_FD 
		   "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_FD 
		   "," OZ_EM_VECT OZ_EM_INT 
		   "," OZ_EM_VECT OZ_EM_INT);
  
  {
    PropagatorExpect pe;
    
    OZ_EXPECT(pe, 0, expectVectorVectorLiteral);
    OZ_EXPECT(pe, 1, expectProperRecordIntVarMinMax);
    OZ_EXPECT(pe, 2, expectProperRecordIntVarMinMax);
    OZ_EXPECT(pe, 3, expectProperRecordIntVarMinMax);
    OZ_EXPECT(pe, 4, expectProperRecordInt);
    OZ_EXPECT(pe, 5, expectVectorInt);
    SAMELENGTH_VECTORS(1, 2);
    SAMELENGTH_VECTORS(1, 3);
    SAMELENGTH_VECTORS(1, 4);
  }

  OZ_Term starts = OZ_args[1], durs = OZ_args[2], use = OZ_args[3], 
    surfaces = OZ_args[4], caps = OZ_args[5];


  VectorIterator vi(OZ_args[0]);
  VectorIterator viCap(caps);

  for (int i = 0; i < OZ_vectorSize(OZ_args[0]); i++) {
    OZ_Term tasks    = vi.getNext();
    OZ_Term capacity = viCap.getNext();

    PropagatorExpect pe;

    VectorIterator vi_tasks(tasks);
    while (vi_tasks.anyLeft()) {
      OZ_Term task = vi_tasks.getNext();
      OZ_Term start_task = OZ_subtree(starts, task);
      OZ_Term dur_task = OZ_subtree(durs, task);
      OZ_Term use_task = OZ_subtree(use, task);
      OZ_Term surface_task = OZ_subtree(surfaces, task);
      if (!start_task || !dur_task || !use_task || !surface_task) 
	return OZ_typeError(expectedType, 0, "Scheduling applications expect "
			    "that all task symbols are features of the records denoting the start times, the durations and the resource usages.");
      pe.expectIntVarMinMax(OZ_subtree(starts, task));
      pe.expectIntVarMinMax(OZ_subtree(durs, task));
      pe.expectIntVarMinMax(OZ_subtree(use, task));
    }

    OZ_Return r = pe.impose(new Propagator_Cumulative(tasks, starts, durs, 
						      use, surfaces, 
						      capacity),
			    OZ_getLowPrio());

    if (r == FAILED) return FAILED;
  }
  return OZ_ENTAILED;
  
}
OZ_C_proc_end




OZ_Return Propagator_Cumulative::propagate(void)
{

  //////////
  // Cumulative constraint: lean version of edge-finding 
  // and some interval stuff
  //////////
  int &ts      = reg_sz;
  int * surface = reg_surfaces;
  int capacity = reg_capacity;


  DECL_DYN_ARRAY(OZ_FDIntVar, x, ts);
  DECL_DYN_ARRAY(OZ_FDIntVar, dur, ts);
  DECL_DYN_ARRAY(OZ_FDIntVar, use, ts);


  int i, j;
  for (i = ts; i--; ) {
    x[i].read(reg_l[i]);
    dur[i].read(reg_offset[i]);
    use[i].read(reg_use[i]);
  }

  int upFlag = 0;
  int downFlag = 0;
  int disjFlag = 0;
  int cap_flag = 0;
  OZ_Boolean vars_left = OZ_FALSE;

  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
  }

  int mysup = OZ_getFDSup();
  
  DECL_DYN_ARRAY(Set2, Sets, ts);
  
  for (i = ts; i--; )
    Sets[i].ext = (int *) OZ_FDIntVar::operator new(sizeof(int) * ts);

  // memory is automatically disposed when propagator is left

  struct TISet {
    int low, up, size, min, max, empty;
    // extension for tasks inside task intervals
  };

  struct TISet ** taskints;
  taskints = ((TISet **)::new TISet*[ts]);
  for (i=0; i<ts; i++) {
    taskints[i] = ::new TISet[ts];
  }
  
  int left,right,tiFlag;

#ifdef DUR
  /* propagate durations and usage */
  for (i=0; i<ts; i++) {  
    FailOnEmpty(*use[i] <= capacity);
    struct Min_max *info = &MinMax[i];	
    int surfI = surface[i];
    if (surfI > 0) {
      int span = info->max + dur[i]->getMaxElem() - info->min;
      if (use[i]->getMinElem() * span < surface[i]) 
	FailOnEmpty(*use[i] >= (int) ceil((double) surface[i] / (double) span));
      FailOnEmpty(*dur[i] >= (int) ceil((double) surfI / (double) use[i]->getMaxElem()));
      FailOnEmpty(*dur[i] <= (int) floor((double) surfI / (double) use[i]->getMinElem()));
      FailOnEmpty(*use[i] >= (int) ceil((double) surfI / (double) dur[i]->getMaxElem()));
      FailOnEmpty(*use[i] <= (int) floor((double) surfI / (double) dur[i]->getMinElem()));
    }
  }
#endif

  for (i=ts; i--;){
    MinMax[i].durMin = dur[i]->getMinElem();
    MinMax[i].durMax = dur[i]->getMaxElem();
    MinMax[i].useMin = use[i]->getMinElem();
    MinMax[i].useMax = use[i]->getMaxElem();
  }

tiloop:

  /////////
  // Initialize task intervals, cubic complexity
  ////////
  for (left = 0; left < ts; left++) 
    for (right = 0; right < ts; right++) {
      struct TISet *cset = &taskints[left][right];
      cset->low        = MinMax[left].min;
      cset->up         = MinMax[right].max + MinMax[right].durMax;
      
      cset->max        = MinMax[left].max + MinMax[left].durMax;
      cset->min        = MinMax[right].min;

      int cdur = 0;
      int csize = 0;
      int empty = 1;
      int clst = 0;
      if ( (cset->low <= MinMax[right].min)
	   && (MinMax[left].max + MinMax[left].durMax <= cset->up)
	   ) 
	{
	  // otherwise the task interval is trivially empty
          for (int l=0; l < ts; l++) {
	    int dueL     = MinMax[l].max + MinMax[l].durMax;
	    int releaseL = MinMax[l].min;
	    if ( (cset->low <= releaseL)
		 && (dueL <= cset->up) ) {
	      empty = 0;
	      csize += surface[l];

	      if ( (l!=left) && (l!=right) ) {
		cset->max = intMax(cset->max, dueL);
		cset->min = intMin(cset->min, releaseL);
	      }

	    }
	  }
	}
      cset->size  = csize;
      cset->empty = empty;

      if ( (empty == 0) && ( (cset->up - cset->low) * capacity < csize) ) {
	goto failure;
      }

    }

//goto capLoop;
  //////////  
  // Do the edge-finding
  //////////  

  for (left = 0; left < ts; left++) 
    for (right = 0; right < ts; right++) {
      struct TISet *cset = &taskints[left][right];
      if (cset->empty == 0) {
	int releaseTI = cset->low;
	int dueTI     = cset->up;
	int sizeTI    = cset->size;
	for (i = 0; i < ts; i++) {
	  int contained = 0;
	  int maxI     = MinMax[i].max;
	  int releaseI = MinMax[i].min;
	  int dueI     = maxI + MinMax[i].durMax;
	  int sizeAll, tsizeTI, treleaseTI, tdueTI;
	  if ( (releaseI >= releaseTI) && (dueI <= dueTI) ) {
	    // I is in TI
            contained = 1;
 	    sizeAll = sizeTI;
	    tsizeTI = sizeTI - surface[i]; 
	    if (i==left) {
	      treleaseTI = cset->min;
	      tdueTI = dueTI;
	    }
	    else {
	      if (i==right) {
		tdueTI = cset->max;
		treleaseTI = releaseTI;
	      }
	      else {
		treleaseTI = releaseTI;
		tdueTI = dueTI;
	      }
	    }
	  }
	  else {
	    // I is not in TI
	    treleaseTI = releaseTI;
	    tdueTI     = dueTI;
	    tsizeTI     = sizeTI;
	    sizeAll     = surface[i] + sizeTI;
	  }

	  if (contained == 0) {
	    //  Caseau on cumulative scheduling, ICLP96
	    int slack = (tdueTI - treleaseTI)*capacity - tsizeTI;
	    if ( (surface[i] - intMax(0,treleaseTI-releaseI)*MinMax[i].useMax > slack)
		 && (MinMax[i].useMin*(tdueTI - treleaseTI) > slack)
		 && (releaseI < tdueTI - (int) floor( (double) slack / (double) MinMax[i].useMin))) {
	      FailOnEmpty(*x[i] >= tdueTI - (int) floor( (double) slack / (double) MinMax[i].useMin));
	      tiFlag = 1;
	    }
	    if ( (surface[i] - intMax(0,dueI-tdueTI)*MinMax[i].useMax > slack)
		 && (MinMax[i].useMin*(tdueTI - treleaseTI) > slack)
		 && (MinMax[i].max+MinMax[i].durMin > treleaseTI + (int) ceil( (double) slack / (double) MinMax[i].useMin) )) {
	      FailOnEmpty(*x[i] <= treleaseTI + (int) ceil( (double) slack / (double) MinMax[i].useMin) - MinMax[i].durMin);
	      FailOnEmpty(*dur[i] <= treleaseTI + (int) ceil( (double) slack / (double) MinMax[i].useMin) - MinMax[i].min);
	      tiFlag = 1;
	    }
	  }

	  if ( ((tdueTI - treleaseTI) * capacity < sizeAll) &&
	       ((tdueTI - releaseI) * capacity < sizeAll) ) {
	    int delta = tsizeTI - (tdueTI - treleaseTI) * (capacity - MinMax[i].useMin);
	    if (delta > 0) {
	      // l must be last
	      int val = treleaseTI + (int) ceil((double) delta / (double) MinMax[i].useMax);
	      if (releaseI < val) {
		FailOnEmpty(*x[i] >= val);
	      }
	    }
	  }

	  if ( ((tdueTI - treleaseTI) * capacity < sizeAll) &&
	       ((dueI - treleaseTI) * capacity < sizeAll) ) {
	    int delta = tsizeTI - (tdueTI - treleaseTI) * (capacity - MinMax[i].useMin);
	    if (delta > 0) {
	      // l must be first
	      int val = tdueTI - (int) ceil((double) delta / (double) MinMax[i].useMax);
	      if (dueI > val) {
		FailOnEmpty(*x[i] <= val - MinMax[i].durMin);
		FailOnEmpty(*dur[i] <= val - MinMax[i].min);
	      }
	    }
	  }
	}
      }
    }

	  
#ifdef DUR
  /* propagate durations and usage */
  for (i=0; i<ts; i++) {  
    struct Min_max *info = &MinMax[i];	
    int surfI = surface[i];
    if (surfI > 0) {
      FailOnEmpty(*dur[i] >= (int) ceil((double) surfI / (double) use[i]->getMaxElem()));
      FailOnEmpty(*dur[i] <= (int) floor((double) surfI / (double) use[i]->getMinElem()));
      FailOnEmpty(*use[i] >= (int) ceil((double) surfI / (double) dur[i]->getMaxElem()));
      FailOnEmpty(*use[i] <= (int) floor((double) surfI / (double) dur[i]->getMinElem()));
      int span = info->max + dur[i]->getMaxElem() - info->min;
      if (use[i]->getMinElem() * span < surface[i]) 
	FailOnEmpty(*use[i] >= (int) ceil((double) surface[i] / (double) span));
    }
  }
#endif

  for (i=0; i<ts; i++) {
    int mini = x[i]->getMinElem();	
    int maxi = x[i]->getMaxElem();	
    if (mini > MinMax[i].min) {
      MinMax[i].min = mini; tiFlag=1;
    }
    if (maxi < MinMax[i].max) {
      MinMax[i].max = maxi; tiFlag=1;
    }

    mini = dur[i]->getMinElem();	
    maxi = dur[i]->getMaxElem();	
    if (mini > MinMax[i].durMin) {
      MinMax[i].durMin = mini; tiFlag=1;
    }
    if (maxi < MinMax[i].durMax) {
      MinMax[i].durMax = maxi; tiFlag=1;
    }
    mini = use[i]->getMinElem();	
    maxi = use[i]->getMaxElem();	
    if (mini > MinMax[i].useMin) {
      MinMax[i].useMin = mini; tiFlag=1;
    }
    if (maxi < MinMax[i].useMax) {
      MinMax[i].useMax = maxi; tiFlag=1;
    }

  }


  if (tiFlag == 1) {
    tiFlag = 0;
    goto tiloop;
  }


//goto finish;
capLoop:

  //////////
  // do the capacity checking
  //////////
  {
    int interval_nb = 0;	  
    
    DECL_DYN_ARRAY(Interval, Intervals, ts);

    //////////
    // compute intervals, where the task occupies place in any case
    //////////
    for (i=ts; i--;){
      // latest start time < earliest completion?
      int lst = MinMax[i].max;
      int ect = MinMax[i].min + MinMax[i].durMin;
      if (lst < ect) {
	Intervals[interval_nb].left = lst;
	Intervals[interval_nb].right = ect;
	Intervals[interval_nb].use = MinMax[i].useMin;
	interval_nb++;
      }
    }

    //////////
    // compute left and right bound for cumulative checking
    //////////
    int min_left = mysup;
    int max_right = 0;
    int sum = 0;
    for (i=0; i<ts; i++) {
      int iMin = MinMax[i].min;
      int iDue = MinMax[i].max + MinMax[i].durMax;
      sum = sum + surface[i];
      if (iMin < min_left) min_left = iMin;
      if (iDue > max_right) max_right = iDue;
    }
    // test whether the capacity is sufficient for all tasks
    if (sum > capacity * (max_right - min_left))
      goto failure;

    Intervals[interval_nb].left = min_left;
    Intervals[interval_nb].right = max_right;
    Intervals[interval_nb].use = 0;
    interval_nb++;

    

    //////////
    // sort the intervals lexicographically
    //////////
    Interval * intervals = Intervals;
    qsort(intervals, interval_nb, sizeof(Interval), CompareIntervals);

    //////////
    // compute the set of all bounds of intervals
    //////////
    int double_nb = interval_nb*2;
    DECL_DYN_ARRAY(int, IntervalBounds, double_nb);

    for (i=0; i<interval_nb; i++) {
      IntervalBounds[2*i] = Intervals[i].left;
      IntervalBounds[2*i+1] = Intervals[i].right;
    }
    
    //////////
    // sort the bounds in ascending order
    //////////
    int * intervalBounds = IntervalBounds;
    qsort(intervalBounds, double_nb, sizeof(int), CompareBounds);


    //////////
    // compute the set of intervals, for which there is exclusion
    //////////
    DECL_DYN_ARRAY(Interval, ExclusionIntervals, double_nb);
    int exclusion_nb = 0;
    int low_counter  = 0;
    int left_pt      = 0;
    int right_pt     = 0;


    while (left_pt < double_nb) {
      int left_val = IntervalBounds[left_pt];
      while ((right_pt < double_nb) && (IntervalBounds[right_pt] == left_val))
	right_pt++;
      if (right_pt == double_nb)  break;
      int right_val = IntervalBounds[right_pt];
      int cum = 0;

      for (i=low_counter; i<interval_nb; i++) {

	int leftInt = Intervals[i].left;
	int rightInt = Intervals[i].right;
	if (leftInt > right_val) break;
	if ( (left_val >= leftInt) && (right_val <= rightInt) ) {
	  cum = cum + (right_val - left_val) * Intervals[i].use;
	}
      }


      if (cum > (right_val - left_val) * capacity) {
	goto failure;
      }
      if (cum > 0) {
	ExclusionIntervals[exclusion_nb].left = left_val;
	ExclusionIntervals[exclusion_nb].right = right_val;
	ExclusionIntervals[exclusion_nb].use = cum;
	exclusion_nb++;
      }
      left_pt = right_pt;
    }

    int last = 0;

    //////////
    // exclude from the tasks the intervals, which indicate that 
    // the task connot be scheduled here because of no sufficient place.
    //////////
    // do not use reg_flag anymore, it is not worth it.
    // the commented region does contain code which avoids the 
    // production of holes in domains
    // perhaps some tests before generalizing domains could improve

#ifdef HOLES
    for (i=0; i<ts; i++) {
      int lst = MinMax[i].max;
      int ect = MinMax[i].min +  MinMax[i].durMin;
      for (j=0; j<exclusion_nb; j++) {
	Interval Exclusion = ExclusionIntervals[j];
	int span = Exclusion.right - Exclusion.left;
	if (Exclusion.use + span * MinMax[i].useMin > span * capacity) {
	  int left = Exclusion.left;
	  int right = Exclusion.right;
	  if (lst < ect) {
	    if ( (lst <= left) && (right <= ect) ) continue;
	    else {
	      if (Exclusion.use + span * MinMax[i].useMin > span * capacity) {
		OZ_FiniteDomain la;
		// The following holds because of construction of 
		// the exclusion intervals! They describe fulled columns!
		la.initRange(left-MinMax[i].durMin+1,right-1);
		FailOnEmpty(*x[i] -= la);
	      }
	    }
	  }
	  else {
	    if (Exclusion.use + span * MinMax[i].useMin > span * capacity) {
	      OZ_FiniteDomain la;
	      la.initRange(left-MinMax[i].durMin+1,right-1);
	      FailOnEmpty(*x[i] -= la);
	    }
	  }
	}
      }
    }
#else
    /* for bounds reasoning more efficiently */
    for (i=0; i<ts; i++) { 
      int est = MinMax[i].min;
      int ect = est + MinMax[i].durMin;
      int lst = MinMax[i].max;
      int lct = lst + MinMax[i].durMax;
      // from left to right
      for (j=0; j<exclusion_nb; j++) {
	Interval Exclusion = ExclusionIntervals[j];
	int low = Exclusion.left;
	int up  = Exclusion.right;
	if (low > est) 
	  break;
	else {
	  if ( (est >= low) && (est < up) ) {
	    int span = up - low;
	    if ( ((lst < ect) && ((lst > low) || (up > ect))) ||
		 (lst >= ect) )
	      if (Exclusion.use + span * MinMax[i].useMin > span * capacity) {	    
		FailOnEmpty(*x[i] >= up);
	      }
	  }
	}
      }
      // from right to left
      for (j=exclusion_nb-1; j>=0; j--) {
	Interval Exclusion = ExclusionIntervals[j];
	int low = Exclusion.left;
	int up  = Exclusion.right;
	if (up < lct)
	  break;
	else {
	  if ( (lct > low) && (lct <= up) ) {
	    int span = up - low;
	    if ( ((lst < ect) && ((lst > low) || (up > ect))) ||
		 (lst >= ect) )
	      if (Exclusion.use + span * MinMax[i].useMin > span * capacity) {	    
		FailOnEmpty(*x[i] <= low - MinMax[i].durMin);
		FailOnEmpty(*dur[i] <= low - MinMax[i].min);
	      }
	  }
	}
      }
    }
#endif


    //////////
    // update the min/max values
    //////////
    /* propagate durations and usage */

   for (i=0; i<ts; i++) {  
     struct Min_max *info = &MinMax[i];	
     int surfI = surface[i];
     if (surfI > 0) {
       FailOnEmpty(*dur[i] >= (int) ceil((double) surfI / (double) use[i]->getMaxElem()));
       FailOnEmpty(*dur[i] <= (int) floor((double) surfI / (double) use[i]->getMinElem()));
       FailOnEmpty(*use[i] >= (int) ceil((double) surfI / (double) dur[i]->getMaxElem()));
       FailOnEmpty(*use[i] <= (int) floor((double) surfI / (double) dur[i]->getMinElem()));
       int span = info->max + dur[i]->getMaxElem() - info->min;
       if (use[i]->getMinElem() * span < surface[i]) 
	 FailOnEmpty(*use[i] >= (int) ceil((double) surface[i] / (double) span));
     }
   }
    for (i=0; i<ts; i++) {
      int mini = x[i]->getMinElem();	
      int maxi = x[i]->getMaxElem();	
      if (mini > MinMax[i].min) {
	MinMax[i].min = mini; cap_flag=1;
      }
      if (maxi < MinMax[i].max) {
	MinMax[i].max = maxi; cap_flag=1;
      }
      mini = dur[i]->getMinElem();	
      maxi = dur[i]->getMaxElem();	
      if (mini > MinMax[i].durMin) {
	MinMax[i].durMin = mini; cap_flag=1;
      }
      if (maxi < MinMax[i].durMax) {
	MinMax[i].durMax = maxi; cap_flag=1;
      }
      mini = use[i]->getMinElem();	
      maxi = use[i]->getMaxElem();	
      if (mini > MinMax[i].useMin) {
	MinMax[i].useMin = mini; cap_flag=1;
      }
      if (maxi < MinMax[i].useMax) {
	MinMax[i].useMax = maxi; cap_flag=1;
      }
    }
    if (cap_flag == 1) {
      cap_flag = 0;
      goto capLoop;
    }

  }


finish:  

  for (i=0; i<ts; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;

  for (i = ts; i--; i>=0) {
    vars_left = vars_left | x[i].leave();
    vars_left = vars_left | dur[i].leave();
    vars_left = vars_left | use[i].leave();
  }
  return vars_left ? SLEEP : PROCEED;

failure:
  for (i=0; i<ts; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;

  for (i=0; i<ts; i++) {
    x[i].fail();
    dur[i].fail();
    use[i].fail();
  }
  return FAILED;   

}

//-----------------------------------------------------------------------------
// static member

OZ_CFun Propagator_Cumulative::spawner = sched_cumulative_FD;
