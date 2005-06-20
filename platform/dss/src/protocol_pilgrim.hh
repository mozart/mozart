/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Zacharias El Banna, 2002
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
#ifndef __PROTOCOL_PILGRIM_HH
#define __PROTOCOL_PILGRIM_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"
namespace _dss_internal{ //Start namespace

// **************************  Migratory Token  ***************************
  enum Pilgrim_Token {
    PLGT_NON_MEMBER,
    PLGT_RING_MEMBER,
    PLGT_SPAWNING_JOBS,
    PLGT_WAITING_FOR_JOBS,
    PLGT_SOLE_MEMBER
  };

  class RingElement{
  public: 
    DSite *a_site; 
    RingElement  *a_prev; 
    RingElement  *a_next; 
    RingElement(DSite*, RingElement*, RingElement*);
  private:
    RingElement(const RingElement&):a_site(NULL), a_prev(NULL),a_next(NULL){}
    RingElement operator=(const RingElement&){ return *this;} 
  };
  

  class ProtocolPilgrimManager:public ProtocolManager {
  private:
    RingElement *a_ringEle;
    FifoQueue<  TwoContainer<DSite, bool> > a_enterLeaveQueue; 
    void m_enterLeave();

    ProtocolPilgrimManager(const ProtocolPilgrimManager&):
      ProtocolManager(), a_ringEle(NULL),
      a_enterLeaveQueue(){}
    ProtocolPilgrimManager& operator=(const ProtocolPilgrimManager&){ return *this; }

  public:
    ProtocolPilgrimManager(DSite* mysite);
    ProtocolPilgrimManager(::MsgContainer*);
    ~ProtocolPilgrimManager(){};
    void makeGCpreps();
    void msgReceived(::MsgContainer*,DSite*);
    void sendMigrateInfo(MsgContainer*); 
  };
  

  class ProtocolPilgrimProxy:public ProtocolProxy{
  private:
    DSite*    a_next;     // NULL
    Pilgrim_Token a_state;  //empty
    FifoQueue< OneContainer<GlobalThread> > a_Pqueue; // int queue
    int a_jobs; 
    int a_use;
    
    void m_requestToken();
    void m_deregisterToken();
    void m_forwardToken();
    void m_resumeOperations();

    ProtocolPilgrimProxy(const ProtocolPilgrimProxy&):
      ProtocolProxy(PN_NO_PROTOCOL), a_next(NULL), a_state(PLGT_NON_MEMBER),
      a_Pqueue(), a_jobs(0),a_use(0){}
    ProtocolPilgrimProxy& operator=(const ProtocolPilgrimProxy&){ return *this; }

  public:
    ProtocolPilgrimProxy(DSite*);
    bool m_initRemoteProt(DssReadBuffer*);
    ~ProtocolPilgrimProxy(); 

    bool isWeakRoot(){
      return (a_state != PLGT_NON_MEMBER);
    };
    
    bool clearWeakRoot();    
    
    OpRetVal protocol_Access(GlobalThread* const, ::PstOutContainerInterface**&);
    void makeGCpreps(); 
    void msgReceived(::MsgContainer*,DSite*);
    void remoteInitatedOperationCompleted(DssOperationId* opId,::PstOutContainerInterface* pstOut);
    
    void localInitatedOperationCompleted(); 
  };

} //End namespace
#endif 
