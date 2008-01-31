/*
 *  Authors:
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
#ifndef __MSLBASE_HH
#define __MSLBASE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "dss_comService.hh"
namespace _msl_internal{ //Start namespace
  
  class Site; 
  class ComServiceInterface; 
  class IO_factory;
  class Timers;
  class InterRouter;
  class SiteHT; 
  class ImmediateEvents; 
  class MsgCnt;
  class Event; 
  
  

class MsgnLayerEnv{
public: 
  Site*                a_destSite; 
  Site*                a_srcSite; 
  Site*                a_mySite;
  ::ComServiceInterface* a_comService; 
  Timers*              a_timers;
  InterRouter*         a_interRouter;
  ImmediateEvents*     a_immediateEvents;
  ::AppMslClbkInterface*      a_clbck;   
  SiteHT*              a_siteHT; 
  
public: // Counters 
  int                        a_OSWriteCounter;
  int                        a_OSReadCounter;
  int                        a_ContCounter;
  
  int                        a_SendCounter;
  int                        a_RecCounter;

  int                        a_routeIds; 
  
public: // system wide settings
  
  bool                       a_ipIsbehindFW;
public:
  void m_stateChange(Site*, const DSiteState&);
  void m_unsentMessages(Site*, MsgCnt*);  
  void m_loopBack(MsgCnt*); 
  void m_appendImmediateEvent(Event*); 
  void m_AppMessageReceived(MsgCnt*, Site*); 
  void m_CscMessageReceived(MsgCnt*, Site*); 
  DSS_LongTime* m_getCurrTime();
  
public: 
  void m_gcSweep();
  void m_heartBeat(const int& TimePassedInMs);
  
  int m_getFirewallReopenTimeout();
  int m_getReopenRemoteTimeout();
  
  
  MsgnLayerEnv(::AppMslClbkInterface* clbk, 
	       ::ComServiceInterface* csc, 
	       ::MsgnLayer *msl, 
	       const bool& sec);
  ~MsgnLayerEnv();
private: 
  MsgnLayerEnv(const MsgnLayerEnv&);
  MsgnLayerEnv& operator=(const MsgnLayerEnv&);
};
  


  

  class Event{
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
  private:
    Event(const Event& e) { DebugCode(a_allocated++); }
    Event& operator=(const Event& e){ return *this; }
  public:
    // If you want to do additional stuff with the object when scheduling
    virtual void event_action(){};
    virtual void event_execute(MsgnLayerEnv* env)=0; //Everything must execute something
    virtual void m_makeGCpreps(){};

    Event() { DebugCode(a_allocated++); };

    virtual ~Event(){ DebugCode(a_allocated--); };
  };


    
  enum TransMedium{
    TM_ROUTE, 
    TM_TCP
  };
  
  // Defined here because we need the C_CSC definition adn the C_APPLICATION definitions
  // in the msgnLayer.cc file. 
    enum MslMessageType {
    // Communication layer messages:
    C_FIRST = 0,                  // Just for the index, must be first of C_-msgs
    C_CSC,                        // Communication service message
    C_APPLICATION,                // Message send by the application using the msgnLayre(dss?)
    C_ANON_PRESENT,
    C_INIT_PRESENT,
    C_ANON_NEGOTIATE,
    C_INIT_NEGOTIATE,
    C_ANON_NEGOTIATE_ANS,
    C_ACK,
    C_CLOSE_HARD,
    C_CLOSE_WEAK,
    C_CLOSE_ACCEPT,
    C_CLOSE_REJECT,
    C_CLEAR_REFERENCE,
    C_SET_ACK_PROP,
    C_ROUTE_DISCOVERY,
    C_SET_ROUTE,
    C_TARGET_TOUCHED,
    C_ROUTE,
    C_DIRECT_MSG,
    C_PING,
    C_LAST
  };
  
  
}
#endif
