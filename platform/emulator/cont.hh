/*
 * FBPS Saarbr"ucken
 * Last modified: $Date$ from $Author$
 * Version: $Revision$
 * State: $State$
 *
 * class Continuation: (PC,Y,G,[X])
 */


#ifndef __CONTHH
#define __CONTHH

#ifdef INTERFACE
#pragma interface
#endif

class Continuation {
protected:
  ProgramCounter pc;
  RefsArray yRegs, gRegs, xRegs;
public:
  USEFREELISTMEMORY

  Continuation(void)
  : pc(NOCODE), yRegs(NULL), gRegs(NULL) , xRegs(NULL) {}
  Continuation(ProgramCounter p, RefsArray y, RefsArray g=0,
               RefsArray x=0, int i=0);

  void init (ProgramCounter p, RefsArray y, RefsArray g=0,
             RefsArray x=0, int i=0);

  void defeat () {
    pc = NOCODE;
    yRegs = NULL;
    gRegs = NULL;
    xRegs = NULL;
    //  ... BTW, telemetric data tells us that this code
    // is never  used ;-))
  }

  void disposeRegs () {
    //  kost@ : TODO ?!!
    // RS tells me that "that's at your own risk!" ;-)
    // freeListDispose (xRegs, getRefsArraySizE (xRegs));
    // freeListDispose (yRegs, getRefsArraySizE (yRegs));
    // freeListDispose (zRegs, getRefsArraySizE (zRegs));
  }

  void dispose () {
    disposeRegs ();
    //  kost@ : Is this really needed? ...
    freeListDispose (this, sizeof (*this));
  }

  ProgramCounter getPC(void) { return pc;}

  void setPC(ProgramCounter p) { pc = p;}

  RefsArray getY(void) { return yRegs;}
  void setY(RefsArray Y);

  RefsArray getG(void) {return gRegs;}
  void setG(RefsArray G) { gRegs = G;}

  int getXSize(void) {return xRegs ? getRefsArraySize(xRegs) : 0;}
  RefsArray getX(void) { return xRegs;}
  void getX(RefsArray x) {
    if (xRegs) {
      int i = getRefsArraySize(xRegs);
      while ((--i) >= 0)
        x[i] = xRegs[i];
    }
  }

  void Continuation::setX(RefsArray x, int i);

  void gcRecurse(void);
  Continuation * gc();
}; // Continuation

#endif
