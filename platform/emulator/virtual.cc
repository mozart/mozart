#include "perdio.hh"
#include "perdio_debug.hh"  
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "urlc.hh"
#include "marshaler.hh"
#include "comm.hh"
#include "msgbuffer.hh"

VirtualSite* createVirtualSite(Site* s,int i){
  Assert(0);
  return NULL;}

void unmarshalUselessVirtualInfo(MsgBuffer*){
  Assert(0);
  return;}

VirtualInfo* unmarshalVirtualInfo(MsgBuffer*){
  Assert(0);
  return NULL;}

void marshalVirtualInfo(VirtualInfo*,MsgBuffer*){
  Assert(0);
  return;}

void zeroRefsToVirtual(VirtualSite *vs){
  Assert(0);
  return;}

void nonZeroRefsToVirtual(VirtualSite *vs){
  Assert(0);
  return;}


Bool inMyGroup(Site* s,VirtualInfo* vi){
  Assert(0);
  return NO;}

int sendTo_VirtualSite(VirtualSite*,MsgBuffer*,MessageType,Site*, int){
  Assert(0);
  return 0;}

int discardUnsentMessage_VirtualSite(VirtualSite* vs,int i){
  Assert(0);
  return 0;}

int getQueueStatus_VirtualSite(VirtualSite* vs,int &noMsgs){
  Assert(0);
  return 0;}

SiteStatus siteStatus_VirtualSite(VirtualSite* vs){
  Assert(0);
  return SITE_OK;}

MonitorReturn monitorQueue_VirtualSite(VirtualSite* vs,int size,int no_msgs,void* storePtr){
  Assert(0);
  return NO_MONITOR_EXISTS;}

MonitorReturn demonitorQueue_VirtualSite(VirtualSite* vs){
  Assert(0);
  return NO_MONITOR_EXISTS;}

ProbeReturn installProbe_VirtualSite(VirtualSite*,ProbeType pt,int frequency,void* storePtr){
  Assert(0);
  return PROBE_NONEXISTENT;}

ProbeReturn deinstallProbe_VirtualSite(VirtualSite*,ProbeType pt){
  Assert(0);
  return PROBE_NONEXISTENT;}

ProbeReturn probeStatus_VirtualSite(VirtualSite* vs,ProbeType &pt,int &frequncey,void* &storePtr){
  Assert(0);
  return PROBE_NONEXISTENT;}

GiveUpReturn giveUp_VirtualSite(VirtualSite* vs){
  Assert(0);
  return SITE_NOW_NORMAL;}

void discoveryPerm_VirtualSite(VirtualSite* vs){
  Assert(0);
  return;}

void dumpVirtualInfo(VirtualInfo* vi){
  Assert(0);
  return;}

MsgBuffer* getVirtualMsgBuffer(Site* s){
  Assert(0);
  return NULL;}
