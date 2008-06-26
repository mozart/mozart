/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Erik Klintskog, 1998
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
#pragma implementation "dss_psDKS.hh"
#endif

#include "dss_psDKS.hh"
#include "dss_msgLayerInterface.hh"
#include "msl_serialize.hh"
namespace  _dss_internal{





  class PsDksMessage: public DksMessage{
    PstOutContainerInterface*  a_out;
     PstInContainerInterface*   a_in;
   public:
     PsDksMessage(PstOutContainerInterface* out):
       a_out(out), a_in(NULL){;}
     PsDksMessage(PstInContainerInterface* in):
       a_out(NULL), a_in(in){;}
     PstInContainerInterface* getInPstContainer(){
       PstInContainerInterface* tmp = a_in;
       a_in = NULL;
       return tmp;
     };
    PstOutContainerInterface* getOutPstContainer(){
      if(a_in != NULL)
        {
          a_out = a_in->loopBack2Out();
        }
      PstOutContainerInterface* tmp = a_out;
      a_out = NULL;
      return tmp;
    }
  };



  class DksContainerMessage: public DksBcMessage{
  public:
    PstDataContainer *a_cnt;
    DksContainerMessage(PstDataContainer* p):a_cnt(p){;}
    ~DksContainerMessage(){
      if(a_cnt)
        delete a_cnt;
    }
  };


  // ********************** PS_DKS_userClass ***********************************'

  void PS_DKS_userClass::dks_functional(){
    a_kbrInterface->m_kbrFunctional();
  }

  void PS_DKS_userClass::m_newResponsability(int begin, int end, int n, DksMessage* msg){
    PsDksMessage *pMsg = static_cast<PsDksMessage*>(msg);
    a_kbrInterface->m_kbrNewResp(begin, end, n, pMsg->getInPstContainer());
  }

  void PS_DKS_userClass::m_receivedRoute(int Key, DksMessage *msg){
    PsDksMessage *pMsg = static_cast<PsDksMessage*>(msg);
     a_kbrInterface->m_kbrMessage(Key, pMsg->getInPstContainer());
  }

  void PS_DKS_userClass::m_receivedRouteNext(int Key, DksMessage *msg){
    Assert(0);
  }

  void PS_DKS_userClass::m_receivedBroadcast(DksBcMessage* cont){
    DksContainerMessage* dcm = static_cast<DksContainerMessage*>(cont);
    a_kbrInterface->m_bcMessage(dcm->a_cnt->m_getPstIn());
  }
  void PS_DKS_userClass::pushDksBcMessage(MsgContainer* msg, DksBcMessage* cont){
    DksContainerMessage* dcm = static_cast<DksContainerMessage*>(cont);
    msg->pushADC(dcm->a_cnt->m_createReplica());
  }

  DksBcMessage *PS_DKS_userClass::popDksBcMessage(MsgContainer* msg){
     ExtDataContainerInterface* dcv = msg->popADC();
     PstDataContainer* pdc = static_cast<PstDataContainer*>(dcv);
     return new DksContainerMessage(pdc);
  }


  DksMessage*  PS_DKS_userClass::m_divideResp(int start, int stop, int n){
    PstOutContainerInterface* resp = a_kbrInterface->m_kbrDivideResp(start, stop, n);
    return new PsDksMessage(resp);
  }

  void PS_DKS_userClass::pushDksMessage(MsgContainer* msg, DksMessage* dm){
    PsDksMessage* pdm = static_cast<PsDksMessage*>(dm);
    PstOutContainerInterface* pst = pdm->getOutPstContainer();
    gf_pushPstOut(msg, pst) ;
  }

  DksMessage *PS_DKS_userClass::popDksMessage(MsgContainer* msg){
    PstInContainerInterface* pst = gf_popPstIn(msg);
    PsDksMessage* pdm = new PsDksMessage(pst);
    return static_cast<DksMessage*>(pdm);
  }

  PS_DKS_userClass::PS_DKS_userClass(DSS_Environment* env, KbrCallbackInterface* inf):
    DSS_Environment_Base(env), a_kbrInstance(NULL), a_kbrInterface(inf)
  {;}

  void
  PS_DKS_userClass::m_setKbrInstance(KbrInstance* inst){
    a_kbrInstance = inst;
  }
  void
  PS_DKS_userClass::m_setKbrCallBack(KbrCallbackInterface* inf){
    a_kbrInterface = inf;
  }
  KbrInstance*
  PS_DKS_userClass::m_getKbrInstance(){
    return a_kbrInstance;
  }

  KbrCallbackInterface*
  PS_DKS_userClass::m_getKbrCallBack(){
    return a_kbrInterface;

  }

  // *******************************  KbrInstanceImpl ***********************'

  void
  KbrInstanceImpl::m_setCallback(KbrCallbackInterface* intf){
    a_usrClass->m_setKbrCallBack(intf);
  }

  KbrCallbackInterface*
  KbrInstanceImpl::m_getCallback(){
    return a_usrClass->m_getKbrCallBack();
  }
  KbrResult KbrInstanceImpl::m_route(int key, PstOutContainerInterface* value){
    switch(a_node->m_route(key, new PsDksMessage(value)))
      {
      case  DRR_DO_LOCAL:
        return KBR_LOCAL;
      case   DRR_ROUTING:
        return KBR_REMOTE;
      case  DRR_OPENING:
        return KBR_FAILED_OPENING;
      case DRR_CLOSING:
        return KBR_FAILED_CLOSING;
      case  DRR_INVALID_KEY:
        return KBR_FAILED_INVALIDKEY;
      }
    return KBR_LOCAL; // just for the stupid compiler!!
  }


  KbrResult KbrInstanceImpl::m_broadcast(PstOutContainerInterface* value){
    PstOutContainerInterface** plcholder = NULL;
    PstDataContainer* p = new PstDataContainer(a_node->m_getEnvironment(), plcholder);
    // This could be done when we know that a message will be produced...
    printf("Plcholder %d\n", (int)plcholder);
    *plcholder = value;

    switch(a_node->m_broadcastRing(new DksContainerMessage(p)))
      {
      case  DRR_DO_LOCAL:
        return KBR_LOCAL;
      case   DRR_ROUTING:
        return KBR_REMOTE;
      case  DRR_OPENING:
        return KBR_FAILED_OPENING;
      case DRR_CLOSING:
        return KBR_FAILED_CLOSING;
      case  DRR_INVALID_KEY:
        return KBR_FAILED_INVALIDKEY;
      }
    return KBR_LOCAL; // just for the stupid compiler!!
  }


  int  KbrInstanceImpl::m_getId(){
    return a_node->m_getId();
  }

  void
  KbrInstanceImpl::m_join(){
    a_node->m_joinDksRing();
  }
  void KbrInstanceImpl::m_leave(){
    Assert(0);
  }
  void KbrInstanceImpl::m_marshal(DssWriteBuffer* buf){
    a_node->m_marshal(buf);
  }

  KbrInstanceImpl::KbrInstanceImpl(DksInstance* node, PS_DKS_userClass* usr):
    a_usrClass(usr), a_node(node)
  {;}

}
