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
#include "rel_filter.hh"

//-----------------------------------------------------------------------------

// X + C <= Y
template <class ENGINE, class FDVAR, class PFDVAR>
class PEL_LessEqOffset : public PEL_Propagator {
protected:
  int _x, _y, _c;
public:
  PEL_LessEqOffset(PFDVAR &x, int c,
                   PFDVAR &y)
    :  _c(-c), _x(x.newId(*_pe)), _y(y.newId(*_pe))
  {
    CDM(("constr lesseqoff _x=%d _y=%d _c=%d\n", _x, _y, _c));
  }
  //
  void print(ENGINE &e) {
    // kost@ : this code does NOT instantiate!
    // I've simplified it (below);
    /*
    printf("LessEqOffset x(%s,%d) + c(%d) <= ",
           (*(FDVAR *) e[_x])->toString(), _x, _c);
    printf("y(%s,%d)\n", (*(FDVAR *) e[_y])->toString(), _y);
    */
    printf("LessEqOffset x(,%d) + c(%d) <= ", _x, _c);
    printf("y(,%d)\n", _y);
  }
  //
  virtual pf_return_t propagate(PEL_Engine &e);
};

template <class ENGINE, class PFDVAR, class FDVAR>
void make_PEL_LessEqOffset(ENGINE &engine,
                           PFDVAR &x, int c,
                           PFDVAR &y)
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
    // kost@ : this code does NOT instantiate!
    // I've simplified it (below);
    /*
    printf("GreaterOffset x(%s,%d) + c(%d) > ",
           (*(FDVAR *) e[_y])->toString(), _y, -_c+1);
    printf("y(%s,%d)\n", (*(FDVAR *) e[_x])->toString(), _x);
    */
    printf("GreaterOffset x(,%d) + c(%d) > ", this->_y, -(this->_c)+1);
    printf("y(,%d)\n", this->_x);
  }
};

template <class ENGINE, class PFDVAR, class FDVAR>
void make_PEL_GreaterOffset(ENGINE &engine,
                            PFDVAR &x, int c,
                            PFDVAR &y)
{
  engine.expectIntVarBounds(x);
  engine.expectIntVarBounds(y);
  engine.impose(new PEL_GreaterOffset<ENGINE,FDVAR,PFDVAR>(x, c, y));
}

template <class FDVAR>
class PEL_Filter {
private:
  int _r;
  _PropagatorController_V_V<int,
    FDVAR,pf_entailed,pf_failed,pf_sleep> * _iter;
public:
  PEL_Filter(_PropagatorController_V_V<int,
    FDVAR,pf_entailed,pf_failed,pf_sleep> &iter) : _iter(&iter), _r(-1) {}
  PEL_Filter &entail(void) {
    CDM(("PEL_Filter::entail()\n"));
    _r = _iter->vanish();
    return *this;
  }
  PEL_Filter &leave(int vars_left = 0) {
    CDM(("PEL_Filter::leave()\n"));
    _r = _iter->leave(vars_left);
    return *this;
  }
  PEL_Filter &fail(void) {
    CDM(("PEL_Filter::fail()\n"));
    _r = _iter->fail();
    return *this;
  }
  pf_return_t operator ()() { return pf_return_t(_r); }
};

template <class ENGINE, class FDVAR, class PFDVAR>
pf_return_t PEL_LessEqOffset<ENGINE, FDVAR, PFDVAR>::propagate(PEL_Engine &e)
{
  //
  FDVAR &x = *(FDVAR *) e[this->_x];
  int c = this->_c;
  FDVAR &y = *(FDVAR *) e[this->_y];
  //
  _PropagatorController_V_V<int,
    FDVAR,pf_entailed,pf_failed,pf_sleep> iter(x, y);
  PEL_Filter<FDVAR> s(iter);
  //
  return filter_lessEqOffset(s, x, y, c)();
}

#endif /* __PEL_FNCTS_HH__ */
