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

  namespace {
    enum SC_message {
      SC_ASYNCH,     // asynchronuous operation
      SC_SYNCH,      // synchronuous operation
      SC_RETURN,     // return from synchronuous operation
      SC_PERMFAIL    // make the entity permfail
    };
  }

  class SimpleOp: public DssOperationId{
  public: 
    GlobalThread *a_threadId; 
    DSite* a_sender; 
    SimpleOp(GlobalThread* id, DSite* sender) :
      a_threadId(id), a_sender(sender) {}
  private:
    SimpleOp(const SimpleOp&):a_threadId(NULL), a_sender(NULL){}
    SimpleOp operator=(const SimpleOp&){ return *this; }
  };


  
  ProtocolSimpleChannelManager::ProtocolSimpleChannelManager() :
    failed(false) {}

  void ProtocolSimpleChannelManager::sendMigrateInfo(::MsgContainer* msg){
    msg->pushIntVal(failed);
    gf_pushPstOut(msg, a_coordinator->retrieveEntityState());
    static_cast<ProtocolSimpleChannelProxy*>(a_coordinator->m_getProxy()->m_getProtocol())->stateHolder = false; 
  }

  ProtocolSimpleChannelManager::ProtocolSimpleChannelManager(::MsgContainer *msg){
    failed = msg->popIntVal();
    ::PstInContainerInterface* builder = gf_popPstIn(msg);
    static_cast<ProtocolSimpleChannelProxy*>(a_coordinator->m_getProxy()->m_getProtocol())->stateHolder = true; 
    a_coordinator->installEntityState(builder); 
  }

  void
  ProtocolSimpleChannelManager::msgReceived(::MsgContainer *msg, DSite* sender){
    if (failed) { // return SC_PERMFAIL
      sendToProxy(sender, SC_PERMFAIL); return;
    }
    int msgType = msg->popIntVal();
    switch (msgType) {
    case SC_ASYNCH:
    case SC_SYNCH: {
      AbsOp aop = static_cast<AbsOp>(msg->popIntVal());
      PstInContainerInterface* builder = gf_popPstIn(msg);
      PstOutContainerInterface* ans = NULL;
      if (msgType == SC_ASYNCH) {
	SimpleOp* so = new SimpleOp(NULL, NULL);
	if (a_coordinator->m_doe(aop, NULL, so, builder, ans) == AOCB_FINISH)
	  delete so;
      }
      else {
	GlobalThread* id = popThreadId(msg);
	SimpleOp* so = new SimpleOp(id, sender);
	if (a_coordinator->m_doe(aop, id, so, builder, ans) == AOCB_FINISH) {
	  delete so;
	  sendToProxy(sender, SC_RETURN, id, ans);
	}
      }
      break;
    }
    case SC_PERMFAIL: {
      failed = true;
      sendToProxy(sender, SC_PERMFAIL);
      break;
    }
    default:
      Assert(0);
    }
  }

  void
  ProtocolSimpleChannelProxy::remoteInitatedOperationCompleted(DssOperationId* opId,
							       ::PstOutContainerInterface* pstOut)
  {
    SimpleOp *so = static_cast<SimpleOp*>(opId); 
    if (so->a_sender!=NULL)
      sendToProxy(so->a_sender, SC_RETURN, so->a_threadId, pstOut);
    delete so; 
  }

  void
  ProtocolSimpleChannelProxy::msgReceived(::MsgContainer *msg, DSite* u){
    if (failed) return;
    switch (msg->popIntVal()) {
    case SC_RETURN: {
      GlobalThread *th = popThreadId(msg);
      ::PstInContainerInterface* load = gf_popPstIn(msg);
      th->resumeRemoteDone(load);
      a_susps.remove(th);
      break;
    }
    case SC_PERMFAIL:
      makeFailed();
      a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
      break;
    }
  }

  OpRetVal
  ProtocolSimpleChannelProxy::protocol_Synch(GlobalThread* const th_id,
					     ::PstOutContainerInterface**& msg,
					     const AbsOp& aop){
    msg = NULL;   // default
    if (failed) return DSS_RAISE;
    // Check if we it is a home proxy, in such cases the 
    // operation can be performed without sending messages. 
    if (stateHolder) return DSS_PROCEED;
    if (!sendToManager(SC_SYNCH, aop, UnboundPst(msg), th_id))
      return DSS_RAISE;
    a_susps.append(th_id);
    return DSS_SUSPEND;
  }

  OpRetVal
  ProtocolSimpleChannelProxy::protocol_Asynch(::PstOutContainerInterface**& msg,
					      const AbsOp& aop){
    msg = NULL;   // default
    if (failed) return DSS_RAISE;
    if (stateHolder) return DSS_PROCEED;
    if (!sendToManager(SC_ASYNCH, aop, UnboundPst(msg)))
      return DSS_RAISE;
    return DSS_SKIP;
  }

  OpRetVal
  ProtocolSimpleChannelProxy::protocol_Kill(GlobalThread* const th_id) {
    if (failed) return DSS_RAISE;
    if (!sendToManager(SC_PERMFAIL)) return DSS_RAISE;
    a_susps.append(th_id);
    return DSS_SUSPEND;
  }


  ProtocolSimpleChannelProxy::ProtocolSimpleChannelProxy() :
    ProtocolProxy(PN_SIMPLE_CHANNEL), stateHolder(true), failed(false) {}

  bool ProtocolSimpleChannelProxy::m_initRemoteProt(DssReadBuffer*){
    stateHolder = false; // change to non-stateholder
    return true;
  }

  FaultState
  ProtocolSimpleChannelProxy::siteStateChanged(DSite* s,
					       const DSiteState& state) {
    if (!failed && !stateHolder && (a_proxy->m_getCoordinatorSite() == s)) {
      switch (state) {
      case DSite_OK:         return FS_PROT_STATE_OK;
      case DSite_TMP:        return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM:
      case DSite_LOCAL_PRM:  makeFailed(); return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }

  void ProtocolSimpleChannelProxy::makeFailed() {
    failed = true;
    // wake up suspensions (not complete yet)
  }
  
  void ProtocolSimpleChannelProxy::localInitatedOperationCompleted(){ ; }

} //end namespace
