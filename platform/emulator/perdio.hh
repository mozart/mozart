/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __PERDIOHH
#define __PERDIOHH

#include "tagged.hh"

/*
 * Format of messages:
 *   GSEND DIF    ; Send a message to the global port of the site
 */
enum MessageType {
  GSEND           // global send
};

void remoteSend(Port *p, TaggedRef msg);
Bool isLocalAddress(NetAddress *na);

#endif /* __PERDIOHH */
