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

//
void marshalTertiaryStub(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
{
  OZD_error("'marshalTertiary' called without DP library?");
}
#ifdef USE_FAST_UNMARSHALER
OZ_Term unmarshalTertiaryStub(MsgBuffer *bs, MarshalTag tag)
{
  OZD_error("'unmarshalTertiary' called without DP library?");
  return ((OZ_Term) 0);
}
OZ_Term unmarshalOwnerStub(MsgBuffer *bs,MarshalTag mt)
{
  OZD_error("'unmarshalOwner' called without DP library?");
  return ((OZ_Term) 0);
}
OZ_Term unmarshalVarStub(MsgBuffer*,Bool, Bool)
{
  OZD_error("'unmarshalVar' called without DP library?");
  return ((OZ_Term) 0);
}
#else
OZ_Term unmarshalTertiaryRobustStub(MsgBuffer *bs, MarshalTag tag, int *error)
{
  OZ_error("'unmarshalTertiaryRobust' called without DP library?");
  return ((OZ_Term) 0);
}
OZ_Term unmarshalOwnerRobustStub(MsgBuffer *bs,MarshalTag mt, int *error)
{
  OZ_error("'unmarshalOwnerRobust' called without DP library?");
  return ((OZ_Term) 0);
}
//
OZ_Term unmarshalVarRobustStub(MsgBuffer*,Bool, Bool, int *error)
{
  OZ_error("'unmarshalVarRobust' called without DP library?");
  return ((OZ_Term) 0);
}
#endif
Bool marshalVariableStub(TaggedRef*, MsgBuffer*)
{
  OZD_error("'marshalVariable' called without DP library?");
  return (NO);
}
Bool triggerVariableStub(TaggedRef*){
  OZD_error("'triggerVariable' called without DP library?");
  return (NO);
}
  
void marshalObjectStub(ConstTerm *t, MsgBuffer *bs)
{
  OZD_error("'marshalObject' called without DP library?");
}
void marshalSPPStub(TaggedRef term, MsgBuffer *bs,Bool trail)
{
  OZD_error("'marshalSPP' called without DP library?");
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
void gCollectBorrowTableUnusedFramesStub() {}
void gCollectFrameToProxyStub() {}

//
void gCollectPerdioFinalStub() {}
void gCollectPerdioRootsStub() {}
void gCollectEntityInfoStub(Tertiary *t)
{
  Assert(t->getInfo() == (EntityInfo *) 0);
}

// exit hook;
void dpExitStub() {;}


// Debug stuff;
#ifdef DEBUG_CHECK
void maybeDebugBufferGetStub(BYTE b) {}
void maybeDebugBufferPutStub(BYTE b) {}
#endif

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
void (*marshalTertiary)(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
  = marshalTertiaryStub;
#ifdef USE_FAST_UNMARSHALER
OZ_Term (*unmarshalTertiary)(MsgBuffer *bs, MarshalTag tag)
  = unmarshalTertiaryStub;
OZ_Term (*unmarshalOwner)(MsgBuffer *bs,MarshalTag mt)
  = unmarshalOwnerStub;
//
OZ_Term (*unmarshalVar)(MsgBuffer*,Bool,Bool)
  = unmarshalVarStub;
#else
OZ_Term (*unmarshalTertiaryRobust)(MsgBuffer *bs, MarshalTag tag,int *error)
  = unmarshalTertiaryRobustStub;
OZ_Term (*unmarshalOwnerRobust)(MsgBuffer *bs,MarshalTag mt,int *error)
  = unmarshalOwnerRobustStub;
OZ_Term (*unmarshalVarRobust)(MsgBuffer*,Bool,Bool,int*)
  = unmarshalVarRobustStub;
#endif
Bool (*marshalVariable)(TaggedRef*, MsgBuffer*)
  = marshalVariableStub;
Bool (*triggerVariable)(TaggedRef*)
  = triggerVariableStub;
void (*marshalObject)(ConstTerm *t, MsgBuffer *bs)
  = marshalObjectStub;
void (*marshalSPP)(TaggedRef term, MsgBuffer *bs,Bool trail)
  = marshalSPPStub;

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
//
//
void (*gCollectBorrowTableUnusedFrames)()
  = gCollectBorrowTableUnusedFramesStub;
void (*gCollectFrameToProxy)()
  = gCollectFrameToProxyStub;

//
void (*gCollectPerdioFinal)()
  = gCollectPerdioFinalStub;
void (*gCollectPerdioRoots)()
  = gCollectPerdioRootsStub;
void (*gCollectEntityInfo)(Tertiary*)
  = gCollectEntityInfoStub;

// exit hook;
void (*dpExit)()
  = dpExitStub;

// Debug stuff;
#ifdef DEBUG_CHECK
void (*maybeDebugBufferGet)(BYTE b)
  = maybeDebugBufferGetStub;
void (*maybeDebugBufferPut)(BYTE b)
  = maybeDebugBufferPutStub;
#endif

// distribution handlers

Bool (*distHandlerInstall)(unsigned short,unsigned short,Thread*,
				TaggedRef, TaggedRef)
  = distHandlerInstallStub;

Bool (*distHandlerDeInstall)(unsigned short,unsigned short,Thread*,
				  TaggedRef, TaggedRef)
  = distHandlerDeInstallStub;


