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
  "update_reference",
  "ref_to_owner",
  "ref_to_borrow",

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
  "pong_term",
  "pong_pl",
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

  "conn_clearreference",
  "conn_ping"
};

int default_mess_priority[M_LAST] = {
  MSG_PRIO_MEDIUM, //none
    
  MSG_PRIO_MEDIUM, //port_send
  MSG_PRIO_MEDIUM, //update_reference
  MSG_PRIO_MEDIUM, //ref_to_owner
  MSG_PRIO_MEDIUM, //ref_to_borrow

  MSG_PRIO_MEDIUM, //register
  MSG_PRIO_MEDIUM, //redirect
  MSG_PRIO_MEDIUM, //acknowledge
  MSG_PRIO_MEDIUM, //surrender

  MSG_PRIO_MEDIUM, //cell_lock_get
  MSG_PRIO_MEDIUM, //cell_lock_forward
  MSG_PRIO_MEDIUM, //cell_lock_dump
  MSG_PRIO_MEDIUM, //cell_contents

  MSG_PRIO_MEDIUM, //cell_read
  MSG_PRIO_MEDIUM, //cell_remoteread
  MSG_PRIO_MEDIUM, //cell_readans
  MSG_PRIO_MEDIUM, //cell_cantput
  MSG_PRIO_MEDIUM, //lock_token

  MSG_PRIO_MEDIUM, //lock_cantput
  MSG_PRIO_MEDIUM, //chain_ack
  MSG_PRIO_MEDIUM, //chain_question
  MSG_PRIO_MEDIUM, //chain_answer
  MSG_PRIO_MEDIUM, //ask_error

  MSG_PRIO_MEDIUM, //tell_error
  MSG_PRIO_MEDIUM, //get_lazy
  MSG_PRIO_MEDIUM, //send_lazy

  MSG_PRIO_MEDIUM, //unask_error

  MSG_PRIO_MEDIUM, //get_status
  MSG_PRIO_MEDIUM, //send_status

  MSG_PRIO_MEDIUM, //requested
  MSG_PRIO_MEDIUM, //deregister
  MSG_PRIO_MEDIUM, //send_ping
  MSG_PRIO_HIGH,  //ping
  MSG_PRIO_MEDIUM,//pong 
  MSG_PRIO_MEDIUM,//pong

  MSG_PRIO_EAGER, //conn_first_NOT_A_VALID_MESSAGETYPE

  MSG_PRIO_EAGER, //conn_present
  MSG_PRIO_EAGER, //conn_negotiate
  MSG_PRIO_EAGER, //conn_negotiate_answer

  MSG_PRIO_EAGER, //conn_ack
  MSG_PRIO_EAGER, //conn_set_ack_properties

  MSG_PRIO_EAGER, //conn_close_hard
  MSG_PRIO_EAGER, //conn_close_weak
  MSG_PRIO_EAGER, //conn_close_accept
  MSG_PRIO_EAGER, //conn_close_reject
  
  MSG_PRIO_EAGER, //conn_clearreference
  MSG_PRIO_EAGER
};
