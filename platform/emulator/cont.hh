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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
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
  RefsArray xRegs,yRegs;
  Abstraction *cap;
public:
  USEFREELISTMEMORY

  Continuation(void)
  : pc(NOCODE), xRegs(NULL), yRegs(NULL) , cap(NULL) {}

  Continuation(ProgramCounter p, RefsArray y, Abstraction *cap=0,
               RefsArray x=0, int i=0)
    : pc(p), yRegs(y), cap(cap)
  {
    setX (x, i);
  }

  void init(ProgramCounter p, RefsArray y, Abstraction *CAP=0,
            RefsArray x=0, int i=0) {
    pc = p;
    yRegs = y;
    cap = CAP;
    setX (x, i);
  }

  void gc();

  ProgramCounter getPC(void)   { return pc; }
  void setPC(ProgramCounter p) { pc = p; }
  RefsArray getY(void)         { return yRegs; }
  void setY(RefsArray Y)       { yRegs = Y; }
  Abstraction *getCAP(void)    { return cap; }
  void setCAP(Abstraction *CAP)  { cap = CAP; }
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
