/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  $Log$
  Revision 1.3  1996/08/02 16:25:50  scheidhr
  more Perdio work: send Ports over the net

  Revision 1.2  1996/08/02 11:14:18  mehl
  perdio uses tcp now

  Revision 1.1  1996/07/26 15:17:44  mehl
  perdio communication: see ~mehl/perdio.oz

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

#endif /* __PERDIOHH */
