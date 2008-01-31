/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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

#include "glue_siteRepresentation.hh"
#include "glue_base.hh"
#include "glue_ozSite.hh"
#include "glue_interface.hh"
#include "pstContainer.hh"
#include <netinet/in.h>

Glue_SiteRep* site_address_representations=NULL; 
Glue_SiteRep* thisGSite = NULL;

int RTT_UPPERBOUND = 30000;     // higher bound for rtt monitor

// UTILS 


void putStr(DssWriteBuffer *buf, char *str, int len)
{
  for (int i = 0; i < len; i++) 
    buf->putByte(*(str+i));
}
void getStr(DssReadBuffer *buf, char *str, int len){
  for (int i = 0; i < len; i++)
    *(str+i) = buf->getByte(); 
}
void cleanStr(DssReadBuffer *buf, int len){
  for (int i = 0; i < len; i++)
    buf->getByte(); 
}

// GLUE_SITE_ADDRESS methods


Glue_SiteRep::Glue_SiteRep(int ip, int port, int id, DSite *name, OZ_Term ozSite):
  a_ipAddress(ip),
  a_portNum(port),
  a_idNum(id),
  a_dssSite(name),
  a_next(site_address_representations),
  a_ozSite(ozSite)
{
  site_address_representations = this;
}

void Glue_SiteRep::m_gc(){
  if (a_ozSite) oz_gCollectTerm(a_ozSite, a_ozSite);
}

void
Glue_SiteRep::m_setConnection(DssChannel* vc) {
  // give the connection to the DSite, and monitor the connection
  a_dssSite->m_connectionEstablished(vc);
}

OZ_Term Glue_SiteRep::m_getInfo() {
  char ip[16];
  //a_ipAddress is in network byte order!
  unsigned int ip_addr=ntohl(a_ipAddress);
  sprintf(ip,"%d.%d.%d.%d",
	  (ip_addr/(256*256*256))%256,
	  (ip_addr/(256*256))%256,
	  (ip_addr/256)%256,
	  ip_addr%256);
  return OZ_recordInit(oz_atom("site"),
		       oz_cons(oz_pairAA("ip", ip),
		       oz_cons(oz_pairAI("port", a_portNum),
		       oz_cons(oz_pairAI("id", a_idNum),
		       oz_nil()))));
}

void    
Glue_SiteRep::marshalCsSite( DssWriteBuffer* const buf){
  putInt(buf, a_ipAddress); 
  putInt(buf, a_portNum); 
  putInt(buf, a_idNum);
}



void    
Glue_SiteRep::updateCsSite( DssReadBuffer* const buf){
  // here we can check that the address hasn't changed since we
  // last heard of the site
  (void) getInt(buf); 
  (void) getInt(buf);
  (void) getInt(buf);
}
void    
Glue_SiteRep::disposeCsSite(){
  printf("we are deleted\n"); 
}

void    
Glue_SiteRep::monitor() {
  // We monitor all our connections
  a_dssSite->m_installRTmonitor(0, RTT_UPPERBOUND);
}

void    
Glue_SiteRep::reportRtViolation(int rtt, int low, int high) {
  if (a_dssSite->m_getFaultState() <= DSite_TMP) {
    if (rtt == high) {
      // This means that no message was received recently
      a_dssSite->m_stateChange(DSite_TMP);
      a_dssSite->m_installRTmonitor(RTT_UPPERBOUND, RTT_UPPERBOUND);
    } else {
      // A message has been received, so the site is ok again
      a_dssSite->m_stateChange(DSite_OK);
      a_dssSite->m_installRTmonitor(0, RTT_UPPERBOUND);
    }
  }
}

DssChannel *    
Glue_SiteRep::establishConnection(){
  OZ_Term command;
  command = OZ_recordInit(oz_atom("connect"),
			  oz_cons(oz_pair2(oz_int(1),this->m_getOzSite()),
				  oz_nil()));
  doPortSend(tagged2Port(g_connectPort), command, NULL);  
  // return NULL to indicate that the operation is assynchronous
  return NULL; 
}


void     
Glue_SiteRep::closeConnection(DssChannel* con){}




void gCollectGASreps(){
  Glue_SiteRep** ptr = &site_address_representations; 
  while((*ptr) != NULL)
    {
      Glue_SiteRep *tmp = *ptr; 
      // raph: we must keep the representation of this site!
      if (tmp != thisGSite && tmp->m_getOzSite() == (OZ_Term) 0)
	{
	  *ptr = tmp->m_getNext(); 
	  delete tmp; 
	}
      else
	{
	  tmp->m_gc();
	  ptr = tmp->m_getNextPP(); 
	}
    }
}
