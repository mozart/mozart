/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
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
#include "os.hh"
#include "comObj.hh"
#include "hashtbl.hh"

/**********************************************************************/
/*   SECTION :: Site                                                  */
/**********************************************************************/

// AN! remove REMOTE also, possibly make it ACTIVE
//
// 'REMOTE'
#define REMOTE_SITE           0x1
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

  // AN! REMOTE_SITE=>ACTIVE
  REMOTE_SITE			// Active means connecteable
  REMOTE_SITE | CONNECTED	// connected

  //
  PERM_SITE			// permanently down;
  PERM_SITE | SECONDARY_TABLE_SITE        // ... in secondary table;

  // AN! does this still exist?
  // next 1 is transitory:
  // discovered by third party to be dead:
  REMOTE_SITE | PERM_SITE | CONNECTED     //

  //
  MY_SITE			// 
*/

enum FindType {
  SAME = 0,
  NONE,
  I_AM_YOUNGER,
  I_AM_OLDER
};

//

//
// There is one perdio-wide known "distribition" site object.  
// This is like 'mySite', but provides communication peers for 
// accessing (addressing) 'mySite';
extern DSite *myDSite;

class NetAddress {
public:
  /* DummyClassConstruction(NetAddress) */
  DSite* site;
  Ext_OB_TIndex index;

public:
  NetAddress() {}
  NetAddress(DSite* s, Ext_OB_TIndex i) : site(s), index(i) {}
  
  void set(DSite *s, Ext_OB_TIndex i) { site=s, index=i; }

  Bool same(NetAddress *na) { return (na->site==site && na->index==index); }

  Bool isLocal() { return (site==myDSite); }
};

//
class DSite: public BaseSite, public GenDistEntryNode<DSite> {
private:
  unsigned short flags;
  ComObj* comObj;

  //
protected:
  unsigned short getType() { return (flags); }

  //
private:
  void setType(unsigned int i) { flags=i; }

  void disconnect() {
    flags &= (~CONNECTED);
    comObj = NULL;
  }

  Bool connect() {
    unsigned int t=getType();
    PD((SITE,"connect, the type of this site: %d",t));
    Assert(!(t & MY_SITE));
    if(t & CONNECTED) return OK;
    if(t & (PERM_SITE)) return NO;

    Assert(t & REMOTE_SITE);
    comObj=createComObj(this);
    // AN! this instantiation makes latter changes of perdioCh... unused
    comObj->installProbe(0,ozconf.dpProbeTimeout,ozconf.dpProbeInterval);
    Assert(comObj!=NULL);
    PD((SITE,"connect; not connected yet, connecting to remote %d",comObj));
    flags |= CONNECTED;    
    return OK;
  }    

  void makePermConnected() {
    flags |= PERM_SITE;
    flags &= (~CONNECTED);
  }              

  void makePerm() {
    flags |= PERM_SITE;
  }

public:
  // If this site allready has a comObj that one is returned,
  // else the incoming is used and the state is set to connected.
  ComObj *setComObj(ComObj *comObj) {
    unsigned int t=getType();
    if(t & PERM_SITE) {
      // This site is already discovered perm. Due to a late 
      // message delivery, a comObj appears. Refuse it by returning -1.
      return (ComObj *) -1;
    }
    if(t & CONNECTED) 
      return this->comObj;
    else {
      setType(CONNECTED|REMOTE_SITE);
      comObj->installProbe(0,ozconf.dpProbeTimeout,ozconf.dpProbeInterval);
      this->comObj=comObj;
      return NULL;
    }
  }

  ComObj* getComObj() {   
    if(!connect()) {PD((SITE,"getComObj not connected"));return NULL;}
    Assert(getType() & CONNECTED);
    Assert(getType() & REMOTE_SITE);
    PD((SITE,"getComObj returning the remote %d",comObj));
    Assert(comObj!=NULL);
    return comObj;}

  //
public:
  DSite() {}			// 'unmarshalDSite()';
  DSite(ip_address a, oz_port_t p, TimeStamp *t)
    : BaseSite(a, p, t) {
    DebugCode(flags = (unsigned short) -1);
  }
  DSite(ip_address a, oz_port_t p, TimeStamp* t, unsigned short ty)
    : BaseSite(a, p, t) {
    flags=ty;
  }

  DSite(ip_address a, oz_port_t p, TimeStamp &t)
    : BaseSite(a, p, t) {
    DebugCode(flags = (unsigned short) -1);
  }
  DSite(ip_address a, oz_port_t p, TimeStamp& t, unsigned short ty)
    : BaseSite(a, p, t) {
    flags=ty;
  }

  // Support for GenDistEntryNode.
  // Hashing & comparison proceeds on address/port!
  unsigned int value4hash() {
    unsigned int v = (unsigned int) address;
    v = ((v<<7)^(v>>25)) ^ ((unsigned int) port);
    return (v);
  }
  int compare(DSite *hbs) {
    int cmpSite = (int) hbs->address - (int) address;
    if (cmpSite == 0) {
      return ((int) hbs->port - (int) port);
    } else {
      return (cmpSite);
    }
  }

  void setMyDSite() { setType(MY_SITE); }

  void makeGCMarkSite() { flags |= GC_MARK; }
  void removeGCMarkSite() { flags &= ~(GC_MARK); }
  Bool isGCMarkedSite() { return flags & GC_MARK; }

  // AN! what does 'active' really mean? 
  Bool ActiveSite() {
    return ((getType() & REMOTE_SITE) ? OK : NO);
  }

  Bool remoteComm() {
    if (getType() & REMOTE_SITE) return OK;
    if (getType() & PERM_SITE) return OK;      // ATTENTION
    return NO;
  }

  void passiveToPerm() {
    Assert(!(ActiveSite()));
    flags |= PERM_SITE;
  }

  //
  void putInSecondary() {
    Assert(!(MY_SITE & getType()));
    setType(getType() | SECONDARY_TABLE_SITE);
  }
  Bool isInSecondary() {
    if(getType() & SECONDARY_TABLE_SITE) return OK;
    else return NO;
  }

  //
  Bool isConnected() { return ((getType() & CONNECTED)); }
  Bool isPerm(){return (getType() & PERM_SITE);}

  Bool canBeFreed(){
    Assert(!isGCMarkedSite());
    if(flags & MY_SITE) {return NO;}
    unsigned short t=getType();
    if(ActiveSite() && !isPerm() &&
       ((t & CONNECTED) /*|| u.readCtr!=0*/)){ // Check over tests AN!
      Assert(t & REMOTE_SITE);
      Assert(comObj!=NULL);
      if(comObj->canBeFreed()) {
	comController->deleteComObj(comObj);
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
    setType(MY_SITE);
  }

  // 
  // kost@ : init's are for new(ly inserted) site objects;
  void initRemote() {
    setType(REMOTE_SITE);
  }
  void initPerm() {
    setType(PERM_SITE);
  }

  //
  // kost@ : 'makeActive*()' are for former passive (GName'd) site
  // objects
  void makeActiveRemote() {
    Assert(!(getType() & MY_SITE)); 
    setType(REMOTE_SITE);
  }

  // provided to network-comm
  void dumpRemoteSite() {
    Assert(getType() & CONNECTED);
    Assert(getType() & REMOTE_SITE);
    disconnect();
  }

  // for use by the protocol-layer
  int send(MsgContainer *msgC) {
    if(connect()){
      Assert(getType() & REMOTE_SITE);
      getComObj()->send(msgC);
      return ACCEPTED;
    } else {
      return PERM_NOT_SENT;
    }
  }

  //
  int getQueueStatus(){
    unsigned short t=getType();
    if(!(t & CONNECTED)){
      return 0;
    } else {
      Assert(t & REMOTE_SITE);
      return getComObj()->getQueueStatus();
    }
  };

  SiteStatus siteStatus() {
    unsigned short t=getType();
    return ((t & PERM_SITE) ? SITE_PERM : SITE_OK);
  }

  //
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
      Assert(t & REMOTE_SITE);
      PD((TCP_INTERFACE,"discoveryPerm REMOTE_SITE"));
      comController->deleteComObj(getComObj());
      makePermConnected();
      return;
    }
    makePerm();
  }

  //
  // provided for comm-layer
  //
  void communicationProblem(MsgContainer *msgC, FaultCode fc);
  void probeFault(ProbeReturn pr);

  // misc - statistics;
  //    unsigned short getTypeStatistics() { return (getType()); }
  OZ_Term getStateStatistics();

  char* stringrep();
  char* stringrep_notype();

  OZ_Term getOzRep();
};

char *oz_site2String(DSite *s);

//
// Marshaller uses that;
DSite* unmarshalDSite(MarshalerBuffer *mb);

//
// Faking a port from a ticket;
DSite *findDSite(ip_address a, int port, TimeStamp &stamp);

//
// kost@ : that's a part of the boot-up procedure ('perdioInit()');
// Actually, it is used by 'initNetwork()' because ip, port, timestamp
// are not known prior its initialization;
DSite* makeMyDSite(ip_address a, oz_port_t p, TimeStamp &t);

//
void gcDSiteTable();

//
class DSiteHashTable : public GenDistEntryTable<DSite> {
public: 
  DSiteHashTable(int sizeAsPowerOf2)
    : GenDistEntryTable<DSite>(sizeAsPowerOf2) {}

  //
  FindType find(DSite *target, DSite* &found) {
    found = (DSite *) htFind(target);
    if (found) {
      TimeStamp *targetTS = target->getTimeStamp();
      TimeStamp *foundTS = found->getTimeStamp();
      int cmp = foundTS->compareTimeStamps(targetTS);
      if (cmp)
	return (cmp > 0 ? I_AM_YOUNGER : I_AM_OLDER);
      else
	return (SAME);
    } else {
      return (NONE);
    }
  }

  //
  void insert(DSite* s) {
    htAdd(s);
  }
  void insertAny(DSite* s) {
    htAdd(s);
  }
  void remove(DSite *s) {
    Assert(htFind(s));
    htDel(s);
  }

  //
  void cleanup() {
    for (int i = getSize(); i--; ) {
      DSite **pds = getFirstNodeRef(i);
      DSite *site = *pds;
      while (site) {
	if (!(site->isGCMarkedSite()) && site->canBeFreed()) {
	  deleteNode(site, pds);
	  delete site;
	  // 'pds' stays in place;
	} else {
	  site->removeGCMarkSite();
	  pds = site->getNextNodeRef();
	}

	//
	site = *pds;
      }
    }
  }

  void print_contents(){
    for (int i = getSize(); i--; ) {
      printf("Entry %d:", i);
      DSite *site = getFirstNode(i);
      while (site) {
	printf("\t%s\n", site->stringrep());
	site = site->getNext();
      }
      printf("\n");
    }
  }
};

extern DSiteHashTable* primarySiteTable;
extern DSiteHashTable* secondarySiteTable;

#endif // __DSITE_HH
