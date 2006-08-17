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
#pragma implementation "protocol_once_only.hh"
#endif

#include "protocol_once_only.hh"

namespace _dss_internal{ //Start namespace

  // Quick description of the protocol.
  //
  // Registration of proxy P:
  //    P                   M
  //    |---PROT_REGISTER-->|
  //    |<---OO_REDIRECT----|   if transient already bound
  // or:
  //    |<----OO_UPDATE-----|   if changes are provided by pst
  // or:
  //    |<--PROT_PERMFAIL---|   if entity is permfail
  //
  // Deregistration of proxy P:
  //    P                     M
  //    |---PROT_DEREGISTER-->|
  //
  // Proxy P binds transient (not bound yet):
  //    P                 M                 P'
  //    |-----OO_BIND---->|                 |
  //    |<--OO_REDIRECT---|---OO_REDIRECT-->|   (sent to all proxies)
  //
  // Proxy P sends an update (transient not bound yet):
  //    P                       M                       P'
  //    |---OO_UPDATE_REQUEST-->|                       |
  //    |<------OO_UPDATE-------|-------OO_UPDATE------>|   (sent to all)
  //
  // Any message OO_BIND or OO_UPDATE_REQUEST received after the
  // binding of the transient is ignored.  The proxy will eventually
  // receive the OO_REDIRECT message (or possibly PROT_PERMFAIL).
  //
  // Note.  The message OO_UPDATE sent back to P may contain a thread
  // id to wakeup the requesting thread.  We do not need this in the
  // binding case, since OO_REDIRECT wakes up all suspended threads.
  //
  // Note.  For the sake of consistency, the entity cannot fail after
  // it has been bound.  Indeed, a proxy that sends a PROT_PERMFAIL
  // message in that case will eventually receive the OO_REDIRECT
  // message.  Conceptually, the binding makes the transient
  // disappear; it can therefore no longer fail.
  //
  //
  //
  // Message formats (optional parameters are between square brackets):
  //    OO_BIND AbsOp Pst [GlobalThread]
  //    OO_REDIRECT Pst
  //    OO_UPDATE_REQUEST AbsOp Pst [GlobalThread]
  //    OO_UPDATE AbsOp Pst [GlobalThread]

  namespace{
    // In the message descriptions, "PM" means a message sent by a
    // proxy to its manager, and so on.
    enum OO_msg_names{
      OO_BIND,           // PM: try to bind the once only Manager
      OO_REDIRECT,       // MP: tell the binding to proxies
      OO_GETSTATUS,      // PM: get the status (bound or not) of a Manager
      OO_RECEIVESTATUS,  // MP: answer to a getstatus
      OO_UPDATE_REQUEST, // PM: request to update
      OO_UPDATE          // MP: send update to proxie(s)
    };

    // type of registration
    enum OO_Reg_Type {
      OO_REG_AUTO   = 0, // the proxy is already registered
      OO_REG_MANUAL = 1  // the proxy must register manually
    };
  }

  // whether autoregistration must be used
#define AUTOREGISTRATION



  /******************** ProtocolOnceOnlyManager ********************/

  // normal constructor  
  ProtocolOnceOnlyManager::ProtocolOnceOnlyManager(DSite* const site) {
    setStatus(TRANS_STATUS_FREE);
    registerProxy(site);
  }

  // register a remote proxy
  void ProtocolOnceOnlyManager::registerRemote(DSite* s) {
    if (isRegisteredProxy(s)) return;

    registerProxy(s);   // inserted in a_proxies

    // send an update for changes if necessary
    ::PstOutContainerInterface *ans;
    a_coordinator->m_doe(AO_OO_CHANGES, NULL, NULL, NULL, ans);
    if (ans != NULL) sendToProxy(s, OO_UPDATE, AO_OO_UPDATE, ans);
  }

  // send an OO_REDIRECT message to proxy at site s
  void ProtocolOnceOnlyManager::sendRedirect(DSite *s) {
    Assert(a_coordinator->m_getProxy() != NULL);
    sendToProxy(s, OO_REDIRECT, a_coordinator->retrieveEntityState());
  }

  // treat messages  
  void
  ProtocolOnceOnlyManager::msgReceived(MsgContainer * msg, DSite* s) {
    int msgType = msg->popIntVal();
    switch (msgType) {
    case PROT_REGISTER: {
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received REGISTER %p",this,s);
      if (isPermFail()) { sendToProxy(s, PROT_PERMFAIL); break; }
      if (getStatus() == TRANS_STATUS_BOUND) { sendRedirect(s); break; }
      registerRemote(s);
      break;
    }
    case PROT_DEREGISTER: {
      deregisterProxy(s);
      break;
    }
    case PROT_PERMFAIL: {
      if (isPermFail() || getStatus() == TRANS_STATUS_BOUND) break;
      makePermFail();
      break;
    }
    case OO_BIND: {
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received Bind",this);
      if (isPermFail() || getStatus() == TRANS_STATUS_BOUND) break;
      // do the callback in order to bind
      AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
      PstInContainerInterface* arg = gf_popPstIn(msg);
      GlobalThread* tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
      Assert(a_coordinator->m_getProxy() != NULL);
      PstOutContainerInterface* ans = NULL;
      a_coordinator->m_doe(aop, tid, NULL, arg, ans);
      setStatus(TRANS_STATUS_BOUND);
      // send OO_REDIRECT to all proxies
      while (!a_proxies.isEmpty()) sendRedirect(a_proxies.pop());
      break;
    }
    case OO_UPDATE_REQUEST: {
      if (isPermFail() || getStatus() == TRANS_STATUS_BOUND) break;
      int aop = msg->popIntVal();
      PstInContainerInterface* arg = gf_popPstIn(msg);
      GlobalThread* tid = (msg->m_isEmpty() ? NULL : popThreadId(msg));
      PstOutContainerInterface* ans = arg->loopBack2Out();
      // send OO_UPDATE to all proxies except requester
      for (Position<DSite*> p(a_proxies); p(); p++) {
	if ((*p) != s) sendToProxy(*p, OO_UPDATE, aop, ans->duplicate());
      }
      // send OO_UPDATE to requester (with GlobalThread if provided)
      if (tid) sendToProxy(s, OO_UPDATE, aop, ans, tid);
      else sendToProxy(s, OO_UPDATE, aop, ans);
      break; 
    }
    default: 
      a_coordinator->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }



  /******************** ProtocolOnceOnlyProxy ********************/
  
  // simple constructor
  ProtocolOnceOnlyProxy::ProtocolOnceOnlyProxy() : ProtocolProxy(PN_TRANSIENT) {
    setStatus(TRANS_STATUS_FREE);
    setRegistered(true);     // home proxy is registered by manager
  }

  // destructor
  ProtocolOnceOnlyProxy::~ProtocolOnceOnlyProxy() {
    Assert(a_susps.isEmpty());
    // deregister if this proxy is remote, and the transient is not bound
    if (getStatus() < TRANS_STATUS_WAITING && !a_proxy->m_isHomeProxy())
      protocol_Deregister();
  }

  // initiate a bind
  OpRetVal
  ProtocolOnceOnlyProxy::protocol_Terminate(GlobalThread* const th_id,
					    ::PstOutContainerInterface**& msg,
					    const AbsOp& aop)
  {
    msg = NULL;   // default
    if (isPermFail()) return DSS_RAISE;
    switch (getStatus()) {
    case TRANS_STATUS_FREE:
      // send an OO_BIND message to the manager
      if (th_id) sendToManager(OO_BIND, aop, UnboundPst(msg), th_id);
      else sendToManager(OO_BIND, aop, UnboundPst(msg));
      setStatus(TRANS_STATUS_WAITING);
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
  ProtocolOnceOnlyProxy::protocol_Update(GlobalThread* const th_id,
					 ::PstOutContainerInterface**& msg,
					 const AbsOp& aop)
  {
    msg = NULL;   // default
    if (isPermFail()) return DSS_RAISE;
    switch (getStatus()) {
    case TRANS_STATUS_FREE:
      // send an OO_UPDATE_REQUEST message to the manager.  This
      // request is useless if we already attempted to bind the
      // transient, since the binding will succeed before this update.
      if (th_id) sendToManager(OO_UPDATE_REQUEST, aop, UnboundPst(msg), th_id);
      else sendToManager(OO_UPDATE_REQUEST, aop, UnboundPst(msg));
      // fall through
    case TRANS_STATUS_WAITING:
      // suspend the current thread until an answer comes back
      if (th_id) a_susps.append(th_id);
      return DSS_SUSPEND;
    default:
      return DSS_RAISE;
    }
  }

  // kill the entity
  OpRetVal
  ProtocolOnceOnlyProxy::protocol_Kill() {
    if (!isPermFail() && getStatus() < TRANS_STATUS_WAITING) {
      setStatus(TRANS_STATUS_WAITING);   // later requests will never succeed
      return ProtocolProxy::protocol_Kill();
    }
    return DSS_SKIP;
  }

  // receive a message
  void
  ProtocolOnceOnlyProxy::msgReceived(MsgContainer * msg, DSite*) {
    if (isPermFail() || getStatus() == TRANS_STATUS_BOUND) return;
    int msgType = msg->popIntVal();
    switch (msgType) {
    case OO_REDIRECT: {
      setStatus(TRANS_STATUS_BOUND);
      PstInContainerInterface* cont = gf_popPstIn(msg);
      //
      // The home proxy does not have to perform the bind, it is
      // already done by the manager...
      if (!a_proxy->m_isHomeProxy()) a_proxy->installEntityState(cont);
      // resume all suspensions (not really clear about the resumeDoLocal)
      while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
      break;
    }
    case OO_UPDATE: {
      // do the callback in order to update
      AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
      PstInContainerInterface* cont = gf_popPstIn(msg);
      PstOutContainerInterface* ans;
      a_proxy->m_doe(aop, NULL, NULL, cont, ans);
      // resume calling thread if this is a confirmation
      if (!msg->m_isEmpty()) {
	GlobalThread* tid = popThreadId(msg);
	a_susps.remove(tid);
	tid->resumeDoLocal(NULL);
      }
      break; 
    }
    case PROT_PERMFAIL: {
      makePermFail();
      break;
    }
    default: 
      a_proxy->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }

  // marshal proxy information (autoregistration mechanism)
  bool ProtocolOnceOnlyProxy::marshal_protocol_info(DssWriteBuffer *buf,
						    DSite* dest) {
#ifdef AUTOREGISTRATION
    if (dest && a_proxy->m_isHomeProxy()) {
      // we automatically register dest at the manager
      ProtocolOnceOnlyManager* pm =
	static_cast<ProtocolOnceOnlyManager*>(a_proxy->a_coordinator->a_prot);
      pm->registerRemote(dest);
      buf->putByte(OO_REG_AUTO);

    } else {
      buf->putByte(OO_REG_MANUAL);
    }
#endif
    return true;
  }

  bool 
  ProtocolOnceOnlyProxy::dispose_protocol_info(DssReadBuffer *buf) {
#ifdef AUTOREGISTRATION
    buf->getByte();
#endif
    return true;
  }

  // initialize remote proxy (for registration)
  bool
  ProtocolOnceOnlyProxy::m_initRemoteProt(DssReadBuffer* buf) {
#ifdef AUTOREGISTRATION
    Assert(!a_proxy->m_isHomeProxy());
    if (buf->getByte() == OO_REG_AUTO) return true;
#endif
    dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send REGISTER",this);
    setRegistered(false);
    protocol_Register();
    return true;
  }

  // interpret a site failure
  FaultState
  ProtocolOnceOnlyProxy::siteStateChanged(DSite* s, const DSiteState& state) {
    if (getStatus() <= TRANS_STATUS_WAITING &&
	(a_proxy->m_getCoordinatorSite() == s)) {
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
