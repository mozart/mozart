/*
 *  Authors:
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */


/* The Oz machine level debugger */

#ifndef __TRACEH
#define __TRACEH

#ifdef DEBUG_TRACE
Bool ozd_trace(char *s,
               ProgramCounter PC=NOCODE,RefsArray Y=NULL,Abstraction *G=NULL);
void ozd_tracerOn();
void ozd_tracerOff();

OZ_C_proc_proto(BIhalt);
#endif

#endif /* __TRACEH */
