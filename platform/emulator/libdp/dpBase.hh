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
class DSiteHashTable;
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
class MsgTermSnapshot;

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

#endif 
