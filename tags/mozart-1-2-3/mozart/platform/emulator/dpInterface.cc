/*
 *  Authors:
 *    Per Brand, Konstantin Popov
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Per Brand, Konstantin Popov 1998
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

#ifdef INTERFACE
#pragma interface "dpInterface.hh"
#endif

#include "base.hh"
#include "dpInterface.hh"
#include "value.hh"
#include "os.hh"

//
Bool isPerdioInitializedStub()
{
  return (NO);
}

//
OZ_Return portSendStub(Tertiary *p, TaggedRef msg)
{
  OZD_error("'portSend' called without DP library?");
  return (PROCEED);
}
OZ_Return cellDoExchangeStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZD_error("'cellDoExchange' called without DP library?");
  return (PROCEED);
}
OZ_Return objectExchangeStub(Tertiary*,TaggedRef,TaggedRef,TaggedRef)
{
  OZD_error("'objectExchange' called without DP library?");
  return (PROCEED);
}
OZ_Return cellDoAccessStub(Tertiary*,TaggedRef)
{
  OZD_error("'cellDoAccess' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAtAccessStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAtAccess' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAtExchangeStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAtExchange' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAssignExchangeStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAssignExchange' called without DP library?");
  return (PROCEED);
}

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
// interface;
void lockLockProxyStub(Tertiary *t, Thread *thr)
{
  OZD_error("'lockLockProxy' called without DP library?");
}
LockRet lockLockManagerOutlineStub(LockManagerEmul *lfu, Thread *thr)
{
  OZD_error("'lockLockManagerOutline' called without DP library?");
  return LOCK_WAIT;
}
void unlockLockManagerOutlineStub(LockManagerEmul *lfu, Thread *thr)
{
  OZD_error("'unlockLockManagerOutline' called without DP library?");
}
LockRet lockLockFrameOutlineStub(LockFrameEmul *lfu, Thread *thr)
{
  OZD_error("'lockLockFrameOutline' called without DP library?");
  return LOCK_WAIT;
}
void unlockLockFrameOutlineStub(LockFrameEmul *lfu, Thread *thr)
{
  OZD_error("'unlockLockFrameOutline' called without DP library?");
}

// interface for GC;
void gCollectProxyRecurseStub(Tertiary *t)
{
  OZD_error("'gCollectProxyRecurse' called without DP library?");
}
void gCollectManagerRecurseStub(Tertiary *t)
{
  OZD_error("'gCollectManagerRecurse' called without DP library?");
}
ConstTerm* gCollectDistResourceStub(ConstTerm*)
{
  OZD_error("'gCollectDistResource' called without DP library?");
  return ((ConstTerm *) 0);
}

//
void gCollectDistCellRecurseStub(Tertiary *t)
{
  OZD_error("'gCollectDistCellRecurse' called without DP library?");
}
void gCollectDistLockRecurseStub(Tertiary *t)
{
  OZD_error("'gCollectDistLockRecurse' called without DP library?");
}
//
void gCollectDistPortRecurseStub(Tertiary *t)
{
  OZD_error("'gCollectDistPortRecurse' called without DP library?");
}

//
// (Only) gCollect method - for cells & locks (because we have to know
// whether they are accessible locally or not);
ConstTerm* auxGCollectDistCellStub(Tertiary *t)
{
  OZD_error("'auxGCollectDistCell' called without DP library?");
  return ((ConstTerm *) 0);
}
ConstTerm* auxGCollectDistLockStub(Tertiary *t)
{
  OZD_error("'auxGCollectDistLock' called without DP library?");
  return ((ConstTerm *) 0);
}

//
ConstTerm *gCollectStatefulSpecStub(Tertiary *t)
{
  OZD_error("'gCollectStatefulSpec' called without DP library?");
  return ((ConstTerm *) 0);
}

//
void gCollectEntityInfoStub(Tertiary *t)
{
  Assert(t->getInfo() == (EntityInfo *) 0);
}

//
void gCollectPerdioStartStub() {}
void gCollectPerdioFinalStub() {}
void gCollectBorrowTableUnusedFramesStub() {}
void gCollectPerdioRootsStub() {}

// exit hook;
void dpExitStub() {;}

// hook to make changing of tcpcache-size dynamic
void changeTCPLimitStub() {;}

Bool distHandlerInstallStub(unsigned short x,unsigned short y,
				 Thread* th,TaggedRef a,TaggedRef b){
  OZD_error("'distHandlerInstall' called without DP library?");  
  return PROCEED;}

Bool distHandlerDeInstallStub(unsigned short x,unsigned short y,
				   Thread* th,TaggedRef a,TaggedRef b){
  OZD_error("'distHandlerDeInstall' called without DP library?");  
  return PROCEED;}

//
// Link interface function pointers against stubs;

//
Bool (*isPerdioInitialized)() = isPerdioInitializedStub;

// 
OZ_Return (*portSend)(Tertiary *p, TaggedRef msg)
  = portSendStub;
OZ_Return (*cellDoExchange)(Tertiary*,TaggedRef,TaggedRef)
  = cellDoExchangeStub;
OZ_Return (*objectExchange)(Tertiary*,TaggedRef,TaggedRef,TaggedRef)
  = objectExchangeStub;
OZ_Return (*cellDoAccess)(Tertiary*,TaggedRef)
  = cellDoAccessStub;
OZ_Return (*cellAtAccess)(Tertiary*,TaggedRef,TaggedRef)
  = cellAtAccessStub;
OZ_Return (*cellAtExchange)(Tertiary*,TaggedRef,TaggedRef)
  = cellAtExchangeStub;
OZ_Return (*cellAssignExchange)(Tertiary*,TaggedRef,TaggedRef)
  = cellAssignExchangeStub;

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
void (*lockLockProxy)(Tertiary *t, Thread *thr)
  = lockLockProxyStub;
LockRet (*lockLockManagerOutline)(LockManagerEmul *lfu, Thread *thr)
  = lockLockManagerOutlineStub;
void (*unlockLockManagerOutline)(LockManagerEmul *lfu, Thread *thr)
  = unlockLockManagerOutlineStub;
LockRet (*lockLockFrameOutline)(LockFrameEmul *lfu, Thread *thr)
  = lockLockFrameOutlineStub;
void (*unlockLockFrameOutline)(LockFrameEmul *lfu, Thread *thr)
  = unlockLockFrameOutlineStub;

//
void (*gCollectProxyRecurse)(Tertiary *t)
  = gCollectProxyRecurseStub;
void (*gCollectManagerRecurse)(Tertiary *t)
  = gCollectManagerRecurseStub;
ConstTerm* (*gCollectDistResource)(ConstTerm*)
  = gCollectDistResourceStub;
void (*gCollectDistCellRecurse)(Tertiary *t)
  = gCollectDistCellRecurseStub;
void (*gCollectDistLockRecurse)(Tertiary *t)
  = gCollectDistLockRecurseStub;
void (*gCollectDistPortRecurse)(Tertiary *t)
  = gCollectDistPortRecurseStub;
//
void (*gCollectEntityInfo)(Tertiary*)
  = gCollectEntityInfoStub;

//
void (*gCollectPerdioStart)()
  = gCollectPerdioStartStub;
void (*gCollectPerdioRoots)()
  = gCollectPerdioRootsStub;
void (*gCollectBorrowTableUnusedFrames)()
  = gCollectBorrowTableUnusedFramesStub;
void (*gCollectPerdioFinal)()
  = gCollectPerdioFinalStub;

// exit hook;
void (*dpExit)()
  = dpExitStub;

// hook to make changing of tcpcache-size dynamic
void (*changeTCPLimit)()
  = changeTCPLimitStub;

// distribution handlers

Bool (*distHandlerInstall)(unsigned short,unsigned short,Thread*,
				TaggedRef, TaggedRef)
  = distHandlerInstallStub;

Bool (*distHandlerDeInstall)(unsigned short,unsigned short,Thread*,
				  TaggedRef, TaggedRef)
  = distHandlerDeInstallStub;


