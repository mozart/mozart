/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __DPBASE_HH
#define __DPBASE_HH

#include "base.hh"

class LockProxy;
class CellProxy;
class CellFrame;
class LockFrame;
class LockManager;
class CellManager;
class VirtualInfo;
class VirtualSite;
class RemoteSite;
class TransController;
class Watcher;
class Twin;
class InformElem;
class BorrowEntry;
class OwnerEntry;
class DSite;
class ChainElem;
class GName;
class ManagerVar;
class ProxyVar;
class ClassVar;
class ObjectVar;
class ExportedProxyVar;
class GCStubVar;
class DPMarshaler;

//
typedef unsigned int FaultInfo;

//
typedef enum {
  VAR_PROXY,
  VAR_MANAGER,
  VAR_LAZY,
  VAR_FREE,
  VAR_FUTURE,    
  VAR_KINDED
} VarKind;

//
typedef enum {
  OBJECT,
  OBJECT_AND_CLASS
} LazyFlag ;
typedef enum {
  LT_OBJECT,
  LT_CLASS
} LazyType;

//
// OTIs ("onwer table indices") or BTI ("borrow table indicies") are
// no "int"s, but OB_TIndex"es. These is the internal representation
// of these indices. Currently, we'are defined as follows:
class OB_Entry;
typedef OB_Entry* OB_TIndex;
//
#define MakeOB_TIndex(ptr)		((OB_Entry*) ptr)
#define OB_TIndex2Ptr(ob_ti)		((void *) ob_ti)
#define OB_TIndex2Int(ob_ti)		(ToInt32((void *) ob_ti))

//
// Externally, OB_TIndex"es are represented as Ext_OB_TIndex:
typedef int Ext_OB_TIndex;
//
#define MakeExt_OB_TIndex(i)		((Ext_OB_TIndex) i)
#define Ext_OB_TIndex2Int(e_ob_ti)	((int) e_ob_ti)

#endif 
