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


// bind and inform sites

// from perdio.cc
void bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v);
int compareNetAddress(PerdioVar *lVar,PerdioVar *rVar);

void PerdioVar::primBind(TaggedRef *lPtr,TaggedRef v)
{
  setSuspList(am.checkSuspensionList(this, getSuspList(), pc_std_unif));

  TaggedRef vv=deref(v);
  if (isAnyVar(vv)) {
    Assert(isPerdioVar(vv));
    PerdioVar *pv=tagged2PerdioVar(vv);
    if (pv==this) return;
    pv->setSuspList(am.checkSuspensionList(pv, pv->getSuspList(),
                                           pc_std_unif));
    relinkSuspListTo(pv);
  }
  doBind(lPtr, v);
}

Bool PerdioVar::unifyPerdioVar(TaggedRef * lPtr, TaggedRef * rPtr, Bool prop)
{
  TaggedRef rVal = *rPtr;
  TaggedRef lVal = *lPtr;

  Assert(!isNotCVar(rVal));

  PerdioVar *lVar = this;

  if (isCVar(rVal)) {
    if (tagged2CVar(rVal)->getType() != getType()) {
      warning("PerdioVAR = other CVAR: not implemented");
      return FALSE;
    }

    PerdioVar *rVar = tagged2PerdioVar(rVal);

    PD(PD_VAR,"unify i:%d i:%d",lVar->getIndex(),rVar->getIndex());

    if (prop) {
      if (am.isLocalSVar(lVar)) {
        if (am.isLocalSVar(rVar)) {
          int cmp = compareNetAddress(lVar,rVar);
          Assert(cmp!=0);
          if (cmp<0) {
            bindPerdioVar(lVar,lPtr,makeTaggedRef(rPtr));
          } else {
            bindPerdioVar(rVar,rPtr,makeTaggedRef(lPtr));
          }
          return TRUE;
        }
        am.doBindAndTrail(rVal, rPtr,makeTaggedRef(lPtr));
        return TRUE;
      }
    }
    am.doBindAndTrail(lVal, lPtr,makeTaggedRef(rPtr));
    return TRUE;
  }

  Assert(!isAnyVar(rVal));
  if (prop && am.isLocalSVar(lVar)) {
    bindPerdioVar(lVar,lPtr,rVal);
    return TRUE;
  } else {
    if (prop) am.checkSuspensionList(lVal,pc_std_unif);
    am.doBindAndTrail(lVal, lPtr,rVal);
    return TRUE;
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
