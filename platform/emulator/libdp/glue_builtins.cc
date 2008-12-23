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
// handles IRIX6 (6.5 at least), IRIX64, HPUX_700 (9.05 at least),
// Solaris does not try to be BSD4.3-compatible by default (but one
// can say '-DBSD_COMP' and then that stuff is included as well)
#if defined(SOLARIS_SPARC)
#include    <sys/sockio.h>
#else
#include    <sys/ioctl.h>
#endif
#endif

/****************************** Utils ******************************/

#define DeclareSiteListIN(ARG,VAR,LEN)                                  \
OZ_Term VAR = OZ_in(ARG);                                               \
int LEN = 0;                                                            \
{ OZ_Term arg = VAR;                                                    \
  while (OZ_isCons(arg)) {                                              \
    TaggedRef a = oz_safeDeref(OZ_head(arg));                           \
    if (OZ_isVariable(a)) OZ_suspendOn(a);                              \
    if (!oz_isOzSite(a)) return OZ_typeError(ARG, "list(site)");        \
    arg = OZ_tail(arg);                                                 \
    ++LEN;                                                              \
  }                                                                     \
  if (OZ_isVariable(arg)) OZ_suspendOn(arg);                            \
  if (!OZ_isNil(arg)) return OZ_typeError(ARG,"list(site)");            \
}

#define oz_expectInt(VAR,REF,ERROR)             \
  int VAR;                                      \
  {                                             \
    TaggedRef _VAR = oz_safeDeref(REF);         \
    if (oz_isVarOrRef(_VAR)) {                  \
      OZ_suspendOn(_VAR);                       \
    } else if (oz_isSmallInt(_VAR)) {           \
      VAR = tagged2SmallInt(_VAR);              \
    } else if (oz_isBigInt(_VAR)) {             \
      VAR = tagged2BigInt(_VAR)->getInt();      \
    } else {                                    \
      ERROR;                                    \
    }                                           \
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


// create a connection for the given site, with the given pair of file
// descriptors
OZ_BI_define(BIsetConnection,2,0){
  oz_declareNonvarIN(0,site);
  if (!oz_isOzSite(site)) oz_typeError(0, "site");
  oz_declareSTupleIN(1, pair);
  if (pair->getWidth() != 2) oz_typeError(1, "pair of ints");
  oz_expectInt(fd0, pair->getArg(0), oz_typeError(1, "pair of ints"));
  oz_expectInt(fd1, pair->getArg(1), oz_typeError(1, "pair of ints"));

  DssChannel* channel = new SocketChannel(fd0, fd1);
  ozSite2GlueSite(site)->m_setConnection(channel);

  // notify the DP port
  OZ_Term ack = OZ_mkTupleC("connection_received", 2, site, OZ_in(1));
  doPortSend(tagged2Port(g_connectPort),ack,NULL);
  return PROCEED;

}OZ_BI_end


// hand over an anonymous connection to the DSS (arg is a pair of file
// descriptors)
OZ_BI_define(BIacceptConnection,1,0){
  oz_declareSTupleIN(0, pair);
  if (pair->getWidth() != 2) oz_typeError(0, "pair of ints");
  oz_expectInt(fd0, pair->getArg(0), oz_typeError(0, "pair of ints"));
  oz_expectInt(fd1, pair->getArg(1), oz_typeError(0, "pair of ints"));

  DssChannel* channel = new SocketChannel(fd0, fd1);
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
    ozSite2DSite(site)->m_stateChange(FS_GLOBAL_PERM);
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
  static FaultState dfs[] = { FS_OK, FS_TEMP, FS_LOCAL_PERM, FS_GLOBAL_PERM };
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


/**********************************************************************/
/*   Misc Builtins                                                    */
/**********************************************************************/


extern
int raiseUnixError(char *f,int n, char * e, char * g);

//
#define WRAPCALL(f, CALL, RET)                          \
int RET;                                                \
while ((RET = CALL) < 0) {                              \
  if (ossockerrno() != EINTR) { RETURN_UNIX_ERROR(f); } \
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
  OZ_Term lba = oz_nil();       // list of broadcast addresses;

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
#ifdef WIN32
                 (const char *) &on,
#else
                 (void *) &on,
#endif
                 (socklen_t) sizeof(on))
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

OZ_BI_define(BIannotate,2,0) {
  oz_declareSafeDerefIN(0, entity);
  oz_declareNonvarIN(1, list);
  // check list
  TaggedRef var;
  if (!OZ_isList(list, &var)) {
    if (var == 0) oz_typeError(1, "list");
    oz_suspendOn(var);
  }
  // parse list
  Annotation a;
  OZ_Return ret = a.parseTerm(list);
  if (ret != PROCEED) return ret;

  // handle special case: sites
  if (oz_isOzSite(entity)) {
    if (a.pn == PN_NO_PROTOCOL || a.pn == PN_IMMEDIATE) return PROCEED;
    return oz_raise(E_SYSTEM, AtomDp, "annotation", 2, entity, list);
  }

  // get entity mediator, and annotate
  Mediator* med = glue_getMediator(entity);
  if (med) {
    if (med->annotate(a)) return PROCEED;
    return oz_raise(E_SYSTEM, AtomDp, "annotation", 2, entity, list);
  }
  return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);

} OZ_BI_end


OZ_BI_define(BIgetAnnotation,1,1) {
  oz_declareSafeDerefIN(0,entity);
  Annotation a;
  if (oz_isOzSite(entity)) { // special case: sites
    a = Annotation(PN_IMMEDIATE, AA_NO_ARCHITECTURE, RC_ALG_NONE);
  } else {
    Mediator* med = glue_getMediator(entity);
    if (!med)
      return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);
    a = med->getAnnotation();
  }
  OZ_RETURN(a.toTerm());
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
    // translate GlueFaultState to FaultState (HACK!)
    static FaultState dfs[] = { FS_OK, FS_TEMP, FS_LOCAL_PERM, FS_GLOBAL_PERM };
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
  if (oz_isOzSite(entity)) {   // put site to state FS_LOCAL_PERM
    ozSite2DSite(entity)->m_stateChange(FS_LOCAL_PERM);
    return PROCEED;
  }
  return oz_raise(E_SYSTEM, AtomDp, "nondistributable entity", 1, entity);
} OZ_BI_end


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
