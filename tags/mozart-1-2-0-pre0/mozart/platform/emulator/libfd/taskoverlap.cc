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

#include "taskoverlap.hh"
#include "rel.hh"
#include "taskoverlap_filter.hh"

//-----------------------------------------------------------------------------
// propagation member function


OZ_Return TasksOverlapPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_xd, &yd = reg_yd;

  OZ_FDIntVar x(reg_x), y(reg_y), o(reg_b);
  PropagatorController_V_V_V P(x, y, o);
  OZ_Filter<OZ_Propagator> s(this, &P);
  OZ_Return r = tasksoverlap.filter(s, x, xd, y, yd, o)();
  return r;
}

OZ_BI_define(fdp_tasksOverlap, 5, 0)
{
  OZ_Expect pe;
  OZ_Return r;
  //  
  return make_tasksoverlap(r, pe,
			   OZ_in(0), OZ_in(1), OZ_in(2), OZ_in(3), OZ_in(4));
}
OZ_BI_end

OZ_PropagatorProfile TasksOverlapPropagator::profile;
