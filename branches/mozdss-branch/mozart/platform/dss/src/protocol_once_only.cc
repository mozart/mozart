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
  // receive the OO_REDIRECT message.
  //
  // Note.  The purpose of OO_UPDATE_CONFIRM is to wakeup the
  // requesting thread.  We do not need such a message in the binding
  // case, since OO_REDIRECT wakes up all suspended threads.

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
      OO_UPDATE_CONFIRM  // MP: update confirmation (to requesting proxy)
    };

    // type of registration
    enum OO_Reg_Type {
      OO_REG_AUTO   = 0, // the proxy is already registered
      OO_REG_MANUAL = 1  // the proxy must register manually
    };
  }



  /******************** ProtocolOnceOnlyManager ********************/

  // normal constructor  
  ProtocolOnceOnlyManager::ProtocolOnceOnlyManager(DSite* const site) :
    a_proxies(new OneContainer<DSite>(site,NULL)), a_bound(false)
  {}

  // fill msgC with manager migration info
  void ProtocolOnceOnlyManager::sendMigrateInfo(MsgContainer* msgC) {
    OneContainer<DSite> *ptr = a_proxies;
    int len = 0;
    for (; ptr != NULL; ptr = ptr->a_next) len++;
    msgC->pushIntVal(a_bound);
    msgC->pushIntVal(len);
    for (ptr = a_proxies; ptr != NULL; ptr = ptr->a_next)
      msgC->pushDSiteVal(ptr->a_contain1);
  }

  // constructor called in case of migration
  ProtocolOnceOnlyManager::ProtocolOnceOnlyManager(MsgContainer* const msgC) :
    a_proxies(NULL)
  {
    a_bound = ((msgC->popIntVal())!=0);
    for (int len = msgC->popIntVal(); len > 0 ; len--)
      a_proxies = new OneContainer<DSite>(msgC->popDSiteVal(), NULL);
  }

  // destructor
  ProtocolOnceOnlyManager::~ProtocolOnceOnlyManager() {
    t_deleteList(a_proxies);
  }

  // gc
  void ProtocolOnceOnlyManager::makeGCpreps(){
    t_gcList(a_proxies);
  }

  // register a remote proxy
  void ProtocolOnceOnlyManager::register_remote(DSite* s) {
    // don't insert it twice
    t_deleteCompare(&a_proxies, s);
    a_proxies = new OneContainer<DSite>(s, a_proxies);

    // send an update for changes if necessary
    ::PstOutContainerInterface *ans;
    manager->m_doe(AO_OO_CHANGES, NULL, NULL, NULL, ans);
    if (ans != NULL) {
      MsgContainer *msgC = manager->m_createProxyProtMsg();
      msgC->pushIntVal(OO_UPDATE);
      msgC->pushIntVal(AO_OO_UPDATE);
      gf_pushPstOut(msgC, ans);
      manager->m_sendToProxy(s, msgC);
    }
  }

  // send an OO_REDIRECT message to proxy at site s
  void ProtocolOnceOnlyManager::sendRedirect(DSite *s) {
    Assert(manager->m_getProxy() != NULL);
    MsgContainer *msgC = manager->m_createProxyProtMsg();
    msgC->pushIntVal(OO_REDIRECT);
    gf_pushPstOut(msgC, manager->retrieveEntityState());
    manager->m_sendToProxy(s, msgC); 
  }

  // treat messages  
  void
  ProtocolOnceOnlyManager::msgReceived(MsgContainer * msg, DSite* s) {
    int msgType = msg->popIntVal();
    switch (msgType) {
    case OO_REGISTER: {
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received REGISTER %p",this,s);
      if (a_bound)
	sendRedirect(s);
      else
	register_remote(s);
      break;
    }
    case OO_DEREGISTER:{
      // Remove proxy from list.  If the once_only is bound, the proxy
      // has already been removed.  This latecoming request can thus
      // be dropped.
      if (a_bound) break;
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received DEREGISTER %p\n",this,s);
#ifdef DEBUG_CHECK
      bool t = t_deleteCompare(&a_proxies, s);
      Assert(t);
#else
      t_deleteCompare(&a_proxies, s);
#endif
      break;
    }
    case OO_BIND: {
      if (a_bound) break; 
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received Bind",this);
      // do the callback in order to bind
      AbsOp aop    = static_cast<AbsOp>(msg->popIntVal());
      GlobalThread *id  = gf_popThreadIdVal(msg, manager->m_getEnvironment());
      ::PstInContainerInterface *builder = gf_popPstIn(msg);
      Assert(manager->m_getProxy() != NULL);
      ::PstOutContainerInterface *ans;
      manager->m_doe(aop,id,NULL,builder, ans);
      a_bound = true; 
      // send OO_REDIRECT to all proxies
      while (a_proxies!=NULL) {
	OneContainer<DSite> *next = a_proxies->a_next;
	sendRedirect(a_proxies->a_contain1);
	delete a_proxies;
	a_proxies = next;
      }
      break;
    }
    case OO_UPDATE_REQUEST: {
      if (a_bound) break;
      int aop = msg->popIntVal();
      GlobalThread *id  = gf_popThreadIdVal(msg, manager->m_getEnvironment());
      ::PstInContainerInterface *builder = gf_popPstIn(msg);
      ::PstOutContainerInterface *ans = builder->loopBack2Out();
      // send OO_UPDATE to all proxies except requester
      for (OneContainer<DSite> *pl = a_proxies; pl != NULL ; pl=pl->a_next) {
	if (pl->a_contain1 != s) {
	  MsgContainer *msgC = manager->m_createProxyProtMsg();
	  msgC->pushIntVal(OO_UPDATE);
	  msgC->pushIntVal(aop);
	  gf_pushPstOut(msgC,ans->duplicate());
	  manager->m_sendToProxy(pl->a_contain1,msgC);
	}
      }
      // send OO_UPDATE_CONFIRM to requester
      MsgContainer *msgC = manager->m_createProxyProtMsg();
      msgC->pushIntVal(OO_UPDATE_CONFIRM);
      msgC->pushIntVal(aop);
      gf_pushPstOut(msgC, ans);
      gf_pushThreadIdVal(msgC,id);
      manager->m_sendToProxy(s,msgC);
      break; 
      
    }
    case OO_GETSTATUS: 
      manager->m_getEnvironment()->a_map->GL_error("Msg %d to variable not implemented yet", msgType);
    default: 
      manager->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }



  /******************** ProtocolOnceOnlyProxy ********************/
  
  // simple constructor
  ProtocolOnceOnlyProxy::ProtocolOnceOnlyProxy() :
    ProtocolProxy(PN_TRANSIENT), a_susps(NULL), a_bound(false) {}

  // destructor
  ProtocolOnceOnlyProxy::~ProtocolOnceOnlyProxy() {
    Assert(a_susps == NULL);
    // deregister if this proxy is remote, and the transient is not bound
    if (!a_bound && a_proxy->m_getProxyStatus() == PROXY_STATUS_REMOTE) {
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(OO_DEREGISTER);
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send DEREGISTER", this);
      a_proxy->m_sendToCoordinator(msgC);
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
    if (a_bound) {
      msg = NULL; 
      return DSS_PROCEED; 
    }
    // send an OO_BIND message to the manager    
    MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
    msgC->pushIntVal(OO_BIND);
    msgC->pushIntVal(aop);
    gf_pushThreadIdVal(msgC,th_id);
    msg = gf_pushUnboundPstOut(msgC);
    if (a_proxy->m_sendToCoordinator(msgC)) {
      // suspend the current thread until an answer comes back
      a_susps = new OneContainer<GlobalThread>(th_id, a_susps);
      return DSS_SUSPEND;
    }
    // the message could not be sent
    msg = NULL;
    return DSS_RAISE;
  }

  // initiate an update
  OpRetVal 
  ProtocolOnceOnlyProxy::protocol_Update(GlobalThread* const th_id,
					 ::PstOutContainerInterface**& msg,
					 const AbsOp& aop)
  {
    if (a_bound) {
      msg = NULL;
      return DSS_PROCEED;
    }
    // send an OO_UPDATE_REQUEST message to the manager
    MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
    msgC->pushIntVal(OO_UPDATE_REQUEST);
    msgC->pushIntVal(aop); 
    gf_pushThreadIdVal(msgC,th_id);
    msg = gf_pushUnboundPstOut(msgC);
    if (a_proxy->m_sendToCoordinator(msgC)) {
      // suspend the current thread until an answer comes back
      a_susps = new OneContainer<GlobalThread>(th_id, a_susps);
      return DSS_SUSPEND;
    }
    // the message could not be sent
    msg = NULL;
    return DSS_RAISE;
  }

  // receive a message
  void
  ProtocolOnceOnlyProxy::msgReceived(MsgContainer * msg, DSite*) {
    int msgType = msg->popIntVal();
    switch (msgType) {
    case OO_REDIRECT: {
      a_bound = true;
      PstInContainerInterface* cont = gf_popPstIn(msg);
      //
      // The home proxy does not have to perform the bind, it is
      // already done by the manager...
      if (a_proxy->m_getProxyStatus() != PROXY_STATUS_HOME)
	a_proxy->installEntityState(cont);
      // resume all suspensions
      while (a_susps) {
	OneContainer<GlobalThread>* next = a_susps->a_next;
	// not really clear about this
	(a_susps->a_contain1)->resumeDoLocal(NULL);
	delete a_susps;
	a_susps = next;
      }
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
	GlobalThread* id = gf_popThreadIdVal(msg, a_proxy->m_getEnvironment());
	t_deleteCompare(&a_susps, id);
	id->resumeDoLocal(NULL);
      }
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
    if (dest && a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME) {
      // we automatically register dest at the manager
      ProtocolOnceOnlyManager* pm =
	static_cast<ProtocolOnceOnlyManager*>(a_proxy->a_man->a_prot);
      pm->register_remote(dest);
      buf->putByte(OO_REG_AUTO);

    } else {
      buf->putByte(OO_REG_MANUAL);
    }
    return true;
  }

  bool 
  ProtocolOnceOnlyProxy::dispose_protocol_info(DssReadBuffer *buf) {
    buf->getByte();
    return true;
  }

  // initialize remote proxy (for registration)
  bool
  ProtocolOnceOnlyProxy::m_initRemoteProt(DssReadBuffer* buf) {
    Assert(a_proxy->m_getProxyStatus() == PROXY_STATUS_REMOTE);
    switch (buf->getByte()) {
    case OO_REG_AUTO:
      break;
    case OO_REG_MANUAL: {
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(OO_REGISTER);
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send REGISTER",this);
      a_proxy->m_sendToCoordinator(msgC);
      break;
    }
    }
    return true;
  }

  // interpret a site failure
  FaultState
  ProtocolOnceOnlyProxy::siteStateChanged(DSite* s, const DSiteState& state) {
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
