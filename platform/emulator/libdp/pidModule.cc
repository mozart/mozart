/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@dfki.de>
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "base.hh"
#include "dpBase.hh"

#include "am.hh"
#include "perdio.hh"
#include "table.hh"
#include "os.hh"
#include "builtins.hh"

#ifndef WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <netdb.h>

// The following sample code represents a practical implementation of the
// CRC (Cyclic Redundancy Check) employed in PNG chunks. (See also ISO
// 3309 [ISO-3309] or ITU-T V.42 [ITU-V42] for a formal specification.) 


/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];
   
/* Make the table for a fast CRC. */
inline
void make_crc_table(void) {
  unsigned long c;
  int n, k;
   
  for (n = 0; n < 256; n++) {
    c = (unsigned long) n;
    for (k = 0; k < 8; k++) {
      if (c & 1)
	c = 0xedb88320L ^ (c >> 1);
      else
	c = c >> 1;
    }
    crc_table[n] = c;
  }

}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below)). */

inline
unsigned long update_crc(unsigned long crc, unsigned char *buf, int len) {
  unsigned long c = crc;
  int n;
   
  make_crc_table();
  for (n = 0; n < len; n++) {
    c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  }
  return c;
}

OZ_BI_define(BIgetCRC,1,2) {
  oz_declareVirtualStringIN(0,s);

  int l = strlen(s);
  
  /* Return the CRC of the bytes buf[0..len-1]. */
  unsigned long crc = update_crc(0xffffffffL, (unsigned char *) s, 
				 strlen(s)) ^ 0xffffffffL;
  
  int hi = (int) ((crc & 0xffff0000L) >> 16);
  int lo = (int) (crc & 0x0000ffffL);
  
  OZ_out(0) = oz_int(lo);
  OZ_out(1) = oz_int(hi);
    
  return PROCEED;
  
} OZ_BI_end

//
extern OZ_Term GateStream;

//
OZ_BI_define(BIGetPID,0,1)
{
  initDP();

  // pid = pid(host:String port:Int time:Int#Int)
  char *nodename = oslocalhostname();
  if(nodename==NULL) { return oz_raise(E_ERROR,E_SYSTEM,"getPidUname",0); }
  struct hostent *hostaddr=gethostbyname(nodename);
  free(nodename);
  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));

  OZ_Term host = oz_pairA("host",oz_string(inet_ntoa(tmp)));
  OZ_Term port = oz_pairA("port",oz_int(myDSite->getPort()));
  OZ_Term time = 
    oz_pairA("time",
	     OZ_pair2(oz_unsignedLong((unsigned long) myDSite->getTimeStamp()->start),
		      oz_int(myDSite->getTimeStamp()->pid)));
  // NOTE: converting time_t to an unsigned long, maybe a [long] double!

  OZ_Term l = oz_cons(host,oz_cons(port,oz_cons(time,oz_nil())));
  OZ_RETURN(OZ_recordInit(OZ_atom("PID"),l));
} OZ_BI_end

OZ_BI_define(BIReceivedPID,1,0)
{
  oz_declareIN(0,stream);

  //
  initDP();

  return oz_unify(GateStream,stream);
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
    int i = oz_IntToC(timeV);
    if (i <= 0) goto tbomb;
    time = (time_t) i;
  } else if (oz_isBigInt(timeV)) {
    unsigned long i = tagged2BigInt(timeV)->getUnsignedLong();
    if (i==0 && i == OzMaxUnsignedLong) goto tbomb;
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
