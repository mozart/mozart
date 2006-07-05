/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog,     2004 
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

#ifndef __COORDINATOR_FWDCHAIN_HH
#define __COORDINATOR_FWDCHAIN_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "coordinator.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  enum CoordFwdChainStatus{
    MANAGER_STATUS_WAITING, // Requesting side waiting for migrate answer
    MANAGER_STATUS_PROT,    // Normal state
    MANAGER_STATUS_REF_COM, // In forwarding state, received messages are 
    // forwarded to the real manager
    MANAGER_STATUS_REF_INC  // Old Manager waits for commit. 
  };
  
  class ProxyFwdChain;
  
  class CoordinatorFwdChain: public Coordinator{
    friend class ProxyFwdChain;
  private:
    SimpleList<Pair<HomeReference*, int> >    a_refList;
    SimpleQueue<Pair<DSite*, MsgContainer*> > a_deliverQueue;
    DSite *a_coordPtr;     // the pointer is used when migrating.
  protected:
    CoordFwdChainStatus a_ms:2;
  private:
    void   m_queueProtMessage(MsgContainer *m,DSite* fromsite); 
    void   m_deliverProtMessages(DSite* dest);
    void   m_forwardMessage(MsgContainer* msgC ,DSite* fromsite, DSite *dest);

    void   m_sendMigratingState(DSite*);
    void   m_initiateMigration();
    void   m_setReceivingMigration();
    int    m_getEpoch();
    DSite *m_getCoordSite();

    void m_sendRefUpdateCoord(DSite* s);
    void m_sendRefUpdateProxy(DSite* s);
  public:

    
    
    CoordinatorFwdChain(ProtocolManager* const prot,
			const RCalg& gc_annot,
			DSS_Environment* const env);
    CoordinatorFwdChain(NetIdentity ni, ProxyFwdChain* const p,
		     DSS_Environment* const env);
    virtual ~CoordinatorFwdChain();
    
    virtual void m_initProxy(Proxy *p);
    
    // ******************* AS ***********************
    // The current ref manager is also the latest we know of..
    
    virtual ::MsgContainer *m_createProxyProtMsg();
    virtual ::MsgContainer *m_createProxyRefMsg();
    virtual bool m_sendToProxy(DSite* dest, MsgContainer* msg);
    // ****************** REFERENCE ****************

    // ***************** PROXY + AS ****************
    DSS_GC m_getDssDGCStatus();

    virtual void m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite);
    virtual void m_receiveRefMsg( MsgContainer *msgC, DSite* fromsite);
    virtual void m_receiveAsMsg(  MsgContainer *msgC, DSite* fromsite);

    void m_makeGCpreps();
    char *m_stringrep();
    // ******************  FAILURE ***************************
    
    virtual void    m_msgUnsent(DSite*s, MsgContainer* msg);
    virtual void    m_siteStateChange(DSite *, const DSiteState&);
    
    virtual void m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 
    virtual void m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 

  };


  class ProxyFwdChain: public Proxy {
    friend class CoordinatorFwdChain;
  private:
    DSite* a_coordSite; 
    RemoteReference* a_ref;
    int a_epoch; 
    ProxyFwdChain& operator=(const ProxyFwdChain&){ return *this; }
  private:
    int m_getEpoch();
    void _mergeReference(int epoch, DSite *coord, DssReadBuffer* bs);
    void m_updateRemote(DSite*); 
  public:

    
    
    // ************** CONSTRUCTORS ******************
    ProxyFwdChain(NetIdentity ni, ProtocolProxy* const prot,
		   DSS_Environment* const env);
    ~ProxyFwdChain();
    
    virtual void m_initHomeProxy(Coordinator *m);
    virtual bool m_initRemoteProxy(DssReadBuffer *bs);
    
    // ****************** GC *************************

    virtual void    m_makeGCpreps(); 
    
    // ******************** REFERNCE *****************************
    virtual void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    virtual void m_mergeReferenceInfo(DssReadBuffer *bs);
    virtual void m_makePersistent();
    
    // *******************  CNET **********************
    
    virtual bool  m_sendToCoordinator(::MsgContainer* msg);
    virtual DSite* m_getCoordinatorSite();

    virtual ::MsgContainer *m_createCoordProtMsg();
    virtual ::MsgContainer *m_createProxyProtMsg();
    virtual ::MsgContainer *m_createCoordRefMsg();
    virtual ::MsgContainer *m_createProxyRefMsg();
    
    
    // ************** PROXY + AS ********************
    void m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite);
    void m_receiveRefMsg( MsgContainer *msgC, DSite* fromsite);
    void m_receiveAsMsg(  MsgContainer *msgC, DSite* fromsite);

    // ***************** REFERENCE *******************

    void m_replaceReference(Reference* oldRR, Reference *newRR);

    char *m_stringrep();

    virtual DSS_GC getDssDGCStatus();
    // ***************** Failures *******************************
    virtual void    m_siteStateChange(DSite *, const DSiteState&);
    virtual void    m_msgUnsent(DSite*s, MsgContainer* msg); 
    
    virtual void m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 
    virtual void m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 

    // *************** INTERACTION *************************************
    virtual bool manipulateCNET(void* arg); 
    
  };

  
  // For migratory proxy we can only conclude that the manager is not there, however
  // the state might have been secured so only remove all the garbage ref-info
  // and wait for further knowledge (or need)
  

} //End Namespace
#endif
