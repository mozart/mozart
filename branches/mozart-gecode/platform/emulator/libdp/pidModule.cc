/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#include "am.hh"
#include "perdio.hh"
#include "table.hh"
#include "os.hh"
#include "builtins.hh"
#include "pickleBase.hh"

#ifndef WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif


OZ_BI_define(BIgetCRC,1,1) 
{
  oz_declareVirtualStringIN(0,s);

  crc_t crc = update_crc(init_crc(),(unsigned char *) s, strlen(s));
    
  OZ_RETURN(OZ_unsignedInt(crc));
} OZ_BI_end

//
extern OZ_Term GateStream;

//
OZ_BI_define(BIGetPID,0,1)
{
  initDP();
  /*
  // pid = pid(host:String port:Int time:Int#Int)
  char *nodename = oslocalhostname();
  if(nodename==NULL) { return oz_raise(E_ERROR,E_SYSTEM,"getPidUname",0); }
  struct hostent *hostaddr=gethostbyname(nodename);
  free(nodename);
  if (hostaddr==NULL) {
    nodename = "localhost";
    hostaddr=gethostbyname(nodename);
    OZ_warning("Unable to reach the net, using localhost instead\n");
  }
  */

  unsigned int addr = ntohl(myDSite->getAddress());
  unsigned int *paddr = &addr;
  OZ_Term host = oz_pairA("host",OZ_string(osinet_ntoa((char *) paddr)));
  OZ_Term port = oz_pairA("port",oz_int(myDSite->getPort()));
  OZ_Term time = 
    oz_pairA("time",
	     OZ_pair2(OZ_unsignedLong((unsigned long) myDSite->getTimeStamp()->start),
		      OZ_int(myDSite->getTimeStamp()->pid)));
  // NOTE: converting time_t to an unsigned long, maybe a [long] double!

  OZ_Term l = oz_cons(host,oz_cons(port,oz_cons(time,oz_nil())));
  OZ_RETURN(OZ_recordInit(OZ_atom("PID"),l));
} OZ_BI_end

OZ_BI_define(BIReceivedPID,0,1)
{
  //
  initDP();

  OZ_Term gate = GateStream;
  GateStream = OZ_unit(); // clear it - avoid space leak
  OZ_RETURN(gate);
} OZ_BI_end


OZ_BI_define(BITicket2Port,4,1)
{
  oz_declareVirtualStringIN(0,host);
  oz_declareIntIN(1,port);
  oz_declareNonvarIN(2,timeV);
  oz_declareIntIN(3,pid);

  //
  initDP();

  time_t time;
  if (oz_isSmallInt(timeV)) {
    int i = OZ_intToC(timeV);
    if (i <= 0) goto tbomb;
    time = (time_t) i;
  } else if (oz_isBigInt(timeV)) {
    unsigned long i = OZ_intToCulong(timeV);
    if (i==0)
      goto tbomb;
    time = (time_t) i;
  } else {
  tbomb:
    return oz_raise(E_ERROR,E_SYSTEM,"PID.send",2,
		    OZ_atom("badTime"),OZ_in(2));
  }
    
  ip_address addr = ntohl(inet_addr(host));
  if (addr == (ip_address)-1L) {
    return oz_raise(E_ERROR,E_SYSTEM,"PID.send",2,
		    OZ_atom("inet_addr"),OZ_in(0));
  }

  TimeStamp ts(time,pid);
  DSite *site = findDSite(addr, port, ts);

  if (!site) {
    return oz_raise(E_ERROR,E_SYSTEM,"Ticket2Port",4,
		    OZ_atom("findDSite"),OZ_in(0),OZ_in(1),
		    OZ_in(2));}
  
  OZ_RETURN(getGatePort(site));
} OZ_BI_end


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modPID-if.cc"

#endif
