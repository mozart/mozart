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


Glue_SiteRep::Glue_SiteRep(int ip, int port, DSite *name, OZ_Term ozSite):
  a_ipAddress(ip),
  a_portNum(port),
  a_dssSite(name),
  a_next(site_address_representations),
  a_conThreadVar((OZ_Term) 0),
  a_ozSite(ozSite)
{
  site_address_representations = this;
  a_RId = NULL;
}



char* Glue_SiteRep::m_stringrep(){
  static char buf[200]; 
  int a = a_ipAddress; 
  /*sprintf(buf,"ip: %d.%d.%d.%d port: %d rid: %s",
	  255+(a/(256*256*256))%256,
	  255+(a/(256*256))%256,
	  255+(a/256)%256,
	  256+a%256,  a_portNum, a_RId);
  */
  sprintf(buf," %s", a_RId); 
  return buf;
}


void Glue_SiteRep::m_storeConnectionThread( OZ_Term v)
{
  // Currently, we can only have one thread per site-connection. 
  Assert(a_conThreadVar == (OZ_Term) 0); 
  a_conThreadVar = v; 
}


Glue_SiteRep*  Glue_SiteRep::m_getNext(){
  return a_next; 
}

void Glue_SiteRep::m_gc(){
  if (a_ozSite) oz_gCollectTerm(a_ozSite, a_ozSite);
  if (a_conThreadVar) oz_gCollectTerm(a_conThreadVar, a_conThreadVar); 
}

DSite* Glue_SiteRep::m_getDssSite(){ return  a_dssSite;}
void Glue_SiteRep::m_setDssSite(DSite* sa){  a_dssSite = sa;}

void
Glue_SiteRep::m_setConnection(VirtualChannelInterface* vc) {
  // give the connection to the DSite, and monitor the connection
  a_dssSite->m_connectionEstablished(vc);
}

OZ_Term Glue_SiteRep::m_getOzSite(){ return a_ozSite;}

OZ_Term Glue_SiteRep::m_getInfo() {
  char ip[15];
  sprintf(ip,"%d.%d.%d.%d",
	  (a_ipAddress/(256*256*256))%256,
	  (a_ipAddress/(256*256))%256,
	  (a_ipAddress/256)%256,
	  a_ipAddress%256);
  return OZ_recordInit(oz_atom("siteInfo"),
		       oz_cons(oz_pairAA("ip", ip),
		       oz_cons(oz_pairAI("port", a_portNum),
		       oz_cons(oz_pairAA("siteId", a_RId),
		       oz_nil()))));
}


// A copy of an allready existing site is received. The SiteAddress 
// information should be read out of the buffer and discared. 
void
Glue_SiteRep::m_updateAddress(DssReadBuffer* const buf){
  // here we can check that the address hasn't changed since we
  // last heard of the site
  (void) getInt(buf); 
  (void) getInt(buf);
  (void) cleanStr(buf, getInt(buf));
}


void Glue_SiteRep::m_setRId(char* RId) { 
  if (a_RId != NULL)
    free(a_RId);

  a_RId = RId; 
}



void    
Glue_SiteRep::marshalCsSite( DssWriteBuffer* const buf){
  putInt(buf, a_ipAddress); 
  putInt(buf, a_portNum); 
  
  int RIdLen = strlen(a_RId);
  putInt(buf, RIdLen);
  putStr(buf, a_RId, RIdLen);
}



void    
Glue_SiteRep::updateCsSite( DssReadBuffer* const buf){
  // here we can check that the address hasn't changed since we
  // last heard of the site
  (void) getInt(buf); 
  (void) getInt(buf);
  (void) cleanStr(buf, getInt(buf));
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

VirtualChannelInterface *    
Glue_SiteRep::establishConnection(){
  OZ_Term Requestor,LocalOzState,addrinfo,DistOzState,command;

  Requestor=OZ_recordInit(oz_atom("requestor"),
				  // old   oz_cons(oz_pairAA("id", site->stringrep_notype()),
				  oz_cons(oz_pairAA("id", "shit"),
					  oz_cons(oz_pairAI("req",(int) this), 
						  oz_nil())));
  LocalOzState=OZ_recordInit(oz_atom("localstate"),
			     oz_cons(oz_pairA("connectionFunctor",
					      g_defaultConnectionProcedure),
				     oz_cons(oz_pairA("localState",
						      oz_nil()),
					     oz_nil())));
  addrinfo=OZ_recordInit(oz_atom("ip_addr"),
			 oz_cons(oz_pairAI("addr",ntohl(this->getIpNum())),
			 oz_cons(oz_pairAI("port",this->getPortNum()),
						  oz_nil())));
  DistOzState=OZ_recordInit(oz_atom("diststate"),
				    oz_cons(oz_pairAA("type","ordinary"),
					    oz_cons(oz_pairA("parameter",
							     addrinfo),
						    oz_nil())));
  command = OZ_recordInit(oz_atom("connect"),
				  oz_cons(oz_pair2(oz_int(1),Requestor),
				  oz_cons(oz_pair2(oz_int(2),LocalOzState),
				  oz_cons(oz_pair2(oz_int(3),DistOzState),
					  oz_cons(oz_pair2(oz_int(4),this->m_getOzSite()),
						  oz_nil()))))); 
  doPortSend(tagged2Port(g_connectPort), command, NULL);  
  // return NULL to indicate that the operation is assynchronous
  return NULL; 
}


void     
Glue_SiteRep::closeConnection( VirtualChannelInterface* con){
  ;
}




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
