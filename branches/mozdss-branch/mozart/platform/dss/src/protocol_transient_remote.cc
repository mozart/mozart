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
  // Deregistration of proxy P (for current proxy: see in code)
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
  //      |----TR_REDIRECT--->|   if P* is remote
  //      |-----TR_BOUND----->|   if P* is the home proxy
  //
  // (3)  HP                  M                   P'
  //      |                   |----TR_REDIRECT--->|   (sent to others)
  //      |<-----TR_BOUND-----|                   |   if P* is remote
  //
  // In the best case, P* initiates the binding.  If there is no other
  // remote proxy, only one message is sent over the network.  The
  // protocol is optimal for that case.
  //
  // The message TR_BOUND does not carry any information.  It is used
  // by the manager and its proxy to notify each other when the
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
  //      |                     |<--TR_UPDATE_REPLY---|       |
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
  // (1)  P                 M                 P*
  //      |--PROT_PERMFAIL->|                 |
  //      |                 |--PROT_PERMFAIL->|
  //
  // (2)  P                 M                 P*      P'
  //      |                 |<-PROT_PERMFAIL--|       |
  //      |<-PROT_PERMFAIL--|                         |
  //      |                 |------PROT_PERMFAIL----->|
  //
  // The manager detects the failure of proxy P*.  The entity fails
  // everywhere if it was not bound yet.
  //
  //    M                   P
  //    |---PROT_PERMFAIL-->| (to all proxies)
  //
  //
  //
  // Message formats (optional parameters are between square brackets):
  //    TR_BIND AbsOp Pst [GlobalThread]
  //    TR_BOUND
  //    TR_REDIRECT Pst
  //    TR_UPDATE_REQUEST AbsOp Pst [GlobalThread]
  //    TR_UPDATE_REPLY AbsOp Pst [GlobalThread]
  //    TR_UPDATE AbsOp Pst [GlobalThread]

  namespace{
    // In the message descriptions, "PM" means a message sent by a
    // proxy to its manager, and so on.
    enum TR_msg_names{
      TR_BIND,           // PM,MP*: request to bind the transient
      TR_BOUND,          // HP<->M: transient bound, notify HP or manager
      TR_REDIRECT,       // MP,P*M: tell the binding
      TR_GETSTATUS,      // PM,MP*: get the status (bound or not)
      TR_RECEIVESTATUS,  // P*M,MP: answer to a getstatus
      TR_UPDATE_REQUEST, // PM,MP*: request to update
      TR_UPDATE_REPLY,   //    P*M: send update reply to manager
      TR_UPDATE          // P*M,MP: send update to proxie(s)
    };
    
    // type of registration
    enum TR_Reg_Type {
      TR_REG_AUTO,       // the proxy is registered
      TR_REG_MANUAL,     // the proxy must register manually
      TR_REG_TOKEN       // the proxy is registered, and has the token
    };
  }



  /******************** ProtocolTransientRemoteManager ********************/
  
  // normal constructor
  ProtocolTransientRemoteManager::
  ProtocolTransientRemoteManager(DSite* const s) : a_current(s) {
    setStatus(TRANS_STATUS_FREE);
    registerProxy(s);
  }

  // fill msg with manager migration info
  void ProtocolTransientRemoteManager::sendMigrateInfo(MsgContainer* msg) {
    ProtocolManager::sendMigrateInfo(msg);
    msg->pushDSiteVal(a_current);
    while (!a_requests.isEmpty()) {
      TR_request req = a_requests.pop();
      msgPush(msg, req.type);
      msgPush(msg, req.aop);
      msgPush(msg, req.pst);
      msgPush(msg, req.thr);
    }
  }

  // constructor called in case of migration
  ProtocolTransientRemoteManager::
  ProtocolTransientRemoteManager(MsgContainer* const msg) :
    ProtocolManager(msg), a_current(NULL) {
    a_current = msg->popDSiteVal();
    while (!msg->m_isEmpty()) {
      TR_request req = { msg->popIntVal(), msg->popIntVal(),
			 gf_popPstIn(msg)->loopBack2Out(), popThreadId(msg) };
      a_requests.append(req);
    }
  }

  void ProtocolTransientRemoteManager::makeGCpreps() {
    ProtocolManager::makeGCpreps();
    // mark all threads in buffered messages
    for (Position<TR_request> p(a_requests); p(); p++) (*p).makeGCpreps();
  }

  // register a remote proxy at s
  void ProtocolTransientRemoteManager::registerRemote(DSite *s) {
    registerProxy(s);

    // send an update for changes if necessary
    PstOutContainerInterface *ans;
    a_coordinator->m_doe(AO_OO_CHANGES, NULL, NULL, NULL, ans);
    if (ans != NULL) sendToProxy(s, TR_UPDATE, AO_OO_UPDATE, ans);
  }

  // register a remote proxy at s, and returns true if s is given the
  // write token
  bool ProtocolTransientRemoteManager::registerToken(DSite *s) {
    dssLog(DLL_BEHAVIOR,"TRAN REM (%p) RegisteringRemote:\nnew: %s\ncur: %s",
	   this, a_current->m_stringrep(), 
	   a_coordinator->m_getEnvironment()->a_myDSite->m_stringrep()); 

    // return immediately if the proxy is already registered
    if (isRegisteredProxy(s)) return false;

    registerRemote(s);

    // if the home proxy has the write token, we give it to s
    if (a_current == a_coordinator->m_getEnvironment()->a_myDSite) {
      setCurrent(s); return true;
    }
    return false;
  }

  // make s the current proxy, and forward unprocessed requests to it
  void ProtocolTransientRemoteManager::setCurrent(DSite* s) {
    DSite *mySite = a_coordinator->m_getEnvironment()->a_myDSite;
    Assert((a_current == mySite || s == mySite) && a_current != s);

    a_current = s;

    // update the status of the home proxy
    ProtocolProxy* pp = a_coordinator->m_getProxy()->m_getProtocol();
    static_cast<ProtocolTransientRemoteProxy*>(pp)->setToken(s == mySite);

    // forward buffered requests
    for (Position<TR_request> p(a_requests); p(); p++) {
      TR_request req = *p;
      if (req.type == PROT_PERMFAIL)
	sendToProxy(s, PROT_PERMFAIL);
      else if (req.thr)
	sendToProxy(s, req.type, req.aop, req.pst->duplicate(), req.thr);
      else
	sendToProxy(s, req.type, req.aop, req.pst->duplicate());
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
    case PROT_REGISTER: {
      if (isPermFail()) { sendToProxy(s, PROT_PERMFAIL); break; }
      if (getStatus() == TRANS_STATUS_BOUND) { sendRedirect(s); break; }
      if (!isRegisteredProxy(s)) registerRemote(s);
      break;
    }
    case PROT_DEREGISTER: {
      deregisterProxy(s);
      // if s is the current proxy, the home proxy gets the token back
      if (s == a_current && !isPermFail() && getStatus() < TRANS_STATUS_BOUND)
	setCurrent(a_coordinator->m_getEnvironment()->a_myDSite);
      break;
    }
    case TR_BIND: {
      // forward request to the proxy a_current (home or remote).
      dssLog(DLL_BEHAVIOR,"TRANSIENT REMOTE (%p): Received Bind",this);
      if (isPermFail() || getStatus() > TRANS_STATUS_FREE) break;
      setStatus(TRANS_STATUS_WAITING);
      int aop = msg->popIntVal();
      PstOutContainerInterface* arg = gf_popPstIn(msg)->loopBack2Out();
      GlobalThread *tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
      // enqueue request
      TR_request req = { msgType, aop, arg->duplicate(), tid };
      a_requests.append(req);
      // forward to current proxy
      if (tid) sendToProxy(a_current, TR_BIND, aop, arg, tid);
      else sendToProxy(a_current, TR_BIND, aop, arg);
      break;
    }
    case TR_REDIRECT: {
      // A remote proxy has bound the transient, so install the state...
      PstInContainerInterface *arg = gf_popPstIn(msg);
      a_coordinator->installEntityState(arg);
      // fall through
    }
    case TR_BOUND: {
      // This is either a bind coming from the home proxy, or it is an
      // indication that the entity instance at this site is fully
      // initiated, and that redirects can be sent.
      setStatus(TRANS_STATUS_BOUND);
      // the current proxy knows the binding
      deregisterProxy(a_current);
      // notify the home proxy if is not the current proxy
      DSite *mySite = a_coordinator->m_getEnvironment()->a_myDSite;
      if (a_current != mySite) {
	sendToProxy(mySite, TR_BOUND); deregisterProxy(mySite);
      }
      // send TR_REDIRECT to all other proxies
      while (!a_proxies.isEmpty()) sendRedirect(a_proxies.pop());
      // empty a_requests
      while (!a_requests.isEmpty()) a_requests.pop().dispose();
      break;
    }
    case TR_UPDATE_REQUEST: {
      // A remote proxy requests an update.
      if (isPermFail() || getStatus() > TRANS_STATUS_FREE) break;
      int aop = msg->popIntVal();
      PstOutContainerInterface *arg = gf_popPstIn(msg)->loopBack2Out();
      GlobalThread *tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
      // enqueue request
      TR_request req = { msgType, aop, arg->duplicate(), tid };
      a_requests.append(req);
      // forward to current proxy
      if (tid) sendToProxy(a_current, TR_UPDATE_REQUEST, aop, arg, tid);
      else sendToProxy(a_current, TR_UPDATE_REQUEST, aop, arg);
      break;
    }
    case TR_UPDATE_REPLY:
    case TR_UPDATE: {
      if (isPermFail() || getStatus() == TRANS_STATUS_BOUND) break;
      // The current proxy sends an update, simply forward it.
      Assert(s == a_current);
      int aop = msg->popIntVal();
      PstOutContainerInterface *ans = gf_popPstIn(msg)->loopBack2Out();
      GlobalThread *tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
      DSite *requester = (tid ? tid->m_getGUIdSite() : NULL);
      // send TR_UPDATE to all proxies except the requester
      for (Position<DSite*> p(a_proxies); p(); p++) {
	if ((*p) != requester && (*p) != a_current)
	  sendToProxy(*p, TR_UPDATE, aop, ans->duplicate());
      }
      // send specific TR_UPDATE to requester if present
      if (requester) sendToProxy(requester, TR_UPDATE, aop, ans, tid);
      else ans->dispose();
      // pop request if it is a reply
      if (msgType == TR_UPDATE_REPLY) a_requests.pop().dispose();
      break;
    }
    case PROT_PERMFAIL: {
      if (isPermFail() || getStatus() > TRANS_STATUS_FREE) break;
      if (s == a_current) {
	// empty a_requests
	while (!a_requests.isEmpty()) a_requests.pop().dispose();
	makePermFail();
      } else {
	setStatus(TRANS_STATUS_WAITING);
	// enqueue request
	TR_request req = { PROT_PERMFAIL, 0, NULL, NULL };
	a_requests.append(req);
	// forward to current proxy
	sendToProxy(a_current, PROT_PERMFAIL);
      }
      break;
    }
    default: 
      a_coordinator->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }

  // interpret a site failure
  void
  ProtocolTransientRemoteManager::m_siteStateChange(DSite* s,
						    const DSiteState& state) {
    if (isRegisteredProxy(s) && state >= DSite_GLOBAL_PRM) {
      deregisterProxy(s);
      if (s == a_current) makePermFail();
    }
  }



  /******************** ProtocolTransientRemoteProxy ********************/

  // constructor
  ProtocolTransientRemoteProxy::
  ProtocolTransientRemoteProxy(): ProtocolProxy(PN_TRANSIENT_REMOTE) {
    setStatus(TRANS_STATUS_FREE);
    setRegistered(true);     // home proxy is registered by manager
    setToken(true);          // home proxy has the token
  }

  // destructor
  ProtocolTransientRemoteProxy::~ProtocolTransientRemoteProxy(){
    Assert(a_susps.isEmpty());
    // deregister if this proxy is remote, and the transient is not
    // bound.  Don't care about the write token, the manager knows.
    if (getStatus() < TRANS_STATUS_WAITING && !a_proxy->m_isHomeProxy())
      protocol_Deregister();
  }

  // initiate a bind
  OpRetVal 
  ProtocolTransientRemoteProxy::
  protocol_Terminate(GlobalThread* const th_id,
		     PstOutContainerInterface**& msg, const AbsOp& aop) {
    if (isPermFail()) return DSS_RAISE;
    msg = NULL;   // default
    switch (getStatus()) {
    case TRANS_STATUS_FREE:
      if (hasToken()) { // we bind directly and notify the manager
	setStatus(TRANS_STATUS_BOUND);
	if (a_proxy->m_isHomeProxy())
	  sendToManager(TR_BOUND);
	else
	  sendToManager(TR_REDIRECT, UnboundPst(msg));
	return DSS_PROCEED; 
      }
      // send binding request to manager
      setStatus(TRANS_STATUS_WAITING);
      if (th_id) sendToManager(TR_BIND, aop, UnboundPst(msg), th_id);
      else sendToManager(TR_BIND, aop, UnboundPst(msg));
      // fall through
    case TRANS_STATUS_WAITING:
      // suspend the current thread until an answer comes back
      if (th_id) a_susps.append(th_id);
      return DSS_SUSPEND;
    default:
      return DSS_RAISE;
    }
  }

  // initiate an update
  OpRetVal 
  ProtocolTransientRemoteProxy::protocol_Update(GlobalThread* const th_id,
						PstOutContainerInterface**& msg,
						const AbsOp& aop) {
    if (isPermFail()) return DSS_RAISE;
    msg = NULL;   // default
    switch (getStatus()) {
    case TRANS_STATUS_FREE:
      if (hasToken()) {
	// this proxy makes the update, and notifies the manager
	sendToManager(TR_UPDATE, aop, UnboundPst(msg));
	return DSS_PROCEED;
      }
      // send update request to manager
      if (th_id) sendToManager(TR_UPDATE_REQUEST, aop, UnboundPst(msg), th_id);
      else sendToManager(TR_UPDATE_REQUEST, aop, UnboundPst(msg));
      // fall through
    case TRANS_STATUS_WAITING:
      // the update is useless because of the binding attempt, suspend
      if (th_id) a_susps.append(th_id);
      return DSS_SUSPEND;
    default:
      return DSS_RAISE;
    }
  }

  // kill the entity
  OpRetVal
  ProtocolTransientRemoteProxy::protocol_Kill() {
    if (!isPermFail() && getStatus() < TRANS_STATUS_WAITING) {
      setStatus(TRANS_STATUS_WAITING);
      ProtocolProxy::protocol_Kill();
      if (hasToken()) makePermFail(); // make it fail immediately
    }
    return DSS_SKIP;
  }

  // receive a message
  void
  ProtocolTransientRemoteProxy::msgReceived(::MsgContainer * msg, DSite*) {
    if (isPermFail() || getStatus() == TRANS_STATUS_BOUND) return;
    int msgType = msg->popIntVal();
    switch (msgType) {
    case TR_BIND: {
      if (!hasToken()) break;
      // this proxy has the write token, and is asked to bind
      AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
      PstInContainerInterface *arg = gf_popPstIn(msg);
      GlobalThread* tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
      PstOutContainerInterface* ans = NULL;
      a_proxy->m_doe(aop, tid, NULL, arg, ans);
      setStatus(TRANS_STATUS_BOUND);
      if (a_proxy->m_isHomeProxy()) sendToManager(TR_BOUND);
      else sendToManager(TR_REDIRECT, a_proxy->retrieveEntityState());
      break;
    }
    case TR_REDIRECT: {
      // sent to remote proxies only
      Assert(!a_proxy->m_isHomeProxy());
      PstInContainerInterface* cont = gf_popPstIn(msg);
      a_proxy->installEntityState(cont); 
      // fall through
    }
    case TR_BOUND: {
      Assert(msgType != TR_BOUND || a_proxy->m_isHomeProxy());
      // the state has been installed, simply wake up suspensions
      setStatus(TRANS_STATUS_BOUND);
      while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
      break;
    }
    case TR_UPDATE_REQUEST: {
      if (!hasToken()) break;
      // this proxy has the write token, and is asked to update
      AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
      PstInContainerInterface *arg = gf_popPstIn(msg);
      GlobalThread *tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
      PstOutContainerInterface *ans;
      a_proxy->m_doe(aop, tid, NULL, arg, ans);
      // send TR_UPDATE_REPLY to manager
      if (tid) sendToManager(TR_UPDATE_REPLY, aop, arg->loopBack2Out(), tid);
      else sendToManager(TR_UPDATE_REPLY, aop, arg->loopBack2Out());
      break;
    }
    case TR_UPDATE: {
      // do the update
      AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
      PstInContainerInterface* arg = gf_popPstIn(msg);
      PstOutContainerInterface* ans;
      a_proxy->m_doe(aop, NULL, NULL, arg, ans);
      // resume calling thread if present
      if (!msg->m_isEmpty()) {
	GlobalThread* tid = popThreadId(msg);
	a_susps.remove(tid);
	tid->resumeDoLocal(NULL);
      }
      break;
    }
    case PROT_PERMFAIL: {
      makePermFail();
      if (hasToken()) sendToManager(PROT_PERMFAIL);
      break;
    }
    default: 
      a_proxy->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }

  // marshal proxy information (autoregistration mechanism)
  bool
  ProtocolTransientRemoteProxy::marshal_protocol_info(DssWriteBuffer *buf,
						      DSite *dest) {
    if (dest && a_proxy->m_isHomeProxy()) {
      ProtocolManager* pm = a_proxy->a_coordinator->a_prot;
      if (static_cast<ProtocolTransientRemoteManager*>(pm)->registerToken(dest))
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
    Assert(!a_proxy->m_isHomeProxy() && isRegistered() && hasToken());
    switch (buf->getByte()) {
    case TR_REG_AUTO:
      setToken(false);
      break;
    case TR_REG_TOKEN:
      break; 
    case TR_REG_MANUAL:
      setToken(false);
      setRegistered(false);
      protocol_Register();
      break;
    }
    return true;
  }

  // interpret a site failure
  FaultState
  ProtocolTransientRemoteProxy::siteStateChanged(DSite* s,
						 const DSiteState& state) {
    if (getStatus() <= TRANS_STATUS_WAITING &&
	a_proxy->m_getCoordinatorSite() == s) {
      switch (state) {
      case DSite_OK:         return FS_PROT_STATE_OK;
      case DSite_TMP:        return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM:
      case DSite_LOCAL_PRM:  makePermFail(); return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }
  
} //end namespace
