/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Konstantin Popov (kost@sics.se)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __SITE_HH
#define __SITE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "genhashtbl.hh"
#include "mbuffer.hh"

// time_t
#include <time.h>

// Broadcast IP-addresses starts with zero-byte
#define NON_BROADCAST_MIN 16777215

typedef unsigned short oz_port_t;
typedef unsigned int ip_address;

//
//
class TimeStamp {
public:
  time_t start;
  int pid;

  //
public:
  TimeStamp() { DebugCode(start = (time_t) 0; pid = 0;); }
  TimeStamp(time_t s, int p): start(s), pid(p) {}
};

//
// 'BaseSite' (now) constitutes a base for both 'NSite's ("naming
// site") and 'DSite's ("distribution site");
class BaseSite{
friend class Site;
friend class DSite;
  //
protected:
  ip_address address;
  TimeStamp timestamp;
  oz_port_t port;

public:
#ifdef DEBUG_CHECK
  // never create these...
  void* operator new(size_t) {
    OZD_error("BaseSite is created???"); return (void *) 1;
  }
  void* operator new(size_t, void *) {
    OZD_error("BaseSite is created???"); return (void *) 1;
  }
#endif
  //
  BaseSite() {}			// ... just allocating space for it;
  BaseSite(ip_address a, oz_port_t p, TimeStamp &t)
    : address(a), port(p), timestamp(t) {}
  BaseSite(ip_address a, oz_port_t p, TimeStamp *t)
    : address(a), port(p), timestamp(*t) {}

  //
  // These should be used for unmarshalling, debugging/output, and -
  // for creating of tickets;
  ip_address getAddress() { return (address); }
  oz_port_t getPort() { return (port); }
  TimeStamp *getTimeStamp() { return (&timestamp); }

  //
  int hash();

  //
  void marshalBaseSite(MarshalerBuffer* buf);
  void marshalBaseSiteForGName(MarshalerBuffer* buf);
  void marshalBaseSiteForGName(PickleMarshalerBuffer *buf);
#ifdef USE_FAST_UNMARSHALER   
  void unmarshalBaseSite(MarshalerBuffer* buf);
  void unmarshalBaseSiteGName(MarshalerBuffer* buf);
#else
  void unmarshalBaseSiteRobust(MarshalerBuffer* buf, int *error);
  void unmarshalBaseSiteGNameRobust(MarshalerBuffer* buf, int *error);
#endif

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

//
#define NSITE_GC_MARK      0x1

//
// 'Site' is used for naming sites (GNames --> pickling);
class Site : public BaseSite {
private:
  unsigned short flags;		// essentially a GC bit only;

  //
public:
  //
  void* operator new(size_t size){ 
    Assert(sizeof(Site) <= sizeof(Construct_5));
    return ((Site *) genFreeListManager->getOne_5());}

  void freeSite(){
    genFreeListManager->putOne_5((FreeListEntry*) this);}

  unsigned short getType() { return (flags); }

  //
  Site() {}
  Site(ip_address a, oz_port_t p, TimeStamp &t)
    : BaseSite (a, p, t) {}
  Site(Site *s)
    : BaseSite(s->address, s->port, s->timestamp) {}
  ~Site() {}

  //
  void setGCFlag() { flags |= NSITE_GC_MARK; }
  void resetGCFlag() { flags &= ~NSITE_GC_MARK; }
  Bool hasGCFlag() { return (flags & NSITE_GC_MARK); }
  void marshalSite(MarshalerBuffer *buf) { marshalBaseSite(buf); }
  void marshalSiteForGName(MarshalerBuffer *buf) {
    marshalBaseSiteForGName(buf);
  }
  void marshalSiteForGName(PickleMarshalerBuffer *buf) {
    marshalBaseSiteForGName(buf);
  }
};

//
#define SITE_TABLE_SIZE    10

//
class SiteHashTable: public GenHashTable {
public:
  SiteHashTable(int size): GenHashTable(size) {}

  //
  Site *find(Site *s, int hvalue) {
    GenHashNode *ghn = htFindFirst(hvalue);
    Site* found;
    while (ghn!=NULL) {
      found = (Site*) (ghn->getBaseKey());
      if(s->compareSites(found)==0) return found;
      ghn=htFindNext(ghn,hvalue);}
    return NULL;}

  void insert(Site *s, int hvalue) {
    GenHashBaseKey *ghn_bk;
    GenHashEntry *ghn_e=NULL;  
    ghn_bk = (GenHashBaseKey*)(void*) s;
    htAdd(hvalue,ghn_bk,ghn_e);}

  void remove(Site *s, int hvalue) {
    GenHashNode *ghn=htFindFirst(hvalue);
    Site* found;
    while(ghn!=NULL){
      found = (Site*) (ghn->getBaseKey());
      if(s->compareSites(found)==0){
	htSub(hvalue,ghn);
	return;}
      ghn=htFindNext(ghn,hvalue);}
    Assert(0);}

  //
  void cleanup();
};

//
extern SiteHashTable* siteTable;

//
void gCollectSiteTable();

//
// Marshaller uses that;
#ifdef USE_FAST_UNMARSHALER   
Site* unmarshalSite(MarshalerBuffer *);
#else
Site* unmarshalSiteRobust(MarshalerBuffer *, int *);
#endif

//
// There is one universe-wide known site object:
//
extern Site *mySite;

//
// kost@ : that's a part of the boot procedure ('AM::init()');
void initSite();

#endif /* __SITE_HH */
