/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "taskintervals.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>

//-----------------------------------------------------------------------------

//#define ADDITIONALPRUNING



static inline int intMin(int a, int b) { return a < b ? a : b; }
static inline int intMax(int a, int b) { return a > b ? a : b; }

// for cpIterate
 struct StartDurTerms {
  OZ_Term start;
  int dur;
};

// for cpIterateCap
struct StartDurUseTerms {
  OZ_Term start;
  int dur;
  int use;
};

static int compareDurs(const void * a, const void * b) {
  return ((StartDurTerms *) b)->dur - ((StartDurTerms *) a)->dur;
}

static int compareDursUse(const void * a, const void * b) {
  return ((StartDurUseTerms *) b)->dur * ((StartDurUseTerms *) b)->use
    - ((StartDurUseTerms *) a)->dur * ((StartDurUseTerms *) a)->use;
}

static OZ_FDIntVar * xx;
static int * dd;



TaskIntervalsPropagator::TaskIntervalsPropagator(OZ_Term tasks, 
						 OZ_Term starts, 
						 OZ_Term durs)
  : Propagator_VD_VI(OZ_vectorSize(tasks))
{
  VectorIterator vi(tasks);
  int i = 0;
		     
  while (vi.anyLeft()) {
    OZ_Term task = vi.getNext();

    reg_l[i]      = OZ_subtree(starts, task);
    reg_offset[i] = OZ_intToC(OZ_subtree(durs, task));
    i += 1;
  } // while
    OZ_ASSERT(i == reg_sz);

}

OZ_C_proc_begin(sched_taskIntervals, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_VECT OZ_EM_LIT "," 
		   OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT);
  
  {
    PropagatorExpect pe;
    
    OZ_EXPECT(pe, 0, expectVectorVectorLiteral);
    OZ_EXPECT(pe, 1, expectProperRecordIntVarMinMax);
    OZ_EXPECT(pe, 2, expectProperRecordInt);
    SAMELENGTH_VECTORS(1, 2);
  }
  
  OZ_Term starts = OZ_args[1], durs = OZ_args[2];

  VectorIterator vi(OZ_args[0]);

  for (int i = OZ_vectorSize(OZ_args[0]); i--; ) {
    OZ_Term tasks = vi.getNext();

    PropagatorExpect pe;

    VectorIterator vi_tasks(tasks);
    while (vi_tasks.anyLeft()) {
      OZ_Term task = vi_tasks.getNext();
      OZ_Term start_task = OZ_subtree(starts, task);
      OZ_Term dur_task = OZ_subtree(durs, task);
      if (!start_task || !dur_task) 
	return OZ_typeError(expectedType, 0, "Scheduling applications expect that all task symbols are features of the records denoting the start times and durations.");
      pe.expectIntVarMinMax(OZ_subtree(starts, task));
    }

    OZ_Return r = pe.impose(new TaskIntervalsPropagator(tasks, starts, durs),
			    OZ_getLowPrio());

    if (r == FAILED) return FAILED;
  }
  return OZ_ENTAILED;
}
OZ_C_proc_end


class Min_max {
public:
  int min, max;
};

OZ_Return TaskIntervalsPropagator::propagate(void)
{
  int &ts  = reg_sz;
  int * dur = reg_offset;

  struct Set {
    int low, up, dur, extSize;
    int * ext;
    // extension for tasks inside task intervals
    int max, min;
  };


  DECL_DYN_ARRAY(OZ_FDIntVar, x, ts);
  PropagatorController_VV P(ts, x);


  int left, right, i, j, k, l;
  int fd_sup = OZ_getFDSup();
  int loopFlag = 0;
  int disjFlag = 0;

  // task interval data structures

  struct Set ** taskints;
  taskints = ((Set **)::new Set*[ts]);
  for (i=0; i<ts; i++) {
    taskints[i] = ::new Set[ts];
  }

  for (i=0; i<ts; i++)
    for (j=0; j<ts; j++)
      taskints[i][j].ext = ::new int[ts];


  // read fd variables
  for (i = ts; i--; )
    x[i].read(reg_l[i]);

  // initialize min/max values
  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
  }


tiloop:



  /////////
  // Initialize task intervals, cubic complexity
  ////////
  for (left = 0; left < ts; left++) 
    for (right = 0; right < ts; right++) {
      struct Set *cset = &taskints[left][right];
      cset->low        = MinMax[left].min;
      cset->up         = MinMax[right].max + dur[right];
      
      cset->max        = MinMax[left].max + dur[left];
      cset->min        = MinMax[right].min;

      int cdur = 0;
      int csize = 0;
      if ( (cset->low <= MinMax[right].min)
	   && (MinMax[left].max + dur[left] <= cset->up)
	   ) 
	{
	  // otherwise the task interval is trivially empty
          for (l=0; l < ts; l++) {
	    int durL     = dur[l];
	    int dueL     = MinMax[l].max + durL;
	    int releaseL = MinMax[l].min;
	    if ( (cset->low <= releaseL)
		 && (dueL <= cset->up) ) {
	      cdur += durL;
	      cset->ext[csize++] = l;

	      if ( (l!=left) && (l!=right) ) {
		cset->max = intMax(cset->max, dueL);
		cset->min = intMin(cset->min, releaseL);
	      }

	    }
	  }
	}
      cset->dur     = cdur;
      cset->extSize = csize;
      
      if ( (csize > 0) && (cset->up - cset->low < cdur) ) {
	goto failure;
      }
    }


  //////////  
  // Do the edge-finding
  //////////  

  for (left = 0; left < ts; left++) 
    for (right = 0; right < ts; right++) {
      struct Set *cset = &taskints[left][right];
      int setSize = cset->extSize;
      if (setSize > 1) {
	int releaseTI = cset->low;
	int dueTI     = cset->up;
	int durTI     = cset->dur;
	for (i = 0; i < ts; i++) {
	  int maxI     = MinMax[i].max;
	  int durI     = dur[i];
	  int releaseI = MinMax[i].min;
	  int dueI     = maxI + durI;
	  int durAll, tdurTI, treleaseTI, tdueTI;
	  if ( (releaseI >= releaseTI) && (dueI <= dueTI) ) {
	    // I is in TI
 	    durAll = durTI;
	    tdurTI = durTI - durI; 
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
	    tdurTI     = durTI;
	    durAll     = durI + durTI;
	  }
	  
	  int tmax = 0;
	  int tmin = fd_sup;
	  for (k=0; k<setSize; k++) {
	    int element = cset->ext[k];
	    if (element!=i) {
	      tmax = intMax(tmax, MinMax[element].max);
	      tmin = intMin(tmin, MinMax[element].min + dur[element]);
	    }
	  }
	  if ( (tdueTI - treleaseTI >= durAll) &&
	       (MinMax[i].min + durI <= tmax) &&   // I before the last
	       (tmin <= maxI) )                    // I after the first
	    {
	    // I may be inside
            if (tdueTI - releaseI < durAll) {
	      // I cannot be first but may be inside
              // ask the oracle
	      if (releaseI < treleaseTI + dur[left]) {
		// otherwise it would be useless
                // after_first
		int mini = fd_sup;
		for (j=0; j<setSize; j++) {
		  int element = cset->ext[j];
		  if (element != i)
		    mini = intMin(mini, MinMax[element].min+dur[element]);
		}
		if (releaseI < mini) {
		  loopFlag = 1;
		  FailOnEmpty( *x[i] >= mini);
		}
	      }
	    }
	    if (dueI - treleaseTI < durAll) {
	      // I cannot be last but may be inside
              // ask the oracle
	      if (dueI > tdueTI - dur[right]) {
		// otherwise it would be useless
                // before_last
		int maxi = 0;
		for (j=0; j<setSize; j++) {
		  int element = cset->ext[j];
		  if (element != i)
		    maxi = intMax(maxi, MinMax[element].max);
		}
		if (dueI > maxi) {
		  loopFlag = 1;
		  FailOnEmpty( *x[i] <= maxi-durI);
		}
	      }
	    }
	  }
	  else {
	    if (tdueTI - releaseI < durAll) {
	      // I cannot be first and not inside --> I must be last
	      if (releaseI < treleaseTI + tdurTI) {
		loopFlag = 1;
		FailOnEmpty( *x[i] >= treleaseTI + tdurTI);
	      }
	      // all others in TI must be finished before I starts
	      for (j = 0; j < setSize; j++) {
		int element = cset->ext[j];
		int right = maxI - dur[element];
		if ( (i!=element) && (MinMax[element].max > right) ) {
		  loopFlag = 1;
		  FailOnEmpty( *x[element] <= right);
		  /*
		    // is not useful
		  OZ_Term left_side_task = reg_l[element];
		  OZ_Term right_side_task = reg_l[i];
		  addImpose(fd_prop_bounds, left_side_task);
		  addImpose(fd_prop_bounds, right_side_task);
		  
		  impose(new LessEqOffPropagator(left_side_task, right_side_task,
		  -dur[element]));
		  */
		}
	      }
	    }
	    if (dueI - treleaseTI < durAll) {
	      // I cannot be last and not inside --> I must be first
	      if (maxI > tdueTI - tdurTI - durI) {
		loopFlag = 1;
		FailOnEmpty( *x[i] <= tdueTI - tdurTI - durI);
	      }
	      // all others in TI must start after I has finished
	      for (j = 0; j < setSize; j++) {
		int element = cset->ext[j];
		int right = releaseI + durI;
		if ( (i!=element) && (MinMax[element].min < right) ) {
		  loopFlag = 1;
		  FailOnEmpty( *x[element] >= right);
		  /*
		    // is not useful
		  OZ_Term left_side_task = reg_l[i];
		  OZ_Term right_side_task = reg_l[element];
		  addImpose(fd_prop_bounds, left_side_task);
		  addImpose(fd_prop_bounds, right_side_task);
		  
		  impose(new LessEqOffPropagator(left_side_task, right_side_task,
		  -dur[i]));
		  */
		}
	      }
	    }
	  }
	}
      }
    }
	  


  for (i=0; i<ts; i++) {
    MinMax[i].min = x[i]->getMinElem();	
    MinMax[i].max = x[i]->getMaxElem();	
  }


  if (loopFlag == 1) {
    loopFlag = 0;
    goto tiloop;
  }

reifiedloop:

   //////////
   // do the reification in a loop
   //////////
   for (i=0; i<ts; i++)
     for (j=i+1; j<ts; j++) {
       int xui = MinMax[i].max, di = dur[i], xlj = MinMax[j].min;
       if (xui + di <= xlj) continue;
       int xuj = MinMax[j].max, dj = dur[j], xli = MinMax[i].min;
       if (xuj + dj <= xli) continue;
       if (xli + di > xuj) {
	 if (xuj > xui - dj) {
	   disjFlag = 1;
	   FailOnEmpty(*x[j] <= xui - dj);
	   MinMax[j].max = x[j]->getMaxElem();
	 }
	 if (xli < xlj + dj) {
	   disjFlag = 1;
	   FailOnEmpty(*x[i] >= xlj + dj);
	   MinMax[i].min = x[i]->getMinElem();
	 }
       }
       if (xlj + dj > xui) {
	 if (xui > xuj - di) {
	   disjFlag = 1;
	   FailOnEmpty(*x[i] <= xuj - di);
	   MinMax[i].max = x[i]->getMaxElem();
	 }
	 if (xlj < xli + di) {
	   disjFlag = 1;
	   FailOnEmpty(*x[j] >= xli + di);
	   MinMax[j].min = x[j]->getMinElem();
	 }
       }
     }

  if (disjFlag == 1) {
    disjFlag = 0;
    goto reifiedloop;
  }


  for (i=0; i<ts; i++) 
    for (j=0; j<ts; j++) 
      :: delete [] taskints[i][j].ext;

  for (i=0; i<ts; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;

  return P.leave();

failure:
  for (i=0; i<ts; i++) 
    for (j=0; j<ts; j++) 
      :: delete [] taskints[i][j].ext;

  for (i=0; i<ts; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;

  return P.fail();
}

//-----------------------------------------------------------------------------

struct Interval {
  int left, right, use;
};

static int ozcdecl CompareIntervals(const void *In1, const void *In2) 
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

CPIteratePropagatorCumTI::CPIteratePropagatorCumTI(OZ_Term tasks, 
						   OZ_Term starts, 
						   OZ_Term durs, 
						   OZ_Term use,
						   OZ_Term cap)
  : Propagator_VD_VI_VI_I(OZ_vectorSize(tasks))
{

  reg_capacity = OZ_intToC(cap);

  VectorIterator vi(tasks);
  int i = 0;
		     
  DECL_DYN_ARRAY(StartDurUseTerms, sdu, reg_sz);

  while (vi.anyLeft()) {
    OZ_Term task = vi.getNext();

    sdu[i].start = OZ_subtree(starts, task);
    sdu[i].dur = OZ_intToC(OZ_subtree(durs, task));
    sdu[i].use = OZ_intToC(OZ_subtree(use, task));
    i += 1;
  } // while

  OZ_ASSERT(i == reg_sz);

  qsort(sdu, reg_sz, sizeof(StartDurUseTerms), compareDursUse);

  for (i = reg_sz; i--; ) {
    reg_l[i]      = sdu[i].start;
    reg_offset[i] = sdu[i].dur;
    reg_use[i]    = sdu[i].use;
  }
}

OZ_C_proc_begin(sched_cumulativeTI, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_VECT OZ_EM_LIT "," OZ_EM_VECT OZ_EM_FD 
		   "," OZ_EM_VECT OZ_EM_INT "," OZ_EM_VECT OZ_EM_INT 
		   "," OZ_EM_VECT OZ_EM_INT);
  
  {
    PropagatorExpect pe;
    
    OZ_EXPECT(pe, 0, expectVectorVectorLiteral);
    OZ_EXPECT(pe, 1, expectProperRecordIntVarMinMax);
    OZ_EXPECT(pe, 2, expectProperRecordInt);
    OZ_EXPECT(pe, 3, expectProperRecordInt);
    OZ_EXPECT(pe, 4, expectVectorInt);
    SAMELENGTH_VECTORS(1, 2);
    SAMELENGTH_VECTORS(1, 3);
  }

  OZ_Term starts = OZ_args[1], durs = OZ_args[2], use = OZ_args[3], 
    caps = OZ_args[4];


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
      if (!start_task || !dur_task || !use_task) 
	return OZ_typeError(expectedType, 0, "Scheduling applications expect that all task symbols are features of the records denoting the start times, the durations and the resource usages.");
      pe.expectIntVarMinMax(OZ_subtree(starts, task));
    }

    OZ_Return r = pe.impose(new CPIteratePropagatorCumTI(tasks, starts, durs, 
						       use, capacity),
			    OZ_getLowPrio());

    if (r == FAILED) return FAILED;
  }
  return OZ_ENTAILED;
  
}
OZ_C_proc_end


struct Set2 {
  int dSi, sUp, sLow, extSize;
  int * ext;
};

OZ_Return CPIteratePropagatorCumTI::propagate(void)
{

  //////////
  // Cumulative constraint: lean version of edge-finding 
  // and some interval stuff
  //////////
  int &ts      = reg_sz;
  int * dur    = reg_offset;
  int * use    = reg_use;
  int capacity = reg_capacity;

  DECL_DYN_ARRAY(OZ_FDIntVar, x, ts);

  PropagatorController_VV P(ts, x);
  
  int i, j;
  for (i = ts; i--; )
    x[i].read(reg_l[i]);

  xx = x;
  dd = reg_offset;

  
  int upFlag = 0;
  int downFlag = 0;
  int disjFlag = 0;
  int cap_flag = 0;

  int kUp;
  int kDown;
  int dur0;
  int mSi;

  /*
  DECL_DYN_ARRAY(int, set0, ts);
  DECL_DYN_ARRAY(int, compSet0, ts);
  DECL_DYN_ARRAY(int, forCompSet0Up, ts);
  DECL_DYN_ARRAY(int, forCompSet0Down, ts);
  DECL_DYN_ARRAY(int, outSide, ts);
  */

  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
    /*
    forCompSet0Up[i] = i;
    forCompSet0Down[i] = i;
    */
  }

  int set0Size;
  int compSet0Size;
  int outSideSize;
  int mysup = OZ_getFDSup();
  
  DECL_DYN_ARRAY(Set2, Sets, ts);
  
  for (i = ts; i--; )
    Sets[i].ext = (int *) OZ_FDIntVar::operator new(sizeof(int) * ts);


  struct TISet {
    int low, up, dur, size, min, max, empty, ect, lst, overlap;
    // extension for tasks inside task intervals
  };

  struct TISet ** taskints;
  taskints = ((TISet **)::new TISet*[ts]);
  for (i=0; i<ts; i++) {
    taskints[i] = ::new TISet[ts];
  }
  
  int left,right,tiFlag;


  // memory is automatically disposed when propagator is left

  //////////  
  // do the reified stuff for task pairs.
  //////////  
  for (i=0; i<ts; i++)
    for (j=i+1; j<ts; j++) {
      if (use[i] + use[j] > capacity) {
	int xui = MinMax[i].max, di = dur[i], xlj = MinMax[j].min;
	if (xui + di <= xlj) continue;
	int xuj = MinMax[j].max, dj = dur[j], xli = MinMax[i].min;
	if (xuj + dj <= xli) continue;
	if (xli + di > xuj) {
	  int val1 = xui - dj;
	  if (xuj > val1) {
	    FailOnEmpty(*x[j] <= val1);
	    MinMax[j].max = x[j]->getMaxElem();
	  }
	  int val2 = xlj + dj;
	  if (xli < val2) {
	    FailOnEmpty(*x[i] >= val2);
	    MinMax[i].min = x[i]->getMinElem();
	  }
	}
	if (xlj + dj > xui) {
	  int val1 = xuj - di;
	  if (xui > val1) {
	    FailOnEmpty(*x[i] <= val1);
	    MinMax[i].max = x[i]->getMaxElem();
	  }
	  int val2 = xli + di;
	  if (xlj < val2) {
	    FailOnEmpty(*x[j] >= val2);
	    MinMax[j].min = x[j]->getMinElem();
	  }
	}
      }
    }


tiloop:



  /////////
  // Initialize task intervals, cubic complexity
  ////////
  for (left = 0; left < ts; left++) 
    for (right = 0; right < ts; right++) {
      struct TISet *cset = &taskints[left][right];
      cset->low        = MinMax[left].min;
      cset->up         = MinMax[right].max + dur[right];
      
      cset->max        = MinMax[left].max + dur[left];
      cset->min        = MinMax[right].min;

      int cdur = 0;
      int csize = 0;
      int empty = 1;
      int clst = 0;
      int cect = OZ_getFDSup();
      int overlap=0;
      if ( (cset->low <= MinMax[right].min)
	   && (MinMax[left].max + dur[left] <= cset->up)
	   ) 
	{
	  // otherwise the task interval is trivially empty
          for (int l=0; l < ts; l++) {
	    int durL     = dur[l];
	    int dueL     = MinMax[l].max + durL;
	    int releaseL = MinMax[l].min;
	    if ( (cset->low <= releaseL)
		 && (dueL <= cset->up) ) {
	      empty = 0;
	      cdur += durL;
	      csize += use[l]*durL;
	      clst = intMax(clst, dueL-durL);
	      cect = intMin(cect, releaseL+durL);

	      if ( (l!=left) && (l!=right) ) {
		cset->max = intMax(cset->max, dueL);
		cset->min = intMin(cset->min, releaseL);
	      }

	    }
	    else {
	      // add the overlapping amount of tasks
	      int overlapTmp = intMin(intMax(0,releaseL+durL-cset->low),
				      intMin(intMax(0,cset->up-dueL+durL),
					     intMin(durL,cset->up-cset->low)));
	      overlap += overlapTmp*use[l];
	    }
	  }
	}
      cset->dur   = cdur;
      cset->size  = csize;
      cset->empty = empty;
      cset->ect   = cect;
      cset->lst   = clst;
      cset->overlap = overlap;

      if ( (empty == 0) && ( (cset->up - cset->low) * capacity < csize+overlap) ) {
	goto failure;
      }

    }


  //////////  
  // Do the edge-finding
  //////////  

  for (left = 0; left < ts; left++) 
    for (right = 0; right < ts; right++) {
      struct TISet *cset = &taskints[left][right];
      if (cset->empty == 0) {
	int releaseTI = cset->low;
	int dueTI     = cset->up;
	int durTI     = cset->dur;
	int sizeTI    = cset->size;
	for (i = 0; i < ts; i++) {
	  int maxI     = MinMax[i].max;
	  int durI     = dur[i];
	  int useI     = use[i];
	  int releaseI = MinMax[i].min;
	  int dueI     = maxI + durI;
	  int sizeAll, tsizeTI, treleaseTI, tdueTI;
	  int contained = 0;
	  if ( (releaseI >= releaseTI) && (dueI <= dueTI) ) {
	    // I is in TI
	    contained = 1;
 	    sizeAll = sizeTI;
	    tsizeTI = sizeTI - durI*useI; 
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
	    sizeAll     = durI*useI + sizeTI;
	  }

	  int overlapI = intMin(intMax(0,releaseI+durI-treleaseTI),
				intMin(intMax(0,tdueTI-dueI+durI),
				       intMin(durI,tdueTI-treleaseTI)))*use[i];
	  int OS = sizeAll;
	  int OT = tsizeTI;

#ifdef ADDITIONALPRUNING
	  if (contained == 0) {
	    sizeAll = sizeAll + cset->overlap - overlapI;
	    tsizeTI = tsizeTI + cset->overlap - overlapI;

	    // due to Mats email
	    if (sizeAll > capacity*(tdueTI - releaseI)) {
	      int delta = sizeAll - capacity*(tdueTI - releaseI);
	      DECL_DYN_ARRAY(int, s1, ts);
	      DECL_DYN_ARRAY(int, s2, ts);
	      int s1_count=0, s2_count=0;
	      for (j=0;j<ts;j++) {
		if (use[j]+useI > capacity) 
		  s2[s2_count++] = j;
		else s1[s1_count++] = j;
	      }
	      for (j=0;j<s1_count;j++) {
		delta -= use[s1[j]]*dur[s1[j]];
	      }
	      if (delta > 0) {
		for (j=0;j<s2_count;j++) {
		  if (MinMax[s2[j]].min + dur[s2[j]] <= releaseI)
		    delta -= use[s2[j]]*dur[s2[j]];
		}
		if (delta > 0 )
		  printf("habuakakdasd\n");
	      }
	    }
	    sizeAll = OS;
	    tsizeTI = OT;
	  }
	  
	    //due to nuijten p.62 -- 65
	  if ( (releaseI < releaseTI) || (dueI > dueTI) ) {
	    if ( (treleaseTI < releaseI) && (releaseI < cset->ect) &&
		 (tsizeTI + (intMin(releaseI+durI,tdueTI) -treleaseTI)*use[i] > (tdueTI - treleaseTI) * capacity) ) {
	      if (MinMax[i].min < cset->ect){
		FailOnEmpty(*x[i] >= cset->ect);
		tiFlag = 1;
	      }
	    }
	    if ( (cset->lst < dueI) && (dueI < tdueTI) &&
		 (tsizeTI + (dueTI - intMax(dueI-durI,treleaseTI) )*use[i] > (tdueTI - treleaseTI) * capacity) ) {
	      if (MinMax[i].max > cset->lst - durI) {
		FailOnEmpty(*x[i] <= cset->lst - durI);
		tiFlag = 1;
	      }
	    }
	    if ( (releaseI < treleaseTI) && (treleaseTI < releaseI+durI) &&
		 (tsizeTI + (releaseI+durI- treleaseTI)*use[i] > (tdueTI - treleaseTI) * capacity) ) {
	      int delta = tsizeTI - (tdueTI - treleaseTI) * (capacity - useI);
	      if (delta > 0) {
		int val = treleaseTI + (int) ceil((double) delta / (double) useI);
		for (int m = 0; m<ts; m++) {
		  if ((m != i) && (MinMax[m].min >= releaseTI) && 
		      (MinMax[m].max+dur[m] <= dueTI) ) {
		    if (use[m]+use[i] > capacity) {
		    FailOnEmpty(*x[m] <= MinMax[i].max - dur[m]);
		    FailOnEmpty(*x[i] >= MinMax[m].min + dur[m]);
		    }
		  }
		}
		if (releaseI < val) {
		  tiFlag = 1;
		  FailOnEmpty(*x[i] >= val);
		}
	      }
	    }
	    if  ( (dueI-durI < tdueTI) && (tdueTI < dueI) &&
		  (tsizeTI + (tdueTI- (dueI-durI))*use[i] > (tdueTI - treleaseTI) * capacity) ) {
	      int delta = tsizeTI - (tdueTI - treleaseTI) * (capacity - useI);
	      if (delta > 0) {
		int val = tdueTI + (int) floor((double) delta / (double) useI);
		if (dueI > val) {
		  tiFlag = 1;
		  FailOnEmpty(*x[i] <= val - durI);
		}
	      }
	    }
	  }


	  if (contained == 0) {
	    sizeAll = sizeAll + cset->overlap - overlapI;
	    tsizeTI = tsizeTI + cset->overlap - overlapI;

	    //  Caseau on cumulative scheduling, ICLP96
	    int slack = (tdueTI - treleaseTI)*capacity - tsizeTI;
	    if ( (useI*durI - intMax(0,treleaseTI-releaseI)*useI > slack)
		 && (useI*(tdueTI - treleaseTI) > slack)
		 && (releaseI < tdueTI - (int) floor( (double) slack / (double) useI))) {
	      FailOnEmpty(*x[i] >= tdueTI - (int) floor( (double) slack / (double) useI));
	      tiFlag = 1;
	    }
	    if ( (useI*durI - intMax(0,dueI-tdueTI)*useI > slack)
		 && (useI*(tdueTI - treleaseTI) > slack)
		 && (dueI > treleaseTI + (int) ceil( (double) slack / (double) useI) )) {
	      FailOnEmpty(*x[i] <= treleaseTI + (int) ceil( (double) slack / (double) useI) - durI);
	      tiFlag = 1;
	    }
	    sizeAll = OS;
	    tsizeTI = OT;
	  }
#endif

	  // if task i is contained, this does not work!

	  if (contained == 0) {
	    sizeAll = sizeAll + cset->overlap - overlapI;
	    tsizeTI = tsizeTI + cset->overlap - overlapI;
	  }

	  if ( ((tdueTI - treleaseTI) * capacity < sizeAll) &&
	       ((dueI - treleaseTI) * capacity < sizeAll) ) {
	    int delta = tsizeTI - (tdueTI - treleaseTI) * (capacity - useI);
	    if (delta > 0) {
	      // l must be first
	      int val = tdueTI - (int) ceil((double) delta / (double) useI);
	      if (dueI > val) {
		tiFlag = 1;
		FailOnEmpty(*x[i] <= val - durI);
	      }
	    }
	  }


	  sizeAll = OS;
	  tsizeTI = OT;
	  
	  // if task i is contained, this does not work!
	  if (contained == 0) {
	    sizeAll = sizeAll + cset->overlap - overlapI;
	    tsizeTI = tsizeTI + cset->overlap - overlapI;
	  }

	  if ( ((tdueTI - treleaseTI) * capacity < sizeAll) &&
	       ((tdueTI - releaseI) * capacity < sizeAll) ) {
	    int delta = tsizeTI - (tdueTI - treleaseTI) * (capacity - useI);
	    if (delta > 0) {
	      // l must be last
	      int val = treleaseTI + (int) ceil((double) delta / (double) useI);
	      if (releaseI < val) {
		tiFlag = 1;
		FailOnEmpty(*x[i] >= val);
	      }
	    }
	  }


	}
      }
    }
	  

  for (i=0; i<ts; i++) {
    MinMax[i].min = x[i]->getMinElem();	
    MinMax[i].max = x[i]->getMaxElem();	
  }


  if (tiFlag == 1) {
    tiFlag = 0;
    goto tiloop;
  }






reifiedloop:

   for (i=0; i<ts; i++)
     for (j=i+1; j<ts; j++) {
       if (use[i] + use[j] > capacity) {
	 int xui = MinMax[i].max, di = dur[i], xlj = MinMax[j].min;
	 if (xui + di <= xlj) continue;
	 int xuj = MinMax[j].max, dj = dur[j], xli = MinMax[i].min;
	 if (xuj + dj <= xli) continue;
	 if (xli + di > xuj) {
	   if (xuj > xui - dj) {
	     disjFlag = 1;
	     FailOnEmpty(*x[j] <= xui - dj);
	     MinMax[j].max = x[j]->getMaxElem();
	   }
	   if (xli < xlj + dj) {
	     disjFlag = 1;
	     FailOnEmpty(*x[i] >= xlj + dj);
	     MinMax[i].min = x[i]->getMinElem();
	   }
	 }
	 if (xlj + dj > xui) {
	   if (xui > xuj - di) {
	     disjFlag = 1;
	     FailOnEmpty(*x[i] <= xuj - di);
	     MinMax[i].max = x[i]->getMaxElem();
	   }
	   if (xlj < xli + di) {
	     disjFlag = 1;
	     FailOnEmpty(*x[j] >= xli + di);
	     MinMax[j].min = x[j]->getMinElem();
	   }
	 }
       }
     }

  if (disjFlag == 1) {
    disjFlag = 0;
    goto reifiedloop;
  }


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
      int ect = MinMax[i].min + dur[i];
      if (lst < ect) {
	Intervals[interval_nb].left = lst;
	Intervals[interval_nb].right = ect;
	Intervals[interval_nb].use = use[i];
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
      int iDue = MinMax[i].max + dur[i];
      sum = sum + use[i] * dur[i];
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
    qsort(intervals, interval_nb, sizeof(Interval), CompareIntervals)
;
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

      /*
      // counting the overlap is not worth the effort for cumulative reasoning
      int cumOver = 0;
      //this destroys the cumulative reasoning
      for (i = 0; i < ts; i++) {
	if (MinMax[i].max >= MinMax[i].min + dur[i]) {
	  int overlap = intMin(intMax(0,MinMax[i].min+dur[i]-left_val),
			       intMin(intMax(0,right_val-MinMax[i].max),
				      intMin(dur[i],right_val-left_val)));
	  cumOver = cumOver + overlap * use[i];
	}
      }
      */

      
      if (cum > (right_val - left_val) * capacity) {
        goto failure;
      }
      /*
      // it's not worth the effort
      if (cum+cumOver > (right_val - left_val) * capacity) {
        goto failure;
      }
      */

      if (cum > 0) {
	ExclusionIntervals[exclusion_nb].left = left_val;
	ExclusionIntervals[exclusion_nb].right = right_val;
	ExclusionIntervals[exclusion_nb].use = cum;
	exclusion_nb++;
      }
      left_pt = right_pt;
    }

    int last = 0;
    /*
    int last_left = 0;
    int last_right = 0;
    */

    //////////
    // exclude from the tasks the intervals, which indicate that 
    // the task connot be scheduled here because of no sufficient place.
    //////////
    // do not use reg_flag anymore, it is not worth it.
    // the commented region does contain code which avoids the 
    // production of holes in domains

    // perhaps some tests before generalizing domains could improve
    for (i=0; i<ts; i++) {
      int lst = MinMax[i].max;
      int ect = MinMax[i].min + dur[i];
      int use_i = use[i];
      int dur_i = dur[i];
      for (j=0; j<exclusion_nb; j++) {
	Interval Exclusion = ExclusionIntervals[j];
	int span = Exclusion.right - Exclusion.left;
	if (Exclusion.use + span * use_i > span * capacity) {
	  int left = Exclusion.left;
	  int right = Exclusion.right;
	  if (lst < ect) {
	    if ( (lst <= left) && (right <= ect) ) continue;
	    else {
	      if (Exclusion.use + span * use_i > span * capacity) {
		OZ_FiniteDomain la;
		// The following holds because of construction of 
		// the exclusion intervals! They describe fulled columns!
		la.initRange(left-dur_i+1,right-1);
		FailOnEmpty(*x[i] -= la);
		
		// new
		// for capacity > 1 we must count the used resource. 
		// But this is too expensive. really???
		if ((left - last < dur_i) && (capacity == 1)) {
		  OZ_FiniteDomain la;
		  la.initRange(last,right-1);
		  FailOnEmpty(*x[i] -= la);
		}
		last = right;
	      }
	    }
	  }
	  else {
	    if (Exclusion.use + span * use_i > span * capacity) {
	      OZ_FiniteDomain la;
	      la.initRange(left-dur_i+1,right-1);
	      FailOnEmpty(*x[i] -= la);
	      // new
	      // for capacity > 1 we must count the used resource. 
	      // But this is too expensive.
	      if ((left - last < dur_i) && (capacity == 1)) {
		OZ_FiniteDomain la;
		la.initRange(last,right-1);
		FailOnEmpty(*x[i] -= la);
	      }
	      last = right;
	      }
	  }
	}
      }
    }





    //////////
    // update the min/max values
    //////////
    for (i=0; i<ts; i++) {
      int mini = x[i]->getMinElem();
      int maxi = x[i]->getMaxElem();
      if (mini > MinMax[i].min) {
	cap_flag = 1;
	MinMax[i].min = mini;
      }
      if (maxi < MinMax[i].max) {
	cap_flag = 1;
	MinMax[i].max = maxi;
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

  return P.leave();


failure:


  for (i=0; i<ts; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;

  return P.fail();
}



//----------------------------------------------------------------------

OZ_CFunHeader CPIteratePropagatorCumTI::spawner = sched_cumulativeTI;
OZ_CFunHeader TaskIntervalsPropagator::spawner = sched_taskIntervals;

