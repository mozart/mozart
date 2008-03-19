/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 * 
 *  Contributors:
 *    Erik Klintskog (erik@sics.se)
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Boriss Mejias (bmc@info.ucl.ac.be)
 * 
 *  Copyright:
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
#include "value.hh"
#include "pickleBase.hh" // for the crc rotines
#include "builtins.hh"
#include "os.hh"
#include "dpMarshaler.hh"
#include "dss_object.hh"
#include "glue_interface.hh"
#include "glue_tables.hh"
#include "glue_utils.hh"
#include "engine_interface.hh"
#include "glue_site.hh"
#include "glue_buffer.hh"
#include "glue_base.hh"
#include "glue_mediators.hh"
#include "glue_faults.hh"
#include "glue_marshal.hh"
#include "pstContainer.hh"

#include "glue_ioFactory.hh"

#include "wsock.hh"
#ifndef WINDOWS
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#endif
// handles IRIX6 (6.5 at least), IRIX64, HPUX_700 (9.05 at least),
// Solaris does not try to be BSD4.3-compatible by default (but one
// can say '-DBSD_COMP' and then that stuff is included as well)
#if defined(SOLARIS_SPARC)
#include    <sys/sockio.h>
#else
#include    <sys/ioctl.h>
#endif

/****************************** Utils ******************************/

#define DeclareSiteListIN(ARG,VAR,LEN)					\
OZ_Term VAR = OZ_in(ARG);						\
int LEN = 0;								\
{ OZ_Term arg = VAR;							\
  while (OZ_isCons(arg)) {						\
    TaggedRef a = oz_safeDeref(OZ_head(arg));				\
    if (OZ_isVariable(a)) OZ_suspendOn(a);				\
    if (!oz_isOzSite(a)) return OZ_typeError(ARG, "list(site)");	\
    arg = OZ_tail(arg);							\
    ++LEN;								\
  }									\
  if (OZ_isVariable(arg)) OZ_suspendOn(arg);				\
  if (!OZ_isNil(arg)) return OZ_typeError(ARG,"list(site)");		\
}

inline 
int dssIsCons(OZ_Term list, OZ_Term *hd, OZ_Term *tl) { 
  if (!OZ_isCons(list)) {
    return 0;
  }
  *hd = OZ_head(list);
  *tl = OZ_tail(list);
  return 1;
}

OZ_Return getRecordField(OZ_Term record, char* field, int &ans){
  if(OZ_isRecord(record)){
    SRecord *srec = tagged2SRecord(record);
    int index = srec->getIndex(oz_atom(field));
    if (index>=0)
      {
	OZ_Term t0 = srec->getArg(index);
	NONVAR(t0,t);
	if(OZ_isInt(t))
	  {
	    ans = oz_intToC(t);
	    return OZ_ENTAILED;
	  }
      }
  }
  return OZ_FAILED;
}


/**********************************************************************
                     Handling of site connections
 **********************************************************************/

/* Handover a route(0) to a remote site(1). */
OZ_BI_define(BIhandoverRoute,2,0) {
  DeclareSiteListIN(0, slist, len);
  oz_declareNonvarIN(1, peer);

  if (!oz_isOzSite(peer)) return OZ_typeError(1, "site");
  // arguments have been checked, now proceed
  
  // check size of list (upper limit is an implementation limit)
  if (len < 2) 
    return oz_raise(E_SYSTEM, AtomDp, "route too small", 0);

  if (len > 6) 
    return oz_raise(E_SYSTEM, AtomDp, "route too long", 0);

  DSite** route = new DSite* [len];
  OZ_Term head, tail;
  int count = len;
  while (dssIsCons(slist, &head, &tail)) {
    Assert(count > 0);
    route[--count] = ozSite2DSite(head);
    slist = tail;
  }
  Assert(count == 0);

  ozSite2DSite(peer)->m_virtualCircuitEstablished(len, route); 
  return PROCEED;

}OZ_BI_end


// create a connection for the given site, with the given file descriptor
OZ_BI_define(BIsetConnection,2,0){
  oz_declareNonvarIN(0,site);
  OZ_declareInt(1,fd);
  if (!oz_isOzSite(site)) return OZ_typeError(0, "site");

  DssChannel* channel = new SocketChannel(fd);
  ozSite2GlueSite(site)->m_setConnection(channel);

  // notify the DP port
  OZ_Term ack = OZ_recordInit(oz_atom("connection_received"),
			      oz_cons(oz_pair2(oz_int(1),site),
				      oz_cons(oz_pair2(oz_int(2),OZ_int(fd)),
					      oz_nil())));
  doPortSend(tagged2Port(g_connectPort),ack,NULL);
  return PROCEED;

}OZ_BI_end


// hand over an anonymous connection to the DSS (arg is a file descriptor)
OZ_BI_define(BIacceptConnection,1,0){
  OZ_declareInt(0,fd);
  DssChannel* channel = new SocketChannel(fd);
  glue_com_connection->a_msgnLayer->m_anonymousChannelEstablished(channel); 
  return PROCEED;
}OZ_BI_end


// Not connection failed stupid, but 
// statechange

OZ_BI_define(BIconnFailed,2,0) {
  oz_declareNonvarIN(0, site);
  oz_declareNonvarIN(1, reason);
  if (!oz_isOzSite(site)) return OZ_typeError(0, "site");
  
  if (oz_eq(reason, oz_atom("perm"))) {
    ozSite2DSite(site)->m_stateChange(DSite_GLOBAL_PRM);
  } 
  else if (oz_eq(reason, oz_atom("temp"))) {
    // This could be reported to the comObj, but the comObj also
    // has its own timer to discover this.
    ;
  }
  else {
    // AN: For now do as for temp. Could go ahead and inform comObj.
    ;
  }
  return PROCEED;
}OZ_BI_end


// change the fault state of a site
OZ_BI_define(BIsetSiteState, 2, 0) {
  oz_declareNonvarIN(0, site);
  oz_declareNonvarIN(1, state);
  if (!oz_isOzSite(site)) return OZ_typeError(0, "site");

  // first parse new state
  GlueFaultState fs;
  if (!atomToFS(state, fs))
    return oz_raise(E_SYSTEM, AtomDp, "invalid fault state", 1, state);

  // translate GlueFaultState to DSiteState (HACK!)
  static DSiteState dfs[] =
    { DSite_OK, DSite_TMP, DSite_LOCAL_PRM, DSite_GLOBAL_PRM };
  // set new state
  ozSite2DSite(site)->m_stateChange(dfs[fs]);
  return PROCEED;

} OZ_BI_end



OZ_BI_define(BImigrateManager,1,0){
  OZ_declareTerm(0,entity);
  // Check that the argument actually is 
  // distributed and fetch the ProxyName
  ConstTermWithHome *ct = static_cast<ConstTermWithHome*>(tagged2Const(entity));
  if ((oz_isPort(entity) || oz_isCell(entity)) && ct->isDistributed()) {
    CoordinatorAssistant *pi = ct->getMediator()->getCoordinatorAssistant();
    //ZACHARIAS: argument 2 is unecessary so pass any void*
    pi->manipulateCNET(NULL); 
  }
  if(oz_isArray(entity)) {
    if(ct->isDistributed()){
      CoordinatorAssistant *pi = ct->getMediator()->getCoordinatorAssistant();
      pi->manipulateCNET(NULL); 
    }
  }
  return PROCEED; 
}OZ_BI_end


 // initialization of DP module
OZ_BI_define(BIinitDP, 1, 0) {
  oz_declareNonvarIN(0, port);
  if (!oz_isPort(port)) { oz_typeError(0, "port"); }

  g_connectPort = port;
  OZ_protect(&g_connectPort);

  initDP();
  return PROCEED;
} OZ_BI_end


// For setting up a ticket

OZ_BI_define(BIgetCRC,1,1) 
{
  oz_declareVirtualStringIN(0,s);

  crc_t crc = update_crc(init_crc(),(unsigned char *) s, strlen(s));
    
  OZ_RETURN(OZ_unsignedInt(crc));
} OZ_BI_end

#define PORT_TO_TICK_BUF_LEN 400
unsigned char portToTickBuf[PORT_TO_TICK_BUF_LEN];

OZ_BI_define(BIportToMS,1,1)
{
  oz_declareNonvarIN(0,prt);
  if (!oz_isPort(prt)) { oz_typeError(0,"Port"); }

  // globalize the port, and get its mediator
  glue_globalizeEntity(prt);
  Mediator *med = glue_getMediator(prt);

  // marshal the Dss abstract entity, and entity-specific stuff
  GlueWriteBuffer buf(portToTickBuf, PORT_TO_TICK_BUF_LEN);
  med->getCoordinatorAssistant()->marshal(&buf, PMF_FREE);
  med->marshalData(&buf);

  // turn it into a string
  int len = buf.bufferUsed();
  char *str = encodeB64((char*) portToTickBuf, len);
  OZ_RETURN(OZ_string(str));
}OZ_BI_end


OZ_BI_define(BImsToPort,1,1)
{
  oz_declareProperStringIN(0,str);
  int len = strlen(str); 
  unsigned char* raw_buf = (unsigned char*) decodeB64((char*)str, len);

  // unmarshal the Dss abstract entity, and entity-specific stuff  
  GlueReadBuffer buf(raw_buf, len);
  AbstractEntityName aen;
  bool trail;
  CoordinatorAssistant* proxy = dss->unmarshalProxy(&buf, PUF_FREE, aen, trail);
  Assert(!trail);

  // build mediator and entity if not present
  Mediator* med = dynamic_cast<Mediator*>(proxy->getAbstractEntity());
  if (!med) { // create mediator
    med = glue_newMediator(GLUE_PORT);
    med->setProxy(proxy);
  }
  med->unmarshalData(&buf);
  free(raw_buf);

  OZ_RETURN(med->getEntity());

}OZ_BI_end


/**********************************************************************/
/*   Misc Builtins                                                    */
/**********************************************************************/


extern
int raiseUnixError(char *f,int n, char * e, char * g);

//
#define WRAPCALL(f, CALL, RET)				\
int RET;						\
while ((RET = CALL) < 0) {				\
  if (ossockerrno() != EINTR) { RETURN_UNIX_ERROR(f); }	\
}
//
#define RETURN_UNIX_ERROR(f) \
{ return raiseUnixError(f,ossockerrno(), OZ_unixError(ossockerrno()), "dpMisc"); }

//

//
// Return the list of broadcast addresses available (may be empty if
// none were found)
OZ_BI_define(BIgetBroadcastAddresses,0,1)
{
  int bsize, nba, i;
  char *buff;
  OZ_Term lba = oz_nil();	// list of broadcast addresses;

#if defined(SIOCGIFCONF) && defined(SIOCGIFFLAGS) && defined(SIOCGIFBRDADDR)
  struct ifconf ifc;
  struct ifreq *ifrp;

  WRAPCALL("socket", ossocket(PF_INET, SOCK_DGRAM, 0), desc);

  // how many interfaces can be there??! :-))
  bsize = sizeof(struct ifreq) * 16;
  buff = (char *) malloc(bsize);
  if (buff==NULL) {
    close(desc);
    RETURN_UNIX_ERROR("virtual memory exhausted!");
  }

  //
  ifc.ifc_len = bsize;
  ifc.ifc_buf = buff;
  if (ioctl(desc, SIOCGIFCONF, &ifc) < 0) {
    free(buff);
    close(desc);
    RETURN_UNIX_ERROR("SIOCGIFCONF failed!");
  }

  //
  ifrp = ifc.ifc_req;
  nba = ifc.ifc_len / sizeof(struct ifreq);

  //
  for (i = 0; i < nba; i++, ifrp++) {
    struct ifreq ifr;
    // take only 'UP' and 'BROADCAST' interfaces:
    strcpy(ifr.ifr_name, ifrp->ifr_name);
    if (ioctl(desc, SIOCGIFFLAGS, &ifr) < 0 ||
	!(ifr.ifr_flags & IFF_UP) ||
	!(ifr.ifr_flags & IFF_BROADCAST))
      continue;

    //
    strcpy(ifr.ifr_name, ifrp->ifr_name);
    // take only inet addresses, if any:
    if (// strncmp(ifr.ifr_name, "lo", 2) == 0 ||
	ioctl(desc, SIOCGIFBRDADDR, &ifr) < 0 ||
	ifr.ifr_broadaddr.sa_family != AF_INET) {
      // none: let it go;
      continue;
    } else {
      lba = oz_cons(OZ_string(inet_ntoa(((struct sockaddr_in *) &ifr.ifr_broadaddr)->sin_addr)), lba);
    }
  }

  //
  close(desc);
  free(buff);
#else
  // last resort - just give '-1' away;
  struct in_addr ia;
#if defined(WINDOWS)
  ia.s_addr = (u_long) -1;
#else
  ia.s_addr = (in_addr_t) -1;
#endif
  lba = oz_cons(OZ_string(inet_ntoa(ia)), lba);
#endif

  //
  OZ_RETURN(lba);
} OZ_BI_end

//
// Allow the broadcast messages for a socket;
OZ_BI_define(BIsockoptBroadcast,1,0)
{
#ifdef SO_BROADCAST
  OZ_declareInt(0, desc);
  int on = 1;
  //
  if (
#ifdef HAVE_SOCKLEN_T
      setsockopt(desc, SOL_SOCKET, SO_BROADCAST,
		 (void *) &on, (socklen_t) sizeof(on))
#else
      setsockopt(desc, SOL_SOCKET, SO_BROADCAST,
		 (const char *) &on, (int) sizeof(on))
#endif
      < 0)
    RETURN_UNIX_ERROR("setsockopt failed!");
#endif
  return PROCEED;
} OZ_BI_end




// Returns the total number of received and sent messages.
// Intresting for debuging/tuning distributed applications.

OZ_BI_define(BIgetMsgCntr,0,1)
{
  int sent, received, oswritten, osread, cont; 
  dss->operateIntParam(DSS_STATIC, DSS_STATIC_GET_COMINFO, 0, sent);
  dss->operateIntParam(DSS_STATIC, DSS_STATIC_GET_COMINFO, 1, received);
  dss->operateIntParam(DSS_STATIC, DSS_STATIC_GET_COMINFO, 2, oswritten);
  dss->operateIntParam(DSS_STATIC, DSS_STATIC_GET_COMINFO, 3, osread);
  dss->operateIntParam(DSS_STATIC, DSS_STATIC_GET_COMINFO, 4, cont);
  
  
  OZ_RETURN(OZ_recordInit(oz_atom("msgStatistics"),
			  oz_cons(oz_pairAI("sent",sent),
			  oz_cons(oz_pairAI("received",received),
			  oz_cons(oz_pairAI("oswritten",oswritten),
			  oz_cons(oz_pairAI("osread",osread),
			  oz_cons(oz_pairAI("cont",cont),
				  oz_nil())))))));
}OZ_BI_end   


OZ_BI_define(BIprintDPTables,0,0)
{
  mediatorTable->print();
  int arg = 0; 
  dss->operateIntParam(DSS_STATIC,DSS_STATIC_DEBUG_TABLES,0,arg);
  return OZ_ENTAILED;
}OZ_BI_end  

OZ_BI_define(BIsetDssLogLevel,1,0)
{
  OZ_declareInt(0, Level);
  dss->operateIntParam(DSS_STATIC, DSS_STATIC_LOG_PARAMETER, 0, Level);
  return OZ_ENTAILED;

}OZ_BI_end  

OZ_BI_define(BIprintDssMemoryAllocation,0,0)
{
  int arg = 0; 
  dss->operateIntParam(DSS_STATIC, DSS_STATIC_MEMORY_ALLOCATION, 0, arg);
  return OZ_ENTAILED;
}OZ_BI_end  

OZ_BI_define(BIcreateLogFile,1,0)
{
  return OZ_FAILED;
}OZ_BI_end



/************************* access RPC wrapper *************************/

OZ_BI_define(BIsetRPC,1,0) {
  oz_declareNonvarIN(0, proc);
  if (!oz_isProcedure(proc) || oz_procedureArity(proc) != 3) {
    oz_typeError(0, "procedure/3");
  }
  return oz_unify(getRPC(), proc);
} OZ_BI_end



/******************** Annotations and faults ********************/

OZ_BI_define(BIsetAnnotation,4,0)
{
  // raph: For the sake of simplicity, the list of annotations is
  // parsed by DPControl.annotate.  The latter calls this builtin with
  // three integers (pn, aa, rc).  The builtin only checks the
  // consistency of the annotation for the given entity.
  oz_declareSafeDerefIN(0,entity);
  oz_declareIntIN(1,pn);
  oz_declareIntIN(2,aa);
  oz_declareIntIN(3,rc);

  if (oz_isOzSite(entity)) { // special case: sites
    if (pn == PN_NO_PROTOCOL || pn == PN_IMMEDIATE) return PROCEED;
    return oz_raise(E_SYSTEM, AtomDp, "incorrect protocol", 0);
  }

  Annotation a = getAnnotation(entity);

  // check incrementality and consistency
  if (pn != PN_NO_PROTOCOL) {
    if (a.pn != PN_NO_PROTOCOL && a.pn != pn) goto incremental_error;
    // check protocol consistency (quite rough, not complete yet)
    switch (pn) {
    case PN_NO_PROTOCOL: break;
    case PN_SIMPLE_CHANNEL:
      if (!oz_isConst(entity)) goto protocol_error;
      break;
    case PN_MIGRATORY_STATE:
    case PN_PILGRIM_STATE:
    case PN_EAGER_INVALID:
    case PN_LAZY_INVALID:
      if (!oz_isConst(entity)) goto protocol_error;
      break;
    case PN_TRANSIENT:
    case PN_TRANSIENT_REMOTE:
      if (!oz_isVarOrRef(entity)) goto protocol_error;
      break;
    case PN_IMMEDIATE:
    case PN_IMMUTABLE_LAZY:
    case PN_IMMUTABLE_EAGER:
      if (!oz_isConst(entity)) goto protocol_error;
      break;
    default:
      goto protocol_error;
    }
    a.pn = static_cast<ProtocolName>(pn);
  }
  if (aa != AA_NO_ARCHITECTURE) {
    if (a.aa != AA_NO_ARCHITECTURE && a.aa != aa) goto incremental_error;
    a.aa = static_cast<AccessArchitecture>(aa);
  }
  if (rc != RC_ALG_NONE) {
    if (a.rc != RC_ALG_NONE && a.rc != rc) goto incremental_error;
    a.rc = static_cast<RCalg>(rc);
  }

  // set annotation
  setAnnotation(entity, a);
  return PROCEED;

 incremental_error:
  return oz_raise(E_SYSTEM, AtomDp, "non-incremental annotation", 0);

 protocol_error:
  return oz_raise(E_SYSTEM, AtomDp, "incorrect protocol", 0);

} OZ_BI_end


OZ_BI_define(BIgetAnnotation,1,3)
{
  oz_declareSafeDerefIN(0,entity);
  if (oz_isOzSite(entity)) { // special case: sites
    OZ_out(0) = oz_int(PN_IMMEDIATE);
    OZ_out(1) = oz_int(AA_NO_ARCHITECTURE);
    OZ_out(2) = oz_int(RC_ALG_NONE);
  }
  Annotation a = getAnnotation(entity);
  OZ_out(0) = oz_int(a.pn);
  OZ_out(1) = oz_int(a.aa);
  OZ_out(2) = oz_int(a.rc);
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIgetFaultStream,1,1)
{
  oz_declareSafeDerefIN(0,entity);
  Mediator* med = glue_getMediator(entity);
  if (med)
    OZ_RETURN(med->getFaultStream());
  if (oz_isOzSite(entity))
    OZ_RETURN(ozSite2GlueSite(entity)->getFaultStream());
  return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);
} OZ_BI_end


OZ_BI_define(BIgetFaultState,1,1)
{
  oz_declareSafeDerefIN(0,entity);
  Mediator* med = glue_getMediator(entity);
  if (med)
    OZ_RETURN(fsToAtom(med->getFaultState()));
  if (oz_isOzSite(entity))
    OZ_RETURN(ozSite2GlueSite(entity)->getFaultState());
  return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);
} OZ_BI_end


OZ_BI_define(BIsetFaultState,2,0)
{
  oz_declareSafeDerefIN(0,entity);
  oz_declareNonvarIN(1,state);

  // first parse new state
  GlueFaultState fs;
  if (!atomToFS(state, fs))
    return oz_raise(E_SYSTEM, AtomDp, "invalid fault state", 1, state);

  Mediator* med = glue_getMediator(entity);
  if (med) {
    // check state transition
    if (!validFSTransition(med->getFaultState(), fs))
      return oz_raise(E_SYSTEM, AtomDp, "invalid fault transition", 1, state);
    // set new state
    med->setFaultState(fs);
    return PROCEED;
  }
  if (oz_isOzSite(entity)) {
    // translate GlueFaultState to DSiteState (HACK!)
    static DSiteState dfs[] =
      { DSite_OK, DSite_TMP, DSite_LOCAL_PRM, DSite_GLOBAL_PRM };
    // set new state
    ozSite2DSite(entity)->m_stateChange(dfs[fs]);
    return PROCEED;
  }
  return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);

} OZ_BI_end

/************************* Killing entities *************************/

OZ_BI_define(BIkill,1,0)
{
  oz_declareSafeDerefIN(0,entity);

  Mediator* med = glue_getMediator(entity);
  if (med) {
    if (med->isDistributed())
      med->abstractOperation_Kill();
    else
      med->setFaultState(GLUE_FAULT_PERM);
    return PROCEED;
  }
  if (oz_isOzSite(entity)) {
    return oz_raise(E_SYSTEM, AtomDp, "Kill: not implemented yet", 1, entity);
  }
  return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);
} OZ_BI_end


OZ_BI_define(BIbreak,1,0)
{
  oz_declareSafeDerefIN(0,entity);

  Mediator* med = glue_getMediator(entity);
  if (med) {   // simply set fault state to localFail (at least)
    med->setFaultState(GLUE_FAULT_LOCAL);
    return PROCEED;
  }
  if (oz_isOzSite(entity)) {   // put site to state DSite_LOCAL_PRM
    ozSite2DSite(entity)->m_stateChange(DSite_LOCAL_PRM);
    return PROCEED;
  }
  return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);
} OZ_BI_end



/**********************************************************************/
/*   Fault Builtins                                                    */
/**********************************************************************/


/* NOTE


   Due to the limited time available the implementation of 
   fault detection is not complete. Enough is implemented to 
   display the basic concepts. 

   Lacking fucntionality on lang-level: 
   
   + Injectors,or their only interesting format, exception raisers. 
   
   + The failure reporting over the EMU structure is lacking. 
   
*/


OZ_BI_define(BIinstallFaultPort,1,0){
  oz_declareNonvarIN(0,port);
  if (!oz_isPort(port)) {
    oz_typeError(0,"Port");
  }
  g_faultPort = port;
  OZ_protect(&g_faultPort);
  return PROCEED; 
}OZ_BI_end

OZ_BI_define(BIdistHandlerInstall,4,1){
  OZ_warning("Watcher installation disabled");
  OZ_RETURN(oz_bool(TRUE));
}OZ_BI_end




OZ_BI_define(BIdistHandlerDeInstall,2,1){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc0     = OZ_in(1);  
  OZ_RETURN(oz_bool(TRUE));
}OZ_BI_end

OZ_BI_define(BIgetEntityCond,2,0){
  OZ_warning("Watcher deinstallation disabled");
  OZ_RETURN(oz_cons(AtomNormal,oz_nil()));
}OZ_BI_end


// send a value (1) on a given site (0)
OZ_BI_define(BIsendMsgToSite,2,0){
  oz_declareNonvarIN(0, site);
  oz_declareIN(1, value);
  if (!oz_isOzSite(site)) oz_typeError(0, "site");

  MsgContainer* msg =
    glue_com_connection->a_msgnLayer->createCscSendMsgContainer();
  msg->pushPstOut(new PstOutContainer(value));
  ozSite2DSite(site)->m_sendMsg(msg);

  return PROCEED;
}OZ_BI_end


/* Get a list with all the sites in this process. */
OZ_BI_define(BIgetAllSites,0,1){
   OZ_Term siteLst = oz_nil();

   for(GlueSite* cur = getGlueSites(); cur; cur = cur->getNext()) {
     siteLst = oz_cons(cur->getOzSite(), siteLst); 
   }

   OZ_RETURN(siteLst);
}OZ_BI_end

/* Get a list with all connected sites. */
OZ_BI_define(BIgetConSites,0,1){
  OZ_Term siteLst = oz_nil();
  
  for(GlueSite* cur = getGlueSites(); cur; cur = cur->getNext()) {
    if ((cur->getDSite()->m_getChannelStatus() & CS_COMMUNICATING)
	&& (cur != thisGSite))
      siteLst = oz_cons(cur->getOzSite(), siteLst); 
   }
   OZ_RETURN(siteLst);
}OZ_BI_end


OZ_BI_define(BIgetChannelStatus,1,1){
  oz_declareNonvarIN(0, site);
  if (!oz_isOzSite(site)) return OZ_typeError(0, "site");

  ConnectivityStatus cstatus = ozSite2DSite(site)->m_getChannelStatus();
  Bool COMM    = cstatus & CS_COMMUNICATING;
  Bool CHANNEL = cstatus & CS_CHANNEL;
  Bool CIRCUIT = cstatus & CS_CIRCUIT;
  
  //! this record must still be completed with the other fields 
  OZ_RETURN(OZ_recordInit(oz_atom("cs"),
			  oz_cons(oz_pairA("communicating", oz_bool(COMM)),
			  oz_cons(oz_pairA("channel", oz_bool(CHANNEL)),
			  oz_cons(oz_pairA("circuit", oz_bool(CIRCUIT)),
			  oz_nil())))));
}OZ_BI_end


/* Get the site proper to current process. */
OZ_BI_define(BIgetThisSite,0,1){
  OZ_RETURN(thisGSite->getOzSite());
}OZ_BI_end


/* Get/Set the site info*/
OZ_BI_define(BIgetSiteInfo,1,1){
  oz_declareNonvarIN(0, site);
  if (!oz_isOzSite(site)) return OZ_typeError(0, "site");
  OZ_RETURN(ozSite2GlueSite(site)->getInfo());
}OZ_BI_end

OZ_BI_define(BIsetSiteInfo,2,0){
  oz_declareNonvarIN(0, site);
  if (!oz_isOzSite(site)) return OZ_typeError(0, "site");
  oz_declareNonvarIN(1, value);
  return tagged2OzSite(site)->putFeatureV(AtomInfo, value);
}OZ_BI_end


/*
 * The builtin table
 */
#ifndef MODULES_LINK_STATIC
#include "modGlue-if.cc"
#endif
