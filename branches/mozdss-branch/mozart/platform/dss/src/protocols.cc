/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *    Zacharias El Banna (zeb@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
#include "protocol_dksBroadcast.hh"
#include "protocol_migratory.hh"
#include "protocol_eagerinvalid.hh"     
#include "protocol_once_only.hh"
#include "protocol_immediate.hh"	     
#include "protocol_pilgrim.hh"
#include "protocol_immutable_eager.hh"  
#include "protocol_simple_channel.hh"
#include "protocol_immutable_lazy.hh"   
#include "protocol_transient_remote.hh"
#include "protocol_lazyinvalid.hh"


namespace _dss_internal{ 
  

#ifdef DEBUG_CHECK
  int ProtocolManager::a_allocated=0;
  int ProtocolProxy::a_allocated=0;
#endif
  
  
  char *ProtocolProxy::m_stringrep(){
    static char buf[45];
    // Erik, removed a printout that was pretybad.
    sprintf(buf,"Protocol:%d",p_name);
    return buf;
  } 
  
  
  //// Proxy to Manager /////
  void gf_sendProxyToManager(Proxy* p, int i1){
    MsgContainer *msgC = p->m_createCoordProtMsg();
    gf_createSndMsg(msgC,i1); 
    p->m_sendToCoordinator(msgC);
  }
  ///// Proxy to Proxy
  void gf_sendProxyToProxy(Proxy* p, DSite* s, int i1){
    MsgContainer *msgC = p->m_createProxyProtMsg();
    gf_createSndMsg(msgC,i1);
    s->m_sendMsg(msgC); 
  }

  void gf_sendProxyToProxy(Proxy* p, DSite* s, int i1, DSite* s1){
    MsgContainer *msgC = p->m_createProxyProtMsg();
    gf_createSndMsg(msgC,i1, s1);
    s->m_sendMsg(msgC); 
  }
  
   void gf_sendProxyToProxy(Proxy* p, DSite* s, int i1,
		      PstOutContainerInterface* out ){
     MsgContainer *msgC = p->m_createProxyProtMsg();
     gf_createSndMsg(msgC,i1,out);
     s->m_sendMsg(msgC); 
  }
  
  //// Manager to Proxy 
  
  
void gf_sendManagerToProxy(Coordinator* m, DSite* s, int i1){
    MsgContainer *msgC = m->m_createProxyProtMsg();
    gf_createSndMsg(msgC,i1);
    s->m_sendMsg(msgC); 
  }


  void gf_sendManagerToProxy(Coordinator* m, DSite* s, int i1, DSite *s1){
    MsgContainer *msgC = m->m_createProxyProtMsg();
    gf_createSndMsg(msgC,i1,s1);
    s->m_sendMsg(msgC); 
  }
  
  ProtocolManager *gf_createProtManager(MsgContainer* msgC, ProtocolName pn){
    switch(pn){
    case PN_MIGRATORY_STATE: 
      return  new ProtocolMigratoryManager(msgC);   
      break;
    case PN_SIMPLE_CHANNEL:    
      return  new ProtocolSimpleChannelManager(msgC);  
      break;
    case PN_EAGER_INVALID:  
      return new ProtocolEagerInvalidManager(msgC); 
      break;
    case PN_TRANSIENT:     
      return new ProtocolOnceOnlyManager(msgC);    
      break;
    default:
      return NULL;
    }
  }
 void gf_createProtocolProxyManager(ProtocolName prot, DSS_Environment* env, ProtocolManager *&pman, ProtocolProxy *&pprox){
      
      switch(prot){
      case PN_DKSBROADCAST:
	pman  = new ProtocolDksBcManager();
	pprox = new ProtocolDksBcProxy();
	static_cast<ProtocolDksBcProxy*>(pprox)->m_initHome(env); 
	break;
      case PN_SIMPLE_CHANNEL:
	pman  = new ProtocolSimpleChannelManager();
	pprox = new ProtocolSimpleChannelProxy();
	break;
      case PN_EAGER_INVALID:
	pman  = new ProtocolEagerInvalidManager(env->a_myDSite); 
	pprox = new ProtocolEagerInvalidProxy();
	break;
      case PN_LAZY_INVALID:
	pman  = new ProtocolLazyInvalidManager(env->a_myDSite); 
	pprox = new ProtocolLazyInvalidProxy();
	break;
      case PN_MIGRATORY_STATE:
	pman  = new ProtocolMigratoryManager(env->a_myDSite);
	pprox = new ProtocolMigratoryProxy();
	break;
      case PN_TRANSIENT:
	pprox = new ProtocolOnceOnlyProxy();
	pman  = new ProtocolOnceOnlyManager(env->a_myDSite);
      break;
      case PN_TRANSIENT_REMOTE:
	pprox = new ProtocolTransientRemoteProxy();
	pman  = new ProtocolTransientRemoteManager(env->a_myDSite);
	break; 
      case PN_PILGRIM_STATE: 
	pprox = new ProtocolPilgrimProxy(env->a_myDSite);
	pman  = new ProtocolPilgrimManager(env->a_myDSite);
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
      default: pprox = NULL; pman = NULL; Assert(0);
      }
    }
  
  
  
  ProtocolProxy* gf_createRemoteProxy(ProtocolName prot, DSite* myDSite){
    switch(prot){
    case PN_DKSBROADCAST:    return new ProtocolDksBcProxy(); break; 
    case PN_SIMPLE_CHANNEL:  return new ProtocolSimpleChannelProxy();  break;
    case PN_MIGRATORY_STATE: return new ProtocolMigratoryProxy(); break;
    case PN_TRANSIENT:       return new ProtocolOnceOnlyProxy();      break;
    case PN_TRANSIENT_REMOTE:return new ProtocolTransientRemoteProxy();      break;
    case PN_EAGER_INVALID:   return new ProtocolEagerInvalidProxy(); break;
    case PN_LAZY_INVALID:    return new ProtocolLazyInvalidProxy(); break;
    case PN_PILGRIM_STATE:   return new ProtocolPilgrimProxy(myDSite); break;
    case PN_IMMUTABLE_LAZY:  return new ProtocolImmutableLazyProxy(); break;
    case PN_IMMUTABLE_EAGER: return new ProtocolImmutableEagerProxy(); break;
    case PN_IMMEDIATE:       return new ProtocolImmediateProxy(); break;
    default:  Assert(0); return NULL;
    }
  }
  
  
  
  
}

