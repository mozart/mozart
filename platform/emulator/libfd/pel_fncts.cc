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

#include "rel.hh"
#include "rel_filter.hh"
#include "pel_fncts.hh"

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

template <class ENGINE, class FDVAR, class PFDVAR>
pf_return_t PEL_LessEqOffset<ENGINE, FDVAR, PFDVAR>::propagate(PEL_Engine &e)
{
  //
  FDVAR &x = *(FDVAR *) e[_x];
  int c = _c;
  FDVAR &y = *(FDVAR *) e[_y];
  //
  PEL_ParamIterator_V_V iter(x, y);
  PEL_Service s(iter);
  //
  return filter_lessEqOffset(s, x, y, c)();
}
