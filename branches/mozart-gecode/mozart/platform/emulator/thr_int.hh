/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#ifndef __TIHH
#define __TIHH

#include "base.hh"
#include "suspendable.hh"

Thread * oz_newThread(int prio=DEFAULT_PRIORITY);

Thread * oz_newThreadToplevel(void);

Thread * oz_newThreadInject(Board *bb);

Thread * oz_newThreadSuspended(int prio=DEFAULT_PRIORITY);

Thread * oz_newThreadSuspended(Board* bb, int prio=DEFAULT_PRIORITY);

Thread * oz_newThreadPropagate(Board *bb);

void oz_disposeThread(Thread *tt);

void oz_wakeupThread(Thread *tt);

#endif
