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

// Values: literal, list, records, ...

#ifndef __DPINTERFACE_HH
#define __DPINTERFACE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "pickle.hh"

#define SIZEOFPORTPROXY 12
//
Bool isPerdioInitialized();

// interface;
OZ_Return portSend(Tertiary *p, TaggedRef msg);
OZ_Return cellDoExchange(Tertiary*,TaggedRef,TaggedRef);
OZ_Return cellDoAccess(Tertiary*,TaggedRef);
OZ_Return cellAtAccess(Tertiary*,TaggedRef,TaggedRef);
OZ_Return cellAtExchange(Tertiary*,TaggedRef,TaggedRef);
OZ_Return cellAssignExchange(Tertiary*,TaggedRef,TaggedRef);

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
// interface;
void lockLockProxy(Tertiary *t, Thread *thr);
void lockLockManagerOutline(LockManagerEmul *lfu, Thread *thr);
void unlockLockManagerOutline(LockManagerEmul *lfu, Thread *thr);
void lockLockFrameOutline(LockFrameEmul *lfu, Thread *thr);
void unlockLockFrameOutline(LockFrameEmul *lfu, Thread *thr);

//
Bool marshalTertiary(Tertiary *t, MarshalTag tag, MsgBuffer *bs);
OZ_Term unmarshalTertiary(MsgBuffer *bs, MarshalTag tag);
OZ_Term unmarshalOwner(MsgBuffer *bs,MarshalTag mt);
//
OZ_Term unmarshalVar(MsgBuffer*);
Bool marshalVariable(TaggedRef*, MsgBuffer*);
void marshalObject(ConstTerm *t, MsgBuffer *bs);
void marshalSPP(TaggedRef term, MsgBuffer *bs,Bool trail);
//
// Perdio variables;
Bool perdioVarValid(OzVariable *cv, TaggedRef val);
OZ_Return perdioVarUnify(OzVariable *cv, TaggedRef *ptr,
			 TaggedRef val, ByteCode *scp);
OZ_Return perdioVarBind(OzVariable *cv, TaggedRef *ptr,
			TaggedRef val, ByteCode *scp);
void perdioVarAddSusp(OzVariable *cv, TaggedRef *v,
		      Suspension susp, int unstable);
OzVariable* gcCopyPerdioVar(OzVariable *cv);
void gcPerdioVarRecurse(OzVariable *cv);
void perdioVarPrint(OzVariable *cv, ostream &out, int depth);
VariableStatus perdioVarStatus(OzVariable *cv);
OZ_Term perdioVarIsDet(OzVariable *cv);

// interface for GC;
void gcProxyRecurse(Tertiary *t);
void gcManagerRecurse(Tertiary *t);
ConstTerm* gcDistResource(ConstTerm*);

//
void gcDistCellRecurse(Tertiary *t);
void gcDistLockRecurse(Tertiary *t);
//
void gcDistPortRecurse(Tertiary *t);
//
// (Only) gc method - for cells & locks (because we have to know
// whether they are accessible locally or not);
ConstTerm* auxGcDistCell(Tertiary *t);
ConstTerm* auxGcDistLock(Tertiary *t);

//
ConstTerm *gcStatefulSpec(Tertiary *t);

//
void gcBorrowTableUnusedFrames();
void gcFrameToProxy();

//
void gcPendThread(PendThread**);
void gcPerdioFinal();
void gcPerdioRoots();
void gcEntityInfo(Tertiary*);

//
// kost@ : PER-LOOK: i don't see where these two procedures are either
// used or defined;
void lockGlobalize(LockLocal*, int);
void cellGlobalize(CellLocal*, int);

// exit hook;
void dpExit();

// Debug stuff;
void maybeDebugBufferGet(BYTE b);
void maybeDebugBufferPut(BYTE b);

//
// kost@ : temporarily interface methods;
// Ralf will fix it;
Bool isPerdioInitialized();
void perdioInitLocal();

#endif // __DPINTERFACE_HH

