/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997)
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

#if defined(INTERFACE)
#pragma implementation "cont.hh"
#endif

#include "cont.hh"

void Continuation::setX(RefsArray x, int i)
{
  if (i <= 0 || x == NULL) {
    xRegs = NULL;
  } else {
    xRegs = allocateRefsArray(i,NO);
    while ((--i) >= 0) {
      Assert(MemChunks::isInHeap(x[i]));
      xRegs[i] = x[i];
    }
  }
}
