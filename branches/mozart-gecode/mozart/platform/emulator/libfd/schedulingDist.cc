/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#include "base.hh"
#include "schedulingDist.hh"
#include "rel.hh"
#include "auxcomp.hh"


//////////////////////////////////////////////////////////////////////
// TaskIntervalsProof
//////////////////////////////////////////////////////////////////////

#define PAR 4
#define INITIALSIZE 10000


static inline double min(double a, double b) { return a < b ? a : b; }
static inline double max(double a, double b) { return a > b ? a : b; }
static inline int intMin(int a, int b) { return a < b ? a : b; }
static inline int intMax(int a, int b) { return a > b ? a : b; }


static int *constraints;
static int initConstraints[INITIALSIZE];


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


//////////
// CONSTRUCTOR
//////////

TaskIntervalsProof::TaskIntervalsProof(OZ_Term tasks, OZ_Term start,
				       OZ_Term durs, OZ_Term st, int flag) 
{
  stream = st;
  int i,j;
  int current = 0;
  reg_fds_size = 0;
  reg_max_nb_tasks = 0;

  int task_width    = OZ_width(tasks);
  reg_nb_tasks_size = task_width;
  reg_nb_tasks      = OZ_hallocCInts(reg_nb_tasks_size);

  for (i=0; i<task_width; i++) {
    int cwidth = OZ_width(OZ_getArg(tasks, i));
    reg_nb_tasks[i] = cwidth;
    reg_max_nb_tasks = intMax(cwidth, reg_max_nb_tasks);
    reg_fds_size += cwidth;
  }

  reg_pe    = OZ_hallocOzTerms(1);
  reg_pe[0] = OZ_subtree(start, OZ_atom("pe"));

  reg_fds  = OZ_hallocOzTerms(reg_fds_size);

  reg_durs = ((int **)::new int*[reg_nb_tasks_size + 1]) + 1;
  for (i=0; i<task_width; i++) {
    reg_durs[i] = ::new int[reg_nb_tasks[i]];
  }
  reg_durs[-1] = (int *) 0;


  for (i=0; i<task_width; i++) {
    OZ_Term ctasks = OZ_getArg(tasks, i);
    for (j=0; j<reg_nb_tasks[i]; j++) {
      OZ_Term ctask      = OZ_getArg(ctasks, j);
      reg_fds[current++] = OZ_subtree(start, ctask);
      reg_durs[i][j]     = OZ_intToC(OZ_subtree(durs, ctask));
    }
  }

  reg_order_vector_size = BoolMatrix3::requiredVectSize(task_width,
							reg_max_nb_tasks,
							reg_max_nb_tasks);
  reg_order_vector = OZ_hallocCInts(reg_order_vector_size);
  BoolMatrix3 bm3(reg_order_vector, reg_order_vector_size, task_width,
		  reg_max_nb_tasks, reg_max_nb_tasks);
  bm3.init();

  reg_flag = flag;
}

//////////
// DESTRUCTOR
//////////

TaskIntervalsProof::~TaskIntervalsProof(void)
{

  if (int(reg_durs[-1]) == 0) {
    for (int i=0; i<reg_nb_tasks_size; i++) {
      :: delete reg_durs[i];
    }
    reg_durs--;
    :: delete reg_durs;
  } else {
    reg_durs[-1] = (int *) int(reg_durs[-1]) - 1;
  }

}

//////////
// BUILTIN
//////////
OZ_BI_define(sched_taskIntervalsProof, 5, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_VECT OZ_EM_LIT "," OZ_EM_RECORD OZ_EM_FD \
		   "," OZ_EM_RECORD OZ_EM_INT "," OZ_EM_STREAM "," OZ_EM_INT);

  PropagatorExpect pe;

  pe.collectVarsOff();
  OZ_EXPECT(pe, 0, expectVectorVectorLiteral);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectVectorInt);
  pe.collectVarsOn();

  OZ_EXPECT(pe, 3, expectStream);
  OZ_EXPECT(pe, 4, expectInt);

  return pe.impose(new TaskIntervalsProof(OZ_in(0), OZ_in(1), 
					  OZ_in(2), OZ_in(3), 
					  OZ_intToC(OZ_in(4))));
}
OZ_BI_end

//////////
// COPYING
//////////

void TaskIntervalsProof::gCollect(void) {
  OZ_gCollectTerm(stream);

  reg_fds          = OZ_gCollectAllocBlock(reg_fds_size, reg_fds);
  reg_pe           = OZ_gCollectAllocBlock(1, reg_pe);
  reg_nb_tasks     = OZ_copyCInts(reg_nb_tasks_size, reg_nb_tasks);
  reg_order_vector = OZ_copyCInts(reg_order_vector_size, reg_order_vector);

}

void TaskIntervalsProof::sClone(void) {
  reg_durs[-1] = (int *) int(reg_durs[-1]) + 1;

  OZ_sCloneTerm(stream);

  reg_fds          = OZ_sCloneAllocBlock(reg_fds_size, reg_fds);
  reg_pe           = OZ_sCloneAllocBlock(1, reg_pe);
  reg_nb_tasks     = OZ_copyCInts(reg_nb_tasks_size, reg_nb_tasks);
  reg_order_vector = OZ_copyCInts(reg_order_vector_size, reg_order_vector);

}

//////////
// SPAWNER
//////////

OZ_PropagatorProfile TaskIntervalsProof::profile;


//////////
// RUN METHOD
//////////

OZ_Return TaskIntervalsProof::propagate(void) 
{
  /*
    one propagator for two distribution strategies.
    reg_flag == 0: proof of optimality
    reg_flag == 1: find good solutions
   */

  OZ_DEBUGPRINTTHIS("in ");

  OZ_Stream st(stream);

  /////////////
  //  Read the FDs
  ////////////

  DECL_DYN_ARRAY(OZ_FDIntVar *, xptr, reg_nb_tasks_size);
  FDIntVarArr2 all_fds(reg_nb_tasks_size, xptr, reg_nb_tasks);

  int i, j, k, l, left, right;

  for (i = 0, k = 0; i < reg_nb_tasks_size; i += 1)
    for (j = 0; j < reg_nb_tasks[i]; j += 1)
      all_fds[i][j].read(reg_fds[k++]);

  BoolMatrix3 bm3(reg_order_vector, reg_order_vector_size, reg_nb_tasks_size,
		  reg_max_nb_tasks, reg_max_nb_tasks);

  OZ_FDIntVar pe_var;
  pe_var.read(reg_pe[0]);

  DECL_DYN_ARRAY(int, resource_starts, reg_nb_tasks_size);
  j = 0;
  for (i = 0; i < reg_nb_tasks_size; i++) {
    resource_starts[i] = j;
    j = j + reg_nb_tasks[i];
  }


  DECL_DYN_ARRAY(int, firsts, reg_max_nb_tasks);
  DECL_DYN_ARRAY(int, lasts, reg_max_nb_tasks);

  struct Set {
    int low, up, dur, extSize;
    int * ext;
  };
  
  struct min_max_set {
    int min, max;
  };


  struct min_max_set ** all_vars;
  all_vars = ((min_max_set **)::new min_max_set*[reg_nb_tasks_size]);
  for (i=0; i<reg_nb_tasks_size; i++) {
    all_vars[i] = ::new min_max_set[reg_nb_tasks[i]];
  }


  struct Set ** taskints;
  taskints = ((Set **)::new Set*[reg_max_nb_tasks]);
  for (i=0; i<reg_max_nb_tasks; i++) {
    taskints[i] = ::new Set[reg_max_nb_tasks];
  }


  for (i=0; i<reg_max_nb_tasks; i++)
    for (j=0; j<reg_max_nb_tasks; j++)
      taskints[i][j].ext = ::new int[reg_max_nb_tasks];


  // this data structure is used to store information for
  // the redundant propagators which are added during search
  // for the most promising candidates to distribute with
  constraints = initConstraints;
  int * constraintsExtension = NULL;

  int constraintLimit = INITIALSIZE;


  ////////////
  // Stream processing
  ///////////

  while (!st.isEostr()) {
    OZ_Term e = st.get();
    if (OZ_isTuple(e) && ! OZ_isLiteral(e)) {
      const char * label = OZ_atomToC(OZ_label(e));
      if (! strcmp("dist", label)) {

	OZ_Term old_out = OZ_getArg(e, 0);
	OZ_Term new_out = OZ_getArg(e, 1);

	////////////////////
        // Initialization
        ///////////////////
	     
        // to store FD bounds

	
	int ** all_durs = reg_durs;

	for (i=0; i < reg_nb_tasks_size; i++) {
	  for (j=0; j < reg_nb_tasks[i]; j++) {
	    all_vars[i][j].min = all_fds[i][j]->getMinElem();
	    all_vars[i][j].max = all_fds[i][j]->getMaxElem();
	  }
	}



	if (OZ_isTuple(old_out) && ! OZ_isLiteral(old_out)) {
	  const char * old_label = OZ_atomToC(OZ_label(old_out));
	  // fill in order
          if (!strcmp("#", old_label)) {
	    // enter the ordering decision made in Oz
	    int res   = OZ_intToC( OZ_getArg(old_out, 0));
	    int left  = OZ_intToC( OZ_getArg(old_out, 1));
	    int right = OZ_intToC( OZ_getArg(old_out, 2));
	    bm3.set(res, left, right);
	  }
	  else {
	    printf("not nil\n");
	    goto failure;
	  }
	}
	else {
	  if ((OZ_isAtom(old_out)) && (!strcmp("nil", OZ_atomToC(OZ_label(old_out))))) {}
	  else {
	    printf(" no Tuple \n");
	    goto failure;
	  }
	}


	
        ////////////
        // Go for all resources
        ///////////

        int best_resource   = 0;
	struct Set best_set;
	int best_cost       = OZ_getFDSup();
	int best_left       = 0;
	int best_right      = 0;
	int constraintsSize = 0;
	int upper           = pe_var->getMinElem();


	best_set.ext = ::new int[reg_max_nb_tasks];


	for (i=0; i < reg_nb_tasks_size; i++) {

	  /////////
          // Initialize task intervals
          ////////
	  for (left = 0; left < reg_nb_tasks[i]; left++) 
	    for (right = 0; right < reg_nb_tasks[i]; right++) {
	      struct Set *cset = &taskints[left][right];
	      cset->low        = all_vars[i][left].min;
	      cset->up         = all_vars[i][right].max + all_durs[i][right];
	      int cdur = 0;
	      int csize = 0;
	      if ( (cset->low <= all_vars[i][right].min)
		   && (all_vars[i][left].max + all_durs[i][left] <= cset->up)
		   ) 
		{
		// otherwise the task interval is trivially empty
		for (l=0; l < reg_nb_tasks[i]; l++) 
		  if ( (cset->low <= all_vars[i][l].min)
		       && (all_vars[i][l].max + all_durs[i][l] <= cset->up) ) {
		    cdur += all_durs[i][l];
		    cset->ext[csize++] = l;
		  }
	      }
	      cset->dur     = cdur;
	      cset->extSize = csize;

	      if ( (csize > 0) && (cset->up - cset->low < cdur) ) {
		goto failure;
	      }
	    }

	  ///////////
          // Compute the best interval and redundant propagators
          //////////

	  struct Set *loc_best_set = NULL;
	  int loc_best_nc    = 0;
	  int loc_best_slack = 0;
	  int loc_best_left  = 0;
	  int loc_best_right = 0;
	  int loc_best_slack_nc  = OZ_getFDSup();
	  int loc_resource_slack = OZ_getFDSup();
	  for (left=0; left < reg_nb_tasks[i]; left++) 
	    for (right=0; right < reg_nb_tasks[i]; right++) {
              // check data structure for storing redundant propagators
	      if (constraintsSize + reg_max_nb_tasks*4
		  > constraintLimit) {
		int newLimit = constraintsSize + reg_max_nb_tasks*4;
		if (constraintLimit > INITIALSIZE)
		  ::delete [] constraintsExtension;
		constraintsExtension = ::new int[newLimit];
		for (k=0; k<constraintsSize; k++)
		  constraintsExtension[k] = constraints[k];
		constraints = constraintsExtension;
		constraintLimit = newLimit;
	      }


	      struct Set *cset = &taskints[left][right];	
	      // test whether task interval is trivially empty
	      if (cset->extSize > 1) {
		int count_firsts = 0;
		int count_lasts = 0;
		int cdur = cset->dur;
		int up = cset->up;
		int low = cset->low;
		
		loc_resource_slack = intMin(loc_resource_slack, up-low-cdur);
		
		// compute firsts and lasts
	        for (l = 0; l < cset->extSize; l++) {
		  int task = cset->ext[l];
		  if ( (left != task)
		       && (all_vars[i][task].min <= up-cdur)
		       && (bm3.is(i, left, task) == 0) )
		    firsts[count_firsts++] = task;
		  if ( (right != task)
		       && (all_vars[i][task].max+all_durs[i][task] >= low + cdur)
		       && (bm3.is(i, task, right) == 0) )
		    lasts[count_lasts++] = task;
		}
	  
		// test whether firsts or lasts are empty
                if (count_firsts == 0) {
		  for (l=0; l < cset->extSize; l++) {
		    int task = cset->ext[l];
		    if ( (task != left) 
		      && (bm3.is(i, left, task) == 0) ) {
		      // task >= left + d(left)
		      constraints[constraintsSize] = 1;
		      constraints[constraintsSize+1] = i;
		      constraints[constraintsSize+2] = task;
		      constraints[constraintsSize+3] = left;
		      constraintsSize +=4;

		      FailOnEmpty(*all_fds[i][task] >= all_vars[i][left].min + all_durs[i][left]);
		      FailOnEmpty(*all_fds[i][left] <= all_vars[i][task].max - all_durs[i][left]);

		      OZ_Term left_side_task = reg_fds[resource_starts[i]
						      + task];
		      OZ_Term right_side_task = reg_fds[resource_starts[i] 
						       + left];
		      addImpose(fd_prop_bounds, left_side_task);
		      addImpose(fd_prop_bounds, right_side_task);
		      impose(new LessEqOffPropagator(right_side_task, left_side_task,
						    -all_durs[i][left]));

		    }
		  }
		  int value = up - cdur;
		  if (all_vars[i][left].max > value) {
		    // left =< up - cdur
		    FailOnEmpty(*all_fds[i][left] <= value);
		  }
		}
		else {
		  if (count_lasts == 0) {
		    for (l=0; l<cset->extSize; l++) {
		      int task = cset->ext[l];
		      if ( (task != right) 
			   && (bm3.is(i, task, right) == 0) ) {
			// task + d(task) =< right
			constraints[constraintsSize] = 0;
			constraints[constraintsSize+1] = i;
			constraints[constraintsSize+2] = task;
			constraints[constraintsSize+3] = right;
			constraintsSize +=4;


			FailOnEmpty(*all_fds[i][right] >= all_vars[i][task].min + all_durs[i][task]);
			FailOnEmpty(*all_fds[i][task] <= all_vars[i][right].max - all_durs[i][task]);

			OZ_Term left_side_task = reg_fds[resource_starts[i] 
							+ task];
			OZ_Term right_side_task = reg_fds[resource_starts[i] 
							 + right];
			addImpose(fd_prop_bounds, left_side_task);
			addImpose(fd_prop_bounds, right_side_task);
			
			impose(new LessEqOffPropagator(left_side_task, right_side_task,
						      -all_durs[i][task]));

		      }
		    }
		    // right >= low + cdur - d(right)
  	            int value = low + cdur - all_durs[i][right];
		    if (all_vars[i][right].min < value) {
		      FailOnEmpty(*all_fds[i][right] >= value);

		    }
		  }
		  else {
		    if ( (count_firsts > 0) && (count_lasts > 0) ) {
		      int slack = up - low - cdur;
		      int nc = intMin( count_firsts+1, count_lasts+1);
		      if (slack*nc < loc_best_slack_nc) {
			loc_best_set = &taskints[left][right];	
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
    

	// are all resources serialized?
	if (best_cost == OZ_getFDSup()) {
	  if (OZ_unify(new_out, OZ_int(-1)) == FAILED) // mm_u
	    goto failure;
	}
	else {
	  // compute firsts and lasts for the best task interval
 	  int count_firsts = 0;
	  int count_lasts = 0;
	  int cdur = best_set.dur;
	  int up = best_set.up;
	  int low = best_set.low;

	  for (l=0; l<best_set.extSize; l++) {
	    int task = best_set.ext[l];
	    if ( (best_left != task)
		 && (all_vars[best_resource][task].min <= up-cdur)
		 //&& (ordered[best_resource][best_left][task] == 0) )
		 && (bm3.is(best_resource, best_left, task) == 0) )
	      firsts[count_firsts++] = task;
	    if ( (best_right != task)
		 && (all_vars[best_resource][task].max + all_durs[best_resource][task] >= low + cdur)
		 //&& (ordered[best_resource][task][best_right] == 0) )	
		 && (bm3.is(best_resource,task,best_right) == 0) )
	     lasts[count_lasts++] = task;
	  }
  
	  
          // Remember the imposed redundant propagators
	  for (i = 0; i < constraintsSize / 4; i++){
	    int cur = i * 4;
	    int resource           = constraints[cur+1];
	    int left_side          = constraints[cur+2];
	    int right_side         = constraints[cur+3];
	    switch (constraints[cur]) {
	    case 0:
	      bm3.set(resource, left_side, right_side);
	      break;
	    case 1:
	      bm3.set(resource, right_side, left_side);			   
	      break;
	    }
	  }


	  // Compute the most promising pair of tasks
          // the used names are conform to Caseau's paper  and code
	  int test;
	  if (reg_flag == 0) test = (count_firsts < count_lasts);
	  else test = (count_firsts <= count_lasts);
	  if (test) {
	    int deltaS = OZ_getFDSup();
	    for (l=0; l < count_firsts; l++) 
	      deltaS = intMin(deltaS, all_vars[best_resource][firsts[l]].min);
	    deltaS -= low;
	    
	    int slackS = up - low - cdur;
	    
	    int bonus;
	    if (reg_flag == 0) 
	      bonus = funcF(slackS, deltaS, upper);
	    else 
	      bonus = funcFOpt(slackS, deltaS, upper);
	    
	    int p1 = 0;
	    int p2 = 0;
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
	      int v1, v2;
	      if (reg_flag == 0) {
		v1 = funcG(min_left, max_left, dur_left, min_l, max_l, dur_l, upper);
		v2 = intMin( funcG(min_l, max_l, dur_l, min_left, max_left, dur_left, upper), bonus);
		if (v1 > v2) {
		  side = 1; v = v1;
		}
		else {
		  side = 0; v = v2;
		}
	      }
	      else {
		v1 = funcGOpt(min_left, max_left, dur_left, min_l, max_l, dur_l, upper);
		v2 = intMax( funcGOpt(min_l, max_l, dur_l, min_left, max_left, dur_left, upper), bonus);
		if (v1 < v2) {
		  side = 1; v = v1;
		}
		else {
		  side = 0; v = v2;
		}
	      }
	      if (v < g_costs) {
		g_costs = v;
		p1 = best_left;
		p2 = firsts_l;
	      }
	    }
	    
	    int min_p1 = all_vars[best_resource][p1].min;
	    int max_p1 = all_vars[best_resource][p1].max;
	    int dur_p1 = all_durs[best_resource][p1];
	    int min_p2 = all_vars[best_resource][p2].min;
	    int max_p2 = all_vars[best_resource][p2].max;
	    int dur_p2 = all_durs[best_resource][p2];
	    
	    OZ_Term tmp_out = OZ_tuple(OZ_atom("#"), 3);
	    OZ_putArg(tmp_out, 0, OZ_int(best_resource));
	    
	    int left, right;
	    if (reg_flag == 0) {
	      left = funcG(min_p1, max_p1, dur_p1, min_p2, max_p2, dur_p2, upper);
	      right = funcG(min_p2, max_p2, dur_p2, min_p1, max_p1, dur_p1, upper);
	    }
	    else {
	      left = funcGOpt(min_p1, max_p1, dur_p1, min_p2, max_p2, dur_p2, upper);
	      right = funcGOpt(min_p2, max_p2, dur_p2, min_p1, max_p1, dur_p1, upper);
	    }

	    if (  left >= right) {
	      // return p1#p2
	      OZ_putArg(tmp_out, 1, OZ_int(p1));
	      OZ_putArg(tmp_out, 2, OZ_int(p2));
	      if (OZ_unify(new_out, tmp_out) == FAILED) // mm_u
		goto failure;
	    }
	    else {
	      // return p2#p1
	      OZ_putArg(tmp_out, 1, OZ_int(p2));
	      OZ_putArg(tmp_out, 2, OZ_int(p1));
	      if (OZ_unify(new_out, tmp_out) == FAILED) // mm_u
		goto failure;
	    }
	  }
	  else {
	    int deltaS = 0;
	    for (l=0; l<count_lasts;  l++) 
	      deltaS = intMax(deltaS, 
			      all_vars[best_resource][lasts[l]].max + all_durs[best_resource][lasts[l]]);
	    deltaS = up - deltaS;
	    
	    int slackS = up - low - cdur;
	    
	    int bonus;
	    if (reg_flag == 0)
	      bonus = funcF(slackS, deltaS, upper);
	    else 
	      bonus = funcFOpt(slackS, deltaS, upper);
	    
	    int p1 = 0;
	    int p2 = 0;
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
	      int v1, v2;
	      if (reg_flag == 0) {
		v1 = funcG(min_l, max_l, dur_l, min_right, max_right, dur_right, upper);
		v2 = intMin( funcG(min_right, max_right, dur_right, min_l, max_l, dur_l, upper), bonus);
		if (v1 > v2) {
		  side = 1; v = v1;
		}
		else {
		  side = 0; v = v2;
		}
	      }
	      else {
		v1 = funcGOpt(min_l, max_l, dur_l, min_right, max_right, dur_right, upper);
		v2 = intMax( funcGOpt(min_right, max_right, dur_right, min_l, max_l, dur_l, upper), bonus);
		if (v1 < v2) {
		  side = 1; v = v1;
		}
		else {
		  side = 0; v = v2;
		}
	      }
	      if (v < g_costs) {
		g_costs = v;
		p1 = lasts_l;
		p2 = best_right;
	      }
	    }
	    
	    int min_p1 = all_vars[best_resource][p1].min;
	    int max_p1 = all_vars[best_resource][p1].max;
	    int dur_p1 = all_durs[best_resource][p1];
	    int min_p2 = all_vars[best_resource][p2].min;
	    int max_p2 = all_vars[best_resource][p2].max;
	    int dur_p2 = all_durs[best_resource][p2];
	    
	    OZ_Term tmp_out = OZ_tuple(OZ_atom("#"), 3);
	    OZ_putArg(tmp_out, 0, OZ_int(best_resource));

	    int left, right;
	    if (reg_flag == 0) {
	      left = funcG(min_p1, max_p1, dur_p1, min_p2, max_p2, dur_p2, upper);
	      right = funcG(min_p2, max_p2, dur_p2, min_p1, max_p1, dur_p1, upper);
	    }
	    else {
	      left = funcGOpt(min_p1, max_p1, dur_p1, min_p2, max_p2, dur_p2, upper);
	      right = funcGOpt(min_p2, max_p2, dur_p2, min_p1, max_p1, dur_p1, upper);
	    }
	    if ( left >= right ) {
	      // return p1#p2
	      OZ_putArg(tmp_out, 1, OZ_int(p1));
	      OZ_putArg(tmp_out, 2, OZ_int(p2));
	      if (OZ_unify(new_out, tmp_out) == FAILED) // mm_u
		goto failure;
	    }
	    else {
	      // return p2#p1
	      OZ_putArg(tmp_out, 1, OZ_int(p2));
	      OZ_putArg(tmp_out, 2, OZ_int(p1));
	      if (OZ_unify(new_out, tmp_out) == FAILED) // mm_u
		goto failure;
	    }
	  }
	}

        :: delete [] best_set.ext;
	
      } 
      else {
	goto failure;
      }
      
    } else {
      goto failure;
    }
  }

  if (!st.isValid()) 
    goto failure;
  

  for (i=0; i<reg_nb_tasks_size; i++) {
      :: delete all_vars[i];
    }
  :: delete [] all_vars;

  for (i=0; i<reg_max_nb_tasks; i++) 
    for (j=0; j<reg_max_nb_tasks; j++) {
      :: delete [] taskints[i][j].ext;
    }

  for (i=0; i<reg_max_nb_tasks; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;


  if (constraintLimit > INITIALSIZE)
    :: delete [] constraintsExtension;

  stream = st.getTail();
  for (i = 0, k = 0; i < reg_nb_tasks_size; i += 1)
    for (j = 0; j < reg_nb_tasks[i]; j += 1)
      all_fds[i][j].leave();

  pe_var.leave();

  return st.leave() ? SLEEP : PROCEED;
  
failure:
  st.fail(); 

  for (i = 0, k = 0; i < reg_nb_tasks_size; i += 1)
    for (j = 0; j < reg_nb_tasks[i]; j += 1)
      all_fds[i][j].fail();

  for (i=0; i<reg_nb_tasks_size; i++) {
      :: delete all_vars[i];
    }
  :: delete [] all_vars;

  for (i=0; i<reg_max_nb_tasks; i++) 
    for (j=0; j<reg_max_nb_tasks; j++) {
      :: delete [] taskints[i][j].ext;
    }
  for (i=0; i<reg_max_nb_tasks; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;


  if (constraintLimit > INITIALSIZE)
    :: delete [] constraintsExtension;

  return FAILED;   
}


