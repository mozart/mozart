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

void OZ_CPIVar::dropParameter(void)
{
  if (oz_isVar(var)) {
    OzVariable *ov    = tagged2Var(var);
    int isNonEncapTagged = ov->isParamNonEncapTagged();
    int isEncapTagged    = ov->isParamEncapTagged();
    void * cpi_raw       = ov->getRawAndUntag();
    
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
	ov->dropPropagator(prop);
      }
    }
    //
    // tag parameter again
    //
    if (isNonEncapTagged) {
      ov->setStoreFlag();
    }
    if (isEncapTagged) {
      ov->setReifiedFlag();
    }
    ov->putRawTag(cpi_raw);
    forward->_nb_refs -= 1;
  }
  setState(drop_e);
}

void initCPI(void)
{
  OZ_CPIVar::_vars_removed = oz_nil();
}

void OZ_CPIVar::add_vars_removed(OZ_Term * tp) {
  Assert(!oz_isRef(*tp));
  _vars_removed = oz_cons(makeTaggedRef(tp), _vars_removed);
}

int OZ_CPIVar::is_in_vars_removed(OZ_Term * tp) {
  Assert(_vars_removed || (_vars_removed == oz_nil()));

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

void * OZ_CPIVar::operator new(size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_CPIVar::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_CPIVar::operator new[](size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_CPIVar::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}

#endif

OZ_CPIVar * _getCPIVar(OZ_Term v)
{
  OzVariable *ov = tagged2Var(oz_deref(v));
  return (OZ_CPIVar *) ov->getRaw();
}

// End of File
//-----------------------------------------------------------------------------
