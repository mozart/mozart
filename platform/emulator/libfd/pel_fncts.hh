/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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
 *     http://www.mozart-oz.org/
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __PEL_FNCTS_HH__
#define __PEL_FNCTS_HH__

#include "pel_engine.hh"

//-----------------------------------------------------------------------------

// X + C <= Y
template <class ENGINE, class FDVAR, class PFDVAR>
class PEL_LessEqOffset : public PEL_Propagator {
protected:
  int _x, _y, _c;
public:
  PEL_LessEqOffset(PFDVAR &x, int c,
                   PFDVAR &y)
    :  _c(-c) {
    _x = x.newId(*_pe);
    _y = y.newId(*_pe);
    CDM(("constr lesseqoff _x=%d _y=%d _c=%d\n", _x, _y, _c));
  }
  //
  void print(ENGINE &e) {
    printf("LessEqOffset x(%s,%d) + c(%d) <= ",
           (*(FDVAR *) e[_x])->toString(), _x, _c);
    printf("y(%s,%d)\n", (*(FDVAR *) e[_y])->toString(), _y);
  }
  //
  virtual pf_return_t propagate(PEL_Engine &e);
};

template <class ENGINE, class PFDVAR, class FDVAR>
void make_PEL_LessEqOffset(ENGINE &engine,
                           PFDVAR &x, int c,
                           PFDVAR &y, FDVAR * dummy)
{
  engine.expectIntVarBounds(x);
  engine.expectIntVarBounds(y);
  engine.impose(new PEL_LessEqOffset<ENGINE,FDVAR,PFDVAR>(x, c, y));
}

// X + C > Y == Y - C - 1 <= X
template <class ENGINE, class FDVAR, class PFDVAR>
class PEL_GreaterOffset : public PEL_LessEqOffset<ENGINE,FDVAR,PFDVAR> {
public:
  PEL_GreaterOffset(PFDVAR &x, int c,
                    PFDVAR &y)
    : PEL_LessEqOffset<ENGINE,FDVAR,PFDVAR>(y, -c+1, x) {}
  //
  void print(ENGINE &e) {
    printf("GreaterOffset x(%s,%d) + c(%d) > ",
           (*(FDVAR *) e[_y])->toString(), _y, -_c+1);
    printf("y(%s,%d)\n", (*(FDVAR *) e[_x])->toString(), _x);
  }
};

template <class ENGINE, class PFDVAR, class FDVAR>
void make_PEL_GreaterOffset(ENGINE &engine,
                            PFDVAR &x, int c,
                            PFDVAR &y, FDVAR * dummy)
{
  engine.expectIntVarBounds(x);
  engine.expectIntVarBounds(y);
  engine.impose(new PEL_GreaterOffset<ENGINE,FDVAR,PFDVAR>(x, c, y));
}

#endif /* __PEL_FNCTS_HH__ */
