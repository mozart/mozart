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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
#include "space.hh"

//
Bool isPerdioInitializedStub()
{
  return (NO);
}

//
OZ_Return portSendStub(Tertiary *p, TaggedRef msg)
{
  OZ_error("'portSend' called without DP library?");
  return (PROCEED);
}
OZ_Return cellDoExchangeStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZ_error("'cellDoExchange' called without DP library?");
  return (PROCEED);
}
OZ_Return cellDoAccessStub(Tertiary*,TaggedRef)
{
  OZ_error("'cellDoAccess' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAtAccessStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZ_error("'cellAtAccess' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAtExchangeStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZ_error("'cellAtExchange' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAssignExchangeStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZ_error("'cellAssignExchange' called without DP library?");
  return (PROCEED);
}

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
// interface;
void lockLockProxyStub(Tertiary *t, Thread *thr)
{
  OZ_error("'lockLockProxy' called without DP library?");
}
void lockLockManagerOutlineStub(LockManagerEmul *lfu, Thread *thr)
{
  OZ_error("'lockLockManagerOutline' called without DP library?");
}
void unlockLockManagerOutlineStub(LockManagerEmul *lfu, Thread *thr)
{
  OZ_error("'unlockLockManagerOutline' called without DP library?");
}
void lockLockFrameOutlineStub(LockFrameEmul *lfu, Thread *thr)
{
  OZ_error("'lockLockFrameOutline' called without DP library?");
}
void unlockLockFrameOutlineStub(LockFrameEmul *lfu, Thread *thr)
{
  OZ_error("'unlockLockFrameOutline' called without DP library?");
}

//
Bool marshalTertiaryStub(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
{
  OZ_error("'marshalTertiary' called without DP library?");
  return (NO);
}
OZ_Term unmarshalTertiaryStub(MsgBuffer *bs, MarshalTag tag)
{
  OZ_error("'unmarshalTertiary' called without DP library?");
  return ((OZ_Term) 0);
}
OZ_Term unmarshalOwnerStub(MsgBuffer *bs,MarshalTag mt)
{
  OZ_error("'unmarshalOwner' called without DP library?");
  return ((OZ_Term) 0);
}
//
OZ_Term unmarshalVarStub(MsgBuffer*,Bool, Bool)
{
  OZ_error("'unmarshalVar' called without DP library?");
  return ((OZ_Term) 0);
}
Bool marshalVariableStub(TaggedRef*, MsgBuffer*)
{
  OZ_error("'marshalVariable' called without DP library?");
  return (NO);
}
void marshalObjectStub(ConstTerm *t, MsgBuffer *bs)
{
  OZ_error("'marshalObject' called without DP library?");
}
void marshalSPPStub(TaggedRef term, MsgBuffer *bs,Bool trail)
{
  OZ_error("'marshalSPP' called without DP library?");
}

// interface for GC;
void gcProxyRecurseStub(Tertiary *t)
{
  OZ_error("'gcProxyRecurse' called without DP library?");
}
void gcManagerRecurseStub(Tertiary *t)
{
  OZ_error("'gcManagerRecurse' called without DP library?");
}
ConstTerm* gcDistResourceStub(ConstTerm*)
{
  OZ_error("'gcDistResource' called without DP library?");
  return ((ConstTerm *) 0);
}

//
void gcDistCellRecurseStub(Tertiary *t)
{
  OZ_error("'gcDistCellRecurse' called without DP library?");
}
void gcDistLockRecurseStub(Tertiary *t)
{
  OZ_error("'gcDistLockRecurse' called without DP library?");
}
//
void gcDistPortRecurseStub(Tertiary *t)
{
  OZ_error("'gcDistPortRecurse' called without DP library?");
}

//
// (Only) gc method - for cells & locks (because we have to know
// whether they are accessible locally or not);
ConstTerm* auxGcDistCellStub(Tertiary *t)
{
  OZ_error("'auxGcDistCell' called without DP library?");
  return ((ConstTerm *) 0);
}
ConstTerm* auxGcDistLockStub(Tertiary *t)
{
  OZ_error("'auxGcDistLock' called without DP library?");
  return ((ConstTerm *) 0);
}

//
ConstTerm *gcStatefulSpecStub(Tertiary *t)
{
  OZ_error("'gcStatefulSpec' called without DP library?");
  return ((ConstTerm *) 0);
}

//
void gcBorrowTableUnusedFramesStub() {}
void gcFrameToProxyStub() {}

//
void gcPerdioFinalStub() {}
void gcPerdioRootsStub() {}
void gcEntityInfoStub(Tertiary *t)
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

//
// Link interface function pointers against stubs;

//
Bool (*isPerdioInitialized)() = isPerdioInitializedStub;

// 
OZ_Return (*portSend)(Tertiary *p, TaggedRef msg)
  = portSendStub;
OZ_Return (*cellDoExchange)(Tertiary*,TaggedRef,TaggedRef)
  = cellDoExchangeStub;
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
void (*lockLockManagerOutline)(LockManagerEmul *lfu, Thread *thr)
  = lockLockManagerOutlineStub;
void (*unlockLockManagerOutline)(LockManagerEmul *lfu, Thread *thr)
  = unlockLockManagerOutlineStub;
void (*lockLockFrameOutline)(LockFrameEmul *lfu, Thread *thr)
  = lockLockFrameOutlineStub;
void (*unlockLockFrameOutline)(LockFrameEmul *lfu, Thread *thr)
  = unlockLockFrameOutlineStub;
//
Bool (*marshalTertiary)(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
  = marshalTertiaryStub;
OZ_Term (*unmarshalTertiary)(MsgBuffer *bs, MarshalTag tag)
  = unmarshalTertiaryStub;
OZ_Term (*unmarshalOwner)(MsgBuffer *bs,MarshalTag mt)
  = unmarshalOwnerStub;
//
OZ_Term (*unmarshalVar)(MsgBuffer*,Bool,Bool)
  = unmarshalVarStub;
Bool (*marshalVariable)(TaggedRef*, MsgBuffer*)
  = marshalVariableStub;
void (*marshalObject)(ConstTerm *t, MsgBuffer *bs)
  = marshalObjectStub;
void (*marshalSPP)(TaggedRef term, MsgBuffer *bs,Bool trail)
  = marshalSPPStub;

//
void (*gcProxyRecurse)(Tertiary *t)
  = gcProxyRecurseStub;
void (*gcManagerRecurse)(Tertiary *t)
  = gcManagerRecurseStub;
ConstTerm* (*gcDistResource)(ConstTerm*)
  = gcDistResourceStub;
void (*gcDistCellRecurse)(Tertiary *t)
  = gcDistCellRecurseStub;
void (*gcDistLockRecurse)(Tertiary *t)
  = gcDistLockRecurseStub;
void (*gcDistPortRecurse)(Tertiary *t)
  = gcDistPortRecurseStub;

//
//
//
void (*gcBorrowTableUnusedFrames)()
  = gcBorrowTableUnusedFramesStub;
void (*gcFrameToProxy)()
  = gcFrameToProxyStub;

//
void (*gcPerdioFinal)()
  = gcPerdioFinalStub;
void (*gcPerdioRoots)()
  = gcPerdioRootsStub;
void (*gcEntityInfo)(Tertiary*)
  = gcEntityInfoStub;

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
