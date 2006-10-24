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
#include "schedulingDistAux.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// Firsts Lasts Get Candidates
//////////////////////////////////////////////////////////////////////

struct min_max_dur_set {
  int min, max, dur;
};
struct min_max_dur_setFL {
  int min, max, dur, id;
};

static inline int intMin(int a, int b) { return a < b ? a : b; }
static inline int intMax(int a, int b) { return a > b ? a : b; }

static int CompareFirsts(const void *x, const void *y)
{
  const min_max_dur_setFL *Int1 = (const min_max_dur_setFL*) x;
  const min_max_dur_setFL *Int2 = (const min_max_dur_setFL*) y;
  int min1 = Int1->min;
  int min2 = Int2->min;
  if (min1 < min2) return -1;
  else {
    if (min1 == min2) {
      return Int1->max - Int2->max;
    }
    else return 1;
  }
}

static int CompareLasts(const void *x, const void *y)
{
  const min_max_dur_setFL *Int1 = (const min_max_dur_setFL *) x;
  const min_max_dur_setFL *Int2 = (const min_max_dur_setFL *) y;
  int max1 = Int1->max;
  int max2 = Int2->max;
  int dur1 = Int1->dur;
  int dur2 = Int2->dur;
  if (max1+dur1 > max2+dur2) return -1;
  else {
    if (max1+dur1 == max2+dur2) {
      return Int2->min + dur2 - (Int1->min + dur1);
    }
    else return 1;
  }
}

//////////
// CONSTRUCTOR
//////////

FirstsLasts::FirstsLasts(OZ_Term tasks, OZ_Term start,
			       OZ_Term durs, OZ_Term st, int flag) 
{
  stream = st;
  int i,j;
  int current = 0;
  reg_fds_size = 0;
  reg_resource = -1;
  reg_max_nb_tasks = 0;

  int task_width        = OZ_width(tasks);
  reg_nb_tasks_size     = task_width;
  reg_nb_tasks          = OZ_hallocCInts(reg_nb_tasks_size);
  reg_ordered_resources = OZ_hallocCInts(reg_nb_tasks_size);

  for (i=0; i<task_width; i++) {
    int cwidth = OZ_width(OZ_getArg(tasks, i));
    reg_nb_tasks[i] = cwidth;
    reg_ordered_resources[i] = 0;
    reg_max_nb_tasks = intMax(cwidth, reg_max_nb_tasks);
    reg_fds_size += cwidth;
  }

  reg_fds      = OZ_hallocOzTerms(reg_fds_size);
  reg_ordered  = OZ_hallocCInts(reg_fds_size);

  reg_durs = ((int **)::new int*[reg_nb_tasks_size + 1]) + 1;
  for (i=0; i<task_width; i++) {
    reg_durs[i] = ::new int[reg_nb_tasks[i]];
  }
  reg_durs[-1] = (int *) 0;


  for (i=0; i<task_width; i++) {
    OZ_Term ctasks = OZ_getArg(tasks, i);
    for (j=0; j<reg_nb_tasks[i]; j++) {
      OZ_Term ctask        = OZ_getArg(ctasks, j);
      reg_ordered[current] = 0;
      reg_fds[current++]   = OZ_subtree(start, ctask);
      reg_durs[i][j]     = OZ_intToC(OZ_subtree(durs, ctask));
    }
  }

  reg_flag = flag;

}

//////////
// DESTRUCTOR
//////////

FirstsLasts::~FirstsLasts()
{

  if (int(reg_durs[-1]) == 0) {
    for (int i=0; i<reg_nb_tasks_size; i++) {
      :: delete [] reg_durs[i];
    }
    reg_durs--;
    :: delete [] reg_durs;
  } else {
    reg_durs[-1] = (int *) int(reg_durs[-1]) - 1;
  }

}

//////////
// BUILTIN
//////////
OZ_BI_define(sched_firstsLasts, 5, 0)
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

  return pe.impose(new FirstsLasts(OZ_in(0), OZ_in(1), OZ_in(2), 
				      OZ_in(3), OZ_intToC(OZ_in(4))));
}
OZ_BI_end

//////////
// COPYING
//////////

void FirstsLasts::gCollect(void) {
  OZ_gCollectTerm(stream);

  reg_fds      = OZ_gCollectAllocBlock(reg_fds_size, reg_fds);
  reg_ordered  = OZ_copyCInts(reg_fds_size, reg_ordered);
  reg_nb_tasks = OZ_copyCInts(reg_nb_tasks_size, reg_nb_tasks);
  reg_ordered_resources = OZ_copyCInts(reg_nb_tasks_size, 
				       reg_ordered_resources);

}

void FirstsLasts::sClone(void) {
  reg_durs[-1] = (int *) int(reg_durs[-1]) + 1;

  OZ_sCloneTerm(stream);

  reg_fds      = OZ_sCloneAllocBlock(reg_fds_size, reg_fds);
  reg_ordered  = OZ_copyCInts(reg_fds_size, reg_ordered);
  reg_nb_tasks = OZ_copyCInts(reg_nb_tasks_size, reg_nb_tasks);
  reg_ordered_resources = OZ_copyCInts(reg_nb_tasks_size, 
				       reg_ordered_resources);

}

//////////
// SPAWNER
//////////

OZ_PropagatorProfile FirstsLasts::profile;


//////////
// RUN METHOD
//////////

OZ_Return FirstsLasts::propagate(void) 
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_Stream st(stream);

  /////////////
  //  Some data structures
  ////////////

  int i, j, k, l, left, right;
  int sumDur = 0;

  DECL_DYN_ARRAY(min_max_dur_setFL, firsts, reg_max_nb_tasks);
  DECL_DYN_ARRAY(min_max_dur_setFL, lasts, reg_max_nb_tasks);

  DECL_DYN_ARRAY(int, resource_starts, reg_nb_tasks_size);
  j = 0;
  for (i = 0; i < reg_nb_tasks_size; i++) {
    resource_starts[i] = j;
    j = j + reg_nb_tasks[i];
  }

  DECL_DYN_ARRAY(OZ_FDIntVar *, xptr, reg_nb_tasks_size);
  FDIntVarArr2 all_fds(reg_nb_tasks_size, xptr, reg_nb_tasks);

  int currentRes = -1;
  int current_tasks = -1;

  int fd_sup = OZ_getFDSup();

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
	     
        int task = -1;
	if (OZ_isTuple(old_out) && ! OZ_isLiteral(old_out)) {
	  const char * old_label = OZ_atomToC(OZ_label(old_out));
	  // fill in order
          if (!strcmp("#", old_label)) {
	    task = OZ_intToC( OZ_getArg(old_out, 0));
	  }
	  else {
	    goto failure;
	  }
	}
	else {
	  if ((OZ_isAtom(old_out)) && (!strcmp("nil", OZ_atomToC(OZ_label(old_out))))) {}
	  else {
	    goto failure;
	  }
	}

	int best_resource = -1;
	int best_slack = fd_sup;
	DECL_DYN_ARRAY(int, considered_resources, reg_nb_tasks_size);	
	int nb_considered_resources = 0;
	int best_global_slack = fd_sup;

	// find the resource or mark it only
        if (reg_resource == -1) {
	  // first call;
	  goto findResource;
	}
	else {
	  // mark task ordered which was delived by Oz through the stream
	  int rStart = resource_starts[reg_resource];
	  reg_ordered[rStart + task] = 1;
	  // test whether the current resource is serialized
	  int finished = 0;
	  for (i=0; i<reg_nb_tasks[reg_resource]; i++)
	    if (reg_ordered[rStart + i] == 0) finished = 1;
	  if (finished == 0) {
	    // find a new resource to serialize
	    reg_ordered_resources[reg_resource] = 1;
	    reg_resource = -1;
	    goto findResource;
	  }
	  else {
	    // consider already chosen resource
	    int rStart = resource_starts[reg_resource];
	    for (j = 0; j < reg_nb_tasks[reg_resource]; j++)
	      all_fds[reg_resource][j].read(reg_fds[rStart+j]);
	    considered_resources[nb_considered_resources++] = reg_resource;
	    goto afterFind;
	  }
	}


      findResource:

	// if only one resource available, take this without search!

	if ((reg_nb_tasks_size == 1) && (reg_ordered_resources[0] == 0)){
	  reg_resource = 0;
	  int ct = reg_nb_tasks[0];
	  int rStart = resource_starts[0];
	  for (j = 0; j < ct; j++)
	    all_fds[0][j].read(reg_fds[rStart+j]);
	  considered_resources[nb_considered_resources++] = 0;
	  goto afterFind;
	}

	// test all resources and find the best one
	for (i=0; i<reg_nb_tasks_size; i++) {
	  if (reg_ordered_resources[i] == 0) {
	    // not ordered yet
	    int current_slack = fd_sup;
	    int ct = reg_nb_tasks[i];
	    int rStart = resource_starts[i];
	    for (j = 0; j < ct; j++)
	      all_fds[i][j].read(reg_fds[rStart+j]);
	    considered_resources[nb_considered_resources++] = i;

	    int globmin = fd_sup;
	    int globmax = 0;
	    int globdur = 0;
	    // compute local slacks
	    for (j=0; j<ct; j++) {
	      int jMin = all_fds[i][j]->getMinElem();
	      int jDue = all_fds[i][j]->getMaxElem() + reg_durs[i][j];
	      globdur = globdur + reg_durs[i][j];
	      if (jMin < globmin) globmin = jMin;
	      if (jDue > globmax) globmax = jDue;
	      for (k=0; k<ct; k++) {
		int kMin = all_fds[i][k]->getMinElem();
		int kDue = all_fds[i][k]->getMaxElem() + reg_durs[i][k];
		if ( (jMin <= kMin) && (jDue <= kDue) ) {
		  int demand = 0;
		  for (l=0; l<ct; l++) {
		    int lMin = all_fds[i][l]->getMinElem();
		    int lDue = all_fds[i][l]->getMaxElem() + reg_durs[i][l];
		    if ( (jMin <= lMin) && (lDue <= kDue) )
		      demand = demand + reg_durs[i][l];
		  }
		  int supply = kDue - jMin;
		  if (supply - demand < current_slack){
		    current_slack = supply - demand;
		  }
		}
	      }
	    }
	    if (current_slack < best_slack) {
	      best_slack        = current_slack;
	      best_resource     = i;
	      best_global_slack = globmax - globmin - globdur;
	    }
	    else {
	      if ((current_slack == best_slack) &&
		  (globmax - globmin - globdur < best_global_slack)) {
		best_slack        = current_slack;
		best_resource     = i;
		best_global_slack = globmax - globmin - globdur;
	      }
	    }



	  }
	}
	if (best_resource == -1) {
	}
	else {
	  reg_resource = best_resource;
	}
	
      afterFind:


	if (reg_resource == -1) {
	  // all resources are serialized
	  if (OZ_unify(new_out, OZ_atom("finished")) == FAILED) // mm_u
	    goto failure;
	}
	else {
	  // Compute auxiliary data structures for firsts/lasts computation
	  int max1 = -1;
	  int max2 = -1;
	  int min1 = fd_sup;
	  int min2 = fd_sup;
	  int rStart = resource_starts[reg_resource];
	  current_tasks = reg_nb_tasks[reg_resource];
	  currentRes = reg_resource;
	  DECL_DYN_ARRAY(min_max_dur_set, all_tasks, current_tasks);
	  for (j=0; j < current_tasks; j++) {
	    if (reg_ordered[rStart + j] == 0) {
	      int cmin = all_fds[currentRes][j]->getMinElem();
	      int cmax = all_fds[currentRes][j]->getMaxElem();
	      int cdur = reg_durs[currentRes][j];
	      int cdue = cmax + cdur;
	      all_tasks[j].min = cmin;
	      all_tasks[j].max = cmax;
	      all_tasks[j].dur = cdur;
	      sumDur           = sumDur + reg_durs[currentRes][j];
	      if (cdue > max1) {
		max2 = max1;
		max1 = cdue;
	      }
	      else {
		if (cdue > max2) 
		  max2 = cdue;
	      }
	      if (cmin < min1 ) {
		min2 = min1;
		min1 = cmin;
	      }
	      else {
		if (cmin < min2)
		  min2 = cmin;
	      }
	    }
	  }
	  
	  
	  // compute firsts and lasts in linear time
	  int number_of_firsts = 0;
	  int number_of_lasts  = 0;
	  int how_many         = 0;
	  int first_valid      = -1;

	  for (i=0; i<current_tasks; i++) {
	    if (reg_ordered[rStart + i] == 0) {
	      first_valid = i;
	      how_many++;
	      int cmin = all_tasks[i].min;
	      int cmax = all_tasks[i].max;
	      int cdur = all_tasks[i].dur;
	      int cdue = cmax + cdur;
	      int up = 0;
	      int down = 0;
	      if (cdue == max1) up = max2;
	      else up = max1;
	      if (up - cmin >= sumDur) {
		firsts[number_of_firsts].id = i;
		firsts[number_of_firsts].min = all_tasks[i].min;
		firsts[number_of_firsts].max = all_tasks[i].max;
		firsts[number_of_firsts].dur = all_tasks[i].dur;
		number_of_firsts++;
	      }
	      if (cmin == min1) down = min2;
	      else down = min1;
	      if (cmax + cdur - down >= sumDur) {
		lasts[number_of_lasts].id = i;
		lasts[number_of_lasts].min = all_tasks[i].min;
		lasts[number_of_lasts].max = all_tasks[i].max;
		lasts[number_of_lasts].dur = all_tasks[i].dur;
		number_of_lasts++;
	      }
	    }
	  }


	  OZ_Term nil = OZ_nil();
	  OZ_Term ret = nil;
	  OZ_Term ret2 = nil;
	  OZ_Term tmp_out = OZ_tuple(OZ_atom("#"), 4);
	  
	  if ( (how_many > 1) && ( (number_of_lasts==0) || 
				   (number_of_firsts==0) ) )
	    {
	      // we need at least one first and one last
	      goto failure;
	    }
	  else {
	    for (i=0; i< current_tasks; i++) {
	      if (reg_ordered[rStart + i]==0) {
		OZ_Term task1 = OZ_int(i);  
		ret2 = OZ_cons(task1, ret2);
	      }
	    }

	    if (how_many == 1) goto imposeOne; 
	    else if ((reg_flag == 0) && (number_of_lasts < number_of_firsts))
	      goto imposeLasts;
	    else if ((reg_flag == 0) && (number_of_lasts > number_of_firsts))
	      goto imposeFirsts;
	    else if (number_of_firsts == 1) goto imposeFirsts;
	    else if (number_of_lasts == 1) goto imposeLasts;
	    else if (reg_flag == 1)  goto imposeFirsts;
	    else if (reg_flag == 2)  goto imposeLasts;
	    else {
	      int min1, min2;
	      if (CompareFirsts( & (firsts[0]), & (firsts[1])) == -1) {
		min1 = 0; min2 = 1;
	      }
	      else {
		min1 = 1; min2 = 0;
	      }
	      int max1, max2;
	      if (CompareLasts( & (lasts[0]), & (lasts[1])) == -1) {
		max1 = 0; max2 = 1;
	      }
	      else {
		max1 = 1; max2 = 0;
	      }
	      for (i=2; i<number_of_firsts; i++)
		if (CompareFirsts( & (firsts[i]), & (firsts[min1])) == -1) {
		  min2 = min1;
		  min1 = i;
		}
		else {
		  if (CompareFirsts( & (firsts[i]), &(firsts[min2])) == -1) {
		    min2 = i;
		  }
		}
	      for (i=2; i<number_of_lasts; i++)
		if (CompareLasts(&(lasts[i]), &(lasts[max1])) == -1) {
		  max2 = max1;
		  max1 = i;
		}
		else {
		  if (CompareLasts(&(lasts[i]), &(lasts[max2])) == -1) {
		    max2 = i;
		  }
		}
	      int diff1 = ozabs(firsts[min1].min - firsts[min2].min);
	      int diff2 = ozabs(lasts[max1].max + lasts[max1].dur - 
				lasts[max2].max - lasts[max2].dur);
	      if (diff1 > diff2)
		goto imposeFirsts;
	      else goto imposeLasts;
	    }
	  }
	    
	imposeOne:
	  ret = OZ_cons(OZ_int(first_valid), nil);
	  OZ_putArg(tmp_out, 0, OZ_atom("firsts"));
	  OZ_putArg(tmp_out, 1, ret);
	  OZ_putArg(tmp_out, 2, ret2);
	  OZ_putArg(tmp_out, 3, OZ_int(reg_resource));
	  if (OZ_unify(new_out, tmp_out) == FAILED) { // mm_u
	    goto failure;
	  }
	  goto leave;
	imposeLasts:
	  for (i=number_of_lasts-1; i>=0; i--) {
	    OZ_Term task1 = OZ_int(lasts[i].id);  
	    ret = OZ_cons(task1, ret);
	  }
	  OZ_putArg(tmp_out, 0, OZ_atom("lasts"));
	  OZ_putArg(tmp_out, 1, ret);
	  OZ_putArg(tmp_out, 2, ret2);
	  OZ_putArg(tmp_out, 3, OZ_int(reg_resource));
	  if (OZ_unify(new_out, tmp_out) == FAILED) { // mm_u
	    goto failure;
	  }
	  goto leave;
	  
	imposeFirsts:
	  for (i=number_of_firsts-1; i>=0; i--) {
	    OZ_Term task1 = OZ_int(firsts[i].id);  
	    ret = OZ_cons(task1, ret);
	  }
	  OZ_putArg(tmp_out, 0, OZ_atom("firsts"));
	  OZ_putArg(tmp_out, 1, ret);
	  OZ_putArg(tmp_out, 2, ret2);
	  OZ_putArg(tmp_out, 3, OZ_int(reg_resource));
	  if (OZ_unify(new_out, tmp_out) == FAILED) { // mm_u
	    goto failure;
	  }
	}
      leave:	
	for (i=0; i<nb_considered_resources; i++) {
	  int resource = considered_resources[i];
	  for (j = 0; j < reg_nb_tasks[resource]; j++)
	    all_fds[resource][j].leave();
	}
	  

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
  
  stream = st.getTail();

  return st.leave() ? SLEEP : PROCEED;
  
failure:
  st.fail(); 

  if (currentRes > 0) 
    for (j = 0; j < current_tasks; j++)
      all_fds[currentRes][j].fail();

  return FAILED;   
}
