/* 
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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

  // Quick description of the protocol.
  //
  // This protocol is a variant of the "once only" protocol (see
  // protocol_once_only.cc), where one of the proxies is responsible
  // for binding and updating the transient.  In the "once only"
  // protocol, this role was played by the manager.  The first remote
  // proxy automatically plays this role.
  //
  // In the following description, P* denotes the proxy that has the
  // write token, and HP denotes the "home" proxy.
  //
  // Registration of proxy P:
  //    P                 M
  //    |---TR_REGISTER-->|
  //    |<--TR_REDIRECT---|   replied only if transient already bound
  //
  // Deregistration of proxy P:
  //    P                   M
  //    |---TR_DEREGISTER-->|
  //
  // Proxy P wants to bind the transient.  If P is remote and does not
  // have the write token, go to step (1).  If P is remote and has the
  // write token, go to step (2).  Otherwise, go to step (3).
  //
  // (1)  P                   M                   P*
  //      |------TR_BIND----->|                   |
  //      |                   |------TR_BIND----->|
  //
  // (2)  P*                  M                   HP
  //      |-----TR_BOUND----->|                   |   (skip this step
  //      |                   |-----TR_BOUND----->|    if P*=HP)
  //
  // (3)  HP                  M                   P'
  //      |---TR_HOME_BOUND-->|                   |
  //      |                   |----TR_REDIRECT--->|   (sent to others)
  //
  // In the best case, P* initiates the binding.  If there is no other
  // remote proxy, only one message is sent over the network.  The
  // protocol is optimal for that case.
  //
  // Proxy P wants to update the transient.  This scheme is very
  // similar to the binding case; proxy P* serializes all updates.
  // The tricky part is that the manager must remind the proxies'
  // request in order to send the update confirmation to the right
  // proxy.
  //
  // (1)  P                     M                     P*
  //      |--TR_UPDATE_REQUEST->|                     |
  //      |                     |--TR_UPDATE_REQUEST->|
  //
  // (2)  P         P*                    M               P'
  //      |         |--TR_UPDATE_CONFIRM->|               |
  //      |         |                     |---TR_UPDATE-->|
  //      |<------TR_UPDATE_CONFIRM-------|               |
  //
  // Special case: P* initiates the update.  No confirmation is
  // required here, therefore P* sends a TR_UPDATE, which is forwarded
  // to other proxies by the manager.
  //
  // (1*) P*              M               P'
  //      |---TR_UPDATE-->|               |
  //      |               |---TR_UPDATE-->|   (sent to others)
  //
  // Like in the "once only" protocol, any binding or update request
  // received after the binding of the transient is ignored.

  namespace{
    // In the message descriptions, "PM" means a message sent by a
    // proxy to its manager, and so on.
    enum TR_msg_names{
      TR_REGISTER,       // PM: register a proxy at the manager
      TR_DEREGISTER,     // PM: remove the registration at the manager
      TR_BIND,           // PM,MP*: request to bind the transient
      TR_BOUND,          // P*M,MHP: remote proxy has bound the transient
      TR_HOME_BOUND,     // HPM: home proxy tells that transient is bound
      TR_REDIRECT,       // MP: tell the binding to proxies
      TR_GETSTATUS,      // PM,MP*: get the status (bound or not)
      TR_RECEIVESTATUS,  // P*M,MP: answer to a getstatus
      TR_UPDATE_REQUEST, // PM,MP*: request to update
      TR_UPDATE,         // P*M,MP: send update to proxie(s)
      TR_UPDATE_CONFIRM  // P*M,MP: update confirmation (to requesting proxy)
    };
    
    // type of registration
    enum TR_Reg_Type {
      TR_REG_AUTO   = 0, // the proxy is already registered
      TR_REG_MANUAL = 1, // the proxy must register manually
      TR_REG_TOKEN  = 2  // the proxy is already registered, and has the token
    };
  }



  /******************** ProtocolTransientRemoteManager ********************/
  
  // normal constructor
  ProtocolTransientRemoteManager::ProtocolTransientRemoteManager(DSite* const site) :
    a_proxies(new OneContainer<DSite>(site,NULL)),
    a_bound(false), a_current(site)
  {}

  // fill msgC with manager migration info
  void ProtocolTransientRemoteManager::sendMigrateInfo(MsgContainer* msgC) {
    OneContainer<DSite> *ptr = a_proxies;
    int len = 0;
    for (; ptr != NULL; ptr = ptr->a_next) len++;
    msgC->pushIntVal(a_bound);
    msgC->pushDSiteVal(a_current);
    msgC->pushIntVal(len);
    for (ptr = a_proxies; ptr != NULL; ptr = ptr->a_next)
      msgC->pushDSiteVal(ptr->a_contain1);
  }

  // constructor called in case of migration
  ProtocolTransientRemoteManager::ProtocolTransientRemoteManager(::MsgContainer* const msgC) :
    a_proxies(NULL), a_bound(false), a_current(NULL)
  {
    a_bound = ((msgC->popIntVal())!=0);
    a_current = msgC->popDSiteVal();
    for (int len = msgC->popIntVal(); len > 0 ; len--)
      a_proxies = new OneContainer<DSite>(msgC->popDSiteVal(), NULL);
  }

  // destructor
  ProtocolTransientRemoteManager::~ProtocolTransientRemoteManager() {
    t_deleteList(a_proxies);
  }

  // gc
  void ProtocolTransientRemoteManager::makeGCpreps() {
    t_gcList(a_proxies);
  }

  // register a remote proxy at dest, and returns true if dest is
  // given the write token
  bool ProtocolTransientRemoteManager::register_remote(DSite *dest) {
    dssLog(DLL_BEHAVIOR,"TRAN REM (%p) RegisteringRemote:\nnew: %s\ncur: %s",
	   this, a_current->m_stringrep(), 
	   manager->m_getEnvironment()->a_myDSite->m_stringrep()); 

    t_deleteCompare(&a_proxies, dest);   // ensures the invariant

    if (a_current == manager->m_getEnvironment()->a_myDSite) {
      // the home proxy has the write token; we give it away
      a_current = dest; 
      dssLog(DLL_BEHAVIOR,"TRAN REM (%p)New dest %s", this,
	     dest->m_stringrep()); 
      return true;
    } else {
      // register dest
      a_proxies = new OneContainer<DSite>(dest, a_proxies);
      return false;
    }
  }

  // send an TR_REDIRECT message to proxy at site s
  void ProtocolTransientRemoteManager::sendRedirect(DSite *s) {
    Assert(manager->m_getProxy() != NULL);
    MsgContainer *msgC = manager->m_createProxyProtMsg();
    msgC->pushIntVal(TR_REDIRECT);
    gf_pushPstOut(msgC,manager->retrieveEntityState());
    s->m_sendMsg(msgC);
  }

  // treat messages
  void
  ProtocolTransientRemoteManager::msgReceived(MsgContainer* msg, DSite* s) {
    int msgType = msg->popIntVal();
    switch (msgType) {
    case TR_REGISTER: {
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Received REGISTER %p",
	     this,s);
      if (a_bound)
	sendRedirect(s);
      else
	a_proxies = new OneContainer<DSite>(s, a_proxies);
      break;
    }
    case TR_DEREGISTER: {
      // Remove proxy from list.  If the once_only is bound, the proxy
      // has already been removed.  This latecoming request can thus
      // be dropped.
      if (a_bound) break;
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Received DEREGISTER %p\n",
	     this,s);
      // remember: a_current is not a member of a_proxies (if remote).
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
      break;
    }
    case TR_BIND: {
      // forward request to the proxy a_current (home or remote).
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Received Bind",this);
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
      // This is either a bind coming from the home proxy, or it is an
      // indication that the entity instance at this site is fully
      // initiated, and that redirects can be sent.
      a_bound = true;
      // All proxies, except the home proxy and the current proxy, are
      // informed about the binding.
      DSite *mySite = manager->m_getEnvironment()->a_myDSite;
      while (a_proxies!=NULL) {
	OneContainer<DSite> *next = a_proxies->a_next; 
	DSite *si = a_proxies->a_contain1;
	if (si!=mySite && si!=a_current) sendRedirect(si);
	delete a_proxies;
	a_proxies = next;
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



  /******************** ProtocolTransientRemoteProxy ********************/

  // constructor
  ProtocolTransientRemoteProxy::ProtocolTransientRemoteProxy():
    ProtocolProxy(PN_TRANSIENT_REMOTE), a_susps(NULL),
    a_bound(false), a_writeToken(false)
  {}

  // destructor
  ProtocolTransientRemoteProxy::~ProtocolTransientRemoteProxy(){
    Assert(a_susps == NULL);
    // deregister if this proxy is remote, and the transient is not
    // bound.  Don't care about the write token, the manager knows.
    if (!a_bound && a_proxy->m_getProxyStatus() == PROXY_STATUS_REMOTE) {
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(TR_DEREGISTER);
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Send DEREGISTER", this);
      a_proxy->m_sendToCoordinator(msgC);
    }
  }

  void
  ProtocolTransientRemoteProxy::makeGCpreps(){
    t_gcList(a_susps);
  }

  // resume all suspensions
  void ProtocolTransientRemoteProxy::wkSuspThrs() {
    OneContainer<GlobalThread>* next;
    while (a_susps) {
      // not really clear about this
      (a_susps->a_contain1)->resumeDoLocal(NULL);
      next = a_susps->a_next;
      delete a_susps;
      a_susps = next; 
    }
  }

  // initiate a bind
  OpRetVal 
  ProtocolTransientRemoteProxy::protocol_Terminate(GlobalThread* const th_id,
						   ::PstOutContainerInterface**& msg,
						   const AbsOp& aop)
  {
    if (a_bound) {
      msg = NULL; 
      return DSS_PROCEED; 
    }
    if (a_writeToken) {
      // this proxy binds the transient, and notifies the manager
      MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME ?
		       TR_HOME_BOUND : TR_BOUND);
      msg = gf_pushUnboundPstOut(msgC);
      a_proxy->m_sendToCoordinator(msgC);
      return DSS_PROCEED; 

    } else {
      // send binding request to manager
      MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(TR_BIND);
      msg = gf_pushUnboundPstOut(msgC);
      if (a_proxy->m_sendToCoordinator(msgC)) {
	dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Send Bind",this);
	a_susps = new OneContainer<GlobalThread>(th_id, a_susps);
	return DSS_SUSPEND;
      } else {
	msg = NULL;
	return DSS_RAISE;
      }
    }
  }

  // initiate an update
  OpRetVal 
  ProtocolTransientRemoteProxy::protocol_Update(GlobalThread* const th_id,
						::PstOutContainerInterface**& msg,
						const AbsOp& aop)
  {
    printf("not implemented yet\n"); 
    Assert(0); 
    return (DSS_SUSPEND);
  }

  // receive a message
  void
  ProtocolTransientRemoteProxy::msgReceived(::MsgContainer * msg, DSite*) {
    int msgType = msg->popIntVal();
    switch (msgType) {
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
    case TR_REDIRECT: {
      // sent to remote proxies only
      Assert(a_proxy->m_getProxyStatus() != PROXY_STATUS_HOME && !a_bound);
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

  // marshal proxy information (autoregistration mechanism)
  bool
  ProtocolTransientRemoteProxy::marshal_protocol_info(DssWriteBuffer *buf,
						      DSite *dest) {
    if (dest && a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME) {
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
	buf->putByte(TR_REG_TOKEN);
      else
	buf->putByte(TR_REG_AUTO);

    } else {
      buf->putByte(TR_REG_MANUAL);
    }
    return true;
  }
  
  bool 
  ProtocolTransientRemoteProxy::dispose_protocol_info(DssReadBuffer *buf) {
    buf->getByte();
    return true;
  }

  // initialize remote proxy (for registration)
  bool
  ProtocolTransientRemoteProxy::m_initRemoteProt(DssReadBuffer* buf) {
    Assert(a_proxy->m_getProxyStatus() == PROXY_STATUS_REMOTE);
    switch (buf->getByte()) {
    case TR_REG_AUTO:
      break;
    case TR_REG_TOKEN:
      a_writeToken = true;
      break; 
    case TR_REG_MANUAL: {
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(TR_REGISTER);
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Send REGISTER",this);
      a_proxy->m_sendToCoordinator(msgC);
      break;
    }
    }
    return true;
  }

  // interpret a site failure
  FaultState
  ProtocolTransientRemoteProxy::siteStateChanged(DSite* s,
						 const DSiteState& state) {
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
