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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "site.hh"
#pragma implementation "msgbuffer.hh"
#endif

#include "base.hh"
#include "site.hh"
#include "am.hh"
#include "os.hh"
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

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

static ip_address getMySiteIP()
{
  char *nodename = oslocalhostname();
  if(nodename==0) { OZ_error("getMySiteIP: don't know local hostname"); }
  struct hostent *hostaddr;
  hostaddr=gethostbyname(nodename);
  if(hostaddr==0) {
    OZ_error("getMySiteIP: can't resolve local hostname (%s)", nodename);
  }
  free(nodename);
  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));
  ip_address ip=ntohl(tmp.s_addr);
  return (ip);
}

//
SiteHashTable* siteTable = 0;

//
void initSite()
{
  ip_address myIP = getMySiteIP();
  TimeStamp timestamp(time(0), osgetpid());
  mySite = new Site(myIP, (port_t) 0, timestamp);
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
    GenCast(ghn->getBaseKey(),GenHashBaseKey*,s,Site*);
    if((!(s->hasGCFlag())) && s!=mySite){
      s->freeSite();
      deleteFirst(ghn);
      ghn=getByIndex(i);
      continue;}
    else{
      s->resetGCFlag();}
    ghn1=ghn->getNext();
    while(ghn1!=NULL){
      GenCast(ghn1->getBaseKey(),GenHashBaseKey*,s,Site*);
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
void gcSiteTable() {
  siteTable->cleanup();
}

Site *mySite;

Site* unmarshalSite(MsgBuffer *buf)
{
  Site tryS;

  //
  int minor = buf->getMinor();
  tryS.unmarshalBaseSiteGName(buf,minor);

  //
  int hvalue = tryS.hash();
  Site *s = siteTable->find(&tryS, hvalue);
  if (!s) {
    s = new Site(&tryS);
    siteTable->insert(s, hvalue);
  }
  return (s);
}
