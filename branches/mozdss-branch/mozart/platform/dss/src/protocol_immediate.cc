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
#pragma implementation "protocol_immediate.hh"
#endif

#include "protocol_immediate.hh"
namespace _dss_internal{ //Start namespace
  //  namespace{
//      enum IM_Cont {
//         IM_C_FULL = 0,
//         IM_C_SKEL = 1
//       };
//   }

  ProtocolImmediateManager::ProtocolImmediateManager(){;}

  void
  ProtocolImmediateManager::msgReceived(MsgContainer *msg, DSite* sender){;}

  bool
  ProtocolImmediateProxy::m_initRemoteProt(DssReadBuffer* buf){
    //    stateHolder = true; 
    //       bool skel = !(buf->getByte() == IM_C_FULL);
    //  MsgContainer *msgC = a_proxy->m_createProtMsg();
    //msgC->setMessageType(M_MANAGER_PROTOCOL);
    //a_proxy->m_sendToCoordinator(msgC);
    return false;
  }
  
  bool
  ProtocolImmediateProxy::marshal_protocol_info(DssWriteBuffer *buf, DSite *dest){
    // Packa ihop allt och skicka över returnera false
    //  buf->putByte(IM_C_FULL);
    return false;
  }

  bool 
  ProtocolImmediateProxy::dispose_protocol_info(DssReadBuffer *buf) {
    return false;
  }

  void
  ProtocolImmediateProxy::remoteInitatedOperationCompleted(DssOperationId* opId,
							   PstOutContainerInterface* pstOut)
  {;}

  void
  ProtocolImmediateProxy::msgReceived(MsgContainer *msg, DSite* u){
    GlobalThread *th = gf_popThreadIdVal(msg, a_proxy->m_getEnvironment()); 
    PstInContainerInterface* load = gf_popPstIn(msg);
    th->resumeRemoteDone(load); 
  }

  OpRetVal
  ProtocolImmediateProxy::protocol_send(GlobalThread* const th_id){
    return DSS_PROCEED;
  }

  ProtocolImmediateProxy::ProtocolImmediateProxy():ProtocolProxy(PN_IMMEDIATE){;}

  void 
  ProtocolImmediateProxy::localInitatedOperationCompleted(){ ; }

} //end namespace
