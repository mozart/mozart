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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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


/*
 * read an integer environment variable or use a default
 */
inline
int getenvDefault(char *envvar, int def)
{
  char *s = getenv(envvar);
  if (s) {
    int ret = atoi(s);
#ifdef DEBUG
    fprintf(stderr,"Using %s=%d\n", envvar, ret);
#endif
    return ret;
  }
  return def;
}

inline
char *getenvDefault(char *envvar, char *def) {
  char *s = getenv(envvar);
  return s ? s : def;
}

void ConfigData::init() {
  showLoad              = 0;
  showCacheLoad         = 0;
  ozHome                = getenvDefault("OZHOME","unknown");
  ozPath                = OZ_PATH;
  linkPath              = OZ_PATH;
  printDepth            = PRINT_DEPTH;
  printWidth            = PRINT_WIDTH;
  errorPrintDepth       = ERROR_PRINT_DEPTH;
  errorPrintWidth       = ERROR_PRINT_WIDTH;
  errorThreadDepth      = ERROR_THREAD_DEPTH;
  errorDebug            = ERROR_DEBUG;
  errorLocation         = ERROR_LOCATION;
  errorHints            = ERROR_HINTS;

  showFastLoad          = SHOW_FAST_LOAD;
  showForeignLoad       = SHOW_FOREIGN_LOAD;
  showIdleMessage       = SHOW_IDLE_MESSAGE;
  showSuspension        = SHOW_SUSPENSION;

  stopOnToplevelFailure = STOP_ON_TOPLEVEL_FAILURE;

  gcFlag                = GC_FLAG;
  gcVerbosity           = GC_VERBOSITY;

  stackMaxSize          = STACKMAXSIZE * TASKFRAMESIZE;
  stackMinSize          = STACKMINSIZE * TASKFRAMESIZE;

  heapMaxSize           = HEAPMAXSIZE;
  heapMinSize           = HEAPMINSIZE;
  heapFree              = HEAPFREE;
  heapTolerance         = HEAPTOLERANCE;
  heapThreshold         = INITIALHEAPTHRESHOLD;
  numToplevelVars       = getenvDefault("OZTOPLEVELVARS",NUM_TOPLEVEL_VARS);
  heapBlockSize         = getenvDefault("OZHEAPBLOCKSIZE",HEAPBLOCKSIZE);

  timeDetailed          = TIMEDETAILED;

  hiMidRatio            = DEFAULT_HI_MID_RATIO;
  midLowRatio           = DEFAULT_MID_LOW_RATIO;
#ifdef DEBUG_CHECK
  dumpCore              = 0;
#else
  dumpCore              = 1;
#endif

  runningUnderEmacs     = 0;
  browser               = 0;

  debugPerdio = 0;
  debugIP = 0;
  maxTcpCache = MAX_TCP_CACHE;
  maxUdpPacket = MAX_UDP_PACKET;
  tcpPacketSize = TCP_PACKET_SIZE;

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

}
