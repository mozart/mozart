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
  //    P                 M
  //    |---OO_REGISTER-->|
  //    |<--OO_REDIRECT---|   if transient already bound
  // or:
  //    |<---OO_UPDATE----|   if changes are provided by pst
  // or:
  //    |<--OO_PERMFAIL---|   if entity is permfail
  //
  // Deregistration of proxy P:
  //    P                   M
  //    |---OO_DEREGISTER-->|
  //
  // Proxy P binds transient (not bound yet):
  //    P                 M                 P'
  //    |-----OO_BIND---->|                 |
  //    |<--OO_REDIRECT---|---OO_REDIRECT-->|   (sent to all proxies)
  //
  // Proxy P sends an update (transient not bound yet):
  //    P                       M                       P'
  //    |---OO_UPDATE_REQUEST-->|                       |
  //    |<--OO_UPDATE_CONFIRM---|-------OO_UPDATE------>|   (sent to all)
  //
  // Any message OO_BIND or OO_UPDATE_REQUEST received after the
  // binding of the transient is ignored.  The proxy will eventually
  // receive the OO_REDIRECT message (or possibly OO_PERMFAIL).
  //
  // Note.  The purpose of OO_UPDATE_CONFIRM is to wakeup the
  // requesting thread.  We do not need such a message in the binding
  // case, since OO_REDIRECT wakes up all suspended threads.
  //
  // Proxy P makes the entity fail (if not bound or failed yet):
  //    P                 M                 P'
  //    |---OO_PERMFAIL-->|                 |
  //    |<--OO_PERMFAIL---|---OO_PERMFAIL-->|   (sent to all proxies)

  namespace{
    // In the message descriptions, "PM" means a message sent by a
    // proxy to its manager, and so on.
    enum OO_msg_names{
      OO_REGISTER,       // PM: register a proxy at the manager
      OO_DEREGISTER,     // PM: remove the registration at the manager
      OO_BIND,           // PM: try to bind the once only Manager
      OO_REDIRECT,       // MP: tell the binding to proxies
      OO_GETSTATUS,      // PM: get the status (bound or not) of a Manager
      OO_RECEIVESTATUS,  // MP: answer to a getstatus
      OO_UPDATE_REQUEST, // PM: request to update
      OO_UPDATE,         // MP: send update to proxie(s)
      OO_UPDATE_CONFIRM, // MP: update confirmation (to requesting proxy)
      OO_PERMFAIL        // **: make the entity permfail
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
  ProtocolOnceOnlyManager::ProtocolOnceOnlyManager(DSite* const site) :
    a_proxies(), a_bound(false), a_failed(false) {
    a_proxies.push(site);
  }

  // fill msgC with manager migration info
  void ProtocolOnceOnlyManager::sendMigrateInfo(MsgContainer* msgC) {
    msgC->pushIntVal((a_bound ? 1 : 0) + (a_failed ? 2 : 0));
    for (Position<DSite*> p(a_proxies); p(); p++) msgC->pushDSiteVal(*p);
  }

  // constructor called in case of migration
  ProtocolOnceOnlyManager::ProtocolOnceOnlyManager(MsgContainer* const msgC) :
    a_proxies()
  {
    int status = msgC->popIntVal();
    a_bound = status & 1;
    a_failed = status & 2;
    while (!msgC->m_isEmpty()) a_proxies.push(msgC->popDSiteVal());
  }

  // destructor
  ProtocolOnceOnlyManager::~ProtocolOnceOnlyManager() {}

  // gc
  void ProtocolOnceOnlyManager::makeGCpreps(){
    t_gcList(a_proxies);
  }

  // register a remote proxy
  void ProtocolOnceOnlyManager::register_remote(DSite* s) {
    // do nothing if the proxy is already registered
    if (a_proxies.contains(s)) return;

    // insert s in a_proxies
    a_proxies.push(s);

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
    case OO_REGISTER: {
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received REGISTER %p",this,s);
      if (a_bound) sendRedirect(s);
      else if (a_failed) sendToProxy(s, OO_PERMFAIL);
      else register_remote(s);
      break;
    }
    case OO_DEREGISTER:{
      // Remove proxy from list.  If the once_only is bound, the proxy
      // has already been removed.  This latecoming request can thus
      // be dropped.
      if (a_bound || a_failed) break;
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received DEREGISTER %p\n",this,s);
#ifdef DEBUG_CHECK
      bool t = a_proxies.remove(s);
      Assert(t);
#else
      a_proxies.remove(s);
#endif
      break;
    }
    case OO_BIND: {
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received Bind",this);
      if (a_bound || a_failed) break; 
      // do the callback in order to bind
      AbsOp aop    = static_cast<AbsOp>(msg->popIntVal());
      GlobalThread *id = popThreadId(msg);
      ::PstInContainerInterface *builder = gf_popPstIn(msg);
      Assert(a_coordinator->m_getProxy() != NULL);
      ::PstOutContainerInterface *ans;
      a_coordinator->m_doe(aop,id,NULL,builder, ans);
      a_bound = true; 
      // send OO_REDIRECT to all proxies
      while (!a_proxies.isEmpty()) sendRedirect(a_proxies.pop());
      break;
    }
    case OO_UPDATE_REQUEST: {
      if (a_bound || a_failed) break;
      int aop = msg->popIntVal();
      GlobalThread *id = popThreadId(msg);
      ::PstInContainerInterface *builder = gf_popPstIn(msg);
      ::PstOutContainerInterface *ans = builder->loopBack2Out();
      // send OO_UPDATE to all proxies except requester
      for (Position<DSite*> p(a_proxies); p(); p++) {
	if ((*p) != s) sendToProxy(*p, OO_UPDATE, aop, ans->duplicate());
      }
      // send OO_UPDATE_CONFIRM to requester
      sendToProxy(s, OO_UPDATE_CONFIRM, aop, ans, id);
      break; 
      
    }
    case OO_PERMFAIL: {
      if (a_bound || a_failed) break;   // It will no longer fail
      a_failed = true;
      // tell to all proxies
      while (!a_proxies.isEmpty()) sendToProxy(a_proxies.pop(), OO_PERMFAIL);
      break;
    }
    case OO_GETSTATUS: 
      a_coordinator->m_getEnvironment()->a_map->GL_error("Msg %d to variable not implemented yet", msgType);
    default: 
      a_coordinator->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }

  // check for a possible proxy failure
  void
  ProtocolOnceOnlyManager::m_siteStateChange(DSite* s,
					     const DSiteState& state) {
    if (!a_failed && !a_bound &&
	(state == DSite_GLOBAL_PRM || state == DSite_LOCAL_PRM)) {
      // simply remove it from a_proxies (if present)
      a_proxies.remove(s);
    }
  }



  /******************** ProtocolOnceOnlyProxy ********************/
  
  // simple constructor
  ProtocolOnceOnlyProxy::ProtocolOnceOnlyProxy() :
    ProtocolProxy(PN_TRANSIENT), a_susps(), a_bound(false), a_failed(false) {}

  // destructor
  ProtocolOnceOnlyProxy::~ProtocolOnceOnlyProxy() {
    Assert(a_susps.isEmpty());
    // deregister if this proxy is remote, and the transient is not bound
    if (!a_bound && !a_failed && !a_proxy->m_isHomeProxy()) {
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send DEREGISTER", this);
      sendToManager(OO_DEREGISTER);
    }
  }

  void
  ProtocolOnceOnlyProxy::makeGCpreps() {
    t_gcList(a_susps);
  }

  // initiate a bind
  OpRetVal
  ProtocolOnceOnlyProxy::protocol_Terminate(GlobalThread* const th_id,
					    ::PstOutContainerInterface**& msg,
					    const AbsOp& aop)
  {
    msg = NULL;   // default
    if (a_bound) return DSS_PROCEED;
    if (a_failed) return DSS_RAISE;
    // send an OO_BIND message to the manager
    if (sendToManager(OO_BIND, aop, th_id, UnboundPst(msg))) {
      // suspend the current thread until an answer comes back
      a_susps.push(th_id);
      return DSS_SUSPEND;
    }
    // the message could not be sent
    return DSS_RAISE;
  }

  // initiate an update
  OpRetVal 
  ProtocolOnceOnlyProxy::protocol_Update(GlobalThread* const th_id,
					 ::PstOutContainerInterface**& msg,
					 const AbsOp& aop)
  {
    msg = NULL;   // default
    if (a_bound) return DSS_PROCEED;
    if (a_failed) return DSS_RAISE;
    // send an OO_UPDATE_REQUEST message to the manager
    if (sendToManager(OO_UPDATE_REQUEST, aop, th_id, UnboundPst(msg))) {
      // suspend the current thread until an answer comes back
      a_susps.push(th_id);
      return DSS_SUSPEND;
    }
    // the message could not be sent
    return DSS_RAISE;
  }

  // kill the entity
  OpRetVal
  ProtocolOnceOnlyProxy::protocol_Kill() {
    if (!a_bound && !a_failed) sendToManager(OO_PERMFAIL);
    return DSS_SKIP;
  }

  // receive a message
  void
  ProtocolOnceOnlyProxy::msgReceived(MsgContainer * msg, DSite*) {
    if (a_failed) return;
    int msgType = msg->popIntVal();
    switch (msgType) {
    case OO_REDIRECT: {
      a_bound = true;
      PstInContainerInterface* cont = gf_popPstIn(msg);
      //
      // The home proxy does not have to perform the bind, it is
      // already done by the manager...
      if (!a_proxy->m_isHomeProxy()) a_proxy->installEntityState(cont);
      // resume all suspensions (not really clear about the resumeDoLocal)
      while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
      break;
    }
    case OO_UPDATE_CONFIRM:
    case OO_UPDATE: {
      // do the callback in order to update
      int absOp = msg->popIntVal();
      PstInContainerInterface* cont = gf_popPstIn(msg);
      ::PstOutContainerInterface* ans;
      a_proxy->m_doe(static_cast<AbsOp>(absOp),NULL, NULL, cont, ans);
      // resume calling thread if this is a confirmation
      if (msgType == OO_UPDATE_CONFIRM) {
	GlobalThread* id = popThreadId(msg);
	a_susps.remove(id);
	id->resumeDoLocal(NULL);
      }
      break; 
    }
    case OO_PERMFAIL: {
      a_failed = true;
      a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
      // wake up suspensions (not sure, though)
      while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
      break;
    }
    case OO_RECEIVESTATUS:
      a_proxy->m_getEnvironment()->a_map->GL_error("Msg %d to variable not implemented yet", msgType);
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
      pm->register_remote(dest);
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
    switch (buf->getByte()) {
    case OO_REG_AUTO:
      break;
    case OO_REG_MANUAL: {
#endif
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send REGISTER",this);
      sendToManager(OO_REGISTER);
#ifdef AUTOREGISTRATION
      break;
    }
    }
#endif
    return true;
  }

  // interpret a site failure
  FaultState
  ProtocolOnceOnlyProxy::siteStateChanged(DSite* s, const DSiteState& state) {
    if (!a_bound && !a_failed && (a_proxy->m_getCoordinatorSite() == s)) {
      switch (state) {
      case DSite_OK:         return FS_PROT_STATE_OK;
      case DSite_TMP:        return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM:
      case DSite_LOCAL_PRM:
	a_failed = true;
	// wake up suspensions (not sure, though)
	while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
	return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }
  
} //end namespace
