/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#ifdef VIRTUALSITES
#define USE_VS_MSGBUFFERS
#endif

//
#ifdef USE_VS_MSGBUFFERS
#include "virtual.hh"
#endif

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


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modDPMisc-if.cc"

#endif
