/*
 * FBPS DFKI Saarbr"ucken
 * Author: mehl
 *
 * class Continuation: (PC,Y,G,[X])
 */


#ifndef __CONTHH
#define __CONTHH

#ifdef INTERFACE
#pragma interface
#endif

#include "tagged.hh"

class Continuation {
protected:
  ProgramCounter pc;
  RefsArray yRegs, gRegs, xRegs;
public:
  USEFREELISTMEMORY
  void gcRecurse(void);
  Continuation * gc();
  int32 *getGCField() { return (int32*) &pc; }

  Continuation(void)
  : pc(NOCODE), yRegs(NULL), gRegs(NULL) , xRegs(NULL) {}

  Continuation(ProgramCounter p, RefsArray y, RefsArray g=0,
               RefsArray x=0, int i=0)
    : pc(p), yRegs(y), gRegs(g)
  {
    setX (x, i);
  }

  void init(ProgramCounter p, RefsArray y, RefsArray g=0,
            RefsArray x=0, int i=0) {
    pc = p;
    yRegs = y;
    gRegs = g;
    setX (x, i);
  }

  ProgramCounter getPC(void)   { return pc; }
  void setPC(ProgramCounter p) { pc = p; }
  RefsArray getY(void)         { return yRegs; }
  void setY(RefsArray Y)       { yRegs = Y; }
  RefsArray getG(void)         { return gRegs; }
  void setG(RefsArray G)       { gRegs = G; }
  int getXSize(void)           { return xRegs ? getRefsArraySize(xRegs) : 0; }
  RefsArray getX(void)         { return xRegs; }
  void getX(RefsArray x) {
    if (xRegs) {
      int i = getRefsArraySize(xRegs);
      while ((--i) >= 0)
        x[i] = xRegs[i];
    }
  }

  void setX(RefsArray x, int i);

}; // Continuation

#endif
