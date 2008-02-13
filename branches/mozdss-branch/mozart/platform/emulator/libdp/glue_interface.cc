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

//#if defined(INTERFACE)
//#pragma implementation "glue_interface.hh"
//#endif


#include "engine_interface.hh"
#include "dss_object.hh"
#include "glue_interface.hh"
#include "glue_site.hh"
#include "os.hh"
#include "glue_base.hh"
#include "pstContainer.hh"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
void doPortSend(OzPort *port, TaggedRef val, Board*);
OZ_Return accessCell(OZ_Term cell, OZ_Term &out);

/* From DSS */
MAP::MAP() :
  Mediation_Object() {}

 OZ_Term g_connectPort;
 OZ_Term g_kbrStreamPort;
 OZ_Term g_defaultAcceptProcedure;
 OZ_Term g_faultPort;


PstInContainerInterface* 
MAP::createPstInContainer(){
  return new PstInContainer();
}

void
MAP::GL_error(const char *format, ...){
  va_list ap;
  va_start(ap,format);
  vfprintf(stderr,format,ap);
  fprintf(stderr, "\n -- Error in pid %d --", osgetpid());
  bool loop=true;
  while(loop){;}
  //OZ_error("DSS_warning %s", format,ap);
  va_end(ap);
}

void
MAP::GL_warning(const char *format, ...){
  va_list ap;
  va_start(ap,format);
  vfprintf(stderr,format,ap);
  fprintf(stderr, "\n -- Warning in pid %d --", osgetpid());
  bool loop=true;
  while(loop){;}
  //OZ_warning("DSS_warning %s", format,ap);
  va_end(ap);
}
void 
MAP::kbr_message(int key, PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  printf("Receiving key:%d, msg %s\n", key, toC(pst->a_term)); 
  OzPort *pws = tagged2Port(g_kbrStreamPort);
  OZ_Term msg = OZ_recordInit(oz_atom("message"),
                              oz_cons(oz_pairA("msg", pst->a_term), 
			              oz_cons(oz_pairAI("key", key),
				              oz_nil())));
  doPortSend(pws, msg, NULL);
}


void 
MAP::kbr_divideResp(int start, int stop, int n){
  printf("Divide interval ]%d %d ]\n", start, stop);
  OzPort *pws = tagged2Port(g_kbrStreamPort);
  OZ_Term msg = OZ_recordInit(oz_atom("divide"),
		  	      oz_cons(oz_pairAI("begin",start), 
			 	      oz_cons(oz_pairAI("end", stop), 
					      oz_cons(oz_pairAI("n", n),
					              oz_nil()))));
  doPortSend(pws, msg, NULL); 
}

void 
MAP::kbr_newResp(int start, int stop, int n, PstInContainerInterface* pstin){
  printf("Divide interval ]%d %d ]\n", start, stop);
  OzPort *pws = tagged2Port(g_kbrStreamPort);
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  OZ_Term msg = OZ_recordInit(oz_atom("newResp"),
  			      oz_cons(oz_pairAI("begin",start),
			              oz_cons(oz_pairAI("end", stop),
				              oz_cons(oz_pairAI("n", n),
					              oz_cons(oz_pairA("data", pst->a_term),
							      oz_nil())))));
  doPortSend(pws, msg, NULL); 
}




ComService::ComService(int ip, int port, int id) {
  thisGSite = new GlueSite(NULL, ip, port, id);
} 


ComService::~ComService(){;}

// Create a SiteAddress object from the representation found in 
// the readbuffer. Connect the new object to the passed DssSite
// object. 
CsSiteInterface*
ComService::unmarshalCsSite(DSite* name, DssReadBuffer* const buf) {
  int ip   = getInt(buf);
  int port = getInt(buf);
  int id   = getInt(buf);
  GlueSite* gs = new GlueSite(name, ip, port, id);

  OZ_Term command = OZ_recordInit(oz_atom("new_site"),
				  oz_cons(oz_pair2(oz_int(1), gs->getOzSite()),
					  oz_nil()));
  doPortSend(tagged2Port(g_connectPort), command, NULL); 
  return gs;
}

    
CsSiteInterface *ComService::connectSelfReps(MsgnLayer *msg, DSite* ds){ 
  a_msgnLayer = msg; 
  thisGSite->setDSite(ds);
  return thisGSite;
}

// mark all DSites used by the CSC
void ComService::m_gcSweep() {
  // raph: where are those DSites?
}

// Explicit site handeling
void ComService::m_MsgReceived(CsSiteInterface* CS, MsgContainer* msg){
  PstInContainer *pst = NULL; 
  GlueSite *gs = static_cast<GlueSite*>(CS);

  OZ_Term command =
    OZ_recordInit(oz_atom("directMsg"),
		  oz_cons(oz_pair2(oz_atom("msg"), pst->a_term),
			  oz_cons(oz_pair2(oz_atom("srcSite"), gs->getOzSite()),
				  oz_nil())));
  doPortSend(tagged2Port(g_connectPort), command, NULL); 
}

ExtDataContainerInterface* 
ComService::m_createExtDataContainer(BYTE){
  printf("we're sending NOTHING, thus we should se no extdata containers\n"); 
  return NULL;
}
