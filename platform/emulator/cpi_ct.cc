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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "cpi.hh"

//-----------------------------------------------------------------------------

void * OZ_GenConstraint::operator new(size_t s, int align)
{
  return alignedMalloc(s, align);
}

void OZ_GenConstraint::operator delete(void * p, size_t s)
{
  freeListDispose(p, s);
}

void * OZ_GenCtVar::operator new(size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_GenCtVar::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_GenCtVar::operator new[](size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_GenCtVar::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}
#endif


void OZ_GenCtVar::ask(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  _DEREF(v, varPtr, vtag);
  var = v;

  if (oz_isVariable(vtag)) {
    ctSetLocalConstraint(tagged2GenCtVar(v)->getConstraint());
    setSort(var_e);
  } else {
    ctSetValue(v);
    setSort(val_e);
  }
}


void OZ_GenCtVar::read(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  _DEREF(v, varPtr, vtag);
  var = v;

  if (oz_isVariable(vtag)) {
    // `v' designates a variable

    Assert(isCVar(vtag));

    if (Propagator::getRunningPropagator()->isLocalPropagator()) {
      // local variable per definition

      // set flags
      setState(loc_e);
      setSort(var_e);

      GenCtVariable * ctvar = tagged2GenCtVar(v);
      OZ_GenConstraint * constr = ctvar->getConstraint();

      if (am.onToplevel())
        ctSaveConstraint(constr);

      ctSetLocalConstraint(constr);

      ctSetConstraintProfile();

    } else {
      // don't know before hand if local or global

      // set flags
      setState(am.isLocalSVar(v) ? loc_e : glob_e);
      setSort(var_e);

      GenCtVariable * ctvar = tagged2GenCtVar(v);
      OZ_GenConstraint * constr = ctvar->getConstraint();

      if (isState(glob_e) || am.onToplevel()) {
        ctSetGlobalConstraint(constr);
      } else {
        ctSetLocalConstraint(constr);
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


void OZ_GenCtVar::readEncap(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  _DEREF(v, varPtr, vtag);
  var = v;

  if (oz_isVariable(v)) {
    //`v' designates a variable
    Assert(isCVar(vtag));

    // set flags
    setState(encap_e);
    setSort(var_e);

    GenCtVariable * ctvar = tagged2GenCtVar(v);

    if (ctvar->testReifiedFlag()) {
    // var is already entered somewhere else

      ctRefConstraint(ctvar->getReifiedPatch());

      ctSetConstraintProfile();

    } else {
    // fs var entered first time

      OZ_GenConstraint * constr = ctvar->getConstraint();

      OZ_GenConstraint * constr_copy = ctSetEncapConstraint(constr);

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


OZ_Boolean OZ_GenCtVar::tell(void)
{
  if (testReifiedFlag(var))
    unpatchReifiedCt(var);

  if (!testResetStoreFlag(var)) {
    goto f;
  } else if(!isTouched()) {
    goto t;
  } else {
    Assert(isSort(var_e)); // must be constraint variable

    GenCtVariable * ctvar = tagged2GenCtVar(var);
    OZ_GenConstraint * constr = ctGetConstraint();

    if (constr->isValue()) {
      // a variable has been constrained to a value

      if (isState(loc_e)) {
        // a _local_ variable becomes a value

        ctvar->propagate(OZ_WAKEUP_ALL, pc_propagator);
        doBind(varPtr, constr->toValue());

      } else {
        // a _global_ variable becomes a value

        ctvar->propagate(OZ_WAKEUP_ALL, pc_propagator);
        am.doBindAndTrail(varPtr, constr->toValue());

        ctRestoreConstraint();
      }

      goto f;
    } else {

      OZ_GenWakeUpDescriptor wakeup_descr = ctGetWakeUpDescrptor();

      ctvar->propagate(wakeup_descr, pc_propagator);

      if (isState(glob_e)) {
        GenCtVariable * locctvar = new GenCtVariable(constr,
                                                     ctvar->getDefinition());
        OZ_Term * loctaggedctvar = newTaggedCVar(locctvar);

        ctRestoreConstraint();

        DoBindAndTrailAndIP(varPtr, makeTaggedRef(loctaggedctvar),
                            locctvar, ctvar);
      }

      goto t;
    }
  }

t:
  return OZ_TRUE;

f:
  return OZ_FALSE;
}


void OZ_GenCtVar::fail(void)
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
  } else if (am.onToplevel()) {
    ctRestoreConstraint();
  }
}


OZ_Return OZ_mkCtVariable(OZ_Term v,
                          OZ_GenConstraint * c,
                          OZ_GenDefinition * d)
{
  return tellBasicConstraint(v, c, d);
}

// eof
//-----------------------------------------------------------------------------
