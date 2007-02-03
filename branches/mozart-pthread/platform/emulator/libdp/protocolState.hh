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

void cellLockSendForward(DSite *toS, DSite *rS, Ext_OB_TIndex);
void cellLockSendGet(BorrowEntry*);
void cellLockSendDump(BorrowEntry*);
void cellLockReceiveForward(BorrowEntry*, DSite*, DSite*, Ext_OB_TIndex);
void cellLockReceiveDump(OwnerEntry*,DSite *);
void cellLockReceiveGet(OwnerEntry*,DSite *);

void cellReceiveGet(OwnerEntry* oe,CellManager*,DSite*);
void cellReceiveDump(CellManager*,DSite*);
void cellReceiveForward(BorrowEntry*, DSite*, DSite*, Ext_OB_TIndex);
void cellReceiveContentsManager(OwnerEntry*, OZ_Term, Ext_OB_TIndex);
void cellReceiveContentsFrame(BorrowEntry*, OZ_Term, DSite*, Ext_OB_TIndex);
void cellReceiveRemoteRead(BorrowEntry*, DSite*, Ext_OB_TIndex, DSite*);
void cellReceiveRead(OwnerEntry*,DSite*,DSite*);
void cellReceiveReadAns(Tertiary*,OZ_Term);
void cellReceiveCantPut(OwnerEntry*, OZ_Term, Ext_OB_TIndex, DSite*, DSite*);
void cellSendReadAns(DSite*,DSite*,Ext_OB_TIndex,OZ_Term);
void cellSendRemoteRead(DSite *toS, DSite *mS, Ext_OB_TIndex mI,
			DSite *fS, DSite*);
void cellSendContents(OZ_Term tr, DSite* toS, DSite *mS, Ext_OB_TIndex);
void cellSendRead(BorrowEntry *be,DSite *dS);
void cellSendContentsFailure(OZ_Term, DSite*, DSite*, Ext_OB_TIndex);

void lockReceiveGet(OwnerEntry* oe,LockManager*,DSite*);
void lockReceiveDump(LockManager*,DSite*);
void lockReceiveTokenManager(OwnerEntry*,Ext_OB_TIndex);
void lockReceiveTokenFrame(BorrowEntry*, DSite*, Ext_OB_TIndex);
void lockReceiveForward(BorrowEntry*,DSite*,DSite*, Ext_OB_TIndex);
void lockReceiveCantPut(OwnerEntry*, Ext_OB_TIndex, DSite*, DSite*);
void lockSendToken(DSite*, Ext_OB_TIndex, DSite*);
void lockReceiveCantPut(LockManager *cm, Ext_OB_TIndex, 
			DSite* rsite, DSite* dS);
void lockSendForward(DSite *toS,DSite *fS,OB_TIndex);
void lockSendTokenFailure(DSite*, DSite*, Ext_OB_TIndex);
void lockSendDump(BorrowEntry*,LockFrame*);

void chainReceiveAck(OwnerEntry*, DSite*);
void chainReceiveAnswer(OwnerEntry*, DSite*, Ext_OB_TIndex, DSite*);
void chainReceiveQuestion(BorrowEntry*, DSite*, Ext_OB_TIndex, DSite*);
void chainSendAnswer(BorrowEntry*, DSite*, Ext_OB_TIndex, int, DSite*);
void chainSendQuestion(DSite*, Ext_OB_TIndex, DSite*);
void chainSendAck(DSite*,Ext_OB_TIndex);

void maybeChainSendQuestion(ChainElem*, Tertiary*, DSite*);

#endif // __PROTOCOL_STATE_HH





