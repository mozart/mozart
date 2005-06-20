/* 
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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
#pragma implementation "protocol_once_only.hh"
#endif

#include "protocol_once_only.hh"
namespace _dss_internal{ //Start namespace


  // Yes I know, I've keept the old shity names. 
  // It takes some time to figure them out, so 
  // I've given them a short descrition: 
  //
  namespace{
    enum OO_msg_names{
      OO_REGISTER,     // register a proxy at the manager
      OO_DEREGISTER,   // remove the registration at the manager
      OO_SURRENDER,    // try to bind the once only Manager
      OO_GETSTATUS,    // get the status (bound or not) of a Manager
      OO_ACKNOWLEDGE,  // sent to a proxy if a earlier surrender was succesfull
      OO_REDIRECT,     // information from the Manager to  aproxy about a binding. 
      OO_RECEIVESTATUS, // answer to a getstatus, received at a proxy.   
      OO_UPDATE,         // update from manager to proxie(s)
      OO_UPDATE_REQUEST, // request to update from proxy to manager
      OO_UPDATE_CONFIRM // update confirmation from manager to requesting proxy
    };

    // Assumes that the two netaddresses are not equal. 
    bool inline if_CompareNetAddress(Proxy* const a1, Proxy* const a2){
      return a1->m_getNetId() <  a2->m_getNetId();
    }
  }
  
  void ProtocolOnceOnlyManager::sendRedirect(DSite *s){
    Assert(manager->m_getProxy() != NULL);
    MsgContainer *msgC = manager->m_createProxyProtMsg();
    msgC->pushIntVal(OO_REDIRECT);
    gf_pushPstOut(msgC, manager->retrieveEntityState());
    manager->m_sendToProxy(s, msgC); 
  }
  
  ProtocolOnceOnlyManager::ProtocolOnceOnlyManager(DSite* const site):
    a_proxies(new OneContainer<DSite>(site,NULL)),a_bound(false){
    // Created OnceOnly manager
  }

  ProtocolOnceOnlyManager::ProtocolOnceOnlyManager(MsgContainer* const msgC):a_proxies(NULL),a_bound(false){
    a_bound = ((msgC->popIntVal())!=0);
    for(int len = msgC->popIntVal(); len > 0 ; len --){
      a_proxies = new OneContainer<DSite>(msgC->popDSiteVal(),NULL);
    }
  }

  void ProtocolOnceOnlyManager::sendMigrateInfo(MsgContainer* msgC){
    int len=0; 
    msgC->pushIntVal(a_bound); 
    OneContainer<DSite> *ptr = a_proxies;
    for(; ptr != NULL; ptr = ptr->a_next) len ++; 
    msgC->pushIntVal(len);
    for(ptr = a_proxies; ptr != NULL; ptr = ptr->a_next) msgC->pushDSiteVal(ptr->a_contain1);
  }


  void
  ProtocolOnceOnlyManager::makeGCpreps(){
    t_gcList(a_proxies);
  }
  
  void
  ProtocolOnceOnlyManager::msgReceived(MsgContainer * msg, DSite* s){
    int msgType = msg->popIntVal();
    switch(msgType){
    case OO_REGISTER: {
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received REGISTER %p",this,s);
      if (a_bound)
	sendRedirect(s);
      else
	a_proxies = new OneContainer<DSite>(s, a_proxies);
      break;
    }
    case OO_DEREGISTER:{
      //Remove proxy from list
      // If the once_only is bound, the 
      // proxy has allready been removed. 
      // This latecomming request to unregister 
      // can then be droped. 
      if(a_bound) 
	break; 
      else{
	dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received DEREGISTER %p\n",this,s);
#ifdef DEBUG_CHECK
	bool t = t_deleteCompare(&a_proxies, s);
	Assert(t);
#else
	t_deleteCompare(&a_proxies, s);
#endif
	break;
      }
    }
    case OO_SURRENDER: {
      if (a_bound) break; 
      dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Received Surrender",this);
      AbsOp aop    = static_cast<AbsOp>(msg->popIntVal());
      GlobalThread *id  = gf_popThreadIdVal(msg, manager->m_getEnvironment());
      ::PstInContainerInterface *builder = gf_popPstIn(msg);
      Assert(manager->m_getProxy() != NULL);
      ::PstOutContainerInterface *ans;
      // Erik, look at this one...
      manager->m_doe(aop,id,NULL,builder, ans);
      a_bound = true; 
      while(a_proxies!=NULL){
	OneContainer<DSite> *pl=a_proxies->a_next; 
	sendRedirect(a_proxies->a_contain1);
	delete a_proxies; 
	a_proxies = pl; 
      }
      break;
    }
    case OO_UPDATE_REQUEST:{
      int aop = msg->popIntVal();
      GlobalThread *id  = gf_popThreadIdVal(msg, manager->m_getEnvironment());
      ::PstInContainerInterface *builder = gf_popPstIn(msg);
      ::PstOutContainerInterface *ans = builder->loopBack2Out();
      for(OneContainer<DSite> *pl = a_proxies; pl != NULL ; pl=pl->a_next){
	if (pl->a_contain1 != s){
	  MsgContainer *msgC = manager->m_createProxyProtMsg();
	  msgC->pushIntVal(OO_UPDATE);
	  msgC->pushIntVal(aop); 
	  gf_pushPstOut(msgC,ans->duplicate()); 
	  manager->m_sendToProxy(s,msgC); 
	}
      }
      MsgContainer *msgC = manager->m_createProxyProtMsg();
      msgC->pushIntVal(OO_UPDATE_CONFIRM);
      msgC->pushIntVal(aop); 
      gf_pushPstOut(msgC, ans);
      gf_pushThreadIdVal(msgC,id);
	    
      manager->m_sendToProxy(s,msgC);
      break; 
      
    }
    case OO_GETSTATUS: 
      manager->m_getEnvironment()->a_map->GL_error("Msg %d to variable not implemented yet", msgType);
    default: 
      manager->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }

  ProtocolOnceOnlyManager::~ProtocolOnceOnlyManager(){ t_deleteList(a_proxies); }
  

  void
  ProtocolOnceOnlyProxy::msgReceived(MsgContainer * msg, DSite*)
  {
    int msgType = msg->popIntVal();
    switch(msgType){
    case OO_ACKNOWLEDGE:
      dssError("Acknowledge not implemented yet, sorry");
      break;
    case OO_REDIRECT:{
      a_bound = true;
      PstInContainerInterface* cont = gf_popPstIn(msg);
      //
      // The home-proxy does not have to perform the bind, it is
      // allready done by the Manager... 
      //
      if (a_proxy->m_getProxyStatus() != PROXY_STATUS_HOME) 
	a_proxy->installEntityState(cont); 
      TwoContainer<GlobalThread,ProtOOop>* tmp;
      while(a_susps)
	{
	  // not really clear about this
	  (a_susps->a_contain1)->resumeDoLocal(NULL);
	  tmp = a_susps->a_next;
	  delete a_susps;
	  a_susps = tmp; 
	}
      break;
    }
    case OO_UPDATE_CONFIRM:
    case OO_UPDATE:{
      ::PstOutContainerInterface* ans;
      int absOp = msg->popIntVal();
      PstInContainerInterface* cont = gf_popPstIn(msg);
      
      
      a_proxy->m_doe(static_cast<AbsOp>(absOp),NULL, NULL, cont, ans);
      if(msgType == OO_UPDATE_CONFIRM)
	{
	  GlobalThread* id = gf_popThreadIdVal(msg, a_proxy->m_getEnvironment()); 
	  t_deleteCompare(&a_susps, id); 
	  id->resumeDoLocal(NULL); 
	}

      
      TwoContainer<GlobalThread,ProtOOop>* tmp, **ptr = &a_susps; 
      while(*ptr != NULL) 
	{
	  tmp = *ptr; 
	  if(tmp->a_contain2 == ProtOO_wait){
	    tmp->a_contain1->resumeDoLocal(NULL); 
	    (*ptr) = tmp->a_next;
	    delete tmp; 
	  }
	  else{
	    ptr = &(tmp->a_next);
	  }
	}
      
      break; 
    }
      
    case OO_RECEIVESTATUS:
      a_proxy->m_getEnvironment()->a_map->GL_error("Msg %d to variable not implemented yet", msgType);
    default: 
      a_proxy->m_getEnvironment()->a_map->GL_error("Unknown Msg %d to variable", msgType);
    }
  }


  OpRetVal
  ProtocolOnceOnlyProxy::protocol_Terminate(GlobalThread* const th_id,::PstOutContainerInterface**& msg,
					    const AbsOp& aop){
    if (a_bound) {
      msg = NULL; 
      return DSS_PROCEED; 
    }
    
    ::MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
    msgC->pushIntVal(OO_SURRENDER);
    msgC->pushIntVal(aop);
    gf_pushThreadIdVal(msgC,th_id);
    
    msg = gf_pushUnboundPstOut(msgC);
    if (a_proxy->m_sendToCoordinator(msgC) ) 
      {
	a_susps = new TwoContainer<GlobalThread, ProtOOop>(th_id, ProtOO_terminate , a_susps);
	return (DSS_SUSPEND);
      }
    msg = NULL; 
    return DSS_RAISE;
  }
  

  OpRetVal 
  ProtocolOnceOnlyProxy::protocol_Update(GlobalThread* const th_id, ::PstOutContainerInterface**& msg,
			   const AbsOp& aop){
    if (a_bound) {
      msg = NULL; 
      return DSS_PROCEED; 
    }
    ::MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
    msgC->pushIntVal(OO_UPDATE_REQUEST);
    msgC->pushIntVal(aop); 
    gf_pushThreadIdVal(msgC,th_id);
  
    msg = gf_pushUnboundPstOut(msgC);
    if (a_proxy->m_sendToCoordinator(msgC) ) 
      {
	a_susps = new TwoContainer<GlobalThread, ProtOOop>(th_id, ProtOO_write, a_susps);
	return (DSS_SUSPEND);
      }
    msg = NULL; 
    return DSS_RAISE;
  }
  

  

  ProtocolOnceOnlyProxy::ProtocolOnceOnlyProxy():
    ProtocolProxy(PN_TRANSIENT), a_susps(NULL), a_bound(false){
  }

  bool
  ProtocolOnceOnlyProxy::m_initRemoteProt(DssReadBuffer*){
    ::MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
    msgC->pushIntVal(OO_REGISTER);
    dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send REGISTER",this);
    a_proxy->m_sendToCoordinator(msgC);
    return true;
  }

  void
  ProtocolOnceOnlyProxy::makeGCpreps(){
    ;
  }


  ProtocolOnceOnlyProxy::~ProtocolOnceOnlyProxy(){
    Assert(a_susps == NULL);
    if(!a_bound && a_proxy->m_getProxyStatus() == PROXY_STATUS_REMOTE)
      {
	::MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
	msgC->pushIntVal(OO_DEREGISTER);
	dssLog(DLL_BEHAVIOR,"ONCE ONLY (%p): Send DEREGISTER", this);
	a_proxy->m_sendToCoordinator(msgC);
      }
  }
  
} //end namespace
 
