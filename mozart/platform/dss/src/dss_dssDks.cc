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

//#if defined(INTERFACE)
//#pragma implementation "dss_dssDKS.hh"
//#endif

#include "dss_dssDks.hh"
#include "dss_msgLayerInterface.hh"
#include "msl_serialize.hh"
namespace  _dss_internal{ 
  

  // **************** DummyDksMessage *************************

  // used when intervals are diveded and installed. The message only 
  // exists localy, when pushed in a message container nothing is actually
  // inserted. 
  
  class DummyDksMessage: public DksMessage{
    static int a_allocated ; 
  public: 
    DummyDksMessage(){
      printf("created DummyDksMessage %d\n", ++a_allocated); 
    }
    ~DummyDksMessage(){
      printf("deleted DummyDksMessage %d\n", --a_allocated); 
    }
  };


  int DummyDksMessage::a_allocated = 0; 
  

  // ******************* DssDksBcMessage *********************

  DssDksBcMessage::DssDksBcMessage(PstDataContainer* p, int a):
    a_cnt(p), aop(a)
  {
    ;
  }
  DssDksBcMessage::~DssDksBcMessage(){
    ;
  }
  int DssDksBcMessage::m_getAop(){
    return aop;
  }
  PstDataContainer* DssDksBcMessage::m_getData(){
    return a_cnt;
  }

  // ********************* DksBcClass *************************
  
  void  DksBcClass::m_receivedRoute(int Key, DksMessage*){
    printf("should not happend DksBcClass::m_receivedRoute\n");
    Assert(0); 
  }
  void DksBcClass::m_receivedRouteNext(int Key, DksMessage*){
    printf("should not happend DksBcClass:m_receivedRouteNext\n"); 
    Assert(0); 
  }
  DksMessage* DksBcClass::m_divideResp(int start, int stop, int n){
    return new DummyDksMessage(); 
  }
  void DksBcClass::m_newResponsability(int begin, int end, int n, DksMessage* msg){
    printf("DksBcClass:m_newResponsability:, maybe we should delete the dksMsg\n"); 
  }
  void DksBcClass::pushDksMessage(MsgContainer* msgC, DksMessage* msg){
    printf("DksBcClass::pushDksMessage maybe we should delete the dksMsg\n");  
  }
  
  DksMessage *DksBcClass::popDksMessage(MsgContainer* msgC){
    return new DummyDksMessage(); 
  }
  
  // internal methods used when the DKSNode works with 
  // broadcast messages. 
  void DksBcClass::pushDksBcMessage(MsgContainer* msg, DksBcMessage* dbm){
    DssDksBcMessage *ddbm = static_cast<DssDksBcMessage*>(dbm);
    msg->pushIntVal(ddbm->m_getAop()); 
    msg->pushADC(ddbm->m_getData()->m_createReplica()); 
  }
  
  DksBcMessage *DksBcClass::popDksBcMessage(MsgContainer* msg){
    int aop = msg->popIntVal(); 
    ExtDataContainerInterface* dcv = msg->popADC();
    return new DssDksBcMessage(static_cast<PstDataContainer*>(dcv), aop); 
  }
  void DksBcClass::m_receivedBroadcast(DksBcMessage*){
    dssError("DksBcClass::m_receivedBroadcast ==> this method should be overridden\n"); 
  }
  
  DksBcClass::DksBcClass(){ // the darn compiler is giving me a hard time, lets hope this works
    ;
  }
  
  
}
