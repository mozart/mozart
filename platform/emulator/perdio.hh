/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  SICS
  Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
  Author: brand,scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __PERDIOHH
#define __PERDIOHH

#include "tagged.hh"

#define DummyClassConstruction(X) \
X();  \
~X(); \
(X&);

enum MessageType {
  SITESEND,          // send to site
  PORTSEND,           // send to port
  ASK_FOR_CREDIT,
  OWNER_CREDIT,
  BORROW_CREDIT
};

int remoteSend(PortProxy *p, TaggedRef msg);
void networkSiteDec(int sd);

#define tert2PortManager(t)   ((PortManager*) t)
#define tert2PortLocal(t)     ((PortLocal*) t)
#define tert2PortProxy(t)     ((PortProxy*) t)


void gcOwnerTable();
void gcBorrowTable();
void gcPortProxy(PortProxy* );
void gcPortManager(PortManager* );

#endif /* __PERDIOHH */
