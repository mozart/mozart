/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *     Raphael Collet, 2006
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *     Alejandro Arbelaez, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */


#include "am.hh"
#include "var_base.hh"
#include "GeSpace.hh"

//#define RANDOM_PROPAGATE 1

OZ_BI_define(BI_prop_gec, 0, 0) {
  //  printf("Thread %p running at board %p\n",oz_currentThread(), oz_currentBoard());fflush(stdout);
  //  printf("Thread genericspace running %p\n", oz_currentBoard()->getGenericSpace(true));fflush(stdout);
  using namespace Gecode;
  Board *cb = oz_currentBoard();
  /* Curren board cannot be failed */
  Assert(!cb->isFailed());
  /* Current board must contains a not NULL generic space*/
  GenericSpace *gs = cb->getGenericSpace(true);
  Assert(gs);  
  
  /*
    The generic space could not be failed, if it does then the
    execution of lateThread will result in an failure.
  */
  if (gs->failed()) { return FAILED; }

  /*
    This thread only runs when the status variable was needed,
    LateThread is only created runable when running at top level,
    otherwise it is created as a suspended thread and added to the
    status susp list.
  */
  TaggedRef status = cb->getStatus();
  DEREF(status, statusPtr); 
  Assert(oz_isNeeded(status) || cb->isRoot());
  Assert(!gs->isStable()); 

#ifdef RANDOM_PROPAGATE
  /*
    (ggutierrez): Produce a random number to execute or not a real
    propagation of the generic space. This code is temporal and has
    the purpose of testing the possibility of alternating propagation
    and thread executions. Fake propagation can be viewed as an
    intermediate state in which part of the propagation is run but not
    completely finished (preempted by the virtual machine).
  */
  double r = ((double)rand() / ((double)(RAND_MAX)+(double)(1)));
  long int M = 1000;
  double x = (r*M);
  // create a random integer in the range [0,M)
  int y = (int)x;
  
  if (y < M/3) {
    // The space is not going to be propagated. lateThread should
    // re-suspend on status.
    printf("Fake propagation\n");fflush(stdout);
    oz_var_addQuietSusp(statusPtr, oz_currentThread());
    return SUSPEND;
  } else {
#endif
  //printf("Real propagation\n");fflush(stdout); 
  if (gs->mstatus() == SS_FAILED) {
    cb->deleteGenericSpace();
    return FAILED;
  } 

  /*
    After propagation the generic space is stable, if it is not
    entailed, this thread must suspend.
  */
  if (!gs->isEntailed())
	return SUSPEND;
  
  // no variable left in gs, delete it and vanish
  if(gs->isSolved()) {   
      cb->deleteGenericSpace();
  }
  cb->deleteLateThread();
  return PROCEED;
#ifdef RANDOM_PROPAGATE
  }
#endif

} OZ_BI_end
   
