/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *    Konstantin Popov <kost@sics.se>
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

#include "wsock.hh"
#include "base.hh"
#include "dpBase.hh"

#include "perdio.hh"
#include "table.hh"
#include "dpMarshaler.hh"

#include "builtins.hh"
#include "os.hh"
#include "value.hh"
#include "base.hh"

#include "var.hh"
#include "msgContainer.hh"        

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

extern OZ_Term defaultAcceptProcedure;
extern OZ_Term defaultConnectionProcedure;

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
OZ_BI_define(BIcrash,0,0)   /* only for debugging */
{
  initDP();

  exit(1);  

  return PROCEED;
} OZ_BI_end


#ifdef DEBUG_PERDIO

OZ_BI_define(BIdvset,2,0)
{
  initDP();

  OZ_declareInt(0,what);
  OZ_declareInt(1,val);

  if (val) {
    DV->set(what);
  } else {
    DV->unset(what);
  }
  return PROCEED;
} OZ_BI_end

#else

OZ_BI_define(BIdvset,2,0)
{
  initDP();

  OZ_declareInt(0,what);
  OZ_declareInt(1,val);
  OZ_warning("has no effect - you must compile with DEBUG_PERDIO");
  return PROCEED;
} OZ_BI_end

#endif


/**********************************************************************/
/*   Misc Builtins                                            */
/**********************************************************************/

OZ_BI_define(BIslowNet,2,0)
{
  initDP();

  oz_declareIntIN(0,arg0);
  oz_declareIntIN(1,arg1);
#ifdef SLOWNET
  TSC_LATENCY = arg0;
  TSC_TOTAL_A = arg1;
  printf("New slownetvals ms:%d buff:%d \n", TSC_LATENCY, TSC_TOTAL_A);
#else
  printf("Slownet not installed\n");
#endif
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIclose,1,0)   
{
  oz_declareIntIN(0,time);
  dpExitWithTimer((unsigned int) time);
  osExit(0);
  return PROCEED;
} OZ_BI_end



OZ_BI_define(BIinitIPConnection,1,1)
{
  oz_declareNonvarIN(0,rec);

  OZ_Term ipf = oz_atom("ip");
  OZ_Term portf = oz_atom("port");
  OZ_Term fwf = oz_atom("firewall");
  OZ_Term cpr = oz_atom("connectProc");
  int ip,port;
  Bool fw;

  if (oz_isLiteral(rec)){
    ;
  }
  else if (oz_isLTuple(rec)) {
    ;
  }
  else if (oz_isSRecord(rec)) {
    SRecord *srec = tagged2SRecord(rec);
    int index = srec->getIndex(ipf);
    if (index>=0) { 
      OZ_Term t = srec->getArg(index);
      /* if(!oz_isString(t))
	oz_typeError(-1,"String");
      */
      char *s=oz_str2c(t);
      ip = (int)ntohl(inet_addr(s));
      setIPAddress(ip);
    }
    index = srec->getIndex(portf);
    if (index>=0) { 
      OZ_Term t = srec->getArg(index);
      if(!oz_isInt(t))
	oz_typeError(-1,"Int");
      port = OZ_intToC(t);
      setIPPort(port);
    }
    index = srec->getIndex(fwf);
    if (index>=0) { 
      OZ_Term t = srec->getArg(index);
      if(!oz_isBool(t))
	oz_typeError(-1,"Bool");
      fw = OZ_boolToC(t);
      setFirewallStatus(fw);
    }
    index = srec->getIndex(cpr);
    if (index>=0) {
      OZ_Term proc0 = srec->getArg(index);
      NONVAR(proc0,proc);
      if(!oz_isChunk(proc))
	oz_typeError(-1,"Chunk");
      defaultConnectionProcedure = proc;
      oz_protect(&defaultConnectionProcedure);
    }
  } else {
    oz_typeError(0,"Record");
  }

  initDP();
  ip = getIPAddress();
  int *pip= &ip;
  OZ_RETURN(OZ_recordInit(oz_atom("ipInfo"),
			  oz_cons(
				  oz_pairA("ip",OZ_string(osinet_ntoa((char *) pip)))
				  , 
				  oz_cons(oz_pairAI("port",getIPPort()),
					  oz_cons(oz_pairA("firewall",oz_bool(getFireWallStatus())), 
						  oz_cons(oz_pairAA("acceptProc","<>"),
							  oz_cons(oz_pairAA("connectProc","<>"),
								  oz_nil())
					  )
				  )
					  )
	    )));
} OZ_BI_end

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
  initDP();
  //  OZ_RETURN(oz_pairII(globalWriteCounter,globalReadCounter));
  OZ_RETURN(OZ_recordInit(oz_atom("globalMsgStatistics"),
			  oz_cons(oz_pairAI("sent",globalSendCounter),
			  oz_cons(oz_pairAI("received",globalRecCounter),
			  oz_cons(oz_pairAI("oswritten",globalOSWriteCounter),
			  oz_cons(oz_pairAI("osread",globalOSReadCounter),
			  oz_cons(oz_pairAI("cont",globalContCounter),
			  oz_cons(oz_pairAI("btResize",btResize),
			  oz_cons(oz_pairAI("btCompactify",btCompactify),
			  oz_cons(oz_pairAI("otResize",otResize),
				  oz_cons(oz_pairAI("otCompactify",otCompactify),
				  oz_nil())))))))))));
}OZ_BI_end   

OZ_BI_define(BIgetConnectWstream,0,1)
{
  initDP();
  OZ_RETURN(ConnectPortStream);
}OZ_BI_end 


OZ_BI_define(BIprintDPTables,0,0)
{
  initDP();
  ownerTable->print();
  borrowTable->print();
  return OZ_ENTAILED;
}OZ_BI_end  

OZ_BI_define(BIcreateLogFile,1,0)
{
  OZ_declareVirtualString(0,name);

  initDP();

  FILE *tmp=fopen(name, "w");
  if(tmp==NULL) {
    return OZ_FAILED;
  }
  else {
    logfile=tmp;
    setbuf(logfile,NULL); // No buffering to see immediate result
    return OZ_ENTAILED;
  }
}OZ_BI_end


OZ_BI_define(BIsetDGC,2,1)
{
  OZ_declareTerm(0,entity);
  OZ_declareTerm(1,algorithm);
  OB_TIndex OTI = (OB_TIndex) -1; 
  if(OZ_isPort(entity) || OZ_isCell(entity) || oz_isLock(entity))
    {
      Tertiary *tert =  (Tertiary*) tagged2Const(entity);
      if (tert->isManager())
	OTI = MakeOB_TIndex(tert->getTertPointer());
      else
	OZ_RETURN(oz_atom("not_manager"));
    }
  else{
    DEREF(entity,e0);
    if(oz_isVarOrRef(entity)){
      if (oz_isManagerVar(*e0))
	OTI = oz_getManagerVar(*e0)->getIndex();}
    else
      OZ_RETURN(oz_atom("not_manager"));
  }
  if (ownerIndex2ownerEntry(OTI)->homeRef.removeAlgorithm(algorithm))
    OZ_RETURN(oz_true());
  else
    OZ_RETURN(oz_false());
}OZ_BI_end  

OZ_BI_define(BIgetDGC,1,1)
{ 
  OZ_declareTerm(0,entity);
  if(OZ_isPort(entity) || OZ_isCell(entity) || oz_isLock(entity))
    {
      Tertiary *tert =  (Tertiary*) tagged2Const(entity);
      if (tert->isLocal())
	OZ_RETURN(oz_atom("local_entity"));
      if (tert->isManager()){
	OZ_RETURN(ownerIndex2ownerEntry(MakeOB_TIndex(tert->getTertPointer()))->homeRef.extract_info());}
      OZ_RETURN(borrowIndex2borrowEntry(MakeOB_TIndex(tert->getTertPointer()))->remoteRef.extract_info());
    }
  DEREF(entity,e0);
  if(oz_isVarOrRef(entity)){
    if (oz_isManagerVar(*e0))
      OZ_RETURN(ownerIndex2ownerEntry(oz_getManagerVar(*e0)->getIndex())->homeRef.extract_info());
    if (oz_isProxyVar(*e0))
      OZ_RETURN(borrowIndex2borrowEntry(oz_getProxyVar(*e0)->getIndex())->remoteRef.extract_info());
  }
  OZ_RETURN(oz_atom("local_entity"));
}OZ_BI_end  


OZ_BI_define(BIgetMsgPriority,0,1)
{
  OZ_Term msgPrio=oz_nil();
  for (int i=M_NONE+1; i<C_FIRST; i++) {
    char* prioType; 
    switch(default_mess_priority[i]){
    case MSG_PRIO_EAGER: 
      prioType = "eager";
      break;
    case MSG_PRIO_LAZY:
      prioType = "lazy";
      break;
    case MSG_PRIO_HIGH:
      prioType = "high";
      break;
    case MSG_PRIO_MEDIUM:
      prioType = "medium"; 
      break;
    case MSG_PRIO_LOW:
      prioType = "low";
      break;
    case USE_PRIO_OF_SENDER:
      prioType = "sender";
      break;
    default:
      prioType = "Error";
    }
    msgPrio=oz_cons(oz_pairAA(mess_names[i],prioType),msgPrio);
  }
   OZ_RETURN(OZ_recordInit(oz_atom("msgPrio"),msgPrio));

}OZ_BI_end

OZ_BI_define(BIsetMsgPriority,2,0)
{
  oz_declareNonvarIN(0,msg);
  oz_declareNonvarIN(1,prio);
  int int_prio;
  if (OZ_eq(prio,oz_atom("eager"))){
    int_prio =  MSG_PRIO_EAGER;
  }
  else
    if (OZ_eq(prio,oz_atom("lazy"))){
      int_prio =  MSG_PRIO_LAZY;
    }
    else
      if (OZ_eq(prio,oz_atom("high"))){
	int_prio =  MSG_PRIO_HIGH;
      }
      else
	if (OZ_eq(prio,oz_atom("medium"))){
	  int_prio =  MSG_PRIO_MEDIUM;
	}
	else
	  if (OZ_eq(prio,oz_atom("low"))){
	    int_prio =  MSG_PRIO_LOW;
	  }
	  else
	    if (OZ_eq(prio,oz_atom("sender"))){
	      int_prio = USE_PRIO_OF_SENDER;
	    }
	    else
	      return FAILED;
  
  for (int i=M_NONE+1; i<C_FIRST; i++) 
    if (OZ_eq(oz_atom(mess_names[i]),msg)){
      default_mess_priority[i]=int_prio;
      return OZ_ENTAILED;
    }
  return FAILED;
}OZ_BI_end




OZ_BI_define(BIsendCping,5,0)   
{
  oz_declareVirtualStringIN(0,host);
  oz_declareIntIN(1,Port);
  oz_declareIntIN(2,Ts);
  oz_declareIntIN(3,Pid);
  oz_declareIntIN(4,Pings);
  if (Pings < 0) return PROCEED;
  ip_address addr = ntohl(inet_addr(host));
  TimeStamp ts(Ts,Pid);
  DSite *site = findDSite(addr, Port, ts);
  int id = ++sendJobbCntr;
  MsgContainer *msgC = msgContainerManager->newMsgContainer(site);
  msgC->put_C_SEND_PING_PONG(id,Pings*2);
  send(msgC);
  return newSendJobb(id);
} OZ_BI_end

OZ_BI_define(BIsendMpongTerm,6,0)   
{
  oz_declareVirtualStringIN(0,host);
  oz_declareIntIN(1,Port);
  oz_declareIntIN(2,Ts);
  oz_declareIntIN(3,Pid);
  oz_declareIntIN(4,Pings);
  oz_declareIN(5,trm);
  if (Pings < 0) return PROCEED;
  ip_address addr = ntohl(inet_addr(host));
  TimeStamp ts(Ts,Pid);
  DSite *site = findDSite(addr, Port, ts);
  int id = ++sendJobbCntr;
  MsgContainer *msgC = msgContainerManager->newMsgContainer(site);
  msgC->put_M_PONG_TERM(myDSite,Pings*2,id,trm);
  send(msgC);
  return newSendJobb(id);
} OZ_BI_end


OZ_BI_define(BIsendMpongPL,5,0)   
{
  oz_declareVirtualStringIN(0,host);
  oz_declareIntIN(1,Port);
  oz_declareIntIN(2,Ts);
  oz_declareIntIN(3,Pid);
  oz_declareIntIN(4,Pings);
  if (Pings < 0) return PROCEED;
  ip_address addr = ntohl(inet_addr(host));
  TimeStamp ts(Ts,Pid);
  DSite *site = findDSite(addr, Port, ts);
  int id = ++sendJobbCntr;
  MsgContainer *msgC = msgContainerManager->newMsgContainer(site);
  msgC->put_M_PONG_PL(myDSite,Pings*2,id);
  send(msgC);
  return newSendJobb(id);
} OZ_BI_end




#ifndef MODULES_LINK_STATIC

#include "modDPMisc-if.cc"

#endif

