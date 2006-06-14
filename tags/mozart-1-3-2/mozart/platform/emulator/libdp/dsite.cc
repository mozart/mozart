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

#if defined(INTERFACE)
#pragma implementation "dsite.hh"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>

#include "os.hh"
#include "dsite.hh"
#include "comm.hh"
#include "mbuffer.hh"
#include "byteBuffer.hh"
#define SITE_CUTOFF           100

/**********************************************************************/
/*   SECTION ::  Site Hash Table                                     */
/**********************************************************************/


#define PRIMARY_SITE_TABLE_SIZE    4
#define SECONDARY_SITE_TABLE_SIZE  6

//
#include "hashtblDefs.cc"
template class GenDistEntryTable<DSite>;

DSiteHashTable* primarySiteTable
= new DSiteHashTable(PRIMARY_SITE_TABLE_SIZE);
DSiteHashTable* secondarySiteTable
= new DSiteHashTable(SECONDARY_SITE_TABLE_SIZE);

void gcDSiteTable()
{
  primarySiteTable->cleanup();
  secondarySiteTable->cleanup();
}

/**********************************************************************/
/*   SECTION ::  General unmarshaling routines                        */
/**********************************************************************/

static inline
void primaryToSecondary(DSite *s)
{
  primarySiteTable->remove(s);
  s->putInSecondary();
  secondarySiteTable->insertAny(s);
}

static
DSite* unmarshalDSiteInternal(MarshalerBuffer *buf, DSite *tryS, MarshalTag mt)
{
  DSite *s;  
  
  FindType rc = primarySiteTable->find(tryS,s);    
  switch(rc){
  case SAME: {
    PD((SITE,"unmarshalsite SAME"));
    if(mt==DIF_SITE_PERM){
      if(s->isPerm()){
	return s;}
      s->discoveryPerm();
      return s;}

    //
    Assert(mt == DIF_SITE);
    if(s != myDSite && !s->ActiveSite()) {
      s->makeActiveRemote();}
    return s;}

  case NONE:
    PD((SITE,"unmarshalsite NONE"));
    break;
    
  case I_AM_YOUNGER:{
    PD((SITE,"unmarshalsite I_AM_YOUNGER"));
    if (secondarySiteTable->find(tryS,s) == SAME) 
      // kost@ : 's' is not necessarily the SAME: it can have a
      // different timestamp!
      return s;
    s = new DSite(tryS->getAddress(), tryS->getPort(), tryS->getTimeStamp(),
		  PERM_SITE);
    secondarySiteTable->insert(s);
    return s;}
  
  case I_AM_OLDER:{
    PD((SITE,"unmarshalsite I_AM_OLDER"));
    primaryToSecondary(s);
    break;}

  default: Assert(0);}

  // type is left blank here:
  s = new DSite(tryS->getAddress(), tryS->getPort(), tryS->getTimeStamp());
  primarySiteTable->insert(s);
  
  //
  if(mt==DIF_SITE_PERM){
    PD((SITE,"initsite DIF_SITE_PERM"));
    s->initPerm();
    return s;}
  
  Assert(mt == DIF_SITE);
  PD((SITE,"initsite DIF_SITE"));
  s->initRemote();
  return s;
}

DSite *findDSite(ip_address a,int port,TimeStamp &stamp)
{
  DSite tryS(a, port, stamp);
  return (unmarshalDSiteInternal(NULL, &tryS, DIF_SITE));
}

DSite* unmarshalDSite(MarshalerBuffer *buf)
{
  PD((UNMARSHAL,"site"));
  MarshalTag mt = (MarshalTag) buf->get();
  
  // Shortcut when sending own site. 
  if (mt == DIF_SITE_SENDER)
    return   ((ByteBuffer *)buf)->getSite();
  DSite tryS;

  tryS.unmarshalBaseSite(buf);
  return unmarshalDSiteInternal(buf, &tryS, mt);
}

/**********************************************************************/
/*   SECTION :: BaseSite object methods                               */
/**********************************************************************/

OZ_Term DSite::getStateStatistics() {
  if(isConnected()) {
    // We have a comObj, the question is, is that physically connected,
    // i.e. does it have a transObj. Responses may be connected or passive
    return comObj->getStateStatistics();
  }
  else if(isPerm()) {
    return oz_atom("perm");
  }
  else if(this==myDSite) {
    return oz_atom("mine");
  }
  else 
    return oz_atom("passive");
}

char *DSite::stringrep()
{
  static char buf[100];
  ip_address a=getAddress();
  sprintf(buf,"type:%d %d.%d.%d.%d:%d:%ld/%d",
	  getType(),
	  (a/(256*256*256))%256,
	  (a/(256*256))%256,
	  (a/256)%256,
	  a%256,
	  getPort(), getTimeStamp()->start,getTimeStamp()->pid);
  return buf;
}

char *DSite::stringrep_notype()
{
  static char buf[100];
  ip_address a=getAddress();
  sprintf(buf,"%d.%d.%d.%d:%d:%ld/%d",
	  (a/(256*256*256))%256,
	  (a/(256*256))%256,
	  (a/256)%256,
	  a%256,
	  getPort(), getTimeStamp()->start,getTimeStamp()->pid);
  return buf;
}

char *oz_site2String(DSite *s) { return s->stringrep(); }

//

/**********************************************************************/
/*   SECTION :: Site object methods                                   */
/**********************************************************************/


void DSite::marshalDSite(MarshalerBuffer *buf)
{
  PD((MARSHAL,"Site"));
  unsigned int type = getType();
  if (type & PERM_SITE) {
    marshalDIF(buf, DIF_SITE_PERM);
    marshalBaseSite(buf);
  } else {
    Assert((type & REMOTE_SITE) || (this==myDSite) );
    marshalDIF(buf, DIF_SITE);
    marshalBaseSite(buf);
  }
}


/**********************************************************************/
/*   SECTION :: memory management  methods                            */
/**********************************************************************/

//
DSite* myDSite =  NULL;

//
// kost@ : that's a part of the boot-up procedure ('perdioInit()');
// Actually, it is used by 'initNetwork()' because ip, port, timestamp
// are not known prior its initialization;
DSite* makeMyDSite(ip_address a, oz_port_t p, TimeStamp &t) {
  DSite *s = new DSite(a,p,t);
  s->setMyDSite();
  primarySiteTable->insert(s);
  s->initMyDSite();
  return s;
} 

OZ_Term DSite::getOzRep(){
  int sent = 0, received = 0, lastrtt = -1;
  if(remoteComm() && isConnected())
    {
      ComObj *c= getComObj();
      received = getNORM_ComObj(c);
      sent     = getNOSM_ComObj(c);
      lastrtt  = getLastRTT_ComObj(c);
    }
  TimeStamp *ts = getTimeStamp();
  ip_address a=getAddress();
  char ip[100];
  sprintf(ip,"%d.%d.%d.%d",
	  (a/(256*256*256))%256,
	  (a/(256*256))%256,
	  (a/256)%256,
	  a%256);
  return OZ_recordInit(oz_atom("site"),
      oz_cons(oz_pairA("siteid", oz_atom(stringrep_notype())),
      oz_cons(oz_pairAI("port",(int)getPort()),
      oz_cons(oz_pairAI("timestamp",(int)ts->start),
      oz_cons(oz_pairAI("addr",a),
      oz_cons(oz_pairAA("ip",ip),
      oz_cons(oz_pairAI("sent",sent),
      oz_cons(oz_pairAI("received",received),
      oz_cons(oz_pairAI("lastRTT",lastrtt),
      oz_cons(oz_pairAI("pid",ts->pid),
      oz_cons(oz_pairA("state",getStateStatistics()),
	      oz_nil())))))))))));
}
