/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *    Zacharias El Banna (zeb@sics.se)
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *
 *  Copyright:
 *    Erik Klintskog,  2003
 *    Zacharias El Banna, 2003
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
#pragma implementation "protocols.hh"
#endif

#include "protocols.hh"
#include "protocol_migratory.hh"
#include "protocol_invalid.hh"
#include "protocol_once_only.hh"
#include "protocol_immediate.hh"
#include "protocol_pilgrim.hh"
#include "protocol_immutable_eager.hh"
#include "protocol_simple_channel.hh"
#include "protocol_immutable_lazy.hh"
#include "protocol_transient_remote.hh"
#include "protocol_sited.hh"

namespace _dss_internal{

  // Quick description of the basic protocol.
  //
  // Registration of proxy P:
  //    P                   M
  //    |---PROT_REGISTER-->|
  //    |<--PROT_PERMFAIL---|   if entity is permfail
  //
  // Deregistration of proxy P:
  //    P                     M
  //    |---PROT_DEREGISTER-->|
  //
  // Proxy P makes the entity fail:
  //    P                   M                   P'
  //    |---PROT_PERMFAIL-->|                   |
  //    |<--PROT_PERMFAIL---|---PROT_PERMFAIL-->| (to all registered proxies)

#ifdef DEBUG_CHECK
  int ProtocolManager::a_allocated=0;
  int ProtocolProxy::a_allocated=0;
#endif

  /************************* ProtocolManager *************************/

  void ProtocolManager::sendMigrateInfo(MsgContainer* msg) {
    msg->pushIntVal(a_status);
    msg->pushIntVal(a_proxies.size());
    while (!a_proxies.isEmpty()) msg->pushDSiteVal(a_proxies.pop());
  }

  ProtocolManager::ProtocolManager(MsgContainer* msg) :
    a_coordinator(NULL), a_proxies()
  {
    a_status = msg->popIntVal();
    int n = msg->popIntVal();
    while (n--) registerProxy(msg->popDSiteVal());
  }

  void ProtocolManager::m_siteStateChange(DSite* s, const FaultState& state) {
    // ignore proxies in states FS_LOCAL_PERM or FS_GLOBAL_PERM
    if (state & FS_PERM) a_proxies.remove(s);
  }

  void ProtocolManager::makePermFail() {
    a_status |= 1;
    while (!a_proxies.isEmpty()) sendToProxy(a_proxies.pop(), PROT_PERMFAIL);
  }

  /************************* ProtocolProxy *************************/

  char *ProtocolProxy::m_stringrep(){
    static char buf[45];
    // Erik, removed a printout that was pretybad.
    sprintf(buf,"Protocol:%d",p_name);
    return buf;
  }

  OpRetVal ProtocolProxy::protocol_Register() {
    if (!isRegistered()) {
      setRegistered(true); sendToManager(PROT_REGISTER);
    }
    return DSS_SKIP;
  }

  OpRetVal ProtocolProxy::protocol_Deregister() {
    if (isRegistered()) {
      setRegistered(false); sendToManager(PROT_DEREGISTER);
    }
    return DSS_SKIP;
  }

  void ProtocolProxy::makePermFail(FaultState s) {
    if (s == FS_GLOBAL_PERM) {
      a_status = (a_status & ~3) | 1;   // permfailed and deregistered
      a_proxy->updateFaultState(FS_STATE_GLOBAL_PERM);
    }
    while (!a_susps.isEmpty()) a_susps.pop()->resumeFailed();
  }

  OpRetVal ProtocolProxy::protocol_Kill() {
    if (!isPermFail()) sendToManager(PROT_PERMFAIL);
    return DSS_SKIP;
  }

  // ** Entity operations: default implementation
  OpRetVal
  ProtocolProxy::operationMonitor() {
    return protocol_Register();
  }

  OpRetVal
  ProtocolProxy::operationKill() {
    return protocol_Kill();
  }

  OpRetVal
  ProtocolProxy::operationRead(GlobalThread*, PstOutContainerInterface**&) {
    return DSS_INTERNAL_ERROR_NO_OP;
  }

  OpRetVal
  ProtocolProxy::operationWrite(GlobalThread*, PstOutContainerInterface**&) {
    return DSS_INTERNAL_ERROR_NO_OP;
  }

  OpRetVal
  ProtocolProxy::operationWrite(PstOutContainerInterface**&) {
    return DSS_INTERNAL_ERROR_NO_OP;
  }

  OpRetVal
  ProtocolProxy::operationBind(GlobalThread*, PstOutContainerInterface**&) {
    return DSS_INTERNAL_ERROR_NO_OP;
  }

  OpRetVal
  ProtocolProxy::operationAppend(GlobalThread*, PstOutContainerInterface**&) {
    return DSS_INTERNAL_ERROR_NO_OP;
  }

  /****************************************************************/

  // create a pair manager-proxy (on home site)
  void gf_createProtocolProxyManager(ProtocolName prot, DSS_Environment* env,
                                     ProtocolManager *&pman, ProtocolProxy *&pprox) {
    switch(prot){
    case PN_SIMPLE_CHANNEL:
      pman  = new ProtocolSimpleChannelManager(env->a_myDSite);
      pprox = new ProtocolSimpleChannelProxy();
      break;
    case PN_MIGRATORY_STATE:
      pman  = new ProtocolMigratoryManager(env->a_myDSite);
      pprox = new ProtocolMigratoryProxy();
      break;
    case PN_PILGRIM_STATE:
      pman  = new ProtocolPilgrimManager(env->a_myDSite);
      pprox = new ProtocolPilgrimProxy(env->a_myDSite);
      break;
    case PN_EAGER_INVALID:
      pman  = new ProtocolEagerInvalidManager(env->a_myDSite);
      pprox = new ProtocolEagerInvalidProxy();
      break;
    case PN_LAZY_INVALID:
      pman  = new ProtocolLazyInvalidManager(env->a_myDSite);
      pprox = new ProtocolLazyInvalidProxy();
      break;
    case PN_TRANSIENT:
      pman  = new ProtocolOnceOnlyManager(env->a_myDSite);
      pprox = new ProtocolOnceOnlyProxy();
      break;
    case PN_TRANSIENT_REMOTE:
      pman  = new ProtocolTransientRemoteManager(env->a_myDSite);
      pprox = new ProtocolTransientRemoteProxy();
      break;
    case PN_IMMUTABLE_LAZY:
      pman  = new ProtocolImmutableLazyManager();
      pprox = new ProtocolImmutableLazyProxy();
      break;
    case PN_IMMUTABLE_EAGER:
      pman  = new ProtocolImmutableEagerManager();
      pprox = new ProtocolImmutableEagerProxy();
      break;
    case PN_IMMEDIATE:
      pman  = new ProtocolImmediateManager();
      pprox = new ProtocolImmediateProxy();
      break;
    case PN_SITED:
      pman  = new ProtocolSitedManager();
      pprox = new ProtocolSitedProxy();
      break;
    default:
      pman = NULL;
      pprox = NULL;
      Assert(0);
    }
  }

  // create a remote proxy
  ProtocolProxy* gf_createRemoteProxy(ProtocolName prot, DSite* myDSite){
    switch(prot){
    case PN_SIMPLE_CHANNEL:   return new ProtocolSimpleChannelProxy();
    case PN_MIGRATORY_STATE:  return new ProtocolMigratoryProxy();
    case PN_PILGRIM_STATE:    return new ProtocolPilgrimProxy(myDSite);
    case PN_EAGER_INVALID:    return new ProtocolEagerInvalidProxy();
    case PN_LAZY_INVALID:     return new ProtocolLazyInvalidProxy();
    case PN_TRANSIENT:        return new ProtocolOnceOnlyProxy();
    case PN_TRANSIENT_REMOTE: return new ProtocolTransientRemoteProxy();
    case PN_IMMUTABLE_LAZY:   return new ProtocolImmutableLazyProxy();
    case PN_IMMUTABLE_EAGER:  return new ProtocolImmutableEagerProxy();
    case PN_IMMEDIATE:        return new ProtocolImmediateProxy();
    case PN_SITED:            return new ProtocolSitedProxy();
    default: Assert(0); return NULL;
    }
  }

  // create a manager (resulting from manager migration)
  ProtocolManager *gf_createProtManager(MsgContainer* msgC, ProtocolName pn){
    switch(pn){
    case PN_SIMPLE_CHANNEL:   return new ProtocolSimpleChannelManager(msgC);
    case PN_MIGRATORY_STATE:  return new ProtocolMigratoryManager(msgC);
    case PN_PILGRIM_STATE:    return new ProtocolPilgrimManager(msgC);
    case PN_EAGER_INVALID:    return new ProtocolEagerInvalidManager(msgC);
    case PN_LAZY_INVALID:     return new ProtocolLazyInvalidManager(msgC);
    case PN_TRANSIENT:        return new ProtocolOnceOnlyManager(msgC);
    case PN_TRANSIENT_REMOTE: return new ProtocolTransientRemoteManager(msgC);
    default: Assert(0); return NULL;
    }
  }

}
