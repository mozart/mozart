/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
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

#ifndef __PROTOCOL_STATE_HH
#define __PROTOCOL_STATE_HH

#include "base.hh"
#include "value.hh"
#include "dpBase.hh"
//#include "dpMarshaler.hh"

void cellLockSendForward(DSite *toS,DSite *rS,int mI);
void cellLockSendGet(BorrowEntry*);
void cellLockSendDump(BorrowEntry*);
void cellLockReceiveForward(BorrowEntry*,DSite*,DSite*,int);
void cellLockReceiveDump(OwnerEntry*,DSite *);
void cellLockReceiveGet(OwnerEntry*,DSite *);

void cellReceiveGet(OwnerEntry* oe,CellManager*,DSite*);
void cellReceiveDump(CellManager*,DSite*);
void cellReceiveForward(BorrowEntry*,DSite*,DSite*,int);
void cellReceiveContentsManager(OwnerEntry*,TaggedRef,int);
void cellReceiveContentsFrame(BorrowEntry*,TaggedRef,DSite*,int);
void cellReceiveRemoteRead(BorrowEntry*,DSite*,int,DSite*);
void cellReceiveRead(OwnerEntry*,DSite*,DSite*);
void cellReceiveReadAns(Tertiary*,TaggedRef);
void cellReceiveCantPut(OwnerEntry*,TaggedRef,int,DSite*,DSite*);
void cellSendReadAns(DSite*,DSite*,int,TaggedRef);
void cellSendRemoteRead(DSite* toS,DSite* mS,int mI,DSite* fS,DSite*);
void cellSendContents(TaggedRef tr,DSite* toS,DSite *mS,int mI);
void cellSendRead(BorrowEntry *be,DSite *dS);
void cellSendContentsFailure(TaggedRef,DSite*,DSite*,int);

void lockReceiveGet(OwnerEntry* oe,LockManager*,DSite*);
void lockReceiveDump(LockManager*,DSite*);
void lockReceiveTokenManager(OwnerEntry*,int);
void lockReceiveTokenFrame(BorrowEntry*,DSite*,int);
void lockReceiveForward(BorrowEntry*,DSite*,DSite*,int);
void lockReceiveCantPut(OwnerEntry*,int,DSite*,DSite*);
void lockSendToken(DSite*,int,DSite*);
void lockReceiveCantPut(LockManager *cm,int mI,DSite* rsite, DSite* dS);
void lockSendForward(DSite *toS,DSite *fS,int mI);
void lockSendTokenFailure(DSite*,DSite*,int);
void lockSendDump(BorrowEntry*,LockFrame*);

void chainReceiveAck(OwnerEntry*, DSite*);
void chainReceiveAnswer(OwnerEntry*,DSite*,int,DSite*);
void chainReceiveQuestion(BorrowEntry*,DSite*,int,DSite*);
void chainSendAnswer(BorrowEntry*,DSite*,int,int,DSite*);
void chainSendQuestion(DSite*,int,DSite*);
void chainSendAck(DSite*,int);

void maybeChainSendQuestion(ChainElem*, Tertiary*, DSite*);

#endif // __PROTOCOL_STATE_HH





