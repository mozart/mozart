/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "cpi.hh"

//-----------------------------------------------------------------------------

void * OZ_FSetVar::operator new(size_t s)
{
  return heap_new(s);
}

void OZ_FSetVar::operator delete(void * p, size_t s)
{
  heap_delete(p, s);
}

#ifdef __GNUC__
void * OZ_FSetVar::operator new[](size_t s)
{
  return heap_new(s);
}

void OZ_FSetVar::operator delete[](void * p, size_t s)
{
  heap_delete(p, s);
}
#endif


void OZ_FSetVar::ask(OZ_Term v)
{
  Assert(isRef(v) || !isAnyVar(v));

  _DEREF(v, varPtr, vtag);
  var = v;

  if (isFSetValue(vtag)) {
    set = *tagged2FSetValue(v);
    setPtr = &set;
    setSort(val_e);
  } else {
    Assert(isGenFSetVar(v, vtag));

    GenFSetVariable * fsvar = tagged2GenFSetVar(v);

    setPtr = &fsvar->getSet();
    setSort(var_e);
  }
}

void OZ_FSetVar::read(OZ_Term v)
{
  Assert(isRef(v) || !isAnyVar(v));

  _DEREF(v, varPtr, vtag);
  var = v;

  if (isFSetValue(vtag)) {
    set = *tagged2FSetValue(v);
    setPtr = &set;
    setSort(val_e);
    known_in = set.getCardMin();
    known_not_in = 0;
    card_size = 1;
  } else {
    Assert(isCVar(vtag));

    if (am.currentThread->isLocalThread()) {
    // local variable per definition

      setState(loc_e);
      GenFSetVariable * fsvar = tagged2GenFSetVar(v);

      setSort(var_e);

      if (am.currentBoard->isRoot())
        set = fsvar->getSet();
      setPtr = &fsvar->getSet();

      known_in = setPtr->getKnownIn();
      known_not_in = setPtr->getKnownNotIn();
      card_size = setPtr->getCardSize();

    } else {
      // don't know before hand if local or global

      GenFSetVariable * fsvar = tagged2GenFSetVar(v);
      setState(am.isLocalCVar(v) ? loc_e : glob_e);

      if (isState(glob_e) || am.currentBoard->isRoot())
        set = fsvar->getSet();
      setPtr = &fsvar->getSet();
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
  Assert(isRef(v) || !isAnyVar(v));

  _DEREF(v, varPtr, vtag);
  var = v;

  if (isFSetValue(vtag)) {
    set = *tagged2FSetValue(v);
    setPtr = &set;
    setSort(val_e);
    known_in = set.getCardMin();
    known_not_in =  32*fset_high - known_in;
    card_size = 1;
    setState(loc_e); // TMUELLER: why, ought to be redundant
    setSort(val_e);
  } else {
    Assert(isCVar(vtag));

    setState(encap_e);
    GenFSetVariable * fsvar = tagged2GenFSetVar(v);

    if (fsvar->testReifiedFlag()) {
    // var is already entered somewhere else
      setSort(var_e);
      setPtr = fsvar->getReifiedPatch();
      known_in = setPtr->getKnownIn();
      known_not_in = setPtr->getKnownNotIn();
      card_size = setPtr->getCardSize();
    } else {
    // fd var entered first time
      setSort(var_e);
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
#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tell_fs: " << *setPtr << endl << flush;
#endif

  if (testReifiedFlag(var))
    unpatchReified(var);

  if (!testResetStoreFlag(var)) {
    goto f;
  } else if(!isTouched()) {
    goto t;
  } else {
    Assert(isSort(var_e)); // must be finite set variable

    if (setPtr->isValue()) {
      if (isState(loc_e)) {
        tagged2GenFSetVar(var)->becomesFSetValueAndPropagate(varPtr);
      } else {
        OZ_FSetValue setvalue = *setPtr;
        *setPtr = set;
        tagged2GenFSetVar(var)->propagate(var, fs_prop_val);
        am.doBindAndTrail(var, varPtr,
                          makeTaggedFSetValue(new OZ_FSetValue(setvalue)));
      }
      goto f;
    } else {
      if (known_in < setPtr->getKnownIn())
        tagged2GenFSetVar(var)->propagate(var, fs_prop_glb);

      if (known_not_in < setPtr->getKnownNotIn())
        tagged2GenFSetVar(var)->propagate(var, fs_prop_lub);

      if (card_size > setPtr->getCardSize())
        tagged2GenFSetVar(var)->propagate(var, fs_prop_val);

      if (isState(glob_e)) {
        GenFSetVariable * locfsvar = new GenFSetVariable(*setPtr);
        OZ_Term * loctaggedfsvar = newTaggedCVar(locfsvar);
        *setPtr = set;
        am.doBindAndTrailAndIP(var, varPtr,
                               makeTaggedRef(loctaggedfsvar),
                               locfsvar, tagged2GenFSetVar(var), OZ_FALSE);
      }
      goto t;
    }
  }
t:
#ifdef DEBUG_TELLCONSTRAINTS
  taggedPrint(makeTaggedRef(varPtr));
  cout << "(t)" << endl << flush;
#endif
  return OZ_TRUE;

f:
#ifdef DEBUG_TELLCONSTRAINTS
  taggedPrint(makeTaggedRef(varPtr));
  cout << "(f)" << endl << flush;
#endif
  return OZ_FALSE;
}

void OZ_FSetVar::fail(void)
{
  if (isSort(val_e))
    return;
  if (isState(encap_e)) {
    unpatchReified(var);
    return;
  }

  // dont't change the order of the calls (side effects!)
  if (testResetStoreFlag(var) && isState(glob_e) && isSort(var_e)) {
    *setPtr = set;
  } else if (am.currentBoard->isRoot()) {
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
