/*
 *  Authors:
 *    Konstantin Popov
 * 
 *  Contributors:
 *
 *  Copyright:
 *    Konstantin Popov 1997-1998
 * 
 *  Last change:
 *    $Date$
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
#include "dpBase.hh"
#include "comm.hh"


//
// Here is the interface to virtual sites.
// Pointers to procedures are set by 'new mailbox'/'init server'
// builtins;
//

//
extern VirtualSite* (*createVirtualSite)(DSite* s);

//
extern void (*zeroRefsToVirtual)(VirtualSite *vs);

//
// The 'mt', 'storeSite' and 'storeIndex' parameters are used whenever a 
// communication layer reports a problem with a message which has
// been previously accepted for delivery;
extern int
(*sendTo_VirtualSite)(VirtualSite *vs, MarshalerBuffer *mb,
                      MessageType mt, DSite *storeSite, int storeIndex);

//
extern int (*discardUnsentMessage_VirtualSite)(VirtualSite *vs, int msgNum);

//
extern int (*getQueueStatus_VirtualSite)(VirtualSite *vs, int &noMsgs);

//
extern SiteStatus (*siteStatus_VirtualSite)(VirtualSite* vs); 

//
extern MonitorReturn
(*monitorQueue_VirtualSite)(VirtualSite *vs, 
			    int size, int no_msgs, void *storePtr);
extern MonitorReturn (*demonitorQueue_VirtualSite)(VirtualSite* vs);

//
extern ProbeReturn
(*installProbe_VirtualSite)(VirtualSite *vs, ProbeType pt, int frequency);
extern ProbeReturn
(*deinstallProbe_VirtualSite)(VirtualSite *vs, ProbeType pt);
extern ProbeReturn
(*probeStatus_VirtualSite)(VirtualSite *vs,
			   ProbeType &pt, int &frequncey, void* &storePtr);

//
extern GiveUpReturn (*giveUp_VirtualSite)(VirtualSite* vs);

//
extern void (*discoveryPerm_VirtualSite)(VirtualSite *vs);

//
extern MarshalerBuffer* (*getVirtualMarshalerBuffer)(DSite* site);

//
extern void (*dumpVirtualMarshalerBuffer)(MarshalerBuffer* m);

//
extern void (*siteAlive_VirtualSite)(VirtualSite *vs);

///
/// An exit hook - kill slaves, reclaim shared memory, etc...
/// In fact, this declaration cannot be used since 'virtaulSite.hh' 
/// should not be included into 'am.cc';
extern void (*virtualSitesExit)();
