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

#include "base.hh"
#include "pickle.hh"
#include "glue_site.hh"
#include "glue_base.hh"
#include "glue_buffer.hh"
#include "glue_interface.hh"
#include "pstContainer.hh"
#include <netinet/in.h>

Glue_SiteRep* site_address_representations=NULL; 
Glue_SiteRep* thisGSite = NULL;

int RTT_INIT        =  5000;     // initial timeout
int RTT_UPPERBOUND  = 30000;     // higher bound for rtt monitor

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
  a_ozSite(ozSite),
  rtt_avg(0),
  rtt_mdev(0),
  rtt_timeout(RTT_INIT)
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
Glue_SiteRep::working() {
  rtt_avg = 0;                              // reset rtt parameters
  rtt_timeout = RTT_INIT;
  a_dssSite->m_monitorRTT(rtt_timeout);
}

void
Glue_SiteRep::reportRTT(int rtt) {
  switch (a_dssSite->m_getFaultState()) {
  case DSite_TMP:
    a_dssSite->m_stateChange(DSite_OK);
    // fall through
  case DSite_OK:
    if (rtt > RTT_UPPERBOUND) rtt = RTT_UPPERBOUND;
    if (rtt_avg) {
      int err = rtt - rtt_avg;
      rtt_avg += err / 2;
      rtt_mdev += (abs(err) - rtt_mdev) / 4;
    } else {
      rtt_avg = rtt;
      rtt_mdev = rtt;
    }
    rtt_timeout = rtt_avg + rtt_mdev;
    a_dssSite->m_monitorRTT(rtt_timeout);
    // fall through
  default:
    break;     // (stop monitoring when permfailed)
  }
}

void
Glue_SiteRep::reportTimeout(int timeout) {
  switch (a_dssSite->m_getFaultState()) {
  case DSite_OK:  a_dssSite->m_stateChange(DSite_TMP);
  case DSite_TMP: a_dssSite->m_monitorRTT(rtt_timeout);
  default: break;     // (stop monitoring when permfailed)
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



/****************************** Oz_Site ******************************/

int Oz_Site::getIdV(void) {
  return OZ_E_SITE;
}

OZ_Term Oz_Site::typeV(void) {
  return OZ_atom("site");
}

OZ_Return Oz_Site::eqV(OZ_Term t) {
  if (OZ_isExtension(t)) {
    OZ_Extension *e = OZ_getExtension(t);
    if (e->getIdV() == OZ_E_SITE) {
      Oz_Site *w = static_cast<Oz_Site *>(e);
      if (w->a_gSite == a_gSite)
	return PROCEED;
    }
  }
  return FAILED;
}

OZ_Term Oz_Site::printV(int depth) {
  return OZ_atom("<site>");
}

OZ_Term Oz_Site::printLongV(int depth, int offset) {
  return printV(depth); 
}

OZ_Extension *Oz_Site::gCollectV(void) {
  return new Oz_Site(*this);
}

OZ_Extension *Oz_Site::sCloneV(void) {
   printf("Should not clone address sites DSite!!\n"); 
   Assert(0); 
   return new Oz_Site(*this);
}

void Oz_Site::gCollectRecurseV(void) 
{
}

void Oz_Site::sCloneRecurseV(void) {}

void  Oz_Site::setGSR(Glue_SiteRep* gsa){
  a_gSite = gsa;
}

OZ_Boolean toBePickledV() { return (OZ_TRUE); }
void pickleV(MarshalerBuffer *bs, GenTraverser *gt) {
  // This should only be called when we explicit put a site reference into 
  // a pickle save. 
  
  // call the DSite and ask for serialization... 
  
}


// The idea goes as follows. The Oz-object calls the DSS object that will 
// create a serialized representation. The oz-object does thus not serialize
// a thing. It is completly done by the DSS. 
void Oz_Site::marshalSuspV(OZ_Term te, ByteBuffer *bs, GenTraverser *gt) {
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs);
  (a_gSite->m_getDssSite())->m_marshalDSite(gwb);
}


static
OZ_Term unmarshalOzSite(MarshalerBuffer *bs, Builder*)
{
  Assert(0); 
  return (UnmarshalEXT_Error);
}

static
OZ_Term suspUnmarshalOzSite(ByteBuffer *mb, Builder*,
			    GTAbstractEntity* &bae)
{
  GlueReadBuffer *grb = static_cast<GlueReadBuffer *>(mb); 
  DSite *ds = glue_com_connection->a_msgnLayer->m_UnmarshalDSite(grb); 
  Glue_SiteRep *gsa = static_cast< Glue_SiteRep *>(ds->m_getCsSiteRep());
  return gsa->m_getOzSite();
}

static
OZ_Term unmarshalOzSiteCont(ByteBuffer *mb, Builder*,
			    GTAbstractEntity* bae)
{
  Assert(0);
  return (UnmarshalEXT_Error);
}

void OzSite_init()
{
  static Bool done = NO;
  if (done == NO) {
    done = OK;
    oz_registerExtension(OZ_E_SITE, unmarshalOzSite,
			 suspUnmarshalOzSite, unmarshalOzSiteCont);
  }
}

