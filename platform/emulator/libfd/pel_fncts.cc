/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
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

#include "fdaux.hh"
#include "rel_filter.hh"
#include "pel_fncts.hh"

//#define NEW

typedef  _PropagatorController_V_V<int,
  PEL_FDIntVar,pf_entailed,pf_failed,pf_sleep> PEL_ParamIterator_V_V;

class PEL_Service {
private:
  int _r;
  PEL_ParamIterator_V_V * _iter;
public:
  PEL_Service(PEL_ParamIterator_V_V &iter) : _iter(&iter), _r(-1) {}
  PEL_Service &entail(void) {
    CDM(("PEL_Service::entail()\n"));
    _r = _iter->vanish();
    return *this;
  }
  PEL_Service &leave(int vars_left = 0) {
    CDM(("PEL_Service::leave()\n"));
    _r = _iter->leave(vars_left);
    return *this;
  }
  PEL_Service &fail(void) {
    CDM(("PEL_Service::fail()\n"));
    _r = _iter->fail();
    return *this;
  }
  pf_return_t operator ()() { return pf_return_t(_r); }
};

#ifdef NEW

pf_return_t PEL_LessEqOffset::propagate(PEL_Engine &e)
{
  //
  PEL_FDIntVar &x = *(PEL_FDIntVar *) e[_x];
  int c = _c;
  PEL_FDIntVar &y = *(PEL_FDIntVar *) e[_y];
  //
  PEL_ParamIterator_V_V iter(x, y);
  PEL_Service s(iter);
  //
  return filter_lessEqOffset(s, x, y, c)();
}

#else

pf_return_t PEL_LessEqOffset::propagate(PEL_Engine &e)
{
  //
  PEL_FDIntVar &x = *(PEL_FDIntVar *) e[_x];
  int c = _c;
  PEL_FDIntVar &y = *(PEL_FDIntVar *) e[_y];
  //
  FailOnEmpty(*x <= (y->getMaxElem() - c));
  FailOnEmpty(*y >= (x->getMinElem() + c));
  //
  if (x->getMaxElem() + c <= y->getMinElem()) {
    x.leave();
    y.leave();
    CDM(("\t-> entailed\n"));
    return pf_entailed;
  }
  //
  if (x->getMinElem() + c > y->getMaxElem()) {
    CDM(("\t-> failed\n"));
    goto failure;
  }
  {
    pf_return_t r = (x.leave() | y.leave()) ? pf_sleep : pf_entailed;
    CDM(("\t-> %s\n", r == pf_sleep ? "sleep" : "entailed"));
    return r;
  }
 failure:
  CDM(("\t-> failed\n"));
  return pf_failed;
}
#endif
