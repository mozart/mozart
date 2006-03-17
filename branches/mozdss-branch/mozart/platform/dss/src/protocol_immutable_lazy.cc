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
#pragma implementation "protocol_immutable_lazy.hh"
#endif

#include "protocol_immutable_lazy.hh"
namespace _dss_internal{ //Start namespace
  ProtocolImmutableLazyManager::ProtocolImmutableLazyManager(){}
  
  void
  ProtocolImmutableLazyManager::msgReceived(MsgContainer *msg, DSite* sender){
    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
    gf_pushPstOut(msgC, a_coordinator->retrieveEntityState());
    sender->m_sendMsg(msgC);
  }
  
  ProtocolImmutableLazyProxy::ProtocolImmutableLazyProxy():ProtocolProxy(PN_IMMUTABLE_LAZY), stateHolder(true), a_readers(NULL){;} 
  
  void
  ProtocolImmutableLazyProxy::msgReceived(MsgContainer *msg, DSite* u){
    PstInContainerInterface* load = gf_popPstIn(msg);
    a_proxy->installEntityState(load);
    while(a_readers!=NULL){
      OneContainer<GlobalThread>* ptr = a_readers->a_next; 
      (a_readers->a_contain1)->resumeDoLocal(NULL);
      delete a_readers;
      a_readers  = ptr;
    }    
    a_proxy->a_AbsEnt_Interface->m_getAEreference()->accessMediator()->localize();
    stateHolder = true;   
  }
 
  bool
  ProtocolImmutableLazyProxy::m_initRemoteProt(DssReadBuffer*){
    stateHolder = false; 
    return true;
  }
  
  void  ProtocolImmutableLazyProxy::remoteInitatedOperationCompleted(DssOperationId*, PstOutContainerInterface*) {;}

  void ProtocolImmutableLazyProxy::localInitatedOperationCompleted(){ ; }

  OpRetVal
  ProtocolImmutableLazyProxy::protocol_send(GlobalThread* const th_id, PstOutContainerInterface**& msg){
    // Check if we it is a home proxy, in such cases the 
    // operation can be performed without sending messages. 
    if (stateHolder){
      msg = NULL;
      return DSS_PROCEED;
    }
    if (a_readers == NULL) {
      MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
      if(a_proxy->m_sendToCoordinator(msgC) == false)
	{
	  msg = NULL; 
	  return (DSS_RAISE);
	}
    }
    a_readers = new OneContainer<GlobalThread>(th_id, a_readers);
    return (DSS_SUSPEND);
  }

  
} //end namespace
