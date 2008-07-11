/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2004
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
#pragma implementation "dgc_tl.hh"
#endif

#include "dgc_tl.hh"
#include "referenceConsistency.hh"
#include "msl_serialize.hh"

namespace _dss_internal{

  enum RLmessageType{

  }; 

  // ************************** PUBLIC TL_HOME *********************************
  inline void  TL_Home::extend_period(const int& lease_ms){
    a_expireDate = (m_getEnvironment()->a_msgnLayer->m_getCurrTime());
    a_expireDate.increaseTime(lease_ms);
  }

  
  TL_Home::TL_Home(HomeReference* const p, GCalgorithm* const g,
		   const int& period):
    HomeGCalgorithm(p,g,RC_ALG_TL),
    a_periodTime(period),
    a_expireDate(DSS_LongTime()){
    extend_period(period);
  }

  TL_Home::~TL_Home(){}


  void
  TL_Home::m_getReferenceInfo(DssWriteBuffer *bs, DSite *dest){
    gf_MarshalNumber(bs, a_periodTime);
    extend_period(a_periodTime);
  }


  bool
  TL_Home::m_isRoot(){
    return ((m_getEnvironment()->a_msgnLayer->m_getCurrTime()) <= a_expireDate);
  }

  void
  TL_Home::m_getCtlMsg(DSite* msite, MsgContainer* msg){
    MsgContainer *msgC = m_createRemoteMsg();
    msgC->pushIntVal(a_periodTime);
    m_sendToRemote(msite, msgC); 
    extend_period(a_periodTime);
  }


  bool TL_Home::setLeasePeriod(const int& val) {
    return (((a_expireDate - (m_getEnvironment()->a_msgnLayer->m_getCurrTime())) < (val + LEAST_PERIOD) && val > LEAST_PERIOD) ? (a_periodTime = val)!=0 : false);
  }

  // *************************** PUBLIC TL_REMOTE *********************************
  static unsigned int tl_update_timer_expired(void* v){
    return static_cast<TL_Remote*>(v)->updateTimerExpired();
  }

  inline void  TL_Remote::extend_period(const int& lease_ms){
    a_expireDate = (m_getEnvironment()->a_msgnLayer->m_getCurrTime());
    a_expireDate.increaseTime(lease_ms);
  }

  inline void
  TL_Remote::setTimer(const int& period){
    Assert(a_timer == NULL);
    a_timer = m_getEnvironment()->a_msgnLayer->m_setTimer((period - a_periodTime),
					      tl_update_timer_expired,
						reinterpret_cast<void*>(this));
  }


  TL_Remote::TL_Remote(RemoteReference* const p, DssReadBuffer *bs,
		       GCalgorithm* const g, const int& period):
    RemoteGCalgorithm(p,g,RC_ALG_TL),
    a_periodTime(period),
    a_expireDate(DSS_LongTime()),
    a_timer(NULL){
    int lease_ms = gf_UnmarshalNumber(bs);
    extend_period(lease_ms);
    if (lease_ms < a_periodTime){
      // Request more time
      MsgContainer* msg = m_createHomeMsg();
      m_sendToHome(msg); 
    } else
      setTimer(lease_ms); // a_timer == NULL
  }

  TL_Remote::~TL_Remote()
  { 
    m_getEnvironment()->a_msgnLayer->m_clearTimer(a_timer); 
  }

  unsigned int TL_Remote::updateTimerExpired(){
    int lt = a_expireDate - (m_getEnvironment()->a_msgnLayer->m_getCurrTime());
    // ZACHARIAS: below should be something like 1.5*UPDATE_LEASE to avoid excess timers
    if(lt > a_periodTime)
      return lt;

    a_timer = NULL;
    
    m_sendToHome(m_createHomeMsg());
    return 0;
  }
 

  void TL_Remote::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    gf_MarshalNumber(bs, (a_expireDate - ((m_getEnvironment()->a_msgnLayer->m_getCurrTime()))));
  }


  void TL_Remote::m_mergeReferenceInfo(DssReadBuffer *bs){
    DSS_LongTime lt = (m_getEnvironment()->a_msgnLayer->m_getCurrTime());
    int lease_ms = gf_UnmarshalNumber(bs);
    lt.increaseTime(lease_ms);
    if (lt > a_expireDate){
      a_expireDate = lt;
      // If the new time received is a "valid" time 
      // install a timer if none exists
      if (a_timer == NULL && lease_ms > a_periodTime)
	setTimer(lease_ms);
    }
  }


  void TL_Remote::m_dropReference()
  {
    m_getEnvironment()->a_msgnLayer->m_clearTimer(a_timer);
    a_timer = NULL;
  }

  bool TL_Remote::m_isRoot() { return false; }


  void TL_Remote::m_getCtlMsg(DSite*, MsgContainer* msg){
    //assume "update response"
    int lease_ms  = msg->popIntVal();
    extend_period(lease_ms);
    if (a_timer == NULL &&  lease_ms > a_periodTime)
      setTimer(lease_ms);
  }

  bool TL_Remote::setUpdatePeriod(const int& val){
    return (((a_expireDate - (m_getEnvironment()->a_msgnLayer->m_getCurrTime())) > (val+LEAST_PERIOD) && val > LEAST_PERIOD) ? (a_periodTime = val)!=0: false);
  }

  
}
