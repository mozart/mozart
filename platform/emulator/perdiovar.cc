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
Bool PerdioVar::bindPerdioVar(TaggedRef *lPtr,TaggedRef v,PerdioVar *rVar=0)
{
  warning("bindPerdioVar: only does local unification: no protocol yet");

  if (rVar) relinkSuspListTo(rVar);
  doBind(lPtr, v);
  return TRUE;
}

Bool PerdioVar::surrender(TaggedRef *lPtr,TaggedRef v,PerdioVar *rVar)
{
  warning("surrender: only does local unification: no protocol yet");
  return TRUE;
}


Bool PerdioVar::unifyPerdioVar(TaggedRef * lPtr, TaggedRef * rPtr, Bool prop)
{
  TaggedRef rVal = *rPtr;
  TaggedRef lVal = *lPtr;

  Assert(!isNotCVar(rVal));
  
  PerdioVar *lVar = this;
  Assert(!lVar->isBound());

  if (isCVar(rVal)) {
    if (tagged2CVar(rVal)->getType() != getType()) {
      warning("PerdioVAR = other CVAR: not implemented");
      return FALSE;
    }
    if (prop) {
      am.checkSuspensionList(lVal,pc_std_unif);
      am.checkSuspensionList(rVal,pc_std_unif);
    }

    PerdioVar *rVar = tagged2PerdioVar(rVal);
    Assert(!rVar->isBound());

    Bool l_is_local = (prop && am.isLocalSVar(lVar));
    Bool r_is_local = (prop && am.isLocalSVar(rVar));
    switch (l_is_local + 2 * r_is_local) {
    case TRUE + 2 * TRUE: // v and t are local
      {
	int cmp = lVar->compare(rVar);
	Assert(cmp!=0);
	if (cmp<0) {
	  if (lVar->isManager()) {
	    return lVar->bindPerdioVar(lPtr,makeTaggedRef(rPtr),rVar);
	  } else {
	    return lVar->surrender(lPtr,makeTaggedRef(rPtr),rVar);
	  }
	  return TRUE;
	}
	Assert(cmp>0);
	if (rVar->isManager()) {
	  return rVar->bindPerdioVar(rPtr,makeTaggedRef(lPtr),lVar);
	} else {
	  return rVar->surrender(rPtr,makeTaggedRef(lPtr),lVar);
	}
      }
      return TRUE;
      
    case TRUE + 2 * FALSE: // r is local and l is global
      am.doBindAndTrail(rVal, rPtr,makeTaggedRef(lPtr));
      return TRUE;
      
    case FALSE + 2 * TRUE: // r is global and l is local
      am.doBindAndTrail(lVal, lPtr,makeTaggedRef(rPtr));
      return TRUE;

    case FALSE + 2 * FALSE: // r and l are global
      am.doBindAndTrail(lVal, lPtr,makeTaggedRef(rPtr));
      return TRUE;
      
    default:
      Assert(0);
      return FALSE;
      break;
    }
  } else {
    Assert(!isAnyVar(rVal));
    if (prop && am.isLocalSVar(this)) {
      return bindPerdioVar(lPtr,rVal);
    } else {
      if (prop) am.checkSuspensionList(lVal,pc_std_unif);
      am.doBindAndTrail(lVal, lPtr,rVal);
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
  {"PerdioVar.is",	      2, PerdioVar_is},

  {0,0,0,0}
};

void BIinitPerdioVar()
{
  BIaddSpec(pvarSpec);
}

