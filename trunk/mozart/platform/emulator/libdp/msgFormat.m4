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
message(M_PORT_SEND,Index,Term)

message(M_UPDATE_REFERENCE,Index)
message(M_OWNER_REF,Index,Credit,Int,Int2)
message(M_BORROW_REF,Term)

message(M_REGISTER,Index)
message(M_DEREGISTER,Index)
message(M_REDIRECT,Site,Index,Term)
message(M_ACKNOWLEDGE,Index)
message(M_SURRENDER,Index,Term)
message(M_GETSTATUS,Index)
message(M_SENDSTATUS,Site,Index,Term)

message(M_CELL_LOCK_GET,Index,Site)
message(M_CELL_LOCK_FORWARD,Site,Index,Site2)
message(M_CELL_LOCK_DUMP,Index,Site)

message(M_CELL_CONTENTS,Site,Index,Term)
message(M_CELL_READ,Index,Site)
message(M_CELL_REMOTEREAD,Site,Index,Site2)
message(M_CELL_READANS,Site,Index,Term)

message(M_LOCK_TOKEN,Site,Index)

message(M_CELL_CANTPUT,Index,Site,Term,Site2)
message(M_LOCK_CANTPUT,Index,Site,Site2)
message(M_CHAIN_ACK,Index,Site)
message(M_CHAIN_QUESTION,Index,Site,Site2)
message(M_CHAIN_ANSWER,Index,Site,Int,Site2)
message(M_ASK_ERROR,Index,Site,Int);
message(M_TELL_ERROR,Site,Index,Int,Int2);

message(M_GET_LAZY,Index,Int,Site)
message(M_SEND_LAZY,Site,Index,Int,FullTopTerm)

message(M_UNASK_ERROR,Index,Site,Int)

message(M_UNUSED)

message(M_PING)

message(M_PONG_TERM,Site,Int,Int2,Term)
message(M_PONG_PL,Site,Int,Int2)

message(C_PRESENT,String,Site)        // String is Version
message(C_NEGOTIATE,String,Site,Term)
message(C_NEGOTIATE_ANS,Term)
message(C_SEND_PING_PONG,Int,Int2)  

message(C_ACK)
message(C_SET_ACK_PROP,Int,Int2)      // time, length
  
message(C_CLOSE_HARD)
message(C_CLOSE_WEAK)
message(C_CLOSE_ACCEPT)
message(C_CLOSE_REJECT)

message(C_CLEAR_REFERENCE)
