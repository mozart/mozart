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
