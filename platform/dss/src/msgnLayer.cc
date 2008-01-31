/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
 * 
 *  Contributors:
 * 
 *  Copyright:
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
 
#include "dss_comService.hh"
#include "mslBase.hh"
#include "msl_msgContainer.hh"
#include "msl_comObj.hh"
#include "msl_tcpTransObj.hh"
#include "msl_dsite.hh"
using namespace _msl_internal;

#define WRAP_TIME 0xFFFFFFFF


char* DSS_LongTime::stringrep() {
    static char s[2*sizeof(unsigned long)*8+2];
    if(high==0)
      sprintf(s,"%010ld",low);
    else
      sprintf(s,"%ld%010ld",high,low);
    return s;
  }

#define WRAP_TIME  0xFFFFFFFF

DSS_LongTime::DSS_LongTime():low(0),high(0){}

void DSS_LongTime::increaseTime(const unsigned int& interval) {
  if(WRAP_TIME-interval>low)      // No overflow
    low+=interval;
  else {                          // Would create overflow
    low -= (WRAP_TIME-interval); 
      high++;                       // Assumes interval << WRAP_TIME
  }
}

bool DSS_LongTime::operator<=(const DSS_LongTime &t2) { 
  return ((high < t2.high) || ((high == t2.high) && (low <= t2.low))); 
}

bool DSS_LongTime::operator>(const DSS_LongTime &t2) {
  return ((high > t2.high) || ((high == t2.high) && (low >  t2.low))); 
}

bool DSS_LongTime::operator!=(const DSS_LongTime &t2) { 
  return ((low != t2.low)  ||  (high != t2.high)); 
}
  
  // This is assumed to be used only to compare times that are rather close
  // to each other and thus fit in an int.
int DSS_LongTime::operator-(const DSS_LongTime &t2) {
  if(this->high==t2.high)
    return this->low-t2.low; 
  else if(this->high==t2.high+1) {
    return (WRAP_TIME-t2.low)+this->low; // this->low+WRAP_TIME - t2.low
    // rewritten to avoid
    // overflow
  }
  else
    printf("%s: Time difference too far apart.",__FILE__);
  return -1;
}
  

MsgnLayer::MsgnLayer(::AppMslClbkInterface* clb , ::ComServiceInterface* csc, 
		     const bool& sec):
  a_mslEnv(NULL),
  a_myDSite(NULL)
{
  a_mslEnv = new MsgnLayerEnv(clb, csc, this, sec);
  a_myDSite = static_cast<DSite*>(a_mslEnv->a_mySite); 
}


MsgnLayer::~MsgnLayer(){
  delete a_mslEnv;
}

 DSite*
 MsgnLayer::m_getDestDSite(){
   return static_cast<DSite*>(a_mslEnv->a_destSite); 
 }
DSite*
MsgnLayer::m_getSourceDSite(){
  return static_cast<DSite*>(a_mslEnv->a_srcSite); 
}


MsgContainer *
MsgnLayer::createCscSendMsgContainer(){
  MsgCnt* msg = new MsgCnt(C_CSC, false);
  return static_cast<MsgContainer*>(msg); 
}


MsgContainer *
MsgnLayer::createAppSendMsgContainer(){
  MsgCnt* msg = new MsgCnt(C_APPLICATION, false);
  return static_cast<MsgContainer*>(msg); 
}

DSite*
MsgnLayer::m_UnmarshalDSite(DssReadBuffer* buf){
  Site *s = a_mslEnv->a_siteHT->m_unmarshalSite(buf);
  return static_cast<DSite*>(s); 
}

void
MsgnLayer::m_anonymousChannelEstablished(VirtualChannelInterface* channel){

    // Look into the channel and find out that it is: 
    // a reliable-fifo-channel medium, and voila, we can use the
    // TCP transobj. 
  
  TCPTransObj *transObj= new TCPTransObj(a_mslEnv); 
  ComObj   *comObj  = new ComObj(NULL, a_mslEnv);
  transObj->setOwner(comObj);
  transObj->setChannel(channel);
  comObj->m_acceptAnonConnection(static_cast<TransObj*>(transObj));
}


TimerElementInterface* MsgnLayer::m_setTimer( const unsigned int& time, TimerWakeUpProc t, void* const arg){
  // the timer interafcfe allows for reseting old timer elements. This
  // is an old heritage from the days when an unsorted list was used. 
  // Howver, this is not the case now, when we use timewheels. 
  
  TimerElement *ele = NULL; 
  a_mslEnv->a_timers->setTimer(ele, time, t, arg); 
  return static_cast<TimerElementInterface*>(ele); 
}

void MsgnLayer::m_clearTimer(TimerElementInterface* tel){
  TimerElement* te = static_cast<TimerElement*>(tel);
  a_mslEnv->a_timers->clearTimer(te);
}


void
MsgnLayer::m_heartBeat(const int& TimePassedInMs){
  a_mslEnv->m_heartBeat( TimePassedInMs);
}

void 
MsgnLayer::m_gcResources(){
  a_mslEnv->m_gcSweep(); 
}

DSS_LongTime MsgnLayer::m_getCurrTime(){
  return a_mslEnv->a_timers->currTime();
}



