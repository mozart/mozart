/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "perdiovar.hh"
#endif

#include "am.hh"
#include "genvar.hh"

Bool PerdioVar::unifyPerdioVar(TaggedRef * vptr, TaggedRef * tptr, Bool prop)
{
  warning("unifyPerdioVar: only does local unification: no protocol yet");

  TaggedRef t = *tptr;
  TaggedRef v = *vptr;

  Assert(! isNotCVar(t));

  if (isCVar(t)) {
    if (tagged2CVar(t)->getType() != getType()) {
      warning("PerdioVAR = other CVAR: not implemented");
      return FALSE;
    }
    if (prop) {
      am.checkSuspensionList(v,pc_std_unif);
      am.checkSuspensionList(t,pc_std_unif);
    }

    PerdioVar *term = tagged2PerdioVar(t);

    Bool v_is_local = (prop && am.isLocalSVar(this));
    Bool t_is_local = (prop && am.isLocalSVar(term));
    switch (v_is_local + 2 * t_is_local) {
    case TRUE + 2 * TRUE: // v and t are local
      if (heapNewer(vptr, tptr)) { // bind v to t
        relinkSuspListTo(term);
        doBind(vptr, makeTaggedRef(tptr));
      } else { // bind t to v
        term->relinkSuspListTo(this);
        doBind(tptr, makeTaggedRef(vptr));
      }
      return TRUE;

    case TRUE + 2 * FALSE: // v is local and t is global
      am.doBindAndTrail(t, tptr,makeTaggedRef(vptr));
      return TRUE;

    case FALSE + 2 * TRUE: // v is global and t is local
      am.doBindAndTrail(v, vptr,makeTaggedRef(tptr));
      return TRUE;

    case FALSE + 2 * FALSE: // v and t is global
      am.doBindAndTrail(v, vptr,makeTaggedRef(tptr));
      return TRUE;

    default:
      Assert(0);
      return FALSE;
      break;
    }
  } else {
    Assert(!isAnyVar(t));
    if (prop && am.isLocalSVar(this)) {
      doBind(vptr,t);
      return TRUE;
    } else {
      if (prop) am.checkSuspensionList(v,pc_std_unif);
      am.doBindAndTrail(v, vptr,t);
      return TRUE;
    }
  }
}

Bool PerdioVar::valid(TaggedRef *varPtr, TaggedRef v)
{
  Assert(!isRef(v) && !isAnyVar(v));

  return TRUE;
}


//-----------------------------------------------------------------------------
// Implementation of interface functions

OZ_C_proc_begin(PerdioVar_is, 2)
{
  return OZ_unify(OZ_getCArg(1),
                  isPerdioVar(deref(OZ_getCArg(0)))?NameTrue:NameFalse);
}
OZ_C_proc_end

// ---------------------------------------------------------------------
// Distributed stuff
// ---------------------------------------------------------------------


static
BIspec pvarSpec[] = {
  {"PerdioVar.is",            2, PerdioVar_is},

  {0,0,0,0}
};

void BIinitPerdioVar()
{
  BIaddSpec(pvarSpec);
}
