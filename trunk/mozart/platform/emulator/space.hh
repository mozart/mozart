/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Kostja Popow, 1998
 *    Michael Mehl, 1998
 *    Christian Schulte, 1998
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

#ifndef __SPACEHH
#define __SPACEHH

#include "base.hh"
#include "am.hh"
#include "board.hh"

Bool oz_installScript(Script &script);

InstType oz_installPath(Board *to);
void oz_reduceTrailOnSuspend();
void oz_reduceTrailOnFail();
void oz_reduceTrailOnEqEq();


/* -------------------------------------------------------------------------
 * TODO
 * ------------------------------------------------------------------------- */

inline
void oz_deinstallCurrent()
{
  oz_reduceTrailOnSuspend();
  am.setCurrent(oz_currentBoard()->getParent());
}


#endif
