/*
 *  Authors:
 *    Author's name (Author's email address)
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
/*
 * FBPS DFKI, Saarbr"ucken
 * Author: mehl
 *
 * class Continuation: (PC,Y,G,[X])
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
