/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
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

#if defined(INTERFACE)
#pragma implementation "ozconfig.hh"
#endif

#include "ozconfig.hh"
#include "base.hh"

#include <stdlib.h>
#include <stdio.h>

ConfigData ozconf;

Bool getDefaultPropertyBool(char *p,Bool def)
{
  char *str=getenv(p);
  if (!str) return def;
  char *rest;
  long l = strtol(str, &rest, 10);
  if (*rest!=0) return def;
  return l != 0;
}


void ConfigData::init() {
  printDepth		= PRINT_DEPTH;
  printWidth		= PRINT_WIDTH;
  errorPrintDepth	= ERROR_PRINT_DEPTH;
  errorPrintWidth	= ERROR_PRINT_WIDTH;
  errorThreadDepth    	= ERROR_THREAD_DEPTH;
  errorDebug   	        = ERROR_DEBUG;
  errorLocation   	= ERROR_LOCATION;
  errorHints     	= ERROR_HINTS;

  showIdleMessage	= SHOW_IDLE_MESSAGE;
  showSuspension	= SHOW_SUSPENSION;
  useFutures		= getDefaultPropertyBool("oz.style.useFutures",NO);

  stopOnToplevelFailure = STOP_ON_TOPLEVEL_FAILURE;

  gcFlag		= GC_FLAG;
  gcVerbosity		= GC_VERBOSITY;
  codeGCcycles          = CODE_GC_CYLES;

  stackMaxSize          = STACKMAXSIZE * TASKFRAMESIZE;
  stackMinSize          = STACKMINSIZE * TASKFRAMESIZE;

  heapMaxSize           = HEAPMAXSIZE;
  heapMinSize           = HEAPMINSIZE;
  heapFree              = HEAPFREE;
  heapTolerance         = HEAPTOLERANCE;
  heapThreshold         = INITIALHEAPTHRESHOLD;

  timeDetailed          = TIMEDETAILED;

  hiMidRatio            = DEFAULT_HI_MID_RATIO;
  midLowRatio           = DEFAULT_MID_LOW_RATIO;
#ifdef DEBUG_CHECK
  dumpCore		= 0;
#else
  dumpCore		= 1;
#endif

  runningUnderEmacs     = 0;

  debugPerdio  = 0;
  perdioMinimal = 0;
  debugIP = 0;
  maxTcpCache = MAX_TCP_CACHE;
  maxUdpPacket = MAX_UDP_PACKET;
  tcpPacketSize = TCP_PACKET_SIZE;

  perdioSeifHandler = 1;
  perdioUseAltVarProtocol = FALSE;

  dpTableDefaultOwnerTableSize = DEFAULT_OWNER_TABLE_SIZE;
  dpTableDefaultBorrowTableSize = DEFAULT_BORROW_TABLE_SIZE;
  dpTableLowLimit = DP_TABLE_LOW_LIMIT;
  dpTableExpandFactor = DP_TABLE_EXPAND_FACTOR;
  dpTableBuffer = DP_TABLE_BUFFER;
  dpTableWorthwhileRealloc = DP_TABLE_WORTHWHILE_REALLOC;

  closetime = DEFAULT_CLOSE_TIME;

  /* set osname and cpu */
  /* "ozplatform" (defined in version.cc) has the form <osname-cpu>, 
   * so split it up */
  extern char *ozplatform;

  osname = ozstrdup(ozplatform);
  cpu = osname;
  while(1) {
    if (*cpu=='-') {
      *cpu='\0';
      cpu++;
      break;
    }
    if (*cpu=='\0') {
      cpu = "unknown";  /* should never happen */
      break;
    }
    cpu++;
  }

  url="";

}

