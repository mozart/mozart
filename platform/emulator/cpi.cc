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

#if defined(INTERFACE)
#pragma implementation "cpi.hh"
#endif

#include "conf.h"


#include <math.h>

#include "cpi.hh"

CpiHeapClass CpiHeap;

EnlargeableArray<_spawnVars_t> staticSpawnVars(CPIINITSIZE);
EnlargeableArray<_spawnVars_t> staticSpawnVarsProp(CPIINITSIZE);
EnlargeableArray<_spawnVars_t> staticSuspendVars(CPIINITSIZE);

int staticSpawnVarsNumber = 0;
int staticSpawnVarsNumberProp = 0;
int staticSuspendVarsNumber = 0;

#if defined(OUTLINE)
#define inline
#include "cpi.icc"
#undef inline
#endif

#ifdef TMUELLER

void OZ_CPIVar::dropParameter(void)
{
  OzVariable * cvar    = tagged2CVar(var);
  void * cpi_raw       = cvar->getRawAndUntag();
  int isNonEncapTagged = cvar->isParamNonEncapTagged();
  int isEncapTagged    = cvar->isParamEncapTagged();

  OZ_CPIVar * forward = (OZ_CPIVar *) cpi_raw;
#ifdef DEBUG_REMOVE_PARAMS
  printf("this=%p nb_refs=%d\n", this, forward->_nb_refs);
#endif
  //
  // remove propagator from event list only if there is at most one
  // occurrence in the parameter set, otherwise do not do anything
  //
  if (forward->_nb_refs == 1) {
#ifdef DEBUG_REMOVE_PARAMS
    printf("!!! Dropping parameter !!!\n");
#endif
    if  (OZ_CPIVar::_first_run) {
      OZ_CPIVar::add_vars_removed(varPtr);
    } else {
      Propagator * prop = Propagator::getRunningPropagator();
      cvar->dropPropagator(prop);
    }
  }
  //
  // tag parameter again
  //
  if (isNonEncapTagged) {
    cvar->setStoreFlag();
  }
  if (isEncapTagged) {
    cvar->setReifiedFlag();
  }
  cvar->putRawTag(cpi_raw);
  forward->_nb_refs -= 1;
}

void OZ_CPIVar::add_vars_removed(OZ_Term * tp) {
  Assert(!oz_isRef(*tp));
  _vars_removed = oz_cons(makeTaggedRef(tp), _vars_removed);
}
int OZ_CPIVar::is_in_vars_removed(OZ_Term * tp) {
  for (OZ_Term p = _vars_removed; oz_nil() != p; p = oz_tail(p)) {
    if (oz_head(p) == makeTaggedRef(tp)) {
      return 1;
    }
  }
  return 0;
}
void OZ_CPIVar::reset_vars_removed(void) {
  _first_run = 0;
}
void OZ_CPIVar::set_vars_removed(void) {
  _first_run = 1;
  _vars_removed = oz_nil();
}

#endif

// End of File
//-----------------------------------------------------------------------------
