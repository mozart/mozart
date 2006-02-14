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
#include "glue_entities.hh"
#include "dss_object.hh"
#include "glue_interface.hh"
#include "glue_tables.hh"
#include "glue_utils.hh"
#include "engine_interface.hh"
#include "glue_siteRepresentation.hh"
#include "glue_buffer.hh"
#include "glue_base.hh"
#include "glue_mediators.hh"
#include "glue_marshal.hh"
#include "glue_ozSite.hh" // used for Oz_Site 
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

#define DeclareSiteListIN(ARG,VAR)					\
OZ_Term VAR = OZ_in(ARG);						\
{ OZ_Term arg = VAR;							\
  while (OZ_isCons(arg)) {						\
    TaggedRef a = OZ_head(arg);						\
    if (OZ_isVariable(a)) OZ_suspendOn(a);				\
    /*if (!OZ_isAtom(a))    return OZ_typeError(ARG,"list(Atom)"); */	\
    arg = OZ_tail(arg);							\
  }									\
  if (OZ_isVariable(arg)) OZ_suspendOn(arg);				\
  if (!OZ_isNil(arg))     return OZ_typeError(ARG,"list(site)");	\
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

DSite *ozSite2DssSite(OZ_Term site){
  Oz_Site *oz_site = static_cast<Oz_Site*>(OZ_getExtension(OZ_deref(site)));
  Glue_SiteRep *gsr = oz_site->getGSR();
  return gsr->m_getDssSite();
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


OZ_Term createGrantRecord(void *g){
  return OZ_recordInit(oz_atom("grant"),
		       oz_cons(oz_pairAI("key",reinterpret_cast<int>(g)),oz_nil()));
}


OZ_Return readGrantRecord(OZ_Term record, void *&g){
  int gr;
  OZ_Return ret=getRecordField(record,"key",gr);
  if(ret!=OZ_ENTAILED) return ret;
  g = reinterpret_cast<void*> (gr);
  return OZ_ENTAILED;
}

/* Handover a route(0) to a remote site(1). */
OZ_BI_define(BIhandoverRoute,2,0) {
  DeclareSiteListIN(0, slist);
  OZ_declareTerm(1, peer);

  OZ_Term hd, tl;
  DSite *lst_sn = NULL;
  int nrSites = 0;
  OZ_Term tmp = slist; 
  while (dssIsCons(tmp, &hd, &tl)) {
    if (OZ_isVariable(hd)) return SUSPEND;
    
    if (OZ_isExtension(hd)) {
      OZ_Extension *e = OZ_getExtension(hd);
      if (e->getIdV() != OZ_E_SITE) 
	return OZ_typeError(0,"site");
    }
    else return FAILED;
    nrSites++; 
    tmp = tl; 
  }
  
  // route list too small 
  if (nrSites < 2) 
    return OZ_FAILED; 

  // route list too long; the marshaler does not support more
  if (nrSites > 6) 
    return OZ_FAILED; 

  DSite **dsVec = new DSite*[nrSites];

  int tmpNum = nrSites; 
  while (dssIsCons(slist, &hd, &tl)) {
    Assert(tmpNum > 0);

    /*
    Oz_Site *oz_site =
      static_cast<Oz_Site*>(OZ_getExtension(OZ_deref(hd)));
    Glue_SiteRep *gsa = oz_site->getGSR();
    gsa->m_showRId();
    */

    dsVec[--tmpNum] = ozSite2DssSite(hd);
    slist = tl;
  }

  Assert(tmpNum == 0); 
  ozSite2DssSite(peer)->m_virtualCircuitEstablished(nrSites, dsVec); 
  
  return PROCEED;
}OZ_BI_end



OZ_BI_define(BIhandover,2,0){
  oz_declareNonvarIN(0,requestor);
  OZ_declareTerm(1,settings);
  int  fd;
  OZ_Return ret; 

  ret=getRecordField(settings,"fd",fd);
  if(ret!=OZ_ENTAILED) return ret;
  
  // encapsulating the filedescriptor in a Transport_Channel object. 
  // It can now be used freely from the DSS, using the virtual 
  // functions defined in DssTransportChannel and implemented
  // in Glue_TransportChannel. 
  VirtualChannelInterface* channel =   glue_ioFactory->channelFromFd(fd);
  
  DSite *requested_by = NULL; 
  if(!oz_eq(requestor,oz_atom("accept")))
    {
      int con;
      OZ_Return ret=getRecordField(requestor,"req",con);
      if(ret!=OZ_ENTAILED) return ret;
      Glue_SiteRep *sa = reinterpret_cast<Glue_SiteRep*>(con);
      sa->m_setConnection(channel);
      return OZ_ENTAILED;
    }
  glue_com_connection->a_msgnLayer->m_anonymousChannelEstablished(channel); 
  return OZ_ENTAILED;
}OZ_BI_end



// Not connection failed stupid, but 
// statechange

OZ_BI_define(BIconnFailed,2,0) {
  oz_declareNonvarIN(0,requestor);
  oz_declareNonvarIN(1,reason);
  
  int Cid;
  OZ_Return ret = getRecordField(requestor, "req", Cid);
  if (ret != OZ_ENTAILED) return ret;
  
  Glue_SiteRep *sa = reinterpret_cast<Glue_SiteRep*>(Cid);
  
  if (oz_eq(reason, oz_atom("perm"))) {
    sa->m_getDssSite()->m_stateChange(DSite_GLOBAL_PRM);
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



OZ_BI_define(BImigrateManager,1,0){
  OZ_declareTerm(0,entity);
  // Check that the argument actually is 
  // distributed and fetch the ProxyName
  ConstTermWithHome *ct = static_cast<ConstTermWithHome*>(tagged2Const(entity));
  if ((oz_isPort(entity) || oz_isCell(entity)) && ct->isDistributed()) {
    CoordinatorAssistantInterface *pi = 
      static_cast<Mediator*>(ct->getMediator())->getCoordinatorAssistant();
    //ZACHARIAS: argument 2 is unecessary so pass any void*
    pi->manipulateCNET(NULL); 
  }
  if(oz_isArray(entity)) {
    if(ct->isDistributed()){
      CoordinatorAssistantInterface *pi = 
        static_cast<Mediator*>(ct->getMediator())->getCoordinatorAssistant();
      pi->manipulateCNET(NULL); 
    }
  }
  return PROCEED; 
}OZ_BI_end


OZ_BI_define(BIinitIPConnection,6,1)
{
  oz_declareIntIN(0, port);
  oz_declareProperStringIN(1,addr);
  oz_declareNonvarIN(2, ozsiteId);
  oz_declareNonvarIN(3,proc);
  oz_declareNonvarIN(4,ozPort);
  oz_declareIntIN(5, primKey);
  if (!oz_isAtom(ozsiteId)) {
    return OZ_typeError(0,"atom");
  }
  const char *siteId = OZ_atomToC(ozsiteId);

  int ip = (int)inet_addr(addr);
  
  g_defaultConnectionProcedure = proc;
  OZ_protect(&g_defaultConnectionProcedure);
  initDP(port,ip, siteId, primKey); 
  
  g_connectPort = ozPort;
  OZ_protect(&g_connectPort);

  OZ_RETURN(OZ_recordInit(oz_atom("ipInfo"),
			  oz_cons(
				  oz_pairAA("ip",addr), 
				  oz_cons(oz_pairAI("port",80),
					  oz_cons(oz_pairA("firewall",oz_bool(TRUE)),
						  oz_cons(oz_pairAA("acceptProc","<>"),
							  oz_cons(oz_pairAA("connectProc","<>"),
								  oz_nil())
							  )
						  )
					  )
				  )));
  
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

  // marshal the Dss abstract entity
  GlueWriteBuffer buf(portToTickBuf, PORT_TO_TICK_BUF_LEN);
//  pm->marshal(&buf, PMF_FREE);
  med->getCoordinatorAssistant()->marshal(&buf, PMF_FREE);

  // turn it into a string
  int len = buf.bufferUsed();
  char *str = encodeB64((char*) portToTickBuf, len);
  OZ_RETURN(OZ_string(str));
}OZ_BI_end


OZ_BI_define(BImsToPort,1,1)
{
  oz_declareProperStringIN(0,str);
  int len = strlen(str); 
  unsigned char* raw_buf = (unsigned char*)decodeB64((char*)str, len);
  
  AbstractEntity *ae; 
  AbstractEntityName aen;
  
  GlueReadBuffer buf(raw_buf, len);
  DSS_unmarshal_status status = dss->unmarshalProxy(ae,&buf, PUF_FREE,aen);
  free(raw_buf);
  
  if(status.exist) {
    PortMediator *med = static_cast<PortMediator*>(ae->accessMediator());
    OZ_RETURN(med->getEntity());

  } else {
    // create a port whose stream is unused
    OzPort* p = new OzPort(oz_currentBoard(), makeTaggedNULL());
    TaggedRef t = makeTaggedConst(p);
    p->setMediator(new PortMediator(t, ae));
    OZ_RETURN(t);
  }
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

OZ_BI_define(BIgetOperCntr,0,1)
{
  OZ_error("Removed during reconstruction");
  OZ_RETURN(oz_nil());
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


OZ_BI_define(BIsetDGC,2,1)
{
  OZ_error("Removed during reconstruction");
  OZ_RETURN(oz_false());
}OZ_BI_end  

OZ_BI_define(BIgetDGC,1,1)
{ 
  OZ_declareTerm(0,entity);
  OZ_RETURN(oz_atom("local_entity"));
}OZ_BI_end  

// ZACHARIAS
OZ_BI_define(BIgetDGCAlgs,0,1)
{ 
  OZ_error("Removed during interface reconstruction");
  OZ_RETURN(oz_true());
}OZ_BI_end  


OZ_BI_define(BIgetDGCAlgInfo,1,1)
{ 
  OZ_error("Removed during interface reconstruction");
  OZ_RETURN(oz_true());
}OZ_BI_end  


OZ_BI_define(BIsetDGCAlg,2,0)
{ 
  OZ_error("Removed during reconstruction");
  return OZ_ENTAILED;
}OZ_BI_end  


OZ_BI_define(BIsetDGCAlgProp,3,0)
{ 
  OZ_error("Removed during reconstruction");
  return OZ_ENTAILED;
}OZ_BI_end  


OZ_BI_define(BIgetMsgPriority,0,1)
{
  OZ_error("Removed during reconstruction");
  OZ_RETURN(oz_nil());
}OZ_BI_end

OZ_BI_define(BIsetMsgPriority,2,0)
{
  OZ_error("Removed during reconstruction");
  return FAILED;
}OZ_BI_end




OZ_BI_define(BIsendCping,5,0)   
{
  OZ_error("Removed during reconstruction"); return PROCEED; 
} OZ_BI_end

OZ_BI_define(BIsendMpongTerm,6,0)   
{
  OZ_error("Removed during reconstruction"); return PROCEED; 
} OZ_BI_end


OZ_BI_define(BIsendMpongPL,5,0)   
{
  OZ_error("Removed during reconstruction"); return PROCEED; 
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
  else
    return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);
} OZ_BI_end

OZ_BI_define(BIgetFaultState,1,1)
{
  oz_declareSafeDerefIN(0,entity);
  Mediator* med = glue_getMediator(entity);
  if (med)
    OZ_RETURN(fsToAtom(med->getFaultState()));
  else
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
  if (med == NULL)
    return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);

  // check state transition
  if (!validFaultStateTransition(med->getFaultState(), fs))
    return oz_raise(E_SYSTEM, AtomDp, "invalid fault transition", 1, state);

  // set new state
  med->setFaultState(fs);
  return PROCEED;

} OZ_BI_end

OZ_BI_define(BIgetMaxRtt,0,1)
{
  OZ_RETURN(oz_int(RTT_UPPERBOUND));
} OZ_BI_end

OZ_BI_define(BIsetMaxRtt,1,0)
{
  oz_declareIntIN(0,maxrtt);
  RTT_UPPERBOUND = maxrtt;
  return PROCEED;
} OZ_BI_end



OZ_BI_define(BItablesExtract,0,1)
{
  OZ_RETURN(oz_false());
} OZ_BI_end

OZ_BI_define(BIsiteStatistics,0,1)
{
  Assert(0); 
  OZ_RETURN(oz_false());
} OZ_BI_end

			       
OZ_BI_define(BI_DistMemInfo,0,1)
{
  OZ_RETURN(oz_nil());
} OZ_BI_end


//
// The names from marshalBase is not printable. 
// A new set of names are defined here.
// If incompatibilites should ocour please update this
// array to the same number of entries as the master copy.
// Erik 
const struct {
  MarshalTag tag;
  char *name;
} dif_Mynames[] = {
  { DIF_UNUSED0,         "unused0"},
  { DIF_SMALLINT,        "smallint"},
  { DIF_BIGINT,          "bigint"},
  { DIF_FLOAT,           "float"},
  { DIF_ATOM_DEF,        "atom_def"},
  { DIF_NAME_DEF,        "name_def"},
  { DIF_UNIQUENAME_DEF,  "uniquename_def"},
  { DIF_RECORD_DEF,      "record_def"},
  { DIF_TUPLE_DEF,       "tuple_def"},
  { DIF_LIST_DEF,        "list_def"},
  { DIF_REF,             "ref"},
  { DIF_UNUSED1,         "unused1"},
  { DIF_OWNER_DEF,       "owner_def"},
  { DIF_UNUSED2,         "unused2"},
  { DIF_PORT_DEF,        "port_def"}, // 
  { DIF_CELL_DEF,        "cell_def"},
  { DIF_LOCK_DEF,        "lock_def"},
  { DIF_VAR_DEF,         "var_def"},
  { DIF_BUILTIN_DEF,     "builtin_def"},
  { DIF_DICT_DEF,        "dict_def"},
  { DIF_OBJECT_DEF,      "object_def"},
  { DIF_UNUSED3,         "unused3"},
  { DIF_UNUSED4,         "unused4"},
  { DIF_CHUNK_DEF,       "chunk_def"},
  { DIF_PROC_DEF,        "proc_def"},
  { DIF_CLASS_DEF,       "class_def"},
  { DIF_ARRAY_DEF,       "array_def"},
  { DIF_FSETVALUE,       "fsetvalue"},
  { DIF_ABSTRENTRY,      "abstrentry"},
  { DIF_UNUSED5,         "unused5"},
  { DIF_UNUSED6,         "unused6"},
  { DIF_SITE,            "site"},
  { DIF_UNUSED7,         "unused7"},
  { DIF_SITE_PERM,       "site_perm"},
  { DIF_UNUSED8,         "unused8"},
  { DIF_COPYABLENAME_DEF,"copyablename_def"},
  { DIF_EXTENSION_DEF,   "extension_def"},
  { DIF_RESOURCE_DEF,    "resource_def"},
  { DIF_RESOURCE,        "resource"},
  { DIF_READONLY_DEF,    "readonly_def"},
  { DIF_VAR_AUTO_DEF,    "automatically_registered_var_def"},
  { DIF_READONLY_AUTO_DEF, "automatically_registered_readonly_def"},
  { DIF_EOF,             "eof"},
  { DIF_CODEAREA,        "code_area_segment"},
  { DIF_VAR_OBJECT_DEF,  "var_object_exported_def"},
  { DIF_SYNC,            "sync"},
  { DIF_CLONEDCELL_DEF,  "clonedcell_def"},
  { DIF_STUB_OBJECT_DEF, "object_exported_def"},
  { DIF_SUSPEND,         "marshaling_suspended"},
  { DIF_LIT_CONT,        "dif_literal_continuation"},
  { DIF_EXT_CONT,        "dif_extension_continuation"},
  { DIF_SITE_SENDER,     "site_opt"},
  { DIF_RECORD,          "record"},
  { DIF_TUPLE,           "tuple"},
  { DIF_LIST,            "list"},
  { DIF_PORT,            "port"},
  { DIF_CELL,            "cell"},
  { DIF_LOCK,            "lock"},
  { DIF_BUILTIN,         "builtin"},
  { DIF_DICT,            "dict"},
  { DIF_OBJECT,          "object"},
  { DIF_CHUNK,           "chunk"},
  { DIF_PROC,	         "proc"},
  { DIF_CLASS,           "class"},
  { DIF_EXTENSION,       "extension"},
  { DIF_STUB_OBJECT,     "object_exported"},
  { DIF_BIGINT_DEF,      "bigint_def"},
  { DIF_CLONEDCELL,      "clonedcell"},
  { DIF_ARRAY,           "array"},
  { DIF_ATOM,            "atom"},
  { DIF_NAME,  	         "name"},
  { DIF_UNIQUENAME,      "uniquename"},
  { DIF_COPYABLENAME,    "copyablename"},
  { DIF_OWNER,           "owner"},
  { DIF_VAR,             "var"},
  { DIF_READONLY,        "readonly"},
  { DIF_VAR_AUTO,        "automatically_registered_var"},
  { DIF_READONLY_AUTO,   "automatically_registered_readonly"},
  { DIF_VAR_OBJECT,      "var_object_exported"},
  { DIF_LAST,            "last"}
};


OZ_BI_define(BIperdioStatistics,0,1)
{
  Assert(0);
  OZ_RETURN(oz_nil());
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


void cellOperationDoneReadImpl(OzCell*, TaggedRef, int);

OZ_BI_define(BIremoteExecDone,3,0){
  OZ_Term entity  =   OZ_in(0);
  OZ_Term ans     =   OZ_in(1);
  OZ_Term id      =   OZ_in(2); 
  
  OzCell *cell = tagged2Cell(entity);
  int id_int = OZ_intToC(id); 
  cellOperationDoneReadImpl(cell, ans, id_int); 
  return PROCEED;
}OZ_BI_end


/* Send a Pst message to this site. */
OZ_BI_define(BIsendMsgToSite,2,0){
  OZ_declareTerm(0, tag_site);
  OZ_declareTerm(1, msg);

  if (OZ_isExtension(tag_site)) {
    OZ_Extension *e = OZ_getExtension(tag_site);
    if (e->getIdV() != OZ_E_SITE) 
      return OZ_typeError(0,"site");
  }
  else return FAILED; 

  PstOutContainer *load = new PstOutContainer(msg);
  MsgContainer *msgC = NULL; 
  ozSite2DssSite(tag_site)->m_sendMsg(msgC);

  return PROCEED;
}OZ_BI_end


/* Get a list with all the sites in this process. */
OZ_BI_define(BIgetAllSites,0,1){
   OZ_Term siteLst = oz_nil();

   for(Glue_SiteRep* cur = site_address_representations; 
       cur != NULL;  
       cur = cur->m_getNext() ) {
     siteLst = oz_cons(cur->m_getOzSite(), siteLst); 
   }

   OZ_RETURN(siteLst);
}OZ_BI_end

/* Get a list with all connected sites. */
OZ_BI_define(BIgetConSites,0,1){
  OZ_Term siteLst = oz_nil();
  
  for(Glue_SiteRep* cur = site_address_representations; 
      cur != NULL;  
      cur = cur->m_getNext() ) {
    if(((cur->m_getDssSite()->m_getChannelStatus() & CS_COMMUNICATING) == CS_COMMUNICATING) 
       && (cur != thisGSite))
      siteLst = oz_cons(cur->m_getOzSite(), siteLst); 
   }
   OZ_RETURN(siteLst);
}OZ_BI_end


OZ_BI_define(BIgetChannelStatus,1,1){
  OZ_declareTerm(0, tag_site);
  ConnectivityStatus cstatus;
  Bool COMMUNICATING = FALSE;
  Bool CHANNEL = FALSE;
  Bool CIRCUIT = FALSE;
  Bool ROUTER = FALSE;

  if (OZ_isExtension(tag_site)) {
    OZ_Extension *e = OZ_getExtension(tag_site);
    if (e->getIdV() != OZ_E_SITE) 
      return OZ_typeError(0,"site");
  }
  else return FAILED;

  cstatus = ozSite2DssSite(tag_site)->m_getChannelStatus();

  if ((cstatus & CS_COMMUNICATING) == CS_COMMUNICATING) 
    COMMUNICATING = TRUE;
  if ((cstatus & CS_CHANNEL) == CS_CHANNEL) 
    CHANNEL = TRUE;
  if ((cstatus & CS_CIRCUIT) == CS_CIRCUIT) 
    CIRCUIT = TRUE;
  
  //! this record must still be completed with the other fields 
  OZ_RETURN(OZ_recordInit(oz_atom("cs"),
			  oz_cons(oz_pairA("communicating", oz_bool(COMMUNICATING)),
			  oz_cons(oz_pairA("channel", oz_bool(CHANNEL)),
			  oz_cons(oz_pairA("circuit", oz_bool(CIRCUIT)),
			  oz_nil())))));
}OZ_BI_end

/* Get the site proper to current process. */
OZ_BI_define(BIgetThisSite,0,1){
  OZ_RETURN(thisGSite->m_getOzSite());
}OZ_BI_end


/* Get the site info*/
OZ_BI_define(BIgetSiteInfo,1,1){
  OZ_declareTerm(0, tag_site);

  if (OZ_isExtension(tag_site)) {
    OZ_Extension *e = OZ_getExtension(tag_site);
    if (e->getIdV() != OZ_E_SITE) 
      return OZ_typeError(0,"site");
  }
  else return FAILED;

  Oz_Site *oz_site =
    static_cast<Oz_Site*>(OZ_getExtension(OZ_deref(tag_site)));
  Glue_SiteRep *gsa = oz_site->getGSR();
  OZ_RETURN(gsa->m_getInfo());
}OZ_BI_end

TaggedRef BI_remoteExecDone =  makeTaggedConst(new Builtin("", "remoteExecDone", 3, 0, BIremoteExecDone, OK));

OZ_BI_define(BIcreateDHT,1,0)
{
  oz_declareNonvarIN(0,ozPort);
  g_kbrStreamPort = ozPort;
  OZ_protect(&g_kbrStreamPort);
  // dss->kbr_start(NULL);
  Assert(0); 
  printf("dhtStarted\n"); 
  return PROCEED;

}OZ_BI_end

OZ_BI_define(BIcreateSiteRef,0,1)
{
  GlueWriteBuffer buf(portToTickBuf, PORT_TO_TICK_BUF_LEN); 
  GlueWriteBuffer *ptr = &buf; 
  Assert(dynamic_cast<DssWriteBuffer*>(ptr));
  DssWriteBuffer *dwb_ptr = static_cast<DssWriteBuffer*>(ptr); 
  printf("buf %d\n", ptr); 
  thisGSite->m_getDssSite()->m_marshalDSite(ptr);
  int len = buf.bufferUsed();
  char *str = encodeB64((char*)portToTickBuf, len);
  printf("site: %s\n", str); 
  OZ_RETURN( OZ_string(str));
}OZ_BI_end


OZ_BI_define(BIconnectDHT,2,0)
{

  oz_declareProperStringIN(1,str);
  oz_declareNonvarIN(0,ozPort);
  g_kbrStreamPort = ozPort;
  OZ_protect(&g_kbrStreamPort);
  
  int len = strlen(str); 
  unsigned char* raw_buf = (unsigned char*)decodeB64((char*)str, len);
  GlueReadBuffer buf(raw_buf, len);
  DSite *dsite = glue_com_connection->a_msgnLayer->m_UnmarshalDSite(&buf);
  delete raw_buf;
  Assert(0);
  // dss->kbr_start(dsite);
  return PROCEED;
}OZ_BI_end


OZ_BI_define(BIinsertDHTitem,2,0)
{
  oz_declareIntIN(0,key);
  OZ_declareTerm(1,value);
  Assert(0);
  // bool ans = dss->kbr_route(key, new PstOutContainer(value)); 
  // We should actually check the answer...
  //Assert(ans); 
  return PROCEED; 
}OZ_BI_end


OZ_BI_define(BIlookupDHTitem,1,0)
{
  oz_declareIntIN(0,key);
  //dss->dht_lookup(key); 
  return PROCEED; 
}OZ_BI_end




OZ_BI_define(BIkbrTransferResp,1,0)
{
  OZ_declareTerm(0,resp);
  // dss->kbr_transferResp(new PstOutContainer(resp));
  Assert(0);
  return PROCEED; 
}OZ_BI_end

/*
 * The builtin table
 */
#ifndef MODULES_LINK_STATIC
#include "modGlue-if.cc"
#endif
