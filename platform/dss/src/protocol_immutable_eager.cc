/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Per Sahlin, 2003
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
#pragma implementation "protocol_immutable_eager.hh"
#endif

#include "protocol_immutable_eager.hh"

namespace _dss_internal{ //Start namespace

  // Quick description of the protocol.
  //
  // The protocol is extremely simple.  When a proxy is created, it
  // sends an empty message to its manager, which replies with the
  // state.  The proxy then installs the state.

  namespace {
    enum Immutable_Message {
      IMM_GET,       // PM: ask state to manager
      IMM_PUT        // MP: send state to proxy
    };
  }



  /******************** ProtocolImmutableManager ********************/

  void ProtocolImmutableManager::sendMigrateInfo(MsgContainer* msg) {
    ProtocolManager::sendMigrateInfo(msg);
    // we also take a copy of the state, just in case
    msgPush(msg, a_coordinator->retrieveEntityState());
  }

  ProtocolImmutableManager::ProtocolImmutableManager(MsgContainer* msg) :
    ProtocolManager(msg) {
    ProtocolProxy* pp = a_coordinator->m_getProxy()->m_getProtocol();
    static_cast<ProtocolImmutableProxy*>(pp)->m_installState(gf_popPstIn(msg));
  }

  void
  ProtocolImmutableManager::msgReceived(MsgContainer *msg, DSite* sender){
    if (isPermFail()) {
      sendToProxy(sender, PROT_PERMFAIL); return;
    }
    switch (msg->popIntVal()) {
    case PROT_REGISTER:
      registerProxy(sender);
      break;
    case PROT_DEREGISTER:
      deregisterProxy(sender);
      break;
    case IMM_GET:
      sendToProxy(sender, IMM_PUT, a_coordinator->retrieveEntityState());
      break;
    case PROT_PERMFAIL:
      if (!isRegisteredProxy(sender)) registerProxy(sender);
      makePermFail();
      break;
    default:
      Assert(0);
    }
  }



  /******************** ProtocolImmutableProxy ********************/

  ProtocolImmutableProxy::ProtocolImmutableProxy(const ProtocolName& pn) :
    ProtocolProxy(pn) {
    setStatus(true);     // the home proxy has the state
  }

  ProtocolImmutableProxy::~ProtocolImmutableProxy() {
    Assert(a_susps.isEmpty());
    if (!a_proxy->m_isHomeProxy()) protocol_Deregister();
  }

  bool ProtocolImmutableProxy::m_initRemoteProt(DssReadBuffer*) {
    setStatus(false);     // remote proxies don't have the state
    return false;
  }

  void
  ProtocolImmutableProxy::m_requestState() {
    sendToManager(IMM_GET);
  }

  void
  ProtocolImmutableProxy::m_installState(PstInContainerInterface* builder) {
    if (getStatus()) {     // state already installed
      builder->dispose();
    } else {
      setStatus(true);
      a_proxy->installEntityState(builder);
      while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
    }
  }


  void
  ProtocolImmutableProxy::msgReceived(MsgContainer *msg, DSite*){
    if (isPermFail()) return;
    switch (msg->popIntVal()) {
    case IMM_PUT:
      m_installState(gf_popPstIn(msg));
      break;
    case PROT_PERMFAIL:
      makePermFail();
      break;
    default:
      Assert(0);
    }
  }


  // handle site failures
  FaultState
  ProtocolImmutableProxy::siteStateChanged(DSite* s, const FaultState& state) {
    if (!isPermFail() && s == a_proxy->m_getCoordinatorSite()) {
      switch (state) {
      case FS_OK:          return FS_STATE_OK;
      case FS_TEMP:        return FS_STATE_TEMP;
      case FS_LOCAL_PERM:  makePermFail(state); return FS_STATE_LOCAL_PERM;
      case FS_GLOBAL_PERM: makePermFail(state); return FS_STATE_GLOBAL_PERM;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }



  /******************** ProtocolImmutableEagerProxy ********************/

  bool 
  ProtocolImmutableEagerProxy::m_initRemoteProt(DssReadBuffer*){
    setStatus(false);
    m_requestState();
    return false;
  }

  OpRetVal
  ProtocolImmutableEagerProxy::protocol_Access(GlobalThread* const th_id){
    if (isPermFail()) return DSS_RAISE;
    if (getStatus()) return DSS_PROCEED;
    // the state is coming, be patient...
    a_susps.append(th_id);
    return DSS_SUSPEND;
  }

} //end namespace
