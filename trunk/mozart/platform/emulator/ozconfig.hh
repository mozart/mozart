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

#ifndef __CONFH
#define __CONFH

#ifdef INTERFACE
#pragma interface
#endif

// this class contains the configurable parameters
class ConfigData {
public:
  int showLoad;
  int showCacheLoad;
  int printDepth;
  int printWidth;

  int errorPrintDepth; 
  int errorPrintWidth;
  int errorThreadDepth; // int, how many tasks are printed    
  int errorLocation;    // bool, print location
  int errorHints;       // bool, print hints
  int errorDebug;       // bool, add debug information

  int showForeignLoad;	// show message on load
  int showFastLoad;	// show message on fast load
  int showIdleMessage;	// show message on idle
  int showSuspension;   // show message when a suspension is created

  int stopOnToplevelFailure;  // enter machine level debugger on TOPLEVEL FAIL

  int gcFlag;                 // request GC to run
  int gcVerbosity;            // GC verbosity level

  int heapThreshold;
  int heapMaxSize;
  int heapMinSize;
  int heapFree;
  int heapTolerance;
  int heapBlockSize;

  int stackMinSize;
  int stackMaxSize;

  int timeDetailed;

  int hiMidRatio;
  int midLowRatio;
  // int timeSlice; use TIME_SLICE directly, so we save a costly div in AM::restartThread()

  int debugIP;
  int debugPerdio;
  int maxTcpCache;
  int maxUdpPacket;
  int tcpPacketSize;

  int numToplevelVars;

  int dumpCore;

  int runningUnderEmacs;
  int applet;
  int browser;

  char *ozHome;
  char *osname, *cpu;
  char *ozPath;
  char *linkPath;

  /* command line arguments visible from Oz */
  char **argV;
  int argC;
  
public:
  ConfigData() {};
  void init();
};

extern ConfigData ozconf;

#endif
