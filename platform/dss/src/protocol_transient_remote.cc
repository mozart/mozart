/* 
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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
#pragma implementation "protocol_transient_remote.hh"
#endif

#include "protocol_transient_remote.hh"
namespace _dss_internal{ //Start namespace


  // Yes I know, I've keept the old shity names. 
  // It takes some time to figure them out, so 
  // I've given them a short descrition: 
  //
  namespace{
    enum TR_msg_names{
      TR_REGISTER,     // register a proxy at the manager
      TR_DEREGISTER,   // remove the registration at the manager
      TR_BIND,         // request to bind the transient (at manager or proxy)
      TR_BOUND,        // remote proxy has bound the transient
      TR_HOME_BOUND,   // home proxy tells to manager the transient is bound
      TR_REDIRECT,     // tell the binding to a proxy
      TR_ACKNOWLEDGE,  // sent to a proxy if a earlier surrender was succesfull
      TR_GETSTATUS,    // get the status (bound or not) of a Manager
      TR_RECEIVESTATUS // answer to a getstatus, received at a proxy.   
    };
    
    enum TR_Ref_Type{
      TR_RT_AUTO   = 0, // the proxy is already registered
      TR_RT_MANUAL = 1, // the proxy must register manually
      TR_RT_TOKEN  = 2  // the proxy is already registered, and has the token
    };
    
    // Assumes that the two netaddresses are not equal. 
    bool inline if_CompareNetAddress(Proxy* const a1, Proxy* const a2){
      return (a1->m_getNetId() <  a2->m_getNetId());
    }
  }
  
  void ProtocolTransientRemoteManager::sendRedirect(DSite *s){
    Assert(manager->m_getProxy() != NULL);
    MsgContainer *msgC = manager->m_createProxyProtMsg();
    msgC->pushIntVal(TR_REDIRECT);
    gf_pushPstOut(msgC,manager->retrieveEntityState());
    s->m_sendMsg(msgC);
  }
  
  ProtocolTransientRemoteManager::ProtocolTransientRemoteManager(DSite* const site):
    a_proxies(new OneContainer<DSite>(site,NULL)),
    a_bound(false), a_current(site)
  {
    // Created TransientRemote manager
  }

  ProtocolTransientRemoteManager::ProtocolTransientRemoteManager(::MsgContainer* const msgC):
    a_proxies(NULL), a_bound(false), a_current(NULL)
  {
    a_bound = ((msgC->popIntVal())!=0);
    a_current = msgC->popDSiteVal();
    for(int len = msgC->popIntVal(); len > 0 ; len --){
      a_proxies = new OneContainer<DSite>(msgC->popDSiteVal(),NULL);
    }
  }

  void ProtocolTransientRemoteManager::sendMigrateInfo(MsgContainer* msgC){
    int len=0; 
    msgC->pushIntVal(a_bound); 
    msgC->pushDSiteVal(a_current);
    OneContainer<DSite> *ptr = a_proxies;
    for(; ptr != NULL; ptr = ptr->a_next) len ++; 
    msgC->pushIntVal(len);
    for(ptr = a_proxies; ptr != NULL; ptr = ptr->a_next) msgC->pushDSiteVal(ptr->a_contain1);
  }


  void
  ProtocolTransientRemoteManager::makeGCpreps(){
    t_gcList(a_proxies);
  }
  
  bool
  ProtocolTransientRemoteManager::register_remote(DSite *dest){
    dssLog(DLL_BEHAVIOR,"TRAN REM (%p) RegisteringRemote:\nnew: %s\ncur: %s",
	   this, a_current->m_stringrep(), 
	   manager->m_getEnvironment()->a_myDSite->m_stringrep()); 
    if (a_current == manager->m_getEnvironment()->a_myDSite) {
      // the home proxy has the write token; we give it away
      a_current = dest; 
      dssLog(DLL_BEHAVIOR,"TRAN REM (%p)New dest %s", this, dest->m_stringrep()); 
      return true;
    } else {
      // register dest
      a_proxies = new OneContainer<DSite>(dest, a_proxies);
      return false;
    }
  }

  void
  ProtocolTransientRemoteManager::msgReceived(::MsgContainer * msg, DSite* s){
    int msgType = msg->popIntVal();
    switch(msgType){
    case TR_REGISTER: {
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received REGISTER %p",this,s);
      if (a_bound)
	sendRedirect(s);
      else
	a_proxies = new OneContainer<DSite>(s, a_proxies);
      break;
    }
    case TR_DEREGISTER: {
      // Remove proxy from list.  If the once_only is bound, the proxy
      // has already been removed.  This latecoming request to
      // unregister can then be droped.
      if (!a_bound) {
	dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received DEREGISTER %p\n",this,s);
	// remember: a_current is not a member of a_proxies.
	if (s == a_current) {
	  // the current proxy deregisters, the home becomes the current
	  a_current = manager->m_getEnvironment()->a_myDSite;
	  // the following is risky!
	  ProtocolProxy* p = manager->m_getProxy()->m_getProtocol();
	  static_cast<ProtocolTransientRemoteProxy*>(p)->a_writeToken = true;

	} else {
#ifdef DEBUG_CHECK
	  bool t = t_deleteCompare(&a_proxies, s);
	  Assert(t);
#else
	  t_deleteCompare(&a_proxies, s);
#endif
	}
      }
      break;
    }
    case TR_BIND: {
      // forward request to the proxy a_current (home or remote).
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received Surrender",this);
      PstInContainerInterface *builder = gf_popPstIn(msg);
      
      MsgContainer *msgC = manager->m_createProxyProtMsg();
      msgC->pushIntVal(TR_BIND); 
      gf_pushPstOut(msgC, builder->loopBack2Out());
      a_current->m_sendMsg(msgC); 
      break;
    }
    case TR_BOUND: {
      // A remote proxy has bound the transient, forward to home proxy.
      PstInContainerInterface *builder = gf_popPstIn(msg);
      MsgContainer *msgC = manager->m_createProxyProtMsg();
      msgC->pushIntVal(TR_BOUND); 
      gf_pushPstOut(msgC, builder->loopBack2Out());
      manager->m_getEnvironment()->a_myDSite->m_sendMsg(msgC); 
      break;
    }
    case TR_HOME_BOUND: {
      // This is either a bind coming from the HomeProxy, or it is an
      // indication that the entity instance at this site is fully
      // initiated, and that redirects can be sent.
      a_bound = true;
      // All proxies, except the home proxy and the current proxy, are
      // informed about the binding.
      DSite *mySite = manager->m_getEnvironment()->a_myDSite;
      while (a_proxies!=NULL) {
	OneContainer<DSite> *pl=a_proxies->a_next; 
	DSite *si = a_proxies->a_contain1;
	if (si!=mySite && si!=a_current) sendRedirect(si);
	delete a_proxies;
	a_proxies = pl;
      }
      break;
    }
    case TR_GETSTATUS: 
      manager->m_getEnvironment()->a_map->GL_error("Msg %d to variable not implemented yet", msgType);
      break;
    default: 
      manager->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }

  ProtocolTransientRemoteManager::~ProtocolTransientRemoteManager(){
    while(a_proxies){
      OneContainer<DSite> *nxt = a_proxies->a_next; 
      delete a_proxies; 
      a_proxies = nxt; 
    }
  }
  

  void
  ProtocolTransientRemoteProxy::msgReceived(::MsgContainer * msg, DSite*)
  {
    int msgType = msg->popIntVal();
    switch(msgType){
    case TR_ACKNOWLEDGE:
      dssError("Acknowledge not implemented yet, sorry");
      break; 

    case TR_BIND: {
      // this proxy has the write token, and is asked to bind
      Assert(a_writeToken);
      if (!a_bound) {
	a_bound = true; 
	PstInContainerInterface *cont = gf_popPstIn(msg);
	a_proxy->installEntityState(cont); 
	MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
	if (a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME) {
	  msgC->pushIntVal(TR_HOME_BOUND);
	} else {
	  msgC->pushIntVal(TR_BOUND);
	  gf_pushPstOut(msgC,a_proxy->retrieveEntityState());
	}
	a_proxy->m_sendToCoordinator(msgC);
      }
      break;
    }
    case TR_BOUND: {
      // the transient has been bound remotely, this is the home proxy
      Assert(a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME);
      a_bound = true;
      PstInContainerInterface* cont = gf_popPstIn(msg);
      a_proxy->installEntityState(cont);
      wkSuspThrs();
      MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(TR_HOME_BOUND);
      a_proxy->m_sendToCoordinator(msgC);
      break;
    }
    case TR_REDIRECT:{
      Assert(a_proxy->m_getProxyStatus() != PROXY_STATUS_HOME) ;
      a_bound = true;
      PstInContainerInterface* cont = gf_popPstIn(msg);
      a_proxy->installEntityState(cont); 
      wkSuspThrs();
      break;
    }
    case TR_RECEIVESTATUS:
      a_proxy->m_getEnvironment()->a_map->GL_error("Msg %d to variable not implemented yet", msgType);
      break;
    default: 
      a_proxy->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }
  
  OpRetVal 
  ProtocolTransientRemoteProxy::protocol_Terminate(GlobalThread* const th_id,
 ::PstOutContainerInterface**& msg,const AbsOp& aop)
  {
    if (a_writeToken) {
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      if (a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME) 
	msgC->pushIntVal(TR_HOME_BOUND); 
      else
	msgC->pushIntVal(TR_BOUND); 
      msg = gf_pushUnboundPstOut(msgC);
      a_proxy->m_sendToCoordinator(msgC);
      return DSS_PROCEED; 

    } else {
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(TR_BIND);
      msg = gf_pushUnboundPstOut(msgC);
      if (a_proxy->m_sendToCoordinator(msgC)) {
	dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send SURRENDER",this);
	a_susps = new OneContainer<GlobalThread>(th_id, a_susps);
	return DSS_SUSPEND;
      } else {
	msg = NULL;
	return DSS_RAISE;
      }
    }
  }

  OpRetVal 
  ProtocolTransientRemoteProxy::protocol_Update(GlobalThread* const th_id, ::PstOutContainerInterface**& msg,const AbsOp& aop){
    printf(" not imple remote update\n"); 
    Assert(0); 
    return (DSS_SUSPEND);
  }
  


  ProtocolTransientRemoteProxy::ProtocolTransientRemoteProxy():
    ProtocolProxy(PN_TRANSIENT_REMOTE), a_susps(NULL),
    a_bound(false), a_writeToken(false)
  {}
  
  bool
  ProtocolTransientRemoteProxy::m_initRemoteProt(DssReadBuffer* buf){
    // 
    // The HomeProxy is already registered. Only remotes has to
    // register themselves. The message will be created and issued
    // during unmarshaling, but due to the asynchrounous design of the
    // DSS it will be sent later.
    //
    if (a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME) {
      a_writeToken = true;
      return true;
    }
    switch(buf->getByte()) {
    case TR_RT_AUTO:
      break;
    case TR_RT_TOKEN:
      a_writeToken = true;
      break; 
    case TR_RT_MANUAL: {
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(TR_REGISTER);
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send REGISTER",this);
      a_proxy->m_sendToCoordinator(msgC);
      break;
    }
    }
    return true;
  }

  void
  ProtocolTransientRemoteProxy::makeGCpreps(){
    ;
  }


  ProtocolTransientRemoteProxy::~ProtocolTransientRemoteProxy(){
    if(!a_bound && a_proxy->m_getProxyStatus() == PROXY_STATUS_REMOTE) {
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(TR_DEREGISTER);
      // we add an extra argument, in order to release the write token
      msgC->pushIntVal(a_writeToken);
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send DEREGISTER", this);
      a_proxy->m_sendToCoordinator(msgC);
    }
  }


  void ProtocolTransientRemoteProxy::sendMigrateInfo(MsgContainer* msgC){
    if (a_bound){
      msgC->pushIntVal(true);
      gf_pushPstOut(msgC,a_proxy->retrieveEntityState()); 
    } else  {
      msgC->pushIntVal(false);
    }
  }

  void ProtocolTransientRemoteProxy::instantiateMigrateInfo(::MsgContainer* msg){
    if (msg->popIntVal())
      {
	// This was commented out before the restructuring. 
	Assert(0);
	//a_proxy->m_doe_clbk(PCB_OO_WRITE,  gf_popPstIn(msg));
	}
  }

  

  bool 
  ProtocolTransientRemoteProxy::marshal_protocol_info(DssWriteBuffer *buf, DSite *dest)
  {
    if (dest != NULL && a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME) {
      // anyway the write token is no longer at home
      a_writeToken = false;

      // This indirection is DANGEROUS!!!
      // Currently I'm playing with direct connection between the 
      // Proxy and the Manager.  Of course we could use messages
      // instead, it would then be necessary to guarantee that the 
      // internal message arrives before eventual remote messages.
      ProtocolTransientRemoteManager* pm =
	static_cast<ProtocolTransientRemoteManager*>(a_proxy->a_man->a_prot);

      if (pm->register_remote(dest))
	buf->putByte(TR_RT_TOKEN);
      else
	buf->putByte(TR_RT_AUTO);

    } else {
      buf->putByte(TR_RT_MANUAL);
    }
    return true;
  }
  
  bool 
  ProtocolTransientRemoteProxy::dispose_protocol_info(DssReadBuffer *buf )
  {
     buf->getByte();
     return true;
  }
  
  void ProtocolTransientRemoteProxy::wkSuspThrs() {
    OneContainer<GlobalThread>* tmp;
    while (a_susps) {
      // not really clear about this
      (a_susps->a_contain1)->resumeDoLocal(NULL);
      tmp = a_susps->a_next;
      delete a_susps;
      a_susps = tmp; 
    }
  }

  FaultState
  ProtocolTransientRemoteProxy::siteStateChanged(DSite* s,
						 const DSiteState& state)
  {
    // Note: currently we cannot diagnose a failure of the current proxy
    if (!a_bound && (a_proxy->m_getCoordinatorSite() == s)) {
      switch (state) {
      case DSite_OK:         return FS_PROT_STATE_OK;
      case DSite_TMP:        return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM:
      case DSite_LOCAL_PRM:  return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }
  
  
} //end namespace
