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
#pragma implementation "var_emanager.hh"
#endif

#include "var_emanager.hh"
#include "dpMarshaler.hh"

//
extern Bool globalRedirectFlag;

//
ExportedManagerVar::ExportedManagerVar(ManagerVar *mv, DSite *dest)
  : ExtVar(oz_rootBoard()), isMarshaled(NO)
{
  Assert(mv->getIdV() == OZ_EVAR_MANAGER);

  //
  oti = mv->getIndex();
  saveMarshalOwnHead(oti, credit);
  if ((USE_ALT_VAR_PROTOCOL) && globalRedirectFlag == AUT_REG) {
    tag = mv->isFuture() ? DIF_FUTURE_AUTO : DIF_VAR_AUTO;
    mv->registerSite(dest);
  } else {
    tag = mv->isFuture() ? DIF_FUTURE : DIF_VAR;
  }
}

//
void ExportedManagerVar::marshal(ByteBuffer *bs)
{
  DebugCode(PD((MARSHAL,"exported var manager oti:%d", oti)););
  Assert(isMarshaled == NO);
  //
  marshalOwnHeadSaved(bs, tag, oti, credit);
  isMarshaled = OK;
}

//
void ExportedManagerVar::disposeV()
{
  Assert(extVar2Var(this)->isEmptySuspList());
  //
  if (!isMarshaled) {
    discardOwnHeadSaved(oti, credit);
  }
  freeListDispose(sizeof(ExportedManagerVar));
}
