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
#pragma implementation "dpDebug.hh"
#endif

#include "base.hh"
#include "dpBase.hh"
#include "debug.hh"
#include "dpDebug.hh"
#include "network.hh"
#include "thr_int.hh"

//
// This one is used by the 'Fault' library now;
OZ_BI_define(BIcloseCon,1,1)
{
  OZ_declareInt(0,what);
  OZ_RETURN(oz_int(openclose(what)));
} OZ_BI_end



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
  "LOCK_MGR",
  
  "CHAIN",        //45 
  "PORT",
  "TCP_ERROR",
  "TCP_CONNECTION"
};


Bool openClosedConnection(int Type);
void wakeUpTmp(int i, int time);
int openclose(int Type);

OZ_BI_define(BIstartTmp,2,0)
{
  OZ_declareInt(0,val);
  OZ_declareInt(1,time);
  PD((TCPCACHE,"StartTmp v:%d t:%d",val,time));
  if(openClosedConnection(val)){
    PD((TCPCACHE,"StartTmp; continuing"));
    wakeUpTmp(val,time);}
  return PROCEED;
} OZ_BI_end

TaggedRef BI_startTmp;

void wakeUpTmp(int i, int time) {
  PD((TCPCACHE,"Starting DangelingThread"));
  Thread *tt = oz_newThread(LOW_PRIORITY);
  tt->pushCall(BI_startTmp, oz_int(i), oz_int(time));
  tt->pushCall(BI_Delay, oz_int(time));
}


#ifdef DEBUG_PERDIO

#ifdef DEBUG_CHECK
void maybeDebugBufferGetImpl(BYTE b){
  PD((MARSHAL_CT,"one char got c:%d",b));}

void maybeDebugBufferPutImpl(BYTE b){
  PD((MARSHAL_CT,"one char put c:%d",b));}
#endif

#else

#ifdef DEBUG_CHECK
void maybeDebugBufferGetImpl(BYTE b) {}
void maybeDebugBufferPutImpl(BYTE b) {}
#endif


#endif



