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

#ifndef __DPINTERFACE_HH
#define __DPINTERFACE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "pickle.hh"

#define SIZEOFPORTPROXY 16

//
extern Bool (*isPerdioInitialized)();

//
extern OZ_Return (*portSend)(Tertiary *p, TaggedRef msg);
extern OZ_Return (*cellDoExchange)(Tertiary*,TaggedRef,TaggedRef);
extern OZ_Return (*cellDoAccess)(Tertiary*,TaggedRef);
extern OZ_Return (*cellAtAccess)(Tertiary*,TaggedRef,TaggedRef);
extern OZ_Return (*cellAtExchange)(Tertiary*,TaggedRef,TaggedRef);
extern OZ_Return (*cellAssignExchange)(Tertiary*,TaggedRef,TaggedRef);
extern OZ_Return (*objectExchange) (Tertiary*,TaggedRef,TaggedRef,TaggedRef);

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
extern void (*lockLockProxy)(Tertiary *t, Thread *thr);
extern void (*lockLockManagerOutline)(LockManagerEmul *lfu, Thread *thr);
extern void (*unlockLockManagerOutline)(LockManagerEmul *lfu, Thread *thr);
extern void (*lockLockFrameOutline)(LockFrameEmul *lfu, Thread *thr);
extern void (*unlockLockFrameOutline)(LockFrameEmul *lfu, Thread *thr);

//
extern Bool (*marshalTertiary)(Tertiary *t, MarshalTag tag, MsgBuffer *bs);
extern OZ_Term (*unmarshalTertiary)(MsgBuffer *bs, MarshalTag tag);
extern OZ_Term (*unmarshalOwner)(MsgBuffer *bs,MarshalTag mt);
//
extern OZ_Term (*unmarshalVar)(MsgBuffer*,Bool,Bool);
extern Bool (*marshalVariable)(TaggedRef*, MsgBuffer*);
extern void (*marshalObject)(ConstTerm *t, MsgBuffer *bs);
extern void (*marshalSPP)(TaggedRef term, MsgBuffer *bs,Bool trail);

//
extern void (*gcProxyRecurse)(Tertiary *t);
extern void (*gcManagerRecurse)(Tertiary *t);
extern ConstTerm* (*gcDistResource)(ConstTerm*);
extern void (*gcDistCellRecurse)(Tertiary *t);
extern void (*gcDistLockRecurse)(Tertiary *t);
extern void (*gcDistPortRecurse)(Tertiary *t);
//
//
//
extern void (*gcBorrowTableUnusedFrames)();
extern void (*gcFrameToProxy)();

//
extern void (*gcPerdioFinal)();
extern void (*gcPerdioRoots)();
extern void (*gcEntityInfo)(Tertiary*);

// exit hook;
extern void (*dpExit)();

// Debug stuff;
#ifdef DEBUG_CHECK
extern void (*maybeDebugBufferGet)(BYTE b);
extern void (*maybeDebugBufferPut)(BYTE b);
#endif

#endif // __DPINTERFACE_HH
