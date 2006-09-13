/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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
#pragma implementation "protocol_simple_channel.hh"
#endif

#include "protocol_simple_channel.hh"

namespace _dss_internal{ //Start namespace

  // Quick description of the protocol.
  //
  // Proxy P makes an asynchronuous operation:
  //    P                   M
  //    |-----SC_ASYNCH---->|
  //    |<--PROT_PERMFAIL---| if state is permfail
  //
  // Proxy P makes a synchronuous operation:
  //    P                   M
  //    |-----SC_SYNCH----->|
  //    |<----SC_RETURN-----|
  // or:
  //    |<--PROT_PERMFAIL---| if state is permfail

  namespace {
    enum SC_message {
      SC_ASYNCH,     // asynchronuous operation
      SC_SYNCH,      // synchronuous operation
      SC_RETURN      // return from synchronuous operation
    };
  }


  
  /******************** ProtocolSimpleChannelManager ********************/

  ProtocolSimpleChannelManager::ProtocolSimpleChannelManager(DSite* const s) {}

  ProtocolSimpleChannelProxy* ProtocolSimpleChannelManager::homeProxy() const {
    return static_cast<ProtocolSimpleChannelProxy*>
      (a_coordinator->m_getProxy()->m_getProtocol());
  }

  void ProtocolSimpleChannelManager::sendMigrateInfo(MsgContainer* msg) {
    // put generic info, then the entity's state
    ProtocolManager::sendMigrateInfo(msg);
    msgPush(msg, a_coordinator->deinstallEntityState());
    homeProxy()->setStatus(false);
  }

  ProtocolSimpleChannelManager::ProtocolSimpleChannelManager(MsgContainer* msg)
    : ProtocolManager(msg) {
    // install entity state
    homeProxy()->setStatus(true);
    a_coordinator->installEntityState(gf_popPstIn(msg));
  }

  void
  ProtocolSimpleChannelManager::msgReceived(MsgContainer* msg, DSite* sender) {
    if (isPermFail()) { // return PROT_PERMFAIL
      sendToProxy(sender, PROT_PERMFAIL); return;
    }
    int msgType = msg->popIntVal();
    switch (msgType) {
    case SC_ASYNCH:
    case SC_SYNCH: {
      AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
      PstInContainerInterface* arg = gf_popPstIn(msg);
      if (msgType == SC_ASYNCH)
	homeProxy()->do_operation(NULL, NULL, aop, arg);
      else
	homeProxy()->do_operation(sender, popThreadId(msg), aop, arg);
      break;
    }
    case PROT_REGISTER:
      registerProxy(sender);
      break;
    case PROT_DEREGISTER:
      deregisterProxy(sender);
      break;
    case PROT_PERMFAIL:
      makePermFail();
      // we must make the home proxy fail, otherwise it could still
      // access the state
      homeProxy()->makePermFail();
      break;
    default:
      Assert(0);
    }
  }



  /******************** ProtocolSimpleChannelProxy ********************/

  ProtocolSimpleChannelProxy::ProtocolSimpleChannelProxy() :
    ProtocolProxy(PN_SIMPLE_CHANNEL)
  { setStatus(true); }

  ProtocolSimpleChannelProxy::~ProtocolSimpleChannelProxy() {
    if (!a_proxy->m_isHomeProxy()) protocol_Deregister();
  }

  bool ProtocolSimpleChannelProxy::m_initRemoteProt(DssReadBuffer*){
    setStatus(false); // change to non-stateholder
    return true;
  }

  // used for resuming callbacks when they are not atomic
  class RemoteOperation : public DssOperationId {
  public:
    DSite*        sender;
    GlobalThread* caller;
    RemoteOperation(DSite* s, GlobalThread* t) : sender(s), caller(t) {}
  };

  // perform an operation for a remote proxy (use NULL as sender for
  // asynchronuous operations)
  void
  ProtocolSimpleChannelProxy::do_operation(DSite* sender, GlobalThread* caller,
					   AbsOp aop, PstInContainerInterface* arg) {
    RemoteOperation* op = new RemoteOperation(sender, caller);
    PstOutContainerInterface* ans = NULL;
    if (a_proxy->m_doe(aop, caller, op, arg, ans) == AOCB_FINISH)
      ProtocolSimpleChannelProxy::remoteInitatedOperationCompleted(op, ans);
  }

  void
  ProtocolSimpleChannelProxy::remoteInitatedOperationCompleted(DssOperationId* opId,
							       ::PstOutContainerInterface* pstOut)
  {
    RemoteOperation* op = static_cast<RemoteOperation*>(opId);
    if (op->sender) sendToProxy(op->sender, SC_RETURN, op->caller, pstOut);
    delete op;
  }

  void
  ProtocolSimpleChannelProxy::msgReceived(::MsgContainer *msg, DSite* u){
    if (isPermFail()) return;
    switch (msg->popIntVal()) {
    case SC_RETURN: {
      GlobalThread *th = popThreadId(msg);
      ::PstInContainerInterface* load = gf_popPstIn(msg);
      th->resumeRemoteDone(load);
      a_susps.remove(th);
      break;
    }
    case PROT_PERMFAIL:
      makePermFail();
      break;
    default:
      Assert(0);
    }
  }

  OpRetVal
  ProtocolSimpleChannelProxy::protocol_Synch(GlobalThread* const th_id,
					     ::PstOutContainerInterface**& msg,
					     const AbsOp& aop){
    msg = NULL;   // default
    if (isPermFail()) return DSS_RAISE;
    if (getStatus()) return DSS_PROCEED;
    if (!sendToManager(SC_SYNCH, aop, UnboundPst(msg), th_id))
      return DSS_RAISE;
    a_susps.append(th_id);
    return DSS_SUSPEND;
  }

  OpRetVal
  ProtocolSimpleChannelProxy::protocol_Asynch(::PstOutContainerInterface**& msg,
					      const AbsOp& aop){
    msg = NULL;   // default
    if (isPermFail()) return DSS_RAISE;
    if (getStatus()) return DSS_PROCEED;
    if (!sendToManager(SC_ASYNCH, aop, UnboundPst(msg)))
      return DSS_RAISE;
    return DSS_SKIP;
  }

  FaultState
  ProtocolSimpleChannelProxy::siteStateChanged(DSite* s,
					       const DSiteState& state) {
    if (!isPermFail() && (a_proxy->m_getCoordinatorSite() == s)) {
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
