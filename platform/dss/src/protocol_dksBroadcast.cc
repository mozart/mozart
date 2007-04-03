/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2004
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
#pragma implementation "protocol_dksBroadcast.hh"
#endif

#include "protocol_dksBroadcast.hh"
namespace _dss_internal{ //Start namespace


  // ****************** ProtocolDksBcManager ********************'  
  
  ProtocolDksBcManager::ProtocolDksBcManager(){
    ;
  }
  
  void ProtocolDksBcManager::msgReceived(MsgContainer*,DSite*){
    printf("ProtocolDksBcManager received messages\n"); 
  }
  
void ProtocolDksBcManager::makeGCpreps(){
    printf("ProtocolDksBcManager::makeGCpreps\n"); 
  }

  //***************  Class ProtocolDksBcProxy  ************************


  ProtocolDksBcProxy::ProtocolDksBcProxy():
    ProtocolProxy(PN_DKSBROADCAST), 
    a_isFunctional(false),
    a_isReferenceInstance(false),
    a_dks(NULL)
  {
    ;
  }

  void 
  ProtocolDksBcProxy::m_initHome(DSS_Environment* env)
  {
    a_isFunctional = true;
    a_isReferenceInstance = true;
    a_dks = new DksInstance(256, 2, 1, this, env); 
  }
  
  OpRetVal ProtocolDksBcProxy::m_broadCast(PstOutContainerInterface** &pst, const AbsOp& aop){
    if (a_isFunctional){
      PstDataContainer *pdc = new PstDataContainer(a_proxy->m_getEnvironment(), 
						   pst);
      printf("protocol dksBroadcast %d\n", (int) pst); 
      DssDksBcMessage *msg = new DssDksBcMessage(pdc, static_cast<int>(aop)); 
      a_dks->m_broadcastRing(msg);
    }else{
      // a_unsentMsgs.append(makePair(pst, (int) aop));
      Assert(0); 
    }
    return DSS_PROCEED; 
  }
  
  void ProtocolDksBcProxy::m_receivedBroadcast(DksBcMessage* bc){
    DssDksBcMessage *msg = static_cast<DssDksBcMessage*>(bc); 
    PstInContainerInterface* pst = msg->m_getData()->m_getPstIn();
    PstOutContainerInterface* _unused = NULL; 
    a_proxy->m_doe(static_cast<AbsOp>(msg->m_getAop()), NULL, NULL, pst,_unused);
  }
  
  void ProtocolDksBcProxy::dks_functional(){
    printf("the dks is functional, time to serialize all unsent messages\n"); 
  }
  
  bool ProtocolDksBcProxy::isWeakRoot(){
    // the one at the manager site is a weak root since the 
    // manager uses that instance as the reference to the 
    // DKS broadcast ring
    return a_isReferenceInstance;
  }
  void ProtocolDksBcProxy::msgReceived(MsgContainer*,DSite*){
    //
    
  }
  bool ProtocolDksBcProxy::clearWeakRoot(){ return false; }
  void ProtocolDksBcProxy::makeGCpreps(){}
  bool ProtocolDksBcProxy::m_initRemoteProt(DssReadBuffer* buf){
    a_proxy->m_getEnvironment()->a_dksInstHT->m_unmarshalDksInstance(buf, a_dks);
    a_dks->setCallBackService(this); 
    a_dks->m_joinNetwork(a_proxy->m_getCoordinatorSite()); 
    return false; 
  }
  
  char *ProtocolDksBcProxy::m_stringrep(){
    return "<DksBC>"; 
  }
  
  bool ProtocolDksBcProxy::marshal_protocol_info(DssWriteBuffer *buf, DSite *){
    a_dks->m_marshal(buf); 
    return false; 
  }
  bool ProtocolDksBcProxy::dispose_protocol_info(DssReadBuffer *buf){ 
    DksInstance* _unused; 
    a_proxy->m_getEnvironment()->a_dksInstHT->m_unmarshalDksInstance(buf, _unused); 
    Assert(a_dks == _unused); 
    return false; 
  }
  
  void ProtocolDksBcProxy::remoteInitatedOperationCompleted(DssOperationId* opId,
							    ::PstOutContainerInterface* pstOut){
    printf("We don't do anything when remoteIniated operation compledet\n"); 
  }
  
  void ProtocolDksBcProxy::localInitatedOperationCompleted(){
    printf("We don't do anything when localIniated operation compledet\n"); 
  }
  
  
}

