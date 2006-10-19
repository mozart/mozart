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


#include "base.hh"
#include "mozart_cpi.hh"



OZ_PropagatorProfile * OZ_PropagatorProfile::_all_headers = NULL;
static int OZ_PropagatorProfile_firstCall = 1;


OZ_PropagatorProfile::OZ_PropagatorProfile(void)
  : _calls(0), _samples(0), _heap(0),
    _propagator_name("<anonymous propagator>")
{
  if (OZ_PropagatorProfile_firstCall) {
    OZ_PropagatorProfile_firstCall = 0;
    _all_headers = 0;
  }
  _next = _all_headers;
  _all_headers = this;
}

OZ_PropagatorProfile::OZ_PropagatorProfile(char * propagator_name)
  : _calls(0), _samples(0), _heap(0),
    _propagator_name(propagator_name)
{
  if (OZ_PropagatorProfile_firstCall) {
    OZ_PropagatorProfile_firstCall = 0;
    _all_headers = 0;
  }
  _next = _all_headers;
  _all_headers = this;
}


void OZ_PropagatorProfile::operator = (char * propagator_name)
{
  _propagator_name = propagator_name;
}

