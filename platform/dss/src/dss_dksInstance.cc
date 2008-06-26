/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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
#pragma implementation "dss_dksInstance.hh"
#endif

#include "dss_dksInstance.hh"
#include "dss_msgLayerInterface.hh"
#include "msl_serialize.hh"
namespace  _dss_internal{


  //***************** Class DksIntance *****************


  DksInstance::DksInstance(int N, int K, int F, DKS_userClass* usr,
                           DSS_Environment* env) :
    DKSNode(N, K, F,env->a_myDSite->m_getShortId() % N, env->a_myDSite, usr),
    NetIdNode(), BucketHashNode<DksInstance>(),
    DSS_Environment_Base(env)
  {
    // Insert the DKS instance in the HT, it can now be reached by messages
    // and has allso been given a unique identity( a net identity).
    env->a_dksInstHT->m_addNetIdentity(this);
    env->a_dksInstHT->insert(this);

    printf("creating DKS %d\n", N);
  }

  DksInstance::DksInstance(int N, int K, int F, NetIdentity ni,
                           DSite* entry, DSS_Environment* env):
    DKSNode(N, K, F,env->a_myDSite->m_getShortId() % N, env->a_myDSite),
    NetIdNode(ni), BucketHashNode<DksInstance>(),
    DSS_Environment_Base(env),
    a_joins(entry)
  {
    // Inserts the DKS in the HT, the identity of the DKS is definedby its creator(passed in the ni arg).
    env->a_dksInstHT->insert(this);
  }


  void
  DksInstance::m_marshal(DssWriteBuffer* bb){
    printf("Marshaling dksInstance\n");
    gf_marshalNetIdentity(bb, m_getNetId());
    ::gf_MarshalNumber(bb, a_N);
    ::gf_MarshalNumber(bb, a_K);
    ::gf_MarshalNumber(bb, a_F);
    m_getEnvironment()->a_myDSite->m_marshalDSite(bb);
    printf("====>   Done\n");
  }

  int
  DksInstance::m_getMarshaledSize() const {
    return m_getNetId().getMarshaledSize() +
      m_getEnvironment()->a_myDSite->m_getMarshaledSize() + 3 * sz_MNumberMax;
  }

  MsgContainer*
  DksInstance::m_createDKSMsg(){
    MsgContainer *msg = m_getEnvironment()->a_msgnLayer->createAppSendMsgContainer();
    msg->pushIntVal(M_DKS_MSG);
    gf_pushNetIdentity(msg, m_getNetId());
    msg->pushIntVal(a_myId.id);
    return msg;
  }

  void
  DksInstance::m_joinDksRing(){
    m_joinNetwork(a_joins);
  }

  // ******************** class DksInstanceHT **************************

  bool
  DksInstanceHT::m_unmarshalDksInstance(DssReadBuffer* bb,   DksInstance* &inst){
    printf("Unmarshaling dksInstance\n");
    NetIdentity ni = gf_unmarshalNetIdentity(bb, m_getEnvironment());
    inst = lookup(ni.hashCode(), ni);
    int N = ::gf_UnmarshalNumber(bb);
    int K = ::gf_UnmarshalNumber(bb);
    int F = ::gf_UnmarshalNumber(bb);
    DSite *entry = m_getEnvironment()->a_msgnLayer->m_UnmarshalDSite(bb);
    printf("====>   Done\n");
    if (inst) return true;
    inst = new DksInstance(N, K, F, ni,entry, m_getEnvironment());
    return false;
  }

  void
  DksInstanceHT::m_redirectMessage(MsgContainer* msgC, DSite* sender){
    NetIdentity ni =  gf_popNetIdentity(msgC);
    DksInstance* inst = lookup(ni.hashCode(), ni);
    if (inst){
      inst->msgReceived(msgC, sender);
    } else {
      printf("DKS ring not found!\n");
      Assert(0);
    }
  }

  void
  DksInstanceHT::m_gcResources(){
    for (DksInstance* n = getFirst() ; n; n = getNext(n)) {
      n->m_gcResources();
    }
    checkSize();
  }
  void
  DksInstanceHT::m_siteStateChane(DSite* s, const FaultState& state){
    for (DksInstance* n = getFirst() ; n; n = getNext(n)) {
      n->nodeFailed(s, state, NULL);
    }
  }

  DksInstanceHT::DksInstanceHT(int sz, DSS_Environment* env):
    NetIdHT(env), BucketHashTable<DksInstance>(sz)
  {}
}
