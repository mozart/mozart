/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Konstantin Popov, 2001
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

#if defined(INTERFACE)
#pragma implementation "marshalerPatch.hh"
#endif

#include "marshalerPatch.hh"

//
OZ_Term gcStartOVP(OzValuePatch *ovp)
{
  OZ_Term ret;
  OZ_Term *otp = &ret;
  Assert(ovp);

  //
  do {
    Assert(ovp->status == OVP_uninstalled);
    Assert(ovp->val == (OZ_Term) 0);
    OZ_Term *vp = tagged2Ref(ovp->loc);
    OZ_Term v = *vp;

    //
    // Everything but variables is (temporarily) patched;
    if (oz_isRef(v) || !oz_isVarOrRef(v)) {
      ovp->val = v;
      *vp = oz_makeExtVar(ovp);
      DebugCode(ovp->status = OVP_installed;);
      *otp = makeTaggedRef(vp);
      Assert(ovp->val != (OZ_Term) 0);
    } else {
      *otp = oz_makeExtVar(ovp);
      Assert(ovp->val == (OZ_Term) 0);
    }

    //
    otp = &(ovp->next.gc);	// construct the 'gc' linkage;
    ovp = ovp->next.ovp;
  } while (ovp);

  return (ret);
}

//
OzValuePatch* gcFinishOVP(OZ_Term ot)
{
  OzValuePatch *ret;
  OzValuePatch **ovpp = &ret;
  Assert(ot);

  //
  do {
    DEREF(ot, _otp);

    Assert(!oz_isRef(ot));
    Assert(oz_isVarOrRef(ot));	// due to 'gcStartOVP()';
    Assert(oz_isExtVar(ot));	// ditto;
    DebugCode(ExtVarType evt = oz_getExtVar(ot)->getIdV(););
    Assert(evt == OZ_EVAR_DISTRIBUTEDVARPATCH ||
	   evt == OZ_EVAR_MARSHALEDVARPATCH);

    OzValuePatch *ovp = (OzValuePatch *) oz_getExtVar(ot);
    if (ovp->val) {
      Assert(ovp->status == OVP_installed);
      OZ_Term *vp = tagged2Ref(ovp->loc);
      OZ_Term v = ovp->val;
      DEREF(v, _vp);
      *vp = v;
      // must be reset for 'gcStartOVP()'/'gcFinishOVP()':
      ovp->val = (OZ_Term) 0;
      DebugCode(ovp->status = OVP_uninstalled;);
    }

    //
    *ovpp = ovp;		// restore the naked linkage;
    ovpp = &(ovp->next.ovp);
    ot = ovp->next.gc;
  } while (ot);

  return (ret);
}

