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
#include "newmarshaler.hh"
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
#ifdef VIRTUALSITES
#define USE_VS_MSGBUFFERS
#endif

// 
#ifdef USE_VS_MSGBUFFERS
#include "virtual.hh"
#endif

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
  int ip,port;
  Bool fw;

  if (oz_isLiteral(rec));    
  else if (oz_isLTuple(rec));
  else if (oz_isSRecord(rec)) {
    SRecord *srec = tagged2SRecord(rec);
    int index = srec->getIndex(ipf);
    if (index>=0) { 
      OZ_Term t = srec->getArg(index);
      /* if(!oz_isString(t))
	oz_typeError(-1,"String");
      */
      char *s=oz_str2c(t);
      ip = (int)inet_addr(s);
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
  } else {
    oz_typeError(0,"Record");
  }

  initDP();
  ip = ntohl(getIPAddress());
  int *pip= &ip;
  OZ_RETURN(OZ_recordInit(oz_atom("ipInfo"),
			  oz_cons(
				  oz_pairA("ip",OZ_string(osinet_ntoa((char *) pip)))
				  , 
				  oz_cons(oz_pairAI("port",getIPPort()),
					  oz_cons(oz_pairA("firewall",oz_bool(getFireWallStatus())), oz_nil()
						  )
					  )
				  )
			  )
	    );
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
  if (buff < 0) {
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
#ifndef WINDOWS
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

/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modDPMisc-if.cc"

#endif

