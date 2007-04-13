/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *
 *  Copyright:
 *     Raphael Collet, 2006
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
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
  
  /* Curren board cannot be failed */
  Assert(!oz_currentBoard()->isFailed());
  /* Current board must contains a not NULL generic space*/
  GenericSpace *gs = oz_currentBoard()->getGenericSpace(true);
  Assert(gs);  
  
  /* The generic space could not be failed, if it does then the execution
     of lateThread will result in an failure.
  */
  if (gs->failed()) { return FAILED; }

  using namespace Gecode;
  if (!oz_currentBoard()->isRoot()) {  
    // first wait until status is needed
    TaggedRef status = oz_currentBoard()->getStatus();
    DEREF(status, statusPtr); 
    if (!oz_isNeeded(status)) 
      return oz_var_addQuietSusp(statusPtr, oz_currentThread());      
  }
  
  
  if (gs->mstatus() == SS_FAILED) {
    oz_currentBoard()->deleteGenericSpace();
    return FAILED;
  } 
  printf("lateThread %p is entailed?? %d\n",gs,gs->isEntailed());fflush(stdout);
  if (!gs->isEntailed()) {
      TaggedRef t =  gs->getTrigger();
    DEREF(t,t_ptr);
    return oz_var_addSusp(t_ptr, oz_currentThread());
  }
  
  // no variable left in gs, delete it and vanish
  if(gs->isSolved())
    oz_currentBoard()->deleteGenericSpace();
  oz_currentBoard()->deleteLateThread();
  return PROCEED;
} OZ_BI_end
   
