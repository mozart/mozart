/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#include "prop_int.hh"
#include "os.hh"
#include "value.hh"
#include "suspendable.hh"

SuspList * oz_installPropagators(SuspList * local_list, SuspList * glob_list,
				 Board * glob_home)
{
  Assert((local_list && glob_list && (local_list != glob_list)) || 
	 !local_list || !glob_list);

  SuspList * aux = local_list, * ret_list = local_list;

  
  // mark up local suspensions to avoid copying them
  while (aux) {
    aux->getSuspendable()->setTagged();
    aux = aux->getNext();
  }

  glob_home = glob_home->derefBoard();

  // create references to suspensions of global variable
  aux = glob_list;
  while (aux) {
    Suspendable * susp = aux->getSuspendable();
    
    /* NOTE: a possible optimization isTaggedAndUntag (tmueller!) */
	
    if (!susp->isDead() && 
	susp->isPropagator() &&
	!susp->isTagged() && 
	oz_isBetween(susp->getBoardInternal(), glob_home) == B_BETWEEN) {
      ret_list = new SuspList(susp, ret_list);
    }
    
    aux = aux->getNext();
  }

  // unmark local suspensions 
  aux = local_list;
  while (aux) {
    aux->getSuspendable()->unsetTagged();
    aux = aux->getNext();
  }
  
  return ret_list;
}




// Builtin that runs the propagators

OZ_BI_define(BI_prop_lpq, 0, 0) {

  return oz_currentBoard()->scheduleLPQ();

} OZ_BI_end

