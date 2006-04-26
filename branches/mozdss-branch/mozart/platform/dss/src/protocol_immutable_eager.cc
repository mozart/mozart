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
      IMM_PUT,       // MP: send state to proxy
      IMM_PERMFAIL   // PM,MP: make entity permfail
    };
  }



  /******************** ProtocolImmutableManager ********************/

  ProtocolImmutableManager::ProtocolImmutableManager() : failed(false) {}

  void
  ProtocolImmutableManager::msgReceived(MsgContainer *msg, DSite* sender){
    switch (msg->popIntVal()) {
    case IMM_GET:
      if (!failed) {
	sendToProxy(sender, IMM_PUT, a_coordinator->retrieveEntityState());
	break;
      }
      // fall through
    case IMM_PERMFAIL:
      sendToProxy(sender, IMM_PERMFAIL);
      break;
    default:
      Assert(0);
    }
  }



  /******************** ProtocolImmutableProxy ********************/

  ProtocolImmutableProxy::ProtocolImmutableProxy(const ProtocolName& pn) :
    ProtocolProxy(pn), failed(false), stateHolder(true), a_readers() {} 

  void
  ProtocolImmutableProxy::makeGCpreps() {
    t_gcList(a_readers);
  }

  void
  ProtocolImmutableProxy::m_requestState() {
    sendToManager(IMM_GET);
  }


  OpRetVal
  ProtocolImmutableProxy::protocol_Kill(GlobalThread* const th_id){
    if (failed) return DSS_SKIP;
    sendToManager(IMM_PERMFAIL);
    a_readers.push(th_id);
    return DSS_SUSPEND;
  }


  void
  ProtocolImmutableProxy::msgReceived(MsgContainer *msg, DSite* u){
    if (failed) return;
    switch (msg->popIntVal()) {
    case IMM_PUT: {
      PstInContainerInterface* builder = gf_popPstIn(msg);
      a_proxy->installEntityState(builder);
      stateHolder = true;
      while (!a_readers.isEmpty()) a_readers.pop()->resumeDoLocal(NULL);
      break;
    }
    case IMM_PERMFAIL: {
      failed = true;
      a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
      // suspensions should be woken up...
      break;
    }
    default:
      Assert(0);
    }
  }


  // handle site failures
  FaultState
  ProtocolImmutableProxy::siteStateChanged(DSite* s, const DSiteState& state) {
    if (!failed && !stateHolder && s == a_proxy->m_getCoordinatorSite()) {
      switch (state) {
      case DSite_OK:
	return FS_PROT_STATE_OK;
      case DSite_TMP:
	return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM: case DSite_LOCAL_PRM:
	failed = true;
	return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }



  /******************** ProtocolImmutableEagerProxy ********************/

  bool 
  ProtocolImmutableEagerProxy::m_initRemoteProt(DssReadBuffer*){
    stateHolder = false;
    m_requestState();
    return true;
  }

  OpRetVal
  ProtocolImmutableEagerProxy::protocol_Access(GlobalThread* const th_id){
    if (failed) return DSS_RAISE;
    if (stateHolder) return DSS_PROCEED;
    // the state is coming, be patient...
    a_readers.push(th_id);
    return DSS_SUSPEND;
  }

} //end namespace
