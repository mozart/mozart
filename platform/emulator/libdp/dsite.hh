/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Konstantin Popov <kost@sics.se>
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

#ifndef __DSITE_HH
#define __DSITE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dpBase.hh"
#include "marshalerBase.hh"
#include "site.hh"
#include "msgType.hh"
#include "comm.hh"
#include "dpDebug.hh"
#include "fail.hh"
#include "network.hh"
#include "vs_interface.hh"
#include "os.hh"
#include "comObj.hh"

/**********************************************************************/
/*   SECTION :: Site                                                  */
/**********************************************************************/

//
// 'REMOTE'/'VIRTUAL' just says HOW to communicate with...
#define REMOTE_SITE           0x1
#define VIRTUAL_SITE          0x2
#define VIRTUAL_INFO          0x4
#define CONNECTED             0x8
#define PERM_SITE             0x10
#define SECONDARY_TABLE_SITE  0x20
#define MY_SITE		      0x40
#define GC_MARK		      0x80

//
// Flag combination possibilities (discounting gc);
//
/*
  //
  NONE				// GName'd, or "passive";

  //
  REMOTE_SITE			// remote site ...
  REMOTE_SITE | CONNECTED	// ... connected (as such);
  VIRTUAL_SITE | VIRTUAL_INFO	// virtual site ...
  VIRTUAL_SITE | VIRTUAL_INFO | CONNECTED // ... connected;
  REMOTE_SITE | VIRTUAL_INFO	// remote site from a foreign VS group;
  REMOTE_SITE | VIRTUAL_INFO | CONNECTED  // ... connected;

  //
  PERM_SITE			// permanently down;
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
  MY_SITE			// 
  MY_SITE | VIRTUAL_INFO	// 
*/

//
void marshalVirtualInfo(VirtualInfo *vi, MarshalerBuffer *mb);
VirtualInfo* unmarshalVirtualInfo(MarshalerBuffer *mb); 
void unmarshalUselessVirtualInfo(MarshalerBuffer *);
void dumpVirtualInfo(VirtualInfo* vi);

//
// Managing free list: cutoff on 
#define DSITE_FREE_LIST_CUTOFF  16

//
// There is one perdio-wide known "distribition" site object.  
// This is like 'mySite', but provides communication peers for 
// accessing (addressing) 'mySite';
extern DSite *myDSite;


class DSite: public BaseSite {
friend class DSiteHashTable;
friend class ComObj;
private:
  unsigned short flags;
  VirtualInfo *info;
  union {
    ComObj* comObj;
    VirtualSite* vsite;
    int readCtr;
  } u;

  //
protected:

  unsigned short getType(){ return flags;}

private:
  //
  int hashWOTimestamp();

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
      ComObj *rs=createComObj(this,u.readCtr);
      // AN! this instantiation makes latter changes of perdioCh... unused
//        rs->installProbe(0,PROBE_WAIT_TIME,ozconf.perdioCheckAliveInterval);
      Assert(rs!=NULL);
      u.comObj=rs;
      PD((SITE,"connect; not connected yet, connecting to remote %d",rs));}
    else{
      Assert(t & VIRTUAL_SITE);
      Assert(t & VIRTUAL_INFO);
      VirtualSite *vs = (*createVirtualSite)(this);
      Assert(vs!=NULL);
      u.vsite=vs;
      PD((SITE,"connect; not connected yet, connecting to virtual %d",vs));}
    flags |= CONNECTED;    
    return OK;}    

public:
  // If this site allready has a comObj that one is returned,
  // else the incoming is used and the state is set to connected.
  // AN! - neither PERM nor VIRTUAL_SITE considered
  ComObj *setComObj(ComObj *comObj) {
    unsigned int t=getType();
    if(t & CONNECTED) 
      return u.comObj;
    else {
      setType(CONNECTED|REMOTE_SITE);
//        comObj->installProbe(0,PROBE_WAIT_TIME,PERDIO_CHECK_ALIVE_INTERVAL);
      u.comObj=comObj;
      return NULL;
    }
  }
    
  ComObj* getComObj() {   
    if(!connect()) {PD((SITE,"getComObj not connected"));return NULL;}
    Assert(getType() & CONNECTED);
    Assert(getType() & REMOTE_SITE);
    ComObj *rs = u.comObj;
    PD((SITE,"getComObj returning the remote %d",rs));
    Assert(rs!=NULL);
    return  rs;}
  
  // provided to virtual-comm
  VirtualSite* getVirtualSite(){ 
    if(!connect()) {return NULL;}
    Assert(getType() & CONNECTED);
    Assert(getType() & VIRTUAL_SITE);
    Assert(!(getType() & REMOTE_SITE));
    return u.vsite;}


private:
// hopefully this is used to tell comm layer that the site is garbage for closing connection early 
  // AN! never called!
  void zeroActive(){ 
    if(getType() & CONNECTED){
      if(getType() & REMOTE_SITE){
//  	zeroRefsToRemote(getComObj());
	return;}
      Assert(getType() & VIRTUAL_SITE);
      (*zeroRefsToVirtual)(getVirtualSite());
      return;}}

  void makePermConnected(){
    flags |= PERM_SITE;
    flags &= (~CONNECTED);
    u.readCtr=0;
    return;}              
  
  void makePerm(){
    flags |= PERM_SITE;}


public:
  //
  void* operator new(size_t size) {
    Assert(sizeof(DSite)<=sizeof(Construct_7));
    return ((DSite *) genFreeListManager->getOne_7());}

  void freeSite() {
    genFreeListManager->putOne_7((FreeListEntry*) this);}   

  //
  DSite() {}			// 'unmarshalDSite()';
  DSite(ip_address a, port_t p, TimeStamp *t)
    : BaseSite(a, p, t), info((VirtualInfo *) 0) {
    u.readCtr = 0;
    DebugCode(flags = (unsigned short) -1);
  }
  DSite(ip_address a, port_t p, TimeStamp* t, unsigned short ty)
    : BaseSite(a, p, t), info((VirtualInfo *) 0) {
    u.readCtr = 0;	
    flags=ty;}

  DSite(ip_address a, port_t p, TimeStamp &t)
    : BaseSite(a, p, t), info((VirtualInfo *) 0) {
    u.readCtr = 0;
    DebugCode(flags = (unsigned short) -1);
  }
  DSite(ip_address a, port_t p, TimeStamp& t, unsigned short ty)
    : BaseSite(a, p, t), info((VirtualInfo *) 0) {
    u.readCtr = 0;	
    flags=ty;}

  int compareSitesNoTimestamp(DSite *s){
    if(address<s->address) return 0-1;
    if(s->address<address) return 1;    
    if(port< s->port) return 0-1;
    if(s->port< port) return 1;
    return 0;}

  int hashPrimary() {return hashWOTimestamp();}
  int hashSecondary() {return hash();}
  void setMyDSite() { setType(MY_SITE); }

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
    setType(getType() | SECONDARY_TABLE_SITE);}

  Bool isInSecondary(){
    if(getType() & SECONDARY_TABLE_SITE) return OK;
    return NO;}

  //
  Bool isConnected() { return ((getType() & CONNECTED)); }

  Bool isPerm(){return (getType() & PERM_SITE);}

  Bool canBeFreed(){
    Assert(!isGCMarkedSite());
    if(flags & MY_SITE) {return NO;}
    unsigned short t=getType();
    if(ActiveSite() && !isPerm() &&
       ((t & CONNECTED) || u.readCtr!=0)){ // Check over tests AN!
      Assert(u.comObj!=NULL);
      if(u.comObj->canBeFreed()) {
	comController->deleteComObj(u.comObj);
	disconnect();
	return OK;
      }
      else {
	return NO;
      }
    }
    return OK;
  }

  //
  void initMyDSite() {
    info=NULL;
    u.readCtr=0;
    setType(MY_SITE);}

  // 
  // Extending the 'myDSite' to be a virtual one (to be used whenever 
  // a master creates its first child, or a child initializes itself
  // upon 'M_INIT_VS');
  void makeMySiteVirtual(VirtualInfo *v) {
    info = v;
    Assert(u.comObj == (ComObj *) 0);
    Assert(u.vsite == (VirtualSite *) 0);
    Assert(u.readCtr == 0);
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
    u.readCtr=0;
    setType(REMOTE_SITE);}

  void initPerm(){
    info=NULL;
    u.readCtr=0;
    setType(PERM_SITE);}

  void initPassive(){
    info=NULL;
    u.readCtr=0;
    setType(0);}

  void initVirtual(VirtualInfo *vi) {
    info = vi;
    Assert(u.readCtr == 0);
    setType(VIRTUAL_SITE | VIRTUAL_INFO);
  }

  void initRemoteVirtual(VirtualInfo *vi) {
    info = vi;
    u.readCtr = 0;
    setType(REMOTE_SITE | VIRTUAL_INFO);
  }

  //
  // kost@ : 'makeActive*()' are for former passive (GName'd) site
  // objects, and - in the case of virtual sites - when we declare the
  // master site to be virtual one wrt us (see M_INIT_VS);
  void makeActiveRemote(){
    u.readCtr=0;
    Assert(!(getType() & MY_SITE)); 
    setType(REMOTE_SITE);}

  void makeActiveVirtual() {
    // (in fact, it means that it was "(active) remote virtual";)
    Assert(getType() & VIRTUAL_INFO);
    Assert(u.readCtr == 0);
    setType(VIRTUAL_SITE | VIRTUAL_INFO);
  }

  void makeActiveVirtual(VirtualInfo *vi) {
    info = vi;
    Assert(u.readCtr == 0);
    setType(VIRTUAL_SITE | VIRTUAL_INFO);
  }

  void makeActiveRemoteVirtual(VirtualInfo *vi) {
    info = vi;
    Assert(u.readCtr == 0);
    setType(REMOTE_SITE | VIRTUAL_INFO);
  }

  void addVirtualInfoToActive(VirtualInfo *vi) {
    // Note that it cannot happen that a site is actual virtual one
    // but does not have a virtual info;
    Assert(getType() & REMOTE_SITE);
    Assert(!(getType() & VIRTUAL_INFO));
    info = vi;
    setType(getType() | VIRTUAL_INFO);
  }

  //
  Bool hasVirtualInfo() { return (getType() & VIRTUAL_INFO); }
  VirtualInfo* getVirtualInfo() {
    Assert(getType() & VIRTUAL_INFO);
    return (info);
  }

  // provided to network-comm
  void dumpRemoteSite(int readCtr){
    Assert(getType() & CONNECTED);
    Assert(getType() & REMOTE_SITE);
    disconnect();
    u.readCtr=readCtr;}

  // Actually not used now...
  void dumpVirtualSite(void) {
    Assert(getType() & CONNECTED);
    Assert(getType() & VIRTUAL_SITE);
    Assert(!(getType() & REMOTE_SITE));
    disconnect();    
    u.vsite = (VirtualSite *) 0;
  }

  //
  // Compare "virtual info"s of two sites.
  // Note that this is a metod of the 'Site' class since 
  // (a) it contains that virtual info, and (b) address/port/timestamp 
  // fields are private members of 'VirtualInfo' objects;
  Bool isInMyVSGroup(VirtualInfo *vi);

  // for use by the network-comm and virtual-comm
  // ASSUMPTION: network-comm has reclaimed RemoteSite 
  //             virtual-comm has reclaimed VirtualSite
  
  // for use by the protocol-layer

  // AN!
//    int sendTo(MsgContainer *msgC,MessageType mt,DSite* storeSite,int storeIndex)
//      {printf("dsite.hh: OLD sendTo used\n");
//      return PERM_NOT_SENT;}

  int send(MsgContainer *msgC,int priority) {
    if(connect()){
      if(getType() & REMOTE_SITE){
	Assert(priority<5 && priority>1);
	getComObj()->send(msgC,priority);
      }
      else
	Assert(0);
      return ACCEPTED;
    }
    else {
      return PERM_NOT_SENT;
    }
  }

  // AN! this is no longer defined
  int discardUnsentMessage(int msgNum){
    Assert(getType() & CONNECTED);
    if(getType() & VIRTUAL_SITE) {
      return (*discardUnsentMessage_VirtualSite)(getVirtualSite(),msgNum);
    }
    Assert(getType() & REMOTE_SITE);
    //    return discardUnsentMessage_RemoteSite(getComObj(),msgNum);
    return 4711;
  }

  // Should this be redefined for ComObjects? AN!
  int getQueueStatus(int &noMsgs){
    unsigned short t=getType();
    if(!(t & CONNECTED)){
      noMsgs=0;
      return 0;}
      if(t & REMOTE_SITE){
        noMsgs = 0;
        return 0;//getQueueStatus_RemoteSite(getComObj());
      }
    Assert(t & VIRTUAL_SITE);
    return (*getQueueStatus_VirtualSite)(getVirtualSite(),noMsgs);} 

  SiteStatus siteStatus() {
    unsigned short t=getType();
    if(!(t & CONNECTED)) {
      if(t & PERM_SITE) 
	return SITE_PERM;
      return SITE_OK;
    }
    if(t & REMOTE_SITE)
      return SITE_OK;//AN! siteStatus_RemoteSite(getComObj());
    Assert(t & VIRTUAL_SITE);
    return (*siteStatus_VirtualSite)(getVirtualSite());
  }

  MonitorReturn demonitorQueue(){
    unsigned short t=getType();
    if(!(t & CONNECTED)) {
      return NO_MONITOR_EXISTS;}
    if(t & REMOTE_SITE){
//        demonitorQueue_RemoteSite(getComObj());
      return MONITOR_OK;}
    Assert(t & VIRTUAL_SITE);
    return (*demonitorQueue_VirtualSite)(getVirtualSite());}

  MonitorReturn monitorQueue(int size,int noMsgs,void *storePtr){
    if(connect()){
      if(getType() & REMOTE_SITE){
	//monitorQueue_RemoteSite(getComObj(),size); 
	  return MONITOR_OK;}
      Assert(getType() & VIRTUAL_SITE);
      return (*monitorQueue_VirtualSite)(getVirtualSite(),size,noMsgs,storePtr);}
    return MONITOR_PERM;}

  GiveUpReturn giveUp(GiveUpInput flag){ // PERM case 1 user initiated 
    unsigned short t=getType();
    if(t & PERM_SITE) {return SITE_NOW_PERM;}
    if(flag==ALL_GIVEUP){
      if(t & CONNECTED){
  	if(t & REMOTE_SITE) {
	  comController->deleteComObj(getComObj());
	  makePermConnected();
	}
	else
	  Assert(0);
      }
      makePerm();
    }
    if(siteStatus()==SITE_OK){ return SITE_NOW_NORMAL;}
    if(t & CONNECTED){
      if(t & REMOTE_SITE){
	comController->deleteComObj(getComObj());
	makePermConnected();
      }
      Assert(0);
    }
    makePerm();
    return GIVES_UP;
  }

  void marshalDSite(MarshalerBuffer *); 

// PERM case 2) discovered in unmarshaling or 3) in network
  void discoveryPerm(){ 
    PD((TCP_INTERFACE,"discoveryPerm of %d type %d",
  	this->getTimeStamp()->pid,flags));
    unsigned short t=getType();
    if(t==0) {
      flags |= PERM_SITE; 
      return;}    
//      Assert(!(t & SECONDARY_TABLE_SITE));
    if(t & PERM_SITE) return;
    if(t & CONNECTED){
      if(t & REMOTE_SITE){
	PD((TCP_INTERFACE,"discoveryPerm REMOTE_SITE"));
	if(t & VIRTUAL_INFO) {
	  dumpVirtualInfo(info);
	  info=NULL;
	}
	comController->deleteComObj(getComObj());
	makePermConnected();
	return;
      }
      Assert(t & VIRTUAL_SITE);
      dumpVirtualInfo(info);      
      info=NULL;
      (*discoveryPerm_VirtualSite)(getVirtualSite());
      makePermConnected();
      return;
    }
    makePerm();
    if(t & VIRTUAL_INFO) {
      dumpVirtualInfo(info);
    }
  }

  // AN!: left only for VS
  //
  // kost@ : applied whenever an "alive acknowledgement"
  // (e.g. virtual site's 'VS_M_SITE_ALIVE') message is received;
  void siteAlive() {
    if(connect()) {
      if(getType() & REMOTE_SITE) {
	//	siteAlive_RemoteSite(getComObj());
      } else {
	Assert(getType() & VIRTUAL_SITE);
	(*siteAlive_VirtualSite)(getVirtualSite());
      }
    }
  }

  //
  // provided for network and virtual site comm-layers
  //
  void communicationProblem(MsgContainer *msgC, FaultCode fc);

  // Remove after VS
  void communicationProblem(MessageType mt, DSite* storeSite,
			    int storeIndex, FaultCode fc, FaultInfo fi);

  void probeFault(ProbeReturn pr);

  void sitePermProblem(){
    discoveryPerm();}

  void monitorInvoke(MonitorReturn mt,int size,int noMsgs){
    Assert(0);
    OZ_error("not implemented");
    return;}

  Bool isMySite(){
    return flags & MY_SITE;}

  //
  // Used by GC in order to inform DSite about its unreachability;
  void siteZeroActiveRef(DSite *);  

  // misc - statistics;
  unsigned short getTypeStatistics() { return (getType()); }

  //
  // Debug stuff;
  char* stringrep();
  char* stringrep_notype();
};

//
// The 'VirtualInfo' is a supporting service for virtual sites.  DP
// library must be able at least to unmarshal, discard and marshal
// virtual info - a non-virtual site could get virtual info and must
// be able to keep&resend it. The only not used here service are
// constructors from a site ('VirtualInfo(DSite *bs, key_t mboxkIn)');
//
// The 'VirtualInfo' serves two purposes:
// (a) identifies the virtual sites group (the id of the master 
//     in this implementation).
// (b) contains information for receiving/sending messages to 
//     the site.
class VirtualInfo {
#ifdef VIRTUALSITES
  friend void DSite::initVirtualInfoArg(VirtualInfo *vi);
#endif
private:
  // The "street address" of the master virtual site
  // ("virtual site group id");
  ip_address address;
  port_t port;
  TimeStamp timestamp;

  // "box" of the site itself;
  key_t mailboxKey;

  //
private:
  //
  // used by 'DSite::initVirtualInfoArg(VirtualInfo *vi)';
  void setAddress(ip_address aIn) { address = aIn; }
  void setPort(port_t pIn) { port = pIn; }
  void setTimeStamp(TimeStamp &tsIn) { 
    timestamp.start = tsIn.start;
    timestamp.pid = tsIn.pid;
  }

  //
public:
  //
  void* operator new(size_t size) { return (malloc(size)); }
  void* operator new(size_t, void *place) { return (place); }

#ifdef VIRTUALSITES
  //
  // When a "plain" site declares itself as a virtual one (when it
  // creates a first slave site), copy the virtual site group id
  // from the site's 'Site' object, and put the mailbox key:
  VirtualInfo(DSite *bs, key_t mboxkIn)
    : mailboxKey(mboxkIn)
  {
    bs->initVirtualInfoArg(this);
  }

  //
  // When the 'myDSite' of a slave site is extended for the virtual
  // info (by 'BIVSinitServer'), then the mailbox key is given (the
  // only 'BIVSInitServer's argument), and the "virtual site group id"
  // is taken from the mailbox'es virtual info:
  VirtualInfo(VirtualInfo *mvi, key_t mboxkIn)
    : address(mvi->address), port(mvi->port), timestamp(mvi->timestamp),
      mailboxKey(mboxkIn)
  {}
#endif

  //
  // There is free list management of virtual info"s;
  ~VirtualInfo() { OZ_error("VirtualInfo is destroyed?"); }
  // There is nothing to be done when disposed;
  void destroy() {
    DebugCode(address = (ip_address) 0);
    DebugCode(port = (port_t) 0);
    DebugCode(timestamp.start = (time_t) 0);
    DebugCode(timestamp.pid = (int) 0);
    DebugCode(mailboxKey = (key_t) 0);
  }

#ifdef USE_FAST_UNMARSHALER   
  //
  // Another type of initialization - unmarshaliing:
  VirtualInfo(MarshalerBuffer *mb) {
    Assert(sizeof(ip_address) <= sizeof(unsigned int));
    Assert(sizeof(port_t) <= sizeof(unsigned short));
    Assert(sizeof(time_t) <= sizeof(unsigned int));
    Assert(sizeof(int) <= sizeof(unsigned int));
    Assert(sizeof(key_t) <= sizeof(unsigned int));
#ifndef VIRTUALSITES
    Assert(sizeof(key_t) == sizeof(unsigned int));
#endif

    //
    address = (ip_address) unmarshalNumber(mb);
    port = (port_t) unmarshalShort(mb);  
    timestamp.start = (time_t) unmarshalNumber(mb);
    timestamp.pid = (int) unmarshalNumber(mb);
    //
    mailboxKey = (key_t) unmarshalNumber(mb);
  }
#else
  //
  // Another type of initialization - robust unmarshaling:
  VirtualInfo(MarshalerBuffer *mb) {
    Assert(sizeof(ip_address) <= sizeof(unsigned int));
    Assert(sizeof(port_t) <= sizeof(unsigned short));
    Assert(sizeof(time_t) <= sizeof(unsigned int));
    Assert(sizeof(int) <= sizeof(unsigned int));
    Assert(sizeof(key_t) <= sizeof(unsigned int));
#ifndef VIRTUALSITES
    Assert(sizeof(key_t) == sizeof(unsigned int));
#endif
    //
    int trash;
    address = (ip_address) unmarshalNumberRobust(mb, &trash);
    port = (port_t) unmarshalShort(mb);  
    timestamp.start = (time_t) unmarshalNumberRobust(mb, &trash);
    timestamp.pid = (int) unmarshalNumberRobust(mb, &trash);
    //
    mailboxKey = (key_t) unmarshalNumberRobust(mb, &trash);
  }
#endif

  // 
  // NOTE: marshaling must be complaint with
  // '::unmarshalUselessVirtualInfo()';
  void marshal(MarshalerBuffer *mb) {
    Assert(sizeof(ip_address) <= sizeof(unsigned int));
    Assert(sizeof(port_t) <= sizeof(unsigned short));
    Assert(sizeof(time_t) <= sizeof(unsigned int));
    Assert(sizeof(int) <= sizeof(unsigned int));
    Assert(sizeof(key_t) <= sizeof(unsigned int));
#ifndef VIRTUALSITES
    Assert(sizeof(key_t) == sizeof(unsigned int));
#endif

    //
    marshalNumber(mb, address);
    marshalShort(mb, port);  
    marshalNumber(mb, timestamp.start);
    marshalNumber(mb, timestamp.pid);
    //
    marshalNumber(mb, mailboxKey);
  }

  //
  // Returns 'TRUE' if they are the same;
  Bool cmpVirtualInfos(VirtualInfo *vi) {
    if (address == vi->address && port == vi->port &&
	timestamp.start == vi->timestamp.start &&
	timestamp.pid == vi->timestamp.pid) 
      return (TRUE);
    else 
      return (FALSE);
  }

  //
  // There are NO public 'get' methods for address/port/timestamp!
  key_t getMailboxKey() { return (mailboxKey); }
  void setMailboxKey(key_t mbkIn) { mailboxKey = mbkIn; }
};

char *oz_site2String(DSite *s);

//
// Marshaller uses that;
#ifdef USE_FAST_UNMARSHALER   
DSite* unmarshalDSite(MarshalerBuffer *);
#else
DSite* unmarshalDSiteRobust(MarshalerBuffer *, int *);
#endif

//
// Faking a port from a ticket;
DSite *findDSite(ip_address a,int port, TimeStamp &stamp);

//
// kost@ : that's a part of the boot-up procedure ('perdioInit()');
// Actually, it is used by 'initNetwork()' because ip, port, timestamp
// are not known prior its initialization;
DSite* makeMyDSite(ip_address a, port_t p, TimeStamp &t);

//
GenHashNode *getPrimaryNode(GenHashNode* node, int &i);
GenHashNode *getSecondaryNode(GenHashNode* node, int &i);

//
void gcDSiteTable();

#endif // __DSITE_HH
