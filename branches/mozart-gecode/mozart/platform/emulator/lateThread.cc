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

OZ_BI_define(BI_prop_gec, 0, 0) {
  //printf("Thread %p running at board %p\n",oz_currentThread(), oz_currentBoard());fflush(stdout);
  using namespace Gecode;
  Board *cb = oz_currentBoard();
  /* Curren board cannot be failed */
  Assert(!cb->isFailed());
  /* Current board must contains a not NULL generic space*/
  GenericSpace *gs = cb->getGenericSpace(true);
  Assert(gs);  
  
  /*
  	The generic space could not be failed, if it does then the execution
    of lateThread will result in an failure.
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
  printf("lateThread gs %p \n",gs);fflush(stdout);  
  Assert(!gs->isStable()); 
  if (gs->mstatus() == SS_FAILED) {
    cb->deleteGenericSpace();
    return FAILED;
  } 

  /*
  	After propagation the generic space is stable, if it is not 
	entailed, this thread must suspend.
  */
  //printf("lateThread gs 2 %p\n",gs);fflush(stdout);  
  if (!gs->isEntailed())
	return SUSPEND;
  
  // no variable left in gs, delete it and vanish
  if(gs->isSolved()) {   
      cb->deleteGenericSpace();
  }
  cb->deleteLateThread();
  return PROCEED;
} OZ_BI_end
   
