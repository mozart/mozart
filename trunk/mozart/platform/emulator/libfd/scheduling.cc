/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "scheduling.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>

//-----------------------------------------------------------------------------


OZ_C_proc_begin(sched_disjoint_card, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD "," OZ_EM_INT);
  
  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);

  return pe.impose(new SchedCardPropagator(OZ_args[0], OZ_args[1], 
					   OZ_args[2], OZ_args[3]));
}
OZ_C_proc_end

OZ_Return SchedCardPropagator::propagate(void) 
{
  OZ_DEBUGPRINT("in: " << *this);

  int &xd = reg_xd, &yd = reg_yd;

  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);
    
  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  
  if (xu + xd <= yl) return P.vanish();
  if (yu + yd <= xl) return P.vanish();
  
  if (xl + xd > yu) {
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
  }

  if (yl + yd > xu) {
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
  }
  
  
  OZ_DEBUGPRINT("out: " << *this);

  return P.leave();

failure:
  OZ_DEBUGPRINT("fail" << *this);
  return P.fail();
}


//-----------------------------------------------------------------------------


#define INITIALSIZE 10000

static OZ_FDIntVar * xx;
static int * dd;


static int compareDescRel(const void *a, const void *b) {
  return(xx[*(int*)b]->getMinElem()-xx[*(int*)a]->getMinElem());
}

static int compareAscDue(const void *a, const void *b) {
  return(xx[*(int*)a]->getMaxElem() + dd[*(int*)a] - xx[*(int*)b]->getMaxElem() - dd[*(int*)b]);
}

struct StartDurTerms {
  OZ_Term start;
  int dur;
};

int compareDurs(const void * a, const void * b) {
  return ((StartDurTerms *) b)->dur - ((StartDurTerms *) a)->dur;
}

CPIteratePropagator::CPIteratePropagator(OZ_Term tasks, 
					 OZ_Term starts, 
					 OZ_Term durs)
  : Propagator_VD_VI(OZ_vectorSize(tasks))
{
  VectorIterator vi(tasks);
  int i = 0;
		     
  DECL_DYN_ARRAY(StartDurTerms, sd, reg_sz);

  while (vi.anyLeft()) {
    OZ_Term task = vi.getNext();

    sd[i].start = OZ_subtree(starts, task);
    sd[i].dur = OZ_intToC(OZ_subtree(durs, task));
    i += 1;
  } // while

  OZ_ASSERT(i == reg_sz);

  qsort(sd, reg_sz, sizeof(StartDurTerms), compareDurs);

  for (i = reg_sz; i--; ) {
    reg_l[i]      = sd[i].start;
    reg_offset[i] = sd[i].dur;
  }
}

OZ_C_proc_begin(sched_cpIterate, 3)
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
	return OZ_FAILED;
      pe.expectIntVarMinMax(OZ_subtree(starts, task));
    }

    OZ_Return r = pe.impose(new CPIteratePropagator(tasks, starts, durs),
			    OZ_getLowPrio());

    if (r == FAILED) return FAILED;
  }
  return OZ_ENTAILED;
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

class Min_max {
public:
  int min, max;
};

struct Set {
  int cSi, dSi, mSi, sUp, sLow, extSize, val;
  int min,max;
  int * ext;
};


OZ_Return CPIteratePropagator::propagate(void)
{
  int &ts  = reg_sz;
  int * dur = reg_offset;

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

  int kUp;
  int kDown;
  int dur0;
  int mSi;


  DECL_DYN_ARRAY(int, set0, ts);
  DECL_DYN_ARRAY(int, compSet0, ts);
  DECL_DYN_ARRAY(int, forCompSet0Up, ts);
  DECL_DYN_ARRAY(int, forCompSet0Down, ts);
  DECL_DYN_ARRAY(int, outSide, ts);

  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
    forCompSet0Up[i] = i;
    forCompSet0Down[i] = i;
  }

  int set0Size;
  int compSet0Size;
  int outSideSize;
  int mysup = OZ_getFDSup();
  
  DECL_DYN_ARRAY(Set, Sets, ts);


  for (i = ts; i--; )
    Sets[i].ext = (int *) OZ_FDIntVar::operator new(sizeof(int) * ts);

  // memory is automatically disposed when propagator is left

  //////////  
  // do the reified stuff for task pairs.
  //////////  
  for (i=0; i<ts; i++)
    for (j=i+1; j<ts; j++) {
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

cploop:

  //////////  
  // UPPER PHASE
  //////////  


  /*
  // it's not worth it
  // Cut a hole into tasks if it must be scheduled in the following interval
  for (i=0; i<ts; i++)
    if (x[i]->getMaxElem() < x[i]->getMinElem() + dur[i]) {
      OZ_FiniteDomain lb;
      lb.initRange(x[i]->getMinElem()+dur[i], mysup);
      for (j=0; j<ts; j++)
	if (i != j) {
	  OZ_FiniteDomain la, l1;
	  la.initRange(0, x[i]->getMaxElem()- dur[j]);
	  l1 = (la | lb);
	  FailOnEmpty(*x[j] &= l1);
	}
    }
    */

  //////////  
  // sort by descending release date; ie. min(s1) > min(s2) > min(s3) etc.
  //////////  
  qsort(forCompSet0Up, ts, sizeof(int), compareDescRel);

  {
  for (int upTask=0; upTask < ts; upTask++) {
    
    kUp = MinMax[upTask].max + dur[upTask];
    kDown = MinMax[upTask].min;
    dur0 = 0;
    mSi = mysup;
    int maxSi = 0;
    set0Size = 0;
    compSet0Size = 0;
    outSideSize = 0;
    int maxEst = 0;

    //////////  
    // compute set S0 
    //////////  
    int l;
    for (l=0; l<ts; l++) {
      int dl = dur[l];
      int xlMin = MinMax[l].min;
      int xlMaxDL = MinMax[l].max + dl;
      if (( kDown <= xlMin) && ( xlMaxDL <= kUp)) {
	dur0 =+ dl;
	maxEst = max(maxEst,xlMin+dl); 
	mSi = min( mSi, xlMin+dl );
	maxSi = max( maxSi, MinMax[l].max );
	set0[set0Size++] = l;
      }
      else {
	if (xlMaxDL > kUp) {
	  outSide[outSideSize++] = l;
	}
      }
    }
    for (l=0; l<ts; l++) {
      int realL = forCompSet0Up[l];
      if ( (MinMax[realL].min < kDown) && 
	   (MinMax[realL].max + dur[realL] <= kUp) )
	compSet0[compSet0Size++] = realL;
    }

    if (kUp-kDown < dur0) return FAILED;
    
    struct Set *oset = &Sets[0];	
//    oset ->cSi = kDown +dur0;
    oset ->cSi = max(kDown +dur0, maxEst);
    oset->dSi = dur0;
    oset->mSi = mSi;
    oset->min = mSi;
    oset->max = maxSi;
    oset->sUp = kUp;
    oset->sLow = kDown;
    oset->extSize = set0Size;
    {
      for (int k = 0; k < ts; k++) {
	oset->ext[k] = set0[k];
      }
    }

    int setSize = 0;

    //////////  
    // compute the sets Si
    //////////  
    for (l=0; l<compSet0Size; l++) {
      int realL = compSet0[l];
      if (MinMax[realL].max+dur[realL] <= kUp) {
	int setSizeBefore = setSize;	
	struct Set *bset = &Sets[setSizeBefore];	
	setSize++;
	int dSi = bset->dSi + dur[realL];
	int minL = MinMax[realL].min;
	int newCSi = max( bset->cSi, minL+dSi);
	if (newCSi > kUp) 
	  return FAILED;
	else {
	  struct Set *cset = &Sets[setSize];	
	  cset->cSi = newCSi;
	  cset->dSi = dSi;
	  cset->mSi = min(bset->mSi, minL+dur[realL]);
	  cset->min = cset->mSi;
	  cset->max = max(bset->max, MinMax[realL].max);
	  cset->sUp = kUp;
	  cset->sLow = minL;
	  cset->extSize = bset->extSize+1;
	  cset->val = realL;
	}
      }
    }


    //////////  
    // edgepushUp
    //////////  
	 /*
    for (int t1=setSize; t1>=0; t1--) {
      for (int t2=0; t2<ts; t2++) {
	struct Set *s = &Sets[t1];	
	if (MinMax[t2].min > s->sLow) {
	  int tx;
	  if (MinMax[t2].max+dur[t2] <= kUp) {
	    tx = MinMax[t2].min;
	  }
	  else {
	    tx = MinMax[t2].min+dur[t2];
	  }
	  if ((tx > kUp - s->dSi) && (MinMax[t2].min < s->mSi)) {
	    upFlag = 1;
//	    FailOnEmpty(*x[t2] >= s->mSi);
//	    MinMax[t2].min = x[t2]->getMinElem();
  	    constraints[constraintsSize] = t2;
	    constraints[constraintsSize+1] = 1;
	    constraints[constraintsSize+2] = s->mSi;
	    constraintsSize +=3;
	  }
	}
      }
    }
    */


    //////////  
    // Do the edge-finding
    //////////  
    int lCount = 0;
    int setCount = setSize;
    while ( (setCount >= 0) && (lCount < outSideSize) ) {
      int l = outSide[lCount];
      struct Set *s = &Sets[setCount];
      int minL = MinMax[l].min;
      int maxL = MinMax[l].max;
      int durL = dur[l];

      if (maxL+durL > s->sUp) {
	if ( (s->sUp - s->sLow >= s->dSi + durL) &&
	     (minL+durL <= s->max) &&
	     (s->min <= maxL) )
	     {
	  // case 4: l may be inside
          if (s->sUp - minL >= s->dSi + durL) {
	    // case 5: L may be first
	    lCount++; 
	    setCount--;
	  }
	  else {
	    // it cannot be first
	    setCount--;
	    FailOnEmpty(*x[l] >= s->mSi);
	  }
	}
	else {
	  if (s->sUp - minL >= s->dSi + durL) {
	    // case 5: L may be first
	    lCount++;
	  }
	  else {

	    // l must be last
            if (minL < s->cSi) {
	      upFlag = 1;
	      FailOnEmpty(*x[l] >= s->cSi);
	      for (i=0; i < Sets[0].extSize; i++) {
		int sext = Sets[0].ext[i];
		int right = maxL-dur[sext];
		if (right < 0) 
		  return FAILED;
		if (MinMax[sext].max > right) {
		  FailOnEmpty(*x[sext] <= right);
		}
	      }

	      for (i=1; i <= setCount; i++) {
		int sext = Sets[i].val;
		int right = maxL-dur[sext];
		if (right < 0) 
		  return FAILED;
		if (MinMax[sext].max > right) {
		  FailOnEmpty(*x[sext] <= right);
		}
	      }

	      lCount++;
	    }
	    else lCount++;
	  }
	}
      }
      else lCount++;
    }

  }
  }

  for (i=0; i<ts; i++) {
    MinMax[i].min = x[i]->getMinElem();	
    MinMax[i].max = x[i]->getMaxElem();	
  }

  //////////  
  // DOWN PHASE
  //////////  

  
  //////////  
  // sort by ascending due date; ie. max(s1)+dur(s1) < max(s2)+dur(s2)
  //////////  
  qsort(forCompSet0Down, ts, sizeof(int), compareAscDue);


  {
  for (int downTask=0; downTask < ts; downTask++) {
    
    kUp = MinMax[downTask].max + dur[downTask];
    kDown = MinMax[downTask].min;
    dur0 = 0;
    mSi = 0;
    int minSi = mysup;
    set0Size = 0;
    compSet0Size = 0;
    outSideSize = 0;
    int minLst =  mysup;
    
    //////////  
    // compute set S0 
    //////////  
    int l;
	 for (l=0; l<ts; l++) {
	   int dl = dur[l];
	   int xlMin = MinMax[l].min;
	   int xlMax = MinMax[l].max;
	   int xlMaxDL = xlMax + dl;
	   if (( kDown <= xlMin) && ( xlMaxDL <= kUp)) {
	     dur0 =+ dl;
	     mSi = max( mSi, xlMax );
	     minSi = min( minSi, xlMin+dl );
	     minLst = min(minLst,xlMax);
	     set0[set0Size++] = l;
	   }
	   else {
	     if (xlMin < kDown) {
	       outSide[outSideSize++] = l;
	     }
	   }
	 }
    
    for (l=0; l<ts; l++) {
      int realL = forCompSet0Down[l];
      if ( (MinMax[realL].min >= kDown) && 
	   (MinMax[realL].max + dur[realL] > kUp) )
	compSet0[compSet0Size++] = realL;
    }

    if (kUp-kDown < dur0) return FAILED;

    struct Set *oset = &Sets[0];
    oset->cSi = min(kUp - dur0,minLst);
//    oset->cSi = kUp - dur0;
    oset->dSi = dur0;
    oset->mSi = mSi;
    oset->max = mSi;
    oset->min = minSi;
    oset->sUp = kUp;
    oset->sLow = kDown;
    oset->extSize = set0Size;
    {
      for (int k = 0; k < ts; k++) {
	oset->ext[k] = set0[k];
      }
    }

    int setSize = 0;

    //////////  
    // compute the sets Si
    //////////  
    {
      int l;
      for (l=0; l<compSet0Size; l++) {
	int realL = compSet0[l];
	if (MinMax[realL].min >= kDown) {
	  int setSizeBefore = setSize;	
	  struct Set *bset = &Sets[setSizeBefore];	
	  int durL = dur[realL];
	  setSize++;
	  int dSi = bset->dSi + durL;
	  int maxL = MinMax[realL].max+durL;
	  int newCSi = min( bset->cSi, maxL-dSi);
	  if (newCSi < kDown) 
	    return FAILED;
	  else {
	    struct Set *cset = &Sets[setSize];	
	    cset->cSi = newCSi;
	    cset->dSi = dSi;
	    cset->mSi = max(bset->mSi, maxL-durL);
	    cset->min = min(bset->min, MinMax[realL].min+dur[realL]);
	    cset->max = cset->mSi;
	    cset->sUp = maxL;
	    cset->sLow = kDown;
	    cset->extSize = bset->extSize+1;
	    cset->val = realL;
	  }
	}
      }
    }
    
    /*
    //////////  
    // edgepushDown
    //////////  
    for (int t1=setSize; t1>=0; t1--) {
      for (int t2=0; t2<ts; t2++) {
	struct Set *s = &Sets[t1];	
	int durT2 = dur[t2];
	int t2Max = MinMax[t2].max;
	if (t2Max+durT2 < s->sUp) {
	  int tx;
	  if (MinMax[t2].min >= kDown) {
	    tx = t2Max+durT2;
	  }
	  else {
	    tx = t2Max;
	  }
	  if ((tx < kDown + s->dSi) && (t2Max+durT2 > s->mSi)) {
	    downFlag = 1;
//	    FailOnEmpty(*x[t2] <= s->mSi-durT2);
//	    MinMax[t2].max = x[t2]->getMaxElem();
  	    constraints[constraintsSize] = t2;
	    constraints[constraintsSize+1] = 0;
	    constraints[constraintsSize+2] = s->mSi-durT2;
	    constraintsSize +=3;
	  }
	}
      }
    }
    */

    //////////  
    // DO the edge finding
    //////////  
    int lCount = 0;
    int setCount = setSize;
    while ( (setCount >= 0) && (lCount < outSideSize) ) {
      int l = outSide[lCount];
      struct Set *s = &Sets[setCount];
      int minL = MinMax[l].min;
      int maxL = MinMax[l].max;
      int durL = dur[l];

      if (minL < s->sLow) {
	if ( (s->sUp - s->sLow >= s->dSi + durL) &&
	     (minL+durL <= s->max) &&
	     (s->min <= maxL) )
	     {
	  // case 4: l may be inside
	    if (maxL + durL - s->sLow >= s->dSi + durL) {
	    // case 5: L may be last
	    lCount++; 
	    setCount--;
	  }
	  else {
	    // it cannot be last
	      setCount--;
	      FailOnEmpty(*x[l] <= s->mSi - durL);
	  }
	}
	else {
	  if (maxL + durL - s->sLow >= s->dSi + durL) {
	    // case 5: L may be last
	    lCount++;
	  }
	  else {

	    // l must be first
            if (maxL+durL > s->cSi) {
	      downFlag = 1;
	      int right = s->cSi - durL;
	      if (right < 0)
	        return FAILED;
	      FailOnEmpty(*x[l] <= right);

	      int minDL = minL + durL;
	      for (i=0; i < Sets[0].extSize; i++) {
		int element = Sets[0].ext[i];
		if (MinMax[element].min < minDL) {
		  FailOnEmpty(*x[element] >= minDL);
		}
	      }

	      for (i=1; i <= setCount; i++) {
		int element = Sets[i].val;
		if (MinMax[element].min < minDL) {
		  FailOnEmpty(*x[element] >= minDL);
		}
	      }

	      lCount++;
	    }
	    else lCount++;

	  }
	}
      }
      else lCount++;
    }


  }
  }

  for (i=0; i<ts; i++) {
    MinMax[i].min = x[i]->getMinElem();	
    MinMax[i].max = x[i]->getMaxElem();	
  }

  
  if ((upFlag == 1)||(downFlag==1)) {
    upFlag = 0;
    downFlag = 0;
    goto cploop;
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


  return P.leave();

failure:
  return P.fail();
}

//--------------------------------------------------------------

OZ_C_proc_begin(sched_disjunctive, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT);
  
  PropagatorExpect pe;
  
  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  return pe.impose(new DisjunctivePropagator(OZ_args[0], OZ_args[1]),  
		   OZ_getLowPrio());
}
OZ_C_proc_end

OZ_Return DisjunctivePropagator::propagate(void)
{
  int &ts  = reg_sz;
  int * dur = reg_offset;

  DECL_DYN_ARRAY(OZ_FDIntVar, x, ts);
  PropagatorController_VV P(ts, x);
  
  int i, j;
  for (i = ts; i--; )
    x[i].read(reg_l[i]);

  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
  }


  int disjFlag = 0;

reifiedloop:

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


  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------

struct Interval {
  int left, right, use;
};

int ozcdecl CompareIntervals(const void *In1, const void *In2) 
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

int ozcdecl CompareBounds(const void *Int1, const void *Int2) {
  return *(int*)Int1 - *(int*)Int2;
}
OZ_C_proc_begin(sched_cpIterateCap, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT "," OZ_EM_VECT OZ_EM_INT ","  OZ_EM_INT);
  
  PropagatorExpect pe;
  
  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectVectorInt);
  OZ_EXPECT(pe, 2, expectVectorInt);
  OZ_EXPECT(pe, 3, expectInt);
  SAMELENGTH_VECTORS(0, 1);
  
  return pe.impose(new CPIteratePropagatorCap(OZ_args[0], OZ_args[1], 
					      OZ_args[2], OZ_args[3]),  
		   OZ_getLowPrio());
}
OZ_C_proc_end

/*
// flag version for interval reasoning only
OZ_C_proc_begin(sched_cpIterateCap, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT "," OZ_EM_VECT OZ_EM_INT ","  OZ_EM_INT ","  OZ_EM_INT);
  
  PropagatorExpect pe;
  
  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectVectorInt);
  OZ_EXPECT(pe, 2, expectVectorInt);
  OZ_EXPECT(pe, 3, expectInt);
  OZ_EXPECT(pe, 4, expectInt);
  SAMELENGTH_VECTORS(0, 1);
  
  return pe.impose(new CPIteratePropagatorCap(OZ_args[0], OZ_args[1],
					     OZ_args[2], OZ_args[3],
					     OZ_intToC(OZ_args[4])), 
		  OZ_getLowPrio());
}
OZ_C_proc_end
*/

struct Set2 {
  int dSi, sUp, sLow, extSize;
  int * ext;
};


OZ_Return CPIteratePropagatorCap::propagate(void)
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
  /*
  for (i = ts; i--; ){
    dd[i] = dur[i];
  }
  */
  dd = reg_offset;

  
  int upFlag = 0;
  int downFlag = 0;
  int disjFlag = 0;
  int cap_flag = 0;

  int kUp;
  int kDown;
  int dur0;
  int mSi;

  DECL_DYN_ARRAY(int, set0, ts);
  DECL_DYN_ARRAY(int, compSet0, ts);
  DECL_DYN_ARRAY(int, forCompSet0Up, ts);
  DECL_DYN_ARRAY(int, forCompSet0Down, ts);
  DECL_DYN_ARRAY(int, outSide, ts);

  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
    forCompSet0Up[i] = i;
    forCompSet0Down[i] = i;
  }

  int set0Size;
  int compSet0Size;
  int outSideSize;
  int mysup = OZ_getFDSup();
  
  DECL_DYN_ARRAY(Set2, Sets, ts);
  
  for (i = ts; i--; )
    Sets[i].ext = (int *) OZ_FDIntVar::operator new(sizeof(int) * ts);

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


cploop:

  //////////  
  // UPPER PHASE
  //////////  

  //////////  
  // sort by descending release date; ie. min(s1) > min(s2) > min(s3) etc.
  //////////  
  qsort(forCompSet0Up, ts, sizeof(int), compareDescRel);

  {
  for (int upTask=0; upTask < ts; upTask++) {

    kUp = MinMax[upTask].max + dur[upTask];
    kDown = MinMax[upTask].min;
    int use0 = 0;
    set0Size = 0;
    compSet0Size = 0;
    outSideSize = 0;

    // compute set S0 
    int l;
    for (l=0; l<ts; l++) {
      int dl = dur[l];
      int xlMin = MinMax[l].min;
      int xlMaxDL = MinMax[l].max + dl;
      if (( kDown <= xlMin) && ( xlMaxDL <= kUp)) {
	use0 =+ dl*use[l];
	set0[set0Size++] = l;
      }
      else {
	if (xlMaxDL > kUp) {
	  outSide[outSideSize++] = l;
	}
      }
    }
    for (l=0; l<ts; l++) {
      int realL = forCompSet0Up[l];
      if ( (MinMax[realL].min < kDown) && 
	   (MinMax[realL].max + dur[realL] <= kUp) )
	compSet0[compSet0Size++] = realL;
    }


    if ((kUp-kDown)*capacity < use0) {
      return FAILED;
    }

    
    struct Set2 *oset = &Sets[0];	
    oset->dSi = use0;
    oset->sUp = kUp;
    oset->sLow = kDown;
    oset->extSize = set0Size;
    {
      for (int k = 0; k < ts; k++) {
	oset->ext[k] = set0[k];
      }
    }


    int setSize = 0;

    //////////  
    // compute the sets Si
    //////////  
    for (l=0; l<compSet0Size; l++) {
      int realL = compSet0[l];
      if (MinMax[realL].max+dur[realL] <= kUp) {
	int setSizeBefore = setSize;	
	struct Set2 *bset = &Sets[setSizeBefore];	
	setSize++;
	int dSi = bset->dSi + dur[realL]*use[realL];
	int minL = MinMax[realL].min;
	if ( (kUp - minL)*capacity < dSi) {
	  return FAILED;
	}
	else {
	  struct Set2 *cset = &Sets[setSize];	
	  cset->dSi = dSi;
	  cset->sUp = kUp;
	  cset->sLow = minL;
	  cset->extSize = bset->extSize+1;
	}
      }
    }

    //////////  
    // Do the edge-finding
    //////////  
    int lCount = 0;
    int setCount = setSize;
    while ( (setCount >= 0) && (lCount < outSideSize) ) {
      int l = outSide[lCount];
      struct Set2 *s = &Sets[setCount];
      int minL = MinMax[l].min;
      int maxL = MinMax[l].max;
      int durL = dur[l];
      int useL = use[l];

      if (maxL+durL > s->sUp) {
	if ( (s->sUp - s->sLow)*capacity >= s->dSi + durL*useL) {
	  // case 4: l may be inside
          if ( (s->sUp - minL)*capacity >= s->dSi + durL*useL) {
	    // case 5: L may be first
	    lCount++; 
	    setCount--;
	  }
	  else {
	    // it cannot be first
	    setCount--;
	  }
	}
	else {
	  if ( (s->sUp - minL)*capacity >= s->dSi + durL*useL) {
	    // case 5: L may be first
	    lCount++;
	  }
	  else {
	    int rest = s->dSi - (s->sUp - s->sLow)*(capacity - useL);
	    if (rest > 0) {
	      // l must be last
	      int val = s->sLow + (int) ceil((double) rest / (double) useL);
	      if (minL < val) {
		upFlag = 1;
		FailOnEmpty(*x[l] >= val);
//		MinMax[l].min = x[l]->getMinElem();
	      }
	      lCount++;
	    }
	    else lCount++;
	  }
	}
      }
      else lCount++;
    }
  }
  }

  for (i=0; i<ts; i++) {
    MinMax[i].min = x[i]->getMinElem();	
    MinMax[i].max = x[i]->getMaxElem();	
  }

  //////////
  // DOWN PHASE
  //////////

  

  //////////
  // sort by ascending due date; ie. max(s1)+dur(s1) < max(s2)+dur(s2)
  //////////
  qsort(forCompSet0Down, ts, sizeof(int), compareAscDue);


  {
  for (int downTask=0; downTask < ts; downTask++) {
    
    kUp = MinMax[downTask].max + dur[downTask];
    kDown = MinMax[downTask].min;
    int use0 = 0;
    set0Size = 0;
    compSet0Size = 0;
    outSideSize = 0;
    
    //////////
    // compute set S0 
    //////////
    int l;
    for (l=0; l<ts; l++) {
      int dl = dur[l];
      int xlMin = MinMax[l].min;
      int xlMaxDL = MinMax[l].max + dl;
      if (( kDown <= xlMin) && ( xlMaxDL <= kUp)) {
	use0 =+ dl*use[l];
	set0[set0Size++] = l;
      }
      else {
	if (xlMin < kDown) {
	  outSide[outSideSize++] = l;
	}
      }
    }

    for (l=0; l<ts; l++) {
      int realL = forCompSet0Down[l];
      if ( (MinMax[realL].min >= kDown) && 
	   (MinMax[realL].max + dur[realL] > kUp) )
	compSet0[compSet0Size++] = realL;
    }
    
    if ( (kUp-kDown)*capacity < use0) return FAILED;

    struct Set2 *oset = &Sets[0];
    oset->dSi = use0;
    oset->sUp = kUp;
    oset->sLow = kDown;
    oset->extSize = set0Size;
    {
      for (int k = 0; k < ts; k++) {
	oset->ext[k] = set0[k];
      }
    }

    int setSize = 0;

    //////////
    // compute the sets Si
    //////////
    {
      int l;
      for (l=0; l<compSet0Size; l++) {
	int realL = compSet0[l];
	if (MinMax[realL].min >= kDown) {
	  int setSizeBefore = setSize;	
	  struct Set2 *bset = &Sets[setSizeBefore];	
	  int durL = dur[realL];
	  setSize++;
	  int dSi = bset->dSi + durL*use[realL];
	  int maxL = MinMax[realL].max+durL;
	  if ( (maxL - kDown)*capacity < dSi)
	    return FAILED;
	  else {
	    struct Set2 *cset = &Sets[setSize];	
	    cset->dSi = dSi;
	    cset->sUp = maxL;
	    cset->sLow = kDown;
	    cset->extSize = bset->extSize+1;
	  }
	}
      }
    }

    //////////
    // Do the edge-finding
    //////////
    int lCount = 0;
    int setCount = setSize;
    while ( (setCount >= 0) && (lCount < outSideSize) ) {
      int l = outSide[lCount];
      struct Set2 *s = &Sets[setCount];
      int minL = MinMax[l].min;
      int maxL = MinMax[l].max;
      int durL = dur[l];
      int useL = use[l];

//      cout << "setCount: " << setCount << "\n";
//      cout << "lCount: " << lCount << "\n";

      if (minL < s->sLow) {
	if ( (s->sUp - s->sLow)*capacity >= s->dSi + durL*useL) {
	  // case 4: l may be inside
	    if ( (maxL + durL - s->sLow)*capacity >= s->dSi + durL*useL) {
	    // case 5: L may be last
	    lCount++; 
	    setCount--;
	  }
	  else {
	    // it cannot be last
	    setCount--;
	  }
	}
	else {
	  if (maxL + durL - s->sLow >= s->dSi + durL) {
	    // case 5: L may be last
	    lCount++;
	  }
	  else {
	    int rest = s->dSi - (s->sUp - s->sLow)*(capacity - useL);
	    if (rest > 0) {
	      int val = s->sUp - (int) ceil( (double) rest / (double) useL);
	      // l must be first
              if (maxL+durL > val) {
		downFlag = 1;
		int right = val - durL;
		if (right < 0)
		  return FAILED;
		FailOnEmpty(*x[l] <= right);
//		MinMax[l].max = x[l]->getMaxElem();
	      }
	      lCount++;
	    }
	    else lCount++;
	  }
	}
      }
      else lCount++;
    }
  }
  }
  for (i=0; i<ts; i++) {
    MinMax[i].min = x[i]->getMinElem();	
    MinMax[i].max = x[i]->getMaxElem();	
  }


  if ((upFlag == 1)||(downFlag==1)) {
    upFlag = 0;
    downFlag = 0;
    goto cploop;
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
    for (i=0; i<ts; i++) {
      if (MinMax[i].min < min_left) min_left = MinMax[i].min;
      if (MinMax[i].max > max_right) max_right = MinMax[i].max;
    }
    Intervals[interval_nb].left = min_left;
    Intervals[interval_nb].right = max_right;
    Intervals[interval_nb].use = 0;
    interval_nb++;
    
    /*
    cout << "alt\n";
    for(i=0;i<interval_nb;i++){
      cout << Intervals[i].left << " " << Intervals[i].right << endl;
    }
    */

    //////////
    // sort the intervals lexicographically
    //////////
    Interval * intervals = Intervals;
    qsort(intervals, interval_nb, sizeof(Interval), CompareIntervals);

    /*
    cout << "neu\n";
    for(i=0;i<interval_nb;i++){
      cout << Intervals[i].left << " " << Intervals[i].right << " " << Intervals[i].use << endl;
    }
    */

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
	  cum = cum + Intervals[i].use;
	}
      }
      if (cum > capacity) return FAILED;
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
    /*
    if (reg_flag == 0)
      {
      */
	// perhaps some tests before generalizing domains could improve
        for (i=0; i<ts; i++) {
	  int lst = MinMax[i].max;
	  int ect = MinMax[i].min + dur[i];
	  int use_i = use[i];
	  int dur_i = dur[i];
	  for (j=0; j<exclusion_nb; j++) {
	    Interval Exclusion = ExclusionIntervals[j];
	    if (Exclusion.use + use_i > capacity) {
	      int left = Exclusion.left;
	      int right = Exclusion.right;
	      if (lst < ect) {
		if ( (lst <= left) && (right <= ect) ) continue;
		else {
		  if (Exclusion.use + use_i > capacity) {
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
	      else {
		if (Exclusion.use + use_i > capacity) {
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
    /*
      }
  
    else {
      
      OZ_FiniteDomain la, lb;
      for (i=0; i<ts; i++) {
	int lst = MinMax[i].max;
	int ect = MinMax[i].min + dur[i];
	int use_i = use[i];
	int dur_i = dur[i];
	lb.initFull();
	for (j=0; j<exclusion_nb; j++) {
	  Interval Exclusion = ExclusionIntervals[j];
	  if (Exclusion.use + use_i > capacity) {
	    int left = Exclusion.left;
	    int right = Exclusion.right;
	    if (lst < ect) {
	      if ( (lst <= left) && (right <= ect) ) continue;
	      else {
		if (Exclusion.use + use_i > capacity) {
		  la.initRange(left-dur_i+1,right-1);
		  FailOnEmpty(lb -= la);
		  
		  // new
 	          // for capacity > 1 we must count the used resource. 
                  // But this is too expensive.
		  if ((left - last < dur_i) && (capacity == 1)) {
		    la.initRange(last,right-1);
		    FailOnEmpty(lb -= la);
		  }
		  last = right;
		  
		}
	      }
	    }
	    else {
	      if (Exclusion.use + use_i > capacity) {
		la.initRange(left-dur_i+1,right-1);
		FailOnEmpty(lb -= la);
		
		// new
 	        // for capacity > 1 we must count the used resource. 
                // But this is too expensive.
  	        if ((left - last < dur_i) && (capacity == 1)) {
		  la.initRange(last,right-1);
		  FailOnEmpty(lb -= la);
		}
		last = right;
		
	      }
	    }
	  }
	}
	FailOnEmpty(lb >= x[i]->getMinElem());
	FailOnEmpty(lb <= x[i]->getMaxElem());
	FailOnEmpty(*x[i] >= lb.getMinElem());
	FailOnEmpty(*x[i] <= lb.getMaxElem());
      }
    }
    */

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

  return P.leave();

failure:

  return P.fail();
}



//-----------------------------------------------------------------------------


OZ_C_proc_begin(sched_cpIterateCapUp, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT "," OZ_EM_VECT OZ_EM_INT ","  OZ_EM_INT);

  PropagatorExpect pe;
  
  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectVectorInt);
  OZ_EXPECT(pe, 2, expectVectorInt);
  OZ_EXPECT(pe, 3, expectInt);
  SAMELENGTH_VECTORS(0, 1);
  
  return pe.impose(new CPIteratePropagatorCapUp(OZ_args[0], OZ_args[1], 
						OZ_args[2], OZ_args[3]),  
		   OZ_getLowPrio());
}
OZ_C_proc_end

OZ_Return CPIteratePropagatorCapUp::propagate(void)
{
  int &ts      = reg_sz;
  int * dur    = reg_offset;
  int * use    = reg_use;
  int capacity = reg_capacity;

  DECL_DYN_ARRAY(OZ_FDIntVar, x, ts);

  PropagatorController_VV P(ts, x);
  
  int i, j;
  for (i = ts; i--; )
    x[i].read(reg_l[i]);

  int cap_flag = 0;
  int mysup = OZ_getFDSup();

  struct Min_max {
    int min, max;
  };
  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
  }


capLoop:

  // do the capacity checking
  { 
    int interval_nb = 0;	  
    
    DECL_DYN_ARRAY(Interval, Intervals, ts+1);

    // compute intervals, where the task may occupy place 
    for (i=ts; i--;){
      // latest start time < earliest completion?
      Intervals[interval_nb].left = MinMax[i].min;
      Intervals[interval_nb].right = MinMax[i].max + dur[i];
      Intervals[interval_nb].use = use[i];
      interval_nb++;
    }

    // compute left and right bound for cumulative checking
    int min_left = mysup;
    int max_right = 0;
    for (i=0; i<ts; i++) {
      if (MinMax[i].min < min_left) min_left = MinMax[i].min;
      if (MinMax[i].max+dur[i] > max_right) max_right = MinMax[i].max+dur[i];
    }
    Intervals[interval_nb].left = min_left;
    Intervals[interval_nb].right = max_right;
    Intervals[interval_nb].use = 0;
    interval_nb++;
    
    

    Interval * intervals = Intervals;
    qsort(intervals, interval_nb, sizeof(Interval), CompareIntervals);


    // compute the set of all bounds of intervals
    int double_nb = interval_nb*2;
    DECL_DYN_ARRAY(int, IntervalBounds, double_nb);

    for (i=0; i<interval_nb; i++) {
      IntervalBounds[2*i] = Intervals[i].left;
      IntervalBounds[2*i+1] = Intervals[i].right;
    }
    
    int * intervalBounds = IntervalBounds;
    qsort(intervalBounds, double_nb, sizeof(int), CompareBounds);

    // compute the set of intervals, for which there is inclusion
    DECL_DYN_ARRAY(Interval, InclusionIntervals, double_nb);
    int inclusion_nb = 0;
    int low_counter  = 0;
    int left_pt      = 0;
    int right_pt     = 0;


    while (left_pt < double_nb) {
      int left_val = IntervalBounds[left_pt];
      while ((right_pt < double_nb) && (IntervalBounds[right_pt] == left_val))
	right_pt++;
      if (right_pt == double_nb)  break;
      int right_val = IntervalBounds[right_pt];
      // cum is the amount of possible usage
      int cum = 0;
      for (i=low_counter; i<interval_nb; i++) {
	int leftInt = Intervals[i].left;
	int rightInt = Intervals[i].right;
	// really for this???
	if (leftInt > right_val) break;
	if ( (leftInt <= left_val) && (right_val <= rightInt) ) {
	  cum = cum + Intervals[i].use;
	}
      }
      if (cum < capacity) {
	return FAILED;
      }
      InclusionIntervals[inclusion_nb].left = left_val; 
      InclusionIntervals[inclusion_nb].right = right_val;
      InclusionIntervals[inclusion_nb].use = cum;
      inclusion_nb++;
      left_pt = right_pt;
    }


    // include places, where tasks must be scheduled.
    for (i=0; i<ts; i++) {
      int mini  = MinMax[i].min;
      int maxi  = MinMax[i].max + dur[i];
      int use_i = use[i];
      int dur_i = dur[i];
      for (j=0; j<inclusion_nb; j++) {
	Interval Inclusion = InclusionIntervals[j];
	if (Inclusion.use - use_i < capacity) {
	  int left = Inclusion.left;
	  int right = Inclusion.right;
	  if ( (mini <= left) && (right <= maxi) ) {
	    if (Inclusion.use - use_i < capacity) {
	      if (dur_i < right - left) goto failure;
	      OZ_FiniteDomain la;
	      la.initRange(right-dur_i,left);
	      FailOnEmpty(*x[i] &= la);
	      int mini_new = x[i]->getMinElem();
	      int maxi_new = x[i]->getMaxElem();
	      if (mini_new > MinMax[i].min) {
		cap_flag = 1;
		MinMax[i].min = mini_new;
	      }
	      if (maxi_new < MinMax[i].max) {
		cap_flag = 1;
		MinMax[i].max = maxi_new;
	      }
	    }
	  }
	  else {
	    if ( (mini < left) && (left < maxi) && (maxi < right) )
	      cout << "Fatal1: " << mini <<" "<<left<<" "<<maxi<<" "<<right<<endl;
	    else if ( (left < mini) && (mini < right) && (right < maxi) )
	      cout << "Fatal2: " << mini <<" "<<left<<" "<<maxi<<" "<<right<<endl;
	    else if ( (left < mini) && (maxi < right) )
	      cout << "Fatal3: " << mini <<" "<<left<<" "<<maxi<<" "<<right<<endl;
	    else;
	    // note that this can happen because of smaller intervals.
	    // But because we are looping, everything is fine.
	  }
	}
      }
    }




    if (cap_flag == 1) {
      cap_flag = 0;
      goto capLoop;
    }
      
    
  }
  
return P.leave();

failure:
  return P.fail();
}
  
//-----------------------------------------------------------------------------
// static member

OZ_CFun SchedCardPropagator::spawner = sched_disjoint_card;
OZ_CFun CPIteratePropagator::spawner = sched_cpIterate;
OZ_CFun CPIteratePropagatorCap::spawner = sched_cpIterateCap;
OZ_CFun CPIteratePropagatorCapUp::spawner = sched_cpIterateCapUp;
OZ_CFun DisjunctivePropagator::spawner = sched_disjunctive;

