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
  if (isObjectGName()) {
    deleteGName(u.gnameClass);
  }
}

GName *getGNameForUnify(TaggedRef val) {
  if (!isConst(val)) return 0;
  ConstTerm *c=tagged2Const(val);
  switch(c->getType()) {
  case Co_Abstraction:
  case Co_Class:
  case Co_Chunk:
    return ((ConstTermWithHome *)c)->getGName1();
  default:
    return 0;
  }
}

Bool PerdioVar::unifyPerdioVar(TaggedRef *lPtr, TaggedRef *rPtr, ByteCode *scp)
{
  TaggedRef rVal = *rPtr;
  TaggedRef lVal = *lPtr;

  Assert(!isNotCVar(rVal));

  PerdioVar *lVar = this;

  if (isPerdioVar(rVal)) {
    PerdioVar *rVar = tagged2PerdioVar(rVal);

    if (isObjectURL() || isObjectGName() || isURL()) {
      if (rVar->isObjectURL() || rVar->isObjectGName() || rVar->isURL()) {
        if (getGName() != rVar->getGName()) {
          // the following is completely legal
          // warning("mm2:gname mismatch (var-var)");
          return FALSE;
        }
      }
      if (!rVar->isObjectURL() && !rVar->isObjectGName()) {
        /*
         * binding preferences
         * bind perdiovar -> proxy
         * bind url proxy -> object proxy
         */
        Swap(rVal,lVal,TaggedRef);
        Swap(rPtr,lPtr,TaggedRef*);
      }
    }

    PD((PD_VAR,"unify i:%d i:%d",lVar->getIndex(),rVar->getIndex()));

    if (scp==0) {
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

  if (!valid(lPtr,rVal)) return FALSE;

  if (am.isLocalSVar(lVar)) {
    bindPerdioVar(lVar,lPtr,rVal);
    return TRUE;
  } else {
    if (isCVar(rVal)) {
      warning("PerdioVAR = other CVAR: not implemented");
      return FALSE;
    }
    am.checkSuspensionList(lVal,pc_std_unif);
    am.doBindAndTrail(lVal, lPtr,rVal);
    return TRUE;
  }
}

Bool PerdioVar::valid(TaggedRef *varPtr, TaggedRef v)
{
  Assert(!isRef(v) && !isAnyVar(v));

  if (isObjectURL() || isObjectGName() || isURL()) {
    if (getGName() != getGNameForUnify(v)) {
      return FALSE;
    }
  }
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
