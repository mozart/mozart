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

#include "base.hh"
#include "msgType.hh"
#include "dpMarshaler.hh"

SendRecvCounter mess_counter[M_LAST];

char *mess_names[M_LAST] = {
  "none",

  "port_send",
  "ask_for_credit",
  "owner_credit",
  "owner_sec_credit",
  
  "borrow_credit",
  "register",
  "redirect",
  "acknowledge",
  "surrender",

  "cell_lock_get",
  "cell_lock_forward",
  "cell_lock_dump",
  "cell_contents",

  "cell_read",
  "cell_remoteread",
  "cell_readans",
  "cell_cantput",
  "lock_token",

  "lock_cantput",
  "chain_ack",
  "chain_question",
  "chain_answer",
  "ask_error",

  "tell_error",
  "get_lazy",
  "send_lazy",

  "unask_error",

  "get_status",
  "send_status",

  "requested",
  "deregister",
  "send_ping",
  "ping",

  "conn_first_NOT_A_VALID_MESSAGETYPE",

  "conn_present",
  "conn_negotiate",
  "conn_negotiate_answer",

  "conn_ack",
  "conn_set_ack_properties",

  "conn_close_hard",
  "conn_close_weak",
  "conn_close_accept",
  "conn_close_reject",

  "conn_clearreference"
};
