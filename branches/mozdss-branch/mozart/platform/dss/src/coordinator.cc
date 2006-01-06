/*  
 *  Authors:
 *    Zacharias El Banna, 2002 (zeb@sics.se)
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
 
#if defined(INTERFACE)
#pragma implementation "coordinator.hh"
#endif

#include "dssBase.hh"
#include "msl_serialize.hh"
#include "coordinator.hh"
#include "coordinator_stationary.hh"
#include "coordinator_fwdchain.hh"
#include "coordinator_mobile.hh"
#include "protocols.hh"


namespace _dss_internal{ //Start namespace

#ifdef DEBUG_CHECK
  int Coordinator::a_allocated=0;
  int Proxy::a_allocated=0;
#endif



  // *****************************************************************
  //
  // The AS-Node
  //

  AS_Node::~AS_Node(){}

  AS_Node::AS_Node(const AccessArchitecture& a, DSS_Environment* const env):
      NetIdNode(),
      DSS_Environment_Base(env),
      a_aa(a){
      // Created Resolver base node
    }
    
  AS_Node::AS_Node(NetIdentity ni, const AccessArchitecture& a,
	  DSS_Environment* const env):
    NetIdNode(ni),
    DSS_Environment_Base(env),
    a_aa(a){
    // Created Resolver base node
  }
  
  ::MsgContainer *
  AS_Node::m_createASMsg(const MessageType& mt){
    ::MsgContainer *msgC = m_getEnvironment()->a_msgnLayer->createAppSendMsgContainer();
    msgC->pushIntVal(mt); 
    gf_pushNetIdentity(msgC, m_getNetId());
    return msgC;
  }
  // ****************************** Coordinator ***********************************'
  
  Coordinator::Coordinator(const AccessArchitecture& a,
		   ProtocolManager* const p, DSS_Environment* const env):AS_Node(a,env), a_proxy(NULL), a_prot(p){
    DebugCode(a_allocated++);
    m_getEnvironment()->a_coordinatorTable->m_add(this);
  }


  Coordinator::Coordinator(NetIdentity ni, const AccessArchitecture& a,
		   ProtocolManager* const p, DSS_Environment* const env):AS_Node(ni, a,env), a_proxy(NULL), a_prot(p){
    DebugCode(a_allocated++);
    m_getEnvironment()->a_coordinatorTable->m_insert(this);
  }
  

  // Access structures has to delete the ref by themselves  
  Coordinator::~Coordinator(){
    DebugCode(a_allocated--);
    m_getEnvironment()->a_coordinatorTable->m_del(this); //free self and..
    delete a_prot;
  }
  
  
  AOcallback
  Coordinator::m_doe(const AbsOp& aop, DssThreadId* thid, DssOperationId* oId, PstInContainerInterface* builder, PstOutContainerInterface*& ans){
    return a_proxy->m_doe(aop, thid, oId,  builder, ans);
  }
  
  ::PstOutContainerInterface* 
  Coordinator::retrieveEntityState(){
    return a_proxy->retrieveEntityState();
  }
  
  void 
  Coordinator::installEntityState(PstInContainerInterface* builder){
    a_proxy->installEntityState(builder); 
  }

  // ******************* Failure handlers ************************
  void
  Coordinator::m_siteStateChange(DSite *, const DSiteState&){
    printf("site stat changed\n"); 
  }

  void  
  Coordinator::m_undeliveredCoordMsg(DSite* dest, MessageType mtt,MsgContainer* msg){
    delete msg; 
  }
  void  
  Coordinator::m_undeliveredProxyMsg(DSite* dest, MessageType mtt,MsgContainer* msg){
    delete msg; 
  }
  void  
  Coordinator::m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    delete msg; 
  }
  void  
  Coordinator::m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    delete msg; 
  }
  
  // ****************************************** PROXY *******************************************************
  int
  Proxy::sm_getMRsize(){
    //          Mgr                               +     epoch      +      len     +     type       +              algs             = 48
    printf("Lazy calculculation of proxy marshal size, a static 48\n");
    // DSite::sm_getMRsize()
    return ( 48 + sz_MNumberMax + sz_MNumberMax  +  sz_M8bitInt + 5 * sz_M8bitInt + (3 * sz_MNumberMax + sz_M8bitInt));
    

  }

  
  Proxy::Proxy(NetIdentity ni, const AccessArchitecture& a,
	       ProtocolProxy* const prot, AE_ProxyCallbackInterface* ae, DSS_Environment* const env):
    AS_Node(ni,a,env), a_ps(PROXY_STATUS_UNSET), a_currentFS(0),
    a_registeredFS(0), a_prot(prot), a_remoteRef(NULL), a_man(NULL),a_AbsEnt_Interface(ae) {
    DebugCode(a_allocated++);
    setFaultState(FS_NO_FAULT);
    setRegisteredFS(FS_NO_FAULT);
    m_getEnvironment()->a_proxyTable->m_insert(this);
  }

  void 
  Proxy::setFaultState(FaultState s){   
    dssLog(DLL_BEHAVIOR,"PROXY (%p): SetFaultState man:%p fs:%d",this,a_man,s); 
    a_currentFS = s;
  }
  
  // Access structures has to delete the ref by themselves
  Proxy::~Proxy(){
    DebugCode(a_allocated--);
    m_getEnvironment()->a_proxyTable->m_remove(this); //free self    
  }
  

  AOcallback
  Proxy::m_doe(const AbsOp& aop, DssThreadId* thid, DssOperationId* oId, PstInContainerInterface* builder, PstOutContainerInterface*& ans){
    return a_AbsEnt_Interface->applyAbstractOperation(aop, thid, oId, builder, ans);
  }
  
  ::PstOutContainerInterface* 
  Proxy::retrieveEntityState(){
    return a_AbsEnt_Interface->retrieveEntityState(); 
  }
  
  void 
  Proxy::installEntityState(PstInContainerInterface* builder){
    a_AbsEnt_Interface->installEntityState(builder); 
  }
  
  
  bool
  Proxy::clearWeakRoot(){
    return a_prot->clearWeakRoot();
  }
  
  bool
  Proxy::marshal(DssWriteBuffer *buf, const ProxyMarshalFlag& prf = PMF_ORDINARY){
    switch(prf){
    case PMF_ORDINARY:
    case PMF_PUSH: // ERIK: ZACHARIAS Should probably be handled outside of this. i.e in dss_accesbs
      if (m_getEnvironment()->m_getDestDSite() == NULL) {
	m_getEnvironment()->a_map->GL_warning("Called marshalProxy without destination");
	return true;
      };
      break;
    case PMF_FREE:
      dssLog(DLL_ALL,"PROXY (%p): Persistent (Free marshalling) proxy",this);
      m_makePersistent();
      break;
    default: Assert(0);
    }

    DSite* dest = m_getEnvironment()->a_msgnLayer->m_getDestDSite();
    
    // Bit saving schema:
    //        +-------------+-------------+-------------+-------------+
    // head = | access arch | proto name  |   ae name   | marshal flag|
    //        +-------------+-------------+-------------+-------------+
    Assert(AA_NBITS + PN_NBITS + AEN_NBITS + PMF_NBITS <= 16);
    unsigned int head = ((((m_getASname())
			   << PN_NBITS | m_getProtocol()->getProtocolName())
			  << AEN_NBITS | m_getAEname())
			 << PMF_NBITS | prf);

    ::gf_MarshalNumber(buf, head);             // 2 
    gf_marshalNetIdentity(buf, m_getNetId()); 
    m_getReferenceInfo(buf, dest);                 // 8 -> 48 (often ~10, WRC)

    
    //++++++ Returns true except when the protocol is immediate, when the whole node should be distributed.
    bool a = m_getProtocol()->marshal_protocol_info(buf, dest); 
      
     
    return a;
    // ----------------------------------------------------------------
    //                                         12 bytes (alot better than the old 78)
  }



  void Proxy::setRegisteredFS(const FaultState& s){
    // For now, we don't automaticallyt connect when we 
    // have a failure detection need on a particular site.
    //    m_getGUIdSite()->m_connect();
    a_registeredFS = s;
  }

  AE_ProxyCallbackInterface* 
  Proxy::getAEpki()
  {
    return a_AbsEnt_Interface;
  }


  // ******************* Failure handlers ************************
  void Proxy::m_siteStateChange(DSite *, const DSiteState&){
  }
  void Proxy::m_undeliveredCoordMsg(DSite* dest, MessageType mtt,MsgContainer* msg){
    delete msg; 
  }
  void Proxy::m_undeliveredProxyMsg(DSite* dest, MessageType mtt,MsgContainer* msg){
    delete msg; 
  }
  void Proxy::m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    delete msg; 
  }
  void Proxy::m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    delete msg; 
  }
  // ************* Communication ********************
  
  
  bool 
  Proxy:: m_sendToProxy(DSite* dest, ::MsgContainer* msg){
    return dest->m_sendMsg(msg); 
  }
  
  
  ::MsgContainer*
  Proxy::m_createCoordProtMsg(){
    return m_createASMsg(M_PROXY_COORD_PROTOCOL);
  }
  ::MsgContainer* 
  Proxy::m_createProxyProtMsg(){
    return m_createASMsg(M_PROXY_PROXY_PROTOCOL);
  }
  
  ::MsgContainer * 
  Proxy::m_createCoordRefMsg(){
    return m_createASMsg(M_PROXY_COORD_REF);
  }
  ::MsgContainer * 
  Proxy::m_createProxyRefMsg(){
    return m_createASMsg(M_PROXY_PROXY_REF);
  }
  
  


  // ***************************** CoordinatorTable ****************************
  // *
  // * - Only contains managers (and index), 
  // *   reference might be placed here later
  // *
  // ***********************************************************************

  void 
  CoordinatorTable::m_gcResources(){
    dssLog(DLL_BEHAVIOR,"******************** MANAGER TABLE: GC Resources (%d) ********************\n");
    for(NetIdNode *n = m_getNext(NULL) ; n != NULL; n = m_getNext(n)) {
      Coordinator *me = static_cast<Coordinator *>(n);
      if (me->m_getProxy() == NULL && me->m_getDssDGCStatus() == DSS_GC_LOCALIZE){ // Ok this one is a "single" manager, check it
	dssLog(DLL_DEBUG,"MANAGER TABLE: Cleaning up single manager %p",me);
	delete me;
      } else
	me->m_makeGCpreps();
    }
  }
  
  void
  CoordinatorTable::m_siteStateChange(DSite *s, const DSiteState& newState)
  {
    for(NetIdNode *n = m_getNext(NULL) ; n != NULL; n = m_getNext(n)) {
      static_cast<Coordinator *>(n)->m_siteStateChange(s,newState);
    }
  }
  
#ifdef DSS_LOG
  void
  CoordinatorTable::log_print_content(){
    dssLog(DLL_PRINT,"************************* MANAGER TABLE ************************");
    for(NetIdNode *n = m_getNext(NULL) ; n != NULL; n = m_getNext(n)) {
      dssLog(DLL_PRINT,"%p %s",n,static_cast<Coordinator *>(n)->m_stringrep());
    }
    dssLog(DLL_PRINT,"********************** MANAGER TABLE - DONE ********************");
  }
#endif
  
  // ***************************** ProxyTable *************************************
  // *
  // * - Gc is a single step and realized in proxy entries
  // *
  // ******************************************************************************
  
  void
  ProxyTable::m_siteStateChange(DSite *s, const DSiteState& newState){
    for(NetIdNode *n = m_getNext(NULL) ; n != NULL; n = m_getNext(n)) {
      static_cast<Proxy *>(n)->m_siteStateChange(s,newState); 
    }
  }
  
  
  void
  ProxyTable::m_gcResources(){
    for(NetIdNode *n = m_getNext(NULL) ; n != NULL; n = m_getNext(n)) {
      dssLog(DLL_BEHAVIOR,"******************** PROXY TABLE - GC RESOURCES () *********************");
      Proxy *pe = static_cast<Proxy *>(n);
#ifdef DEBUG_CHECK
      dssLog(DLL_DEBUG,"PROXY %p %s",pe,pe->m_stringrep());
#else
      dssLog(DLL_DEBUG,"PROXY %p",pe);
#endif
      pe->m_getGUIdSite()->m_makeGCpreps();
      pe->m_makeGCpreps();
    }
  }
  
#ifdef DSS_LOG
  void
  ProxyTable::log_print_content(){
    for(NetIdNode *n = m_getNext(NULL) ; n != NULL; n = m_getNext(n)) {
      dssLog(DLL_PRINT,"************************** PROXY TABLE () *************************");
      dssLog(DLL_PRINT,"%p %s",n,static_cast<Proxy *>(n)->m_stringrep());
    }
    dssLog(DLL_PRINT,"*********************** PROXY TABLE - DONE *********************");
  }
#endif


  
  
  Proxy* gf_createCoordinationProxy(AccessArchitecture type,
				    NetIdentity ni,
				    ProtocolProxy *prox, 
				    AE_ProxyCallbackInterface *aepc_interface, 
				    DSS_Environment* env){
    
    switch(type){
    case AA_STATIONARY_MANAGER: return  new ProxyStationary(ni,prox,	aepc_interface,  env);
    case AA_MIGRATORY_MANAGER:  return  new ProxyFwdChain(ni, prox, 	aepc_interface,  env);
    case AA_MOBILE_COORDINATOR: return  new ProxyMobile(ni, prox, 	aepc_interface,  env);
    default: Assert(0); 
    }
    return NULL;
  }
  
  Coordinator *gf_createCoordinator(AccessArchitecture type,
				    ProtocolManager *pman,
				    RCalg GC_annot,
				    DSS_Environment *env){
    switch(type){
    case AA_STATIONARY_MANAGER: return new CoordinatorStationary(pman,GC_annot,env);
    case AA_MIGRATORY_MANAGER:  return new CoordinatorFwdChain(pman,GC_annot, env);
    case AA_MOBILE_COORDINATOR: return new CoordinatorMobile(pman, GC_annot, env); 
    default:  Assert(0); 
    }
    return NULL;
  }


} //End namespace
