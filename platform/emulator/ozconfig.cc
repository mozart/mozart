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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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
#include "os.hh"

#include <stdlib.h>
#include <stdio.h>

ConfigData ozconf;

Bool getDefaultPropertyBool(char *p,Bool def)
{
  char *str=osgetenv(p);
  if (!str) return def;
  char *rest;
  long l = strtol(str, &rest, 10);
  if (*rest!=0) return def;
  return l != 0;
}


void ConfigData::init() {
  printDepth		= PRINT_DEPTH;
  printWidth		= PRINT_WIDTH;
  printVerbose          = PRINT_VERBOSE;
  errorPrintDepth	= ERROR_PRINT_DEPTH;
  errorPrintWidth	= ERROR_PRINT_WIDTH;
  errorThreadDepth    	= ERROR_THREAD_DEPTH;
  errorDebug   	        = ERROR_DEBUG;

  showIdleMessage	= SHOW_IDLE_MESSAGE;
  showSuspension	= SHOW_SUSPENSION;
  useFutures		= getDefaultPropertyBool("oz.style.useFutures",NO);

  stopOnToplevelFailure = STOP_ON_TOPLEVEL_FAILURE;

  gcFlag		= GC_FLAG;
  gcVerbosity		= GC_VERBOSITY;
  codeGCcycles          = CODE_GC_CYLES;

  stackMinSize          = STACKMINSIZE * TASKFRAMESIZE;

  heapMinSize           = HEAPMINSIZE;
  heapFree              = HEAPFREE;
  heapTolerance         = HEAPTOLERANCE;
  heapThreshold         = INITIALHEAPTHRESHOLD;

  timeDetailed          = TIMEDETAILED;

  hiMidRatio            = DEFAULT_HI_MID_RATIO;
  midLowRatio           = DEFAULT_MID_LOW_RATIO;

  bwlIterationsPerMS	= 0;
  bwlMSs		= DEFAULT_BWL_MS;

  pickleCells		= DEFAULT_PICKLE_CELLS;

#ifdef DEBUG_CHECK
  dumpCore		= 0;
#else
  dumpCore		= 1;
#endif

  runningUnderEmacs     = 0;

  debugPerdio  = 0;
  debugIP = 0;

  dpSeifHandler = 1;
  dpFlowBufferSize  = DP_FLOWBUFFERSIZE;
  dpFlowBufferTime  = DP_FLOWBUFFERTIME;
  dpUseAltVarProtocol = FALSE;
  dpRetryTimeCeiling = DP_RETRYTIMECEILING;
  dpRetryTimeFloor = DP_RETRYTIMEFLOOR;
  dpRetryTimeFactor = DP_RETRYTIMEFACTOR;
  dpTCPHardLimit = DP_TCPHARDLIMIT;
  dpTCPWeakLimit = DP_TCPWEAKLIMIT;
  dpProbeInterval = DP_PROBEINTERVAL;
  dpProbeTimeout = DP_PROBETIMEOUT;
  dpOpenTimeout = DP_PROBETIMEOUT / 10;
  dpCloseTimeout = DP_PROBETIMEOUT * 10;
  dpWFRemoteTimeout = DP_PROBETIMEOUT * 100;
  dpFirewallReopenTimeout = DP_PROBETIMEOUT / 10;
  dpDefaultBufferSize = DP_DEF_BYTEBUFFER_SIZE;
  dpDefaultMaxBufferSize = DP_DEF_MAX_BYTEBUFFER_SIZE;
  dpBufferSize = DP_DEF_BYTEBUFFER_SIZE;
  dpMaxBufferSize = DP_DEF_MAX_BYTEBUFFER_SIZE;

  dpTableDefaultOwnerTableSize = DEFAULT_OWNER_TABLE_SIZE;
  dpTableDefaultBorrowTableSize = DEFAULT_BORROW_TABLE_SIZE;
  dpTableLowLimit = DP_TABLE_LOW_LIMIT;
  dpTableExpandFactor = DP_TABLE_EXPAND_FACTOR;
  dpTableBuffer = DP_TABLE_BUFFER;
  dpTableWorthwhileRealloc = DP_TABLE_WORTHWHILE_REALLOC;

  dpLogConnectLog = FALSE;
  dpLogMessageLog = FALSE;

  dpUseTimeLease = FALSE;
  dpUseFracWRC  = TRUE; 
  dp_wrc_alpha     = DP_WRC_ALPHA;
  dp_tl_leaseTime  = DP_TL_LEASE;
  dp_tl_updateTime = DP_TL_UPDATE; 
  
  closetime = DEFAULT_CLOSE_TIME;

  /* set osname and cpu */
  /* "ozplatform" (defined in version.cc) has the form <osname-cpu>, 
   * so split it up */
  extern char *ozplatform;

  osname = strdup(ozplatform);
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
  gui=0;

}

