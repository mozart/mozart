/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Ralf Scheidhauer (scheidhr@dfki.de)
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

#include <string.h>

#include "mozart_cpi.hh"

OZ_PropagatorProfile * OZ_PropagatorProfile::_all_headers = NULL;


void OZ_PropagatorProfile::profileReset()
{
  OZ_PropagatorProfile * aux = getFirst();
  while(aux) {
    aux->_calls   = 0;
    aux->_samples = 0;
    aux->_heap = 0;
    aux = aux->getNext();
  }
}


OZ_PropagatorProfile::OZ_PropagatorProfile(char * propagator_name)
  : _calls(0), _samples(0), _heap(0),
    _propagator_name(strdup(propagator_name))
{
  *this = propagator_name;
}


void OZ_PropagatorProfile::operator = (char * propagator_name)
{
  _calls = 0;
  _samples = 0;
  _heap = 0;
  _propagator_name = strdup(propagator_name);

  static int firstCall = 1;
  if (firstCall) {
    firstCall = 0;
    _all_headers = 0;
  }
  _next = _all_headers;
  _all_headers = this;
}
