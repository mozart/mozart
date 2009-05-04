/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "site.hh"
#pragma implementation "mbuffer.hh"
#endif

#include "base.hh"
#include "am.hh"
#include "pickleBase.hh"
#include "os.hh"
#include "site.hh"
#include <sys/types.h>

#ifndef WINDOWS
#include <netinet/in.h>
#include <netdb.h>
#endif

//
void BaseSite::marshalBaseSite(MarshalerBuffer* buf)
{
  marshalNumber(buf, address);
  marshalShort(buf, port);  
  marshalNumber(buf, timestamp.start);
  marshalNumber(buf, timestamp.pid);
}
void BaseSite::marshalBaseSiteForGName(MarshalerBuffer* buf)
{
  marshalNumber(buf, address);
  Assert(port == 0); // kost@ : otherwise hashing will be broken;
  marshalNumber(buf, timestamp.start);
  marshalNumber(buf, timestamp.pid);
}
void BaseSite::marshalBaseSiteForGName(PickleMarshalerBuffer* buf)
{
  marshalNumber(buf, address);
  Assert(port == 0); // kost@ : otherwise hashing will be broken;
  marshalNumber(buf, timestamp.start);
  marshalNumber(buf, timestamp.pid);
}

//
void BaseSite::unmarshalBaseSite(MarshalerBuffer* buf)
{
  address = unmarshalNumber(buf);
  port = unmarshalShort(buf);  
  timestamp.start = unmarshalNumber(buf);
  timestamp.pid = unmarshalNumber(buf);
}
void BaseSite::unmarshalBaseSiteGName(MarshalerBuffer* buf)
{
  address = unmarshalNumber(buf);
  port = 0;
  timestamp.start = unmarshalNumber(buf);
  timestamp.pid = unmarshalNumber(buf);
}


static ip_address getMySiteIP()
{
  char *nodename = oslocalhostname();
  if(nodename==0) { 
    // OZ_warning("getMySiteIP: don't know local hostname"); 
    nodename = (char*) malloc(10);
    strcpy(nodename,"localhost");
  }
  
  // very simple for now
  ip_address ret = (ip_address) update_crc((crc_t)osTotalTime(),
					   (unsigned char*)nodename,
					   strlen(nodename));
  free(nodename);
  return ret;

#if 0
  // this will open an IP connection, so don't do it
  struct hostent *hostaddr=gethostbyname(nodename);
  if(hostaddr==0) {
    OZD_error("getMySiteIP: can't resolve local hostname (%s)", nodename);
  }
  free(nodename);
  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));
  ip_address ip=ntohl(tmp.s_addr);
  return (ip);
#endif
}

//
#include "hashtblDefs.cc"
template class GenDistEntryTable<Site>;

//
SiteHashTable* siteTable = 0;

//
void initSite()
{
  ip_address myIP = getMySiteIP();
  TimeStamp timestamp(time(0), osgetEpid());
  // RS: should also use the port for making it more unique
  mySite = new Site(myIP, (oz_port_t) 0, timestamp);
  //
  siteTable = new SiteHashTable(SITE_TABLE_SIZE);
  siteTable->insert(mySite);
}

void SiteHashTable::cleanup()
{
  for (int i = getSize(); i--; ) {
    Site **ps = getFirstNodeRef(i);
    Site *site = *ps;
    while (site) {
      if (!(site->hasGCFlag()) && site != mySite) {
	deleteNode(site, ps);
	delete site;
	// 'ps' stays in place;
      } else {
	site->resetGCFlag();
	ps = (Site **) site->getNextNodeRef();
      }

      //
      site = *ps;
    }
  }
  compactify();
}

//
void gCollectSiteTable()
{
  siteTable->cleanup();
}

Site *mySite;

Site* unmarshalSite(MarshalerBuffer *buf)
{
  Site tryS;

  //
  tryS.unmarshalBaseSiteGName(buf);

  //
  Site *s = siteTable->find(&tryS);
  if (!s) {
    s = new Site(&tryS);
    siteTable->insert(s);
  }
  return (s);
}
