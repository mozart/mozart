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
  isMarshaled = NO;

  //
  PD((MARSHAL,"var proxy bi: %d", bi));
  isFuture = pv->isFuture();
  ms = borrowTable->getOriginSite(bi);
  if (dest && ms == dest) {
    isToOwner = OK;
    saveMarshalToOwner(bi, oti, credit);
  } else {
    isToOwner = NO;
    saveMarshalBorrowHead(bi, ms, oti, credit);
  }
}

//
void ExportedProxyVar::marshal(ByteBuffer *bs)
{
//    DebugCode(PD((MARSHAL,"exported var proxy bi:%d", bi)););
  Assert(isMarshaled == NO);
  isMarshaled = OK;
  //
  if (isToOwner)
    marshalToOwnerSaved(bs, credit, oti);
  else
    marshalBorrowHeadSaved(bs, (isFuture ? DIF_FUTURE : DIF_VAR),
			   ms, oti, credit);
}

//
void ExportedProxyVar::gCollectRecurseV()
{
  DebugCode(PD((GC, "ExportedProxyVar b:%d", bti)););
  ms->makeGCMarkSite();
  if (credit.owner) credit.owner->makeGCMarkSite();
}

//
void ExportedProxyVar::disposeV()
{
  Assert(extVar2Var(this)->isEmptySuspList());
  //
  if (!isMarshaled) {
    if (isToOwner) {
      discardToOwnerSaved(ms, oti, credit);
    } else {
      discardBorrowHeadSaved(ms, oti, credit);
    }
  }
  freeListDispose(sizeof(ExportedProxyVar));
}

