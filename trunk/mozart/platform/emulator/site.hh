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
#include "hashtbl.hh"
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
  int pid;
  time_t start;

  //
public:
  TimeStamp() { DebugCode(start = (time_t) 0; pid = 0;); }
  TimeStamp(time_t s, int p) : pid(p), start(s) {}

  // positive if 'this' is younger (that is, larger stamp) than 't';
  // NOTE: start with the time stamp itself;
  int compareTimeStamps(TimeStamp *t) {
    int cmp;
    if ((cmp = (int) (start - t->start)) != 0)
      return (cmp);
    else
      return ((int) (pid - t->pid));
  }
};

//
// 'BaseSite' (now) constitutes a base for both 'NSite's ("naming
// site") and 'DSite's ("distribution site");
class BaseSite : public CppObjMemory {
friend class Site;
friend class DSite;
  //
protected:
  ip_address address;
  oz_port_t port;
  TimeStamp timestamp;

public:
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
  void marshalBaseSite(MarshalerBuffer* buf);
  void marshalBaseSiteForGName(MarshalerBuffer* buf);
  void marshalBaseSiteForGName(PickleMarshalerBuffer *buf);
  void unmarshalBaseSite(MarshalerBuffer* buf);
  void unmarshalBaseSiteGName(MarshalerBuffer* buf);
};

//
#define NSITE_GC_MARK      0x1

//
// 'Site' is used for naming sites (GNames --> pickling);
class Site : public BaseSite , public GenDistEntryNode<Site> {
private:
  unsigned short flags;		// essentially a GC bit only;

  //
public:
  Site() {}
  Site(ip_address a, oz_port_t p, TimeStamp &t)
    : BaseSite (a, p, t) {}
  Site(Site *s)
    : BaseSite(s->address, s->port, s->timestamp) {}
  ~Site() {}

  //
  unsigned short getType() { return (flags); }

  //
  unsigned int value4hash() {
    unsigned int v = (unsigned int) address;
    v = ((v<<9)^(v>>23)) ^ ((unsigned int) port);
    v = ((v<<13)^(v>>19)) ^ ((unsigned int) timestamp.start);
    v = ((v<<5)^(v>>27)) ^ ((unsigned int) timestamp.pid);
    // originally, that was like that:
    //      unsigned int v = 
    //        ((unsigned int) address) +
    //        ((unsigned int) port) +
    //        ((unsigned int) timestamp.start) +
    //        ((unsigned int) timestamp.pid);
    return (v);
  }
  //
  int compare(BaseSite *s) {
    int cmp;
    if ((cmp = (int) (address - s->address)) != 0)
      return (cmp);
    else if ((cmp = (int) (port - s->port)) != 0)
      return (cmp);
    else
      return (timestamp.compareTimeStamps(&(s->timestamp)));
  }

  //
  void setGCFlag() { flags |= NSITE_GC_MARK; }
  void resetGCFlag() { flags &= ~NSITE_GC_MARK; }
  Bool hasGCFlag() { return (flags & NSITE_GC_MARK); }
  void marshalSite(MarshalerBuffer *buf) {
    marshalBaseSite(buf);
  }
  void marshalSiteForGName(MarshalerBuffer *buf) {
    marshalBaseSiteForGName(buf);
  }
  void marshalSiteForGName(PickleMarshalerBuffer *buf) {
    marshalBaseSiteForGName(buf);
  }
};

//
#define SITE_TABLE_SIZE    4

//
class SiteHashTable : public GenDistEntryTable<Site> {
public:
  SiteHashTable(int size) : GenDistEntryTable<Site>(size) {}

  //
  void insert(Site *s) {
    Assert(!htFind(s));
    htAdd(s);
  }
  Site *find(Site *s) {
    return ((Site *) htFind(s));
  }
  // nothing is actually removed like this:
  void remove(Site *s) {
    Assert(htFind(s));
    htDel(s);
  }

  //
  void cleanup();
};

//
extern SiteHashTable* siteTable;

//
void gCollectSiteTable();

//
// Marshaller uses that;
Site* unmarshalSite(MarshalerBuffer *);

//
// There is one universe-wide known site object:
//
extern Site *mySite;

//
// kost@ : that's a part of the boot procedure ('AM::init()');
void initSite();

#endif /* __SITE_HH */
