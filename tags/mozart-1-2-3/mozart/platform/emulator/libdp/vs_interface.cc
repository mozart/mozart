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
#include "msgType.hh"
#include "comm.hh"

//
// Zeroth: define dummy "VS not configured" interface method
// implementations:

//
VirtualSite* createVirtualSiteStub(DSite *)
{
  OZ_error("'createVirtualSite' called without 'VIRTUALSITES'?");
  return ((VirtualSite *) 0);
}
//
void zeroRefsToVirtualStub(VirtualSite *)
{
  OZ_error("'zeroRefsToVirtual' called without 'VIRTUALSITES'?");
}
//
int sendTo_VirtualSiteStub(VirtualSite*, MarshalerBuffer*, MessageType, DSite*, int)
{
  int x= *((int *)(0x1));
  OZ_error("'sendTo_VirtualSite' called without 'VIRTUALSITES'?");
  return (-1);
}
//
int discardUnsentMessage_VirtualSiteStub(VirtualSite*, int)
{
  OZ_error("'discardUnsentMessage_VirtualSite' called without 'VIRTUALSITES'?");
  return (-1);
}
//
int getQueueStatus_VirtualSiteStub(VirtualSite*, int &)
{
  OZ_error("'getQueueStatus_VirtualSite' called without 'VIRTUALSITES'?");
  return (-1);
}
//
SiteStatus siteStatus_VirtualSiteStub(VirtualSite *)
{
  OZ_error("'siteStatus_VirtualSite' called without 'VIRTUALSITES'?");
  return ((SiteStatus) -1);
}
//
MonitorReturn monitorQueue_VirtualSiteStub(VirtualSite*, int, int, void*)
{
  OZ_error("'monitorQueue_VirtualSite' called without 'VIRTUALSITES'?");
  return ((MonitorReturn) -1);
}
//
MonitorReturn demonitorQueue_VirtualSiteStub(VirtualSite *)
{
  OZ_error("'demonitorQueue_VirtualSite' called without 'VIRTUALSITES'?");
  return ((MonitorReturn) -1);
}
//
ProbeReturn installProbe_VirtualSiteStub(VirtualSite*, ProbeType, int)
{
  OZ_error("'installProbe_VirtualSite' called without 'VIRTUALSITES'?");
  return ((ProbeReturn) -1);
}
//
ProbeReturn deinstallProbe_VirtualSiteStub(VirtualSite*, ProbeType pt)
{
  OZ_error("'installProbe_VirtualSite' called without 'VIRTUALSITES'?");
  return ((ProbeReturn) -1);
}
//
ProbeReturn probeStatus_VirtualSiteStub(VirtualSite*, ProbeType&, int&, void*&)
{
  OZ_error("'probeStatus_VirtualSite' called without 'VIRTUALSITES'?");
  return ((ProbeReturn) -1);
}
//
GiveUpReturn giveUp_VirtualSiteStub(VirtualSite *)
{
  OZ_error("'giveUp_VirtualSite' called without 'VIRTUALSITES'?");
  return ((GiveUpReturn) -1);
}
//
void discoveryPerm_VirtualSiteStub(VirtualSite *)
{
  OZ_error("'discoveryPerm_VirtualSite' called without 'VIRTUALSITES'?");
}
//
MarshalerBuffer* getVirtualMarshalerBufferStub(DSite *)
{
  OZ_error("'getVirtualMarshalerBuffer' called without 'VIRTUALSITES'?");
  return ((MarshalerBuffer *) 0);
}
//
void dumpVirtualMarshalerBufferStub(MarshalerBuffer *)
{
  OZ_error("'dumpVirtualMarshalerBuffer' called without 'VIRTUALSITES'?");
}
//
void siteAlive_VirtualSiteStub(VirtualSite *vs)
{
  OZ_error("'siteAlive_VirtualSite' called without 'VIRTUALSITES'?");
}
// This one may be called;
void virtualSitesExitStub() {}

//
// First: define interface methods with defaults to "dunno" stubs;
//
VirtualSite* (*createVirtualSite)(DSite* s)
  = createVirtualSiteStub;
void (*zeroRefsToVirtual)(VirtualSite *vs)
  = zeroRefsToVirtualStub;
int (*sendTo_VirtualSite)(VirtualSite *vs, MarshalerBuffer *mb,
                          MessageType mt, DSite *storeSite, int storeIndex)
  = sendTo_VirtualSiteStub;
int (*discardUnsentMessage_VirtualSite)(VirtualSite *vs, int msgNum)
  = discardUnsentMessage_VirtualSiteStub;
int (*getQueueStatus_VirtualSite)(VirtualSite *vs, int &noMsgs)
  = getQueueStatus_VirtualSiteStub;
SiteStatus (*siteStatus_VirtualSite)(VirtualSite* vs)
  = siteStatus_VirtualSiteStub;
MonitorReturn
(*monitorQueue_VirtualSite)(VirtualSite *vs, 
			    int size, int no_msgs, void *storePtr)
  = monitorQueue_VirtualSiteStub;
MonitorReturn (*demonitorQueue_VirtualSite)(VirtualSite* vs)
  = demonitorQueue_VirtualSiteStub;
ProbeReturn
(*installProbe_VirtualSite)(VirtualSite *vs, ProbeType pt, int frequency)
  = installProbe_VirtualSiteStub;
ProbeReturn (*deinstallProbe_VirtualSite)(VirtualSite *vs, ProbeType pt)
  = deinstallProbe_VirtualSiteStub;
ProbeReturn
(*probeStatus_VirtualSite)(VirtualSite *vs,
			   ProbeType &pt, int &frequncey, void* &storePtr)
  = probeStatus_VirtualSiteStub;
GiveUpReturn (*giveUp_VirtualSite)(VirtualSite* vs)
  = giveUp_VirtualSiteStub;
void (*discoveryPerm_VirtualSite)(VirtualSite *vs)
  = discoveryPerm_VirtualSiteStub;
MarshalerBuffer* (*getVirtualMarshalerBuffer)(DSite* site)
  = getVirtualMarshalerBufferStub;
void (*dumpVirtualMarshalerBuffer)(MarshalerBuffer* m)
  = dumpVirtualMarshalerBufferStub;
void (*siteAlive_VirtualSite)(VirtualSite *vs)
  = siteAlive_VirtualSiteStub;
void (*virtualSitesExit)()
  = virtualSitesExitStub;
