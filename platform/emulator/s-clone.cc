/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1999
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

#undef  G_COLLECT
#define S_CLONE

#define OZ_cacBlock             OZ_sCloneBlock
#define oz_cacTerm              oz_sCloneTerm

#define _cacRecurse             sCloneRecurse
#define _cac                    sClone
#define _cacRecurseV            sCloneRecurseV
#define _cacV                   sCloneV

#define _cacFix                 sCloneFix
#define _cacMark                sCloneMark

#define _cacConstTerm           sCloneConstTerm
#define _cacConstTermInline     sCloneConstTermInline
#define _cacConstRecurse        sCloneConstRecurse

#define _cacBoard               sCloneBoard

#define _cacName                sCloneName

#define _cacVarRecurse          sCloneVarRecurse
#define _cacVarInline           sCloneVarInline

#define _cacSRecord             sCloneSRecord

#define _cacSuspendableInline   sCloneSuspendableInline
#define _cacSuspendable         sCloneSuspendable

#define _cacLocalInline         sCloneLocalInline
#define _cacLocalRecurse        sCloneLocalRecurse

#define _cacExtension           sCloneExtension
#define _cacExtensionRecurse    sCloneExtensionRecurse

#define _cacPendThreadEmul      sClonePendThreadEmul


#define _cacReallocStatic       sCloneReallocStatic

#define _cacRefsArray           sCloneRefsArray

#include "cac.cc"


OZ_Term * OZ_sCloneAllocBlock(int n, OZ_Term * frm) {
  if (n==0)
    return (OZ_Term *) NULL;

  OZ_Term * to = (OZ_Term *) freeListMalloc(n * sizeof(OZ_Term));

  OZ_sCloneBlock(frm, to, n);

  return to;
}

Suspendable * suspendableSCloneSuspendable(Suspendable * s) {
  return s->sCloneSuspendable();
}

