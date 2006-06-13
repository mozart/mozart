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
  // protocol, this role was played by the manager.  Here the first
  // remote proxy automatically plays this role.
  //
  // In the following description, P* denotes the proxy that has the
  // write token, and HP denotes the "home" proxy.
  //
  // Registration of proxy P:
  //    P                 M
  //    |---TR_REGISTER-->|
  //    |<--TR_REDIRECT---|   if transient already bound
  // or:
  //    |<---TR_UPDATE----|   if changes are provided by pst
  //
  // Deregistration of proxy P:
  //    P                   M
  //    |---TR_DEREGISTER-->|
  //
  // Proxy P wants to bind the transient.  Skip step (1) if P has the
  // write token.
  //
  // (1)  P                   M                   P*
  //      |------TR_BIND----->|                   |
  //      |                   |------TR_BIND----->|
  //
  // (2)  P*                  M
  //      |-----TR_BOUND----->|   if P* is remote
  //      |---TR_HOME_BOUND-->|   if P* is the home proxy
  //
  // (3)  HP                  M                   P'
  //      |                   |----TR_REDIRECT--->|   (sent to others)
  //      |<--TR_HOME_BOUND---|                   |   if P* is remote
  //
  // In the best case, P* initiates the binding.  If there is no other
  // remote proxy, only one message is sent over the network.  The
  // protocol is optimal for that case.
  //
  // The message TR_HOME_BOUND does not carry any information.  It is
  // used by the manager and its proxy to notify each other when the
  // transient is bound on the home site.
  //
  // Proxy P wants to update the transient.  This scheme is very
  // similar to the binding case; proxy P* serializes all updates.
  // The manager sends the update confirmation to the requesting
  // proxy, and regular update messages to the other proxies.
  //
  // (1)  P                     M                     P*
  //      |--TR_UPDATE_REQUEST->|                     |
  //      |                     |--TR_UPDATE_REQUEST->|
  //
  // (2)  P                     M                     P*      P'
  //      |                     |<-----TR_UPDATE------|       |
  //      |<-----TR_UPDATE------|                             |
  //      |                     |----------TR_UPDATE--------->|
  //
  // Special case: P* initiates the update.  P* sends a TR_UPDATE,
  // which is directly forwarded to other proxies by the manager.
  //
  // (1*) P*              M               P'
  //      |---TR_UPDATE-->|               |
  //      |               |---TR_UPDATE-->|   (sent to others)
  //
  // Like in the "once only" protocol, any binding or update request
  // received after the binding of the transient is ignored.
  //
  // Proxy P makes the entity permfail.  Like in the update case, the
  // requests are serialized through proxy P*.  Step (2) only occurs
  // if the transient is not bound or failed yet.
  //
  // (1)  P               M               P*
  //      |--TR_PERMFAIL->|               |
  //      |               |--TR_PERMFAIL->|
  //
  // (2)  P               M               P*      P'
  //      |               |<-TR_PERMFAIL--|       |
  //      |<-TR_PERMFAIL--|                       |
  //      |               |------TR_PERMFAIL----->|
  //
  // The manager detects the failure of proxy P*.  The entity fails
  // everywhere if it was not bound yet.
  //
  //    M                 P
  //    |---TR_PERMFAIL-->| (to all proxies)
  //
  //
  //
  // Message formats (optional parameters are between square brackets):
  //    TR_BIND AbsOp Pst
  //    TR_BOUND Pst
  //    TR_HOME_BOUND
  //    TR_REDIRECT Pst
  //    TR_UPDATE_REQUEST AbsOp Pst [GlobalThread]
  //    TR_UPDATE AbsOp Pst [GlobalThread]

  namespace{
    // In the message descriptions, "PM" means a message sent by a
    // proxy to its manager, and so on.
    enum TR_msg_names{
      TR_REGISTER,       //     PM: register a proxy at the manager
      TR_DEREGISTER,     //     PM: remove the registration at the manager
      TR_BIND,           // PM,MP*: request to bind the transient
      TR_BOUND,          //    P*M: remote proxy has bound the transient
      TR_HOME_BOUND,     // HP<->M: transient bound, notify HP or manager
      TR_REDIRECT,       //     MP: tell the binding to proxies
      TR_GETSTATUS,      // PM,MP*: get the status (bound or not)
      TR_RECEIVESTATUS,  // P*M,MP: answer to a getstatus
      TR_UPDATE_REQUEST, // PM,MP*: request to update
      TR_UPDATE,         // P*M,MP: send update to proxie(s)
      TR_PERMFAIL        //  PM,MP: make the entity permfail
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
    a_proxies(), a_current(site), a_status(TRANS_STATUS_FREE) {
    a_proxies.push(site);
  }

  // fill msgC with manager migration info
  void ProtocolTransientRemoteManager::sendMigrateInfo(MsgContainer* msgC) {
    msgC->pushIntVal(a_status);
    msgC->pushDSiteVal(a_current);
    for (Position<DSite*> p(a_proxies); p(); p++) msgC->pushDSiteVal(*p);
  }

  // constructor called in case of migration
  ProtocolTransientRemoteManager::ProtocolTransientRemoteManager(::MsgContainer* const msgC) :
    a_proxies(), a_current(NULL)
  {
    a_status = static_cast<TransientStatus>(msgC->popIntVal());
    a_current = msgC->popDSiteVal();
    while (!msgC->m_isEmpty()) a_proxies.push(msgC->popDSiteVal());
  }

  // destructor
  ProtocolTransientRemoteManager::~ProtocolTransientRemoteManager() {}

  // gc
  void ProtocolTransientRemoteManager::makeGCpreps() {
    t_gcList(a_proxies);
  }

  // register a remote proxy at dest
  void ProtocolTransientRemoteManager::register_remote(DSite *dest) {
    // simply add dest to a_proxies
    a_proxies.push(dest);

    // send an update for changes if necessary
    PstOutContainerInterface *ans;
    a_coordinator->m_doe(AO_OO_CHANGES, NULL, NULL, NULL, ans);
    if (ans != NULL) sendToProxy(dest, TR_UPDATE, AO_OO_UPDATE, ans);
  }

  // register a remote proxy at dest, and returns true if dest is
  // given the write token
  bool ProtocolTransientRemoteManager::register_token(DSite *dest) {
    dssLog(DLL_BEHAVIOR,"TRAN REM (%p) RegisteringRemote:\nnew: %s\ncur: %s",
	   this, a_current->m_stringrep(), 
	   a_coordinator->m_getEnvironment()->a_myDSite->m_stringrep()); 

    // return immediately if the proxy is already registered
    if (dest == a_current || a_proxies.contains(dest)) return false;

    // the proxy is not registered yet
    if (a_current == a_coordinator->m_getEnvironment()->a_myDSite) {
      // the home proxy has the write token; we give it away
      a_current = dest; 
      dssLog(DLL_BEHAVIOR,"TRAN REM (%p)New dest %s", this,
	     dest->m_stringrep()); 
      return true;
    } else {
      register_remote(dest);
      return false;
    }
  }

  // send an TR_REDIRECT message to proxy at site s
  void ProtocolTransientRemoteManager::sendRedirect(DSite *s) {
    Assert(a_coordinator->m_getProxy() != NULL);
    sendToProxy(s, TR_REDIRECT, a_coordinator->retrieveEntityState());
  }

  // treat messages
  void
  ProtocolTransientRemoteManager::msgReceived(MsgContainer* msg, DSite* s) {
    int msgType = msg->popIntVal();
    switch (msgType) {
    case TR_REGISTER: {
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Received REGISTER %p",
	     this,s);
      switch (a_status) {
      case TRANS_STATUS_BOUND:  sendRedirect(s); break;
      case TRANS_STATUS_FAILED: sendToProxy(s, TR_PERMFAIL); break;
      default:                  register_remote(s); break;
      }
      break;
    }
    case TR_DEREGISTER: {
      // Remove proxy from list.  If the once_only is bound, the proxy
      // has already been removed.  This latecoming request can thus
      // be dropped.
      if (a_status > TRANS_STATUS_WAITING) break;
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Received DEREGISTER %p\n",
	     this,s);
      // remember: a_current is not a member of a_proxies (if remote).
      if (s == a_current) {
	// the current proxy deregisters, the home becomes the current
	a_current = a_coordinator->m_getEnvironment()->a_myDSite;
	// the following is risky!
	ProtocolProxy* p = a_coordinator->m_getProxy()->m_getProtocol();
	static_cast<ProtocolTransientRemoteProxy*>(p)->a_writeToken = true;
	
      } else {
#ifdef DEBUG_CHECK
	bool t = a_proxies.remove(s);
	Assert(t);
#else
	a_proxies.remove(s);
#endif
      }
      break;
    }
    case TR_BIND: {
      // forward request to the proxy a_current (home or remote).
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Received Bind",this);
      if (a_status > TRANS_STATUS_FREE) break;
      a_status = TRANS_STATUS_WAITING;
      PstInContainerInterface *builder = gf_popPstIn(msg);
      sendToProxy(a_current, TR_BIND, builder->loopBack2Out());
      break;
    }
    case TR_BOUND: {
      // A remote proxy has bound the transient, so install the state...
      PstInContainerInterface *builder = gf_popPstIn(msg);
      a_coordinator->installEntityState(builder);
      // fall through
    }
    case TR_HOME_BOUND: {
      // This is either a bind coming from the home proxy, or it is an
      // indication that the entity instance at this site is fully
      // initiated, and that redirects can be sent.
      a_status = TRANS_STATUS_BOUND;
      // All proxies, except the home proxy and the current proxy, are
      // informed about the binding.
      DSite *mySite = a_coordinator->m_getEnvironment()->a_myDSite;
      while (!a_proxies.isEmpty()) {
	DSite *si = a_proxies.pop();
	if (si == mySite) {
	  // Notify the home proxy that the transient has been bound.
	  if (msgType == TR_BOUND) sendToProxy(si, TR_HOME_BOUND);
	} else {
	  Assert(si != a_current);
	  sendRedirect(si);
	}
      }
      break;
    }
    case TR_UPDATE_REQUEST: {
      // A remote proxy requests an update.
      if (a_status > TRANS_STATUS_FREE) break;
      int aop = msg->popIntVal();
      PstInContainerInterface *builder = gf_popPstIn(msg);
      PstOutContainerInterface *arg = builder->loopBack2Out();
      // Forward the request to the current proxy.
      if (!msg->m_isEmpty())
	sendToProxy(a_current, TR_UPDATE_REQUEST, aop, arg, popThreadId(msg));
      else
	sendToProxy(a_current, TR_UPDATE_REQUEST, aop, arg);
      break;
    }
    case TR_UPDATE: {
      // The current proxy sends an update, simply forward it.
      if (a_status > TRANS_STATUS_WAITING) break;
      Assert(s == a_current);
      int aop = msg->popIntVal();
      PstInContainerInterface *builder = gf_popPstIn(msg);
      PstOutContainerInterface *ans = builder->loopBack2Out();
      // take requester if present
      GlobalThread *tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
      DSite *requester = (tid ? tid->m_getGUIdSite() : NULL);
      // send TR_UPDATE to all proxies except the requester
      for (Position<DSite*> p(a_proxies); p(); p++) {
	if ((*p) != requester && (*p) != a_current)
	  sendToProxy(*p, TR_UPDATE, aop, ans->duplicate());
      }
      // send specific TR_UPDATE to requester if present
      if (requester)
	sendToProxy(requester, TR_UPDATE, aop, ans, tid);
      else
	delete ans;
      break;
    }
    case TR_PERMFAIL: {
      if (a_status > TRANS_STATUS_WAITING) break;
      if (s == a_current) { // The entity failed, forward to all proxies
	a_status = TRANS_STATUS_FAILED;
	while (!a_proxies.isEmpty())
	  sendToProxy(a_proxies.pop(), TR_PERMFAIL);
      } else { // Forward to current proxy
	sendToProxy(a_current, TR_PERMFAIL);
      }
      break;
    }
    case TR_GETSTATUS: 
      a_coordinator->m_getEnvironment()->a_map->GL_error("Msg %d to variable not implemented yet", msgType);
      break;
    default: 
      a_coordinator->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }

  // interpret a site failure
  void
  ProtocolTransientRemoteManager::m_siteStateChange(DSite* s,
						    const DSiteState& state) {
    if (a_status <= TRANS_STATUS_WAITING &&
	(state == DSite_GLOBAL_PRM || state == DSite_LOCAL_PRM)) {
      if (s == a_current) { // the current proxy failed, permfail!
	a_status = TRANS_STATUS_FAILED;
	while (!a_proxies.isEmpty())
	  sendToProxy(a_proxies.pop(), TR_PERMFAIL);
      } else { // simply remove it from a_proxies if present
	a_proxies.remove(s);
      }
    }
  }



  /******************** ProtocolTransientRemoteProxy ********************/

  // constructor
  ProtocolTransientRemoteProxy::ProtocolTransientRemoteProxy():
    ProtocolProxy(PN_TRANSIENT_REMOTE), a_susps(),
    a_status(TRANS_STATUS_FREE), a_writeToken(false)
  {}

  // destructor
  ProtocolTransientRemoteProxy::~ProtocolTransientRemoteProxy(){
    Assert(a_susps.isEmpty());
    // deregister if this proxy is remote, and the transient is not
    // bound.  Don't care about the write token, the manager knows.
    if (a_status < TRANS_STATUS_WAITING && !a_proxy->m_isHomeProxy()) {
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Send DEREGISTER", this);
      sendToManager(TR_DEREGISTER);
    }
  }

  void
  ProtocolTransientRemoteProxy::makeGCpreps(){
    t_gcList(a_susps);
  }

  // resume all suspensions
  void ProtocolTransientRemoteProxy::wkSuspThrs() {
    // not really clear about the resumeDoLocal
    while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
  }

  // notify failure
  void ProtocolTransientRemoteProxy::m_failed() {
    a_status = TRANS_STATUS_FAILED;
    a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
    // resume operations
    while (!a_susps.isEmpty()) a_susps.pop()->resumeFailed();
  }

  // initiate a bind
  OpRetVal 
  ProtocolTransientRemoteProxy::protocol_Terminate(GlobalThread* const th_id,
						   ::PstOutContainerInterface**& msg,
						   const AbsOp& aop)
  {
    msg = NULL;   // default
    switch (a_status) {
    case TRANS_STATUS_FREE:
      if (a_writeToken) { // we directly bind and notify the manager
	a_status = TRANS_STATUS_BOUND;
	if (a_proxy->m_isHomeProxy())
	  sendToManager(TR_HOME_BOUND);
	else
	  sendToManager(TR_BOUND, UnboundPst(msg));
	return DSS_PROCEED; 
      }
      // send binding request to manager
      a_status = TRANS_STATUS_WAITING;
      sendToManager(TR_BIND, UnboundPst(msg));
      // fall through
    case TRANS_STATUS_WAITING:
      // suspend the current thread until an answer comes back
      if (th_id) a_susps.push(th_id);
      return DSS_SUSPEND;
    default:
      return DSS_RAISE;
    }
  }

  // initiate an update
  OpRetVal 
  ProtocolTransientRemoteProxy::protocol_Update(GlobalThread* const th_id,
						::PstOutContainerInterface**& msg,
						const AbsOp& aop)
  {
    msg = NULL;   // default
    switch (a_status) {
    case TRANS_STATUS_FREE:
      if (a_writeToken) {
	// this proxy makes the update, and notifies the manager
	sendToManager(TR_UPDATE, aop, UnboundPst(msg));
	return DSS_PROCEED;
      }
      // send update request to manager
      if (th_id)
	sendToManager(TR_UPDATE_REQUEST, aop, UnboundPst(msg), th_id);
      else
	sendToManager(TR_UPDATE_REQUEST, aop, UnboundPst(msg));
      // fall through
    case TRANS_STATUS_WAITING:
      // the update is useless because of the binding attempt, suspend
      if (th_id) a_susps.push(th_id);
      return DSS_SUSPEND;
    default:
      return DSS_RAISE;
    }
  }

  // kill the entity
  OpRetVal
  ProtocolTransientRemoteProxy::protocol_Kill() {
    if (a_status < TRANS_STATUS_WAITING) {
      a_status = TRANS_STATUS_WAITING;
      if (a_writeToken) { // make it fail immediately
	a_status = TRANS_STATUS_FAILED;
	a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
      }
      sendToManager(TR_PERMFAIL);
    }
    return DSS_SKIP;
  }

  // receive a message
  void
  ProtocolTransientRemoteProxy::msgReceived(::MsgContainer * msg, DSite*) {
    if (a_status == TRANS_STATUS_FAILED) return;
    int msgType = msg->popIntVal();
    switch (msgType) {
    case TR_BIND: {
      // this proxy has the write token, and is asked to bind
      Assert(a_writeToken);
      if (a_status < TRANS_STATUS_BOUND) {
	a_status = TRANS_STATUS_BOUND; 
	PstInContainerInterface *cont = gf_popPstIn(msg);
	a_proxy->installEntityState(cont); 
	if (a_proxy->m_isHomeProxy())
	  sendToManager(TR_HOME_BOUND);
	else
	  sendToManager(TR_BOUND, a_proxy->retrieveEntityState());
      }
      break;
    }
    case TR_HOME_BOUND: {
      // the transient has been bound remotely, this is the home proxy
      Assert(a_proxy->m_isHomeProxy());
      // the state has been installed by the manager, simply wake up
      // suspensions
      a_status = TRANS_STATUS_BOUND;
      wkSuspThrs();
      break;
    }
    case TR_REDIRECT: {
      // sent to remote proxies only
      Assert(!a_proxy->m_isHomeProxy() && a_status < TRANS_STATUS_BOUND);
      a_status = TRANS_STATUS_BOUND;
      PstInContainerInterface* cont = gf_popPstIn(msg);
      a_proxy->installEntityState(cont); 
      wkSuspThrs();
      break;
    }
    case TR_UPDATE_REQUEST: {
      // this proxy has the write token, and is asked to update
      Assert(a_writeToken);
      if (a_status < TRANS_STATUS_BOUND) {
	AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
	PstInContainerInterface *builder = gf_popPstIn(msg);
	GlobalThread *tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
	// do the update
	PstOutContainerInterface *ans;
	a_proxy->m_doe(aop, tid, NULL, builder, ans);
	// send TR_UPDATE to manager
	if (tid)
	  sendToManager(TR_UPDATE, aop, builder->loopBack2Out(), tid);
	else
	  sendToManager(TR_UPDATE, aop, builder->loopBack2Out());
      }
      break;
    }
    case TR_UPDATE: {
      // do the update
      AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
      PstInContainerInterface* builder = gf_popPstIn(msg);
      PstOutContainerInterface* ans;
      a_proxy->m_doe(aop, NULL, NULL, builder, ans);
      // resume calling thread if present
      if (!msg->m_isEmpty()) {
	GlobalThread* tid = popThreadId(msg);
	a_susps.remove(tid);
	tid->resumeDoLocal(NULL);
      }
      break;
    }
    case TR_PERMFAIL: {
      m_failed();
      if (a_writeToken) sendToManager(TR_PERMFAIL);
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
    if (dest && a_proxy->m_isHomeProxy()) {
      // anyway the write token is no longer at home
      a_writeToken = false;

      // This indirection is DANGEROUS!!!
      // Currently I'm playing with direct connection between the 
      // Proxy and the Manager.  Of course we could use messages
      // instead, it would then be necessary to guarantee that the 
      // internal message arrives before eventual remote messages.
      ProtocolTransientRemoteManager* pm =
	static_cast<ProtocolTransientRemoteManager*>(a_proxy->a_coordinator->a_prot);

      if (pm->register_token(dest))
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
    Assert(!a_proxy->m_isHomeProxy());
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
    if (a_status <= TRANS_STATUS_WAITING &&
	a_proxy->m_getCoordinatorSite() == s) {
      switch (state) {
      case DSite_OK:         return FS_PROT_STATE_OK;
      case DSite_TMP:        return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM:
      case DSite_LOCAL_PRM:  m_failed(); return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }
  
} //end namespace
