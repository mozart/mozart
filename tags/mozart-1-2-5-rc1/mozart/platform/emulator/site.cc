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

int BaseSite::hash() {
  BYTE *p=(BYTE*)&address;
  unsigned h=0,g;
  int i;
  int limit=sizeof(address);
  for(i=0;i<limit;i++,p++){
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  p= (BYTE*) &port;
  limit=sizeof(port);
  for(i=0;i<limit;i++,p++){
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  p= (BYTE*) &timestamp;
  limit=sizeof(port);
  for(i=0;i<limit;i++,p++){
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  return (int) h;}

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

#ifdef USE_FAST_UNMARSHALER   

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

#else

void BaseSite::unmarshalBaseSiteRobust(MarshalerBuffer* buf, int *error)
{
  address = unmarshalNumberRobust(buf, error);
  // address should be of int32
  if(*error || (address <= NON_BROADCAST_MIN)) {
    return;
  }
  port = unmarshalShort(buf);  
  timestamp.start = unmarshalNumberRobust(buf, error);
  if(*error || timestamp.start < 0) {
    return;
  }
  timestamp.pid=unmarshalNumberRobust(buf, error);
  // andreas & kost@ : Windows* return arbitrary pid_t"s, 
  // so no MAX_PID whatsoever!
}
void BaseSite::unmarshalBaseSiteGNameRobust(MarshalerBuffer* buf, int *error)
{
  address = unmarshalNumberRobust(buf, error);
  if(*error) return;
  port = 0;
  timestamp.start = unmarshalNumberRobust(buf, error);
  if(*error) return;
  timestamp.pid = unmarshalNumberRobust(buf, error);
  if(*error) return;
}

#endif


static ip_address getMySiteIP()
{
  char *nodename = oslocalhostname();
  if(nodename==0) { 
    // OZ_warning("getMySiteIP: don't know local hostname"); 
    nodename = "localhost";
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
  siteTable->insert(mySite, mySite->hash());
}

void SiteHashTable::cleanup(){
  GenHashNode *ghn,*ghn1;
  Site* s;
  int i=0;
  ghn=getFirst(i);
  while(ghn!=NULL){
    s = (Site*) (ghn->getBaseKey());
    if((!(s->hasGCFlag())) && s!=mySite){
      s->freeSite();
      deleteFirst(ghn);
      ghn=getByIndex(i);
      continue;}
    else{
      s->resetGCFlag();}
    ghn1=ghn->getNext();
    while(ghn1!=NULL){
      s = (Site*) (ghn1->getBaseKey());
      if((s->hasGCFlag()) || (s==mySite)){
	s->resetGCFlag();}
      else{
	s->freeSite();
	deleteNonFirst(ghn,ghn1);
	ghn1=ghn->getNext();
	continue;}
      ghn=ghn1;
      ghn1=ghn1->getNext();}
    i++;
    ghn=getByIndex(i);}
  return;
}

//
void gCollectSiteTable() {
  siteTable->cleanup();
}

Site *mySite;

#ifdef USE_FAST_UNMARSHALER   
Site* unmarshalSite(MarshalerBuffer *buf)
{
  Site tryS;

  //
  tryS.unmarshalBaseSiteGName(buf);

  //
  int hvalue = tryS.hash();
  Site *s = siteTable->find(&tryS, hvalue);
  if (!s) {
    s = new Site(&tryS);
    siteTable->insert(s, hvalue);
  }
  return (s);
}
#else
Site* unmarshalSiteRobust(MarshalerBuffer *buf, int *error)
{
  Site tryS;

  //
  tryS.unmarshalBaseSiteGNameRobust(buf, error);
  if (*error) return ((Site *) 0);

  //
  int hvalue = tryS.hash();
  Site *s = siteTable->find(&tryS, hvalue);
  if (!s) {
    s = new Site(&tryS);
    siteTable->insert(s, hvalue);
  }
  return (s);
}
#endif
