/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Erik Klintskog (erik@sics.se) 
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

#ifndef __MSG_TYPE_HH
#define __MSG_TYPE_HH

//
// kost@ : those who update this, must update 'mess_names' as well!!

enum MessageType {
  M_NONE = 0,

  M_PORT_SEND,	
  M_ASK_FOR_CREDIT,     // OTI SITE
  M_OWNER_CREDIT,	// OTI CREDIT
  M_OWNER_SEC_CREDIT,	// NA CREDIT

  M_BORROW_CREDIT,      // NA  CREDIT
  M_REGISTER,           // OTI SITE
  M_REDIRECT,           // NA  DIF
  M_ACKNOWLEDGE,        // NA
  M_SURRENDER,          // OTI SITE DIF

  M_CELL_LOCK_GET,      // OTI* SITE
  M_CELL_LOCK_FORWARD,  // NA* INTEGER SITE
  M_CELL_LOCK_DUMP,     // OTI* SITE
  M_CELL_CONTENTS,      // NA* DIF

  M_CELL_READ,          // OTI* DIF 
  M_CELL_REMOTEREAD,    // NA* DIF
  M_CELL_READANS,
  M_CELL_CANTPUT,
  M_LOCK_TOKEN,          // NA* 

  M_LOCK_CANTPUT,
  M_CHAIN_ACK,
  M_CHAIN_QUESTION,
  M_CHAIN_ANSWER,
  M_ASK_ERROR,

  M_TELL_ERROR,
  M_GET_LAZY,           // OTI* LazyFlag SITE
  M_SEND_LAZY,          // SITE OTI* DIF

  M_UNASK_ERROR,

  M_GETSTATUS,              // OTI DIF
  M_SENDSTATUS,

  M_REQUESTED,
  M_DEREGISTER,
  M_UNUSED, // Empty spot, dont remove 
  M_PING,

  // Communication layer messages:
  C_FIRST,                  // Just for the index, must be first of C_-msgs

  C_PRESENT,                
  C_NEGOTIATE,
  C_NEGOTIATE_ANS,

  C_ACK,
  C_SET_ACK_PROP,

  C_CLOSE_HARD,
  C_CLOSE_WEAK,
  C_CLOSE_ACCEPT,
  C_CLOSE_REJECT,

  C_CLEAR_REFERENCE,

  M_LAST
};


extern char *mess_names[];

#endif




