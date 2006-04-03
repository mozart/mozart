/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

  
  ProtocolImmutableEagerManager::ProtocolImmutableEagerManager(){}
  
  void
  ProtocolImmutableEagerManager::msgReceived(MsgContainer *msg, DSite* sender){
    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
    gf_pushPstOut(msgC,a_coordinator->retrieveEntityState());
    sender->m_sendMsg(msgC);
  }
  
  ProtocolImmutableEagerProxy::ProtocolImmutableEagerProxy() :
    ProtocolProxy(PN_IMMUTABLE_EAGER), stateHolder(true), a_readers() {} 
  
  void
  ProtocolImmutableEagerProxy::msgReceived(MsgContainer *msg, DSite* u){
    PstInContainerInterface* load = gf_popPstIn(msg);
    a_proxy->installEntityState(load);
    while (!a_readers.isEmpty())
      a_readers.pop()->resumeDoLocal(NULL);
    a_proxy->a_AbsEnt_Interface->m_getAEreference()->accessMediator()->localize();
    stateHolder = true;   
  }

  bool 
  ProtocolImmutableEagerProxy::m_initRemoteProt(DssReadBuffer*){
    stateHolder = false; 
    MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
    a_proxy->m_sendToCoordinator(msgC);
    return true;
  }
  
  void  
  ProtocolImmutableEagerProxy::remoteInitatedOperationCompleted(DssOperationId*, PstOutContainerInterface*) {;}
  void 
  ProtocolImmutableEagerProxy::localInitatedOperationCompleted(){ ; }


  OpRetVal
  ProtocolImmutableEagerProxy::protocol_send(GlobalThread* const th_id){
    // Check if we it is a home proxy, in such cases the 
    // operation can be performed without sending messages. 
    if (stateHolder) return DSS_PROCEED;
    // If the structure is not transfered, the threads should suspend until 
    // the the structure is complete.
    a_readers.push(th_id);
    return (DSS_SUSPEND);
  }

  void
  ProtocolImmutableEagerProxy::makeGCpreps() {
    t_gcList(a_readers);
  }
  
} //end namespace
