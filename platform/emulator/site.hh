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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
#include "msgbuffer.hh"
#include "marshaler.hh"

// time_t
#include <time.h>

typedef unsigned short port_t;
typedef unsigned long ip_address;

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
  port_t port;

public:
  // never create these...
  void* operator new(size_t) {
    OZ_error("BaseSite is created???"); return (0);
  }
  void* operator new(size_t, void *) {
    OZ_error("BaseSite is created???"); return (0);
  }

  //
  BaseSite() {}                 // ... just allocating space for it;
  BaseSite(ip_address a, port_t p, TimeStamp &t)
    : address(a), port(p), timestamp(t) {}
  BaseSite(ip_address a, port_t p, TimeStamp *t)
    : address(a), port(p), timestamp(*t) {}

  //
  // These should be used for unmarshalling, debugging/output, and -
  // for creating of tickets;
  ip_address getAddress() { return (address); }
  port_t getPort() { return (port); }
  TimeStamp *getTimeStamp() { return (&timestamp); }

  //
  int hash();

  //
  // Debug stuff;
  char* stringrep();
  char* stringrep_notype();

  //
  void marshalBaseSite(MsgBuffer* buf){
    marshalNumber(address,buf);
    marshalShort(port,buf);
    marshalNumber(timestamp.start,buf);
    marshalNumber(timestamp.pid,buf);
  }
  void marshalBaseSiteForGName(MsgBuffer* buf){
    marshalNumber(address,buf);
    marshalNumber(timestamp.start,buf);
    marshalNumber(timestamp.pid,buf);
  }
  void unmarshalBaseSite(MsgBuffer* buf){
    address=unmarshalNumber(buf);
    port=unmarshalShort(buf);
    timestamp.start=unmarshalNumber(buf);
    timestamp.pid=unmarshalNumber(buf);
  }

  void unmarshalBaseSiteGName(MsgBuffer* buf, int minor){
    address=unmarshalNumber(buf);
    port = (minor==0) ? unmarshalShort(buf) : 0;
    timestamp.start=unmarshalNumber(buf);
    timestamp.pid=unmarshalNumber(buf);
  }

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
  unsigned short flags;         // essentially a GC bit only;

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
  Site(ip_address a, port_t p, TimeStamp &t)
    : BaseSite (a, p, t) {}
  Site(Site *s)
    : BaseSite(s->address, s->port, s->timestamp) {}
  ~Site() {}

  //
  void setGCFlag() { flags |= NSITE_GC_MARK; }
  void resetGCFlag() { flags &= ~NSITE_GC_MARK; }
  Bool hasGCFlag() { return (flags & NSITE_GC_MARK); }
  void marshalSite(MsgBuffer *buf) { marshalBaseSite(buf); }
  void marshalSiteForGName(MsgBuffer *buf) { marshalBaseSiteForGName(buf); }
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
      GenCast(ghn->getBaseKey(),GenHashBaseKey*,found,Site*);
      if(s->compareSites(found)==0) return found;
      ghn=htFindNext(ghn,hvalue);}
    return NULL;}

  void insert(Site *s, int hvalue) {
    GenHashBaseKey *ghn_bk;
    GenHashEntry *ghn_e=NULL;
    GenCast(s,Site*,ghn_bk,GenHashBaseKey*);
    htAdd(hvalue,ghn_bk,ghn_e);}

  void remove(Site *s, int hvalue) {
    GenHashNode *ghn=htFindFirst(hvalue);
    Site* found;
    while(ghn!=NULL){
      GenCast(ghn->getBaseKey(),GenHashBaseKey*,found,Site*);
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
void gcSiteTable();

//
// Marshaller uses that;
Site* unmarshalSite(MsgBuffer *);

//
// There is one universe-wide known site object:
//
extern Site *mySite;

//
// kost@ : that's a part of the boot-up procedure ('AM::init()');
void initSite();

#endif /* __SITE_HH */
