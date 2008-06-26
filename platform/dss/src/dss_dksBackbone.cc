/*
 *  Authors:
 *    Erik Klintskog, 2003 (erik@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
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
#pragma implementation "dss_dksBackbone.hh"
#endif

#include "dss_dksBackbone.hh"
#include "coordinator_mobile.hh"
namespace _dss_internal{


#define DBB_RANGE ((1<<16) - 1)


  // ********************** MISC ********************************************

  int lf_hashNetIdentity(NetIdentity ni){
    return (ni.site->m_getShortId() + ni.index) % DBB_RANGE;
  }

  bool lf_keyInInterval(int key, int start, int stop, int n){
    if(stop < start)
      return (key > start || key < stop);
    return (key > start && key < stop);

  }


  void  lf_transferBackboneService(BackboneServiceNode *db, LargeMessage *lm){
    lm->pushNetId(db->m_getNetId());
    lm->pushInt(db->a_srv->m_getType());
    lm->pushLM(db->a_srv->m_transferService());
  }

  // *********************** class DksBackboneMessage ***********************

  enum BbMsgType{
    DBMT_SERVICE_MSG,
    DBMT_SERVICE_INSERT,
    DBMT_ROUTE_NOT_USED,
    DBMT_TRANSFER_RESP
  };


  class DksBackboneMessage: public DksMessage{
    LargeMessage *a_lm;
  public:
    DksBackboneMessage(LargeMessage* lm):
      a_lm(lm)
    {;}

    DksBackboneMessage(MsgContainer* msg):
      a_lm(gf_popLargeMessage(msg))
    {;}

    void m_pack(MsgContainer* msg){
      gf_pushLargeMessage(msg, a_lm);
    }

    LargeMessage* m_getLM(){
      LargeMessage *ans = a_lm;
      a_lm = NULL;
      return ans;
    }
  };
  // *********************** class DksBackBone *******************************


  DksBackbone::DksBackbone(DSS_Environment* env):
    DSS_Environment_Base(env), a_serviceHT(100, env),
    a_instance(NULL){
    ;
  }
  DksBackbone::DksBackbone(DksInstance *instance, DSS_Environment* env):
    DSS_Environment_Base(env), a_serviceHT(100, env),
    a_instance(instance){
    ;
  }

  void DksBackbone::m_sendToService(NetIdentity ni, LargeMessage* lm){
    int hsh =  lf_hashNetIdentity(ni);
    printf("DksBackbone::m_sendToService %d\n",hsh);
    LargeMessage *lm_msg = new LargeMessage();
    lm_msg->pushInt(DBMT_SERVICE_MSG);
    lm_msg->pushNetId(ni);
    lm_msg->pushLM(lm);
    if(a_instance->m_route(hsh, new DksBackboneMessage(lm_msg)) == DRR_DO_LOCAL){
      printf("doing it local\n");
      BackboneServiceNode *nin = a_serviceHT.lookup(ni.hashCode(), ni);
      if(nin){
        printf("found service\n");
        BackboneService *bs = nin->a_srv;
        lm_msg->popInt();
        lm_msg->popNetId();
        bs->m_messageReceived(lm_msg->popLM(), m_getEnvironment());
      }
      delete lm_msg;
    }
    else{
      printf("sending remote\n");
    }
  }

  void DksBackbone::m_insertService(NetIdentity ni, BackboneService* bs){
    int hsh = lf_hashNetIdentity(ni);
    LargeMessage *lm_msg = new LargeMessage();
    lm_msg->pushInt(DBMT_SERVICE_INSERT);
    lm_msg->pushNetId(ni);
    lm_msg->pushInt(bs->m_getType());
    lm_msg->pushLM(bs->m_transferService());
    printf("Inserting %d -- ", hsh);
    if(a_instance->m_route(hsh, new DksBackboneMessage(lm_msg)) == DRR_DO_LOCAL){
      printf("local \n");
      BackboneServiceNode* srv = new BackboneServiceNode(ni, bs);
      a_serviceHT.insert(srv);
    }
    else{
      printf("remote\n");
      delete bs;
    }
  }

  void DksBackbone::m_receivedRoute(int Key, DksMessage* dmsg){
    printf("DksBackbone::m_receivedRoute --> key %d\n", Key);
    LargeMessage *lm_msg = static_cast<DksBackboneMessage*>(dmsg)->m_getLM();
    switch(static_cast<BbMsgType>(lm_msg->popInt())){
    case  DBMT_SERVICE_MSG:{
      NetIdentity ni = lm_msg->popNetId();
      BackboneServiceNode *nin = a_serviceHT.lookup(ni.hashCode(), ni);
      LargeMessage *lm = lm_msg->popLM();
      if(nin){
        BackboneService *bs = nin->a_srv;
        bs->m_messageReceived(lm, m_getEnvironment());
      }
      else{
        printf("no such DksBackboneService at this process\n");
      }
      break;
    }
    case DBMT_SERVICE_INSERT:{
      m_installBackboneService(lm_msg);
      break;
    }
    case DBMT_ROUTE_NOT_USED:{
      dssError(" DksBackbone::m_receivedRoute --> DBMT_ROUTE_NOT_USED not impl yet\n");
      break;
    }
    default:
      dssError(" DksBackbone::m_receivedRoute --> Unknown message type\n");
    }
  }


  void DksBackbone::m_receivedRouteNext(int Key, DksMessage*){
    dssError("DksBackbone::m_receivedRouteNext --> not impl yet");
  }

  void
  DksBackbone::m_installBackboneService(LargeMessage* lm){
    NetIdentity ni = lm->popNetId();
    int type = lm->popInt();
    LargeMessage *ser_lm = lm->popLM();
    switch(type){
    case BST_MOBILE_COORDINATOR:{
      BackboneService *bs = gf_createMcBackbineST(ser_lm);
      BackboneServiceNode* srv = new BackboneServiceNode(ni, bs);
      a_serviceHT.insert(srv);
      break;
    }
    default:
      Assert(0);
    }
  }

  DksMessage* DksBackbone::m_divideResp(int start, int stop, int n){
    printf("Dividing -- drop [%d -- %d]\n", start, stop);
    SimpleList<BackboneServiceNode*> found;
    BackboneServiceNode* bs;
    for (bs = a_serviceHT.getFirst() ; bs; bs = a_serviceHT.getNext(bs)) {
      if(lf_keyInInterval(lf_hashNetIdentity(bs->m_getNetId()), start, stop, n)){
        found.push(bs);
        printf("node %d is a droper\n", lf_hashNetIdentity(bs->m_getNetId()));
      }else{
        printf("node %d is a keeper\n", lf_hashNetIdentity(bs->m_getNetId()));
      }
    }

    LargeMessage *lm = new LargeMessage();
    while (!found.isEmpty()) {
      BackboneServiceNode *db = found.pop();
      a_serviceHT.remove(db);
      lm->pushNetId(db->m_getNetId());
      lm->pushInt(db->a_srv->m_getType());
      lm->pushLM(db->a_srv->m_transferService());
      delete db->a_srv;
      delete db;
    }
    return new DksBackboneMessage(lm);
  }

  void DksBackbone::m_newResponsability(int begin, int end, int n, DksMessage* msg){
    printf("we are now responsible for %d to %d\n", begin, end);
    LargeMessage* lm = static_cast<DksBackboneMessage*>(msg)->m_getLM();
    while(!lm->isEmpty())
      m_installBackboneService(lm);
  }

  void DksBackbone::dks_functional(){
    printf("the backbone is now up\n");
  }
  void DksBackbone::pushDksMessage(MsgContainer* target, DksMessage* msg){
    static_cast<DksBackboneMessage*>(msg)->m_pack(target);
  }

  DksMessage *DksBackbone::popDksMessage(MsgContainer* msg){
    return new DksBackboneMessage(msg);
  }


  // These methods are only implemented as dummies. Right now we
  // dont allow broadcasts over the backbone network. However, there is
  // nothing that actually prevents us from doing it. It is just a matter
  // of interfaces. How to find the searched for service is an open
  // question. Furthermore, what is a service in this case?
  void DksBackbone::m_receivedBroadcast(DksBcMessage*){
    dssError("DksBackbone::m_receivedBroadcast --> not implemented");
  }
  void DksBackbone::pushDksBcMessage(MsgContainer*, DksBcMessage*){
    dssError("DksBackbone::pushDksBcMessage --> not implemented");
  }
  DksBcMessage *DksBackbone::popDksBcMessage(MsgContainer*){
    dssError("DksBackbone::popDksBcMessage --> not implemented");
    return NULL;
  }
}
