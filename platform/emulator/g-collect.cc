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

#define G_COLLECT
#undef  S_CLONE

#define OZ_cacBlock              OZ_gCollectBlock
#define oz_cacTerm               oz_gCollectTerm

#define _cacRecurse              gCollectRecurse
#define _cac                     gCollect
#define _cacRecurseV             gCollectRecurseV
#define _cacV                    gCollectV

#define _cacMark                 gCollectMark

#define _cacConstTerm            gCollectConstTerm
#define _cacConstTermWithHome    gCollectConstTermWithHome
#define _cacConstRecurse         gCollectConstRecurse

#define _cacBoard                gCollectBoard

#define _cacName                 gCollectName

#define _cacClass                gCollectClass
#define _cacObject               gCollectObject
#define _cacObjectInline         gCollectObjectInline

#define _cacAbstraction          gCollectAbstraction

#define _cacVar                  gCollectVar
#define _cacVarRecurse           gCollectVarRecurse
#define _cacVarInline            gCollectVarInline

#define _cacSRecord              gCollectSRecord

#define _cacSuspendableInline    gCollectSuspendableInline
#define _cacSuspendable          gCollectSuspendable

#define _cacLocalInline          gCollectLocalInline
#define _cacLocalRecurse         gCollectLocalRecurse

#define _cacExtension            gCollectExtension
#define _cacExtensionRecurse     gCollectExtensionRecurse

#define _cacPendThreadEmul       gCollectPendThreadEmul

#define _cacFix                  gCollectFix

#define _cac_varSizes            gCollectVarsizes

#define _cacReallocStatic        gCollectReallocStatic
#define _cacStoreFwd             gCollectStoreFwd

#define _cacRefsArray            gCollectRefsArray
#define _cacRefsArrayIsMarked    gCollectRefsArrayIsMarked
#define _cacRefsArrayMark        gCollectRefsArrayMark
#define _cacRefsArrayUnmark      gCollectRefsArrayUnmark

#define _cacSuspList             gCollectSuspList
#define _cacLocalSuspList        gCollectLocalSuspList

#include "cac.cc"

OZ_Term * OZ_gCollectAllocBlock(int n, OZ_Term * frm) {
  if (n==0)
    return (OZ_Term *) NULL;

  OZ_Term * to = (OZ_Term *) heapMalloc(n * sizeof(OZ_Term));

  OZ_gCollectBlock(frm, to, n);

  return to;
}
