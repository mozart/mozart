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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "filter.hh"

OZ_Service &OZ_Service::replace_propagator(OZ_Propagator * prop,
                                           int vars_drop = 0,
                                           /* (OZ_CPIVar *) */ ...)
{
  DSP(("request replace\n"));
  if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_replace;
      _actions[_nb_actions]._action_params._replacement = prop;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
  }
  //
  va_list ap;
  va_start(ap, vars_drop);
  for (int i = vars_drop; i--; ) {
    OZ_CPIVar * cpivar = va_arg(ap, OZ_CPIVar *);
    cpivar->dropParameter();
  }
  //
    _closed = 1;
    return *this;
}
