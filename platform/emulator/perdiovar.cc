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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "perdiovar.hh"
#endif

#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#include "am.hh"
#include "genvar.hh"
#include "perdio.hh"

// bind and inform sites

// from perdio.cc
int compareNetAddress(PerdioVar *lVar,PerdioVar *rVar);

void PerdioVar::primBind(TaggedRef *lPtr,TaggedRef v)
{
  oz_checkSuspensionList(this, pc_std_unif);

  TaggedRef vv=oz_deref(v);
  if (isSVar(vv)) {
    SVariable *sv=tagged2SVar(vv);
    if (sv==this) return;
    oz_checkSuspensionList(sv, pc_std_unif);
    relinkSuspListTo(sv);
  }
  doBind(lPtr, v);
  if (isObjectClassNotAvail()) {
    deleteGName(u.gnameClass);
  }
}

OZ_Return PerdioVar::unifyV(TaggedRef *lPtr, TaggedRef r, ByteCode *scp)
{
  if (oz_isRef(r)) {
    TaggedRef *rPtr = tagged2Ref(r);
    TaggedRef rVal = *rPtr;
    TaggedRef lVal = *lPtr;
    GenCVariable *cv = tagged2CVar(rVal);

    if (cv->getType()!=PerdioVariable) return FAILED;

    PerdioVar *lVar = this;

    PerdioVar *rVar = (PerdioVar *)cv;

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
      oz_checkSuspensionList(tagged2SVarPlus(lVal),pc_std_unif);
      am.doBindAndTrail(lPtr,makeTaggedRef(rPtr));
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
  if (!valid(lPtr,r)) return FAILED;

  if (am.isLocalSVar(this)) {
    // onToplevel: distributed unification
    return bindPerdioVar(this,lPtr,r);
  } else {
    // in guard: bind and trail
    oz_checkSuspensionList(tagged2SVarPlus(*lPtr),pc_std_unif);
    am.doBindAndTrail(lPtr,r);
    return PROCEED;
  }
}

Bool PerdioVar::valid(TaggedRef *varPtr, TaggedRef v)
{
  Assert(!oz_isRef(v) && !oz_isVariable(v));

  return (isObject()) ? FALSE : TRUE;
}


void PerdioVar::dispose(void)
{
}


//-----------------------------------------------------------------------------
// Implementation of interface functions

#ifdef MISC_BUILTINS

OZ_BI_define(PerdioVar_is, 1,1)
{
  OZ_RETURN(isPerdioVar(oz_deref(OZ_in(0)))?NameTrue:NameFalse);
} OZ_BI_end

#endif
