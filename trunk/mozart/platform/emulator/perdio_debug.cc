/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
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

#ifdef DEBUG_PERDIO

DebugVector *DV = NULL;

char *debugTypeStr[LAST] = {
  "MSG_RECEIVED", //0
    "MSG_SENT",	
    "MSG_PREP",  
    "TABLE",
    "TABLE2",
    "GC",       //  5
    "CREDIT",	
    "LOOKUP",
    "GLOBALIZING",
    "LOCALIZING",    
    "PD_VAR",  // 10
    "CELL",
    "LOCK",
    "SITE_OP",  
    "THREAD_D",

    "MARSHAL",	// 15
    "MARSHAL_CT",
    "UNMARSHAL",
    "UNMARSHAL_CT",
    "MARSHAL_BE",
    "REF_COUNTER", // 20

    "TCP",
    "WEIRD",        
    "TCP_INTERFACE",
    "TCPCACHE",
    "TCPQUEUE",   // 25
    "SITE",
    "REMOTE",   
    "MESSAGE",
    "OS",
    "BUFFER",    // 30
    "READ",
    "WRITE",   
    "CONTENTS",
    "HASH",
    "HASH2",    // 35

  "USER",
  "SPECIAL",
  "ERROR_DET",   
  "WRITE_QUEUE",
  "ACK_QUEUE",    //40
  "CELL_MGR",
  "PROBES",
  "NET_HANDLER",
  "LOCK_MGR"
  "CHAIN"        //45
};

#endif




