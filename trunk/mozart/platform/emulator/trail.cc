/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifdef __GNUC__
#pragma implementation "trail.hh"
#endif

#include "trail.hh"

void Trail::resize(int newSize) 
{
  message("resizing trail from %d to %d\n",size,newSize);
  int oldMark = tos-lastMark;
  Stack::resize(newSize);
  lastMark = tos-oldMark;
}
