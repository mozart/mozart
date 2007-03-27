/*
 *  Authors:
 *    Erik Klintskog,     2004 
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog,     2004
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

#ifndef __DSS_COORDINATOR_MOBILE_HH
#define __DSS_COORDINATOR_MOBILE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "coordinator.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace
  
  class ProxyMobile; 

  class CoordinatorMobile: public Coordinator {
    friend class ProxyMobile;
  protected:
    void m_initProxy(Proxy *p);
    int  a_epoch; 
    void m_migrateCoordinator(DSite*); 
  public:
    // ************** CONSTRUCTORS *******************
    CoordinatorMobile( ProtocolManager* const prot,
		       const RCalg& gc_annot,
		       DSS_Environment* const env);
    
    CoordinatorMobile( NetIdentity ni,  DSS_Environment* const env, 
		       int epoch, ProxyMobile *prx, MsgContainer *msg);
    
    virtual ~CoordinatorMobile();
    
    // ****************** Message Abstractions **********************
    
    virtual ::MsgContainer *m_createProxyProtMsg();
    virtual ::MsgContainer *m_createProxyRefMsg();
    virtual bool m_sendToProxy(DSite* dest, MsgContainer* msg);
    
    // ****************** Message Receivers *************************
    virtual void    m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite);
    virtual void    m_receiveRefMsg(MsgContainer *msgC, DSite* fromsite);
    virtual void    m_receiveAsMsg(MsgContainer *msgC, DSite* sender);
    
      // ******************* MISC *************************************
    
    virtual char   *m_stringrep();
    
    //******************* GC routines ******************************
    virtual void    m_makeGCpreps();
    virtual DSS_GC  m_getDssDGCStatus();
    
    // ******************* Failure handlers ************************
    virtual void    m_siteStateChange(DSite *, const DSiteState&);

    virtual void m_undeliveredProxyMsg(DSite*, MessageType mtt, MsgContainer* msg);
    virtual void m_undeliveredCoordMsg(DSite*, MessageType mtt, MsgContainer* msg);
    virtual void m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 
    virtual void m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 
  };
  
  
  class ProxyMobile: public Proxy {
  private:
    SimpleQueue<MsgContainer*> a_unsentMsgs;
    DSite* a_coordSite; 
    int a_epoch; 
    bool a_fl_coordLost; 
  public:
    // ************** CONSTRUCTORS *******************
    ProxyMobile(NetIdentity ni, ProtocolProxy* const prot,
		DSS_Environment* const env); 
		
    ~ProxyMobile();

    virtual AccessArchitecture getAccessArchitecture() const {
      return AA_MOBILE_COORDINATOR; }

    virtual void m_initHomeProxy(Coordinator *m);
    virtual bool m_initRemoteProxy(DssReadBuffer *bs);
    
    virtual void m_makePersistent();
    
    // ************** Message Receivers  ********************
    
    virtual void  m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite);
    virtual void  m_receiveRefMsg( MsgContainer *msgC, DSite* fromsite);
    virtual void  m_receiveAsMsg(  MsgContainer *msgC, DSite* fromsite);
    
    // **************** GC ***************************
    
    virtual void  m_makeGCpreps();
    virtual char* m_stringrep();
    
    // ***************** REFERENCE *******************
    virtual void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    virtual void  m_mergeReferenceInfo(DssReadBuffer *bs);
    virtual DSS_GC getDssDGCStatus();
    
    // ************* Communication ********************

    virtual bool  m_sendToCoordinator(::MsgContainer* msg); 
    virtual bool  m_sendToProxy(DSite* dest, ::MsgContainer* msg); 
    virtual DSite* m_getCoordinatorSite(); 
    
    virtual ::MsgContainer *m_createProxyProtMsg();
    virtual ::MsgContainer *m_createProxyRefMsg();
    // ****************** FAILURES ************************
    
    virtual void m_siteStateChange(DSite *, const DSiteState&) ;
    virtual void m_undeliveredCoordMsg(DSite* dest, MessageType mtt,MsgContainer* msg);
    virtual void m_undeliveredProxyMsg(DSite* dest, MessageType mtt,MsgContainer* msg);
    virtual void m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg) ; 
    virtual void m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg) ; 
    

    // *************** INTERACTION *************************************
    virtual bool manipulateCNET(void* arg); 
    
    // **************** MOBILITY METHODS *******************
    void m_receivedNewCoordInfo(DSite* site, int epoch);
    // new virtual, called when the proxy looses contact with the coordinator. 
    virtual void m_locateCoordinator();
    
    void m_makeRemote(DSite*, int);

    
  };

  class BackboneService; 
  class LargeMessage; 
  
  BackboneService* gf_createMcBackbineST(LargeMessage*); 
  
}//End Namespace

#endif
