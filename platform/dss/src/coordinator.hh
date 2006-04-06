/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog,     2004
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

#ifndef __DSS_TABLE_HH
#define __DSS_TABLE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "bucketHashTable.hh"
#include "dssBase.hh"
#include "referenceConsistency.hh"
#include "abstractEntityImplementations.hh"
#include "dss_netId.hh"

namespace _dss_internal{ //Start namespace

  // Reminder: Coordination and protocol architecture
  //
  //                                         Abstract Entity
  //                                                ^
  //                                                |
  //                                                v
  //           Coordinator <--------------------> Proxy
  //              ^   ^                           ^   ^
  //              |   |                           |   |
  //              V   v                           v   v
  //  HomeReference  ProtocolManager  ProtocolProxy  RemoteReference
  //
  //       (on home site only)               (on all sites)



  // ********************************** AS_Node **************************************
  //
  // AS is an abstract base-class 
  //
  // 
  
  class AS_Node: public NetIdNode, public DSS_Environment_Base {
  public:
    const AccessArchitecture a_aa:20;
  protected:
    ::MsgContainer *m_createASMsg(const MessageType& mt);
  public:

    AS_Node(NetIdentity ni, const AccessArchitecture& a, DSS_Environment* const env);
    AS_Node(const AccessArchitecture& a, DSS_Environment* const env);
    
    virtual ~AS_Node()=0;
    
    inline AccessArchitecture m_getASname(){ return a_aa; }
    
  };
  
  // *********************************** Manager ************************************
  //
  // Manager is an abstract base-class
  //

  class Coordinator: public AS_Node {
    friend class CoordinatorTable;
  protected:
    Proxy*                 a_proxy;
    
  public:
    ProtocolManager*       a_prot;
    HomeReference*         a_homeRef; 
  private:
    Coordinator& operator=(const Coordinator&){ return *this; }
    Coordinator();
    Coordinator(const Coordinator&); 
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif

    // ************* CONSTRUCTORS *******************
    Coordinator(const AccessArchitecture& a,
		ProtocolManager* const prot, DSS_Environment* const env);

    Coordinator(NetIdentity ni, const AccessArchitecture& a,
		ProtocolManager* const prot, DSS_Environment* const env);
    virtual ~Coordinator();
    
    // Called when the a proxy is created for the coordinator(connects the coordinator to 
    // possible home proxy)
    virtual void    m_initProxy(Proxy *)=0;
    
    inline Proxy *m_getProxy() const { return a_proxy; }
    // ************* DOE Abstraction* ***************
    // used to shortcut the coordinator-proxy connection when at the same process. 
    AOcallback m_doe(const AbsOp& aop,  DssThreadId* thid, DssOperationId*, 
		     ::PstInContainerInterface* builder, 
		     ::PstOutContainerInterface*& );
    
    ::PstOutContainerInterface* retrieveEntityState();
    void installEntityState(::PstInContainerInterface*); 
    // ****************** Message Abstractions **********************
    
    virtual ::MsgContainer *m_createProxyProtMsg()= 0;
    virtual ::MsgContainer *m_createProxyRefMsg() = 0;
    
    virtual bool m_sendToProxy(DSite* dest, MsgContainer* msg) = 0; 
    
    // ****************** Message Receivers *************************
    
    virtual void    m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite)=0;
    virtual void    m_receiveRefMsg(MsgContainer *msgC, DSite* fromsite)=0;
    virtual void    m_receiveAsMsg(MsgContainer *msgC, DSite* sender)=0;

    
    // ******************* MISC *************************************

    virtual char   *m_stringrep()=0;
    
    //******************* GC routines ******************************
    virtual void    m_makeGCpreps()=0;
    virtual DSS_GC  m_getDssDGCStatus()=0;
    
    
    // ******************* Failure handlers ************************
    virtual void m_siteStateChange(DSite *, const DSiteState&);
    virtual void m_undeliveredCoordMsg(DSite* dest, MessageType mtt,MsgContainer* msg);
    virtual void m_undeliveredProxyMsg(DSite* dest, MessageType mtt,MsgContainer* msg); 
    virtual void m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 
    virtual void m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 

    // A note about failure handlers (raph).
    //
    // The current fault state of an entity is stored in its proxy.
    // It is important for a coordinator and its home proxy to be
    // consistent when they report fault states, especially for the
    // upper layers.  In order to ensure this consistency, I advise to
    // not update the fault state directly from the coordinator (or
    // protocol manager).  Instead the coordinator should send a
    // message to its proxy(ies), which will update the fault state.
  
  };

  
  // *********************************** Proxy *************************************
  //
  // Proxy is (as managers) an abstract base-class AND also an endpoint for the
  // Glue through ProxyInterface
  //


  class Proxy: public AS_Node, public ::CoordinatorAssistantInterface{
    friend class ProxyTable;
    friend class ProxyProtocol;
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
    
    static int sm_getMRsize();

  protected:
    ProxyStatus    a_ps:3;             // 3 needed for Windows!
    FaultState     a_currentFS   :FS_NBITS;
    FaultState     a_registeredFS:FS_NBITS;
    ProtocolProxy* a_prot;             // The execution protocol
    RemoteReference*     a_remoteRef;              // NULL indicates that the proxy shares reference with the coordinator. 
  public:
    Coordinator*               a_coordinator;   // For home proxies
    AE_ProxyCallbackInterface* a_AbsEnt_Interface;

  private:
    Proxy& operator=(const Proxy&){ return *this; }
    Proxy();
    Proxy(const Proxy&);
  public:

    // ***************** Constructors ***********************************
    
    Proxy(NetIdentity id, const AccessArchitecture& a,
	  ProtocolProxy* const prot, AE_ProxyCallbackInterface* ,DSS_Environment* const env);
    virtual ~Proxy();
    

    virtual void m_initHomeProxy(Coordinator *m)=0;
    virtual bool m_initRemoteProxy(DssReadBuffer *bs)=0;

    // ***************** Proxy Status ******************************************
    
    inline void m_setProxyStatus(const ProxyStatus& p){ a_ps = p; }
    inline ProxyStatus m_getProxyStatus(){  return a_ps; }
    inline bool m_isHomeProxy() { return a_ps == PROXY_STATUS_HOME; }
    
    
    // **************** Proxy Faults ****************

    virtual void        setRegisteredFS(const FaultState& s);
    virtual FaultState  getRegisteredFS() const {return a_registeredFS;}
    
    virtual FaultState  getFaultState() const { return a_currentFS;} 
    void    updateFaultState(FaultState fs); 
    
    
    // ***************** Marshal *********************
    // Marshaling    Flag = [ORDINARY,FREE, PUSH ?(i.e push)]
    virtual bool marshal(DssWriteBuffer* , const ProxyMarshalFlag&);
    inline AbstractEntityName   m_getAEname(){
      return a_AbsEnt_Interface->m_getName(); 
    }
    
    // ****************** GC **********************
    virtual void    m_makeGCpreps()=0;
    virtual bool    clearWeakRoot();
    
    // ****************** GET **********************
    inline ProtocolProxy    *m_getProtocol(){     return a_prot; }
    AE_ProxyCallbackInterface* getAEpki();
    
    // ************* DOE Abstraction* ***************
    
    AOcallback m_doe(const AbsOp& aop,  DssThreadId* thid, 
		     DssOperationId*, ::PstInContainerInterface* builder,
		     ::PstOutContainerInterface*& );
    
    ::PstOutContainerInterface* retrieveEntityState();
    void installEntityState(::PstInContainerInterface* builder);
    // **************** REFERENCE *******************

    virtual void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest) = 0; 
    virtual void m_mergeReferenceInfo(DssReadBuffer *bs) = 0;
    virtual void m_makePersistent() = 0; 
    
    
    // ************* Communication ********************
    virtual DSite* m_getCoordinatorSite() = 0; 
    
    virtual bool  m_sendToCoordinator(::MsgContainer* msg)= 0; 
    virtual bool  m_sendToProxy(DSite* dest, ::MsgContainer* msg);

    virtual ::MsgContainer *m_createCoordProtMsg();
    virtual ::MsgContainer *m_createProxyProtMsg();
    virtual ::MsgContainer *m_createCoordRefMsg();
    virtual ::MsgContainer *m_createProxyRefMsg();

    
    // ************* Message Receivers *****************
    
    virtual void    m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite)=0;
    virtual void    m_receiveRefMsg(MsgContainer *msgC, DSite* fromsite)=0;
    virtual void    m_receiveAsMsg(MsgContainer *msgC, DSite* sender)=0;

    // ***************** Failures *******************************
    virtual void m_siteStateChange(DSite *, const DSiteState&); 
    virtual void m_undeliveredCoordMsg(DSite* dest, MessageType mtt,MsgContainer* msg);
    virtual void m_undeliveredProxyMsg(DSite* dest, MessageType mtt,MsgContainer* msg); 
    virtual void m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 
    virtual void m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg); 
    
    
    
    // *************** MISC *************************************

    virtual char   *m_stringrep()=0;
    
  
    // *************** INTERACTION *************************************

    virtual bool manipulateRC(const RCalg& alg, const RCop& op, opaque& data){ 
      return a_remoteRef->m_manipulateRC(alg,op,data);
    }
    
    virtual bool manipulateCNET(void* arg) = 0; 
    
  };


  // ******************************** CoordinatorTable *********************************
  // *                                                                             *
  // *******************************************************************************
  
  class CoordinatorTable:private NetIdHT {
  public:
    CoordinatorTable(const int& sz, DSS_Environment* env):
      NetIdHT(sz, env){}
    
    inline void m_insert(Coordinator* man) {m_insertNetId(man); }
    inline void m_add(Coordinator*  man){ m_addNetId(man); }
    inline void m_del(Coordinator* man) { m_removeNetId(man); }

    inline Coordinator *m_find(NetIdentity ni){
      return static_cast<Coordinator *>(m_findNetId(ni));
    }
    
    void m_siteStateChange(DSite *, const DSiteState&);
    // ******************  GC ***********************
    void m_gcResources(); // And also cleanup to some extent
    
#ifdef DSS_LOG
    void log_print_content();
#endif
  };
  

  // ******************************* ProxyTable ************************************
  // *                                                                             *
  // *******************************************************************************
  
  class ProxyTable:private NetIdHT{
  public:
    ProxyTable(const int& sz, DSS_Environment* env):
      NetIdHT(sz, env){}
    
    inline void m_insert(Proxy* pxy) {m_insertNetId(pxy); }
    inline void m_remove(Proxy* const pxy) { m_removeNetId(pxy);  }

    inline Proxy *m_find(NetIdentity ni){
      return static_cast<Proxy *>(m_findNetId(ni));
    }
    
    void m_siteStateChange(DSite *, const DSiteState&);
    // ******************  GC ***********************
    void m_gcResources();
    
#ifdef DSS_LOG
    void log_print_content();
#endif
  };
  
  // Used for creating the different types of coordinators/proxies
   Proxy* gf_createCoordinationProxy(AccessArchitecture type,
				     NetIdentity ni,
				     ProtocolProxy *prox, 
				     AE_ProxyCallbackInterface *aepc_interface,
				     DSS_Environment* env);
  
  Coordinator *gf_createCoordinator(AccessArchitecture type,
				    ProtocolManager *pman,
				    RCalg GC_annot,
				    DSS_Environment *env);

} //End namespace
#endif
