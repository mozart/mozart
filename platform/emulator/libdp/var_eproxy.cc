/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Konstantin Popov (2000)
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "var_eproxy.hh"
#endif

#include "var_eproxy.hh"
#include "dpMarshaler.hh"

//
ExportedProxyVar::ExportedProxyVar(ProxyVar *pv, DSite *dest)
  : ExtVar(oz_rootBoard())
{
  Assert(pv->getIdV() == OZ_EVAR_PROXY);
  int bi = pv->getIndex();
  DebugCode(bti = bi;);
  DebugCode(isMarshaled = NO;);

  //
  PD((MARSHAL,"var proxy bi: %d", bi));
  isFuture = pv->isFuture();
  if (dest && borrowTable->getOriginSite(bi) == dest) {
    saveMarshalToOwner(bi, oti, ct, credit, scm);
    // used for distinguishing between these cases during marshaling,
    // as well as GC needs it:
    ms = (DSite *) 0;
  } else {
    saveMarshalBorrowHead(bi, ms, oti, ct, credit, scm);
  }
}

//
void ExportedProxyVar::marshal(ByteBuffer *bs)
{
//    DebugCode(PD((MARSHAL,"exported var proxy bi:%d", bi)););
  Assert(isMarshaled == NO);
  DebugCode(isMarshaled = OK;);
  //
  if (ms)
    marshalBorrowHeadSaved(bs, (isFuture ? DIF_FUTURE : DIF_VAR),
			   ms, oti, ct, credit, scm);
  else
    marshalToOwnerSaved(bs, ct, oti, scm);
}

//
void ExportedProxyVar::gCollectRecurseV()
{
  DebugCode(PD((GC, "ExportedProxyVar b:%d", bti)););
  if (ms) ms->makeGCMarkSite();
  if (scm) scm->makeGCMarkSite();
}

