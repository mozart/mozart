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
int compareNetAddress(PerdioVar *lVar,PerdioVar *rVar);

void PerdioVar::primBind(TaggedRef *lPtr,TaggedRef v)
{
  setSuspList(am.checkSuspensionList(this, getSuspList(), pc_std_unif));

  TaggedRef vv=oz_deref(v);
  if (oz_isVariable(vv)) {
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

OZ_Return PerdioVar::unifyPerdioVar(TaggedRef *lPtr, TaggedRef *rPtr, ByteCode *scp)
{
  if (isFuture())
    return ((Future*)this)->unifyFuture(lPtr);

  TaggedRef rVal = *rPtr;
  TaggedRef lVal = *lPtr;

  Assert(!isNotCVar(rVal));

  PerdioVar *lVar = this;

  if (isPerdioVar(rVal)) {
    PerdioVar *rVar = tagged2PerdioVar(rVal);

    if (isObject()) {
      if (rVar->isObject()) {
        // both are objects --> token equality
        return lVar==rVar ? PROCEED : FAILED;
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
      return PROCEED;
    } else {
      // not in guard: distributed unification
      Assert(am.isLocalSVar(rVar));
      int cmp = compareNetAddress(lVar,rVar);
      Assert(cmp!=0);
      if (cmp<0) {
        return bindPerdioVar(lVar,lPtr,makeTaggedRef(rPtr));
      } else {
        return bindPerdioVar(rVar,rPtr,makeTaggedRef(lPtr));
      }
    }
  } // both PVARs


  // PVAR := non PVAR
  Assert(!oz_isVariable(rVal));

  if (!valid(lPtr,rVal)) return FAILED;

  if (am.isLocalSVar(lVar)) {
    // onToplevel: distributed unification
    return bindPerdioVar(lVar,lPtr,rVal);
  } else {
    // in guard: bind and trail
    am.checkSuspensionList(lVal,pc_std_unif);
    am.doBindAndTrail(lVal, lPtr,rVal);
    return PROCEED;
  }
}

Bool PerdioVar::valid(TaggedRef *varPtr, TaggedRef v)
{
  Assert(!oz_isRef(v) && !oz_isVariable(v));

  if (isFuture())
    return ((Future*)this)->valid(v);

  return (isObject()) ? FALSE : TRUE;
}


void PerdioVar::dispose(void)
{
  if (isFuture())
    ((Future*)this)->dispose();
}


//-----------------------------------------------------------------------------
// Implementation of interface functions

OZ_BI_define(PerdioVar_is, 1,1)
{
  OZ_RETURN(isPerdioVar(oz_deref(OZ_in(0)))?NameTrue:NameFalse);
} OZ_BI_end
