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
#include "var_fs.hh"

//-----------------------------------------------------------------------------

void * OZ_FSetVar::operator new(size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_FSetVar::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_FSetVar::operator new[](size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_FSetVar::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}
#endif


void OZ_FSetVar::ask(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (isFSetValueTag(vtag)) {
    set = *tagged2FSetValue(v);
    setPtr = &set;
    setSort(val_e);
  } else {
    Assert(isGenFSetVar(v));

    OzFSVariable * fsvar = tagged2GenFSetVar(v);

    setPtr = &fsvar->getSet();
    setSort(var_e);
  }
}

void OZ_FSetVar::read(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (isFSetValueTag(vtag)) {
    set = *tagged2FSetValue(v);
    setPtr = &set;
    setSort(val_e);
    known_in = set.getCardMin();
    // used to be known_not_in=0 which was known to be wrong.
    // however, for values, everything is known, therefore
    // everything that is not known to be in is known to be
    // not in.  The cardinality of the largest set is fs_sup+1
    // because its domain is {0,1,...,fs_sup}. --Denys
    known_not_in = (fs_sup+1) - known_in;
    card_size = 1;
  } else {
    Assert(oz_isCVar(v));

    if (Propagator::getRunningPropagator()->isLocal()) {
    // local variable per definition

      setState(loc_e);
      OzFSVariable * fsvar = tagged2GenFSetVar(v);

      setSort(var_e);

      if (oz_onToplevel())
        set = fsvar->getSet();
      setPtr = &fsvar->getSet();

      known_in = setPtr->getKnownIn();
      known_not_in = setPtr->getKnownNotIn();
      card_size = setPtr->getCardSize();

    } else {
      // don't know before hand if local or global

      OzFSVariable * fsvar = tagged2GenFSetVar(v);
      setState(oz_isLocalVar(fsvar) ? loc_e : glob_e);

      if (isState(glob_e)) {
        set = fsvar->getSet();
        setPtr = &set;
      } else {
        if (oz_onToplevel()) {
          set = fsvar->getSet();
        }
        setPtr = &(fsvar->getSet());
      }

      known_in = setPtr->getKnownIn();
      known_not_in = setPtr->getKnownNotIn();
      card_size = setPtr->getCardSize();
      setSort(var_e);

    }

    setStoreFlag(v);
  }
}

void OZ_FSetVar::readEncap(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (isFSetValueTag(vtag)) {
    set = *tagged2FSetValue(v);
    setPtr = &set;
    setSort(val_e);
    known_in = set.getCardMin();
    // loeckelt:
    known_not_in = fs_sup - known_in + 1;
    // was:
    // known_not_in =  32*fset_high - known_in;
    card_size = 1;
    setState(loc_e); // TMUELLER: why, ought to be redundant
    setSort(val_e);
  } else {
    setState(encap_e);
    setSort(var_e);

    OzFSVariable * fsvar = tagged2GenFSetVar(v);

    if (fsvar->testReifiedFlag()) {
      // fs var already entered somewhere else
      setPtr = fsvar->getReifiedPatch();
      known_in = setPtr->getKnownIn();
      known_not_in = setPtr->getKnownNotIn();
      card_size = setPtr->getCardSize();
    } else {
      // fs var entered first time
      set = fsvar->getSet();
      setPtr = &set;
      known_in = set.getKnownIn();
      known_not_in = set.getKnownNotIn();
      card_size = set.getCardSize();

      fsvar->patchReified(setPtr);
    }
    setReifiedFlag(v);
  }
}

OZ_Boolean OZ_FSetVar::tell(void)
{
  DEBUG_CONSTRAIN_CVAR(("OZ_FSetVar::tell "));

  // someone else has determined it
  if (!oz_isVariable(*varPtr)) {
    return OZ_FALSE;
  }
  //
  if (testReifiedFlag(var)) {
    unpatchReifiedFSet(var);
  }
  //
  if (!testResetStoreFlag(var)) {
    // the constraint has aslready been told, i.e., there were at
    // least two OZ_FSetVar connected to the same store variable
    goto oz_false;
  } else if(!isTouched()) {
    // no constraints have been imposed by the current propagator run.
    // note the cases catches integers too.
    goto oz_true;
  } else {
    //
    // there is a finite set variable in the store
    //
    Assert(isSort(var_e));
    //
    if (setPtr->isValue()) {
      //
      // propagation produced a set value
      //
      if (isState(loc_e)) {
        // local variable
        tagged2GenFSetVar(var)->becomesFSetValueAndPropagate(varPtr);
      } else {
        // global variable
        OZ_FSetValue * set_value = new OZ_FSetValue(*setPtr);
        // wake up
        tagged2GenFSetVar(var)->propagate(fs_prop_val);
        bindGlobalVarToValue(varPtr, makeTaggedFSetValue(set_value));
      }
      goto oz_false;
    } else {
      //
      // propagation produced a set constraint
      //
      // wake up ...
      // ... lower bounds
      if (known_in < setPtr->getKnownIn())
        tagged2GenFSetVar(var)->propagate(fs_prop_glb);
      // ... upper bounds
      if (known_not_in < setPtr->getKnownNotIn())
        tagged2GenFSetVar(var)->propagate(fs_prop_lub);
      // ... cardinality
      if (card_size > setPtr->getCardSize())
        tagged2GenFSetVar(var)->propagate(fs_prop_val);
      //
      if (isState(glob_e)) {
        constrainGlobalVar(varPtr, set);
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

void OZ_FSetVar::fail(void)
{
  if (isSort(val_e))
    return;
  if (isState(encap_e)) {
    unpatchReifiedFSet(var);
    return;
  }

  // dont't change the order of the calls (side effects!)
  if (testResetStoreFlag(var) && isState(glob_e) && isSort(var_e)) {
    *setPtr = set;
  } else if (oz_onToplevel()) {
    *setPtr = set;
  }
}

OZ_Boolean OZ_FSetVar::isTouched(void) const
{
  return ((known_in < setPtr->getKnownIn()) ||
          (known_not_in < setPtr->getKnownNotIn()) ||
          (card_size > setPtr->getCardSize()));
}

int OZ_getFSetInf(void)
{
  return fset_inf;
}

int OZ_getFSetSup(void)
{
  return fset_sup;
}


// End of File
//-----------------------------------------------------------------------------
