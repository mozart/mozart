/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997)
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

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "perdiovar.hh"
#endif

#include "am.hh"
#include "genvar.hh"
#include "perdio.hh"

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
  if (isObjectClassNotAvail()) {
    deleteGName(u.gnameClass);
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

    if (isObject()) {
      if (rVar->isObject()) {
	// both are objects --> token equality
	return lVar==rVar ? PROCEED : FALSE;
      }
      if (!rVar->isObject()) {
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

    // Note: for perdio variables: am.isLocal == am.onToplevel
    if (scp!=0 || !am.isLocalSVar(lVar)) {
      // in any kind of guard then bind and trail
      am.checkSuspensionList(lVal,pc_std_unif);
      am.doBindAndTrail(lVal, lPtr,makeTaggedRef(rPtr));
      return TRUE;
    } else {
      // not in guard: distributed unification
      Assert(am.isLocalSVar(rVar));
      int cmp = compareNetAddress(lVar,rVar);
      Assert(cmp!=0);
      if (cmp<0) {
	bindPerdioVar(lVar,lPtr,makeTaggedRef(rPtr));
      } else {
	bindPerdioVar(rVar,rPtr,makeTaggedRef(lPtr));
      }
      return TRUE;
    }
  } // both PVARs


  // PVAR := non PVAR
  Assert(!isAnyVar(rVal));

  if (!valid(lPtr,rVal)) return FALSE;

  if (am.isLocalSVar(lVar)) {
    // onToplevel: distributed unification
    bindPerdioVar(lVar,lPtr,rVal);
    return TRUE;
  } else {
    // in guard: bind and trail
    am.checkSuspensionList(lVal,pc_std_unif);
    am.doBindAndTrail(lVal, lPtr,rVal);
    return TRUE;
  }
}

Bool PerdioVar::valid(TaggedRef *varPtr, TaggedRef v)
{
  Assert(!isRef(v) && !isAnyVar(v));

  return (isObject()) ? FALSE : TRUE;
}

//-----------------------------------------------------------------------------
// Implementation of interface functions

OZ_BI_define(PerdioVar_is, 1,1)
{
  OZ_RETURN(isPerdioVar(deref(OZ_in(0)))?NameTrue:NameFalse);
} OZ_BI_end
	  
// ---------------------------------------------------------------------
// Distributed stuff
// ---------------------------------------------------------------------


// static
// BIspec pvarSpec[] = {
//   {"PerdioVar.is",	      2, PerdioVar_is},
// 
//   {0,0,0,0}
// };
// 
// void BIinitPerdioVar()
// {
//   BIaddSpec(pvarSpec);
// }

