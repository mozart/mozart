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

#include "cpi.hh"
#include "var_ct.hh"

//-----------------------------------------------------------------------------

void * OZ_Ct::operator new(size_t s, int)
{
  return doubleMalloc(s);
}

void OZ_Ct::operator delete(void * p, size_t s)
{
  freeListDispose(p, s);
}

void * OZ_CtVar::operator new(size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_CtVar::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_CtVar::operator new[](size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_CtVar::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}
#endif


void OZ_CtVar::ask(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (oz_isVariable(vtag)) {
    ctSetLocalConstraint(tagged2GenCtVar(v)->getConstraint());
    setSort(var_e);
  } else {
    ctSetValue(v);
    setSort(val_e);
  }
}


void OZ_CtVar::read(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (oz_isVariable(vtag)) {
    // `v' designates a variable

    Assert(isCVar(vtag));

    if (Propagator::getRunningPropagator()->isLocal()) {
      // local variable per definition

      // set flags
      setState(loc_e);
      setSort(var_e);

      OzCtVariable * ctvar = tagged2GenCtVar(v);
      OZ_Ct * constr = ctvar->getConstraint();

      if (oz_onToplevel())
	ctSaveConstraint(constr);

      ctSetLocalConstraint(constr);

      ctSetConstraintProfile();

    } else {
      // don't know before hand if local or global

      // set flags
      OzCtVariable * ctvar = tagged2GenCtVar(v);
      setState(oz_isLocalVar(ctvar) ? loc_e : glob_e);
      setSort(var_e);

      OZ_Ct * constr = ctvar->getConstraint();

      if (isState(glob_e)) {
	ctRefConstraint(ctSaveConstraint(constr));
      } else {
	if (oz_onToplevel()) {
	  ctSaveConstraint(constr);
	}
	ctRefConstraint(constr);
      }

      ctSetConstraintProfile();
    }

    setStoreFlag(v); // tmueller
  } else {
    // `v' designates a value

    // set flags
    setSort(val_e);

    ctSetValue(v);

    ctSetConstraintProfile();
  }
}


void OZ_CtVar::readEncap(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (oz_isVariable(v)) {
    //`v' designates a variable
    Assert(isCVar(vtag));

    // set flags
    setState(encap_e);
    setSort(var_e);

    OzCtVariable * ctvar = tagged2GenCtVar(v);

    if (ctvar->testReifiedFlag()) {
    // var is already entered somewhere else

      ctRefConstraint(ctvar->getReifiedPatch());

      ctSetConstraintProfile();

    } else {
    // ct var entered first time

      OZ_Ct * constr = ctvar->getConstraint();

      OZ_Ct * constr_copy = ctSetEncapConstraint(constr);

      ctSetConstraintProfile();

      ctvar->patchReified(constr_copy);
    }

    setReifiedFlag(v);
  } else {
    // `v' designates a value

    // set flags
    setSort(val_e);
    setState(loc_e); // TMUELLER: why, ought to be redundant

    ctSetValue(v);

    ctSetConstraintProfile();
  }
}

OZ_Boolean OZ_CtVar::tell(void)
{
  DEBUG_CONSTRAIN_CVAR(("OZ_CtVar::tell "));

  // someone else has already determined it
  if (!oz_isVariable(*varPtr)) {
    return OZ_FALSE;
  }
  //
  if (testReifiedFlag(var)) {
    unpatchReifiedCt(var);
  }
  if (!testResetStoreFlag(var)) {
    // the constraint has already been told, i.e., there were at least
    // two OZ_FDIntVar connected to the same store variable
    goto oz_false;
  } else if(!isTouched()) {
    // the constraint has already been told, i.e., there were at least
    // two OZ_FDIntVar connected to the same store variable
    goto oz_true;
  } else {
    // 
    // there is a generic constraint variable in the store
    //
    Assert(isSort(var_e));
    //
    OzCtVariable * ctvar = tagged2GenCtVar(var);
    OZ_Ct * constr       = ctGetConstraint();
    //
    if (constr->isValue()) {
      //
      // propagation produced a value
      //
      if (isState(loc_e)) {
	// local variable
	ctvar->propagate(OZ_WAKEUP_ALL, pc_propagator);
	bindLocalVarToValue(varPtr, constr->toValue());
      } else {
	// global variable
	// wake up
	ctvar->propagate(OZ_WAKEUP_ALL, pc_propagator);
	bindGlobalVarToValue(varPtr, constr->toValue());
      }
      goto oz_false;
    } else {
      // 
      // propagation produced a set constraint
      //
      // wake up ...
      OZ_CtWakeUp wakeup_descr = ctGetWakeUpDescrptor();
      ctvar->propagate(wakeup_descr, pc_propagator);
      //
      if (isState(glob_e)) {
	constrainGlobalVar(varPtr, constr);
      }
      goto oz_true;
    }
  }
  //
 oz_false:
  //
  // variable is determined
  //
  DEBUG_CONSTRAIN_CVAR(("FALSE\n"));
  return OZ_FALSE;
  //
 oz_true:
  //
  // variable is still undetermined
  //
  DEBUG_CONSTRAIN_CVAR(("TRUE\n"));
  return OZ_TRUE;
}

void OZ_CtVar::fail(void)
{
  if (isSort(val_e))
    return;
  if (isState(encap_e)) {
    unpatchReifiedCt(var);
    return;
  }

  // dont't change the order of the calls (side effects!)
  if (testResetStoreFlag(var) && isState(glob_e) && isSort(var_e)) {
    ctRestoreConstraint();
  } else if (oz_onToplevel()) {
    ctRestoreConstraint();
  }
}


OZ_Return OZ_mkOZ_VAR_CT(OZ_Term v, OZ_Ct * c, OZ_CtDefinition * d)
{
  return tellBasicConstraint(v, c, d);
}

// eof
//-----------------------------------------------------------------------------
