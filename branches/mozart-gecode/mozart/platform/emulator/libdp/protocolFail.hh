/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
 *    Erik Klintskog, 1998
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

#ifndef __PROTOCOLFAILHH
#define __PROTOCOLFAILHH

void receiveAskError(OwnerEntry*,DSite*,EntityCond);
void sendAskError(BorrowEntry*, EntityCond);
void receiveTellError(BorrowEntry*, EntityCond, Bool);
void receiveAskError(OwnerEntry *,DSite*,EntityCond);
void receiveUnAskError(OwnerEntry *,DSite*,EntityCond);
void sendTellError(OwnerEntry *,DSite*,EntityCond,Bool);
void sendUnAskError(BorrowEntry*,EntityCond);
#endif



