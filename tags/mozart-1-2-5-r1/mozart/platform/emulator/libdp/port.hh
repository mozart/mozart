/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
 *    Erik Klintskog, 1998
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

#ifndef __PORT_HH
#define __PORT_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "value.hh"

extern TaggedRef BI_portWait;

class PortManager : public PortWithStream {
public:
  NO_DEFAULT_CONSTRUCTORS2(PortManager)
  PortManager() : PortWithStream(0,0) {} // hack
};

class PortProxy: public Port {
public:
  PendThread* pending;
  NO_DEFAULT_CONSTRUCTORS(PortProxy);
  PortProxy(int i): Port(0,Te_Proxy) { setIndex(i); pending = NULL;}
  Bool canSend();
  void wakeUp();
};

EntityCond getEntityCondPort(Tertiary* );
void port_Temp(PortProxy*);
void port_Ok(PortProxy*);
void port_Perm(PortProxy*);
PendThread *getPendThreadStartFromPort(Tertiary* t);
//
OZ_Return portSendImpl(Tertiary *p, TaggedRef msg);
void gcDistPortRecurseImpl(Tertiary *p);

#endif












