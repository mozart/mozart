/*
 * FBPS Saarbr"ucken
 * Last modified: $Date$ from $Author$
 * Version: $Revision$
 * State: $State$
 *
 * class Continuation: (PC,Y,G,[X])
 */

#if defined(INTERFACE)
#pragma implementation "cont.hh"
#endif

#include "types.hh"
#include "tagged.hh"
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

void Continuation::setY(RefsArray Y) {
  yRegs = Y;
#ifdef DEBUG_CHECK
  if (Y != (RefsArray) 0) {
    for (int i = 0; i < getRefsArraySize(Y); i++) {
      TaggedRef aux = Y[i];
      if (aux != (TaggedRef) 0) { DEREF(aux,_ptr,_tag); }
    }
  }
#endif
}

void Continuation::init(ProgramCounter p, RefsArray y, RefsArray g,
			RefsArray x, int i)
{
  pc = p;
  yRegs = y;
  gRegs = g;
  setX (x, i);


  DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));
#ifdef DEBUG_CHECK
  if (y != (RefsArray) 0) {
    for (int iii = 0; iii < getRefsArraySize(y); iii++) {
      TaggedRef aux = y[iii];
      if (aux != (TaggedRef) 0) { DEREF(aux,_ptr,_tag); }
    }
  }
#endif
}

Continuation::Continuation(ProgramCounter p, RefsArray y, RefsArray g,
			   RefsArray x, int i)
  : pc(p), yRegs(y), gRegs(g)
{
  setX (x, i);

  DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));
#ifdef DEBUG_CHECK
  if (y != (RefsArray) 0) {
    for (int iii = 0; iii < getRefsArraySize(y); iii++) {
      TaggedRef aux = y[iii];
      if (aux != (TaggedRef) 0) { DEREF(aux,_ptr,_tag); }
    }
  }
#endif
}
