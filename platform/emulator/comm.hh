/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __COMM_HH
#define __COMM_HH

#ifdef INTERFACE
#pragma interface
#endif

//
#include "runtime.hh"
#include "codearea.hh"
#include "indexing.hh"
#include "perdio.hh"
#include "perdio_debug.hh"
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "urlc.hh"
#include "marshaler.hh"

//
class MsgBuffer;
class VirtualInfo;
class RemoteSite;
class VirtualSite;
class Site;
class SiteManager;

typedef unsigned short port_t;
typedef unsigned long ip_address;
typedef unsigned int FaultInfo;

/**********************************************************************/
/*   SECTION ::  global variables                                     */
/**********************************************************************/

extern Site * mySite;

/**********************************************************************/
/*   SECTION ::  used here defined in comm.cc                         */
/**********************************************************************/

void siteZeroActiveRef(Site *);

/**********************************************************************/
/*   SECTION :: return codes                                          */
/**********************************************************************/

enum MonitorReturn{
    MONITOR_OK,
    SIZE_THRESHOLD_REACHED,
    NO_MSGS_THRESHOLD_REACHED,
    MONITOR_ALREADY_EXISTS,
    NO_MONITOR_EXISTS,
    MONITOR_PERM
};

enum ProbeReturn{
  PROBE_INSTALLED,
  PROBE_ALREADY_INSTALLED,
  PROBE_DEINSTALLED,
  PROBE_NONEXISTENT,
  PROBE_OF_DIFFERENT_KIND,
  PROBE_PERM,
  PROBE_TEMP,
  PROBE_OK
};

enum GiveUpInput{
  ALL_GIVEUP,         // debug purpose only
  TEMP_GIVEUP
  };

enum GiveUpReturn{
    GIVES_UP,
    SITE_NOW_NORMAL,
    SITE_NOW_PERM
  };

enum ProbeType{
  PROBE_TYPE_ALL,
  PROBE_TYPE_PERM
};

enum SiteStatus{
  SITE_OK,
  SITE_PERM,                    // kost@ : redundant - for debugging only!
  SITE_TEMP
};

// sendTo return
#define ACCEPTED       0
#define PERM_NOT_SENT ~1
// TEMP_NOT_SENT  >0


/**********************************************************************/
/*   SECTION :: provided by communication layer                       */
/**********************************************************************/

Site * unmarshalPSite(MsgBuffer*);
Site * unmarshalSite(MsgBuffer*);

/**********************************************************************/
/*   SECTION :: provided by network communication layer               */
/**********************************************************************/

RemoteSite* createRemoteSite(Site*, int readCtr);

void zeroRefsToRemote(RemoteSite *);
int sendTo_RemoteSite(RemoteSite*, MsgBuffer*, MessageType, Site*, int);
void sendAck_RemoteSite(RemoteSite*);
int discardUnsentMessage_RemoteSite(RemoteSite*,int);
int getQueueStatus_RemoteSite(RemoteSite*);  // return size in bytes
SiteStatus siteStatus_RemoteSite(RemoteSite*);
void monitorQueue_RemoteSite(RemoteSite*,int size);
void demonitorQueue_RemoteSite(RemoteSite*);
void *getMonitorQueue_RemoteSite(RemoteSite*);
ProbeReturn installProbe_RemoteSite(RemoteSite*,ProbeType,int frequency);
ProbeReturn deinstallProbe_RemoteSite(RemoteSite*,ProbeType);
ProbeReturn probeStatus_RemoteSite(RemoteSite*,ProbeType &pt,int &frequncey,void* &storePtr);
GiveUpReturn giveUp_RemoteSite(RemoteSite*);
void discoveryPerm_RemoteSite(RemoteSite*);
void dumpRemoteMsgBuffer(MsgBuffer*);

//
//
void initNetwork();

/**********************************************************************/
/*   SECTION :: provided by virtual site communication layer          */
/**********************************************************************/

//
VirtualSite* createVirtualSite(Site *site);

void unmarshalUselessVirtualInfo(MsgBuffer*); // discard marshalled virtual info (hit or unsent msg)
VirtualInfo* unmarshalVirtualInfo(MsgBuffer*);
void marshalVirtualInfo(VirtualInfo*,MsgBuffer*);

void zeroRefsToVirtual(VirtualSite *);

int sendTo_VirtualSite(VirtualSite*,MsgBuffer*,MessageType,Site*, int);
int discardUnsentMessage_VirtualSite(VirtualSite*,int);
int getQueueStatus_VirtualSite(VirtualSite*,int &noMsgs);  // return size in bytes
SiteStatus siteStatus_VirtualSite(VirtualSite*);
MonitorReturn monitorQueue_VirtualSite(VirtualSite*,int size,int no_msgs,void*);
MonitorReturn demonitorQueue_VirtualSite(VirtualSite*);
ProbeReturn installProbe_VirtualSite(VirtualSite*,ProbeType,int frequency,void*);
ProbeReturn deinstallProbe_VirtualSite(VirtualSite*,ProbeType);
ProbeReturn probeStatus_VirtualSite(VirtualSite*,ProbeType &pt,int &frequncey,void* &storePtr);
GiveUpReturn giveUp_VirtualSite(VirtualSite*);
void discoveryPerm_VirtualSite(VirtualSite*);

void dumpVirtualInfo(VirtualInfo*);

/**********************************************************************/
/*   SECTION :: class BaseSite                                       */
/**********************************************************************/


class TimeStamp {
public:
  time_t start;
  int pid;
  TimeStamp() { DebugCode(start = (time_t) 0; pid = 0;); }
  TimeStamp(time_t s, int p): start(s), pid(p) {}
};

//
// kost@ : we don't really need a separte 'BaseSite';
class BaseSite{
friend class Site;
friend class SiteManager;
friend class SiteHashTable;
private:
protected:
  ip_address address;
  TimeStamp timestamp;
  port_t port;
  unsigned short flags;

  void init(ip_address ip,port_t p, TimeStamp *t){
    address=ip;
    timestamp=*t;
    port=p;
    flags=0;}

  void init(ip_address a,port_t p, TimeStamp *t,unsigned short ty){
    init(a,p,t);
    flags=ty;}

  unsigned short getType(){return flags;}

public:
  BaseSite(): timestamp(TimeStamp(0,0)) {}
  BaseSite(ip_address a,port_t p,TimeStamp &t):address(a),port(p),timestamp(t){}

  ip_address getAddress(){return address;} // ATTENTION
  port_t getPort(){return port;} // ATTENTION
  TimeStamp *getTimeStamp(){return &timestamp;} // ATTENTION
  // kost@ : What attention??? Where??? $%$#% !@#$ @#$!!!!!
  int hashPrimary();
  int hashSecondary();

  char* stringrep();

  void marshalBaseSite(MsgBuffer* buf){
    PD((MARSHAL,"base site =>"));
    marshalNumber(address,buf);
    PD((MARSHAL,"base site address %d",address));
    marshalShort(port,buf);
    PD((MARSHAL,"base site port %d",port));
    marshalNumber(timestamp.start,buf);
    marshalNumber(timestamp.pid,buf);
    PD((MARSHAL,"base site timestamp %d/%d",timestamp.start,timestamp.pid));
  }

  void unmarshalBaseSite(MsgBuffer* buf){
    PD((UNMARSHAL,"base site =>"));
    address=unmarshalNumber(buf);
    PD((UNMARSHAL,"base site address %d",address));
    port=unmarshalShort(buf);
    PD((UNMARSHAL,"base site port %d",port));
    timestamp.start=unmarshalNumber(buf);
    timestamp.pid=unmarshalNumber(buf);
    PD((UNMARSHAL,"base site timestamp %d/%d",timestamp.start,timestamp.pid));
  }


  int compareSitesNoTimestamp(BaseSite *s){
    if(address<s->address) return 0-1;
    if(s->address<address) return 1;
    if(port< s->port) return 0-1;
    if(s->port< port) return 1;
    return 0;}

  int checkTimeStamp(time_t t){
    if(t==timestamp.start) return 0;
    if(t<timestamp.start) return 1;
    return 0-1;}

  int checkTimeStamp(TimeStamp *t){
    int aux = checkTimeStamp(t->start);
    if (aux!=0) return aux;

    if (t->pid==timestamp.pid) return 0;
    if (t->pid<timestamp.pid) return 1;
    return 0-1;}

  int compareSites(BaseSite *s){
    if(address<s->address) return 0-1;
    if(s->address<address) return 1;
    if(port< s->port) return 0-1;
    if(s->port< port) return 1;
    return checkTimeStamp(&s->timestamp);
  }
};

/**********************************************************************/
/*   SECTION :: Site                                                  */
/**********************************************************************/

#define REMOTE_SITE           0x1
#define VIRTUAL_SITE          0x2
#define VIRTUAL_INFO          0x4
#define CONNECTED             0x8
#define PERM_SITE             0x10
#define SECONDARY_TABLE_SITE  0x20
#define MY_SITE               0x40
#define GC_MARK               0x80

//
// Flag combination possibilities (discounting gc);
//
/*
  //
  NONE                          // GName'd, or "passive";

  //
  REMOTE_SITE                   // remote site ...
  REMOTE_SITE | CONNECTED       // ... connected (as such);
  VIRTUAL_SITE | VIRTUAL_INFO   // virtual site ...
  VIRTUAL_SITE | VIRTUAL_INFO | CONNECTED // ... connected;
  REMOTE_SITE | VIRTUAL_INFO    // remote site from a foreign VS group;
  REMOTE_SITE | VIRTUAL_INFO | CONNECTED  // ... connected;

  //
  PERM_SITE                     // permanently down;
  PERM_SITE | SECONDARY_TABLE_SITE        // ... in secondary table;

  //
  // next 3 are transitory:
  // discovered by third party to be dead:
  REMOTE_SITE | PERM_SITE | CONNECTED     //
  // ... and even more: the network layer still believes it's connected to:
  REMOTE_SITE | VIRTUAL_INFO | PERM_SITE | CONNECTED //
  // ... with a virtual site (i (kost@) hardly believe this will be used):
  VIRTUAL_SITE | PERM_SITE | CONNECTED    //

  //
  MY_SITE                       //
  MY_SITE | VIRTUAL_INFO        //
*/

/**********************************************************************/
/*   SECTION :: class Site                                     */
/**********************************************************************/

enum FaultCode{
    COMM_FAULT_PERM_NOT_SENT,
    COMM_FAULT_PERM_MAYBE_SENT,
    COMM_FAULT_TEMP_NOT_SENT,
    COMM_FAULT_TEMP_MAYBE_SENT
  };

class Site: public BaseSite{
friend class SiteHashTable;
friend class RemoteSite;
friend class SiteManager;
private:
  VirtualInfo *info;

  union {
    RemoteSite* rsite;
    VirtualSite* vsite;
    int readCtr;
  } uRVC;   // kost@ : who has thought out this abbreviation??!

  //
private:
  void setType(unsigned int i){flags=i;}

  void disconnectInPerm();

  void disconnect(){
    flags &= (~CONNECTED);
    return;}

  Bool connect(){
    unsigned int t=getType();
    PD((SITE,"connect, the type of this site: %d",t));
    Assert(!(t & MY_SITE));
    if(t & CONNECTED) return OK;
    if(t & (PERM_SITE)) return NO;
    if(t & REMOTE_SITE){
      RemoteSite *rs=createRemoteSite(this,uRVC.readCtr);
      Assert(rs!=NULL);
      uRVC.rsite=rs;
      PD((SITE,"connect; not connected yet, connecting to remote %d",rs));}
    else{
      Assert(t & VIRTUAL_SITE);
      Assert(t & VIRTUAL_INFO);
      VirtualSite *vs = createVirtualSite(this);
      Assert(vs!=NULL);
      uRVC.vsite=vs;
      PD((SITE,"connect; not connected yet, connecting to virtual %d",vs));}
    flags |= CONNECTED;
    return OK;}

  void zeroActive(){
    if(getType() & CONNECTED){
      if(getType() & REMOTE_SITE){
        zeroRefsToRemote(getRemoteSite());
        return;}
      Assert(getType() & VIRTUAL_SITE);
      zeroRefsToVirtual(getVirtualSite());
      return;}}

  void makePermConnected(){
    flags |= PERM_SITE;
    flags &= (~CONNECTED);
    return;}

  void makePerm(){
    flags |= PERM_SITE;}


protected:

  unsigned short getType(){ return flags;}

public:

  Site(){}
  Site(ip_address a,port_t p,TimeStamp  &t):BaseSite(a,p,t){
    uRVC.readCtr=0;
    setType(MY_SITE);}

  void makeGCMarkSite(){flags |= GC_MARK;}
  void removeGCMarkSite(){flags &= ~(GC_MARK);}
  Bool isGCMarkedSite(){return flags & GC_MARK;}

  Bool ActiveSite(){
    if(getType() & (REMOTE_SITE|VIRTUAL_SITE)) return OK;
    return NO;}

  Bool remoteComm(){
    if(getType() & REMOTE_SITE) return OK;
    if(getType() & PERM_SITE) return OK;      // ATTENTION
    return NO;}
  Bool virtualComm(){
    if(getType() & VIRTUAL_SITE) return OK;
    // used for dropping buffers of dead connections?
    if(getType() & PERM_SITE) return OK;
    return NO;}

  void passiveToPerm(){
    Assert(!(ActiveSite()));
    flags |= PERM_SITE;
    return;}

  void putInSecondary(){
    Assert(!(MY_SITE & getType()));
    setType(getType()|SECONDARY_TABLE_SITE);}

  Bool isInSecondary(){
    if(getType() & SECONDARY_TABLE_SITE) return OK;
    return NO;}

  Bool isPerm(){return getType() & PERM_SITE;}

  Bool canBeFreed(){
    Assert(!isGCMarkedSite());
    if(flags & MY_SITE) {return NO;}
    unsigned short t=getType();
    if(ActiveSite() &&
       ((t & CONNECTED) || uRVC.readCtr!=0)){
      zeroActive();
      return NO;}
    return OK;}

  //
  void initMySite() {
    info=NULL;
    uRVC.readCtr=0;
    setType(MY_SITE);}

  //
  // Extending the 'mySite' to be a virtual one (to be used whenever
  // a master creates its first child, or a child initializes itself
  // upon 'M_INIT_VS');
  void makeMySiteVirtual(VirtualInfo *v) {
    info = v;
    Assert(uRVC.rsite == (RemoteSite *) 0);
    Assert(uRVC.vsite == (VirtualSite *) 0);
    Assert(uRVC.readCtr == 0);
    setType(getType() | VIRTUAL_INFO);
  }

#ifdef VIRTUALSITES
  //
  // Initializes 'VirtualInfo' object's address, port and timestamp
  // fields. This is to be used whenever a master virtual site
  // is created.
  // This is the method of this class (BaseSite) since this
  // triple should not be exposed outside these classes. It is used
  // for the 'VirtualInfo' constructor;
  void initVirtualInfoArg(VirtualInfo *vi);
#endif

  //
  // kost@ : init's are for new(ly inserted) site objects;
  void initRemote(){
    info=NULL;
    uRVC.readCtr=0;
    Assert(!(getType() & MY_SITE));
    setType(REMOTE_SITE);}

  void initPerm(){
    info=NULL;
    uRVC.readCtr=0;
    setType(PERM_SITE);}

  void initPassive(){
    info=NULL;
    uRVC.readCtr=0;
    Assert(!(getType() & MY_SITE));
    setType(0);}

  void initVirtual(VirtualInfo *vi) {
    Assert(!((getType()) & VIRTUAL_INFO));
    info = vi;
    Assert(uRVC.readCtr == 0);
    setType(VIRTUAL_SITE | VIRTUAL_INFO);
  }

  void initRemoteVirtual(VirtualInfo *vi) {
    Assert(!((getType()) & VIRTUAL_INFO));
    info = vi;
    uRVC.readCtr = 0;
    setType(REMOTE_SITE | VIRTUAL_INFO);
  }

  //
  // kost@ : 'makeActive*()' are for former passive (GName'd) site
  // objects, and - in the case of virtual sites - when we declare the
  // master site to be virtual one wrt us (see M_INIT_VS);
  void makeActiveRemote(){
    uRVC.readCtr=0;
    Assert(!(getType() & MY_SITE));
    setType(REMOTE_SITE);}

  void makeActiveVirtual() {
    // (in fact, it means that it was "(active) remote virtual";)
    Assert((getType()) & VIRTUAL_INFO);
    Assert(uRVC.readCtr == 0);
    setType(VIRTUAL_SITE | VIRTUAL_INFO);
  }

  void makeActiveVirtual(VirtualInfo *vi) {
    Assert(!((getType()) & VIRTUAL_INFO));
    info = vi;
    Assert(uRVC.readCtr == 0);
    setType(VIRTUAL_SITE | VIRTUAL_INFO);
  }

  void makeActiveRemoteVirtual(VirtualInfo *vi) {
    Assert(!((getType()) & VIRTUAL_INFO));
    info = vi;
    Assert(uRVC.readCtr == 0);
    setType(REMOTE_SITE | VIRTUAL_INFO);
  }

  //
  Bool hasVirtualInfo() { return (getType() & VIRTUAL_INFO); }
  VirtualInfo* getVirtualInfo() {
    Assert(getType() & VIRTUAL_INFO);
    return (info);
  }

// provided to network-comm

  RemoteSite* getRemoteSite() {
    if(!connect()) {PD((SITE,"getRemoteSite not connected"));return NULL;}
    Assert(getType() & CONNECTED);
    Assert(getType() & REMOTE_SITE);
    RemoteSite *rs = uRVC.rsite;
    PD((SITE,"getRemoteSite returning the remote %d",rs));
    Assert(rs!=NULL);
    return  rs;}

  void dumpRemoteSite(int readCtr){
    Assert(getType() & CONNECTED);
    Assert(getType() & REMOTE_SITE);
    disconnect();
    uRVC.readCtr=readCtr;}

// provided to virtual-comm

  VirtualSite* getVirtualSite(){
    if(!connect()) {return NULL;}
    Assert(getType() & CONNECTED);
    Assert(getType() & VIRTUAL_SITE);
    Assert(!(getType() & REMOTE_SITE));
    return uRVC.vsite;}

  void dumpVirtualSite(void) {
    Assert(getType() & CONNECTED);
    Assert(getType() & VIRTUAL_SITE);
    Assert(!(getType() & REMOTE_SITE));
    disconnect();
    uRVC.readCtr = 0;
  }

#ifdef VIRTUALSITES
  //
  // Compare "virtual info"s of two sites.
  // Note that this is a metod of the 'Site' class since
  // (a) it contains that virtual info, and (b) address/port/timestamp
  // fields are private members of 'VirtualInfo' objects;
  Bool isInMyVSGroup(VirtualInfo *vi);
#endif

  // for use by the network-comm and virtual-comm
  // ASSUMPTION: network-comm has reclaimed RemoteSite
  //             virtual-comm has reclaimed VirtualSite

  // for use by the protocol-layer

  int sendTo(MsgBuffer *buf,MessageType mt,Site* storeSite,int storeIndex){
    PD((MSG_SENT,"to_site:%s type:%s",this->stringrep(),mess_names[(int) mt]));
    if(connect()){
      if(getType() & REMOTE_SITE){
        return sendTo_RemoteSite(getRemoteSite(),buf,mt,storeSite,storeIndex);}
      Assert(getType() & VIRTUAL_SITE);
      return sendTo_VirtualSite(getVirtualSite(),buf,mt,storeSite,storeIndex);}
    PD((ERROR_DET,"MsgNot sent, discovered at Site level %d",
        PERM_NOT_SENT));
    return PERM_NOT_SENT;}

  int discardUnsentMessage(int msgNum){
    Assert(getType() & CONNECTED);
    if(getType() & VIRTUAL_SITE) {
      return discardUnsentMessage_VirtualSite(getVirtualSite(),msgNum);
    }
    Assert(getType() & REMOTE_SITE);
    return discardUnsentMessage_RemoteSite(getRemoteSite(),msgNum);}

  int getQueueStatus(int &noMsgs){
    unsigned short t=getType();
    if(!(t & CONNECTED)){
      noMsgs=0;
      return 0;}
    if(t & REMOTE_SITE){
      noMsgs = 0;
      return getQueueStatus_RemoteSite(getRemoteSite());}
    Assert(t & VIRTUAL_SITE);
    return getQueueStatus_VirtualSite(getVirtualSite(),noMsgs);}

  SiteStatus siteStatus(){
    unsigned short t=getType();
    if(!(t & CONNECTED)) {
      if(t & PERM_SITE) {return SITE_PERM;}
      return SITE_OK;}
    if(t & REMOTE_SITE){
      return siteStatus_RemoteSite(getRemoteSite());}
    Assert(t & VIRTUAL_SITE);
    return siteStatus_VirtualSite(getVirtualSite());}

  MonitorReturn demonitorQueue(){
    unsigned short t=getType();
    if(!(t & CONNECTED)) {
      return NO_MONITOR_EXISTS;}
    if(t & REMOTE_SITE){
      demonitorQueue_RemoteSite(getRemoteSite());
      return MONITOR_OK;}
    Assert(t & VIRTUAL_SITE);
    return demonitorQueue_VirtualSite(getVirtualSite());}

  MonitorReturn monitorQueue(int size,int noMsgs,void *storePtr){
    if(connect()){
      if(getType() & REMOTE_SITE){
        monitorQueue_RemoteSite(getRemoteSite(),size); return MONITOR_OK;}
      Assert(getType() & VIRTUAL_SITE);
      return monitorQueue_VirtualSite(getVirtualSite(),size,noMsgs,storePtr);}
    return MONITOR_PERM;}

  ProbeReturn installProbe(ProbeType pt, int frequency){
    if(connect()){
      if(getType() & REMOTE_SITE){
        return installProbe_RemoteSite(getRemoteSite(),pt,frequency);}
      Assert(getType() & VIRTUAL_SITE);
      return installProbe_VirtualSite(getVirtualSite(),PROBE_TYPE_ALL,frequency,NULL);}
    return PROBE_PERM;}

  ProbeReturn deinstallProbe(ProbeType pt){
    unsigned short t=getType();
    if(t & CONNECTED){
      if(t & REMOTE_SITE){
        return deinstallProbe_RemoteSite(getRemoteSite(),pt);}
      Assert(t & VIRTUAL_SITE);
      return deinstallProbe_VirtualSite(getVirtualSite(),PROBE_TYPE_ALL);}
    return PROBE_NONEXISTENT;}

  ProbeReturn probeStatus(ProbeType &pt,int &frequency,void* &storePtr){
    unsigned short t=getType();
    if(t & CONNECTED){
      if(t & REMOTE_SITE){
        return probeStatus_RemoteSite(getRemoteSite(),pt,frequency,storePtr);}
      Assert(t & VIRTUAL_SITE);
      return probeStatus_VirtualSite(getVirtualSite(),pt,frequency,storePtr);}
    return PROBE_NONEXISTENT;}

  GiveUpReturn giveUp(GiveUpInput flag){ // PERM case 1 user initiated
    unsigned short t=getType();
    if(t & PERM_SITE) {return SITE_NOW_PERM;}
    if(flag==ALL_GIVEUP){
      if(t & CONNECTED){
        makePermConnected();
        if(t & REMOTE_SITE){
          return giveUp_RemoteSite(getRemoteSite());}
        Assert(t & VIRTUAL_SITE);
        return giveUp_VirtualSite(getVirtualSite());}
      makePerm();}
    if(siteStatus()==SITE_OK){ return SITE_NOW_NORMAL;}
    if(t & CONNECTED){
      makePermConnected();
      if(t & REMOTE_SITE){
        return giveUp_RemoteSite(getRemoteSite());}
      Assert(t & VIRTUAL_SITE);
      return giveUp_VirtualSite(getVirtualSite());}
    makePerm();
    return GIVES_UP;}

  void marshalSite(MsgBuffer *);
  void marshalPSite(MsgBuffer *buf);

// PERM case 2) discovered in unmarshaling or 3) in network
  void discoveryPerm(){
    unsigned short t=getType();
    if(t==0) {
      flags |= PERM_SITE;
      return;}
    Assert(!(t & SECONDARY_TABLE_SITE));
    if(t & PERM_SITE) return;
    if(t & CONNECTED){
      if(t & REMOTE_SITE){
        if(t & VIRTUAL_INFO) {
          dumpVirtualInfo(info);
          info=NULL;}
        discoveryPerm_RemoteSite(getRemoteSite());
        makePermConnected();
        return;}
      Assert(t & VIRTUAL_SITE);
      dumpVirtualInfo(info);
      info=NULL;
      discoveryPerm_VirtualSite(getVirtualSite());
      makePermConnected();
      return;}
    makePerm();
    if(t & VIRTUAL_INFO){
      dumpVirtualInfo(info);}}


// provided for network and virtual site comm-layers

  void communicationProblem(MessageType mt,Site*
                            storeSite,int storeIndex
                            ,FaultCode fc,FaultInfo fi);

  void probeFault(ProbeReturn pr);

  void sitePermProblem(){
    discoveryPerm();}

  void monitorInvoke(MonitorReturn mt,int size,int noMsgs){
    Assert(0);
    error("not implemented");
    return;}

  void msgReceived(MsgBuffer *);

  Bool isMySite(){
    return flags & MY_SITE;}
};

//
// kost@ 26.3.98 : 'msgReceived()' is NOT a method of a site object.
// That's quite natural: we don't know who send us a message (of
// course, communication layer for remote site do know, but that's
// other story).
void msgReceived(MsgBuffer *);

//
// kost@ : that's a part of the boot-up procedure ('perdioInit()');
Site* makeMySite(ip_address a, port_t p, TimeStamp &t);

/**********************************************************************
 *   SECTION :: new gate support
 **********************************************************************/

extern OZ_Term  GateStream;
Site *findSite(ip_address a,int port,TimeStamp &stamp);

#endif // __COMM_HH
