/*
 *  Authors:
 *    Michael Mehl <mehl@dfki.de>
 *
 *  Copyright:
 *    Michael Mehl (1998)
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


/* The Oz machine level debugger */

#ifndef __TRACEH
#define __TRACEH

#ifdef DEBUG_TRACE
#include "base.hh"

Bool ozd_trace(const char *s,
	       ProgramCounter PC=NOCODE,RefsArray Y=NULL,Abstraction *G=NULL);
void ozd_tracerOn();
void ozd_tracerOff();

#endif

#endif /* __TRACEH */
