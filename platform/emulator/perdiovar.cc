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
Bool PerdioVar::bindManager(TaggedRef *lPtr,TaggedRef v, PerdioVar *rVar)
{
  warning("bindPerdioVar: only does local unification: no protocol yet");

  if (rVar) relinkSuspListTo(rVar);
  doBind(lPtr, v);
  return TRUE;
}

Bool PerdioVar::bindProxy(TaggedRef *lPtr,TaggedRef v, PerdioVar *rVar)
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

    if (prop) {
      if (am.isLocalSVar(lVar)) {
	if (am.isLocalSVar(rVar)) {
	  int cmp = lVar->compare(rVar);
	  Assert(cmp!=0);
	  if (cmp<0) {
	    if (lVar->isManager()) {
	      return lVar->bindManager(lPtr,makeTaggedRef(rPtr));
	    } else {
	      return lVar->bindProxy(lPtr,makeTaggedRef(rPtr));
	    }
	  }
	  Assert(cmp>0);
	  if (rVar->isManager()) {
	    return rVar->bindManager(rPtr,makeTaggedRef(lPtr));
	  } else {
	    return rVar->bindProxy(rPtr,makeTaggedRef(lPtr));
	  }
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
    if (lVar->isManager()) {
      return lVar->bindManager(lPtr,rVal);
    } else {
      return lVar->bindProxy(lPtr,rVal);
    }
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
  {"PerdioVar.is",	      2, PerdioVar_is},

  {0,0,0,0}
};

void BIinitPerdioVar()
{
  BIaddSpec(pvarSpec);
}

