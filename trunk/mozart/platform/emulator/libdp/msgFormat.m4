/* -*- C++ -*-
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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

// Strings are limited to MAX_DP_STRING
message(M_PORT_SEND,3,Index,Term)

message(M_ASK_FOR_CREDIT,3,Index,Site)
message(M_OWNER_CREDIT,3,Index,Credit)
message(M_OWNER_SEC_CREDIT,3,Site,Index,Credit) 
message(M_BORROW_CREDIT,3,Site,Index,Credit)

message(M_REGISTER,3,Index,Site)
message(M_DEREGISTER,3,Index,Site)
message(M_REDIRECT,3,Site,Index,Term)
message(M_ACKNOWLEDGE,3,Site,Index)
message(M_SURRENDER,3,Index,Site,Term)
message(M_GETSTATUS,3,Site,Index)
message(M_SENDSTATUS,3,Site,Index,Term)

message(M_CELL_LOCK_GET,3,Index,Site)
message(M_CELL_LOCK_FORWARD,3,Site,Index,Site2)
message(M_CELL_LOCK_DUMP,3,Index,Site)

message(M_CELL_CONTENTS,3,Site,Index,Term)
message(M_CELL_READ,3,Index,Site)
message(M_CELL_REMOTEREAD,3,Site,Index,Site2)
message(M_CELL_READANS,3,Site,Index,Term)

message(M_LOCK_TOKEN,3,Site,Index)

message(M_CELL_CANTPUT,3,Index,Site,Term,Site2)
message(M_LOCK_CANTPUT,3,Index,Site,Site2)
message(M_CHAIN_ACK,3,Index,Site)
message(M_CHAIN_QUESTION,3,Index,Site,Site2)
message(M_CHAIN_ANSWER,3,Index,Site,Index2,Site2)
message(M_ASK_ERROR,3,Index,Site,Index2);
message(M_TELL_ERROR,3,Site,Index,Index2,Index3);

message(M_GET_LAZY,3,Index,Index2,Site)
message(M_SEND_LAZY,3,Site,Index,Index2,FullTopTerm)

message(M_UNASK_ERROR,3,Index,Site,Index2)

message(M_REQUESTED,3,Index)

message(M_PING,4)

message(C_PRESENT,5,String,Site)        //String is Version
message(C_NEGOTIATE,5,String,Site,Term)
message(C_NEGOTIATE_ANS,5,Term)

message(C_ACK,5)
message(C_SET_ACK_PROP,5,Index,Index2)  // time, length
  
message(C_CLOSE_HARD,5)
message(C_CLOSE_WEAK,5)
message(C_CLOSE_ACCEPT,5)
message(C_CLOSE_REJECT,5)

message(C_CLEAR_REFERENCE,5)
