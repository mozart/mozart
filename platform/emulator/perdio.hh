/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  Log: $Log$
  Log: Revision 1.1  1996/07/26 15:17:44  mehl
  Log: perdio communication: see ~mehl/perdio.oz
  Log:

  ------------------------------------------------------------------------
*/

#ifndef __NETWORKERHH
#define __NETWORKERHH

/*
 * Format of messages:
 *   GSEND DIF    ; Send a message to the global port of the site
 */
enum MessageType {
  GSEND      // global send
};

class Site;

int siteInit();
void siteReceive();
int sendToSite(char *msg, int len, Site *site);
int sendToPort(char *msg, int len, char *host, int port);
int myPort();

#endif /* __NETWORKERHH */

