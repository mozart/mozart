/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __dp_msg_type_hh
#define __dp_msg_type_hh

#include "pickle.hh"

//
// kost@ : those who update this, must update 'mess_names' as well!!

enum MessageType {
  M_NONE = 0,

  M_PORT_SEND,
  M_REMOTE_SEND,        // OTI STRING DIF (implicit 1 credit)
  M_ASK_FOR_CREDIT,     // OTI SITE (implicit 1 credit)
  M_OWNER_CREDIT,       // OTI CREDIT
  M_OWNER_SEC_CREDIT,   // NA CREDIT

  M_BORROW_CREDIT,      // NA  CREDIT
  M_REGISTER,           // OTI SITE (implicit 1 credit)
  M_REDIRECT,           // NA  DIF
  M_ACKNOWLEDGE,        // NA (implicit 1 credit)
  M_SURRENDER,          // OTI SITE DIF (implicit 1 credit)

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
  M_GET_OBJECT,         // OTI* SITE
  M_GET_OBJECTANDCLASS, // OTI* SITE
  M_SEND_OBJECT,        //
  M_SEND_OBJECTANDCLASS,//

  M_FILE = PERDIOMAGICSTART ,          // HACK ALERT see componentBuffer.cc
  M_EXPORT,
  M_UNASK_ERROR,
  M_SEND_GATE,

  M_LAST
};


extern char *mess_names[];


// the names of the difs for statistics


enum {
  MISC_STRING,
  MISC_GNAME,
  MISC_SITE,

  MISC_LAST
};

extern char* misc_names[];

class SendRecvCounter {
private:
  long c[2];
public:
  SendRecvCounter() { c[0]=0; c[1]=0; }
  void send() { c[0]++; }
  long getSend() { return c[0]; }
  void recv() { c[1]++; }
  long getRecv() { return c[1]; }
};

extern SendRecvCounter mess_counter[];

#endif
