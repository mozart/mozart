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

#ifndef __DPINTERFACE_HH
#define __DPINTERFACE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"

#define SIZEOFPORTPROXY (4*sizeof(void*))

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
extern LockRet (*lockLockManagerOutline)(LockManagerEmul *lfu, Thread *thr);
extern void (*unlockLockManagerOutline)(LockManagerEmul *lfu, Thread *thr);
extern LockRet (*lockLockFrameOutline)(LockFrameEmul *lfu, Thread *thr);
extern void (*unlockLockFrameOutline)(LockFrameEmul *lfu, Thread *thr);

//
extern void (*gCollectProxyRecurse)(Tertiary *t);
extern void (*gCollectManagerRecurse)(Tertiary *t);
extern ConstTerm* (*gCollectDistResource)(ConstTerm*);
extern void (*gCollectDistCellRecurse)(Tertiary *t);
extern void (*gCollectDistLockRecurse)(Tertiary *t);
extern void (*gCollectDistPortRecurse)(Tertiary *t);
//
extern void (*gCollectEntityInfo)(Tertiary*);
//
//

//
extern void (*gCollectPerdioStart)();
extern void (*gCollectPerdioRoots)();
extern void (*gCollectBorrowTableUnusedFrames)();
extern void (*gCollectPerdioFinal)();

// exit hook;
extern void (*dpExit)();

// hook to make changing of tcpcache-size dynamic
extern void (*changeTCPLimit)();

// distribution handlers
extern Bool (*distHandlerInstall)(unsigned short,unsigned short,
				       Thread*,TaggedRef, TaggedRef);
extern Bool (*distHandlerDeInstall)(unsigned short,unsigned short,
				       Thread*,TaggedRef, TaggedRef);
#endif // __DPINTERFACE_HH


