/*
 *  Authors:
 *    Kostja Popov (kost@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$
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

//
// Just want to inline these methods..
inline
void TRAVERSERCLASS::traverse(OZ_Term t)
{
  Assert(tosNotRunning == (StackEntry *) 0);
  ensureFree(1);
  put(t);
  doit();
  if (tosNotRunning) {
    setTop(tosNotRunning);
    tosNotRunning = (StackEntry *) 0;
  }
  Assert(tosNotRunning == (StackEntry *) 0);
  // CrazyDebug(fprintf(stdout, " --- %d nodes.\n", debugNODES););
  // CrazyDebug(fflush(stdout););
}

//
inline
void TRAVERSERCLASS::resume()
{
  Assert(proc == (ProcessNodeProc) -1); // not used;
  Assert(tosNotRunning == (StackEntry *) 0);
  Assert(opaque != (Opaque *) -1);
  //
  doit();
  if (tosNotRunning) {
    setTop(tosNotRunning);
    tosNotRunning = (StackEntry *) 0;
  }
  Assert(tosNotRunning == (StackEntry *) 0);
}

//
inline
void TRAVERSERCLASS::resume(Opaque *o)
{
  Assert(proc == (ProcessNodeProc) -1); // not used;
  Assert(tosNotRunning == (StackEntry *) 0);
  Assert(opaque == (Opaque *) -1); // otherwise that's recursive;
  opaque = o;
  //
  doit();
  if (tosNotRunning) {
    setTop(tosNotRunning);
    tosNotRunning = (StackEntry *) 0;
  }
  Assert(tosNotRunning == (StackEntry *) 0);
}
