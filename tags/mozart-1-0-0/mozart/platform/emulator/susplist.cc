/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
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

#if defined(INTERFACE)
#pragma implementation "susplist.hh"
#endif

#include "susplist.hh"
#include "board.hh"

#ifdef OUTLINE
#define inline
#endif

//-----------------------------------------------------------------------------
//                          class SuspList

int SuspList::length(void)
{
  int i=0;
  for(SuspList * aux = this; aux != NULL; aux = aux->getNext()) {
    if (!aux->getElem().isDead() &&
	!aux->getElem().isRunnable() && 
	GETBOARDOBJ(aux->getElem())) {
      i++;
    }
  }
  return i;
} 

int SuspList::lengthProp(void)
{
  int i=0;
  for(SuspList * aux = this; aux != NULL; aux = aux->getNext()) {
    if (!aux->getElem().isDead() &&
	aux->getElem().isRunnable() &&
	GETBOARDOBJ(aux->getElem())) {
      i++;
    }
  }
  return i;
}

//-----------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "susplist.icc"
#undef inline
#endif
